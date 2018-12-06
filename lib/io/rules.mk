LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS := \
	lib/cbuf

MODULE_SRCS += \
   $(LOCAL_DIR)/console.c \
   $(LOCAL_DIR)/io.c \

ifeq ($(WITH_DEBUG_LINEBUFFER), true)
GLOBAL_DEFINES += WITH_DEBUG_LINEBUFFER=1
GLOBAL_CFLAGS += -fno-builtin-printf
endif

include make/module.mk
