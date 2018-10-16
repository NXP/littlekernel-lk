/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef	_SAI_IOCTL_H_
#define	_SAI_IOCTL_H_
#include <dev/dev_ioctl.h>
#include <sys/ioctl.h>

#define SAI_IOC_CNT_EN_IS_READ      (1UL << 31)
#define SAI_IOC_CNT_EN_MODE_SPDIF   (1UL << 30)
#define SAI_IOC_CNT_EN_ENABLE       (1UL << 0)

#define SAI_IOC_CNT_EN      _IOW(IOCTL_DEV_SAI, 0, int *)

struct sai_ioc_cmd_cnt {
    bool is_read;
    uint32_t bitc;
    uint32_t ts;
    uint64_t cpu_ts;
};
#define SAI_IOC_CNT_GET     _IOR(IOCTL_DEV_SAI, 1, struct sai_ioc_cmd_cnt)
#define SAI_IOC_CLK_MODE    _IOW(IOCTL_DEV_SAI, 2, int *)

struct sai_ioc_cmd_timestamp {
    bool is_read;
    uint64_t start;
    uint64_t stop;
};
#define SAI_IOC_TS_GET      _IOR(IOCTL_DEV_SAI, 3, struct sai_ioc_cmd_timestamp)
#define SAI_IOC_TS_SET      _IOW(IOCTL_DEV_SAI, 4, struct sai_ioc_cmd_timestamp)

struct sai_ioc_cmd_latency {
    bool is_read;
    uint32_t activation;
    uint32_t warmup;
};
#define SAI_IOC_LATENCY_GET _IOR(IOCTL_DEV_SAI, 8, struct sai_ioc_cmd_latency)

struct sai_ioc_cmd_pll_k {
    uint32_t sampling_rate;
    uint32_t pll_k_value;
};
#define SAI_IOC_PLL_K_SET   _IOW(IOCTL_DEV_SAI, 9, unsigned *)

#define SAI_IOC_RCE_SET     _IOW(IOCTL_DEV_SAI, 10, uint8_t)

struct sai_ioc_cmd_buffer_sz {
    bool is_read;
    size_t sz;
};
#define SAI_IOC_BUF_SZ_GET  _IOR(IOCTL_DEV_SAI, 11, struct sai_ioc_cmd_buffer_sz)

struct sai_ioc_cmd_space_used {
    bool is_read;
    size_t used;
};

#define SAI_IOC_SZ_USED_GET _IOR(IOCTL_DEV_SAI, 12, struct sai_ioc_cmd_space_used)

#endif /*SAI_IOCTL_H_ */
