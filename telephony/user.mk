ifneq ($(wildcard vendor/samsung_slsi/telephony/),)
PRODUCT_COPY_FILES += vendor/samsung_slsi/telephony/common/device/samsung/init.radio.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.radio.sh
endif
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101/sepolicy/telephony/user/
