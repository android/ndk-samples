# CMake modules

This directory contains CMake modules used by the samples. They can be imported
in any sample with `include(<NAME>)`, where `<NAME>` is the name of the module
file without the extension. For example, `include(absl)` will include absl in
the build and make those libraries available to the sample.
