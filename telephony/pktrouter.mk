PRODUCT_PACKAGES += wfc-pkt-router
PRODUCT_PROPERTY_OVERRIDES += vendor.pktrouter=1
BOARD_VENDOR_SEPOLICY_DIRS += device/google/gs101-sepolicy/telephony/pktrouter
PRODUCT_ARTIFACT_PATH_REQUIREMENT_ALLOWED_LIST += \
    $(TARGET_COPY_OUT_SYSTEM)/bin/oem-iptables-init.sh
