package com.example.native_softkeyboard;

import android.app.NativeActivity;
import android.databinding.DataBindingUtil;
import android.os.Bundle;
import com.example.native_softkeyboard.databinding.SoftkeyboardActivityBinding;

public class SoftKeyboardNativeActivity extends NativeActivity {
    private SoftkeyboardActivityBinding binding;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().takeSurface(null);
        binding = DataBindingUtil.setContentView(this, R.layout.softkeyboard_activity);
    }
    public SoftkeyboardActivityBinding getBinding() {
        return binding;
    }
}
