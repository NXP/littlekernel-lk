/*
 * The Clear BSD License
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <kernel/spinlock.h>
#include <kernel/vm.h>
#include <lk/init.h>
#include <trace.h>
#include <assert.h>
#include <err.h>
#include <libfdt.h>
#include <dev/driver.h>
#include <lib/appargs.h>

#define LOCAL_TRACE 0

extern struct driver __drivers[];
extern struct driver __drivers_end[];

extern struct board __boards[];
extern struct board __boards_end[];

extern pmm_arena_t ram_arena;

paddr_t ram_phys_start = MEMBASE;

static struct list_node of_device_list = LIST_INITIAL_VALUE(of_device_list);
static spin_lock_t of_device_list_lock = SPIN_LOCK_INITIAL_VALUE;
const void *main_fdt_base;
#define DEVICE_CFG_MAX_IRQ 4

int fdtdec_get_string_list(const void *fdt,
                                            int offset,
                                            const char *prop_name,
                                            const char**names,
                                            int count)
{
    int str_len, names_count, i;
    const char *strings;
    strings = fdt_getprop(fdt, offset, prop_name, &str_len);
    if (!strings)
        return 0;

    names_count = fdt_stringlist_count(fdt, offset, prop_name);
    if (names_count > count)
        names_count = count;
    for (i = 0; i < names_count; i++) {
        names[i] = strings;
        strings += strlen(strings) + 1;
    }

    return names_count;
}

int fdtdec_get_bool(const void *fdt, int offset, const char *name)
{
    const int32_t *cell;
    int len;

    cell = fdt_getprop(fdt, offset, name, &len);
    return cell != NULL;
}

int fdt_get_cell_size(const void *fdt, int node, const char *prop_name)
{
    const uint32_t *cell;
    int len;

    cell = fdt_getprop(fdt, node, prop_name, &len);
    if (!cell)
        return -FDT_ERR_NOTFOUND;

    return len;
}

int fdt_get_int(const void *blob, int node, const char *prop_name, int *value)
{
    const int *cell;
    int len;

    cell = fdt_getprop(blob, node, prop_name, &len);
    if (cell && (size_t)len >= sizeof(int)) {
        int val = fdt32_to_cpu(cell[0]);
        *value = val;

        return 0;
    }

    return -FDT_ERR_NOTFOUND;
}

int fdt_get_int_array(const void *fdt, int node,
                    const char *prop_name, uint32_t *array, int count)
{
    const uint32_t *cell;
    int len, elems;
    int i;

    cell = fdt_getprop(fdt, node, prop_name, &len);
    if (!cell)
        return -FDT_ERR_NOTFOUND;
    elems = len / sizeof(uint32_t);
    if (count > elems)
        count = elems;
    for (i = 0; i < count; i++)
        array[i] = fdt32_to_cpu(cell[i]);

    return count;
}

static inline uint64_t fdt_read_cell(const fdt32_t *cell, int size)
{
    uint64_t val = 0;
    while (size--)
        val = (val << 32) | fdt32_to_cpu(*(cell++));
    return val;
}

int fdt_get_args_from_phandle(const void *fdt, int src_node,
                              const char *property_name, const char *cells_name,
                              int index, struct fdt_args *args)
{
    const fdt32_t *properties;
    int size;
    int32_t count;
    int node = -1;
    int phandle, i, j, k;

    if (!args)
        return ERR_INVALID_ARGS;

    if (index < 0)
        return ERR_INVALID_ARGS;

    /* size is the total number of items for this property_name */
    properties = fdt_getprop(fdt, src_node, property_name, &size);
    if (!properties)
        return ERR_NOT_FOUND;

    /* parse all property_name entries until index (included) */
    for (i = 0, j = 0; i <= index; i++, j += count + 1) {
        phandle = fdt32_to_cpu(*properties);
        if (!phandle)
            return ERR_NOT_FOUND;

        node = fdt_node_offset_by_phandle(fdt, phandle);
        if (!node) {
            LTRACEF("%s: phandle not found\n",
                    fdt_get_name(fdt, src_node, NULL));
            return ERR_NOT_FOUND;
        }

        /* count is the "#foo-cells" value from the reference node */
        int ret = fdt_get_int(fdt, node, cells_name, &count);
        if (ret) {
            LTRACEF("%s: could not get %s for %s\n",
                    fdt_get_name(fdt, src_node, NULL), cells_name,
                    fdt_get_name(fdt, node, NULL));
            return ERR_NOT_FOUND;
        }

        if ((unsigned) (j + 1 + count) > (size / sizeof(*properties))) {
            LTRACEF("%s: Not enough arguments\n",
                    fdt_get_name(fdt, src_node, NULL));
            return ERR_NOT_FOUND;
        }

        if (count > MAX_PHANDLE_ARGS) {
            LTRACEF("%s: too many arguments %d\n",
                    fdt_get_name(fdt, src_node, NULL), count);
            count = MAX_PHANDLE_ARGS;
        }

        args->node = node;
        args->args_count = count;
        properties++; /* discards node reference */
        for (k = 0; k < count; k++)
            args->args[k] = fdt32_to_cpu(*properties++);
    }

    return 0;
}

int of_get_args_from_phandle(int src_node,
                   const char *property_name,
                   const char *cells_name,
                   int index,
                   struct fdt_args *args)
{
    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    return fdt_get_args_from_phandle(main_fdt_base, src_node, property_name,
                                    cells_name, index, args);
}

