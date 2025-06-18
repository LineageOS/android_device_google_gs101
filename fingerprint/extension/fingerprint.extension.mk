# Fingerprint extension feature
ifneq (,$(filter aosp% factory%, $(TARGET_PRODUCT)))
# Skip if device is AOSP or factory build
endif
