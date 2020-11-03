/*
 * Copyright 2020-2021 NXP
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

#ifndef __DEV_CLASS_ASRC_H
#define __DEV_CLASS_ASRC_H

#include <compiler.h>
#include <dev/driver.h>

/*! @brief data type */
typedef enum _asrc_audio_data_type {
    ASRC_DataTypeInteger = 0U,  /*!< data type int */
    ASRC_DataTypeFloat          /*!< data type float, single precision floating point format */
} asrc_audio_data_type_t;

/*! @brief audio parameters */
typedef struct asrc_audio_params_s {
    uint32_t                num_channels;
    uint32_t                in_sample_rate;
    uint32_t                out_sample_rate;
    asrc_audio_data_type_t  data_type;
} asrc_audio_params_t;

/*! @brief transfer parameters */
typedef struct asrc_transfer_params_s {
    int32_t                 *in_buffer;
    int32_t                 *out_buffer;
    uint32_t                in_size_bytes;
} asrc_transfer_params_t;

/*! @brief flush parameters */
typedef struct asrc_flush_params_s {
    int32_t                 *out_buffer;
} asrc_flush_params_t;

/*! @brief output parameters */
typedef struct asrc_out_params_s {
    uint32_t                out_size_bytes;
} asrc_out_params_t;

/* asrc interface */
struct asrc_ops {
    struct driver_ops std;
    /**
     * @brief Open the device
     *
     * @param[in] device ASRC device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*open)(struct device *dev);
    /**
     * @brief Close the device
     *
     * @param[in] device ASRC device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*close)(struct device *dev);
   /**
     * @brief Set the device
     *
     * @param[in] device ASRC device
     * @param[in] audio_params ASRC audio parameters
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if format not supported by ASRC device
     */
    status_t (*setup)(struct device *device, asrc_audio_params_t *audio_params);
    /**
      * @brief Start the device
      *
      * @param[in] device ASRC device
      * @param[in] transfer_params ASRC buffer parameters
      *
      * @return O in case of success, a negative value otherwise.
      */
    status_t (*start)(struct device *device, asrc_transfer_params_t *transfer_params, asrc_out_params_t *out_params);
    /**
     * @brief Flush the device (optional)
     *
     * @param[in] device ASRC device
     * @param[in] flush_params ASRC buffer parameters
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*flush)(struct device *device, asrc_flush_params_t *flush_params, asrc_out_params_t *out_params);
    /**
     * @brief Stop the device
     *
     * @param[in] device ASRC device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*stop)(struct device *dev);
};

__BEGIN_CDECLS
status_t class_asrc_open(struct device *dev);
status_t class_asrc_close(struct device *dev);
status_t class_asrc_setup(struct device *dev, asrc_audio_params_t *audio_params);
status_t class_asrc_start(struct device *dev, asrc_transfer_params_t *transfer_params, asrc_out_params_t *out_params);
status_t class_asrc_flush(struct device *dev, asrc_flush_params_t *pflush, asrc_out_params_t *out_params);
status_t class_asrc_stop(struct device *dev);
struct device *class_asrc_get_device_by_id(int id);
__END_CDECLS

#endif /* __DEV_CLASS_ASRC_H */
