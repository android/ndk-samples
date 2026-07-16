# Fix CMake sync error: "Error evaluating generator expression"

The project is using the `$<LINK_LIBRARY:WHOLE_ARCHIVE, ...>` generator expression, which was introduced in CMake 3.24. However, the project's build logic (in `gradle/libs.versions.toml`) is currently forcing the use of CMake 3.22.1. Additionally, several `CMakeLists.txt` files still specify a minimum version of 3.22.1 while using this newer feature.

According to the project's `README.md` and `.github/workflows/build.yml`, the intended CMake version for this project is `4.1.0`.

## Proposed Changes

### Build Configuration

#### [MODIFY] [libs.versions.toml](file:///Users/jakeadamson/Documents/GitHub/ndk-samples/gradle/libs.versions.toml)
- Update `cmake` version from `3.22.1` to `4.1.0` to match the project's requirements and enable support for modern generator expressions.

### CMake Scripts

#### [MODIFY] [CMakeLists.txt (endless-tunnel)](file:///Users/jakeadamson/Documents/GitHub/ndk-samples/endless-tunnel/app/src/main/cpp/CMakeLists.txt)
- Update `cmake_minimum_required` to `VERSION 4.1.0`.

#### [MODIFY] [CMakeLists.txt (camera/basic)](file:///Users/jakeadamson/Documents/GitHub/ndk-samples/camera/basic/src/main/cpp/CMakeLists.txt)
- Update `cmake_minimum_required` to `VERSION 4.1.0`.

#### [MODIFY] [Other CMakeLists.txt files]
- I will perform a project-wide update of `cmake_minimum_required(VERSION 3.22.1)` to `VERSION 4.1.0` to ensure consistency and compliance with the project's stated requirements in the `README.md`.

## Verification Plan

### Automated Verification
- Run `./gradlew :prepareKotlinBuildScriptModel` to ensure the sync error is resolved.
- Run a build of the affected modules (e.g., `./gradlew :endless-tunnel:app:assembleDebug`).

### Manual Verification
- Verify that Android Studio can successfully sync the project without generator expression errors.
