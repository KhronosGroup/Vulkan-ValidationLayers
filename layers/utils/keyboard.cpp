/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
#if defined(DEBUG_CAPTURE_KEYBOARD)
#include "keyboard.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#endif

static bool previous_key_state = true;

static bool GetKeyState(void* wsi_display) {
    bool result = false;

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (wsi_display) {
        char keys_return[32];
        Display* xlib_display = (Display*)wsi_display;
        XQueryKeymap(xlib_display, keys_return);

        KeyCode keycode = XKeysymToKeycode(xlib_display, XK_F1);
        int byte = keycode / 8;
        int bit = keycode % 8;

        return (keys_return[byte] & (1 << bit)) != 0;
    }
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    (void)wsi_display;
    result |= (GetAsyncKeyState(VK_F1) != 0);
#endif

    return result;
}

bool IsDebugKeyPressed(void* wsi_display) {
    // Return true when  transitions from false to true
    const bool hotkey_state = GetKeyState(wsi_display);
    const bool hotkey_pressed = hotkey_state && !previous_key_state;
    previous_key_state = hotkey_state;
    return hotkey_pressed;
}
#endif