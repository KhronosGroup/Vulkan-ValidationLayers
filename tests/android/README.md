# Running Native Code Android

On Android the typical way to run Native code is to create a shared library with specific symbols exported.
This is actually similar to how VVL exports specific functions the vulkan loader knows to find.
The main difference is instead of the loader, it's Java.

- https://github.com/android/ndk-samples/blob/main/camera/basic/src/main/cpp/CMakeLists.txt <- In this example the symbol exported is `ANativeActivity_onCreate`
- https://github.com/android/ndk-samples/blob/main/hello-vulkan/app/src/main/cpp/CMakeLists.txt <- In this example the symbol exported is `Java_com_google_androidgamesdk_GameActivity_initializeNativeCode`

However you can run executables on Android:
https://github.com/android/ndk/discussions/1726

From one of the NDK maintainers: "code executed with "adb shell" runs in a different, more permissive environment than an Android app."

Which seems perfect for us. Since we aren't really shipping an Android app. We just want to test on Android.

Here is an example of using adb and the android emulator:
https://github.com/microsoft/GSL/blob/main/.github/workflows/android.yml

Furthermore it seems possible to run ctest on Android as well:
https://github.com/openxla/iree/pull/9372/files
https://gitlab.xiph.org/xiph/opus/-/merge_requests/28/diffs

## ANativeActivity_onCreate

As was mentioned before, exporting a symbol in your shared library that gets loaded by Java is the idiom for running native code on Android.

In our case `ANativeActivity_onCreate` is the symbol we export for our tests.

`ANativeActivity_onCreate` is defined by `android_native_app_glue.c`

Note: At one point it was neccessary to force export this symbol due to an NDK bug.
https://github.com/android-ndk/ndk/issues/381
