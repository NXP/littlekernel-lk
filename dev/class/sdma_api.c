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
#include <dev/class/sdma.h>
#include <dev/dma.h>


status_t class_dma_request_channel(struct device *dev, struct dma_chan **chan)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->request_channel)
        return ops->request_channel(dev, chan);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_prepare_cyclic(struct device *dev,
       struct dma_chan *chan, paddr_t buf_addr, size_t buf_len,
       size_t period_len, enum dma_data_direction direction,
       struct dma_descriptor **desc)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->prepare_dma_cyclic)
        return ops->prepare_dma_cyclic(dev, chan, buf_addr,
                                    buf_len, period_len, direction, desc);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_slave_config(struct device *dev, struct dma_chan *chan,
                                            struct dma_slave_config *cfg)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->slave_config)
        return ops->slave_config(dev, chan, cfg);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_submit(struct device *dev, struct dma_descriptor *desc)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->submit)
        return ops->submit(dev, desc);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_resume_channel(struct device *dev, struct dma_chan *chan)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->resume_channel)
        return ops->resume_channel(dev, chan);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_pause_channel(struct device *dev, struct dma_chan *chan)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->pause_channel)
        return ops->pause_channel(dev, chan);

    return ERR_NOT_SUPPORTED;
}

status_t class_dma_terminate(struct device *dev, struct dma_chan *chan)
{
    struct sdma_ops *ops = device_get_driver_ops(dev, struct sdma_ops, std);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->terminate)
        return ops->terminate(dev, chan);

    return ERR_NOT_SUPPORTED;
}

