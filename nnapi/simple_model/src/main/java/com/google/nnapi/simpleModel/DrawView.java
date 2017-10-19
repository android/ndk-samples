/*
 * Copyright 2017 The Android Open Source Project
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
 */
package com.google.nnapi.simpleModel;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.View;

import java.util.ArrayList;
import java.util.List;

public class DrawView extends View {
    Paint paint = new Paint();
    List<Pair<View, View>> pairs;
    private void init() {
        paint.setColor(Color.BLACK);
        pairs = new ArrayList<>();
    }

    public DrawView(Context context) {
        super(context);
        init();
    }

    public DrawView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public DrawView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    @Override
    public void onDraw(Canvas canvas) {
        paint.setStrokeWidth(4f);
        int[] start = {0, 0};
        int[] stop = {0, 0};

        for(Pair<View, View> pair :  pairs) {
            View v = pair.first;
            v.getLocationInWindow(start);
            start[0] += v.getWidth();
            start[1] += v.getHeight() / 2;

            v = pair.second;
            v.getLocationInWindow(stop);
            stop[1] += v.getHeight() / 2;
            canvas.drawLine(start[0], start[1], stop[0], stop[1], paint);
        }
    }

    /*
     * left and right UI items to draw a line
     */
    public void AddLine(View left, View right) {
        pairs.add(new Pair(left, right));
    }
}