#
# SPDX-FileCopyrightText: 2019 The Android Open-Source Project
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-FileCopyrightText: The Calyx Institute
# SPDX-License-Identifier: Apache-2.0
#

include build/make/target/board/BoardConfigMainlineCommon.mk
include build/make/target/board/BoardConfigPixelCommon.mk

TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-2a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a55

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := generic
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a53

BOARD_BOOTCONFIG += \
    androidboot.load_modules_parallel=true \
    androidboot.boot_devices=14700000.ufs

BOARD_KERNEL_CMDLINE += \
    fips140.load_sequential=1 \
    exynos_mfc.load_sequential=1 \
    exynos_drm.load_sequential=1 \
    pcie-exynos-core.load_sequential=1 \
    g2d.load_sequential=1 \
    disable_dma32=on \
    dyndbg=\"func alloc_contig_dump_pages +p\" \
    earlycon=exynos4210,0x10A00000 \
    console=ttySAC0,115200 \
    androidboot.console=ttySAC0 \
    printk.devkmsg=on \
    cma_sysfs.experimental=Y \
    rcupdate.rcu_expedited=1 \
    rcu_nocbs=all \
    rcutree.enable_rcu_lazy \
    swiotlb=noforce \
    cgroup.memory=nokmem \
    rodata=on \
    at24.write_timeout=100 \
    log_buf_len=1024K \
    android_arch_task_struct_size=512

TARGET_NO_BOOTLOADER := true
BOARD_PREBUILT_BOOTIMAGE := $(wildcard $(TARGET_KERNEL_DIR)/boot.img)
TARGET_NO_KERNEL := true
BOARD_USES_GENERIC_KERNEL_IMAGE := true
BOARD_MOVE_RECOVERY_RESOURCES_TO_VENDOR_BOOT := true
BOARD_MOVE_GSI_AVB_KEYS_TO_VENDOR_BOOT := true
TARGET_RECOVERY_WIPE := device/google/gs101/conf/recovery.wipe

# This is the fstab file that will be included in the recovery image.  Note that
# recovery doesn't care about the encryption settings, so it doesn't matter
# whether we use the normal or the fips fstab here.
TARGET_RECOVERY_FSTAB_GENRULE = gen_fstab.gs101

TARGET_RECOVERY_PIXEL_FORMAT := ABGR_8888
TARGET_RECOVERY_UI_MARGIN_HEIGHT := 165
TARGET_RECOVERY_UI_LIB := \
	//hardware/google/pixel/recovery:librecovery_ui_pixel \
	libfstab

AB_OTA_UPDATER := true

AB_OTA_PARTITIONS += \
    system \
    system_ext \
    product \
    vbmeta_system \
    vbmeta_vendor \
    vendor \
    vendor_dlkm

ifneq ($(PRODUCT_BUILD_BOOT_IMAGE),false)
AB_OTA_PARTITIONS += boot
endif
ifneq ($(PRODUCT_BUILD_VENDOR_BOOT_IMAGE),false)
AB_OTA_PARTITIONS += vendor_boot
AB_OTA_PARTITIONS += dtbo
endif
ifneq ($(PRODUCT_BUILD_VBMETA_IMAGE),false)
AB_OTA_PARTITIONS += vbmeta
endif
ifneq ($(PRODUCT_BUILD_PVMFW_IMAGE),false)
AB_OTA_PARTITIONS += pvmfw
endif

ifneq (,$(AVB_CUSTOM_KEY_PATH))
BOARD_AVB_ALGORITHM := $(AVB_CUSTOM_ALGORITHM)
BOARD_AVB_KEY_PATH := $(AVB_CUSTOM_KEY_PATH)
else
AVB_CUSTOM_ALGORITHM := SHA256_RSA2048
AVB_CUSTOM_KEY_PATH := external/avb/test/data/testkey_rsa2048.pem
endif

