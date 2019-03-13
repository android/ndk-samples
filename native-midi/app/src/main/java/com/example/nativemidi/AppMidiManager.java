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

import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.media.midi.MidiInputPort;

import java.util.ArrayList;

public class AppMidiManager {
    private static final String TAG = AppMidiManager.class.getName();

    private MidiManager mMidiManager;

    // Selected Device(s)
    private MidiDevice mReceiveDevice; // an "Output" device is one we will RECEIVE data FROM

    private MidiDevice mSendDevice; // an "Input" device is one we will SEND data TO
    private MidiInputPort mSendPort;

    private boolean mUseRunningStatus = true;

    public AppMidiManager(MidiManager midiManager) {
        mMidiManager = midiManager;
    }

    public MidiManager GetMidiManager() {
        return mMidiManager;
    }

    /**
     * Scan attached Midi devices forcefully from scratch
     * @param sendDevices, container for send devices
     * @param receiveDevices, container for receive devices
     */
    public void ScanMidiDevices(ArrayList<MidiDeviceInfo> sendDevices,
                                 ArrayList<MidiDeviceInfo> receiveDevices) {
        sendDevices.clear();
        receiveDevices.clear();
        MidiDeviceInfo[] devInfos = mMidiManager.getDevices();
        for(MidiDeviceInfo devInfo : devInfos) {
            int numInPorts = devInfo.getInputPortCount();
            String deviceName =
                    devInfo.getProperties().getString(MidiDeviceInfo.PROPERTY_NAME);
            if (deviceName == null) {
                continue;
            }
            if (numInPorts > 0) {
                sendDevices.add(devInfo);
            }

            int numOutPorts = devInfo.getOutputPortCount();
            if (numOutPorts > 0) {
                receiveDevices.add(devInfo);
            }
        }
    }

    //
    // Receive Device
    //
    public class OpenMidiReceiveDeviceListener implements MidiManager.OnDeviceOpenedListener {
        @Override
        public void onDeviceOpened(MidiDevice device) {
            mReceiveDevice = device;
            startReadingMidi(mReceiveDevice, 0/*mPortNumber*/);
        }
    }

    public void openReceiveDevice(MidiDeviceInfo devInfo) {
        mMidiManager.openDevice(devInfo, new OpenMidiReceiveDeviceListener(), null);
    }

    public void closeReceiveDevice() {
        if (mReceiveDevice != null) {
            // Native API
            mReceiveDevice = null;
        }
    }

    //
    // Send Device
    //
    public class OpenMidiSendDeviceListener implements MidiManager.OnDeviceOpenedListener {
        @Override
        public void onDeviceOpened(MidiDevice device) {
            mSendDevice = device;
            startWritingMidi(mSendDevice, 0/*mPortNumber*/);
        }
    }

    public void openSendDevice(MidiDeviceInfo devInfo) {
        mMidiManager.openDevice(devInfo, new OpenMidiSendDeviceListener(), null);
    }

    public void closeSendDevice() {
        if (mSendDevice != null) {
            // Native API
            mSendDevice = null;
        }
    }

    private void sendMessages(byte[] msgBuff) {
        writeMidi(msgBuff, msgBuff.length);
    }

    //
    // Message Sending methods
    //
    public void sendNoteOn(byte chan, byte[] keys, byte[] velocities) {
        byte[] keyMsgBuff = MidiDataHelper.make3ByteMsgBuff(
                MidiSpec.MIDICODE_NOTEON, chan, keys, velocities, mUseRunningStatus);
        sendMessages(keyMsgBuff);
    }

    public void sendNoteOff(byte chan, byte[] keys, byte[] velocities) {
        byte[] keyMsgBuff = MidiDataHelper.make3ByteMsgBuff(
                MidiSpec.MIDICODE_NOTEOFF, chan, keys, velocities, mUseRunningStatus);
        sendMessages(keyMsgBuff);
    }

    public void sendController(byte chan, byte controller, byte value) {
        byte[] controllers = {controller};
        byte[] values = {value};
        byte[] msgBuff = MidiDataHelper.make3ByteMsgBuff(
                MidiSpec.MIDICODE_CONTROLLER, chan, controllers, values, mUseRunningStatus);
        sendMessages(msgBuff);
    }

    public void sendPitchBend(byte chan, int value) {
        byte[] lsbs = {(byte)(value & 0xEF)};
        byte[] msbs = {(byte)((value >> 7) & 0xEF)};
        byte[] msgBuff = MidiDataHelper.make3ByteMsgBuff(
                MidiSpec.MIDICODE_PITCHBEND, chan, lsbs, msbs, mUseRunningStatus);
        sendMessages(msgBuff);
    }

    public void sendProgramChange(byte chan, byte value) {
        byte[] values = {value};
        byte[] msgBuff = MidiDataHelper.make2ByteMsgBuff(
                MidiSpec.MIDICODE_PROGCHANGE, chan, values, mUseRunningStatus);
        sendMessages(msgBuff);
    }

    //
    // Native API stuff
    //
    public static void loadNativeAPI() {
        System.loadLibrary("native_midi");
    }

    public native void startReadingMidi(MidiDevice receiveDevice, int portNumber);
    public native void stopReadingMidi();

    public native void startWritingMidi(MidiDevice sendDevice, int portNumber);
    public native void stopWritingMidi();
    public native void writeMidi(byte[] data, int length);
}
