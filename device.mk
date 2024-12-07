#
# SPDX-FileCopyrightText: 2011 The Android Open-Source Project
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-FileCopyrightText: The Calyx Institute
# SPDX-License-Identifier: Apache-2.0
#

# Disable OMX
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.media.omx=0

# Installs gsi keys into ramdisk, to boot a developer GSI with verified boot.
$(call inherit-product, $(SRC_TARGET_DIR)/product/developer_gsi_keys.mk)

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.ipsec_tunnel_migration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnel_migration.xml

# sscoredump
PRODUCT_PROPERTY_OVERRIDES += vendor.debug.ssrdump.type=sscoredump

# Modem
PRODUCT_PACKAGES += dump_modem

# Thermal
PRODUCT_PACKAGES += android.hardware.thermal-service.pixel

# Thermal utils
PRODUCT_PACKAGES += thermal_symlinks

# Ensure enough free space to create zram backing device
PRODUCT_PRODUCT_PROPERTIES += \
    ro.zram_backing_device_min_free_mb=1536

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm-service.clearkey

# misc_writer
PRODUCT_PACKAGES += \
    misc_writer

# Boot control
PRODUCT_PACKAGES += \
    android.hardware.boot-service.default-pixel \
    android.hardware.boot-service.default_recovery-pixel

PRODUCT_SOONG_NAMESPACES += device/google/gs-common/bootctrl/aidl

TARGET_BOARD_PLATFORM := gs101

AB_OTA_POSTINSTALL_CONFIG += \
	RUN_POSTINSTALL_system=true \
	POSTINSTALL_PATH_system=system/bin/otapreopt_script \
	FILESYSTEM_TYPE_system=ext4 \
POSTINSTALL_OPTIONAL_system=true

PRODUCT_SOONG_NAMESPACES += \
	hardware/google/av \
	hardware/google/interfaces \
	hardware/google/pixel \
	device/google/gs101

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	ro.oem_unlock_supported=1

# From system.property
PRODUCT_PROPERTY_OVERRIDES += \
	ro.telephony.default_network=27 \
	persist.vendor.ril.db_ecc.use.iccid_to_plmn=1 \
	persist.vendor.ril.db_ecc.id.type=5

# SIT-RIL Logging setting
PRODUCT_PROPERTY_OVERRIDES += \
	persist.vendor.ril.log_mask=3 \
	persist.vendor.ril.log.base_dir=/data/vendor/radio/sit-ril \
	persist.vendor.ril.log.chunk_size=5242880 \
	persist.vendor.ril.log.num_file=3

# Enable reboot free DSDS
PRODUCT_PRODUCT_PROPERTIES += \
	persist.radio.reboot_on_modem_change=false

# Enable Early Camping
PRODUCT_PRODUCT_PROPERTIES += \
	persist.vendor.ril.camp_on_earlier=1

# Carrier configuration default location
PRODUCT_PROPERTY_OVERRIDES += \
	persist.vendor.radio.config.carrier_config_dir=/vendor/firmware/carrierconfig

# Set the Bluetooth Class of Device
# Service Field: 0x5A -> 90
#    Bit 17: Networking
#    Bit 19: Capturing
#    Bit 20: Object Transfer
#    Bit 22: Telephony
# MAJOR_CLASS: 0x02 -> 2 (Phone)
# MINOR_CLASS: 0x0C -> 12 (Smart Phone)
PRODUCT_PRODUCT_PROPERTIES += \
    bluetooth.device.class_of_device=90,2,12

# Set supported Bluetooth profiles to enabled
PRODUCT_PRODUCT_PROPERTIES += \
	bluetooth.profile.asha.central.enabled?=true \
	bluetooth.profile.a2dp.source.enabled?=true \
	bluetooth.profile.avrcp.target.enabled?=true \
	bluetooth.profile.bas.client.enabled?=true \
	bluetooth.profile.gatt.enabled?=true \
	bluetooth.profile.hfp.ag.enabled?=true \
	bluetooth.profile.hid.device.enabled?=true \
	bluetooth.profile.hid.host.enabled?=true \
	bluetooth.profile.map.server.enabled?=true \
	bluetooth.profile.opp.enabled?=true \
	bluetooth.profile.pan.nap.enabled?=true \
	bluetooth.profile.pan.panu.enabled?=true \
	bluetooth.profile.pbap.server.enabled?=true \
	bluetooth.profile.sap.server.enabled?=true \

