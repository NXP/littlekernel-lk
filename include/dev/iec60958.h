/*
 * Copyright 2019-2021 NXP
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
#ifndef __DEV_IEC60958_H
#define __DEV_IEC60958_H

/**
 * @typedef iec60958_custom_fmt_layout_t
 *  A type describing a custom iec60958 format layout
 *  A negative value should be considered as invalid.
 */
typedef struct iec60958_custom_fmt_layout_s {
    /*! Bit position of data LSB */
    char data_start;
    /*! Data length */
    char data_length;
    /*! Bit position of validity bit */
    char validity_bit;
    /*! Bit position of parity bit */
    char parity_bit;
    /*! Bit position of user bit */
    char user_bit;
    /*! Bit position of channel status bit */
    char channel_status_bit;
    /*! Bit position of block start bit */
    char block_start_bit;
    /*! Occupied width of complete frame */
    char width;
} iec60958_custom_fmt_layout_t;

/**
 * @typedef iec60958_channel_status_t
 *  A type describing the channel status as described by IEC60958-3
 */

struct iec60958_channel_status_s {
    /*! Professional use of channel status block if set, consumer otherwise */
    uint32_t professional:1;
    /*! Main data field represents linear PCM samples if unset, compressed
     * otherwise*/
    uint32_t lpcm:1;
    /*! Copyright protection asserted */
    uint32_t copyright:1;
    /*! Pre-emphasis */
    uint32_t pre_emphasis:3;
    /*! mode */
    uint32_t mode:2;
    /*! Category code */
    uint8_t category;
    /*! Source number */
    uint32_t source:4;
    /*! Channel number */
    uint32_t channel:4;
    /*! Sampling frequency */
    uint32_t sampling_frequency:4;
    /*! Clock accuracy */
    uint32_t clock_accuracy:2;
    /*! Sampling frequency extenstion */
    uint32_t sampling_frequency_extended:2;
    /*! Maximal word length */
    uint32_t max_word_length:1;
    /*! Sample length */
    uint32_t word_length:3;
    /*! Original Sampling frequency */
    uint32_t original_sampling_frequency:4;
    /*! CGMS-A */
    uint32_t cgms_a:2;
    /*! CGMS-A validity */
    uint32_t cgms_a_validity:1;
    /*! Audio sampling frequency coefficient */
    uint32_t audio_sampling_coefficient:4;
    /*! Hidden information */
    uint32_t hidden:1;
    /*! General channel assignement for A channel */
    uint32_t channel_A_number:6;
    /*! General channel assignement for B channel */
    uint32_t channel_B_number:6;
    uint32_t reserved_1:3;
    uint8_t reserved_2[16];
};

typedef union iec60958_channel_status_u {
    struct iec60958_channel_status_s cs;
    uint8_t raw[24];
} iec60958_channel_status_t;

/*
 * HDMI c2.1 Table 9.23
 * Channel Status audio type definitions bits 0,1,3,4 and 5
 * Non-eARC use cases can only be subset below:
 * - UNENCRYPTED_2CH_LPCM
 * - UNENCRYPTED_COMPRESSED
 * Other values are specific to eARC use cases
 */
typedef enum _iec60958_audio_format {
    UNENCRYPTED_2CH_LPCM = 0,
    UNENCRYPTED_nCH_LPCM = 16,
    UNENCRYPTED_FMT = 24,
    UNENCRYPTED_COMPRESSED = 2,
    ENCRYPTED_COMPRESSED = 6,
    ENCRYPTED_nCH_LPCM = 22,
    ENCRYPTED_FMT = 30,
} iec60958_audio_format_t;


static inline iec60958_audio_format_t iec60958_get_audio_format(iec60958_channel_status_t *cs)
{
    return ((cs->raw[0] & 0x3) | ((cs->raw[0] & 0x38) >> 1));
}

/*
 * HDMI c2.1 Table 9.25
 * Channel Status Multi-channel audio layout bits 44, 45, 46, 47
 * Those definitions are applicable only to Multi-Channel L-PCM
 * audio types in eARC use cases.
 */
typedef enum _iec60958_nch_lpcm {
    nCH_LPCM_2CH = 0,
    nCH_LPCM_8CH = 7,
    nCH_LPCM_16CH = 11,
    nCH_LPCM_32CH = 3,
} iec60958_nch_lpcm_t;

static inline int iec60958_get_nch_lpcm(iec60958_channel_status_t *cs)
{
    int ret = -1;

    iec60958_audio_format_t fmt = iec60958_get_audio_format(cs);
    if (!((fmt == UNENCRYPTED_nCH_LPCM) || (ENCRYPTED_nCH_LPCM == fmt)))
        return -1;

    switch (cs->cs.audio_sampling_coefficient) {
    case nCH_LPCM_2CH:
        ret = 2;
        break;
    case nCH_LPCM_8CH:
        ret = 8;
        break;
    case nCH_LPCM_16CH:
        ret = 16;
        break;
    case nCH_LPCM_32CH:
        ret = 32;
        break;
    default:
        ret = -1;
    }

    return ret;
}

