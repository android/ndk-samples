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

// package com.nativemidiapp;
package com.example.nativemidi;

class MidiDataHelper {
    /**
     * Create a MIDI data stream, containing a set of 3-value messages (potentially using
     * Running Status) each of the specific message-code/channel with associated 2-byte parameter
     * pairs.
     * For example, a set of 3, simultaneous key-down messages (a "C major" chord for example) can
     * be formated with a call:
     *    make3ByteMsgBuff(MIDICODE_KEYUP, 0, {60, 64, 67}, {60, 60, 60});
     * Note that each of the key codes in the param_a has a corresponding velocity value in param_b.
     *
     * @param msgID The MIDI message ID.
     * @param channel   The MIDI channel number for the message(s).
     * @param param_a   The 1st parameter bytes.
     * @param param_b   The 2nd parameter bytes.
     * @param useRunningStatus  If true, specifies that message streams should use running-status.
     * @return The MIDI data stream.
     */
    static byte[] make3ByteMsgBuff(
            byte msgID, byte channel, byte[] param_a, byte[] param_b, boolean useRunningStatus) {
        assert param_a.length == param_b.length;

        int numMsgs = param_a.length;

        byte[] msgBuff;
        if (useRunningStatus) {
            msgBuff = new byte[1 + (numMsgs * 2)];
            int byteOffset = 0;
            msgBuff[byteOffset++] =
                    MidiSpec.makeChanMessageCode(msgID, channel);
            for(int index = 0; index < numMsgs; index++) {
                msgBuff[byteOffset++] = param_a[index];
                msgBuff[byteOffset++] = param_b[index];
            }
        } else {
            msgBuff = new byte[numMsgs * 3];
            int byteOffset = 0;
            for(int index = 0; index < numMsgs; index++) {
                msgBuff[byteOffset++] =
                        MidiSpec.makeChanMessageCode(msgID, channel);
                msgBuff[byteOffset++] = param_a[index];
                msgBuff[byteOffset++] = param_b[index];
            }
        }
        return msgBuff;
    }

    /**
     * Create a MIDI data stream, containing a set of 2-value messages (potentially using
     * Running Status) each of the specific message-code/channel with associated 1-byte parameter
     * pairs.
     *
     * @param msgID The MIDI message ID.
     * @param channel   The MIDI channel number for the message(s).
     * @param param_a   The parameter bytes.
     * @param useRunningStatus  If true, specifies that message streams should use running-status.
     * @return The MIDI data stream.
     */
    static byte[] make2ByteMsgBuff(
            byte msgID, byte channel, byte[] param_a, boolean useRunningStatus) {
        int numMsgs = param_a.length;

        byte[] msgBuff;
        if (useRunningStatus) {
            msgBuff = new byte[1 + numMsgs];
            int byteOffset = 0;
            msgBuff[byteOffset++] =
                    MidiSpec.makeChanMessageCode(msgID, channel);
            for(int index = 0; index < numMsgs; index++) {
                msgBuff[byteOffset++] = param_a[index];
            }
        } else {
            msgBuff = new byte[numMsgs * 2];
            int byteOffset = 0;
            for(int index = 0; index < numMsgs; index++) {
                msgBuff[byteOffset++] =
                        MidiSpec.makeChanMessageCode(msgID, channel);
                msgBuff[byteOffset++] = param_a[index];
            }
        }
        return msgBuff;
    }
}