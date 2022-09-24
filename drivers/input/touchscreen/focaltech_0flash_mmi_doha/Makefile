focaltech_0flash_mmi_doha-y += focaltech_core.o
focaltech_0flash_mmi_doha-y += focaltech_esdcheck.o
focaltech_0flash_mmi_doha-y += focaltech_ex_fun.o
focaltech_0flash_mmi_doha-y += focaltech_ex_mode.o
focaltech_0flash_mmi_doha-y += focaltech_flash.o
focaltech_0flash_mmi_doha-y += focaltech_gesture.o
focaltech_0flash_mmi_doha-y += focaltech_point_report_check.o

focaltech_0flash_mmi_doha-y += focaltech_spi_v1_1.o
focaltech_0flash_mmi_doha-y += focaltech_flash_ft8719.o

ifneq ($(filter m y,$(CONFIG_TOUCHSCREEN_FTS_0FLASH_MMI_DOHA_ENABLE_DOUBLE_TAP)),)
	ccflags-y += -DFOCALTECH_SENSOR_EN
endif

obj-$(CONFIG_TOUCHSCREEN_FTS_0FLASH_MMI_DOHA) += focaltech_0flash_mmi_doha.o
