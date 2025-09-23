## Description

### Introduction

The Native MIDI sample is a simple Android application which demonstrates how to
use
[the Android Native MIDI API](http://developer.android.com/preview/features/midi),
a new feature in Android 29.

Note that [AMidi](http://developer.android.com/preview/features/midi) is a
simple data transport mechanism. It works in terms of streams of bytes and does
not impose any MIDI semantics beyond that whole MIDI messages are transported as
a unit. Indeed, basic MIDI constructs such as message IDs, or message formatting
are not represented in either API. However, since the sample does deal with MIDI
semantics, there are definitions and mechanism to support these semantics.

### An Important Nomenclature Observation

AMidi inherits most of its terminology from the existing Android Java MIDI API.
This can cause confusion regarding the naming of port types and their
directionality. The Android Java MIDI API regards the directionality of the
ports from the perspective of the connected MIDI peripheral, not the perspective
of the application. AMidi inherits this perspective so an application uses an
AMidiOutputPort to receive MIDI data from the output port of a device and uses
an AMidiInputPort to send data to the input port of a device. AMidi uses the
verbs “send” and “receive” to make this more clear to the application
programmer, and these conventions are used in the NMTb. Prerequisites By
definition, the Android Native MIDI API requires the use of the Java Native
Interface (JNI). It is assumed that anyone implementing a Native MIDI
application will be familiar with JNI.

### Hardware Setup

This sample requires an input and an output MIDI device connected to Android
devices running Android Qt+. An example configure can be:

![Configure](config.png)

## Screenshots

![screenshot](screenshot.png)
