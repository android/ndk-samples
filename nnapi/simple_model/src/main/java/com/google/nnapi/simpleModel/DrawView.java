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
import android.graphics.Point;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.View;
import android.view.Window;

import java.util.ArrayList;
import java.util.List;

public class DrawView extends View {
    Paint paint = new Paint();
    List<Pair<View,View>> rectBoundingViews;
    List<Pair<View, View>> lines;
    private void init() {
        paint.setColor(Color.BLACK);
        lines = new ArrayList<>();
         rectBoundingViews = new ArrayList<>();
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

        for(Pair<View, View> pair : lines) {
            View v = pair.first;
            v.getLocationInWindow(start);
            start[0] += v.getWidth();
            start[1] += v.getHeight() / 2;

            v = pair.second;
            v.getLocationInWindow(stop);
            stop[1] += v.getHeight() / 2;
            canvas.drawLine(start[0], start[1], stop[0], stop[1], paint);
        }

        // each pair inside rectBoundingViews provides one coordinate for
        // two points (left, right) of the rectangle to be drawn.
        int startX, startY, stopX, stopY;
        int [] loc1 = {0, 0};
        int [] loc2 = {0, 0};
        View view1 = rectBoundingViews.get(0).first;
        view1.getLocationInWindow(loc1);
        startY = view1.getHeight() + loc1[1];
        rectBoundingViews.get(0).second.getLocationInWindow(loc2);
        startY = (startY + loc2[1] ) / 2;

        // for bottom line: either on the top of "compute" button
        // or leaving the same margin at bottom as that of top
        int viewHeight = this.getHeight();
        rectBoundingViews.get(1).second.getLocationInWindow(loc2);
        stopY = loc2[1];
        if (stopY > (viewHeight - startY)) {
           stopY = viewHeight - startY;
        }

        view1 = rectBoundingViews.get(2).first;
        view1.getLocationInWindow(loc1);
        startX = view1.getWidth() + loc1[0];
        rectBoundingViews.get(2).second.getLocationInWindow(loc2);
        startX = (startX + loc2[0]) / 2;

        view1 = rectBoundingViews.get(3).first;
        view1.getLocationInWindow(loc1);
        stopX = view1.getWidth() + loc1[0];
        rectBoundingViews.get(3).second.getLocationInWindow(loc2);
        stopX = (stopX + loc2[0]) / 2;

        paint.setColor(Color.BLUE);
        paint.setStrokeWidth(8f);
        paint.setStyle(Paint.Style.STROKE);
        canvas.drawRect(startX, startY, stopX, stopY, paint);
    }

    /*
     * left and right UI items to draw a line
     */
    public void AddLine(View left, View right) {
        lines.add(new Pair(left, right));
    }
    public void AddBoundingViews(View start, View stop) {
        rectBoundingViews.add(new Pair(start, stop));
    }
}