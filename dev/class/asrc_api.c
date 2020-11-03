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

#include <err.h>
#include <dev/class/asrc.h>
#include <lib/appargs.h>

status_t class_asrc_open(struct device *dev)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->open)
        ret = ops->open(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_asrc_close(struct device *dev)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->close)
        ret = ops->close(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_asrc_setup(struct device *dev, asrc_audio_params_t *pfmt)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->setup)
        ret = ops->setup(dev, pfmt);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_asrc_start(struct device *dev, asrc_transfer_params_t *pxfer, asrc_out_params_t *pout)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->start)
        ret = ops->start(dev, pxfer, pout);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_asrc_flush(struct device *dev, asrc_flush_params_t *pflush, asrc_out_params_t *pout)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->flush)
        ret = ops->flush(dev, pflush, pout);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

status_t class_asrc_stop(struct device *dev)
{
    struct asrc_ops *ops = device_get_driver_ops(dev, struct asrc_ops, std);
    status_t ret;

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->stop)
        ret = ops->stop(dev);
    else
        ret = ERR_NOT_IMPLEMENTED;

    return ret;
}

struct device *class_asrc_get_device_by_id(int bus_id)
{
    return of_get_device_by_class_and_id("asrc", bus_id);
}
