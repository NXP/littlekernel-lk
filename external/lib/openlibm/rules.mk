USE_OPENLIBM=true
LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

SUBDIRS = src bsdsrc

ifneq ($(filter $(ARCH),x86),)
 SUBDIRS += ld80
 MODULE_INCLUDES := $(LOCAL_DIR)/ld80
 ifeq ($(SUBARCH),x86-32)
  _ARCH := i387
 else
  _ARCH := amd64
 endif
else
 ifneq ($(filter $(ARCH),arm64),)
  SUBDIRS += ld128
  MODULE_INCLUDES := $(LOCAL_DIR)/ld128
  _ARCH := aarch64
 else
  _ARCH := $(ARCH)
 endif
endif

SUBDIRS += $(_ARCH)

define INC_template
TEST=test
override CUR_SRCS = $(1)_SRCS
include $(1)/Make.files
SRCS += $$(addprefix $(1)/,$$($(1)_SRCS))
endef

__ARCH := $(ARCH)
ARCH := $(_ARCH)
$(foreach dir,$(SUBDIRS),$(eval $(call INC_template,$(addprefix $(LOCAL_DIR)/,$(dir)))))
ARCH := $(__ARCH)

DUPLICATE_NAMES = $(filter $(patsubst %.S,%,$($(_ARCH)_SRCS)),$(patsubst %.c,%,$(src_SRCS)))
DUPLICATE_SRCS = $(addsuffix .c,$(DUPLICATE_NAMES))

MODULE_SRCS := $(filter-out $(addprefix src/,$(DUPLICATE_SRCS)),$(SRCS))
MODULE_INCLUDES += \
    $(LOCAL_DIR) \
    $(LOCAL_DIR)/include \
    $(LOCAL_DIR)/$(_ARCH) \
    $(LOCAL_DIR)/src

MODULE_CFLAGS := \
                    -fPIC\
                    -std=c99 \
                    -Wall \
                    -DASSEMBLER \
                    -D__BSD_VISIBLE \
                    -Wno-implicit-function-declaration \
                    -Wno-parentheses \
                    -Wno-sign-compare \
                    -Wno-maybe-uninitialized \
                    -fno-gnu89-inline -fno-builtin

GLOBAL_INCLUDES += $(LOCAL_DIR)/src

include make/module.mk