/*
 * Copyright (C) Google Inc.
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
#ifndef endlesstunnel_joystick_support_hpp
#define endlesstunnel_joystick_support_hpp

// A few declarations we need for NDK joystick support as of NDK release 9
typedef float (*_getAxisValue_sig)(const AInputEvent*, int32_t, size_t);
static const int AXIS_HAT_X = 15, AXIS_HAT_Y = 16;
static const int SOURCE_CLASS_JOYSTICK = 16;
static const int KEYCODE_BUTTON_X = 99;
static const int KEYCODE_BUTTON_Y = 100;
static const int KEYCODE_BUTTON_A = 96;
static const int KEYCODE_BUTTON_B = 97;

// source type for "touch navigation" devices (introduced in API 18)
static const int SOURCE_TOUCH_NAVIGATION = 0x00200000;

#endif

