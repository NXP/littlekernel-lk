/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DMA_H
#define _DMA_H

#include <dev/driver.h>
#include <err.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * enum dma_slave_buswidth - defines bus width of the DMA slave
 * device, source or target buses
 */
enum dma_slave_buswidth {
        DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
        DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
        DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
        DMA_SLAVE_BUSWIDTH_3_BYTES = 3,
        DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
        DMA_SLAVE_BUSWIDTH_8_BYTES = 8,
        DMA_SLAVE_BUSWIDTH_16_BYTES = 16,
        DMA_SLAVE_BUSWIDTH_32_BYTES = 32,
        DMA_SLAVE_BUSWIDTH_64_BYTES = 64,
};

/**
 * enum dma_data_direction - dma transfer mode and direction indicator
 * @DMA_MEM_TO_MEM: Async/Memcpy mode
 * @DMA_MEM_TO_DEV: Slave mode & From Memory to Device
 * @DMA_DEV_TO_MEM: Slave mode & From Device to Memory
 * @DMA_DEV_TO_DEV: Slave mode & From Device to Device
 */
enum dma_data_direction {
    DMA_MEM_TO_MEM,
    DMA_MEM_TO_DEV,
    DMA_DEV_TO_MEM,
    DMA_DEV_TO_DEV,
    DMA_TRANS_NONE,
};

/**
 * struct dma_slave_config - DMA slave channel configuration
 * @param src_addr source physical address (RX).
 * @param dst_addr destination physical address (TX).
 * @param src_addr_width source register width in bytes.
 * @param dst_addr_width destination register width in bytes.
 * @param src_maxburst maximum burst size sent to the device.
 * @param dst_maxburst maximum burst size received from the device.
 * @param src_fifo_num bit[0:7] is the fifo number, bit[8:11] is the fifo offset.
 * @param dst_fifo_num same as src_fifo_num.
 */
struct dma_slave_config {
    char *name;
    unsigned int slave_id;
    enum dma_data_direction direction;
    paddr_t src_addr;
    paddr_t dst_addr;
    enum dma_slave_buswidth src_addr_width;
    enum dma_slave_buswidth dst_addr_width;
    uint32_t src_maxburst;
    uint32_t dst_maxburst;
    uint32_t src_fifo_num;
    uint32_t dst_fifo_num;
    uint32_t src_sample_num;
    uint32_t dst_sample_num;
};

struct dma_descriptor;

struct dma_chan {
    const char *name;
    struct list_node node;
    struct device *dma_device;
    struct device *client_device;
    bool chan_available;
    struct dma_slave_config slave_config;
    struct dma_descriptor *desc;
    void *private; /* private data for client-channel */
};
typedef enum dma_xfer_status_e {
    DMA_XFER_COMPLETE,
    DMA_XFER_RUNNING,
    DMA_XFER_PAUSED,
    DMA_XFER_ERROR,
} dma_xfer_status_t;


typedef bool (*dma_cb_t)(struct dma_descriptor *);
typedef bool (*dma_zerocopy_cb_t)(struct dma_descriptor *, uint32_t *, uint32_t *);
struct dma_descriptor {
    struct dma_chan *chan;          /* target channel for this operation */
    dma_cb_t cb;                    /* Client callback to call on completion, error, etc...*/
    uint32_t dma_mode;              /* 0: private buffer, 1: uncached zerocopy, 2: cached zerocopy */
    dma_zerocopy_cb_t zerocopy_cb;  /* Callback for buffer descriptor's update on transfer completion */
    void *cookie;                   /* Client cookie, not used by DMA */
    uint64_t bytes_transferred;
    uint64_t period_elapsed;
    dma_xfer_status_t status;
};


struct dma_chan *dma_request_chan(struct device *dev, const char *name);

int dma_slave_config(struct dma_chan *chan, struct dma_slave_config *config);

struct dma_descriptor *dma_prep_dma_cyclic(
           struct dma_chan *chan, paddr_t buf_addr, size_t buf_len,
           size_t period_len, enum dma_data_direction direction);

status_t dma_submit(struct dma_descriptor *desc);
status_t dma_terminate_sync(struct dma_chan *chan);
status_t dma_pause_channel(struct dma_chan *chan);
status_t dma_resume_channel(struct dma_chan *chan);

#endif /* DMA_H */
