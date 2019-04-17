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
#include <dev/class/pdm.h>

#define PDM_CLASS_CONS(func) \
status_t class_pdm_##func(struct device *dev, bool is_read) \
{\
    struct pdm_ops *ops = device_get_driver_ops(dev, struct pdm_ops, std);\
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

PDM_CLASS_CONS(open)
PDM_CLASS_CONS(close)
PDM_CLASS_CONS(start)
PDM_CLASS_CONS(stop)

status_t class_pdm_setup(struct device *dev, bool is_read, pdm_hw_params_t *pfmt)
{
    struct pdm_ops *ops = device_get_driver_ops(dev, struct pdm_ops, std);
    status_t (*setup)(struct device *dev, pdm_hw_params_t *pfmt);

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

status_t class_pdm_write(struct device *dev, const void *buf, size_t len)
{
    struct pdm_ops *ops = device_get_driver_ops(dev, struct pdm_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->write)
        return ops->write(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

status_t class_pdm_read(struct device *dev, void *buf, size_t len)
{
    struct pdm_ops *ops = device_get_driver_ops(dev, struct pdm_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->read)
        return ops->read(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

