/*
 * Copyright 2018-2020 NXP
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
#ifndef __DEV_CLASS_SAI_H
#define __DEV_CLASS_SAI_H

#include <compiler.h>
#include <dev/driver.h>

#define CLASS_SAI_RX true
#define CLASS_SAI_TX false

/**
 * @typedef sai_cb_evt_t
 *   Asynchronous callback events, either stream changes events or
 * completion of non blocking calls. Mutually exclusive */
typedef enum _sai_cb_evt {
    SAI_EVENT_PERIOD_ELAPSED = 0,   /*!< Period elapsed */
    SAI_EVENT_DRAIN,                /*!< Drain completed */
    SAI_EVENT_ACTIVE,               /*!< Hardware is active */
    SAI_EVENT_ERROR,                /*!< Hits some error(s) */
    SAI_EVENT_ERATE_CHANGE,         /*!< Estimated rate change detected */
    SAI_EVENT_SILENCE,              /*!< Silence injection */
    SAI_EVENT_MAX,                  /*!< Number of events */
} sai_cb_evt_t;

typedef struct sai_evt_period_elasped_s {
    uint64_t cnt;
} sai_evt_period_elapsed_t;

typedef struct sai_evt_silence_s {
    uint64_t cnt;
} sai_evt_silence_t;

typedef int (*sai_cb_t)(sai_cb_evt_t , void *, void *);

typedef enum _sai_bit_clock_polarity {
    SAI_BITCLOCK_POLARITY_ACTIVE_HIGH = 0,
    SAI_BITCLOCK_POLARITY_ACTIVE_LOW,
    SAI_BITCLOCK_POLARITY_UNUSED,
} sai_bitclock_polarity_t;
/*! @brief Master or slave mode */
typedef enum sai_master_slave {
    ksai_Slave = 0x0U, /*!< Slave mode */
    ksai_Master = 0x1U /*!< Master mode */
} sai_slave_master_t;

/* Duplicated from sai_hw.h */
/*! @brief Bit clock source */
typedef enum _sai_bclk {
    kSAI_BClkSourceBusclk = 0x0U, /*!< Bit clock using bus clock */
    kSAI_BClkSourceMclkDiv,       /*!< Bit clock using master clock divider */
    kSAI_BClkSourceOtherSai0,     /*!< Bit clock from other SAI device  */
    kSAI_BClkSourceOtherSai1      /*!< Bit clock from other SAI device */
} sai_bclk_t;

/* Duplicated from sai_hw.h */
/*! @brief Define the SAI bus type */
typedef enum _sai_fmt {
    kSAI_FmtLeftJustified = 0x0U, /*!< Uses left justified format.*/
    kSAI_FmtRightJustified,       /*!< Uses right justified format. */
    kSAI_FmtI2S,                  /*!< Uses I2S format. */
    kSAI_FmtPCMA,                 /*!< Uses I2S PCM A format.*/
    kSAI_FmtPCMB,                 /*!< Uses I2S PCM B format. */
    kSAI_FmtI2SAes3,              /*!< Uses I2S AES3 format. */
    kSAI_FmtI2SPDM,
    kSAI_FmtMax
} sai_fmt_t;

typedef struct sai_format_s {
    uint8_t  sai_protocol;
    uint32_t sampleRate_Hz;   /*!< Sample rate of audio data */
    uint32_t bitWidth;        /*!< Data length of audio data, usually 8/16/24/32 bits */
    uint32_t num_channels;    /*!< number of audio channels */
    uint32_t num_slots;       /*!< number of audio data slots (typically used for TDM) */
    uint32_t period_size;     /*!< period size */
    sai_bitclock_polarity_t polarity; /*!< Force polarity */
    sai_slave_master_t master_slave;  /*!< Master or Slave mode*/
    sai_bclk_t  bitclock_source; /*!< BitClock*/
} sai_format_t;

/* sai interface */
struct sai_ops {
    struct driver_ops std;
    /* TX */
    status_t (*tx_open)(struct device *dev);
    status_t (*tx_close)(struct device *dev);
    status_t (*tx_start)(struct device *dev);
    status_t (*tx_stop)(struct device *dev);
    status_t (*tx_flush)(struct device *dev);
    status_t (*tx_setup)(struct device *dev, sai_format_t *pfmt);
    status_t (*tx_get_cnt)(struct device *dev, uint32_t *bitc, uint32_t *bitc_ts, uint64_t *cpu_ts);
    status_t (*tx_en_cnt)(struct device *dev, bool en);
    status_t (*write)(struct device *dev, const void *buf, size_t len);
    status_t (*tx_set_callback)(struct device *device, sai_cb_t cb, void *cookie);
    /* RX */
    status_t (*rx_open)(struct device *dev);
    status_t (*rx_close)(struct device *dev);
    status_t (*rx_start)(struct device *dev);
    status_t (*rx_stop)(struct device *dev);
    status_t (*rx_flush)(struct device *dev);
    status_t (*rx_setup)(struct device *dev, sai_format_t *pfmt);
    status_t (*rx_get_cnt)(struct device *dev, uint32_t *bitc, uint32_t *bitc_ts, uint64_t *cpu_ts);
    status_t (*rx_en_cnt)(struct device *dev, bool en);
    status_t (*read)(struct device *dev, void *buf, size_t len);
    status_t (*rx_set_callback)(struct device *device, sai_cb_t cb, void *cookie);
};

__BEGIN_CDECLS

status_t class_sai_open(struct device *dev, bool is_read);
status_t class_sai_close(struct device *dev, bool is_read);
status_t class_sai_start(struct device *dev, bool is_read);
status_t class_sai_stop(struct device *dev, bool is_read);
status_t class_sai_flush(struct device *dev, bool is_read);
status_t class_sai_setup(struct device *dev, bool is_read, sai_format_t *pfmt);
status_t class_sai_write(struct device *dev, const void *buf, size_t len);
status_t class_sai_read(struct device *dev, void *buf, size_t len);
struct device * class_sai_get_device_by_id(int bus_id);
status_t class_sai_get_counters(struct device *dev,
                uint32_t *bitc, uint32_t *ts, uint64_t *cpu_ts, bool is_read);
status_t class_sai_en_counters(struct device *dev, bool en, bool is_read);
status_t class_sai_set_callback(struct device *dev, sai_cb_t cb,
                                                void *cookie, bool is_read);
__END_CDECLS

#endif /* __DEV_CLASS_SAI_H */
