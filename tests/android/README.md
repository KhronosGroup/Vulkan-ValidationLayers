# Running Native Code Android

On Android the typical way to run Native code is to create a shared library with specific symbols exported.
This is actually similar to how VVL exports specific functions the vulkan loader knows how to find.
The main difference is instead of the loader, it's Java.

- https://github.com/android/ndk-samples/blob/main/camera/basic/src/main/cpp/CMakeLists.txt <- In this example the symbol exported is `ANativeActivity_onCreate`
- https://github.com/android/ndk-samples/blob/main/hello-vulkan/app/src/main/cpp/CMakeLists.txt <- In this example the symbol exported is `Java_com_google_androidgamesdk_GameActivity_initializeNativeCode`

NOTE: In the NDK examples they use `add_library(foobar SHARED)`. They should be `MODULE`s however. EX: `add_library(foobar MODULE)`. Since the libraries aren't intended to be linked against.
They are intended to be loaded as plugins by the java runtime.

## (WSI) ANativeActivity_onCreate

As was mentioned before, exporting a symbol in your shared library that gets loaded by Java is the idiom for running native code on Android.

In our case `ANativeActivity_onCreate` is the symbol we export for our tests.

`ANativeActivity_onCreate` is defined by `android_native_app_glue.c`

Note: At one point it was neccessary to force export this symbol due to an NDK bug.
https://github.com/android-ndk/ndk/issues/381

For WSI, we need to call `vkCreateAndroidSurfaceKHR` which takes a `ANativeWindow` handle

Android has a life-cycle system for apps (start, background, etc). With the `ANativeActivity_onCreate` we are getting hooks into it

This example should help illustrate things:
https://github.com/sjfricke/Vulkan-NDK-Template/blob/master/app/src/main/cpp/AndroidMain.cpp

1. We need to get the mapping from the Java events so the C++ code can see it

```
    // Set the callback to process system events
    app->onAppCmd = handle_cmd;
```

2. From here we can use `NativeAppGlueAppCmd` (https://developer.android.com/reference/games/game-activity/group/android-native-app-glue) (or just look in `android_native_app_glue.c`)

3. For `APP_CMD_INIT_WINDOW` we can go `android_app->window` to get the `ANativeWindow` handle

Android can have many apps running, but only the ones in the foreground on the device get access to the `ANativeWindow` as that is what decides what is being displayed by SurfaceFlinger (in the AOSP).

When doing a command line executable, we have no proper way to get that handle, and therefore can't create a `VkSurface`.

## Running executables on Android

You can run executables on Android: https://github.com/android/ndk/discussions/1726

Here is an example of using adb with the android emulator: https://github.com/microsoft/GSL/blob/main/.github/workflows/android.yml

Furthermore it seems possible to run ctest on Android as well:
- https://github.com/openxla/iree/pull/9372/files
- https://gitlab.xiph.org/xiph/opus/-/merge_requests/28/diffs

However, WSI functionality for Android requires running the tests as an APK. So we need to build our tests as a `MODULE` library.

We could potentially build both a regular executable / APK in the future if there is enough benefit.
