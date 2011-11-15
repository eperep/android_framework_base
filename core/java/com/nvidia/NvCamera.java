package com.nvidia;

import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;


public class NvCamera extends Camera{
    private static final String TAG = "NvCamera";

    public static class NvCameraInfo extends CameraInfo{
        /**
         * The camera is USB based.
         */
        public static final int CAMERA_USB = 2;

    };

    /**
     * Creates a new Camera object to access a particular hardware camera.
     */
    public static NvCamera open(int cameraId) {
        return new NvCamera(cameraId);
    }

    /**
     * Creates a new Camera object to access the first back-facing camera on the
     * device. If the device does not have a back-facing camera, this returns
     * null.
     */
    public static NvCamera open() {
        int numberOfCameras = getNumberOfCameras();
        NvCameraInfo cameraInfo = new NvCameraInfo();
        for (int i = 0; i < numberOfCameras; i++) {
            getCameraInfo(i, cameraInfo);
            if (cameraInfo.facing == NvCameraInfo.CAMERA_FACING_BACK) {
                return new NvCamera(i);
            }
        }
        return null;
    }

    NvCamera(int cameraId) {
        super(cameraId);
    }


}
