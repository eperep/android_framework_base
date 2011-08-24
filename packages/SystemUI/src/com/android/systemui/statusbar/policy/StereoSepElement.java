/*
 * Copyright (c) 2011 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

package com.android.systemui.statusbar.policy;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.content.Intent;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.lang.System;

import com.android.systemui.R;

/**
 * Class that defines and implements a Stereo Separation Slider
 */
public class StereoSepElement extends RelativeLayout
        implements SeekBar.OnSeekBarChangeListener {
    private static final String TAG = "StatusBar.StereoSepElement";
    private static final String StereoSeparation = "NV_STEREOSEP";
    private static final String stereoCtrlPropName = "NV_STEREOCTRL";
    private static final String stereoAppPropName = "NV_STEREOAPP";
    private static final String sepChangePropName = "NV_STEREOSEPCHG";
    private static final String STATE_ID_KEY = "StateID";
    public static final int STATEID_3DV_CONTROLS  = 1;

    private SeekBar mSlider;
    private TextView mDepth3DView;
    private Context mContext;
    private int mDepth3DVal;
    private boolean bInitDone;

    /* JNI Function Prototypes */
    private native String getPropertyUsingJNI(String propname);

    /* Load JNI library */
    static {
        System.loadLibrary("nvsystemuiext_jni");
    }

    public StereoSepElement(Context context) {
        this(context, null);
    }

    public StereoSepElement(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public StereoSepElement(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        View.inflate(context, R.layout.status_bar_stereo_sep_slider, this);

        mContext = context;

        mDepth3DView = (TextView)findViewById(R.id.status_bar_settings_stereo_separation_val);
        mSlider = (SeekBar)findViewById(R.id.status_bar_settings_stereo_separation_slider);
        mSlider.setOnSeekBarChangeListener(this);

        // Make sure the property is updated with the default for the first time
        // Read the current property value. If it exists, then display that in the UI.
        // Otherwise, we assume that the property needs to be written for the first time, and
        // so write it to a value of 20%
        int progressVal = 20;
        bInitDone = false;
        String strDepthPropName = "persist.sys." + StereoSeparation;
        String strDepthPropVal = getPropertyUsingJNI(strDepthPropName);
        if (!strDepthPropVal.equals("")) {
            bInitDone = true;
            progressVal = Integer.parseInt(strDepthPropVal);
        }
        mSlider.setProgress(progressVal);
    }

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        mDepth3DVal = progress;
        mDepth3DView.setText(progress + "%");

        if (!bInitDone) {
            // Only to be called for initializing the first time
            // Subsequent updates will happen in onStopTrackingTouch()
            setControl(StereoSeparation, Integer.toString(mDepth3DVal));
            bInitDone = true;
        }
    }

    public void onStartTrackingTouch(SeekBar seekBar) {
        // Do nothing for now
    }

    public void onStopTrackingTouch(SeekBar seekBar) {
        setControl(StereoSeparation, Integer.toString(mDepth3DVal));
    }

    private void setControl(String ctrlName, String ctrlValue) {
        // Send the current depth value to the service
        Intent intent = new Intent();

        intent.setClassName("com.nvidia.NvCPLSvc", "com.nvidia.NvCPLSvc.NvCPLService");
        intent.putExtra("com.nvidia.NvCPLSvc."+ ctrlName, ctrlValue);
        intent.putExtra("com.nvidia.NvCPLSvc."+ STATE_ID_KEY, STATEID_3DV_CONTROLS);
        try {
            mContext.startService(intent);
        } catch (Exception e) {
            Log.e(TAG, "Exception while launching NV Service", e);
        }
    }

    /**
      * Show the Stereo separation slider only if HDMI is connected, Stereo is enabled,
      * and a Stereo App is currently running
      */
    public boolean shouldBeVisibile() {
        if (isHDMIConnected() && isStereoEnabled() && isStereoAppRunning()) {
            return true;
        } else {
            return false;
        }
    }

    /** Function that takes action depending on whether
        the slider is currently showing or not */
    public void setSliderIsVisible(boolean bIsVisible) {
        String strIsVisible = bIsVisible ? "1" : "0";
        setControl(sepChangePropName, strIsVisible);
    }

    /** Check if HDMI is Connected */
    private boolean isHDMIConnected() {
        try {
            char[] buffer = new char[1024];
            FileReader hdmiStateFile = new FileReader("/sys/class/switch/tegradc.1/state");
            int len = hdmiStateFile.read(buffer, 0, 1024);

            String hdmiState = (new String(buffer, 0, len)).trim();

            // The value could be "offline", or "wxh". Ignore anything else.
            if (hdmiState.equals("offline")) {
                Log.v(TAG, "HDMI Display NOT connected");
                return false;
            } else {
                String[] strDims = hdmiState.split("x");
                if (strDims.length == 2) {
                    Log.v(TAG, "HDMI Display connected, dims = " + strDims[0] + ", " + strDims[1]);
                    return true;
                } else {
                    Log.v(TAG, "Unrecognized hdmi switch state value.");
                    return false;
                }
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "Couldn't read HDMI state");
            return false;
        } catch (Exception e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /** Check if stereo is enabled */
    private boolean isStereoEnabled() {
        String strCtrlFullPropName = "persist.sys." + stereoCtrlPropName;
        String strCtrlPropVal = getPropertyUsingJNI(strCtrlFullPropName);
        if (strCtrlPropVal.equals("") || strCtrlPropVal.equals("0")) {
            Log.d(TAG, "isStereoEnabled(): Stereo is NOT Enabled");
            return false;
        } else {
            Log.d(TAG, "isStereoEnabled(): Stereo is Enabled");
            return true;
        }
    }

    /** Check if app is running in stereo */
    private boolean isStereoAppRunning() {
        String stereoAppFullPropName = "persist.sys." + stereoAppPropName;
        String strAppPropVal = getPropertyUsingJNI(stereoAppFullPropName);
        if (strAppPropVal.equals("") || strAppPropVal.equals("0")) {
            Log.d(TAG, "isStereoAppRunning(): Stereo App is NOT currently running");
            return false;
        } else {
            Log.d(TAG, "isStereoAppRunning(): Stereo App is currently running");
            return true;
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        setSliderIsVisible(false);
    }
}

