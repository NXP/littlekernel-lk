LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += dev/interrupt/arm_gic/include

ifeq ($(WITH_DEV_INTERRUPT_ARM_GIC_V3),)
MODULE_SRCS += \
	$(LOCAL_DIR)/arm_gic.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/empty.c

MODULE_DEPS += \
	dev/pdev/interrupt \
	dev/interrupt/arm_gic/v3
endif

include make/module.mk
