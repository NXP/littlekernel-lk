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
#include <dev/class/gpt.h>

#define GPT_CLASS_CONS(func) \
status_t class_gpt_##func(struct device *dev) \
{\
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);\
\
    if (!ops)\
        return ERR_NOT_CONFIGURED;\
\
    if (ops->func)\
        return ops->func(dev);\
\
    return ERR_NOT_SUPPORTED;\
}

GPT_CLASS_CONS(close)
GPT_CLASS_CONS(start)
GPT_CLASS_CONS(stop)

status_t class_gpt_open(struct device *dev, void *cfg, gpt_timer_callback_t callback, void *userData)
{
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->open)
        return ops->open(dev, cfg, callback, userData);
    else
        return ERR_NOT_SUPPORTED;
}

status_t class_gpt_get_counter(struct device *dev, uint32_t *counter)
{
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_counter)
        return ops->get_counter(dev, counter);
    else
        return ERR_NOT_SUPPORTED;
}

status_t class_gpt_setup_capture(struct device *dev, void *cfg, gpt_capture_callback_t callback, void *userData)
{
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->setup_capture)
        return ops->setup_capture(dev, cfg, callback, userData);
    else
        return ERR_NOT_SUPPORTED;
}

status_t class_gpt_get_capture(struct device *dev, unsigned int channel, uint32_t *counter)
{
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_capture)
        return ops->get_capture(dev, channel, counter);
    else
        return ERR_NOT_SUPPORTED;
}

status_t class_gpt_enable_capture(struct device *dev, unsigned int ch, bool on)
{
    struct gpt_ops *ops = device_get_driver_ops(dev, struct gpt_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->enable_capture)
        return ops->enable_capture(dev, ch, on);
    else
        return ERR_NOT_SUPPORTED;
}

