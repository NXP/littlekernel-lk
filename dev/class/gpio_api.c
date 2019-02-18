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
#include <trace.h>
#include <assert.h>
#include <stdlib.h>
#include <list.h>
#include <kernel/spinlock.h>
#include <lib/appargs.h>
#include <dev/class/gpio.h>

#define LOCAL_TRACE 0
#include <libfdt.h>
static struct list_node gpio_controller_list = LIST_INITIAL_VALUE(gpio_controller_list);
static spin_lock_t gpio_controller_list_lock = SPIN_LOCK_INITIAL_VALUE;

static unsigned gpio_controller_base;

struct gpio_controller * gpio_controller_add(
                        struct device *dev, unsigned count, size_t private_size)
{
    struct gpio_controller * controller = malloc(sizeof(struct gpio_controller) + private_size);
    ASSERT(controller);

    /* Init the internal controller fields, driver not supposed to modify them */
    controller->dev = dev;
    controller->count = count;
    controller->private_size = private_size;

    spin_lock_saved_state_t lock_state;
    spin_lock_irqsave(&gpio_controller_list_lock, lock_state);
    controller->base = gpio_controller_base++;
    list_add_tail(&gpio_controller_list, &controller->node);
    spin_unlock_irqrestore(&gpio_controller_list_lock, lock_state);

    LTRACEF("GPIO controller (%s-%d) with %d gpios registered\n",
        controller->dev->name, controller->base, controller->count);

    return controller;
}

void gpio_controller_remove(struct gpio_controller *controller)
{

    spin_lock_saved_state_t lock_state;
    spin_lock_irqsave(&gpio_controller_list_lock, lock_state);
    list_delete(&controller->node);
    spin_unlock_irqrestore(&gpio_controller_list_lock, lock_state);

    free(controller);
}

status_t gpio_get_desc(struct device *dev,
                    struct gpio_desc *desc, struct fdt_args *args)
{

    if (dev_to_gpio_ops(dev)->get_gpio_desc) {
        return dev_to_gpio_ops(dev)->get_gpio_desc(dev, desc, args);
    } else {
        if (args->args_count < 1)
            return ERR_INVALID_ARGS;
        desc->dev = dev;
        desc->flags = 0;
        desc->nr = args->args[0];
        if (args->args_count < 2)
            return 0;
        desc->flags = args->args[1];
        return 0;
    }
}

status_t gpio_desc_set_direction(struct gpio_desc *desc)
{
    struct gpio_ops *ops = dev_to_gpio_ops(desc->dev);
    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    if (desc->flags & GPIO_DESC_OUTPUT) {
        int value = desc->flags & GPIO_DESC_OUTPUT_ACTIVE ? 1 : 0;

        if (desc->flags & GPIO_DESC_ACTIVE_LOW)
            value = !value;

        LTRACEF("GPIO ctrl %s: Pin %d: set direction to output, %s (%d)\n",
                desc->dev->name, desc->nr,
                desc->flags & GPIO_DESC_OUTPUT_ACTIVE ? "active" : "not active",
                value);

        return ops->direction_output(desc->dev, desc->nr, value);
    }

    if (desc->flags & GPIO_DESC_INPUT) {
        LTRACEF("GPIO ctrl %s: Pin %d: set direction to input.\n",
                                                    desc->dev->name, desc->nr);
        return ops->direction_input(desc->dev, desc->nr);
    }

    return ERR_INVALID_ARGS;
}

int gpio_desc_set_value(struct gpio_desc *desc, unsigned value)
{
    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    if (!dev_to_gpio_ops(desc->dev)->set_value)
        return ERR_NOT_FOUND;

    LTRACEF("GPIO ctrl %s: Pin %d: set value to %d.\n",
                                            desc->dev->name, desc->nr, value);

    if (desc->flags & GPIO_DESC_ACTIVE_LOW)
        value = !value;
    return dev_to_gpio_ops(desc->dev)->set_value(desc->dev, desc->nr, value);
}