PRODUCT_PROPERTY_OVERRIDES += \
	telephony.active_modems.max_count=2

# HWUI
TARGET_USES_VULKAN = true

# GPU
PRODUCT_PACKAGES += gpu_probe

# Install the OpenCL ICD Loader
PRODUCT_SOONG_NAMESPACES += external/OpenCL-ICD-Loader
PRODUCT_PACKAGES += \
	libOpenCL

PRODUCT_VENDOR_PROPERTIES += \
	ro.hardware.egl=mali \
	ro.hardware.vulkan=mali

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	debug.mali.disable_backend_affinity=true

# Mali Configuration Properties
PRODUCT_VENDOR_PROPERTIES += \
	vendor.mali.platform.config=/vendor/etc/mali/platform.config \
	vendor.mali.debug.config=/vendor/etc/mali/debug.config \
	vendor.mali.base_protected_max_core_count=4 \
	vendor.mali.base_protected_tls_max=67108864 \
	vendor.mali.platform_agt_frequency_khz=24576

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
	frameworks/native/data/etc/android.hardware.vulkan.version-1_3.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version.xml \
	frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level.xml \
	frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute.xml \
	frameworks/native/data/etc/android.software.vulkan.deqp.level-2025-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml \
	frameworks/native/data/etc/android.software.opengles.deqp.level-2025-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml

# Configure EGL blobcache
PRODUCT_VENDOR_PROPERTIES += \
       ro.egl.blobcache.multifile=true \
       ro.egl.blobcache.multifile_limit=33554432 \

PRODUCT_VENDOR_PROPERTIES += \
	ro.opengles.version=196610 \
	graphics.gpu.profiler.support=true

# b/295257834 Add HDR shaders to SurfaceFlinger's pre-warming cache
PRODUCT_VENDOR_PROPERTIES += ro.surface_flinger.prime_shader_cache.ultrahdr=1

DEVICE_PACKAGE_OVERLAYS += device/google/gs101/overlay
DEVICE_PACKAGE_OVERLAYS += device/google/gs101/overlay-lineage

# Enforce the Product interface
PRODUCT_PRODUCT_VNDK_VERSION := current
PRODUCT_ENFORCE_PRODUCT_PARTITION_INTERFACE := true

# Recovery files
PRODUCT_COPY_FILES += \
	device/google/gs101/conf/init.recovery.device.rc:$(TARGET_COPY_OUT_RECOVERY)/root/init.recovery.gs101.rc

# Fstab files
PRODUCT_PACKAGES += \
	fstab.gs101 \
	fstab.gs101.vendor_ramdisk \
	fstab.gs101-fips \
	fstab.gs101-fips.vendor_ramdisk

# Insmod config files
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,init.insmod.*.cfg,$(TARGET_KERNEL_DIR),$(TARGET_COPY_OUT_VENDOR_DLKM)/etc)

# For creating dtbo image
PRODUCT_HOST_PACKAGES += \
	mkdtimg

# CHRE
## hal
PRODUCT_PACKAGES += android.hardware.contexthub-service.generic
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.context_hub.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.context_hub.xml

# Userdata Checkpointing OTA GC
PRODUCT_PACKAGES += \
	checkpoint_gc

# Vendor verbose logging default property
PRODUCT_PROPERTY_OVERRIDES += \
	persist.vendor.verbose_logging_enabled=false

# CP Logging properties
PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.sys.modem.logging.loc = /data/vendor/slog \
	persist.vendor.sys.silentlog.tcp = "On" \
	ro.vendor.cbd.modem_removable = "1" \
	ro.vendor.cbd.modem_type = "s5100sit" \
	persist.vendor.sys.modem.logging.br_num=5 \
	persist.vendor.sys.modem.logging.enable=true

# Enable silent CP crash handling
PRODUCT_PROPERTY_OVERRIDES += \
	persist.vendor.ril.crash_handling_mode=2

# Add support dual SIM mode
PRODUCT_PROPERTY_OVERRIDES += \
	persist.vendor.radio.multisim_switch_support=true