vaddr_t _of_get_io_virtual_base(paddr_t phys, size_t size)
{
    unsigned i;
    for ( i = 0; ; i++ ) {
        if (mmu_initial_mappings[i].size == 0)
            break;
        if (mmu_initial_mappings[i].flags == MMU_INITIAL_MAPPING_FLAG_DEVICE) {
            paddr_t mmu_io_base = mmu_initial_mappings[i].phys;
            vaddr_t mmu_io_vbase = mmu_initial_mappings[i].virt;
            paddr_t mmu_io_end = mmu_io_base + mmu_initial_mappings[i].size;
            if ((mmu_io_base <= phys) && (mmu_io_end >= (phys + size))) {
                LTRACEF("mmu: %lx-%lx , phys: %lx\n", mmu_io_vbase, mmu_io_base, phys);
                return mmu_io_vbase - mmu_io_base + phys;
            }
        }
    }
    return 0;
}

static struct device_cfg_reg *of_device_create_reg_config_data(
                        const void *fdt, int offset, unsigned *count)
{
    int lenp, i, str_len, names_count;
    const char *strings;
    int addr_cells_sz = fdt_address_cells(fdt, fdt_parent_offset(fdt, offset));
    int size_cells_sz = fdt_size_cells(fdt, fdt_parent_offset(fdt, offset));
    if (size_cells_sz < 0)
        size_cells_sz = 0;

    const fdt32_t *cell = fdt_getprop(fdt, offset, "reg", &lenp);
    *count = 0;

    if (!cell)
        return NULL;

    lenp /= ((addr_cells_sz + size_cells_sz) * sizeof(uint32_t));
    *count = lenp;

    struct device_cfg_reg *regs = malloc(sizeof(struct device_cfg_reg) * lenp);
    ASSERT(regs);
    memset(regs, 0, sizeof(struct device_cfg_reg) * lenp);

    for (i = 0; i < lenp; i++) {
        regs[i].base = fdt_read_cell(cell, addr_cells_sz);
        cell += addr_cells_sz;
        regs[i].length  = fdt_read_cell(cell, size_cells_sz);
        cell += size_cells_sz;
        regs[i].vbase = _of_get_io_virtual_base(regs[i].base, regs[i].length);
    }

    strings = fdt_getprop(fdt, offset, "reg-names", &str_len);
    if (!strings)
        return regs;

    names_count = fdt_stringlist_count(fdt, offset, "reg-names");
    if (names_count > lenp)
        names_count = lenp;
    for (i = 0; i < names_count; i++) {
        regs[i].name = strings;
        strings += strlen(strings) + 1;
    }

    if (LOCAL_TRACE) {
        for (i = 0; i < lenp; i++) {
            LTRACEF("IO range:%s [%lx-%lx][%d-%d]\n",
                regs[i].name, regs[i].base, regs[i].length,
                addr_cells_sz, size_cells_sz);
        }
    } while (0);

    return regs;
}

struct device_cfg_reg * device_config_get_register_by_name(
                                        const struct device_config_data *config,
                                        const char *name)
{
    unsigned i;
    for (i = 0; i < config->io_cnt; i++) {
        if (config->io_regions[i].name && strcmp(config->io_regions[i].name, name) == 0)
            return &config->io_regions[i];
    }
    return NULL;
}

struct device_cfg_irq * device_config_get_irq_by_name(
                                        const struct device_config_data *config,
                                        const char *name)
{
    unsigned i;
    for (i = 0; i < config->irqs_cnt; i++) {
        if (config->irqs[i].name && strcmp(config->irqs[i].name, name) == 0)
            return &config->irqs[i];
    }
    return NULL;
}

struct device_cfg_clk * device_config_get_clk_by_name(
                                        const struct device_config_data *config,
                                        const char *name)
{
    unsigned i;

    if ((!config) || (!config->clks))
        return NULL;

    struct device_cfg_clks *clks = config->clks;
    struct device_cfg_clk *cfg = clks->cfg;

    for (i = 0; i < clks->clks_cnt; i++) {
        if (clks->name[i] && strcmp(clks->name[i], name) == 0)
            return cfg;
        cfg++;
    }
    return NULL;
}

struct device_cfg_dplls *of_device_get_plls(struct device *dev)
{
    int offset = dev->node_offset;
    const void *fdt = main_fdt_base;
    int lenp, i, str_len, names_count;
    const char *strings;

    strings = fdt_getprop(fdt, offset, "pll-names", &str_len);
    if (!strings)
        return NULL;

    names_count = fdt_stringlist_count(fdt, offset, "pll-names");
    if (names_count == 0)
        return NULL;

    const fdt32_t *cell = fdt_getprop(fdt, offset, "pll-cfg", &lenp);
    if (cell == NULL)
        return NULL;

    struct device_cfg_dplls *plls = malloc(sizeof(struct device_cfg_dplls));
    ASSERT(plls);
    plls->name = malloc(sizeof(char *) * names_count);
    ASSERT(plls->name);
    fdtdec_get_string_list(fdt, offset, "pll-names", plls->name, names_count);
    plls->cfg = malloc(lenp);
    ASSERT(plls->cfg);
    fdt_get_int_array(fdt, offset, "pll-cfg",
                        (uint32_t*) plls->cfg, lenp / sizeof(uint32_t));

    plls->plls_cnt = names_count;

    if (LOCAL_TRACE) {
        struct device_cfg_dpll *pll = plls->cfg;
        for (i = 0; i < names_count; i++) {
            TRACEF("PLL config %s:\n", plls->name[i]);
            TRACEF("\tid-ref (hex): [%x-%x]\n", pll->id, pll->refSel);
            TRACEF("\trate (dec): %d\n", pll->rate);
            TRACEF("\tfreq = ref * (mdiv * 65536 + kdiv) /" \
                   "(65536 * pdiv * (1 << sdiv))\n");
            TRACEF("\tmdiv-kdiv-pdiv-sdiv (dec):[%d-%d-%d-%d]\n",
                    pll->mdiv, pll->kdiv, pll->pdiv, pll->sdiv);

            pll++;
        }
    } while (0);

