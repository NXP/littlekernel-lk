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

#include <err.h>
#include <dev/class/spdif.h>

#define SPDIF_CLASS_CONS(func) \
status_t class_spdif_##func(struct device *dev, bool is_read) \
{\
    struct spdif_ops *ops = device_get_driver_ops(dev, struct spdif_ops, std);\
    status_t (*func)(struct device *dev);\
\
    if (!ops)\
        return ERR_NOT_CONFIGURED;\
\
    if (is_read)\
        func = ops->rx_##func;\
    else \
        func = ops->tx_##func;\
\
    if (func)\
        return func(dev);\
\
    return ERR_NOT_SUPPORTED;\
}

SPDIF_CLASS_CONS(open)
SPDIF_CLASS_CONS(close)
SPDIF_CLASS_CONS(start)
SPDIF_CLASS_CONS(stop)
SPDIF_CLASS_CONS(flush)

status_t class_spdif_setup(struct device *dev, bool is_read, spdif_hw_params_t *pfmt)
{
    struct spdif_ops *ops = device_get_driver_ops(dev, struct spdif_ops, std);
    status_t (*setup)(struct device *dev, spdif_hw_params_t *pfmt);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (is_read)
        setup = ops->rx_setup;
    else
        setup = ops->tx_setup;

    if (setup)
        return setup(dev, pfmt);

    return ERR_NOT_SUPPORTED;
}

status_t class_spdif_write(struct device *dev, const void *buf, size_t len)
{
    struct spdif_ops *ops = device_get_driver_ops(dev, struct spdif_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->write)
        return ops->write(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

status_t class_spdif_read(struct device *dev, void *buf, size_t len)
{
    struct spdif_ops *ops = device_get_driver_ops(dev, struct spdif_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->read)
        return ops->read(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

status_t class_spdif_set_callback(struct device *dev, spdif_cb_t cb, void *cookie)
{
    struct spdif_ops *ops = device_get_driver_ops(dev, struct spdif_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->set_callback)
        return ops->set_callback(dev, cb, cookie);

    return ERR_NOT_SUPPORTED;
}

