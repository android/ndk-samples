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

import android.app.Activity;
import android.content.Context;

import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;

import android.os.Bundle;

import android.view.View;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import android.os.Handler;

import java.util.ArrayList;

/**
 * Application MainActivity handles UI and Midi device hotplug event from
 * native side.
 */
public class MainActivity extends Activity
    implements View.OnClickListener,
               SeekBar.OnSeekBarChangeListener,
               AdapterView.OnItemSelectedListener {

    private static final String TAG = MainActivity.class.getName();;

    private AppMidiManager mAppMidiManager;

    // Connected devices
    private ArrayList<MidiDeviceInfo> mReceiveDevices = new ArrayList<MidiDeviceInfo>();
    private ArrayList<MidiDeviceInfo> mSendDevices = new ArrayList<MidiDeviceInfo>();

    // Send Widgets
    Spinner mOutputDevicesSpinner;

    SeekBar mControllerSB;
    SeekBar mPitchBendSB;

    EditText mProgNumberEdit;

    // Receive Widgets
    Spinner mInputDevicesSpinner;
    TextView mReceiveMessageTx;

    // Force to load the native library
    static {
        AppMidiManager.loadNativeAPI();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //
        // Init JNI for data receive callback
        //
        initNative();

        //
        // Setup UI
        //
        mOutputDevicesSpinner = (Spinner)findViewById(R.id.outputDevicesSpinner);
        mOutputDevicesSpinner.setOnItemSelectedListener(this);

        ((Button)findViewById(R.id.keyDownBtn)).setOnClickListener(this);
        ((Button)findViewById(R.id.keyUpBtn)).setOnClickListener(this);
        ((Button)findViewById(R.id.progChangeBtn)).setOnClickListener(this);

        mControllerSB = (SeekBar)findViewById(R.id.controllerSeekBar);
        mControllerSB.setMax(MidiSpec.MAX_CC_VALUE);
        mControllerSB.setOnSeekBarChangeListener(this);

        mPitchBendSB = (SeekBar)findViewById(R.id.pitchBendSeekBar);
        mPitchBendSB.setMax(MidiSpec.MAX_PITCHBEND_VALUE);
        mPitchBendSB.setProgress(MidiSpec.MID_PITCHBEND_VALUE);
        mPitchBendSB.setOnSeekBarChangeListener(this);

        mInputDevicesSpinner = (Spinner)findViewById(R.id.inputDevicesSpinner);
        mInputDevicesSpinner.setOnItemSelectedListener(this);

        mProgNumberEdit = (EditText)findViewById(R.id.progNumEdit);

        mReceiveMessageTx = (TextView)findViewById(R.id.receiveMessageTx);

        MidiManager midiManager = (MidiManager) getSystemService(Context.MIDI_SERVICE);
        midiManager.registerDeviceCallback(new MidiDeviceCallback(), new Handler());

        //
        // Setup the MIDI interface
        //
        mAppMidiManager = new AppMidiManager(midiManager);

        // Initial Scan
        ScanMidiDevices();
    }

    /**
     * Device Scanning
     * Methods are called by the system whenever the set of attached devices changes.
     */
    private class MidiDeviceCallback extends MidiManager.DeviceCallback {
        @Override
        public void onDeviceAdded(MidiDeviceInfo device) {
            ScanMidiDevices();
        }

        @Override
        public void onDeviceRemoved(MidiDeviceInfo device) {
            ScanMidiDevices();
        }
    }

    /**
     * Scans and gathers the list of connected physical devices,
     * then calls onDeviceListChange() to update the UI. This has the
     * side-effect of causing a list item to be selected, which then
     * invokes the listener logic which connects the device(s).
     */
    private void ScanMidiDevices() {
        mAppMidiManager.ScanMidiDevices(mSendDevices, mReceiveDevices);
        onDeviceListChange();
    }

    //
    // UI Helpers
    //
    /**
     * Formats a set of MIDI message bytes into a user-readable form.
     * @param message   The bytes comprising a Midi message.
     */
    private void showReceivedMessage(byte[] message) {
        switch ((message[0] & 0xF0) >> 4) {
            case MidiSpec.MIDICODE_NOTEON:
                mReceiveMessageTx.setText(
                        "NOTE_ON [ch:" + (message[0] & 0x0F) +
                                " key:" + message[1] +
                                " vel:" + message[2] + "]");
                break;

            case MidiSpec.MIDICODE_NOTEOFF:
                mReceiveMessageTx.setText(
                        "NOTE_OFF [ch:" + (message[0] & 0x0F) +
                                " key:" + message[1] +
                                " vel:" + message[2] + "]");
                break;

            // Potentially handle other messages here.
        }
    }

    //
    // View.OnClickListener overriden methods
    //
    @Override
    public void onClick(View view) {
        byte[] keys = {60, 64, 67};         // C Major chord
        byte[] velocities = {60, 60, 60};   // Middling velocity
        byte channel = 0;    // send on channel 0
        switch (view.getId()) {
            case R.id.keyDownBtn:
                // Simulate a key-down
                mAppMidiManager.sendNoteOn(channel, keys, velocities) ;
                break;

            case R.id.keyUpBtn:
                // Simulate a key-up (converse of key-down above).
                mAppMidiManager.sendNoteOff(channel, keys, velocities) ;
                break;

            case R.id.progChangeBtn: {
                // Send a MIDI program change message
                try {
                    String progNumStr = mProgNumberEdit.getText().toString();
                    int progNum = Integer.parseInt(progNumStr);

                    mAppMidiManager.sendProgramChange(channel, (byte)progNum);
                } catch (NumberFormatException ex) {
                    // Maybe let the user know
                }
            }
                break;
        }
    }

    //
    // SeekBar.OnSeekBarChangeListener overriden messages
    //
    @Override
    public void onProgressChanged(SeekBar seekBar, int pos, boolean fromUser) {
        switch (seekBar.getId()) {
        case R.id.controllerSeekBar:
            mAppMidiManager.sendController((byte)0, MidiSpec.MIDICC_MODWHEEL, (byte)pos);
            break;

        case R.id.pitchBendSeekBar:
            mAppMidiManager.sendPitchBend((byte)0, pos);
            break;
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {}

    //
    // AdapterView.OnItemSelectedListener overriden methods
    //
    @Override
    public void onItemSelected(AdapterView<?> spinner, View view, int position, long id) {
        switch (spinner.getId()) {
        case R.id.outputDevicesSpinner: {
                MidiDeviceListItem listItem = (MidiDeviceListItem) spinner.getItemAtPosition(position);
                mAppMidiManager.openReceiveDevice(listItem.getDeviceInfo());
            }
            break;

        case R.id.inputDevicesSpinner: {
                MidiDeviceListItem listItem = (MidiDeviceListItem)spinner.getItemAtPosition(position);
                mAppMidiManager.openSendDevice(listItem.getDeviceInfo());
            }
            break;
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {}

    /**
     * A class to hold MidiDevices in the list controls.
     */
    private class MidiDeviceListItem {
        private MidiDeviceInfo mDeviceInfo;

        public MidiDeviceListItem(MidiDeviceInfo deviceInfo) {
            mDeviceInfo = deviceInfo;
        }

        public MidiDeviceInfo getDeviceInfo() { return mDeviceInfo; }

        @Override
        public String toString() {
            return mDeviceInfo.getProperties().getString(MidiDeviceInfo.PROPERTY_NAME);
        }
    }

    /**
     * Fills the specified list control with a set of MidiDevices
     * @param spinner   The list control.
     * @param devices   The set of MidiDevices.
     */
    private void fillDeviceList(Spinner spinner, ArrayList<MidiDeviceInfo> devices) {
        ArrayList<MidiDeviceListItem> listItems = new ArrayList<MidiDeviceListItem>();
        for(MidiDeviceInfo devInfo : devices) {
            listItems.add(new MidiDeviceListItem(devInfo));
        }

        // Creating adapter for spinner
        ArrayAdapter<MidiDeviceListItem> dataAdapter =
                new  ArrayAdapter<MidiDeviceListItem>(this,
                        android.R.layout.simple_spinner_item,
                        listItems);
        // Drop down layout style - list view with radio button
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // attaching data adapter to spinner
        spinner.setAdapter(dataAdapter);
    }

    /**
     * Fills the Input & Output UI device list with the current set of MidiDevices for each type.
     */
    private void onDeviceListChange() {
        fillDeviceList(mOutputDevicesSpinner, mReceiveDevices);
        fillDeviceList(mInputDevicesSpinner, mSendDevices);
    }

    //
    // Native Interface methods
    //
    private native void initNative();

    /**
     * Called from the native code when MIDI messages are received.
     * @param message
     */
    private void onNativeMessageReceive(final byte[] message) {
        // Messages are received on some other thread, so switch to the UI thread
        // before attempting to access the UI
        runOnUiThread(new Runnable() {
            public void run() {
                showReceivedMessage(message);
            }
        });
    }
}
