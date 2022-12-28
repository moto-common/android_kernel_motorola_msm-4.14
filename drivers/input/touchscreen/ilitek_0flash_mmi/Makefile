ifneq ($(filter m y,$(CONFIG_INPUT_ILI_0FLASH_MMI_ENABLE_DOUBLE_TAP)),)
	EXTRA_CFLAGS += -DILI_SENSOR_EN
endif
ifneq ($(filter m y,$(CONFIG_INPUT_ILI_PANEL_NOTIFICATIONS)),)
EXTRA_CFLAGS += -DILI_CONFIG_PANEL_NOTIFICATIONS
endif
ifneq ($(filter m y,$(CONFIG_ILITEK_RESUME_BY_DDI)),)
EXTRA_CFLAGS += -DILI_CONFIG_RESUME_BY_DDI
endif

ilitek_0flash_mmi-objs += ili9881x.o
ilitek_0flash_mmi-objs += ili9881x_spi.o
ilitek_0flash_mmi-objs += ili9881x_qcom.o
ilitek_0flash_mmi-objs += ili9881x_ic.o
ilitek_0flash_mmi-objs += ili9881x_touch.o
ilitek_0flash_mmi-objs += ili9881x_mp.o
ilitek_0flash_mmi-objs += ili9881x_hostdl.o
ilitek_0flash_mmi-objs += ili9881x_node.o

obj-$(CONFIG_TOUCHSCREEN_ILITEK_CHIPSET_0FLASH) += ilitek_0flash_mmi.o
