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

#if defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/keysym.h>
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <stdlib.h>
#endif

static bool previous_key_state = true;

static bool GetKeyState(void* xlib_display, void* xcb_connection) {
    bool result = false;

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (xlib_display) {
        char keys_return[32];
        Display* dpy = (Display*)xlib_display;
        XQueryKeymap(dpy, keys_return);

        KeyCode keycode = XKeysymToKeycode(dpy, XK_F1);
        int byte = keycode / 8;
        int bit = keycode % 8;

        return (keys_return[byte] & (1 << bit)) != 0;
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (xcb_connection) {
        xcb_connection_t* connection = (xcb_connection_t*)xcb_connection;
        xcb_key_symbols_t* symbols = xcb_key_symbols_alloc(connection);

        xcb_query_keymap_cookie_t cookie = xcb_query_keymap(connection);
        xcb_query_keymap_reply_t* reply = xcb_query_keymap_reply(connection, cookie, NULL);
        if (reply) {
            xcb_keycode_t* keycodes = xcb_key_symbols_get_keycode(symbols, XK_F1);
            if (keycodes) {
                for (int i = 0; keycodes[i] != XCB_NO_SYMBOL; ++i) {
                    int byte = keycodes[i] / 8;
                    int bit = keycodes[i] % 8;
                    if (reply->keys[byte] & (1 << bit)) {
                        result = true;
                        break;
                    }
                }
                free(keycodes);
            }
            free(reply);
        }
        xcb_key_symbols_free(symbols);
    }
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    (void)xlib_display;
    (void)xcb_connection;
    result |= (GetAsyncKeyState(VK_F1) != 0);
#endif

    return result;
}

bool IsDebugKeyPressed(void* xlib_display, void* xcb_connection) {
    // Return true when  transitions from false to true
    const bool hotkey_state = GetKeyState(xlib_display, xcb_connection);
    const bool hotkey_pressed = hotkey_state && !previous_key_state;
    previous_key_state = hotkey_state;
    return hotkey_pressed;
}
#endif