int gpio_desc_get_value(struct gpio_desc *desc)
{

    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    int value;

    if (!dev_to_gpio_ops(desc->dev)->get_value)
        return ERR_NOT_FOUND;

    LTRACEF("GPIO ctrl %s: Pin %d: get value.\n", desc->dev->name, desc->nr);

    value = dev_to_gpio_ops(desc->dev)->get_value(desc->dev, desc->nr);
    if (value == -1)
        return ERR_NOT_VALID;

    if (desc->flags & GPIO_DESC_ACTIVE_LOW)
        value = !value;

    return value;
}


int gpio_desc_set_open_drain(struct gpio_desc *desc, unsigned value)
{
    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    int ret;

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    LTRACEF("GPIO ctrl %s: Pin %d: Set open drain mode %d.\n",
                                            desc->dev->name, desc->nr, value);

    if (!dev_to_gpio_ops(desc->dev)->set_open_drain)
        return ERR_NOT_FOUND;

    ret = dev_to_gpio_ops(desc->dev)->set_open_drain(desc->dev, desc->nr, value);
    if (ret == -1)
        return ERR_NOT_VALID;

    return ret;
}

int gpio_desc_get_open_drain(struct gpio_desc *desc)
{
    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    int value;

    if (!dev_to_gpio_ops(desc->dev)->get_open_drain)
        return ERR_NOT_FOUND;

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }
    LTRACEF("GPIO ctrl %s: Pin %d: Get open drain mode.\n",
                                            desc->dev->name, desc->nr);

    value = dev_to_gpio_ops(desc->dev)->get_open_drain(desc->dev, desc->nr);
    if (value == -1)
        return ERR_NOT_VALID;

    return value;
}

status_t of_gpio_request_by_name(int node, const char *gpio_name,
                                            struct gpio_desc * desc, int flags)
{
    struct fdt_args out_args;

    int ret = of_get_args_from_phandle(node, gpio_name, "#gpio-cells", 0, &out_args);

    if (ret < 0) {
        TRACEF("Error while parsing handle and args for %s node\n", gpio_name);
        return ERR_INVALID_ARGS;
    }

    struct device *gpio_device = of_find_device_by_node(out_args.node);
    if (gpio_device == NULL) {
        TRACEF("No node found for gpio controller %s\n", gpio_name);
        return ERR_NOT_FOUND;
    }

    if (!of_device_get_bool(gpio_device, "gpio-controller")) {
        TRACEF("Node %s is not a gpio controller\n", gpio_name);
        return ERR_NOT_FOUND;
    }

    LTRACEF("GPIO device found: %s\n", gpio_device->name);
    ret = gpio_get_desc(gpio_device, desc, &out_args);
    ASSERT(desc->dev);

    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    LTRACEF("GPIO ctrl %s: Request pin %d with label %s.\n",
                                    desc->dev->name, desc->nr, gpio_name);

    dev_to_gpio_ops(gpio_device)->request(gpio_device, desc->nr, gpio_name);

    desc->flags |= flags;

    gpio_desc_set_direction(desc);

    return 0;
}

status_t gpio_request_by_name(struct device *dev, const char *gpio_name,
                                            struct gpio_desc * desc, int flags)
{
    return of_gpio_request_by_name(dev->node_offset, gpio_name, desc, flags);
}

status_t gpio_free(struct device *dev, struct gpio_desc *desc)
{
    struct gpio_controller *ctrl = desc->dev->state;
    ASSERT(ctrl);

    if (desc->nr > ctrl->count) {
        TRACEF("Offset %d out of range (>%d)\n", desc->nr, ctrl->count);
        return ERR_INVALID_ARGS;
    }

    LTRACEF("GPIO ctrl %s: free pin %d\n", desc->dev->name, desc->nr);

    return dev_to_gpio_ops(desc->dev)->free(desc->dev, desc->nr);
}
