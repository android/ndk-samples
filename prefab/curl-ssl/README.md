# curl-ssl

The curl-ssl sample shows how to import native dependencies from Maven. This
sample uses [curl], [OpenSSL], and [JsonCpp] to display a list of the most
recent 10 reviews submitted to AOSP's code review system.

[curl]: https://curl.haxx.se/
[OpenSSL]: https://www.openssl.org/
[JsonCpp]: https://github.com/open-source-parsers/jsoncpp

## Pre-requisites

* Android Gradle Plugin 4.0+
* The [Android NDK](https://developer.android.com/ndk/).

## Getting Started

The C++ code in this sample can be built with either CMake (the default for this
project) or ndk-build. To use ndk-build set the `ndkBuild` project property
either in your `local.properties` file or on the command line by passing the
`-PndkBuild` flag when building.

To build with [Android Studio](http://developer.android.com/sdk/index.html):

1. Open this project in Android Studio.
2. Click *Run/Run 'app'.

To build from the command line:

1. Navigate to this directory in your terminal.
2. Run `./gradlew installDebug` (or `gradlew.bat installDebug` on Windows).

## CA Certificates

As explained by [this Stack Overflow
post](https://stackoverflow.com/a/30430033/632035), the root certificates
presented by Android since ICS are not in the format OpenSSL expects, so we need
to provide our own certificate file. We do this by downloading curl's cacert.pem
and storing that in our assets directory, as described in [this Stack Overflow
post](https://stackoverflow.com/a/31521185/632035).

## Screenshots

![screenshot](screenshot.png)

## Support

If you've found an error in these samples, please [file an
issue](https://github.com/android/ndk-samples/issues/new).

Patches are encouraged, and may be submitted by submitting a pull request
through GitHub. Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for more
details.
