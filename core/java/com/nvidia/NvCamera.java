/*
 * Copyright (c) 2011 NVIDIA Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Class structure based upon Camera in Camera.java:
 * Copyright (C) 2009 The Android Open Source Project
 */

/**
 * @file
 * <b>NVIDIA Tegra Android Camera API Extensions</b>
 *
 * @b Description: Exposes additional functionality from the Android Camera
 *    Hardware Abstraction Layer (Camera HAL) to the Java camera API.

 */

/**
 * @defgroup nvcamera_ext_group NVIDIA Camera Extensions
 *
 * To enhance current camera parameters with additional functionality,
 * NVIDIA has exposed API extensions from the HAL to the Java layer
 * You can use the additional extensions that are available inside
 * the Camera HAL to configure NVIDIA's extended functionality.
 *
 * To facilitate setting these parameters, NVIDIA has extended the Android
 * @c android.hardware.Camera class in the @c com.nvidia.NvCamera class, and
 * it has extended the @c android.hardware.Camera.Parameters class in the
 * @c com.nvidia.NvCamera.NvParameters class. These classes contain the full
 * standard Android API, as they are extensions of the base classes. Because
 * of this, it should be relatively easy to incorporate these classes into
 * existing applications. The extended APIs that these classes implement on
 * top of the stock Android API are described in this section.
 *
 * @{
 */
package com.nvidia;

import android.util.Log;
import java.util.ArrayList;
import java.util.StringTokenizer;

import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;


public class NvCamera extends Camera{
    private static final String TAG = "NvCamera";

    public static class NvCameraInfo extends CameraInfo{
        /**
         * The camera is USB-based.
         */
        public static final int CAMERA_USB = 2;

    };

    public NvWindow newNvWindow() {
        return new NvWindow();
    }

    public class NvWindow {
        public NvWindow() {
            left = 0;
            top = 0;
            right = 0;
            bottom = 0;
            weight = 0;
        }

        public NvWindow(int l, int t, int r, int b, int w) {
            left = l;
            top = t;
            right = r;
            bottom = b;
            weight = w;
        }

        public int left;
        public int top;
        public int right;
        public int bottom;
        public int weight;
    }

    private native final void native_setCustomParameters(String params);
    private native final String native_getCustomParameters();

/** @name General Functions
 *
 * Use these functions in the NvCamera class to transfer the
 * @c NvCamera.NvParameters instance between the application and the HAL, which
 * is similar to the set/getParameters interface that exists in the standard
 * Android API.
 *
 * NVIDIA's extended classes follow the general camera parameters API model in
 * the Android API. For more information about this model, see the Android API
 * documentation at:
 *
 *http://developer.android.com/reference/android/hardware/Camera.Parameters.html
 *
 * Unless otherwise specified, all of the API methods that are described in
 * this section belong to the NvCamera.NvParameters class.
 */
/*@{*/

    public void setParameters(NvParameters params) {
        native_setCustomParameters(params.flatten());
    }

    public NvParameters getParameters() {
        NvParameters p = new NvParameters();
        String s = native_getCustomParameters();
        p.unflatten(s);
        return p;
    }
/*@}*/

    public class NvParameters extends Parameters {
        private static final String NV_NSL_NUM_BUFFERS = "nv-nsl-num-buffers";
        private static final String NV_NSL_SKIP_COUNT = "nv-nsl-burst-skip-count";
        private static final String NV_NSL_BURST_PICTURE_COUNT = "nv-nsl-burst-picture-count";
        private static final String NV_SKIP_COUNT = "nv-burst-skip-count";
        private static final String NV_BURST_PICTURE_COUNT = "nv-burst-picture-count";
        private static final String NV_RAW_DUMP_FLAG = "nv-raw-dump-flag";
        private static final String NV_EV_BRACKET_CAPTURE = "nv-ev-bracket-capture";
        private static final String NV_FOCUS_AREAS = "focus-areas";
        private static final String NV_METERING_AREAS = "metering-areas";
        private static final String NV_COLOR_CORRECTION = "nv-color-correction";
        private static final String NV_SATURATION = "nv-saturation";
        private static final String NV_CONTRAST = "nv-contrast";
        private static final String NV_EDGE_ENHANCEMENT = "nv-edge-enhancement";
        private static final String NV_EXPOSURE_TIME = "nv-exposure-time";
        private static final String NV_PICTURE_ISO = "nv-picture-iso";
        private static final String NV_FOCUS_POSITION = "nv-focus-position";
        private static final String NV_AUTOWHITEBALANCE_LOCK = "auto-whitebalance-lock";
        private static final String NV_AUTOEXPOSURE_LOCK = "auto-exposure-lock";

