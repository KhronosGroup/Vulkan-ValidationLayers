# Copyright 2023 The Android Open Source Project
# Copyright (C) 2023 Valve Corporation
# Copyright (C) 2023 LunarG, Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# To save time, modify the following line to exclude unnessary ABIs
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64

APP_PLATFORM := android-26
# if specified, build with static c++ library
ifeq ($(ANDROID_STL_TYPE),STATIC)
  APP_STL := c++_static
else
  APP_STL := c++_shared
endif

NDK_TOOLCHAIN_VERSION := clang
NDK_MODULE_PATH := .
