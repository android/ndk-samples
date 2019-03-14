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
#include <unistd.h>
#include <atomic>
#include <inttypes.h>
#include <stdio.h>
#include <string>

#include <jni.h>

#include <pthread.h>

#define LOG_TAG "AppMidiManager-JNI"
#include "AndroidDebug.h"

#include <amidi/AMidi.h>

#include "MidiSpec.h"

static AMidiDevice* sNativeReceiveDevice = NULL;
// The thread only reads this value, so no special protection is required.
static AMidiOutputPort* sMidiOutputPort= NULL;

static AMidiDevice* sNativeSendDevice = NULL;
static AMidiInputPort* sMidiInputPort = NULL;

static pthread_t sReadThread;
static std::atomic<bool> sReading(false);

// The Data Callback
extern JavaVM* theJvm;              // Need this for allocating data buffer for...
extern jobject dataCallbackObj;     // This is the (Java) object that implements...
extern jmethodID midDataCallback;   // ...this callback routine

static void SendTheReceivedData(uint8_t* data, int numBytes) {
    JNIEnv* env;
    theJvm->AttachCurrentThread(&env, NULL);
    if (env == NULL) {
        LOGE("Error retrieving JNI Env");
    }

    // Allocate the Java array and fill with received data
    jbyteArray ret = env->NewByteArray(numBytes);
    env->SetByteArrayRegion (ret, 0, numBytes, (jbyte*)data);

    // send it to the (Java) callback
    env->CallVoidMethod(dataCallbackObj, midDataCallback, ret);
}

#if 0
// unblock this method if logging of the midi messages is required.
/**
 * Formats a midi message set and outputs to the log
 * @param   timestamp   The timestamp for when the message(s) was received
 * @param   dataBytes   The MIDI message bytes
 * @params  numDataBytew    The number of bytes in the MIDI message(s)
 */
static void logMidiBuffer(int64_t timestamp, uint8_t* dataBytes, size_t numDataBytes) {
#define DUMP_BUFFER_SIZE    1024
    char midiDumpBuffer[DUMP_BUFFER_SIZE];
    memset(midiDumpBuffer, 0, sizeof(midiDumpBuffer));
    int pos = snprintf(midiDumpBuffer, DUMP_BUFFER_SIZE,
            "%" PRIx64 " ", timestamp);
    for (uint8_t *b = dataBytes, *e = b + numDataBytes; b < e; ++b) {
        pos += snprintf(midiDumpBuffer + pos, DUMP_BUFFER_SIZE - pos,
                "%02x ", *b);
    }
    LOGD("%s", midiDumpBuffer);
}
#endif

/*
 * Receiving API
 */
 /**
  * This routine polls the input port and dispatches received data to the application-provided
  * (Java) callback.
  */
static void* readThreadRoutine(void * context) {
    (void)context;  // unused

    sReading = true;
    // AMidiOutputPort* outputPort = sMidiOutputPort.load();
    AMidiOutputPort* outputPort = sMidiOutputPort;

    const size_t MAX_BYTES_TO_RECEIVE = 128;
    uint8_t incomingMessage[MAX_BYTES_TO_RECEIVE];

    while (sReading) {
        // AMidiOutputPort_receive is non-blocking, so let's not burn up the CPU unnecessarily
        usleep(2000);

        int32_t opcode;
        size_t numBytesReceived;
        int64_t timestamp;
        ssize_t numMessagesReceived =
                AMidiOutputPort_receive(outputPort,
                    &opcode, incomingMessage, MAX_BYTES_TO_RECEIVE,
                    &numBytesReceived, &timestamp);

        if (numMessagesReceived < 0) {
            LOGW("Failure receiving MIDI data %zd", numMessagesReceived);
            // Exit the thread
            sReading = false;
        }
        if (numMessagesReceived > 0 && numBytesReceived >= 0) {
            if (opcode == AMIDI_OPCODE_DATA &&
                (incomingMessage[0] & kMIDISysCmdChan) != kMIDISysCmdChan) {
                // (optionally) Dump to log
                // logMidiBuffer(timestamp, incomingMessage, numBytesReceived);
                SendTheReceivedData(incomingMessage, numBytesReceived);
            } else if (opcode == AMIDI_OPCODE_FLUSH) {
                // ignore
            }
        }
    }   // end while(sReading)

    return NULL;
}

