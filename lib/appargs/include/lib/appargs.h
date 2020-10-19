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
#ifndef LIB_APPARGS_H
#define LIB_APPARGS_H

#include <sys/types.h>
#include <list.h>
#include <dev/class/gpio.h> /* struct gpio_desc */

struct board {
    const char *name;
    const char *compatible;
    void (*fixup)(const void*, int);
};

#define BOARD_EXPORT(name_, compat_, fixup_) \
    const struct board __board_ ## name_ \
        __ALIGNED(sizeof(void *)) __SECTION(".boards") = { \
            .name = #name_, \
            .compatible = compat_, \
            .fixup= fixup_, \
        }

#define MAX_PHANDLE_ARGS 16

struct fdt_args {
    int args_count;
    uint32_t args[MAX_PHANDLE_ARGS];
    int node;
};

struct device_cfg_reg {
    paddr_t base;
    vaddr_t vbase;
    size_t length;
    const char *name;
};

struct device_cfg_irq {
    unsigned irq;
    const char *name;
};

struct device_cfg_clk {
    uint32_t root_clk_idx;
    uint32_t root_clk_mux_idx;
    uint32_t pre_divider;
    uint32_t post_divider;
    uint32_t ccgr;
    uint32_t ccgr_root;
    uint32_t rate;
};

struct device_cfg_clks {
    struct device_cfg_clk *cfg;
    unsigned clks_cnt;
    const char **name;
};

#define MAX_CLK_PER_CONFIG      8
#define MAX_PLL_PER_CONFIG      6
#define MAX_GPR_REG             4

struct device_cfg_dpll {
    uint32_t rate;
    uint32_t mdiv;
    uint32_t pdiv;
    uint32_t sdiv;
    uint32_t kdiv;
    uint32_t refSel;
    uint32_t id;
};

struct device_cfg_dplls {
    struct device_cfg_dpll *cfg;
    unsigned plls_cnt;
    const char **name;
};

struct device_cfg_config {
    struct list_node node;
    const char *name;
    /* Clocks */
    struct device_cfg_clk *clk[MAX_CLK_PER_CONFIG];
    const char *clk_name[MAX_CLK_PER_CONFIG];
    unsigned clk_count;
    /* Pll */
    unsigned pll_init_mask;
    /* GPR */
    uint32_t gpr[3 * MAX_GPR_REG];
    /* GPIO */
    struct gpio_desc gpio;
    /* SAI */
    uint32_t clk_mode;
};

struct device_cfg_pin {
    uint32_t muxRegister;
    uint32_t muxMode;
    uint32_t inputRegister;
    uint32_t inputDaisy;
    uint32_t configRegister;
    uint32_t inputOnfield;
    uint32_t configValue;
};

struct device_cfg_pins {
    struct device_cfg_pin *pins;
    unsigned pins_cnt;
    const char *name;
};

struct device_cfg_dma_channel {
    unsigned chan_id;
    const char *name;
    struct fdt_args dma_spec; /* dma arguments */
};

struct device_cfg_dma_device {
    struct device *device;
    struct dma_device *dma_device;
    struct dma_chan *channels; /* TODO: change this to a list_head */
};

struct device_config_data {
    /* BUS id */
    int bus_id;
    /* IO */
    struct device_cfg_reg *io_regions;
    unsigned io_cnt;
    /* IRQ number */
    struct device_cfg_irq *irqs;
    unsigned irqs_cnt;
    /* Pins */
    struct device_cfg_pins *pins_cfg;
    unsigned pins_cfg_cnt;
    /* Clocks */
    struct device_cfg_clks *clks;
    unsigned clks_cnt;
    /* PLLs */
    struct device_cfg_dplls *plls;
    unsigned plls_cnt;
    /* client DMA channels */
    struct device_cfg_dma_channel *dma_channels_cfg;
    unsigned dma_channels_cnt;
    /* DMA controller instance */
    struct device_cfg_dma_device *dma_device_cfg;

    /* Per driver private area */
    char private[];
};


extern const void *main_fdt_base;
int fdt_get_args_from_phandle(const void *, int, const char *,
                            const char *, int, struct fdt_args *);

int of_get_args_from_phandle(int, const char *, const char *, int,
                                                    struct fdt_args *);

int fdtdec_get_string_list(const void *, int, const char *, const char**, int);
int fdt_get_int_array(const void *, int, const char *, uint32_t *, int);

struct device;

status_t of_device_init_all(unsigned initlvl);
const struct device_config_data * of_get_console_config(void);
int of_device_get_int32(struct device *, const char *, uint32_t *);
int of_device_get_int32_array(struct device *, const char *, uint32_t *, int);
int of_device_get_bool(struct device *, const char *);
int of_device_get_strings(struct device *dev, const char *prop_name,
                          const char **name, int count);
int of_device_get_parent_bus_id(struct device *, uint32_t *);
struct device * of_find_device_by_node(int);
struct device * of_device_lookup_device(struct device *, const char *);
struct device * of_get_device_by_class_and_id(const char *, int);
unsigned of_get_console_clocks(struct device_cfg_clk *, unsigned);
unsigned of_get_console_pins(struct device_cfg_pin *, unsigned);
int of_device_get_args_from_phandle(struct device *dev,
                   const char *list_name,
                   const char *cells_name,
                   int index,
                   struct fdt_args *out_args);
struct device_cfg_reg * device_config_get_register_by_name(
                        const struct device_config_data *, const char *);
struct device_cfg_irq * device_config_get_irq_by_name(
                        const struct device_config_data *, const char *);
struct device_cfg_clk * device_config_get_clk_by_name(
                        const struct device_config_data *, const char *);
struct device_cfg_dpll * device_config_get_pll_by_name(
                        const struct device_config_data *, const char *);
struct device_cfg_config *of_device_get_config_by_name(
                        struct device *, const char *);
struct device_cfg_dplls *of_device_get_plls(struct device *);
void of_get_console_irq_and_base(vaddr_t *, unsigned *);
int of_get_mempaddr(paddr_t *);
int of_get_meminfo(size_t *);
int of_get_memprop(const char *prop_name, uint32_t *array, size_t len);
int of_get_pci_cfg(uintptr_t *);
int of_get_node_by_path(const char *);
int of_get_int_array(int, const char *, uint32_t *, int);
bool of_get_bool(int, const char *);
int of_get_strings(int, const char *, const char **, int);

int appargs_parse_fdt(const void *);

void devcfg_set_clock(struct device_cfg_clk *);
void devcfg_set_clocks(struct device_cfg_clks *);
void devcfg_set_pin(struct device_cfg_pin *);
void devcfg_set_pins(struct device_cfg_pins *);
int devcfg_set_pins_by_name(struct device_cfg_pins *, unsigned, const char *);
void of_setup_clocks(void);
status_t of_boards_fixup(void);

void devcfg_set_dpll(struct device_cfg_dpll *);

paddr_t phys_to_dma(paddr_t phys);
struct device_cfg_dma_channel * device_config_get_dma_channel_by_name(
                        const struct device_config_data *, const char *);

#endif /* LIB_APPARGS_H */
