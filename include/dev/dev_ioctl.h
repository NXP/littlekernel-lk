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

#ifndef	_DEV_IOCTL_H_
#define	_DEV_IOCTL_H_

#define IOCTL_LK    0xC0
#define IOCTL_LK_DEV(x) (IOCTL_LK | (~(IOCTL_LK) & x))

/* Little Kernel major number */
#define IOCTL_DEV_UART      IOCTL_LK_DEV(1)
#define IOCTL_DEV_I2C       IOCTL_LK_DEV(2)
#define IOCTL_DEV_SAI       IOCTL_LK_DEV(3)
#define IOCTL_DEV_SPDIF     IOCTL_LK_DEV(4)
#define IOCTL_DEV_SPI       IOCTL_LK_DEV(5)
#define IOCTL_DEV_GPR       IOCTL_LK_DEV(6)
#define IOCTL_DEV_PDM       IOCTL_LK_DEV(7)

#endif /* DEV_IOCTL_H_ */
