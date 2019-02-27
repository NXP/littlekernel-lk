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
#ifndef __DEV_CLASS_HDMI_H
#define __DEV_CLASS_HDMI_H

#include <compiler.h>
#include <dev/driver.h>
#include <dev/iec60958.h>

/**
 * @typedef hdmi_cb_evt_t
 *   Asynchronous callback events, either stream changes events or
 * completion of non blocking calls. Mutually exclusive */
typedef enum _hdmi_cb_evt {
    HDMI_EVENT_AUDIO_SAMPLE_RATE = 0,   /*!< Audio sampling rate changed */
    HDMI_EVENT_AUDIO_STREAM_TYPE,   /*!< Audio stream type changed */
    HDMI_EVENT_AUDIO_LINK,          /*!< Audio physical connection changed */
    HDMI_EVENT_AUDIO_MCLK,          /*!< Audio mclk status changed */
    HDMI_EVENT_AUDIO_INFOFRAME,     /*!< Audio Infoframe packet received */
    HDMI_EVENT_AUDIO_CHANNEL_STATUS, /*!< Audio channel status packet received */
    HDMI_EVENT_AUDIO_LAYOUT_CHANGE, /*!< Audio channel layout changed */
    HDMI_EVENT_ERROR,         /*!< Hits some error(s) */
    HDMI_EVENT_MAX,         /*!< Number of events */
} hdmi_cb_evt_t;

#define HDMI_CAP_AUDIO_SAMPLE_RATE_CHANGE   (1UL << 0)
#define HDMI_CAP_AUDIO_STREAM_TYPE_CHANGE   (1UL << 2)
#define HDMI_CAP_AUDIO_CHANNEL_STATUS       (1UL << 3)
#define HDMI_CAP_AUDIO_LINK_CHANGE          (1UL << 4)
#define HDMI_CAP_AUDIO_INFOFRAME            (1UL << 5)
#define HDMI_CAP_AUDIO_LAYOUT_CHANGE        (1UL << 6)

#define HDMI_CAP_AUDIO_FMT_60958            (1UL << 24)
#define HDMI_CAP_AUDIO_FMT_61937            (1UL << 25)
#define HDMI_CAP_AUDIO_FMT_CUSTOM           (1UL << 26)

typedef struct hdmi_channel_status_mask_s {
    uint32_t professional:1;
    uint32_t lpcm:1;
    uint32_t copyright:1;
    uint32_t pre_emphasis:1;
    uint32_t mode:1;
    uint32_t category:1;
    uint32_t source:1;
    uint32_t channel:1;
    uint32_t sampling_frequency:1;
    uint32_t clock_accuracy:1;
    uint32_t sampling_frequency_extended:1;
    uint32_t max_word_length:1;
    uint32_t word_length:1;
    uint32_t original_sampling_frequency:1;
    uint32_t cgms_a:1;
    uint32_t cgms_a_validity:1;
    uint32_t audio_sampling_coefficient:1;
    uint32_t hidden:1;
    uint32_t channel_A_number:1;
    uint32_t channel_B_number:1;
} hdmi_channel_status_mask_t;

typedef struct hdmi_channel_status_s {
    iec60958_channel_status_t channel_status;
    hdmi_channel_status_mask_t mask;
} hdmi_channel_status_t;

/**
 * @typedef hdmi_audio_direction_t
 *   A type indicating the audio stream direction
 */
typedef enum _hdmi_audio_direction_e {
    /*! Audio samples flowing to HDMI switch device */
    HDMI_AUDIO_SOURCE = 0U,
    /*! Audio samples flowing from HDMI switch device */
    HDMI_AUDIO_SINK = 1U,
} hdmi_audio_direction_t;

/**
 * @typedef hdmi_audio_if_t
 *   Physical interfaces transporting audio samples.
 */
typedef enum _hdmi_audio_if_e {
    /*! No interface */
    HDMI_AUDIO_IF_NONE,
    /*! HDMI interface */
    HDMI_AUDIO_IF_HDMI,
    /*! SPDIF interface */
    HDMI_AUDIO_IF_SPDIF,
    /*! ARC interface */
    HDMI_AUDIO_IF_ARC,
    /*! I2S interface */
    HDMI_AUDIO_IF_I2S,
} hdmi_audio_if_t;

/**
 * @typedef hdmi_audio_pkt_t
 *   Audio packet type.
 */
typedef enum _hdmi_audio_pkt_e {
    /*! No Audio packets */
    HDMI_AUDIO_PKT_NONE = 0,
    /*! Standard audio packets LCPM or 60958/61937 */
    HDMI_AUDIO_PKT_STD,
    /*! HBR packets LCPM or 60958/61937 */
    HDMI_AUDIO_PKT_HBR,
    /*! Direct Stream Digital packets */
    HDMI_AUDIO_PKT_DSD,
    /*! Direct Stream Transfer packets */
    HDMI_AUDIO_PKT_DST,
    /*! Invalid audio packet */
    HDMI_AUDIO_PKT_INVALID,
} hdmi_audio_pkt_t;