    return plls;
}

struct device_cfg_config *of_device_get_config_by_name(
                                        struct device *dev, const char *cfg_name)
{
    const struct device_config_data *config = dev->config;
    int offset = dev->node_offset;
    const void *fdt = main_fdt_base;

    if (fdt == NULL)
        return NULL;

    struct device_cfg_config *cfg_config =
        malloc(sizeof(struct device_cfg_config));

    ASSERT(cfg_config);
    memset(cfg_config, 0, sizeof(struct device_cfg_config));
    cfg_config->name = cfg_name;

#define CONFIG_NAME_MAX_LEN     32
    char name[CONFIG_NAME_MAX_LEN];

    int ret = snprintf(name, CONFIG_NAME_MAX_LEN, "config-%s", cfg_name);

    /* Fill clock data */
    LTRACEF("Get clock(s) for config '%s', ret %d\n", name, ret);

    int clk_count;
    clk_count = fdt_stringlist_count(fdt, offset, name);
    if (clk_count == 0)
        goto error;

    LTRACEF("%s config has %d clock(s)\n", name, clk_count);

    fdtdec_get_string_list(fdt, offset, name, cfg_config->clk_name, clk_count);

    struct device_cfg_clk *clk;
    const char *clk_name;
    int i;

    for (i = 0; i < clk_count; i++) {
        clk_name = cfg_config->clk_name[i];
        clk = device_config_get_clk_by_name(config, clk_name);

        LTRACEF(" > %d: clock name '%s'%s\n", i, clk_name,
                (!clk) ? " : doesn't exist !" : "");

        ASSERT(clk);
        cfg_config->clk[i] = clk;
    }

    cfg_config->clk_count = clk_count;

    /* pll */
    cfg_config->pll_init_mask = ((1 << MAX_PLL_PER_CONFIG) - 1);
    ret = snprintf(name, CONFIG_NAME_MAX_LEN, "pll-mask-%s", cfg_name);
    fdt_get_int(fdt, offset, name, (int *) &cfg_config->pll_init_mask);

    /* Fill GPR config */
    ret = snprintf(name, CONFIG_NAME_MAX_LEN, "gpr-%s", cfg_name);

    of_device_get_int32_array(dev, name, cfg_config->gpr,
                              ARRAY_SIZE(cfg_config->gpr));

    /* Fill GPIO config */
    ret = snprintf(name, CONFIG_NAME_MAX_LEN, "gpio-%s", cfg_name);
    ret = of_gpio_request_by_name(offset, name, &cfg_config->gpio, 0);
    LTRACEF("GPIO config %s %s found...\n", name, !ret ? "" : "not");

    /* Fill sai clk mode (0 by default, if property not present) */
    ret = snprintf(name, CONFIG_NAME_MAX_LEN, "sai-%s", cfg_name);
    if (of_device_get_int32(dev, name, &cfg_config->clk_mode))
        cfg_config->clk_mode = 0;

    return cfg_config;

error:
    free(cfg_config);
    return NULL;
}

int of_device_get_args_from_phandle(struct device *dev,
                   const char *property_name,
                   const char *cells_name,
                   int index,
                   struct fdt_args *args)
{
    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    return fdt_get_args_from_phandle(main_fdt_base, dev->node_offset,
        property_name, cells_name, index, args);
}

static struct device_cfg_irq *of_device_create_irq_config_data(
                                const void *fdt, int offset, unsigned *count)
{
    int lenp, i, str_len, names_count;
    const char *strings;
    const fdt32_t *cell = fdt_getprop(fdt, offset, "interrupts", &lenp);
    *count = 0;

    if (!cell)
        return NULL;

    /* FIXME: Interrupt cell size is hardcoded to 1 cell */
    lenp /= sizeof(uint32_t);
    *count = lenp;

    struct device_cfg_irq *irqs = malloc(sizeof(struct device_cfg_irq) * lenp);
    ASSERT(irqs);

    for (i = 0; i < lenp; i++) {
        irqs[i].irq = fdt_read_cell(cell, 1);
        cell += 1;
    }

    strings = fdt_getprop(fdt, offset, "interrupt-names", &str_len);
    if (!strings)
        return irqs;

    names_count = fdt_stringlist_count(fdt, offset, "interrupt-names");
    if (names_count > lenp)
        names_count = lenp;
    for (i = 0; i < names_count; i++) {
        irqs[i].name = strings;
        strings += strlen(strings) + 1;
    }

    if (LOCAL_TRACE) {
        for (i = 0; i < lenp; i++) {
            LTRACEF("IRQ: %s [%x]\n", irqs[i].name, irqs[i].irq);
        }
    } while (0);

    return irqs;
}

static struct device_cfg_pins *of_device_create_pins_config_data(
                                const void *fdt, int offset, unsigned *count)
{
    int lenp, i, str_len, names_count;
    const char *strings;
    struct device_cfg_pins *cfg_pins;
    char prop_name[32];

    *count = 0;
    strings = fdt_getprop(fdt, offset, "pinctrl-names", &str_len);
    if (!strings)
        return NULL;

    names_count = fdt_stringlist_count(fdt, offset, "pinctrl-names");
    if (names_count == 0)
        return NULL;

    *count = names_count;

    cfg_pins = malloc(sizeof(struct device_cfg_pins) * names_count);

    for (i = 0; i < names_count; i++) {
        sprintf(prop_name, "pinctrl-%d", i);
        const fdt32_t *cell = fdt_getprop(fdt, offset, prop_name, &lenp);
        ASSERT(cell);
        cfg_pins[i].pins = malloc(lenp); ;
        cfg_pins[i].pins_cnt = lenp / sizeof(struct device_cfg_pin);
        fdt_get_int_array(fdt, offset, prop_name,
                        (uint32_t*) cfg_pins[i].pins, lenp / sizeof(uint32_t));
        cfg_pins[i].name = strings;
        strings += strlen(strings) + 1;
    }

    if (LOCAL_TRACE) {
        for (i = 0; i < names_count; i++) {
            LTRACEF("Pin config %s\n", cfg_pins[i].name);
            hexdump(cfg_pins[i].pins, lenp);
        }
    } while (0);

    return cfg_pins;
}

