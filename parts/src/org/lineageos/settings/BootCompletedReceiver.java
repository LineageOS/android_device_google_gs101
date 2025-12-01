/*
 * SPDX-FileCopyrightText: 2021 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String TAG = "GoogleParts";

    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.d(TAG, "Received boot completed intent");
        EuiccDisabler.enableOrDisableEuicc(context);
    }
}
