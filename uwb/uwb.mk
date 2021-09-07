#
# Copyright (C) 2021 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# UWB - ultra wide band
$(call inherit-product-if-exists, vendor/qorvo/uwb/uwb.mk)

LOCAL_UWB_CAL_DIR=device/google/gs101/uwb/calibration

ifneq (,$(filter raven, $(TARGET_PRODUCT)))
PRODUCT_COPY_FILES += \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration.conf.raven:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration.conf.raven:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-unknown.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration.conf.raven:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-default.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-jp.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-jp.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-ar.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-ar.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-am.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-am.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-az.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-az.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-by.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-by.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-id.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-id.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-kz.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-kz.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-kg.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-kg.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-np.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-np.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-pk.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-pk.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-py.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-py.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-ru.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-ru.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-sb.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-sb.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-tj.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-tj.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-tm.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-tm.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-ua.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-ua.conf \
    $(LOCAL_UWB_CAL_DIR)/UWB-calibration-uz.conf.r4:$(TARGET_COPY_OUT_VENDOR)/etc/UWB-calibration-uz.conf
endif

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.uwb.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.uwb.xml
