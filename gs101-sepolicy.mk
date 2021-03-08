# sepolicy that are shared among devices using whitechapel
BOARD_SEPOLICY_DIRS += device/google/gs101-sepolicy/whitechapel/vendor/google

# unresolved SELinux error log with bug tracking
BOARD_SEPOLICY_DIRS += device/google/gs101-sepolicy/tracking_denials

PRODUCT_PRIVATE_SEPOLICY_DIRS += device/google/gs101-sepolicy/private

#
# Pixel-wide
#
#   Dauntless (uses Citadel policy currently)
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/citadel

#   Wifi
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/wifi_ext

#   PowerStats HAL
BOARD_SEPOLICY_DIRS += hardware/google/pixel-sepolicy/powerstats

# Display
BOARD_SEPOLICY_DIRS += device/google/gs101-sepolicy/display/common
BOARD_SEPOLICY_DIRS += device/google/gs101-sepolicy/display/gs101

# Micro sensor framework (usf)
BOARD_SEPOLICY_DIRS += device/google/gs101-sepolicy/usf
