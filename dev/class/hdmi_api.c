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

#include <err.h>
#include <dev/class/hdmi.h>
#include <lib/appargs.h>
#include <trace.h>

#define LOCAL_TRACE 0

status_t class_hdmi_open(struct device *dev)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->open)
        ret = ops->open(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_close(struct device *dev)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->close)
        ret = ops->close(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_get_capabilities(const struct device * dev, uint32_t *cap)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_capabilities)
        ret = ops->get_capabilities(dev, cap);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_set_callback(struct device *dev, hdmi_cb_t cb, void *cookie)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_callback)
        ret = ops->set_callback(dev, cb, cookie);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_set_audio_format(struct device *dev,
                        hdmi_audio_direction_t direction, hdmi_audio_fmt_t fmt)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_audio_format)
        ret = ops->set_audio_format(dev, direction, fmt);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_set_audio_packet(struct device *dev,
                        hdmi_audio_direction_t direction, hdmi_audio_pkt_t pkt)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_audio_packet)
        ret = ops->set_audio_packet(dev, direction, pkt);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_set_audio_interface(struct device *dev,
                                    hdmi_audio_direction_t direction,
                                    hdmi_audio_if_t in, hdmi_audio_if_t out)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_audio_interface)
        ret = ops->set_audio_interface(dev, direction, in, out);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_get_channel_status(struct device *dev,
                                        hdmi_channel_status_t *channel_status)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_channel_status)
        ret = ops->get_channel_status(dev, channel_status);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_get_audio_infoframe_pkt(struct device *dev,
                                                hdmi_audio_infoframe_pkt_t *pkt)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_audio_infoframe_pkt)
        ret = ops->get_audio_infoframe_pkt(dev, pkt);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_get_audio_custom_fmt_layout(struct device *dev, iec60958_custom_fmt_layout_t *layout)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_audio_custom_fmt_layout)
        ret = ops->get_audio_custom_fmt_layout(dev, layout);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;

}

status_t class_hdmi_get_audio_pkt_type(struct device *dev,
                                        hdmi_audio_pkt_t *pkt)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_audio_pkt_type)
        ret = ops->get_audio_pkt_type(dev, pkt);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_hdmi_get_audio_pkt_layout(struct device *dev,
                                        hdmi_audio_pkt_layout_t *layout)
{
    struct hdmi_ops *ops = device_get_driver_ops(dev, struct hdmi_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_audio_pkt_layout)
        ret = ops->get_audio_pkt_layout(dev, layout);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

struct device * class_hdmi_get_device_by_id(int bus_id)
{
    return of_get_device_by_class_and_id("hdmi", bus_id);
}


void hdmi_print_channel_status(hdmi_channel_status_t *cs)
{

    TRACEF("Channel status:\n");
    if (cs->mask.professional == 1)
        TRACEF("\t%s: %#x\n", "professional", cs->channel_status.cs.professional);

    if (cs->mask.lpcm == 1)
        TRACEF("\t%s: %#x\n", "lpcm", cs->channel_status.cs.lpcm);

    if (cs->mask.copyright == 1)
        TRACEF("\t%s: %#x\n", "copyright", cs->channel_status.cs.copyright);

    if (cs->mask.pre_emphasis == 1)
        TRACEF("\t%s: %#x\n", "pre_emphasis", cs->channel_status.cs.pre_emphasis);

    if (cs->mask.mode == 1)
        TRACEF("\t%s: %#x\n", "mode", cs->channel_status.cs.mode);

    if (cs->mask.category == 1)
        TRACEF("\t%s: %#x\n", "category", cs->channel_status.cs.category);

    if (cs->mask.source == 1)
        TRACEF("\t%s: %#x\n", "source", cs->channel_status.cs.source);

    if (cs->mask.channel == 1)
        TRACEF("\t%s: %#x\n", "channel", cs->channel_status.cs.channel);

    if (cs->mask.sampling_frequency == 1)
        TRACEF("\t%s: %#x\n", "sampling_frequency", cs->channel_status.cs.sampling_frequency);
    if (cs->mask.clock_accuracy == 1)
        TRACEF("\t%s: %#x\n", "clock_accuracy", cs->channel_status.cs.clock_accuracy);

    if (cs->mask.sampling_frequency_extended == 1)
        TRACEF("\t%s: %#x\n", "sampling_frequency_extended ", cs->channel_status.cs.sampling_frequency_extended);

    if (cs->mask.max_word_length == 1)
        TRACEF("\t%s: %#x\n", "max_word_length", cs->channel_status.cs.max_word_length);

    if (cs->mask.word_length == 1)
        TRACEF("\t%s: %#x\n", "word_length", cs->channel_status.cs.word_length);

    if (cs->mask.original_sampling_frequency == 1)
        TRACEF("\t%s: %#x\n", "original_sampling_frequency ", cs->channel_status.cs.original_sampling_frequency);

    if (cs->mask.cgms_a == 1)
        TRACEF("\t%s: %#x\n", "cgms_a", cs->channel_status.cs.cgms_a);

    if (cs->mask.cgms_a_validity == 1)
        TRACEF("\t%s: %#x\n", "cgms_a_validity", cs->channel_status.cs.cgms_a_validity);

    if (cs->mask.audio_sampling_coefficient == 1)
        TRACEF("\t%s: %#x\n", "audio_sampling_coefficient", cs->channel_status.cs.audio_sampling_coefficient);

    if (cs->mask.hidden == 1)
        TRACEF("\t%s: %#x\n", "hidden", cs->channel_status.cs.hidden);

    if (cs->mask.channel_A_number == 1)
        TRACEF("\t%s: %#x\n", "channel_A_number", cs->channel_status.cs.channel_A_number);

    if (cs->mask.channel_B_number == 1)
        TRACEF("\t%s: %#x\n", "channel_B_number", cs->channel_status.cs.channel_B_number);

}


void hdmi_print_infoframe(hdmi_audio_infoframe_pkt_t *pkt)
{
    TRACEF("Audio InfoFrame packet:\n");
    TRACEF("\tChannel Count (cc): %x\n", pkt->value.cc);
    TRACEF("\tCoding type (ct): %x\n", pkt->value.ct);
    TRACEF("\tSample frequency (sf): %x\n", pkt->value.sf);
    TRACEF("\tChannel allocation (ca): %x\n", pkt->value.ca);
    TRACEF("\tLFE Playback level information value (lfepbl): %x\n", pkt->value.lfepbl);
    TRACEF("\tLevel shift value (lsv): %x\n", pkt->value.lsv);
    TRACEF("\tDownmixing inhibit (dm_inh): %x\n", pkt->value.dm_inh);
}
