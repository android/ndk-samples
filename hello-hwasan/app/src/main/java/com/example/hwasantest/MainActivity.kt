package com.example.hellohwasan

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        button_uaf.setOnClickListener { _ ->
            doUseAfterFree();
        }
        button_uaf_loop.setOnClickListener { _ ->
            doUseAfterFreeLoop();
        }
        button_oob.setOnClickListener { _ ->
            doHeapBufferOverflow();
        }
        button_double_free.setOnClickListener { _ ->
            doDoubleFree();
        }
        button_null_deref.setOnClickListener { _ ->
            doNullDeref();
        }
    }

    external fun doUseAfterFree()
    external fun doUseAfterFreeLoop()
    external fun doHeapBufferOverflow()
    external fun doDoubleFree()
    external fun doNullDeref()

    companion object {

        init {
            System.loadLibrary("native-lib")
        }
    }
}
