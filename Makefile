# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2022-present, Ukama Inc.

include ../config.mk 

CUR_MAKE := $(abspath $(firstword $(MAKEFILE_LIST)))
CUR_PATH := $(dir $(CUR_MAKE))

#Source directory
SRC_DIR = linux

ROOTFS_KPATH=$(CUR_PATH)/build

# Config for Builds
ANODE_KCONFIG = ukama_anode_defconfig
CNODE_KCONFIG = comNode_lk_defconfig

# Detect target board and set appropriate variables
ifeq ($(AMPLIFIER_NODE), $(TARGET_BOARD))
	override CC   = arm-linux-gnueabihf-
	override HOST = arm-linux-gnueabihf
	override ARCH = $(ARCH_ARM)
	SRC_DIRS      = $(AMPLIFIER_TARGET)
	KCONFIG       = $(ANODE_KCONFIG) 
	KIMAGE        = zImage
	OS_ARTIFACTS  = $(KIMAGE) modules dtbs
endif

ifeq ($(TOWER_NODE), $(TARGET_BOARD))
	override ARCH   = $(ARCH_X86_64)
	override HOST   = x86_64-linux-musl
	KCONFIG         = $(CNODE_KCONFIG)
	KIMAGE          = bzImage
        OS_ARTIFACTS    = $(KIMAGE) modules
endif

ifeq ($(ACCESS_NODE), $(TARGET_BOARD))
	override CC     = aarch64-linux-gnu-gcc
	override HOST   = aarch64-linux-gnu
	override ARCH   = arm64
endif

ifeq ($(LOCAL), $(TARGET_BOARD))
	override CC     = gcc
	override ARCH   = $(ARCH_X86_64)
	override HOST   = $(shell gcc -dumpmachine)
        KCONFIG         = $(CNODE_KCONFIG)
	KIMAGE          = bzImage
	OS_ARTIFACTS    = $(KIMAGE) modules
endif

.PHONY: $(SRC_DIR) info clean distclean


#Kernel Image
$(OS_ARTIFACTS): info
	@echo Building $@
	$(MAKE) -j$(NPROCS) -C $(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CC) $(KCONFIG)
	$(MAKE) -j$(NPROCS) -C $(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CC) $(OS_ARTIFACTS)
	#Copy Kernel Image
	@echo Copying Kernel Image $(SRC_DIR)/arch/${ARCH}/boot/$(KIMAGE)
	(mkdir -p $(ROOTFS_KPATH) && cp -v $(SRC_DIR)/arch/${ARCH}/boot/$(KIMAGE) $(ROOTFS_KPATH)/$(KIMAGE))
	#Install Modules
	$(MAKE) -j$(NPROCS) -C $(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CC) INSTALL_MOD_PATH=$(ROOTFS_KPATH) modules_install
ifeq ($(ARCH), $(ARCHARM))
	#Install DTBS
	$(MAKE) -j$(NPROCS) -C $(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CC) INSTALL_DTBS_PATH=$(ROOTFS_KPATH)/boot dtbs_install
endif

clean :
	rm -rf $(ROOTFS_KPATH);
	for dir in $(SRC_DIR); do \
                $(MAKE) -j$(NPROCS) -C $$dir -f Makefile $@; \
        done

distclean :
	rm -rf $(ROOTFS_KPATH);
	for dir in $(SRC_DIR); do \
                $(MAKE) -j$(NPROCS) -C $$dir -f Makefile $@; \
        done

info:
	@echo "================================="
	@echo " Building Info "
	@echo "---------------------------------"
	@echo " Target Board  : $(TARGET_BOARD)"
	@echo " Source Dir   : $(SRC_DIR)"
	@echo " Architecture  : $(ARCH)"
	@echo " Compiler      : $(CC)"
	@echo " Config        : $(KCONFIG)"
	@echo " OS Artifacts  : $(OS_ARTIFACTS)"
	@echo "================================="