# Enable chain partition for system.
BOARD_AVB_VBMETA_SYSTEM := system system_ext product
BOARD_AVB_VBMETA_SYSTEM_ALGORITHM := $(AVB_CUSTOM_ALGORITHM)
BOARD_AVB_VBMETA_SYSTEM_KEY_PATH := $(AVB_CUSTOM_KEY_PATH)
BOARD_AVB_VBMETA_SYSTEM_ROLLBACK_INDEX := $(PLATFORM_SECURITY_PATCH_TIMESTAMP)
BOARD_AVB_VBMETA_SYSTEM_ROLLBACK_INDEX_LOCATION := 1

ifneq ($(PRODUCT_BUILD_PVMFW_IMAGE),false)
BOARD_AVB_VBMETA_SYSTEM += pvmfw
endif

# Enable chained vbmeta for boot images
BOARD_AVB_BOOT_ALGORITHM := $(AVB_CUSTOM_ALGORITHM)
BOARD_AVB_BOOT_KEY_PATH := $(AVB_CUSTOM_KEY_PATH)
BOARD_AVB_BOOT_ROLLBACK_INDEX := $(PLATFORM_SECURITY_PATCH_TIMESTAMP)
BOARD_AVB_BOOT_ROLLBACK_INDEX_LOCATION := 2

# Enable chain partition for vendor.
BOARD_AVB_VBMETA_VENDOR := vendor
BOARD_AVB_VBMETA_VENDOR_ALGORITHM := $(AVB_CUSTOM_ALGORITHM)
BOARD_AVB_VBMETA_VENDOR_KEY_PATH := $(AVB_CUSTOM_KEY_PATH)
BOARD_AVB_VBMETA_VENDOR_ROLLBACK_INDEX := $(PLATFORM_SECURITY_PATCH_TIMESTAMP)
BOARD_AVB_VBMETA_VENDOR_ROLLBACK_INDEX_LOCATION := 3

# Verified Boot
ifneq ($(WITH_AVB),true)
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS += --flags 3
endif

TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
BOARD_USERDATAIMAGE_PARTITION_SIZE := 11796480000
PRODUCT_FS_COMPRESSION := 1
BOARD_FLASH_BLOCK_SIZE := 4096

# product.img
BOARD_PRODUCTIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_PRODUCT := product

# system_ext.img
BOARD_SYSTEM_EXTIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_SYSTEM_EXT := system_ext

# vendor.img
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4

# vendor_dlkm.img
BOARD_VENDOR_DLKMIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_VENDOR_DLKM := vendor_dlkm

BOARD_SUPER_PARTITION_SIZE := 8531214336
BOARD_SUPER_PARTITION_GROUPS := google_dynamic_partitions
# Set size to BOARD_SUPER_PARTITION_SIZE - overhead (4MiB) (b/182237294)
BOARD_GOOGLE_DYNAMIC_PARTITIONS_SIZE := 8527020032
BOARD_GOOGLE_DYNAMIC_PARTITIONS_PARTITION_LIST := \
    system \
    system_ext \
    product \
    vendor \
    vendor_dlkm

# Set error limit to BOARD_SUPER_PARTITON_SIZE - 400MB
BOARD_SUPER_PARTITION_ERROR_LIMIT := 8111783936

# Reserve space for gapps install
-include vendor/lineage/config/BoardConfigReservedSize.mk

# Device Tree
BOARD_INCLUDE_DTB_IN_BOOTIMG := true
BOARD_PREBUILT_DTBIMAGE_DIR := $(TARGET_KERNEL_DIR)
BOARD_PREBUILT_DTBOIMAGE := $(BOARD_PREBUILT_DTBIMAGE_DIR)/dtbo.img

BOARD_USES_METADATA_PARTITION := true

# exynos RIL
ENABLE_VENDOR_RIL_SERVICE := true

# Boot.img
BOARD_RAMDISK_USE_LZ4     := true
BOARD_BOOT_HEADER_VERSION := 4
BOARD_MKBOOTIMG_ARGS += --header_version $(BOARD_BOOT_HEADER_VERSION)

