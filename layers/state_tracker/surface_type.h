/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

namespace vvl {

// Kept out of wsi_state.h so the tests can use it without pulling in the whole state tracker
enum class SurfaceType {
    Unknown,

    Android,
    DisplayPlane,
    Headless,
    ImagePipe_FUCHSIA,
    IOS_MVK,
    MacOS_MVK,
    Metal,
    Screen_QNX,
    Wayland,
    Win32,
    Xcb,
    Xlib,
};

}  // namespace vvl
