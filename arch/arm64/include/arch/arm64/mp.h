// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <arch/arm64.h>
//#include <kernel/align.h>
#include <kernel/cpu.h>
#include <reg.h>
#include <compiler.h>

__BEGIN_CDECLS

// bits for mpidr register
#define MPIDR_AFF0_MASK 0xFFULL
#define MPIDR_AFF0_SHIFT 0
#define MPIDR_AFF1_MASK (0xFFULL << 8)
#define MPIDR_AFF1_SHIFT 8
#define MPIDR_AFF2_MASK (0xFFULL << 16)
#define MPIDR_AFF2_SHIFT 16
#define MPIDR_AFF3_MASK (0xFFULL << 32)
#define MPIDR_AFF3_SHIFT 32

// construct a ARM MPID from cluster (AFF1) and cpu number (AFF0)
#define ARM64_MPID(cluster, cpu) (((cluster << MPIDR_AFF1_SHIFT) & MPIDR_AFF1_MASK) | \
                                  ((cpu << MPIDR_AFF0_SHIFT) & MPIDR_AFF0_MASK))
#ifdef NOT_YET
// TODO: add support for AFF2 and AFF3

// Per cpu structure, pointed to by x18 while in kernel mode.
// Aligned on the maximum architectural cache line to avoid cache
// line sharing between cpus.
struct arm64_percpu {
    // cpu number
    uint32_t cpu_num;

    // is the cpu currently inside an interrupt handler
    uint32_t in_irq;
} __CPU_ALIGN;

void arch_init_cpu_map(uint cluster_count, const uint* cluster_cpus);
void arm64_init_percpu_early(void);

// Use the x18 register to always point at the local cpu structure to allow fast access
// a per cpu structure.
// Do not directly access fields of this structure
register struct arm64_percpu* __arm64_percpu __asm("x18");

static inline void arm64_write_percpu_ptr(struct arm64_percpu* percpu) {
    __arm64_percpu = percpu;
}

static inline struct arm64_percpu* arm64_read_percpu_ptr(void) {
    return __arm64_percpu;
}

static inline uint32_t arm64_read_percpu_u32(size_t offset) {
    uint32_t val;

    // mark as volatile to force a read of the field to make sure
    // the compiler always emits a read when asked and does not cache
    // a copy between
    __asm__ volatile("ldr %w[val], [x18, %[offset]]"
                     : [val] "=r"(val)
                     : [offset] "Ir"(offset));
    return val;
}

static inline void arm64_write_percpu_u32(size_t offset, uint32_t val) {
    __asm__("str %w[val], [x18, %[offset]]"
            ::[val] "r"(val), [offset] "Ir"(offset)
            : "memory");
}

static inline cpu_num_t arch_curr_cpu_num(void) {
    return arm64_read_percpu_u32(offsetof(struct arm64_percpu, cpu_num));
}

static inline uint arch_max_num_cpus(void) {
    extern uint arm_num_cpus;

    return arm_num_cpus;
}

// translate a cpu number back to the cluster ID (AFF1)
static inline uint arch_cpu_num_to_cluster_id(uint cpu) {
    extern uint arm64_cpu_cluster_ids[SMP_MAX_CPUS];

    return arm64_cpu_cluster_ids[cpu];
}

// translate a cpu number back to the MP cpu number within a cluster (AFF0)
static inline uint arch_cpu_num_to_cpu_id(uint cpu) {
    extern uint arm64_cpu_cpu_ids[SMP_MAX_CPUS];

    return arm64_cpu_cpu_ids[cpu];
}

#define READ_PERCPU_FIELD32(field) \
    arm64_read_percpu_u32(offsetof(struct arm64_percpu, field))

#define WRITE_PERCPU_FIELD32(field, value) \
    arm64_write_percpu_u32(offsetof(struct arm64_percpu, field), (value))
#else

static inline uint arch_max_num_cpus(void)
{
    return SMP_MAX_CPUS;
}

static inline uint arch_cpu_num_to_cluster_id(uint cpuid)
{
    if (cpuid > SMP_MAX_CPUS)
        return 1;
    return 0;
}
static inline uint arch_cpu_num_to_cpu_id(uint cpu_num)
{
    return cpu_num;
}
#endif /* NOT_YET */

__END_CDECLS
