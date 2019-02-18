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
#ifndef __DEV_CLASS_GPIO_H
#define __DEV_CLASS_GPIO_H

#include <compiler.h>
#include <dev/driver.h>

struct fdt_args;
struct gpio_desc;

/* GPIO callback */
struct gpio_ops {
    struct driver_ops std;
    int (*request)(struct device *dev, unsigned offset, const char *label);
    int (*free)(struct device *dev, unsigned offset);
    int (*direction_input)(struct device *dev, unsigned offset);
    int (*direction_output)(struct device *dev, unsigned offset, int value);
    int (*get_value)(struct device *dev, unsigned offset);
    int (*set_value)(struct device *dev, unsigned offset, int value);
    int (*get_open_drain)(struct device *dev, unsigned offset);
    int (*set_open_drain)(struct device *dev, unsigned offset, int value);
    int (*get_gpio_desc)(struct device *dev, struct gpio_desc *desc, struct fdt_args *args);
};

#define dev_to_gpio_ops(dev) ((struct gpio_ops *)(dev->driver->ops))
struct gpio_desc {
    struct device *dev;
    unsigned flags;
#define GPIO_DESC_REQUESTED     (1 << 0)
#define GPIO_DESC_OUTPUT        (1 << 1)
#define GPIO_DESC_INPUT        (1 << 2)
#define GPIO_DESC_ACTIVE_LOW    (1 << 3)
#define GPIO_DESC_OUTPUT_ACTIVE (1 << 4)
    unsigned nr;
};

struct gpio_controller {
    struct device *dev;
    unsigned base;
    unsigned count;
    struct list_node node;
    size_t private_size;
    char private[];
};
#define gpio_ctrl_to_host(_ctrl) ((void *) (&_ctrl->private[0]))

struct gpio_controller * gpio_controller_add(struct device *, unsigned, size_t);
void gpio_controller_remove(struct gpio_controller *);

status_t gpio_request_by_name(struct device *, const char *, struct gpio_desc *, int);
status_t of_gpio_request_by_name(int, const char *, struct gpio_desc *, int);
status_t gpio_free(struct device *, struct gpio_desc *);

status_t gpio_get_desc(struct device *, struct gpio_desc *, struct fdt_args *args);

//status_t gpio_set_direction_input(struct gpio_desc *);
//status_t gpio_set_direction_output(struct gpio_desc *);
status_t gpio_desc_set_direction(struct gpio_desc *);

int gpio_desc_get_value(struct gpio_desc *);
int gpio_desc_set_value(struct gpio_desc *, unsigned);
int gpio_desc_set_open_drain(struct gpio_desc *, unsigned);
int gpio_desc_get_open_drain(struct gpio_desc *);

__END_CDECLS

#endif /* __DEV_CLASS_GPIO_H */
