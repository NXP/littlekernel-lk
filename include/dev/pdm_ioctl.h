/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020 NXP
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

#ifndef	_PDM_IOCTL_H_
#define	_PDM_IOCTL_H_
#include <dev/dev_ioctl.h>
#include <sys/ioctl.h>

struct pdm_ioc_cmd_timestamp {
    bool is_read;
    uint64_t start;
    uint64_t stop;
};
#define PDM_IOC_TS_GET      _IOR(IOCTL_DEV_PDM, 0, struct pdm_ioc_cmd_timestamp)
#define PDM_IOC_TS_SET      _IOW(IOCTL_DEV_PDM, 1, struct pdm_ioc_cmd_timestamp)

struct pdm_ioc_cmd_latency {
    bool is_read;
    uint32_t activation;
    uint32_t warmup;
};
#define PDM_IOC_LATENCY_GET _IOR(IOCTL_DEV_PDM, 2, struct pdm_ioc_cmd_latency)


#endif /*PDM_IOCTL_H_ */