//
// JNI Functions
//
extern "C" {

/**
 * Native implementation of TBMidiManager.startReadingMidi() method.
 * Opens the first "output" port from specified MIDI device for sReading.
 * @param   env  JNI Env pointer.
 * @param   (unnamed)   TBMidiManager (Java) object.
 * @param   midiDeviceObj   (Java) MidiDevice object.
 * @param   portNumber      The index of the "output" port to open.
 */
void Java_com_example_nativemidi_AppMidiManager_startReadingMidi(
        JNIEnv* env, jobject, jobject midiDeviceObj, jint portNumber) {

    media_status_t status;
    status = AMidiDevice_fromJava(env, midiDeviceObj, &sNativeReceiveDevice);
    // int32_t deviceType = AMidiDevice_getType(sNativeReceiveDevice);
    // ssize_t numPorts = AMidiDevice_getNumOutputPorts(sNativeReceiveDevice);

    AMidiOutputPort *outputPort;
    status = AMidiOutputPort_open(sNativeReceiveDevice, portNumber, &outputPort);

    // sMidiOutputPort.store(outputPort);
    sMidiOutputPort = outputPort;

    // Start read thread
    // pthread_init(true);
    /*int pthread_result =*/ pthread_create(&sReadThread, NULL, readThreadRoutine, NULL);
}

/**
 * Native implementation of the (Java) TBMidiManager.stopReadingMidi() method.
 * @param   (unnamed)   JNI Env pointer.
 * @param   (unnamed)   TBMidiManager (Java) object.
 */
void Java_com_example_nativemidi_AppMidiManager_stopReadingMidi(JNIEnv*, jobject) {
    // need some synchronization here
    sReading = false;
    pthread_join(sReadThread, NULL);

    /*media_status_t status =*/ AMidiDevice_release(sNativeReceiveDevice);
    sNativeReceiveDevice = NULL;
}

/*
 * Sending API
 */
/**
 * Native implementation of TBMidiManager.startWritingMidi() method.
 * Opens the first "input" port from specified MIDI device for writing.
 * @param   env  JNI Env pointer.
 * @param   (unnamed)   TBMidiManager (Java) object.
 * @param   midiDeviceObj   (Java) MidiDevice object.
 * @param   portNumber      The index of the "input" port to open.
 */
void Java_com_example_nativemidi_AppMidiManager_startWritingMidi(
        JNIEnv* env, jobject, jobject midiDeviceObj, jint portNumber) {

    media_status_t status;
    status = AMidiDevice_fromJava(env, midiDeviceObj, &sNativeSendDevice);
    // int32_t deviceType = AMidiDevice_getType(sNativeReceiveDevice);
    // ssize_t numPorts = AMidiDevice_getNumInputPorts(sNativeSendDevice);

    AMidiInputPort *inputPort;
    status = AMidiInputPort_open(sNativeSendDevice, portNumber, &inputPort);
    // sMidiInputPort.store(inputPort);
    sMidiInputPort = inputPort;
}

/**
 * Native implementation of the (Java) TBMidiManager.stopWritingMidi
 * @param   (unnamed)   JNI Env pointer.
 * @param   (unnamed)   TBMidiManager (Java) object.
 */
void Java_com_example_nativemidi_AppMidiManager_stopWritingMidi(JNIEnv*, jobject) {
    /*media_status_t status =*/ AMidiDevice_release(sNativeSendDevice);
    sNativeSendDevice = NULL;
}

/**
 * Native implementation of the (Java) TBMidiManager.writeMidi() method.
 * Writes a byte buffer to the (already open) "input" port.
 * @param   env  JNI Env pointer.
 * @param   (unnamed)   TBMidiManager (Java) object.
 * @param   data    The data buffer.
 * @param   numBytes    The number of bytes to send.
 */
void Java_com_example_nativemidi_AppMidiManager_writeMidi(JNIEnv* env, jobject,
        jbyteArray data, jint numBytes) {
    jbyte* bufferPtr = env->GetByteArrayElements(data, NULL);
    /*ssize_t numSent =*/ AMidiInputPort_send(sMidiInputPort, (uint8_t*)bufferPtr, numBytes);
    env->ReleaseByteArrayElements(data, bufferPtr, JNI_ABORT);
}

} // extern "C"
