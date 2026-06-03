/*
 * SPDX-FileCopyrightText: 2021 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings;

import android.content.Context;
import android.content.pm.PackageManager;
import android.util.Log;

class EuiccDisabler {
    private static final String TAG = "GoogleParts";
    private static final String[] EUICC_PACKAGES = new String[]{
        "com.google.android.euicc",
        "com.google.euiccpixel"
    };

    public static void enableOrDisableEuicc(Context context) {
        PackageManager pm = context.getPackageManager();
        int flag = PackageManager.COMPONENT_ENABLED_STATE_ENABLED;
        for (String pkg : EUICC_PACKAGES) {
            try {
                pm.setApplicationEnabledSetting(pkg, flag, 0);
            } catch (IllegalArgumentException e) {
                Log.d(TAG, "package " + pkg + " is not present");
            }
        }
    }
}