typedef enum _iec60958_audio_sample_rate_ratio {
    AUDIO_SAMPLE_RATE_RATIO_NO_IND = 0,
    AUDIO_SAMPLE_RATE_RATIO_EQUAL = 8,
    AUDIO_SAMPLE_RATE_RATIO_DIV_2 = 4,
    AUDIO_SAMPLE_RATE_RATIO_DIV_4 = 12,
    AUDIO_SAMPLE_RATE_RATIO_DIV_8 = 2,
    AUDIO_SAMPLE_RATE_RATIO_DIV_16 = 10,
    AUDIO_SAMPLE_RATE_RATIO_DIV_32 = 6,
    AUDIO_SAMPLE_RATE_RATIO_MULT_32 = 13,
    AUDIO_SAMPLE_RATE_RATIO_MULT_16 = 3,
    AUDIO_SAMPLE_RATE_RATIO_MULT_8 = 11,
    AUDIO_SAMPLE_RATE_RATIO_MULT_4 = 7,
    AUDIO_SAMPLE_RATE_RATIO_MULT_2 = 15
} iec60958_audio_sample_rate_ratio_t;

/*
 * @struct iec60958_subframe_s Define subframe as defined per IEC60958
 */
struct iec60958_subframe_s {
    /*! Sub frame preamble, B, M, W, (Z, X, Y) */
    uint32_t sync_preamble:4;
    /*! Audio sample 24bit word */
    uint32_t sample_word:24;
    /*! Validity flag */
    uint32_t validity_flag:1;
    /*! User data */
    uint32_t user_data:1;
    /*! Channel status */
    uint32_t channel_status:1;
    /*! Parity bit */
    uint32_t parity_bit:1;
};

/*
 * @struct audio_hal_iec958_block_iband Hold 192 frames, defining an audio block
 * as per IEC60958 specification.
 */
struct iec60958_block_iband_s {
    union {
        struct iec60958_subframe_s subframe[192 * 2];
        uint32_t raw_subframe[192 * 2];
    };
};

/*
 * @struct iec60958_block_oband_s Hold 192 frames, defining an audio block
 * as per IEC60958 specification with signaling information extracted.
 */
struct iec60958_block_oband_s {
    /*! 24b Audio sample word extracted from IEC60958 subframe stored on 32b word,
     * left aligned.
     **/
    uint32_t data[192 * 2];
    /*! Channel status information if extracted from an out-band stream */
    uint8_t cs[24];
    /*! User data  information if extracted from an out-band stream */
    uint8_t user[24];
};

/*
 * @struct iec60958_block_u Hold 192 frames, whatever the hardware device
 * capability to extract signaling information from sub-frame.
 * Used by Symphony to read data from HAL
 */
union iec60958_block_u {
    /*! Raw IEC60958 data, aka in-band */
    struct iec60958_block_iband_s iband;
    /*! IEC60958 data with signaling data extraced aside, aka out-band */
    struct iec60958_block_oband_s oband;
};

static inline int iec60958_get_word_length(struct iec60958_channel_status_s *cs)
{
    int len;
    int offset = cs->max_word_length ? 4 : 0;

    switch(cs->word_length) {
    case 1:
        len = 16 + offset;
        break;
    case 2:
        len = 18 + offset;
        break;
    case 4:
        len = 19 + offset;
        break;
    case 5:
        len = 20 + offset;
        break;
    case 6:
        len = 17 + offset;
        break;
    case 0:
    default:
        len = -1;
        break;
    }

    return len;
}

static inline int iec60958_get_sampling_frequency(struct iec60958_channel_status_s *cs)
{
    int rate;
    unsigned fs = cs->sampling_frequency | (cs->sampling_frequency_extended << 4);

    switch(fs) {
    case 0x00:
        rate = 44100;
        break;
    case 0x02:
        rate = 48000;
        break;
    case 0x03:
        rate = 32000;
        break;
    case 0x04:
        rate = 22050;
        break;
    case 0x05:
        rate = 384000;
        break;
    case 0x06:
        rate = 24000;
        break;
    case 0x08:
        rate = 88200;
        break;
    case 0x0A:
        rate = 96000;
        break;
    case 0x0B:
        rate = 64000;
        break;
    case 0x0C:
        rate = 176400;
        break;
    case 0x0D:
        rate = 352800;
        break;
    case 0x0E:
        rate = 192000;
        break;
        break;
    case 0x09:
        rate = 768000;
        break;
    case 0x15:
        rate = 1536000;
        break;
    case 0x1D:
        rate = 1411200;
        break;
    case 0x1B:
        rate = 256000;
        break;
    case 0x2B:
        rate = 128000;
        break;
    case 0x3B:
        rate = 512000;
        break;
    case 0x2D:
        rate = 705600;
        break;
    case 0x35:
        rate = 1024000;
        break;
    default:
        rate = 0;
        break;
    }

    return rate;
}

static int iec60958_get_rate(iec60958_channel_status_t *cs)
{
    int rate = iec60958_get_sampling_frequency(&cs->cs);

    if (rate <= 0)
        return 0;

    switch (iec60958_get_audio_format(cs)) {
    case UNENCRYPTED_2CH_LPCM:
        /* HDMI or eARC 2chans - nothing to do */
        break;
    case UNENCRYPTED_nCH_LPCM:
    case ENCRYPTED_nCH_LPCM:
        /* eARC multichannel - amend rate per channel layout */
        switch (cs->cs.audio_sampling_coefficient) {
        case nCH_LPCM_2CH:
            /* nothing to do */
            break;
        case nCH_LPCM_8CH:
            rate /= 4;
            break;
        case nCH_LPCM_16CH:
            rate /= 8;
            break;
        case nCH_LPCM_32CH:
            rate /= 16;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return rate;
}



#endif /* __DEV_IEC60958_H */
