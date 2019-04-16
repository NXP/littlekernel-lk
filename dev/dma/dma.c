/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <kernel/mutex.h>

#include <err.h>
#include <list.h>
#include <dev/class/sdma.h>
#include <lib/appargs.h>

#include <trace.h>
#include <assert.h>
#include <dev/dma.h>

#define LOCAL_TRACE 0

status_t dma_slave_config(struct dma_chan *chan, struct dma_slave_config *config)
{
    if ((chan == NULL) || (config == NULL))
        return ERR_INVALID_ARGS;

    if (chan->dma_device == NULL)
        return ERR_NOT_READY;

    return class_dma_slave_config(chan->dma_device, chan, config);
}

/**
 * dma_request_chan - allocate a DMA channel
 * @param dev
 * @param name
 * Returns pointer to appropriate DMA channel on success or NULL
 */
struct dma_chan *dma_request_chan(struct device *dev, const char *name)
{
    struct dma_chan *chan = NULL;
    struct device *dma_device;
    LTRACE_ENTRY;

    /* device is a dma client (e.g. spdif) */
    /* device has: const void * config and void *state, struct driver */

    /* config contains the dma_channel information: name of a specific dma instance
     * and dma_channel arguments (hw_request#, ...)
     */
    struct device_cfg_dma_channel *dma_cfg =
        device_config_get_dma_channel_by_name(dev->config, name);
    if (dma_cfg == NULL)
        return NULL;

    dma_device = of_find_device_by_node(dma_cfg->dma_spec.node);
    if (dma_device == NULL)
        return NULL;

    LTRACEF("found dma %s %s\n", dma_cfg->name, dma_device->name);

    status_t ret = class_dma_request_channel(dma_device, &chan);
    if ((ret) || (chan == NULL)) {
        TRACEF("Error (%d) while requesting a dma channel (%p)\n", ret, chan);
        return NULL;
    }

    chan->client_device = dev;
    chan->name = name;

    LTRACE_EXIT;
    return chan;
}

struct dma_descriptor *dma_prep_dma_cyclic(
           struct dma_chan *chan, paddr_t buf_addr, size_t buf_len,
           size_t period_len, enum dma_data_direction direction
           /* TODO: add callback function ?*/)
{
    if ((chan == NULL) || (chan->dma_device == NULL))
        return NULL;

    struct dma_descriptor *desc;

    status_t ret = class_dma_prepare_cyclic(chan->dma_device, chan,
                            buf_addr, buf_len, period_len, direction, &desc);
    if (ret) {
        TRACEF("Error (%d) while preparing a cyclic transfer\n", ret);
        return NULL;
    }

    return desc;
}

status_t dma_submit(struct dma_descriptor *desc)
{
    if ((desc == NULL) || (desc->chan == NULL))
        return ERR_INVALID_ARGS;

    struct device *dma_device = desc->chan->dma_device;
    if (dma_device == NULL)
        return ERR_NOT_READY;

    desc->bytes_transferred = 0;
    desc->period_elapsed = 0;

    return class_dma_submit(dma_device, desc);
}

status_t dma_resume_channel(struct dma_chan *chan)
{
    if (chan == NULL)
        return ERR_INVALID_ARGS;

    struct device *dma_device = chan->dma_device;
    if (dma_device == NULL) {
        return ERR_NOT_READY;
    }

    return class_dma_resume_channel(dma_device, chan);
}

status_t dma_pause_channel(struct dma_chan *chan)
{
    if (chan == NULL)
        return ERR_INVALID_ARGS;

    struct device *dma_device = chan->dma_device;
    if (dma_device == NULL) {
        return ERR_NOT_READY;
    }

    return class_dma_pause_channel(dma_device, chan);
}

status_t dma_terminate_sync(struct dma_chan *chan)
{
    if (chan == NULL)
        return ERR_INVALID_ARGS;

    struct device *dma_device = chan->dma_device;
    if (dma_device == NULL) {
        return ERR_NOT_READY;
    }

    return class_dma_terminate(dma_device, chan);
}
