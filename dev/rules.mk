LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dev.c \
	$(LOCAL_DIR)/driver.c \
	$(LOCAL_DIR)/class/block_api.c \
	$(LOCAL_DIR)/class/i2c_api.c \
	$(LOCAL_DIR)/class/spi_api.c \
	$(LOCAL_DIR)/class/sai_api.c \
	$(LOCAL_DIR)/class/spdif_api.c \
	$(LOCAL_DIR)/class/gpio_api.c \
	$(LOCAL_DIR)/class/uart_api.c \
	$(LOCAL_DIR)/class/fb_api.c \
	$(LOCAL_DIR)/class/netif_api.c \
	$(LOCAL_DIR)/class/gpt_api.c \
	$(LOCAL_DIR)/class/hdmi_api.c \

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/devices.ld $(LOCAL_DIR)/drivers.ld

include make/module.mk
