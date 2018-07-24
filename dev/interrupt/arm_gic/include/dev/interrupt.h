// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <kernel/mp.h>
#include <stdbool.h>
#include <sys/types.h>
#include <compiler.h>

__BEGIN_CDECLS

#define GIC_BASE_SGI 0
#define GIC_BASE_PPI 16
#define GIC_BASE_SPI 32

enum {
    GICV2 = 2,
    GICV3 = 3,
    GICV4 = 4,
};

enum {
    /* Ignore cpu_mask and forward interrupt to all CPUs other than the current cpu */
    ARM_GIC_SGI_FLAG_TARGET_FILTER_NOT_SENDER = 0x1,
    /* Ignore cpu_mask and forward interrupt to current CPU only */
    ARM_GIC_SGI_FLAG_TARGET_FILTER_SENDER = 0x2,
    ARM_GIC_SGI_FLAG_TARGET_FILTER_MASK = 0x3,

    /* Only forward the interrupt to CPUs that has the interrupt configured as group 1 (non-secure) */
    ARM_GIC_SGI_FLAG_NS = 0x4,
};

enum interrupt_trigger_mode {
    IRQ_TRIGGER_MODE_EDGE = 0,
    IRQ_TRIGGER_MODE_LEVEL = 1,
};

enum interrupt_polarity {
    IRQ_POLARITY_ACTIVE_HIGH = 0,
    IRQ_POLARITY_ACTIVE_LOW = 1,
};

status_t mask_interrupt(unsigned int vector);
status_t unmask_interrupt(unsigned int vector);

void shutdown_interrupts(void);

// Shutdown interrupts for the calling CPU.
//
// Should be called before powering off the calling CPU.
void shutdown_interrupts_curr_cpu(void);

// Configure the specified interrupt vector.  If it is invoked, it muust be
// invoked prior to interrupt registration
status_t configure_interrupt(unsigned int vector,
                             enum interrupt_trigger_mode tm,
                             enum interrupt_polarity pol);

status_t get_interrupt_config(unsigned int vector,
                              enum interrupt_trigger_mode *tm,
                              enum interrupt_polarity *pol);

typedef enum handler_return (*int_handler)(void *arg);

status_t register_int_handler(unsigned int vector, int_handler handler, void *arg);

// These return the [base, max] range of vectors that can be used with zx_interrupt syscalls
// This api will need to evolve if valid vector ranges later are not contiguous
uint32_t interrupt_get_base_vector(void);
uint32_t interrupt_get_max_vector(void);

bool is_valid_interrupt(unsigned int vector, uint32_t flags);

unsigned int remap_interrupt(unsigned int vector);

/* sends an inter-processor interrupt */
status_t interrupt_send_ipi(cpu_mask_t target, mp_ipi_t ipi);

/* performs per-cpu initialization for the interrupt controller */
void interrupt_init_percpu(void);

__END_CDECLS
