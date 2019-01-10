package com.example.helloneon;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class HelloNeon extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hello_neon);

        ((TextView)findViewById(R.id.text_view_hello_neon))
                .setText(stringFromJNI());
    }

    public native String stringFromJNI();

    static {
        System.loadLibrary("hello-neon");
    }
}
