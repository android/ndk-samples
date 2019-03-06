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

#ifndef NATIVEMIDITESTBED_MIDISPEC_H

//
// MIDI Messages
//
// Channel Commands
static const uint8_t kMIDIChanCmd_NoteOff = 8;
static const uint8_t kMIDIChanCmd_NoteOn = 9;
static const uint8_t kMIDIChanCmd_PolyPress = 10;
static const uint8_t kMIDIChanCmd_Control = 11;
static const uint8_t kMIDIChanCmd_ProgramChange = 12;
static const uint8_t kMIDIChanCmd_ChannelPress = 13;
static const uint8_t kMIDIChanCmd_PitchWheel = 14;
// System Commands
static const uint8_t kMIDISysCmdChan    = 0xF0;
static const uint8_t kMIDISysCmd_SysEx = 0xF0;
static const uint8_t kMIDISysCmd_EndOfSysEx =  0xF7;
static const uint8_t kMIDISysCmd_ActiveSensing = 0xFE;
static const uint8_t kMIDISysCmd_Reset = 0xFF;

#define NATIVEMIDITESTBED_MIDISPEC_H
#endif