/**
 * @typedef hdmi_audio_pkt_t
 *   Audio packet type as defined by HDMI spec 1.4 table 7.6
 */
typedef enum _hdmi_audio_pkt_layout_e {
    /*! Audio packet layout 0 - up to 2 channels */
    HDMI_AUDIO_PKT_LAYOUT_0_2CH = 0,
    /*! Audio packet layout 1 - up to 8 channels */
    HDMI_AUDIO_PKT_LAYOUT_1_8CH,
} hdmi_audio_pkt_layout_t;


/**
 * @typedef hdmi_audio_fmt_t
 *   Audio packet format.
 */
typedef enum _hdmi_audio_fmt_e {
    /*! IEC 60958 */
    HDMI_AUDIO_FMT_60958,
    /*! IEC 60937, channel status outband */
    HDMI_AUDIO_FMT_61937,
    /*! Custom packet */
    HDMI_AUDIO_FMT_CUSTOM,
} hdmi_audio_fmt_t;

/**
 * @typedef class_hdmi_cb_t
 *   Asynchronous callback previously registered through set_callback.
 *
 * @param[in]   event   Event type
 * @param[in]   param   Optional parameter. Semantic depends on event type
 * @param[in]   cookie  Callee cookie, set while calling set_callback
 * @return  The callback status error.
 */

typedef int (*hdmi_cb_t)(hdmi_cb_evt_t , void *, void *);

/**
 * @typedef cea861d_audio_infoframe_pkt_s
 *  A type describing the audio infoframe packet as described by HDMI
 *  specification, Table 8.8 and cea861d.
 */
typedef struct cea861d_audio_infoframe_pkt_s {
    /*! Channel count
     *    see CEA-861-D table 17 for details
     */
    uint32_t cc:3;
    /*! Coding Type
     *     shall always be set to a value of 0
     */
    uint32_t reserved_1:1;
    uint32_t ct:4;
    /*! Sample Size
     *     shall always be set to a value of 0
     */
    uint32_t ss:2;
    /*! Sample Frequency
     *     See CEA-861-D table 18 for details. For L-PCM and IEC61937
     *     compressed audio streams, the SF bits shall always be set to a value
     *     of 0 ("Refer to Stream Header"). For One birt Audio and DST streams,
     *     the value indicated by the SF bits shall equal the ACR fs value. For
     *     Super Audio CD, the SF bits are typically set to 0,1,0 to indicate
     *     a Sample Frequency of 2.8224 MSamples/s (i.e. 64*44.1kHz)
     */
    uint32_t sf:3;
    uint32_t reserved_2:3;
    uint8_t reserved_3;
    /*! Channel/Speaker allocation
     *     See CEA-861-D Sections 6.6.2 for details. The CA field is not valid
     *     for IEC-61937 compressed audio streams.
     */
    uint8_t ca;
    /*! LFE Playback level information.
     */
    uint32_t lfepbl:2;
    uint32_t reserved_4:1;
    /*! Level Shift Value (for downmixing).
     *    See CEA-861-D section 6.6.2 and Table 21 for details.
     */
    uint32_t lsv:4;
    /*! Downmixing Inhibit. See CEA-861-D section 6.6.2 and table 22 for
     * details. The DM_INH field is to be set only for DVD-Audio applications
     * and corresponds to the value in the DM_INH field of the current audio
     * stream being played from the disk. The DM_INH field value shall be set to
     * zero in all cases other than DVD-Audio applications.
     */
    uint32_t dm_inh:1;
} cea861d_audio_infoframe_pkt_t ;

/**
 * @typedef hdmi_audio_infoframe_pkt_t
 *  A type describing the audio infoframe packet as described by HDMI
 *  specification, Table 8.8
 */
typedef union hdmi_audio_infoframe_pkt_u {
    cea861d_audio_infoframe_pkt_t value;
    uint8_t raw[5];
}hdmi_audio_infoframe_pkt_t;