static struct device_cfg_clks *of_device_create_clks_config_data(
                                const void *fdt, int offset, unsigned *count)
{
    int lenp, i, str_len, names_count;
    const char *strings;
    struct device_cfg_clks *clks;

    *count = 0;
    strings = fdt_getprop(fdt, offset, "clock-names", &str_len);
    if (!strings)
        return NULL;

    names_count = fdt_stringlist_count(fdt, offset, "clock-names");
    if (names_count == 0)
        return NULL;

    const fdt32_t *cell = fdt_getprop(fdt, offset, "clock-cfg", &lenp);
    if (cell == NULL)
        return NULL;

    *count = names_count;

    clks = malloc(sizeof(struct device_cfg_clks));
    ASSERT(clks);
    clks->name = malloc(sizeof(char *) * names_count);
    ASSERT(clks->name);
    fdtdec_get_string_list(fdt, offset, "clock-names", clks->name, names_count);

    clks->cfg = malloc(lenp);
    ASSERT(clks->cfg);
    fdt_get_int_array(fdt, offset, "clock-cfg",
                        (uint32_t*) clks->cfg, lenp / sizeof(uint32_t));

    clks->clks_cnt = *count;

    if (LOCAL_TRACE) {
        struct device_cfg_clk *clk = clks->cfg;
        for (i = 0; i < names_count; i++) {
            LTRACEF("Clock config %s:\n", clks->name[i]);
            LTRACEF("\troot clk: idx-mux-pre-post: [%x-%x-%x-%x]\n",
                clk->root_clk_idx, clk->root_clk_mux_idx,
                clk->pre_divider, clk->post_divider);
            LTRACEF("\tclk gate: ccgr-root: [%x-%x]\n",
                clk->ccgr, clk->ccgr_root);
            LTRACEF("\tclk rate: [%d]\n", clk->rate);
            clk++;
        }
    } while (0);

    return clks;
}

paddr_t phys_to_dma(paddr_t phys){
    return phys - ((paddr_t)MEMBASE - ram_phys_start);
}

/**
 * Get the client DMA channels
 */
struct device_cfg_dma_channel *of_device_create_dma_channels_config_data(
                                const void *fdt, int offset, unsigned *count)
{
    int lenp, i, str_len, names_count;
    const char *strings;
    struct device_cfg_dma_channel *dma_channels_cfg = NULL;

    LTRACE_ENTRY;
    *count = 0;
    strings = fdt_getprop(fdt, offset, "dma-names", &str_len);
    if (!strings)
        return NULL;
    LTRACEF("strings=%s\n", strings);

    names_count = fdt_stringlist_count(fdt, offset, "dma-names");
    if (names_count == 0)
        return NULL;
    LTRACEF("names_count=%d\n", names_count);

    const fdt32_t *cell = fdt_getprop(fdt, offset, "dmas", &lenp);
    if (cell == NULL)
        return NULL;

    *count = names_count;

    dma_channels_cfg = malloc(sizeof(struct device_cfg_dma_channel) * names_count);
    ASSERT(dma_channels_cfg);
    struct device_cfg_dma_channel *dma_channel_cfg = dma_channels_cfg;
    for (i = 0; i < names_count; i++) {
        dma_channel_cfg->name = strings;
        LTRACEF("dma_channel->name [%d] = %s\n", i, dma_channel_cfg->name);
        /* get DMA parameters */
        int ret;
        ret = fdt_get_args_from_phandle(fdt, offset, "dmas",
                  "#dma-cells",
		  i /*index*/,
		  &dma_channel_cfg->dma_spec /*args*/);
        LTRACEF("ret=%d\n", ret);

        dma_channel_cfg->chan_id = 0; /* ID is assigned later at registration */

        /* move to the next string */
        strings += strlen(strings) + 1;
        dma_channel_cfg++;
    }

    if (LOCAL_TRACE) {
        struct device_cfg_dma_channel *dma_channel_cfg = dma_channels_cfg;
        for (i = 0; i < names_count; i++) {
            LTRACEF("DMA channel name: %s\n", dma_channel_cfg->name);
            LTRACEF("\targ_count=%d, arg: [%d-%d-%d]\n",
                dma_channel_cfg->dma_spec.args_count,
                dma_channel_cfg->dma_spec.args[0],
                dma_channel_cfg->dma_spec.args[1],
                dma_channel_cfg->dma_spec.args[2]);
            dma_channel_cfg++;
        }
    } while (0);

    LTRACE_EXIT;
    return dma_channels_cfg;
}

/**
 * Get the DMA controller instance
 */
static struct device_cfg_dma_device *of_device_create_dma_device_config_data(
                                const void *fdt, int offset)
{
    int lenp;
    struct device *device;
    struct device_cfg_dma_device *dma_device_cfg = NULL;

    LTRACE_ENTRY;

    const fdt32_t *phandle = fdt_getprop(fdt, offset, "#dma-cells", &lenp);
    if (phandle == NULL)
        return NULL;
    LTRACEF("found a DMA device\n");
    int node = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*phandle));
    if (!node)
        return NULL;

    dma_device_cfg = malloc(sizeof(struct device_cfg_dma_device));
    device = of_find_device_by_node(node);
    dma_device_cfg->device = device;

    LTRACE_EXIT;
    return dma_device_cfg;
}