# Touch
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

# Sensors
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
	frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.barometer.xml \
	frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
	frameworks/native/data/etc/android.hardware.sensor.dynamic.head_tracker.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.dynamic.head_tracker.xml \
	frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
	frameworks/native/data/etc/android.hardware.sensor.hifi_sensors.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.hifi_sensors.xml \
	frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml\
	frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
	frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
	frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml

# Add sensor HAL AIDL product packages
PRODUCT_PACKAGES += android.hardware.sensors-service.multihal

# USB HAL
PRODUCT_PACKAGES += \
	android.hardware.usb-service.gs101
PRODUCT_PACKAGES += \
	android.hardware.usb.gadget-service.gs101

# MIDI feature
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml

-include hardware/google/pixel/power-libperfmgr/aidl/device.mk

# IRQ rebalancing.
include hardware/google/pixel/rebalance_interrupts/rebalance_interrupts.mk

# PowerStats HAL
PRODUCT_PACKAGES += \
	android.hardware.power.stats-service.pixel

#
# Audio HALs
#

# Enable AAudio MMAP/NOIRQ data path.
PRODUCT_PROPERTY_OVERRIDES += aaudio.mmap_policy=2
PRODUCT_PROPERTY_OVERRIDES += aaudio.mmap_exclusive_policy=2
PRODUCT_PROPERTY_OVERRIDES += aaudio.hw_burst_min_usec=2000

# Camera
PRODUCT_SOONG_NAMESPACES += \
    hardware/google/camera

# WiFi
PRODUCT_PACKAGES += \
	WifiOverlay

# Connectivity
PRODUCT_PACKAGES += \
        ConnectivityOverlay

# Storage health HAL
PRODUCT_PACKAGES += \
	android.hardware.health.storage-service.default

# Battery Mitigation
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.battery_mitigation.aidl.enable=true

# storage pixelstats
-include hardware/google/pixel/pixelstats/device.mk

# Enable project quotas and casefolding for emulated storage without sdcardfs
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

$(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/launch_with_vendor_ramdisk.mk)

# Enable usage of io_uring in fastbootd
PRODUCT_VENDOR_PROPERTIES += sys.usb.ffs.io_uring_enabled=true

