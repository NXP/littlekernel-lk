/*
 * Copyright 2018-2020 NXP
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
#include <dev/class/sai.h>

#define SAI_CLASS_CONS(func) \
status_t class_sai_##func(struct device *dev, bool is_read) \
{\
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);\
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

SAI_CLASS_CONS(open)
SAI_CLASS_CONS(close)
SAI_CLASS_CONS(start)
SAI_CLASS_CONS(stop)
SAI_CLASS_CONS(flush)
SAI_CLASS_CONS(drop)

status_t class_sai_setup(struct device *dev, bool is_read, sai_format_t *pfmt)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    status_t (*setup)(struct device *dev, sai_format_t *pfmt);

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

status_t class_sai_write(struct device *dev, const void *buf, size_t len)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->write)
        return ops->write(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

status_t class_sai_read(struct device *dev, void *buf, size_t len)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (ops->read)
        return ops->read(dev, buf, len);

    return ERR_NOT_SUPPORTED;
}

status_t class_sai_get_counters(struct device *dev,
                uint32_t *bitc, uint32_t *ts, uint64_t *cpu_ts, bool is_read)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    status_t (*get_cnt)(struct device *dev, uint32_t *bitc,
                                        uint32_t *bitc_ts, uint64_t *cpu_ts);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (is_read)
        get_cnt = ops->rx_get_cnt;
    else
        get_cnt = ops->tx_get_cnt;

    if (get_cnt)
        return get_cnt(dev, bitc, ts, cpu_ts);

    return ERR_NOT_SUPPORTED;
}

status_t class_sai_en_counters(struct device *dev, bool en, bool is_read)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    status_t (*en_cnt)(struct device *dev, bool en);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (is_read)
        en_cnt = ops->rx_en_cnt;
    else
        en_cnt = ops->tx_en_cnt;

    if (en_cnt)
        return en_cnt(dev, en);

    return ERR_NOT_SUPPORTED;
}

status_t class_sai_set_callback(struct device *dev, sai_cb_t cb,
                                                void *cookie, bool is_read)
{
    struct sai_ops *ops = device_get_driver_ops(dev, struct sai_ops, std);
    status_t (*set_cb)(struct device *device, sai_cb_t cb, void *cookie);

    if (!ops)
        return ERR_NOT_CONFIGURED;

    if (is_read)
        set_cb = ops->rx_set_callback;
    else
        set_cb = ops->tx_set_callback;

    if (set_cb)
        return set_cb(dev, cb, cookie);

    return ERR_NOT_SUPPORTED;

}