struct device_cfg_dma_channel * device_config_get_dma_channel_by_name(
                                        const struct device_config_data *config,
                                        const char *name)
{
    unsigned i;
    ASSERT(config != NULL);
    ASSERT(config->dma_channels_cfg != NULL);

    for (i = 0; i < config->dma_channels_cnt; i++) {
        if (config->dma_channels_cfg[i].name && strcmp(config->dma_channels_cfg[i].name, name) == 0)
            return &config->dma_channels_cfg[i];
    }
    return NULL;
}


static int of_device_get_bus_id(const void *fdt, int offset)
{
    int lenp;
    const fdt32_t *cell = fdt_getprop(fdt, offset, "bus-id", &lenp);
    if (cell == NULL)
        return -FDT_ERR_NOTFOUND;

    return  fdt_read_cell(cell, 1);
}

static void * of_device_create_config_data(
                            const void *fdt, int offset, size_t private_size)
{
    struct device_config_data * config =
                    malloc(sizeof(struct device_config_data) + private_size);

    ASSERT(config);
    memset(config, 0, sizeof(struct device_config_data) + private_size);

    unsigned count;
    config->io_regions = of_device_create_reg_config_data(fdt, offset, &count);
    config->io_cnt = count;

    config->irqs = of_device_create_irq_config_data(fdt, offset, &count);
    config->irqs_cnt = count;

    config->pins_cfg = of_device_create_pins_config_data(fdt, offset, &count);
    config->pins_cfg_cnt = count;

    config->clks = of_device_create_clks_config_data(fdt, offset, &count);
    config->clks_cnt = count;

    /* client DMA channels */
    config->dma_channels_cfg = of_device_create_dma_channels_config_data(fdt, offset, &count);
    config->dma_channels_cnt = count;
    LTRACEF("dma_channels_cnt=%d\n", count);

    /* DMA instance */
    config->dma_device_cfg = of_device_create_dma_device_config_data(fdt, offset);

    int bus_id = of_device_get_bus_id(fdt, offset);
    if (bus_id < 0)
        bus_id = -1;
    config->bus_id = bus_id;

    return config;
}

static void of_add_device(struct device *dev)
{
    spin_lock_saved_state_t lock_state;
    spin_lock_irqsave(&of_device_list_lock, lock_state);
    list_add_tail(&of_device_list, &dev->node);
    spin_unlock_irqrestore(&of_device_list_lock, lock_state);
}

static struct device * of_device_create(const void *fdt,
                                                int offset, struct driver *drv)
{
    struct device *mydev;

    mydev = malloc(sizeof(struct device));
    ASSERT(mydev);
    memset(mydev, 0, sizeof(struct device));
    const char *name = fdt_get_name(fdt, offset, NULL);
    ASSERT(name);
    mydev->name = name;
    mydev->driver = drv;
    mydev->node_offset = offset;
    mydev->config = of_device_create_config_data(fdt, offset, drv->prv_cfg_sz);
    LTRACEF("Device %s created, bound to driver %s\n", mydev->name, drv->type);

    return mydev;
}

static struct device * of_device_create_and_add(const void *fdt,
                                int offset, struct driver *drv)
{
    struct device * dev = of_device_create(fdt, offset, drv);

    if (!dev)
        return NULL;

    of_add_device(dev);

    return dev;
}

status_t of_device_populate(const void *fdt)
{
    status_t res = NO_ERROR;
    int offset;

    struct driver *drv = __drivers;
    while (drv != __drivers_end) {
        const char *compatible = drv->type;
        LTRACEF("Parsing for %s device node...\n", compatible);

        offset = fdt_node_offset_by_compatible(fdt, -1, compatible);

        while (offset != -FDT_ERR_NOTFOUND) {

            const char *status = fdt_getprop(fdt, offset, "status", NULL);
            if (status &&
                ((strcmp(status, "ok")  == 0) || (strcmp(status, "okay") == 0)))
                of_device_create_and_add(fdt, offset, drv);

            offset = fdt_node_offset_by_compatible(fdt, offset, compatible);
        }

        drv++;
    }

    return res;
}

status_t of_boards_fixup(void)
{
    int offset;

    if (!main_fdt_base)
        return ERR_NOT_FOUND;

    struct board *brd = __boards;
    while (brd != __boards_end) {
        const char *compatible = brd->compatible;
        LTRACEF("Parsing for %s board node, compatible %s ...\n", brd->name, compatible);

        offset = fdt_node_offset_by_compatible(main_fdt_base, -1, compatible);

        while (offset != -FDT_ERR_NOTFOUND) {
            brd->fixup(main_fdt_base, offset);
            offset = fdt_node_offset_by_compatible(main_fdt_base, offset, compatible);
        }

        brd++;
    }
    return 0;
}

struct device * of_find_device_by_node(int offset)
{
    struct device *dev;
    list_for_every_entry(&of_device_list, dev, struct device, node) {
        if (dev->node_offset == offset) {
            return dev;
        }
    }
    return NULL;
}

status_t of_device_init_all(unsigned initlvl)
{
    struct device *dev;
    status_t res = 0;
    TRACEF("Initializing devices, initlvl(%x)...\n", initlvl);
    list_for_every_entry(&of_device_list, dev, struct device, node) {
        if (!(dev->driver->initlvl & initlvl))
            continue;
        LTRACEF("Initializing device %s...\n", dev->name);
        status_t code = device_init(dev);
        if (code < 0) {
            TRACEF("Driver init failed for driver \"%s\", device \"%s\", reason %d\n",
                   dev->driver->type, dev->name, code);

            res = code;
        }
    }

    return res;
}