        protected NvParameters() {
            super();
        }

/** @name Negative Shutter Lag
 */
/*@{*/
    /**
     * Sets the current number of negative shutter lag buffers.
     *
     * Negative shutter lag will actively store frames from the sensor as long
     * as a nonzero number of buffers are allocated. As the number of buffers
     * that the driver can allocate depends on the available memory size, the
     * actual value may be smaller than the set value.
     *
     * Defaults to 0.
     */
        public void setNSLNumBuffers(int num) {
            String v = Integer.toString(num);
            set(NV_NSL_NUM_BUFFERS, v);
        }

    /**
     * Gets the current number of negative shutter lag buffers.
     *
     * Use this to find out how many NSL buffers were successfully allocated on
     * a request to the driver. It is the application's responsibility to query
     * this after setting it, to confirm the number of successfully allocated
     * buffers. If buffers are allocated and an application requests a
     * resolution change, the driver will dynamically re-allocate the buffers
     * to accommodate the new resolution. If it is unable to allocate the
     * number of previously requested buffers due to memory restrictions, it
     * will allocate as many as it is able to. Because of this, applications
     * must also query this parameter after a resolution change to find out if
     * the number of allocated buffers has changed.
     */
        public int getNSLNumBuffers() {
            return getInt(NV_NSL_NUM_BUFFERS);
        }

    /**
     * Sets the current burst skip count for negative shutter lag.
     *
     * This determines the number of frames on the NSL buffer that will be
     * skipped between each frame that is returned to the application; this is
     * useful for requesting fewer NSL images that span a larger time period.
     *
     * The legal range of this value is constrained by the number of buffers
     * and burst picture count for negative shutter lag. The following formula
     * must be true:
     * <pre>
     *
     *     burstCount + skipCount*(burstCount-1) <= numNSLBuffers
     *
     * </pre>
     *
     * Defaults to 0.
     */
        public void setNSLSkipCount(int count) {
            String v = Integer.toString(count);
            set(NV_NSL_SKIP_COUNT, v);
        }

    /**
     * Gets the current burst skip count for negative shutter lag.
     */
        public int getNSLSkipCount() {
            return getInt(NV_NSL_SKIP_COUNT);
        }

    /**
     * Sets the current burst count for negative shutter lag captures.
     *
     * The value must be not more than the number of allocated negative shutter
     * lag buffers. If this parameter is set, the requested frames will be
     * delivered one-at-a-time to the JPEG callback that is triggered by the
     * standard @c takePicture() request.
     *
     * For example, a negative shutter lag burst of 5 will call the JPEG
     * callback 5 times. The negative shutter lag frames will be delivered
     * chronologically; the frame with the oldest timestamp will be the first
     * frame that is sent to the JPEG callback.
     *
     * Defaults to 0.
     */
        public void setNSLBurstCount(int count) {
            String v = Integer.toString(count);
            set(NV_NSL_BURST_PICTURE_COUNT, v);
        }

    /**
     * Gets the current burst count for negative shutter lag captures.
     */
        public int getNSLBurstCount() {
            return getInt(NV_NSL_BURST_PICTURE_COUNT);
        }
/*@}*/

/** @name Burst Capture
 */
/*@{*/

    /**
     * Sets the number of frames from the sensor to be skipped between frames
     * that are sent to the encoder. For example, with a burst count of 2 and a
     * skip count of 1, the first frame from the sensor will be captured and
     * processed, the next frame will be dropped, and then the frame after that
     * will be captured and processed.
     *
     * Defaults to 0.
     */
        public void setSkipCount(int count) {
            String v = Integer.toString(count);
            set(NV_SKIP_COUNT, v);
        }

    /**
     * Gets the current burst skip count.
     */
        public int getSkipCount() {
            return getInt(NV_SKIP_COUNT);
        }

    /**
     * Sets the current burst picture count.
     *
     * Defaults to 1.
     */
        public void setBurstCount(int count) {
            String v = Integer.toString(count);
            set(NV_BURST_PICTURE_COUNT, v);
        }