BOARD_VENDOR_RAMDISK_FRAGMENTS := dlkm
BOARD_VENDOR_RAMDISK_FRAGMENT.dlkm.KERNEL_MODULE_DIRS := top

# Enable AVB2.0
BOARD_AVB_ENABLE := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 0x04000000
BOARD_VENDOR_BOOTIMAGE_PARTITION_SIZE := 0x04000000
BOARD_DTBOIMG_PARTITION_SIZE := 0x01000000

# Vendor ramdisk image for kernel development
BOARD_BUILD_VENDOR_RAMDISK_IMAGE := true

KERNEL_MODULE_DIR := $(TARGET_KERNEL_DIR)
KERNEL_MODULES := $(wildcard $(KERNEL_MODULE_DIR)/*.ko)

BOARD_VENDOR_KERNEL_MODULES_BLOCKLIST_FILE := $(KERNEL_MODULE_DIR)/vendor_dlkm.modules.blocklist

# Since Pixel 6/6pro doesn't have a system_dlkm partition, the GKI modules are
# on the vendor_dlkm partition. In order to allow them to load properly, we
# need to retain the module signature which would normally get stripped during
# packaging. Disable stripping the vendor_dlkm modules to retain the GKI
# modules' signature. Note, the pixel kernel builds always strip the modules in
# favor of saving space via the kleaf property: strip_modules = True.
BOARD_DO_NOT_STRIP_VENDOR_MODULES := true

# Prebuilt kernel modules that are *not* listed in vendor_boot.modules.load
BOARD_PREBUILT_VENDOR_RAMDISK_KERNEL_MODULES = fips140.ko
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_EXTRA = $(foreach k,$(BOARD_PREBUILT_VENDOR_RAMDISK_KERNEL_MODULES),$(if $(wildcard $(KERNEL_MODULE_DIR)/$(k)), $(k)))

# Kernel modules that are listed in vendor_boot.modules.load
# Starting from 6.1, use modules.load instead. It lists modules for vendor ramdisk regardless of the partition name.
ifneq ($(wildcard $(KERNEL_MODULE_DIR)/modules.load),)
    BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_FILE := $(strip $(shell cat $(KERNEL_MODULE_DIR)/modules.load))
else
    BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_FILE := $(strip $(shell cat $(KERNEL_MODULE_DIR)/vendor_boot.modules.load))
endif
ifndef BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_FILE
$(error vendor_boot.modules.load not found or empty)
endif
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD := $(BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_EXTRA)
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD += $(BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_FILE)
BOARD_VENDOR_RAMDISK_KERNEL_MODULES := $(addprefix $(KERNEL_MODULE_DIR)/, $(BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_EXTRA))
BOARD_VENDOR_RAMDISK_KERNEL_MODULES += $(addprefix $(KERNEL_MODULE_DIR)/, $(notdir $(BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD_FILE)))

BOARD_VENDOR_KERNEL_MODULES_LOAD += $(strip $(shell cat $(KERNEL_MODULE_DIR)/vendor_dlkm.modules.load))
ifndef BOARD_VENDOR_KERNEL_MODULES_LOAD
$(error vendor_dlkm.modules.load not found or empty)
endif
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES)

# SEPolicy
BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/google/pixel-sepolicy/googlebattery \
    hardware/google/pixel-sepolicy/input \
    hardware/google/pixel-sepolicy/powerstats \
    device/google/gs101/sepolicy/certificates \
    device/google/gs101/sepolicy/recovery \
    device/google/gs101/sepolicy/vendor

PRODUCT_PRIVATE_SEPOLICY_DIRS += \
    device/google/gs101/sepolicy/product/private

PRODUCT_PUBLIC_SEPOLICY_DIRS += \
    device/google/gs101/sepolicy/product/public

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    device/google/gs101/sepolicy/system_ext/private

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    device/google/gs101/sepolicy/system_ext/public

# Protected VM firmware
BOARD_PVMFWIMAGE_PARTITION_SIZE := 0x00100000
