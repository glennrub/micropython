MCU_SERIES = m4
MCU_VARIANT = nrf52
MCU_SUB_VARIANT = nrf52840
#SOFTDEV_VERSION = 6.1.1
LD_FILES += boards/nrf52840_1M_256k.ld

DFU ?= 1

ifeq ($(DFU),1)
BOOTLOADER=mcuboot
BOOTLOADER_VERSION=1.0
FLASHER=mcumgr
NRF_DEFINES += -DBOOTLOADER_VTOR=0x12200 -DBOOTLOADER_MCUBOOT_VERSION=10
endif

NRF_DEFINES += -DNRF52840_XXAA
