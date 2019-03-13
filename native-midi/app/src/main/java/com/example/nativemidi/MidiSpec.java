/*
 * Copyright (C) 2019 The Android Open Source Project
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
 *
 */

package com.example.nativemidi;

public class MidiSpec {
    // Message Codes
    public static final byte MIDICODE_NOTEOFF       = 0x08;
    public static final byte MIDICODE_NOTEON        = 0x09;
    public static final byte MIDICODE_POLYPRESS     = 0x0A;
    public static final byte MIDICODE_CONTROLLER    = 0x0B;
    public static final byte MIDICODE_PROGCHANGE    = 0x0C;
    public static final byte MIDICODE_CHANPRESS     = 0x0D;
    public static final byte MIDICODE_PITCHBEND     = 0x0E;

    // System Commands
    public static final byte MIDICODE_SYSEX         = (byte)0xF0;
    public static final byte MIDICODE_ENDOFSYSEX    = (byte)0xF7;
    public static final byte MIDICODE_ACTIVESENSING = (byte)0xFE;
    public static final byte MIDICODE_RESET         = (byte)0xFF;

    // Continuous Controllers
    public static final byte MAX_CC_VALUE = 127;
    public static final byte MIDICC_MODWHEEL = 1;

    public static final int MAX_PITCHBEND_VALUE = 0x3FFF;
    public static final int MID_PITCHBEND_VALUE = 0x2000;

    public static byte makeChanMessageCode(byte msg, byte chan) {
        return (byte)((msg << 4) | chan);
    }
}
