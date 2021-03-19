// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <err.h>
#include <pdev/interrupt.h>
#include <lk/init.h>
#include <sys/types.h>

#define ARM_MAX_INT 1024
#define PDEV_MAX_CHAINED_INT 256

static spin_lock_t lock = SPIN_LOCK_INITIAL_VALUE;

static struct int_handler_struct int_handler_table[ARM_MAX_INT
        + PDEV_MAX_CHAINED_INT];

static struct int_handler_struct *pdev_get_free_chained_int_handler(void);

struct int_handler_struct *pdev_get_int_handler(unsigned int vector)
{
    DEBUG_ASSERT(vector < ARM_MAX_INT);
    return &int_handler_table[vector];
}

status_t register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
    if (!is_valid_interrupt(vector, 0)) {
        return ERR_INVALID_ARGS;
    }

    struct int_handler_struct *h;

    spin_lock_saved_state_t state;
    spin_lock_save(&lock, &state, SPIN_LOCK_FLAG_INTERRUPTS);

    h = pdev_get_int_handler(vector);
    if (handler && h->handler) {
        /* Get to last chained handler, if any */
        while (h->next)
            h = (struct int_handler_struct*) h->next;

        /* Allocate spare chained interrupt handler from pool */
        h->next = (void *) pdev_get_free_chained_int_handler();
        if (!h->next) {
            spin_unlock_restore(&lock, state, SPIN_LOCK_FLAG_INTERRUPTS);
            return ERR_ALREADY_BOUND;
        }

        h = (struct int_handler_struct *) h->next;
    }
    h->handler = handler;
    h->arg = arg;

    spin_unlock_restore(&lock, state, SPIN_LOCK_FLAG_INTERRUPTS);
    return NO_ERROR;
}

static struct int_handler_struct *pdev_get_free_chained_int_handler(void)
{
    for (unsigned i = 0; i < PDEV_MAX_CHAINED_INT; i++) {
        if (int_handler_table[ARM_MAX_INT + i].handler == NULL)
            return &int_handler_table[ARM_MAX_INT + i];
    }

    return NULL;
}

static status_t default_mask(unsigned int vector)
{
    return ERR_NOT_CONFIGURED;
}

static status_t default_unmask(unsigned int vector)
{
    return ERR_NOT_CONFIGURED;
}

static status_t default_configure(unsigned int vector,
                                  enum interrupt_trigger_mode tm,
                                  enum interrupt_polarity pol)
{
    return ERR_NOT_CONFIGURED;
}

static status_t default_get_config(unsigned int vector,
                                   enum interrupt_trigger_mode *tm,
                                   enum interrupt_polarity *pol)
{
    return ERR_NOT_CONFIGURED;
}

static bool default_is_valid(unsigned int vector, uint32_t flags)
{
    return false;
}
static unsigned int default_remap(unsigned int vector)
{
    return 0;
}

static status_t default_send_ipi(cpu_mask_t target, mp_ipi_t ipi)
{
    return ERR_NOT_CONFIGURED;
}

static void default_init_percpu_early(void)
{
}

static void default_init_percpu(void)
{
}

static enum handler_return default_handle_irq(iframe *frame)
{
    return INT_NO_RESCHEDULE;
}

static void default_handle_fiq(iframe *frame)
{
}

static void default_shutdown(void)
{
}

static void default_shutdown_cpu(void)
{
}

static const struct pdev_interrupt_ops default_ops = {
    .mask = default_mask,
    .unmask = default_unmask,
    .configure = default_configure,
    .get_config = default_get_config,
    .is_valid = default_is_valid,
    .remap = default_remap,
    .send_ipi = default_send_ipi,
    .init_percpu_early = default_init_percpu_early,
    .init_percpu = default_init_percpu,
    .handle_irq = default_handle_irq,
    .handle_fiq = default_handle_fiq,
    .shutdown = default_shutdown,
    .shutdown_cpu = default_shutdown_cpu,
};

static const struct pdev_interrupt_ops *intr_ops = &default_ops;

status_t mask_interrupt(unsigned int vector)
{
    return intr_ops->mask(vector);
}

status_t unmask_interrupt(unsigned int vector)
{
    return intr_ops->unmask(vector);
}

status_t configure_interrupt(unsigned int vector, enum interrupt_trigger_mode tm,
                             enum interrupt_polarity pol)
{
    return intr_ops->configure(vector, tm, pol);
}

status_t get_interrupt_config(unsigned int vector, enum interrupt_trigger_mode *tm,
                              enum interrupt_polarity *pol)
{
    return intr_ops->get_config(vector, tm, pol);
}

uint32_t interrupt_get_base_vector(void)
{
    return intr_ops->get_base_vector();
}

uint32_t interrupt_get_max_vector(void)
{
    return intr_ops->get_max_vector();
}

bool is_valid_interrupt(unsigned int vector, uint32_t flags)
{
    return intr_ops->is_valid(vector, flags);
}

unsigned int remap_interrupt(unsigned int vector)
{
    return intr_ops->remap(vector);
}

status_t interrupt_send_ipi(cpu_mask_t target, mp_ipi_t ipi)
{
    return intr_ops->send_ipi(target, ipi);
}

void interrupt_init_percpu(void)
{
    intr_ops->init_percpu();
}

enum handler_return platform_irq(iframe *frame)
{
    return intr_ops->handle_irq(frame);
}

void platform_fiq(iframe *frame)
{
    intr_ops->handle_fiq(frame);
}

void pdev_register_interrupts(const struct pdev_interrupt_ops *ops)
{
    intr_ops = ops;
    smp_mb();
}

static void interrupt_init_percpu_early(uint level)
{
    intr_ops->init_percpu_early();
}

void shutdown_interrupts(void)
{
    intr_ops->shutdown();
}

void shutdown_interrupts_curr_cpu(void)
{
    intr_ops->shutdown_cpu();
}

LK_INIT_HOOK_FLAGS(interrupt_init_percpu_early, interrupt_init_percpu_early, LK_INIT_LEVEL_PLATFORM_EARLY, LK_INIT_FLAG_SECONDARY_CPUS);