__WEAK void devcfg_set_dpll(struct device_cfg_dpll *dpll)
{
}

__WEAK void devcfg_set_clock(struct device_cfg_clk *cfg)
{
}

__WEAK void devcfg_set_pin(struct device_cfg_pin *pin)
{
}

void devcfg_set_clocks(struct device_cfg_clks *clks)
{
    unsigned i;
    if ((clks == NULL) || (clks->clks_cnt == 0))
        return;

    struct device_cfg_clk *cfg = clks->cfg;
    for (i = 0; i < clks->clks_cnt; i++) {
        LTRACEF("Configuring clock %s...\n", clks->name[i]);
        devcfg_set_clock(cfg);
        cfg++;
    }
}

void devcfg_set_pins(struct device_cfg_pins *pins_cfg)
{
    unsigned i;
    LTRACEF("Configuring pins %s\n", pins_cfg->name);
    for (i = 0; i < pins_cfg->pins_cnt; i++)
        devcfg_set_pin(&pins_cfg->pins[i]);
}

int devcfg_set_pins_by_name(struct device_cfg_pins *pins_cfg,
                                            unsigned length, const char *name)
{
    unsigned i;

    if ((!name) || (!pins_cfg))
        return ERR_INVALID_ARGS;

    for (i = 0 ; i < length; i++) {
        if (pins_cfg->name && strcmp(name, pins_cfg->name) == 0) {
            devcfg_set_pins(pins_cfg);
            return 0;
        }
        pins_cfg++;
    }

    return ERR_NOT_FOUND;
}

static const struct device_config_data *console_config;
static int of_console_offset;

static int of_get_console_offset(void)
{
    if (of_console_offset)
        return of_console_offset;

    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    const char * console_path = fdt_get_alias(main_fdt_base, "console");
    if (console_path == NULL)
        return -FDT_ERR_EXISTS;

    LTRACEF("console path %s \n", console_path);
    of_console_offset = fdt_path_offset(main_fdt_base, console_path);

    return of_console_offset;
}

const struct device_config_data * of_get_console_config(void)
{
    if (console_config)
        return console_config;

    if (!main_fdt_base)
        return NULL;

    const char * console_path = fdt_get_alias(main_fdt_base, "console");

    LTRACEF("console path %s \n", console_path);

    int console_offset = fdt_path_offset(main_fdt_base, console_path);
    if (console_offset > 0) {
        struct device * dev = of_find_device_by_node(console_offset);
        if (dev) {
            console_config = dev->config;
        } else {
            console_config = of_device_create_config_data(main_fdt_base, console_offset, 0);
        }
    }

    return console_config;
}

unsigned of_get_console_clocks(struct device_cfg_clk *cfgs, unsigned count)
{
    int lenp;
    unsigned nr_clocks;
    int offset = of_get_console_offset();
    if ((offset < 0) || (!main_fdt_base))
        return 0;

    const fdt32_t *cell = fdt_getprop(main_fdt_base, offset, "clock-cfg", &lenp);
    if (cell == NULL)
        return 0;

    nr_clocks = lenp / sizeof(struct device_cfg_clk);
    if (count > nr_clocks)
        count =  nr_clocks;

    fdt_get_int_array(main_fdt_base,
                    offset,
                    "clock-cfg",
                    (uint32_t*) cfgs,
                    (count * sizeof(struct device_cfg_clk)) / sizeof(uint32_t)
                );

    return count;
}

unsigned of_get_console_pins(struct device_cfg_pin *pins, unsigned count)
{
    int lenp;
    unsigned nr_pins;
    int offset = of_get_console_offset();

    if ((offset < 0) || (!main_fdt_base))
        return 0;

    const fdt32_t *cell = fdt_getprop(main_fdt_base, offset, "pinctrl-0", &lenp);
    if (cell == NULL)
        return 0;

    nr_pins = lenp / sizeof(struct device_cfg_pin);

    if (count > nr_pins)
        count =  nr_pins;

    fdt_get_int_array(main_fdt_base, offset, "pinctrl-0",
                (uint32_t *) pins,
                (count * sizeof(struct device_cfg_pin)) / sizeof(uint32_t)
            );

    return count;
}

void of_get_console_irq_and_base(vaddr_t *vbase, unsigned *irq_number)
{
    *vbase = 0;
    *irq_number = 0;
    const fdt32_t *cell;
    unsigned length;

    int offset = of_get_console_offset();
    if ((offset < 0) || (!main_fdt_base))
        return;

    int addr_cells_sz = fdt_address_cells(main_fdt_base,
                                    fdt_parent_offset(main_fdt_base, offset));
    int size_cells_sz = fdt_size_cells(main_fdt_base,
                                    fdt_parent_offset(main_fdt_base, offset));
    cell = fdt_getprop(main_fdt_base, offset, "reg", NULL);
    if (!cell)
        return;


    *vbase = fdt_read_cell(cell, addr_cells_sz);
    cell += addr_cells_sz;
    length  = fdt_read_cell(cell, size_cells_sz);
    cell += size_cells_sz;

    *vbase = _of_get_io_virtual_base(*vbase, length);

    cell = fdt_getprop(main_fdt_base, offset, "interrupts", NULL);
    if (!cell)
        return;

    *irq_number = fdt_read_cell(cell, 1);

    LTRACEF("Console HW base: %lx - %d\n", *vbase, *irq_number);
}

void of_setup_audio_pll(const void *fdt, int node)
{
    LTRACEF("Configuring %s clock\n", fdt_get_name(fdt, node, NULL));

    struct device_cfg_dpll _of_audio_pll_settings;

    fdt_get_int_array(fdt, node, "settings",
                    (uint32_t *) &_of_audio_pll_settings,
                    sizeof(struct device_cfg_dpll) / sizeof(uint32_t));

    devcfg_set_dpll(&_of_audio_pll_settings);
}

