/*
 * Copyright 2019-2020 NXP
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
#ifndef __DEV_CLASS_SPDIF_H
#define __DEV_CLASS_SPDIF_H

#include <compiler.h>
#include <dev/driver.h>

typedef enum spdif_pkt_format_e {
    SPDIF_FMT_IEC60958 = 0,
    SPDIF_FMT_IEC60958_OUTBAND,
} spdif_pkt_format_t;

typedef struct spdif_hw_params_s {
    spdif_pkt_format_t fmt;
    uint32_t bitWidth;        /*!< Data length of audio data, usually 8/16/24/32 bits */
    uint32_t period_size;      /*!< period size */
} spdif_hw_params_t;

/**
 * @typedef spdif_cb_evt_t
 *   Asynchronous callback events, either stream changes events or
 * completion of non blocking calls. Mutually exclusive */
typedef enum _spdif_cb_evt {
    SPDIF_EVENT_AUDIO_SAMPLE_RATE = 0,  /*!< Audio sampling rate changed */
    SPDIF_EVENT_AUDIO_LINK,             /*!< Audio physical connection changed */
    SPDIF_EVENT_AUDIO_CHANNEL_STATUS,   /*!< Audio channel status packet received */
    SPDIF_EVENT_AUDIO_CLK_LOST,         /*!< Audio recovery clock lost */
    SPDIF_EVENT_ERROR,                  /*!< Hits some error(s) */
    SPDIF_EVENT_MAX,                    /*!< Number of events */
} spdif_cb_evt_t;

/**
 * @typedef spdif_cb_t
 *   Asynchronous callback previously registered through set_callback.
 *
 * @param[in]   event   Event type
 * @param[in]   param   Optional parameter. Semantic depends on event type
 * @param[in]   cookie  Callee cookie, set while calling set_callback
 * @return  The callback status error.
 */

typedef int (*spdif_cb_t)(spdif_cb_evt_t , void *, void *);

/* spdif interface */
struct spdif_ops {
    struct driver_ops std;
    /* TX */
    status_t (*tx_open)(struct device *dev);
    status_t (*tx_close)(struct device *dev);
    status_t (*tx_start)(struct device *dev);
    status_t (*tx_stop)(struct device *dev);
    status_t (*tx_setup)(struct device *dev, spdif_hw_params_t *pfmt);
    status_t (*tx_flush)(struct device *dev);
    status_t (*write)(struct device *dev, const void *buf, size_t len);
    /**
     * @brief Set the callback function for notifying asynchronous changes
     *
     * @param[in] device SPDIF device
     * @param[in] cb Callback function to register
     * @param[in] cookie Private callback data
     *
     * @return O in case of success, a negative value otherwise.
     */

    status_t (*set_callback)(struct device *device, spdif_cb_t cb, void *cookie);

    /* RX */
    status_t (*rx_open)(struct device *dev);
    status_t (*rx_close)(struct device *dev);
    status_t (*rx_start)(struct device *dev);
    status_t (*rx_stop)(struct device *dev);
    status_t (*rx_setup)(struct device *dev, spdif_hw_params_t *pfmt);
    status_t (*rx_flush)(struct device *dev);
    status_t (*read)(struct device *dev, void *buf, size_t len);
};

__BEGIN_CDECLS

status_t class_spdif_open(struct device *dev, bool is_read);
status_t class_spdif_close(struct device *dev, bool is_read);
status_t class_spdif_start(struct device *dev, bool is_read);
status_t class_spdif_stop(struct device *dev, bool is_read);
status_t class_spdif_flush(struct device *dev, bool is_read);
status_t class_spdif_setup(struct device *dev, bool is_read, spdif_hw_params_t *pfmt);
status_t class_spdif_write(struct device *dev, const void *buf, size_t len);
status_t class_spdif_read(struct device *dev, void *buf, size_t len);
status_t class_spdif_set_callback(struct device *dev, spdif_cb_t cb, void *cookie);
struct device * class_spdif_get_device_by_id(int bus_id);

__END_CDECLS

#endif /* __DEV_CLASS_SPDIF_H */