    /**
     * Gets the current burst picture count.
     */
        public int getBurstCount() {
            return getInt(NV_BURST_PICTURE_COUNT);
        }
/*@}*/

/** @name Raw Dump Flag
 */
/*@{*/

    /**
     * Sets the current raw dump flag.
     *
     * This is a 3-bit indicator, where:
     * - bit 0 is raw dump on/off
     * - bit 1 is dump-to-file in camera driver on/off
     * - bit 2 is include NV raw header on/off
     *
     * @note Raw dump feature must be off in burst capture mode.
     *
     * Defaults to 0.
     */
        public void setRawDumpFlag(int flag) {
            String v = Integer.toString(flag);
            set(NV_RAW_DUMP_FLAG, v);
        }

    /**
     * Gets the current raw dump flag.
     */
        public int getRawDumpFlag() {
            return getInt(NV_RAW_DUMP_FLAG);
        }

/*@}*/
/** @name Bracket Capture
 *
 */
/*@{*/
    /**
     * Sets the EV bracket capture setting.
     *
     * After this property is set and a burst capture command is issued, each
     * image in the burst capture sequence will be adjusted by the specified
     * amount by the EV setting (-3 to 3 inclusive).
     *
     * The first image received will have the left most compensation value;
     * likewise, the last image received will contain the last specified
     * compensation value.
     *
     * Setting the value to an empty string disables bracket capture.
     *
     * Interaction with NSL captures during bracket capture NSL is not
     * available. Bracket capture is cleared by setting an empty string, and
     * once cleared NSL will operate as normal. Non-burst captures will operate
     * as normal.
     *
     * - Item value range: [-3.0, +3.0] float
     * - Number of items:  [0, 2-7] , 0 or at least two and up to seven values.
     *
     */
        public void setEvBracketCapture(float[] evValues) {
            StringBuilder evString = new StringBuilder(35);

            if (evValues == null)
            {
                evString.append(" ");
            }
            else
            {
                for (int i = 0; i < evValues.length - 1; i++)
                {
                    evString.append(evValues[i]);
                    evString.append(",");
                }
                evString.append(evValues[evValues.length -1]);
            }
            set(NV_EV_BRACKET_CAPTURE, evString.toString());
        }
/*@}*/

/** @name Focus Areas
 *
 */
/*@{*/

    /**
     * Sets the current focus region of interest.
     *
     * The left, top, right, and bottom window coordinates are specified in
     * screen coordinates, from -1000 to 1000. Weight should always be set to 1
     * for focus areas. When the image is zoomed, the region will take effect
     * relative to the zoomed field of view. The default focus area is subject
     * to change, and we recommend that applications query it with a call to
     * getFocusAreas() before setting it. See the Example 3.
     * At the time this documentation was written, the default focus window was
     * (-240,-240,240,240,1).
     *
     * @note The NvCamera.NvWindow class has integer members left, right, top,
     * bottom, and weight, to facilitate the convenient setting of focus and
     * exposure areas.
     */
        public void setFocusAreas(ArrayList<NvWindow> windowList) {
            String str = WindowsToString(windowList);
            if (str != null)
                set(NV_FOCUS_AREAS, str);
        }

    /**
     * Queries the current focus region of interest.
     *
     * @return The argument that is passed in has the current focus region
     * written to it when the function returns.
     */
        public void getFocusAreas(ArrayList<NvWindow> windowList) {
            String str = get(NV_FOCUS_AREAS);
            StringToWindows(windowList, str);
        }

/*@}*/

/** @name Exposure Areas
 *
 * @note Exposure Modes can be viewed as special use-cases of the exposure
 * areas. Detailed examples are provided in the example code section.
 *
 * - Center: We are currently working out the HAL APIs for programmable
 *   center-weighted windows of varying sizes. Until that is complete, we can
 *   leverage the fact that the driver's default exposure algorithm is
 *   center-weighted across the entire image. We can program this behavior by
 *   setting the special (0,0,0,0,0) exposure area.
 * - Average: this is equivalent to setting an exposure area that covers the
 *   entire field of view, i.e. (-1000,-1000,1000,1000,1);
 * - Spot: this is equivalent to setting a small exposure area around the region
 *   of interest.
 */
/*@{*/