struct _of_setup_clock_s {
    void (*setup)(const void *, int);
    const char *compatible;
} _of_setup_clock_cb[] = {
    { .setup = of_setup_audio_pll, .compatible = "nxp,clocks,audiopll" },
};

static void _of_setup_clocks(const void *fdt, int offset)
{

    LTRACEF("Configuring top clocks node %s\n", fdt_get_name(fdt, offset, NULL));
    int node;
    unsigned i;

    fdt_for_each_subnode(node, fdt, offset) {
        LTRACEF("Looking for driver on %s clock\n", fdt_get_name(fdt, node, NULL));
        for (i = 0; i < ARRAY_SIZE(_of_setup_clock_cb); i++) {
            LTRACEF("Trying compatible %s\n", _of_setup_clock_cb[i].compatible);
            if (fdt_node_check_compatible(fdt, node,
                        _of_setup_clock_cb[i].compatible)== 0) {
                _of_setup_clock_cb[i].setup(fdt, node);
            }
        }
    }
}

void of_setup_clocks(void)
{
    const void *fdt = main_fdt_base;
    int offset = fdt_node_offset_by_compatible(fdt, -1, "nxp,clocks");

    while (offset != -FDT_ERR_NOTFOUND) {
        _of_setup_clocks(fdt, offset);
        offset = fdt_node_offset_by_compatible(fdt, offset, "nxp,clocks");
    }
}

int of_device_get_int32(struct device *dev, const char *prop_name, uint32_t *value)
{
    if (!main_fdt_base)
        return -FDT_ERR_BADMAGIC;

    if ((!prop_name) || (!dev))
        return -FDT_ERR_NOTFOUND;

    const fdt32_t *cell = fdt_getprop(main_fdt_base,
                                dev->node_offset, prop_name, NULL);
    if (cell == NULL)
        return -FDT_ERR_EXISTS;

    *value = fdt_read_cell(cell, 1);
    return 0;
}

int of_device_get_int32_array(struct device *dev, const char *prop_name,
                                                    uint32_t *array, int count)
{
    if (!main_fdt_base)
        return -FDT_ERR_BADMAGIC;

    if ((!prop_name) || (!dev))
        return -FDT_ERR_NOTFOUND;

    return fdt_get_int_array(main_fdt_base, dev->node_offset,
                                                    prop_name, array, count);
}

int of_device_get_bool(struct device *dev, const char *prop_name)
{
    if (!main_fdt_base)
        return -FDT_ERR_BADMAGIC;

    if ((!prop_name) || (!dev))
        return -FDT_ERR_NOTFOUND;

    return fdtdec_get_bool(main_fdt_base, dev->node_offset, prop_name);
}

int of_device_get_strings(struct device *dev, const char *prop_name,
                          const char **name, int count)
{
    int offset = dev->node_offset;
    const void *fdt = main_fdt_base;

    if (fdt == NULL)
        return 0;

    return fdtdec_get_string_list(fdt, offset, prop_name, name, count);
}

int of_device_get_parent_bus_id(struct device *dev, uint32_t *value)
{
    int lenp;
    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    int offset = fdt_parent_offset(main_fdt_base, dev->node_offset);

    const fdt32_t *cell = fdt_getprop(main_fdt_base, offset, "bus-id", &lenp);
    if (cell == NULL)
        return -FDT_ERR_EXISTS;

    *value = fdt_read_cell(cell, 1);
    return 0;
}

struct device * of_device_lookup_device(struct device *dev,
                                                        const char *prop_name)
{
    int offset  = dev->node_offset;
    const void *fdt = main_fdt_base;

    if (fdt == NULL)
        return NULL;

    const fdt32_t *phandle = fdt_getprop(fdt, offset, prop_name, NULL);

    if (phandle == NULL)
        return NULL;

    int node = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*phandle));
    if (!node)
        return NULL;

    return of_find_device_by_node(node);
}

struct device * of_get_device_by_class_and_id(const char *name, int bus_id)
{
    struct device *dev;
    const struct device_config_data * config;

    LTRACEF("Looking for device class %s[%d]\n", name, bus_id);
    list_for_every_entry(&of_device_list, dev, struct device, node) {
        LTRACEF("Trying device %s...\n", dev->name);
        if ((dev->config == NULL) || (dev->driver->ops->device_class == NULL))
            continue;
        config = dev->config;
        const char *class_name = dev->driver->ops->device_class->name;
        LTRACEF("Trying device %s[%s][%d]...\n",
                                        dev->name, class_name, config->bus_id);
        if ((strcmp(class_name, name) == 0) && (config->bus_id == bus_id))
            return dev;
    }

    return NULL;
}

int of_get_memprop(const char *prop_name, uint32_t *array, size_t len)
{
    int offset, err;

    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    offset = fdt_path_offset(main_fdt_base, "/meminfo");
    if (offset < 0)
        return -FDT_ERR_NOTFOUND;

    err = fdt_get_int_array(main_fdt_base, offset,
                            prop_name, array, len);
    if (err != 1)
        return -FDT_ERR_NOTFOUND;

    return 0;
}

int of_get_mempaddr(paddr_t *mempaddr)
{
    uint32_t val;
    int ret;

    ret = of_get_memprop("phys_start", &val, 1);
    *mempaddr = (paddr_t)val;
    return ret;
}

int of_get_meminfo(size_t *memsize)
{
    uint32_t val;
    int ret;

    ret = of_get_memprop("size", &val, 1);
    *memsize = (size_t)val;
    return ret;
}

