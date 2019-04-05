#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019 Valve Corporation
# Copyright (c) 2015-2017, 2019 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Mark Lobodzinski <mark@lunarg.com>

import os,re,sys,string
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple

# Copyright text prefixing all headers (list of strings).
prefixStrings = [
    '/*',
    '** Copyright (c) 2015-2017, 2019 The Khronos Group Inc.',
    '** Copyright (c) 2015-2017, 2019 Valve Corporation',
    '** Copyright (c) 2015-2017, 2019 LunarG, Inc.',
    '** Copyright (c) 2015-2017, 2019 Google Inc.',
    '**',
    '** Licensed under the Apache License, Version 2.0 (the "License");',
    '** you may not use this file except in compliance with the License.',
    '** You may obtain a copy of the License at',
    '**',
    '**     http://www.apache.org/licenses/LICENSE-2.0',
    '**',
    '** Unless required by applicable law or agreed to in writing, software',
    '** distributed under the License is distributed on an "AS IS" BASIS,',
    '** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.',
    '** See the License for the specific language governing permissions and',
    '** limitations under the License.',
    '*/',
    ''
]


platform_dict = {
    'android' : 'VK_USE_PLATFORM_ANDROID_KHR',
    'fuchsia' : 'VK_USE_PLATFORM_FUCHSIA',
    'ggp': 'VK_USE_PLATFORM_GGP',
    'ios' : 'VK_USE_PLATFORM_IOS_MVK',
    'macos' : 'VK_USE_PLATFORM_MACOS_MVK',
    'metal' : 'VK_USE_PLATFORM_METAL_EXT',
    'vi' : 'VK_USE_PLATFORM_VI_NN',
    'wayland' : 'VK_USE_PLATFORM_WAYLAND_KHR',
    'win32' : 'VK_USE_PLATFORM_WIN32_KHR',
    'xcb' : 'VK_USE_PLATFORM_XCB_KHR',
    'xlib' : 'VK_USE_PLATFORM_XLIB_KHR',
    'xlib_xrandr' : 'VK_USE_PLATFORM_XLIB_XRANDR_EXT',
}

#
# Return appropriate feature protect string from 'platform' tag on feature
def GetFeatureProtect(interface):
    """Get platform protection string"""
    platform = interface.get('platform')
    protect = None
    if platform is not None:
        protect = platform_dict[platform]
    return protect

# Check if an object is a non-dispatchable handle
def IsHandleTypeNonDispatchable(tree, handletype):
    handle = tree.find("types/type/[name='" + handletype + "'][@category='handle']")
    if handle is not None and handle.find('type').text == 'VK_DEFINE_NON_DISPATCHABLE_HANDLE':
        return True
    else:
        return False

# Treats outdents a multiline string by the leading whitespace on the first line
# Optionally indenting by the given prefix
def Outdent(string_in, indent=''):
    string_out = re.sub('^ *', '', string_in) # kill stray  leading spaces
    if string_out[0] != '\n':
        return string_in # needs new line to find the first line's indent level

    first_indent = string_out[1:]
    fake_indent = '\n' + ' ' * (len(first_indent) - len(first_indent.lstrip()))
    indent = '\n' + indent

    string_out = string_out.rstrip() + '\n' # remove trailing whitespace except for a newline
    outdent = re.sub(fake_indent, indent, string_out)
    return outdent[1:]

