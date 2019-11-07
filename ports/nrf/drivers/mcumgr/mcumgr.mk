
define MCUBOOT_HOME_MISSING_ERROR


###### ERROR: MCUBOOT home directory not found ############
#                                                         #
# The build target requires an mcuboot home to locate     #
# scripts and security certificates to sign the firmware  #
#                                                         #
# Please point MCUBOOT_HOME to correct path, or run the   #
# download script to fetch a local version:               #
#                                                         #
#      drivers/mcumgr/download_mcuboot.sh                 #
#                                                         #
###########################################################

endef

MCUBOOT_HOME ?= "drivers/mcumgr/mcuboot"

MCUBOOT_PEM = $(MCUBOOT_HOME)/root-rsa-2048.pem
MCUBOOT_IMGTOOL_PY = $(MCUBOOT_HOME)/scripts/imgtool.py

ifeq ($(shell test ! -e $(MCUBOOT_PEM) && echo -n no),no)
    $(error $(MCUBOOT_HOME_MISSING_ERROR))
endif

ifeq ($(shell test ! -e $(MCUBOOT_IMGTOOL_PY) && echo -n no),no)
    $(error $(MCUBOOT_HOME_MISSING_ERROR))
endif