    /**
     * Sets the current exposure region of interest.
     *
     * The left, top, right, and bottom coordinates are specified in screen
     * coordinates, from -1000 to 1000. The weight parameter can range anywhere
     * from 1 to 1000, and it is used by the driver to weight the @b importance
     * of the regions relative to each other in the exposure calculations.
     *
     * When the image is zoomed, the regions will take effect relative to the
     * zoomed field of view. The application can set up to the number of areas
     * that is returned by getMaxNumMeteringAreas(). Multiple regions can be
     * configured in a single call by appending them to the list that is passed
     * as an argument. The special case of all 0's will restore the driver's
     * default exposure algorithm, which uses a Gaussian, center-weighted
     * calculation over the whole image.
     */
        public void setMeteringAreas(ArrayList<NvWindow> windowList) {
            String str = WindowsToString(windowList);
            if (str != null)
                set(NV_METERING_AREAS, str);
        }

    /**
     * Queries the current exposure region of interest.
     *
     * @return The argument that is passed in will have the current
     * exposure region written to it when the function returns.
     */
        public void getMeteringAreas(ArrayList<NvWindow> windowList) {
            String str = get(NV_METERING_AREAS);
            StringToWindows(windowList, str);
        }
/*@}*/

/** @name Color Correction
 *
 */
/*@{*/

    /**
     * Sets the manual 4x4 color correction matrix.
     *
     * Passing in a matrix of all 0's restores the default driver behavior.
     * Values in the array are specified across the rows, i.e. row 0 will be
     * entries 0-3 of the array, row 2 will be entries 4-7, etc.
     *
     * Defaults to all 0's.
     */
        public void setColorCorrection(String str) {
            set(NV_COLOR_CORRECTION, str);
        }

    /**
     * Gets the current color correction matrix, filling the argument with the
     * current values.
     */
        public String getColorCorrection() {
            return get(NV_COLOR_CORRECTION);
        }

        public void setColorCorrection(float[] matrix) {
            if (matrix != null && matrix.length == 16)
            {
                StringBuilder matrixString = new StringBuilder(256);
                for (int i = 0; i < 15; i++)
                {
                    matrixString.append(matrix[i]);
                    matrixString.append(",");
                }
                matrixString.append(matrix[15]);
                set(NV_COLOR_CORRECTION, matrixString.toString());
            }
        }

        public void getColorCorrection(float[] matrix) {
            if (matrix != null && matrix.length == 16)
            {
                String str = get(NV_COLOR_CORRECTION);
                if (str == null)
                {
                    for (int i = 0; i < 16; i++)
                        matrix[i] = 0;
                }
                else
                {
                    StringTokenizer tokenizer = new StringTokenizer(str, ",");
                    int index = 0;
                    while (tokenizer.hasMoreElements())
                    {
                        String token = tokenizer.nextToken();
                        matrix[index++] = Float.parseFloat(token);
                    }
                }
            }
        }
/*@}*/

/** @name Saturation
 *
 */
/*@{*/

    /**
     * Sets the current saturation value.
     *
     * Defaults to 0.
     */
        public void setSaturation(int saturation) {
            String v = Integer.toString(saturation);
            set(NV_SATURATION, v);
        }

    /**
     * Gets the current saturation value.
     */
        public int getSaturation() {
            return getInt(NV_SATURATION);
        }
/*@}*/

/** @name Contrast
 *
 */
/*@{*/

    /**
     * Sets the current contrast value. String argument can be "lowest",
     * "low", "normal", "high", or "highest".
     *
     * Defaults to "normal".
     */
        public void setContrast(String str) {
            set(NV_CONTRAST, str);
        }

    /**
     * Gets the current contrast value.
     */        public String getContrast() {

            return get(NV_CONTRAST);
        }
/*@}*/

/** @name Edge Enhancement
 *
 */
/*@{*/

    /**
     * Sets the edge enhancement value. The valid range is from -100 to 100.
     * Set -101 to turn edge enhancement off.
     *
     * Defaults to 0.
     */

        public void setEdgeEnhancement(int value) {
            String v = Integer.toString(value);
            set(NV_EDGE_ENHANCEMENT, v);
        }


    /**
     * Gets the current edge enhancement value.
     */
        public int getEdgeEnhancement() {
            return getInt(NV_EDGE_ENHANCEMENT);
        }
/*@}*/

/** @name Exposure Time
 *
 */
/*@{*/

    /**
     * Sets the current exposure time. Set to 0 to enable auto-exposure.
     * Nonzero values are interpreted in units of microseconds.
     *
     *  Defaults to auto.
     */
        public void setExposureTime(int value) {
            String v = Integer.toString(value);
            set(NV_EXPOSURE_TIME, v);
        }

