# Architecture

This document describes the layout of the samples repository.

The top level directory is a Gradle project. This directory can be opened in
Android Studio (and that will be the easiest way to work with the samples).

## Directory structure

### Samples

Most subdirectories are the individual sample apps. These can't be opened in
Android Studio individually, as they rely on common code from the top level
project. These subdirectories have their own `build.gradle` (the Groovy DSL) or
`build.gradle.kts` (the Kotlin DSL) files. In Gradle terms these are "projects".
Android Studio calls them "modules". The documentation in this repository will
typically call them "modules".

Each sample has its own README.md that explains what features it demonstrates,
as well as any unique requirements.

To run a sample, use the configuration selector in the top panel of Android
Studio to select the sample and then click run. For example, to run the
endless-tunnel sample game:

![Select and run a sample][docs/run-sample.png]

#### Build types

Android Gradle modules have multiple "build types", sometimes called "variants".
For most of the samples, there are only two build types: debug and release. A
few samples, notably the sanitizers sample, have more. To view the build types
for the modules in this repository, in the Android Studio application menu,
select View -> Tool Windows -> Build Variants. A window will open that lets you
select the active variant for each sample.

### build-logic

The `build-logic` directory contains Gradle convention plugins used by this
repository. This is where Gradle policy decisions that apply to the whole
repository are made.

See the README.md in that directory for more details.

### docs

Documentation and supporting artifacts for this repository. Yes, for now it's
just images for the READMEs and this doc.

### gradle/libs.versions.toml

This is a Gradle [version catalog]. It's the central location that defines the
library and plugin dependencies for each sample.

The Android Gradle Plugin does not support evaluating version catalog fields in
its DSL, so versions for SDK and NDK tools (`ndkVersion`, `compileSdkVersion`,
`targetSdkVersion`, etc) are all defined by the convention plugins in
[build-logic](#build-logic).

[version catalog]: https://docs.gradle.org/current/userguide/platforms.html

### Gradle wrapper

The `gradlew` and `gradlew.bat` scripts are the Gradle wrappers, macOS/Linux-
and Windows batch-compatible respectively. They will manage the Gradle
installation used by this repository for you, ensuring that the correct versions
and environment are used. These should be used instead of running `gradle`
directly.

`gradle/wrapper/gradle-wrapper.properties` defines which version of Gradle will
be used. This should rarely be modified manually. Android Studio's Upgrade
Assistant will manage that file and upgrade to whatever version of Gradle it
prefers for that version of Android Studio and Android Gradle.

### build.gradle and settings.gradle

The build.gradle (or build.gradle.kts) file is the top level build file. It
doesn't do anything interesting but declares which plugins will be used by the
child modules. Each sample's module has its own build.gradle file that defines
properties of the app.

The settings.gradle (or settings.gradle.kts) file configures repositories for
fetching dependencies, and declares each app module.

### Metadata directories

#### .github

This directory contains GitHub metadata files, such as GitHub Action workflows
and Issue templates.

#### .google

The `packaging.yaml` file in this directory is used by Android Studio to enable
the File -> New -> Import Sample feature. As this repository is a monolithic
sample project, it should not need to be changed.
