package com.example.unittest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.example.unittest.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.sampleText.text = "1 + 2 = " + add(1, 2).toString()
    }

    /**
     * A native method that is implemented by the 'unittest' native library,
     * which is packaged with this application.
     */
    external fun add(a: Int, b: Int): Int

    companion object {
        // Used to load the 'unittest' library on application startup.
        init {
            System.loadLibrary("unittest")
        }
    }
}