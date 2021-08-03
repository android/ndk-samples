package com.raptrmatt.hellogl2kotlin

class GL2JNILib {
    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("hellogl2kotlin")
        }

        external fun init(width: Int, height: Int)
        external fun step()
    }
}