    /**
     * Gets the current exposure time.
     */
        public int getExposureTime() {
            return getInt(NV_EXPOSURE_TIME);
        }
/*@}*/

/** @name ISO
 *
 */
/*@{*/

    /**
     * Sets the current ISO value. The string can take on values of
     * "auto","100","200","400", or "800".
     *
     * Defaults to "auto".
     */
        public void setPictureISO(String str) {
            set(NV_PICTURE_ISO, str);
        }

    /**
     * Gets the current ISO value.
     */
        public String getPictureISO() {
            return get(NV_PICTURE_ISO);
        }
/*@}*/

/** @name Focus Position
 *
 */
/*@{*/

    /**
     * Sets the current focus position, if it is supported by the focuser.
     *
     * Defaults to 0.
     */
        public void setFocusPosition(int position) {
            String v = Integer.toString(position);
            set(NV_FOCUS_POSITION, v);
        }

    /**
     * Gets the current focus position.
     */
        public int getFocusPosition() {
            return getInt(NV_FOCUS_POSITION);
        }
/*@}*/

/** @name Auto White Balance and Auto Exposure Lock
 *
 * \c AutoWhiteBalance can be locked, so the colors no longer are adjusted
 * automatically based off the scene. It can be unlocked with the corresponding
 * function.
 *
 * \c AutoExposure can be locked, so the exposure is no longer automatically
 * adjusted based off the scene.
 */
/*@{*/

    /**
     * Sets the auto white balance lock.
     */
        public void setAutoWhiteBalanceLock(boolean lock) {
            String v = Boolean.toString(lock);
            set(NV_AUTOWHITEBALANCE_LOCK, v);
        }

    /**
     * Gets the auto white balance lock.
     */
        public boolean getAutoWhiteBalanceLock() {
            String v = get(NV_AUTOWHITEBALANCE_LOCK);
            return Boolean.valueOf(v);
        }

    /**
     * Sets the auto exposure lock.
     */
        public void setAutoExposureLock(boolean lock) {
            String v = Boolean.toString(lock);
            set(NV_AUTOEXPOSURE_LOCK, v);
        }

    /**
     * Gets the auto exposure lock.
     */
        public boolean getAutoExposureLock() {
            String v = get(NV_AUTOEXPOSURE_LOCK);
            return Boolean.valueOf(v);
        }
/*@}*/

        private String WindowsToString(ArrayList<NvWindow> windowList) {
            if (windowList == null || windowList.size() == 0)
            {
                return null;
            }
            else
            {
                int size = windowList.size();
                StringBuilder windowsString = new StringBuilder(256);
                for (int i = 0; i < size; i++)
                {
                    NvWindow window = windowList.get(i);
                    windowsString.append("(");
                    windowsString.append(window.left);
                    windowsString.append(",");
                    windowsString.append(window.top);
                    windowsString.append(",");
                    windowsString.append(window.right);
                    windowsString.append(",");
                    windowsString.append(window.bottom);
                    windowsString.append(",");
                    windowsString.append(window.weight);
                    windowsString.append(")");
                    if (i != size - 1)
                        windowsString.append(",");
                }
                return windowsString.toString();
            }
        }

        private void StringToWindows(ArrayList<NvWindow> windowList, String str) {
            if (windowList != null && str != null)
            {
                StringTokenizer tokenizer = new StringTokenizer(str, "(");
                while (tokenizer.hasMoreElements())
                {
                    NvWindow window = new NvWindow();
                    String token = tokenizer.nextToken();
                    StringTokenizer subTokenizer = new StringTokenizer(token, ",");
                    String subToken = subTokenizer.nextToken();
                    window.left = Integer.parseInt(subToken);
                    subToken = subTokenizer.nextToken();
                    window.top = Integer.parseInt(subToken);
                    subToken = subTokenizer.nextToken();
                    window.right = Integer.parseInt(subToken);
                    subToken = subTokenizer.nextToken();
                    window.bottom = Integer.parseInt(subToken);
                    subToken = subTokenizer.nextToken();
                    StringTokenizer endTokenizer = new StringTokenizer(subToken, ")");
                    String endToken = endTokenizer.nextToken();
                    window.weight = Integer.parseInt(endToken);
                    windowList.add(window);
                }
            }
        }
    };

/** @name Camera Object
 *
 */
/*@{*/

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

/** @} */

/** @} */
