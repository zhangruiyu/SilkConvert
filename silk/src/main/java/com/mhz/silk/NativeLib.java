package com.mhz.silk;

public class NativeLib {

    // Used to load the 'silk' library on application startup.
    static {
        System.loadLibrary("silk");
    }

    /**
     * A native method that is implemented by the 'silk' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}