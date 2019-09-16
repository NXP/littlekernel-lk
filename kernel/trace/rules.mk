LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/tracepoint.c \
	$(LOCAL_DIR)/tracelog.c \
	$(LOCAL_DIR)/probes.c \
	$(LOCAL_DIR)/tracelog_str.c \
	$(LOCAL_DIR)/tracelog_kernel.c \
	$(LOCAL_DIR)/tracelog_bin.c

GLOBAL_DEFINES += \
	WITH_KERNEL_TRACEPOINT=1

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/traces.ld

include make/module.mk
