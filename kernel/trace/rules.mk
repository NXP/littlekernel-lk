LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/tracepoint.c

GLOBAL_DEFINES += \
	WITH_KERNEL_TRACEPOINT=1

include make/module.mk
