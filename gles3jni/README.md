# gles3jni

gles3jni is an Android C++ sample that demonstrates how to use OpenGL ES 3.0
from JNI/native code.

The OpenGL ES 3.0 rendering path uses a few new features compared to the OpenGL
ES 2.0 path:

- Instanced rendering and vertex attribute divisor to reduce the number of draw
  calls and uniform changes.
- Vertex array objects to reduce the number of calls required to set up vertex
  attribute state on each frame.
- Explicit assignment of attribute locations, eliminating the need to query
  assignments.

## Screenshots

![screenshot](screenshot.png)
