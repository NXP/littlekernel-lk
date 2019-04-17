/*
 * Copyright 2019 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __DEV_CLASS_PDM_H
#define __DEV_CLASS_PDM_H

#include <compiler.h>
#include <dev/driver.h>

typedef struct pdm_hw_params_s {
    uint32_t sampleRate_Hz;   /*!< Sample rate of audio data */
    uint32_t bitWidth;        /*!< Data length of audio data, usually 8/16/24/32 bits */
    uint32_t num_channels;    /*!< number of mic channels */
    uint32_t period_size;      /*!< period size */
    uint32_t start_channel;    /*!< Start channel from mics */
} pdm_hw_params_t;

/* pdm interface */
struct pdm_ops {
    struct driver_ops std;
    /* TX */
    status_t (*tx_open)(struct device *dev);
    status_t (*tx_close)(struct device *dev);
    status_t (*tx_start)(struct device *dev);
    status_t (*tx_stop)(struct device *dev);
    status_t (*tx_setup)(struct device *dev, pdm_hw_params_t *pfmt);
    status_t (*write)(struct device *dev, const void *buf, size_t len);

    /* RX */
    status_t (*rx_open)(struct device *dev);
    status_t (*rx_close)(struct device *dev);
    status_t (*rx_start)(struct device *dev);
    status_t (*rx_stop)(struct device *dev);
    status_t (*rx_setup)(struct device *dev, pdm_hw_params_t *pfmt);
    status_t (*read)(struct device *dev, void *buf, size_t len);
};

__BEGIN_CDECLS

status_t class_pdm_open(struct device *dev, bool is_read);
status_t class_pdm_close(struct device *dev, bool is_read);
status_t class_pdm_start(struct device *dev, bool is_read);
status_t class_pdm_stop(struct device *dev, bool is_read);
status_t class_pdm_setup(struct device *dev, bool is_read, pdm_hw_params_t *pfmt);
status_t class_pdm_write(struct device *dev, const void *buf, size_t len);
status_t class_pdm_read(struct device *dev, void *buf, size_t len);
struct device * class_pdm_get_device_by_id(int bus_id);

__END_CDECLS

#endif /* __DEV_CLASS_PDM_H */
