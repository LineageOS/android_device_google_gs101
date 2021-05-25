PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
	device/google/gs101/gnss/47765/config/lhd.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/lhd.conf \
	device/google/gs101/gnss/47765/config/scd.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/scd.conf \
	device/google/gs101/gnss/47765/config/gps.cer:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/gps.cer \
	device/google/gs101/gnss/47765/firmware/SensorHub.patch:$(TARGET_COPY_OUT_VENDOR)/firmware/SensorHub.patch

ifneq (,$(filter raven, $(TARGET_PRODUCT)))
PRODUCT_COPY_FILES += device/google/gs101/gnss/47765/config/gps.xml.raven:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/gps.xml
else ifneq (,$(filter oriole, $(TARGET_PRODUCT)))
PRODUCT_COPY_FILES += device/google/gs101/gnss/47765/config/gps.xml.oriole:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/gps.xml
else
PRODUCT_COPY_FILES += device/google/gs101/gnss/47765/config/gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/gps.xml
endif

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# userdebug specific
PRODUCT_COPY_FILES += \
	device/google/gs101/gnss/47765/config/lhd2.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/lhd2.conf \
	device/google/gs101/gnss/47765/config/scd2.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/scd2.conf \
	device/google/gs101/gnss/47765/config/gps2.xml:$(TARGET_COPY_OUT_VENDOR)/etc/gnss/gps2.xml
endif

PRODUCT_SOONG_NAMESPACES += \
	device/google/gs101/gnss/47765

PRODUCT_PACKAGES += \
	android.hardware.gnss@2.1-impl-google \
	gps.default \
	flp.default \
	gpsd \
	lhd \
	scd \
	android.hardware.gnss@2.1-service-brcm

PRODUCT_PACKAGES_DEBUG += \
	init.gps_log.rc
