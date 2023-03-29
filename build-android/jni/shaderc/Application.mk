APP_ABI := all
APP_BUILD_SCRIPT := Android.mk

# if specified, build with static c++ library
ifeq ($(ANDROID_STL_TYPE),STATIC)
  APP_STL := c++_static
else
  APP_STL := c++_shared
endif

APP_PLATFORM := android-23
