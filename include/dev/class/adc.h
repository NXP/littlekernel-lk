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
#ifndef __DEV_CLASS_ADC_H
#define __DEV_CLASS_ADC_H

#include <compiler.h>
#include <dev/driver.h>


#define ADC_CAP_PKT_PCM (1UL << 0)
#define ADC_CAP_PKT_DSD (1UL << 1)
#define ADC_CAP_RAW_PDM (1UL << 2)

/**
 * @typedef hdmi_audio_pkt_t
 *   Audio packet type.
 */
typedef enum _adc_audio_pkt_e {
    /*! PCM audio packets */
    ADC_AUDIO_PKT_PCM,
    /*! DSD audio packets */
    ADC_AUDIO_PKT_DSD,
} adc_audio_pkt_t;

typedef enum _adc_audio_fmt_e {
    ADC_AUDIO_FMT_I2S,
    ADC_AUDIO_FMT_LEFT_J,
    ADC_AUDIO_FMT_RIGHT_J,
    ADC_AUDIO_FMT_DSP_A,
    ADC_AUDIO_FMT_DSP_B,
    ADC_AUDIO_FMT_AC97,
} adc_audio_fmt_t;

typedef enum _adc_audio_pcm_format_e {
    ADC_AUDIO_PCM_FMT_16,
    ADC_AUDIO_PCM_FMT_24,
    ADC_AUDIO_PCM_FMT_32,
} adc_audio_pcm_format_t;

typedef struct adc_audio_hw_params_s {
    adc_audio_pcm_format_t pcm_fmt;
    adc_audio_fmt_t fmt;
    adc_audio_pkt_t pkt;
    unsigned num_ch;
    unsigned rate;
} adc_audio_hw_params_t;

/* adc interface */
struct adc_ops {
    struct driver_ops std;
    /**
     * @brief Open the device
     *
     * @param[in] device ADC device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*open)(struct device *dev);
    /**
     * @brief Close the device
     *
     * @param[in] device ADC device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*close)(struct device *dev);
    /**
     * \brief Get ADC switch capabilities
     *
     * @param[in] device ADC device
     * @param[out] capabilities Mask exporting ADC device capabilities
     *
     * \return A mask holding the capabilities of the ADC switch/repeater
     */
    status_t (*get_capabilities)(const struct device * dev, uint32_t *capabilities);
   /**
     * @brief Set the audio physical transport format
     *
     * @param[in] device ADC device.
     * @param[in] hw_params ADC audio hardware parameters.
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if format not supported by ADC device
     */
    status_t (*set_format)(struct device *device, adc_audio_hw_params_t *hw_params);

    status_t (*reset_power)(struct device *device);
};

__BEGIN_CDECLS
status_t class_adc_open(struct device *dev);
status_t class_adc_close(struct device *dev);
status_t class_adc_get_capabilities(const struct device * dev, uint32_t *capabilities);
status_t class_adc_set_format(struct device *device, adc_audio_hw_params_t *params);
status_t class_adc_reset_power(struct device *device);

struct device * class_adc_get_device_by_id(int bus_id);

__END_CDECLS

#endif /* __DEV_CLASS_ADC_H */
