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
#ifndef __DEV_CLASS_GPT_H
#define __DEV_CLASS_GPT_H

#include <compiler.h>
#include <dev/driver.h>

/*! @brief GPT timer callback prototype */
typedef void (*gpt_timer_callback_t)(void *userData);
/*! @brief GPT capture callback prototype */
typedef void (*gpt_capture_callback_t)(void *userData);

/* gpt interface */
struct gpt_ops {
    struct driver_ops std;

    /* initial configuration */
    status_t (*open)(struct device *dev, void *cfg, gpt_timer_callback_t callback, void *userData);
    /* allows driver to be reconfigured */
    status_t (*close)(struct device *dev);
    /* stop the counter */
    status_t (*stop)(struct device *dev);
    /* start the counter */
    status_t (*start)(struct device *dev);
    /* get current counter value */
    status_t (*get_counter)(struct device *dev, uint32_t *counter);
    /* capture configuration */
    status_t (*setup_capture)(struct device *dev, void *cfg, gpt_capture_callback_t callback, void *userData);
    /* enable/disable capture */
    status_t (*enable_capture)(struct device *dev, unsigned int ch, bool on);
    /* get counter value when last capture occured */
    status_t (*get_capture)(struct device *dev, unsigned int channel, uint32_t *counter);
};

__BEGIN_CDECLS

status_t class_gpt_open(struct device *dev, void *cfg, gpt_timer_callback_t callback, void *userData);
status_t class_gpt_close(struct device *dev);
status_t class_gpt_stop(struct device *dev);
status_t class_gpt_start(struct device *dev);
status_t class_gpt_get_counter(struct device *dev, uint32_t *counter);
status_t class_gpt_setup_capture(struct device *dev, void *cfg, gpt_capture_callback_t callback, void *userData);
status_t class_gpt_enable_capture(struct device *dev, unsigned int channel, bool on);
status_t class_gpt_get_capture(struct device *dev, unsigned int channel, uint32_t *counter);
status_t class_gpt_close_capture(struct device *dev, unsigned int channel);

struct device * class_gpt_get_device_by_id(int bus_id);

__END_CDECLS

#endif /* __DEV_CLASS_GPT_H */