/* hdmi interface */
struct hdmi_ops {
    struct driver_ops std;
    /**
     * @brief Open the device
     *
     * @param[in] device HDMI device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*open)(struct device *dev);
    /**
     * @brief Close the device
     *
     * @param[in] device HDMI device
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*close)(struct device *dev);
    /**
     * \brief Get HDMI switch capabilities
     *
     * @param[in] device HDMI device
     * @param[out] capabilities Mask exporting HDMI device capabilities
     *
     * \return A mask holding the capabilities of the HDMI switch/repeater
     */
    status_t (*get_capabilities)(const struct device * dev, uint32_t *capabilities);
    /**
     * @brief Set the callback function for notifying asynchronous changes
     *
     * @param[in] device HDMI device
     * @param[in] cb Callback function to register
     * @param[in] cookie Private callback data
     *
     * @return O in case of success, a negative value otherwise.
     */
    status_t (*set_callback)(struct device *device, hdmi_cb_t cb, void *cookie);
    /**
     * @brief Set the audio sample format
     *
     * @param[in] device HDMI device
     * @param[in] direction Stream direction to apply the operation
     * @param[in] fmt Audio format to use
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if format not supported by HDMI device
     */
    status_t (*set_audio_format)(struct device *device,
                hdmi_audio_direction_t direction, hdmi_audio_fmt_t fmt);
    /**
     * @brief Set the audio sample packet type
     *
     * @param[in] device HDMI device
     * @param[in] direction Stream direction to apply the operation
     * @param[in] pkt Audio packet to use
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if format not supported by HDMI device
     */
    status_t (*set_audio_packet)(struct device *device,
                hdmi_audio_direction_t direction, hdmi_audio_pkt_t pkt);

    /**
     * @brief Set the audio sample interface type
     *
     * @param[in] device HDMI device
     * @param[in] direction Stream direction to apply the operation
     * @param[in] in Physical input audio interface used
     * @param[in] out Physical output audio interface used
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if format not supported by HDMI device
     */
    status_t (*set_audio_interface)(struct device *device,
                                    hdmi_audio_direction_t direction,
                                    hdmi_audio_if_t in, hdmi_audio_if_t out);

    /**
     * @brief Get the channel status
     *
     * @param[in] device HDMI device
     * @param[out] channel_status  Channel status with associated bitfield mask
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if channel status reading not supported by HDMI device
     *    ERR_INVALID_ARGS if channel status is invalid
     */
    status_t (*get_channel_status)(struct device *device,
                                    hdmi_channel_status_t *channel_status);

    /**
     * @brief Get the audio infoframe packet
     *
     * @param[in] device HDMI device
     * @param[out] pkt Audio Infoframe packet content
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if audio infoframe packet reading not supported by HDMI device
     *    ERR_INVALID_ARGS pkt Audio infoframe packet is invalid.. retry.
     */
    status_t (*get_audio_infoframe_pkt)(struct device *device,
                                    hdmi_audio_infoframe_pkt_t *pkt);
    /**
     * @brief Get the audio custom format layout
     *
     * @param[in] device HDMI device
     * @param[out] layout IEC60958 audio custom format layout
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if custom audio format is not supported.
     */
    status_t (*get_audio_custom_fmt_layout)(struct device *device,
                                    iec60958_custom_fmt_layout_t *layout);

    /**
     * @brief Get the audio packet type
     *
     * @param[in] device HDMI device
     * @param[out] pkt Audio packet type pointer
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if custom audio format is not supported.
     */
    status_t (*get_audio_pkt_type)(struct device *dev, hdmi_audio_pkt_t *pkt);

    /**
     * @brief Get the audio packet layout
     *
     * @param[in] device HDMI device
     * @param[out] layout Audio packet layout pointer
     *
     * @return O in case of success, a negative value otherwise.
     *    ERR_NOT_SUPPORTED if custom audio format is not supported.
     */
    status_t (*get_audio_pkt_layout)(struct device *dev, hdmi_audio_pkt_layout_t *layout);
};


__BEGIN_CDECLS
status_t class_hdmi_open(struct device *dev);
status_t class_hdmi_close(struct device *dev);
status_t class_hdmi_get_capabilities(const struct device * dev, uint32_t *capabilities);
status_t class_hdmi_set_callback(struct device *device, hdmi_cb_t cb, void *cookie);
status_t class_hdmi_set_audio_format(struct device *device, hdmi_audio_direction_t direction, hdmi_audio_fmt_t fmt);
status_t class_hdmi_set_audio_packet(struct device *device, hdmi_audio_direction_t direction, hdmi_audio_pkt_t pkt);
status_t class_hdmi_set_audio_interface(struct device *device, hdmi_audio_direction_t direction, hdmi_audio_if_t in, hdmi_audio_if_t out);
status_t class_hdmi_get_channel_status(struct device *device, hdmi_channel_status_t *channel_status);
status_t class_hdmi_get_audio_infoframe_pkt(struct device *device, hdmi_audio_infoframe_pkt_t *pkt);

status_t class_hdmi_get_audio_pkt_type(struct device *, hdmi_audio_pkt_t *);
status_t class_hdmi_get_audio_pkt_layout(struct device *, hdmi_audio_pkt_layout_t *);
status_t class_hdmi_get_audio_custom_fmt_layout(struct device *device, iec60958_custom_fmt_layout_t *layout);

struct device * class_hdmi_get_device_by_id(int bus_id);

void hdmi_print_channel_status(hdmi_channel_status_t *);
void hdmi_print_infoframe(hdmi_audio_infoframe_pkt_t *);
__END_CDECLS

#endif /* __DEV_CLASS_HDMI_H */
