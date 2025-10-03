pluginManagement {
    includeBuild("build-logic")
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "NDK Samples"
include(":base")
include(":bitmap-plasma:app")
include(":camera:basic")
include(":camera:camera-utils")
include(":camera:texture-view")
include(":endless-tunnel:app")
include(":exceptions:app")
include(":gles3jni:app")
include(":hello-gl2:app")
include(":hello-jni:app")
include(":hello-jniCallback:app")
include(":hello-vulkan:app")
include(":native-activity:app")
include(":native-audio:app")
include(":native-codec:app")
include(":native-midi:app")
include(":orderfile:app")
include(":sanitizers:app")
include(":sensor-graph:accelerometer")
include(":teapots:choreographer-30fps")
include(":teapots:classic-teapot")
include(":teapots:image-decoder")
include(":teapots:more-teapots")
include(":teapots:textured-teapot")
include(":unit-test:app")
include(":vectorization")
