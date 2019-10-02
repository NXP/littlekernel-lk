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
#include <dev/class/adc.h>
#include <lib/appargs.h>

#define LOCAL_TRACE 0

status_t class_adc_open(struct device *dev)
{
    struct adc_ops *ops = device_get_driver_ops(dev, struct adc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->open)
        ret = ops->open(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_adc_close(struct device *dev)
{
    struct adc_ops *ops = device_get_driver_ops(dev, struct adc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->close)
        ret = ops->close(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_adc_get_capabilities(const struct device *dev, uint32_t *cap)
{
    struct adc_ops *ops = device_get_driver_ops(dev, struct adc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->get_capabilities)
        ret = ops->get_capabilities(dev, cap);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_adc_set_format(struct device *dev, adc_audio_hw_params_t *params)
{
    struct adc_ops *ops = device_get_driver_ops(dev, struct adc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_format)
        ret = ops->set_format(dev, params);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_adc_reset_power(struct device *dev)
{
    struct adc_ops *ops = device_get_driver_ops(dev, struct adc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->reset_power)
        ret = ops->reset_power(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

struct device* class_adc_get_device_by_id(int bus_id)
{
    return of_get_device_by_class_and_id("adc", bus_id);
}

