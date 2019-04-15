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
#ifndef __DEV_CLASS_SDMA_H
#define __DEV_CLASS_SDMA_H

#include <compiler.h>
#include <dev/driver.h>
#include <dev/dma.h>

/* sdma interface */
struct sdma_ops {
    struct driver_ops std;
    status_t (*request_channel)(struct device *dev, struct dma_chan** chan);
    status_t (*prepare_dma_cyclic)(struct device *dev,
       struct dma_chan *chan, paddr_t buf_addr, size_t buf_len,
       size_t period_len, enum dma_data_direction direction,
       struct dma_descriptor ** descriptor);
    status_t (*slave_config)(struct device *dev, struct dma_chan *ch,
                                            struct dma_slave_config *cfg);
    status_t (*submit)(struct device *dev, struct dma_descriptor *desc);
    status_t (*terminate)(struct device *dev, struct dma_chan *chan);
    status_t (*resume_channel)(struct device *dev, struct dma_chan *chan);
    status_t (*pause_channel)(struct device *dev, struct dma_chan *chan);
};

__BEGIN_CDECLS
struct device * class_sdma_get_device_by_id(int);
status_t class_dma_request_channel(struct device *, struct dma_chan **);
status_t class_dma_prepare_cyclic(struct device *, struct dma_chan *,
    paddr_t, size_t, size_t, enum dma_data_direction, struct dma_descriptor **);
status_t class_dma_slave_config(struct device *, struct dma_chan *,
                                            struct dma_slave_config *);
status_t class_dma_submit(struct device *, struct dma_descriptor *);
status_t class_dma_resume_channel(struct device *, struct dma_chan *);
status_t class_dma_pause_channel(struct device *, struct dma_chan *);
status_t class_dma_terminate(struct device *, struct dma_chan *);
__END_CDECLS

#endif /* __DEV_CLASS_SDMA_H */
