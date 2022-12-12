package com.example.exceptions

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.example.exceptions.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        try {
            throwsException();
            binding.sampleText.text = "No exception thrown";
        } catch (e: java.lang.RuntimeException) {
            binding.sampleText.text = "RuntimeException caught. Message: \"" + e.message + "\"";
        }

    }

    external fun throwsException()

    companion object {
        // Used to load the 'exceptions' library on application startup.
        init {
            System.loadLibrary("exceptions")
        }
    }
}