package com.example.orderfiledemo 

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.orderfiledemo .databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        runWorkload(applicationContext.cacheDir.toString())
        binding.sampleText.text = "Hello, world!"
    }

    /**
     * A native method that is implemented by the 'orderfiledemo' native library,
     * which is packaged with this application.
     */
    external fun runWorkload(tempDir: String)

    companion object {
        // Used to load the 'orderfiledemo' library on application startup.
        init {
            System.loadLibrary("orderfiledemo")
        }
    }
}