# Enforce generic ramdisk allow list
$(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
	frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
	frameworks/native/data/etc/android.hardware.wifi.rtt.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.rtt.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.camera.concurrent.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.concurrent.xml \
	frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml\
	frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml\

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
	frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.xml \

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \

PRODUCT_PROPERTY_OVERRIDES += \
	debug.slsi_platform=1 \
	debug.hwc.winupdate=1

PRODUCT_PROPERTY_OVERRIDES += \
	debug.sf.disable_backpressure=0 \
	debug.sf.enable_gl_backpressure=1

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.use_phase_offsets_as_durations=1
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.late.sf.duration=10500000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.late.app.duration=16600000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.early.sf.duration=16600000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.early.app.duration=16600000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.earlyGl.sf.duration=16600000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.earlyGl.app.duration=16600000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.frame_rate_multiple_threshold=120
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.layer_caching_active_layer_timeout_ms=1000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.treat_170m_as_sRGB=1

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.enable_layer_caching=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.set_idle_timer_ms?=80
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.set_touch_timer_ms=200
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.set_display_power_timer_ms=1000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.use_content_detection_for_refresh_rate=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.max_frame_buffer_acquired_buffers=3

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.supports_background_blur=1
PRODUCT_SYSTEM_PROPERTIES += ro.launcher.blur.appLaunch=0

# Must align with HAL types Dataspace
# The data space of wide color gamut composition preference is Dataspace::DISPLAY_P3
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.wcg_composition_dataspace=143261696

# Display
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.has_wide_color_display=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.has_HDR_display=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.use_color_management=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.protected_contents=true
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.surface_flinger.display_update_imminent_timeout_ms=50

PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.sf.native_mode=2

# limit DPP downscale ratio
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += vendor.hwc.dpp.downscale=2

# Cannot reference variables defined in BoardConfig.mk, uncomment this if
# BOARD_USES_EXYNOS_AFBC_FEATURE is true
# set the dss enable status setup
PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.ddk.set.afbc=1

PRODUCT_CHARACTERISTICS := nosdcard

PRODUCT_PACKAGES += hostapd
PRODUCT_PACKAGES += wpa_supplicant
PRODUCT_PACKAGES += wpa_supplicant.conf

WIFI_PRIV_CMD_UPDATE_MBO_CELL_STATUS := enabled

# Video
PRODUCT_PROPERTY_OVERRIDES += \
    debug.c2.use_dmabufheaps=1 \
    media.c2.dmabuf.padding=512 \
    debug.stagefright.ccodec_delayed_params=1 \
    ro.vendor.gpu.dataspace=1

PRODUCT_PROPERTY_OVERRIDES += \
        debug.stagefright.c2-poolmask=1507328

# Create input surface on the framework side
PRODUCT_PROPERTY_OVERRIDES += \
	debug.stagefright.c2inputsurface=-1 \

PRODUCT_PROPERTY_OVERRIDES += media.c2.hal.selection=aidl

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

PRODUCT_TAGS += dalvik.gc.type-precise

# Location
PRODUCT_PACKAGES += \
    android.hardware.location.gps.prebuilt.xml

# Trusty (KM, GK, Storage)
$(call inherit-product, system/core/trusty/trusty-storage.mk)
$(call inherit-product, system/core/trusty/trusty-base.mk)

PRODUCT_PACKAGES += \
	android.hardware.graphics.composer@2.4-service

# Storage: for factory reset protection feature
PRODUCT_PROPERTY_OVERRIDES += \
	ro.frp.pst=/dev/block/by-name/frp

# System props to enable Bluetooth Quality Report (BQR) feature
PRODUCT_PRODUCT_PROPERTIES += \
	persist.bluetooth.bqr.event_mask=30 \
	persist.bluetooth.bqr.min_interval_ms=500

PRODUCT_ENFORCE_RRO_TARGETS := \
	framework-res

# Dynamic Partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# Use FUSE passthrough
PRODUCT_PRODUCT_PROPERTIES += \
	persist.sys.fuse.passthrough.enable=true

# Use /product/etc/fstab.postinstall to mount system_other
PRODUCT_PRODUCT_PROPERTIES += \
	ro.postinstall.fstab.prefix=/product

PRODUCT_COPY_FILES += \
	device/google/gs101/conf/fstab.postinstall:$(TARGET_COPY_OUT_PRODUCT)/etc/fstab.postinstall

# fastbootd
PRODUCT_PACKAGES += \
	android.hardware.fastboot@1.1-impl.pixel \
	fastbootd

#google iwlan
PRODUCT_PACKAGES += \
	Iwlan

PRODUCT_COPY_FILES += \
	device/google/gs101/default-permissions.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/default-permissions/default-permissions.xml \
	device/google/gs101/component-overrides.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sysconfig/component-overrides.xml

PRODUCT_PACKAGES += \
	android.hardware.health-service.gs101 \
	android.hardware.health-service.gs101_recovery \

# Audio HAL configurations
PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/hearing_aid_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/hearing_aid_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml

# Audio HAL Server & Default Implementations
PRODUCT_PACKAGES += \
    android.hardware.audio.service \
    android.hardware.audio@7.1-impl \
    android.hardware.audio.effect@7.0-impl \
    android.hardware.soundtrigger@2.3-impl \
    android.hardware.bluetooth.audio-impl

# Audio HAL libraries
PRODUCT_PACKAGES += \
    audio.usb.default \
    audio.usbv2.default \
    audio.bluetooth.default \
    audio.r_submix.default

# Audio properties
PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.vc_call_vol_steps=7 \
	ro.config.media_vol_steps=25 \
	ro.audio.monitorRotation = true \
	ro.audio.offload_wakelock=false

# vndservicemanager and vndservice no longer included in API 30+, however needed by vendor code.
# See b/148807371 for reference
PRODUCT_PACKAGES += vndservicemanager
PRODUCT_PACKAGES += vndservice

## Start packet router
PRODUCT_PROPERTY_OVERRIDES += vendor.pktrouter=1

# Thermal HAL
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.enable.thermal.genl=true

# EdgeTPU
# Tflite Darwinn delegate property
PRODUCT_VENDOR_PROPERTIES += vendor.edgetpu.tflite_delegate.force_disable_io_coherency=0

# Edgetpu CPU scheduler property
PRODUCT_VENDOR_PROPERTIES += vendor.edgetpu.cpu_scheduler.policy=FIFO
PRODUCT_VENDOR_PROPERTIES += vendor.edgetpu.cpu_scheduler.priority=99

# A/B support
PRODUCT_PACKAGES += \
	otapreopt_script \
	cppreopts.sh \
	update_engine \
	update_engine_sideload \
	update_verifier

# pKVM
$(call inherit-product, packages/modules/Virtualization/apex/product_packages.mk)
PRODUCT_BUILD_PVMFW_IMAGE := true

# Enable watchdog timeout loop breaker.
PRODUCT_PROPERTY_OVERRIDES += \
	framework_watchdog.fatal_window.second=600 \
	framework_watchdog.fatal_count=3

# Enable zygote critical window.
PRODUCT_PROPERTY_OVERRIDES += \
	zygote.critical_window.minute=10

# Suspend properties
# (b/171433995): Enable short suspend backoffs and reduce max backoff time
PRODUCT_PROPERTY_OVERRIDES += \
    suspend.short_suspend_threshold_millis=2000 \
    suspend.short_suspend_backoff_enabled=true \
    suspend.max_sleep_time_millis=40000

# Enable Incremental on the device
PRODUCT_PROPERTY_OVERRIDES += \
	ro.incremental.enable=true

# Project
include hardware/google/pixel/common/pixel-common-device.mk

# Wifi ext
include hardware/google/pixel/wifi_ext/device.mk

# Keymaster configuration
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.device_id_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.device_id_attestation.xml \
    frameworks/native/data/etc/android.hardware.device_unique_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.device_unique_attestation.xml

# Use 64-bit dex2oat for better dexopt time.
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dex2oat64.enabled=true

# Call deleteAllKeys if vold detects a factory reset
PRODUCT_VENDOR_PROPERTIES += ro.crypto.metadata_init_delete_all_keys.enabled=true

# Hardware Info Collection
include hardware/google/pixel/HardwareInfo/HardwareInfo.mk

# Allow longer timeout for incident report generation in bugreport
# Overriding in /product partition instead of /vendor intentionally,
# since it can't be overridden from /vendor.
PRODUCT_PRODUCT_PROPERTIES += \
	dumpstate.strict_run=false

# AiAi Config
PRODUCT_COPY_FILES += \
    device/google/gs101/allowlist_com.google.android.as.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/sysconfig/allowlist_com.google.android.as.xml

# Camera
PRODUCT_PRODUCT_PROPERTIES += \
    ro.vendor.camera.extensions.package=com.google.android.apps.camera.services \
    ro.vendor.camera.extensions.service=com.google.android.apps.camera.services.extensions.service.PixelExtensions

# Google Assistant
PRODUCT_PRODUCT_PROPERTIES += ro.opa.eligible_device=true

# Lineage Health
include hardware/google/pixel/lineage_health/device.mk

$(call soong_config_set_bool,lineage_health,charging_control_supports_deadline,true)
$(call soong_config_set_bool,lineage_health,charging_control_supports_limit,true)
$(call soong_config_set_bool,lineage_health,charging_control_supports_toggle,false)

# Linker config
PRODUCT_VENDOR_LINKER_CONFIG_FRAGMENTS += \
    device/google/gs101/linker.config.json

# Parts
PRODUCT_PACKAGES += \
    GoogleParts

# Tethering
PRODUCT_PACKAGES += \
    TetheringOverlay

# Touch
include hardware/google/pixel/touch/device.mk

# VINTF
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += \
    device/google/gs101/vintf/vendor_framework_compatibility_matrix.xml
DEVICE_MANIFEST_FILE += \
    device/google/gs101/vintf/manifest.xml
DEVICE_MATRIX_FILE += \
    device/google/gs101/vintf/compatibility_matrix.xml
DEVICE_PRODUCT_COMPATIBILITY_MATRIX_FILE += \
    device/google/gs101/vintf/device_framework_matrix_product.xml