int of_get_pci_cfg(uintptr_t *cfg)
{
    int len, offset, elems;
    const uint32_t *cell;

    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    offset = fdt_path_offset(main_fdt_base, "/ivshmem");
    if (offset < 0)
        return -FDT_ERR_NOTFOUND;

    cell = fdt_getprop(main_fdt_base, offset, "pci_cfg", &len);
    if (!cell)
        return -FDT_ERR_NOTFOUND;

    /* Use a 32bits address here; Convert to uintptr_t right after */
    elems = len / sizeof(uint32_t);
    if (elems != 2)
        return -FDT_ERR_BADPHANDLE;

    int i;
    for (i = 0; i < 2; i++)
        cfg[i] = (uintptr_t)fdt32_to_cpu(cell[i]);

    return 0;
}

int of_get_node_by_path(const char *path)
{
    int offset;

    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    offset = fdt_path_offset(main_fdt_base, path);
    if (offset < 0)
        return -FDT_ERR_NOTFOUND;

    return offset;
}

int of_get_int_array(int node, const char *prop_name, uint32_t *array, int count)
{
    return fdt_get_int_array(main_fdt_base, node, prop_name, array, count);
}

bool of_get_bool(int node, const char *prop_name)
{
    if (!main_fdt_base)
        return false;

    return !!fdtdec_get_bool(main_fdt_base, node, prop_name);
}

int of_get_strings(int node, const char *prop_name, const char **name, int count)
{
    if (!main_fdt_base)
        return -FDT_ERR_NOTFOUND;

    return fdtdec_get_string_list(main_fdt_base, node, prop_name, name, count);
}

static int appargs_check_fdt(const void *fdt)
{
    int err = fdt_check_header(fdt);

    if (err < 0)
        return 1;

    main_fdt_base = fdt;

    return 0;
}

int appargs_parse_fdt(const void *fdt)
{
    int err = appargs_check_fdt(fdt);
    if (err)
        return 1;

    vaddr_t vbase;
    unsigned irq_number;
    of_get_console_irq_and_base(&vbase, &irq_number);

    of_device_populate(fdt);

#if 0
    /* walk the nodes, looking for 'memory' */
    int depth = 0;
    int offset = 0;
    char node_path[128];

    for (;;) {
        offset = fdt_next_node(fdt, offset, &depth);
        if (offset < 0)
            break;

        /* get the name */
        const char *name = fdt_get_name(fdt, offset, NULL);
        if (!name)
            continue;

        fdt_get_path(fdt, offset, node_path, sizeof(node_path));
        printf("node %s - %s\n", name, node_path);

        /* look for the 'memory' property */
        if (strcmp(name, "memory") == 0) {
            int lenp;
            const void *prop_ptr = fdt_getprop(fdt, offset, "reg", &lenp);
            if (prop_ptr && lenp == 0x10) {
                /* we're looking at a memory descriptor */
                //uint64_t base = fdt64_to_cpu(*(uint64_t *)prop_ptr);
                uint64_t len = fdt64_to_cpu(*((const uint64_t *)prop_ptr + 1));
            }
        }
    }
#endif
    return 0;
}

static void appargs_init(uint level)
{
    int err;
#ifndef APPARGS_FDT_OFFSET
#define APPARGS_FDT_OFFSET 0
#endif
    const void *fdt = (const void *) KERNEL_BASE + APPARGS_FDT_OFFSET;

    if ((level != LK_INIT_LEVEL_PLATFORM_EARLY - 1) && (main_fdt_base == NULL))
        return;

    switch(level) {
        case LK_INIT_LEVEL_PLATFORM_EARLY - 1:
        case LK_INIT_LEVEL_PLATFORM_EARLY:
            err = appargs_check_fdt(fdt);
            if (err) {
                TRACEF("No valid Device Tree Blob found @ %p\n", fdt);
                break;
            }

            TRACEF("Valid Device Tree Blob found @ %p\n", fdt);
            break;
        case LK_INIT_LEVEL_PLATFORM - 2:
            appargs_parse_fdt(fdt);
            break;
        case LK_INIT_LEVEL_PLATFORM - 1:
            of_device_init_all(DRIVER_INIT_CORE);
            break;
        case LK_INIT_LEVEL_PLATFORM:
            of_setup_clocks();
            of_device_init_all(DRIVER_INIT_PLATFORM_EARLY);
            of_device_init_all(DRIVER_INIT_PLATFORM);
            break;
        case LK_INIT_LEVEL_TARGET:
            of_device_init_all(DRIVER_INIT_TARGET);
            of_boards_fixup();
            break;
        case LK_INIT_LEVEL_APPS - 1:
            of_device_init_all(DRIVER_INIT_HAL);
            of_device_init_all(DRIVER_INIT_HAL_VENDOR);
            break;
        case LK_INIT_LEVEL_APPS:
            of_device_init_all(DRIVER_INIT_APP);
            break;
        default:
            TRACEF("Invalid init level %x\n", level);
    }
}

LK_INIT_HOOK(appargs_app, appargs_init, LK_INIT_LEVEL_APPS);
LK_INIT_HOOK(appargs_hal, appargs_init, LK_INIT_LEVEL_APPS - 1);
LK_INIT_HOOK(appargs_target, appargs_init, LK_INIT_LEVEL_TARGET);
LK_INIT_HOOK(appargs_platform, appargs_init, LK_INIT_LEVEL_PLATFORM);
LK_INIT_HOOK(appargs_core, appargs_init, LK_INIT_LEVEL_PLATFORM - 1);
LK_INIT_HOOK(appargs_early, appargs_init, LK_INIT_LEVEL_PLATFORM_EARLY - 1);
LK_INIT_HOOK(appargs, appargs_init, LK_INIT_LEVEL_PLATFORM - 2);
