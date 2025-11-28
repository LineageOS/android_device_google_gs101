# ConnectivityThermalPowerManager
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/connectivity_thermal_power_manager

# twoshay
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/input

# google_battery service
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/googlebattery

# sepolicy that are shared among devices using whitechapel
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101/sepolicy/whitechapel/vendor/google

# unresolved SELinux error log with bug tracking
BOARD_SEPOLICY_DIRS += device/google/gs101/sepolicy/tracking_denials

PRODUCT_PRIVATE_SEPOLICY_DIRS += device/google/gs101/sepolicy/private

# Display
BOARD_SEPOLICY_DIRS += device/google/gs101/sepolicy/display/common
BOARD_SEPOLICY_DIRS += device/google/gs101/sepolicy/display/gs101

# system_ext
SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += device/google/gs101/sepolicy/system_ext/public
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += device/google/gs101/sepolicy/system_ext/private

#
# Pixel-wide
#
#   PowerStats HAL
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/powerstats

# Public
PRODUCT_PUBLIC_SEPOLICY_DIRS += device/google/gs101/sepolicy/public

# Health HAL
BOARD_SEPOLICY_DIRS += device/google/gs101/sepolicy/health

BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101/sepolicy/modem/user
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101/sepolicy/telephony/user/
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101/sepolicy/trusty_metricsd

BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/aoc/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/audio/sepolicy/common
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/audio/sepolicy/hidl
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/vendor
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/bcmbt/dump/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/bootctrl/sepolicy/aidl
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/camera/sepolicy/vendor
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/chre/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/dauntless/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/display/sepolicy/exynos
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/edgetpu/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/fingerprint/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gear/dumpstate/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gps/brcm/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gps/dump/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gpu/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/gxp/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/insmod/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/mediacodec/common/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/mediacodec/samsung/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/misc_writer
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/modem/dump_modemlog/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/modem/modem_svc_sit/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/nfc/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/performance/experiments/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/performance/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/pixel_metrics/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/pixel_ril/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/radio/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/ramdump_and_coredump/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/sensors/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/soc/sepolicy/freq
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/soc/sepolicy/soc
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/storage/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/storage/sepolicy/tracking_denials
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/telephony/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/thermal/sepolicy/dump
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/thermal/sepolicy/thermal_hal
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/touch/twoshay/sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs-common/trusty/sepolicy

PRODUCT_PUBLIC_SEPOLICY_DIRS += device/google/gs-common/camera/sepolicy/product/public
PRODUCT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/camera/sepolicy/product/private

PRODUCT_PUBLIC_SEPOLICY_DIRS += device/google/gs-common/betterbug/sepolicy/product/public
PRODUCT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/betterbug/sepolicy/product/private

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/system_ext/private
SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += device/google/gs-common/battery_mitigation/sepolicy/system_ext/public

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/gs_watchdogd/sepolicy

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += device/google/gs-common/sota_app/sepolicy/system_ext