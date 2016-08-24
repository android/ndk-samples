Note:it is not quite working, need some debug

This sample app requires an MPEG-2 Transport Stream file to be
placed in /sdcard/NativeMedia.ts and encoded as:

  video: H.264 baseline profile
  audio: AAC LC stereo

Any actual stream must be created according to the MPEG-2 specification.

Known Issues:
1) Java player crashed: asset could not open on Java side, complaining about the compressed format

