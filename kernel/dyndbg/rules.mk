LOCAL_DIR := $(GET_LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE := $(LOCAL_DIR)
MODULE_SRCS += \
			   $(LOCAL_DIR)/dyndbg_cmd.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/dyndbg.ld

include make/module.mk
