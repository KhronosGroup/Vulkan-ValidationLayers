#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2021 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
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

import os,re,sys,string
import xml.etree.ElementTree as etree
from collections import namedtuple, OrderedDict

# Copyright text prefixing all headers (list of strings).
prefixStrings = [
    '/*',
    '** Copyright (c) 2015-2021 The Khronos Group Inc.',
    '** Copyright (c) 2015-2023 Valve Corporation',
    '** Copyright (c) 2015-2023 LunarG, Inc.',
    '** Copyright (c) 2015-2021 Google Inc.',
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
    'provisional' : 'VK_ENABLE_BETA_EXTENSIONS',
    'directfb' : 'VK_USE_PLATFORM_DIRECTFB_EXT',
    'screen' : 'VK_USE_PLATFORM_SCREEN_QNX',
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

# Return the _EXTENSION_NAME define for the given extension block
def GetNameDefine(interface):
    for enum in interface.findall('require/enum'):
        if enum.get('name', '').endswith('_EXTENSION_NAME'):
            return enum.get('name')
    raise Exception(f'Could find name define for {extension.get("name")}')

# Return a dict containing the dispatchable/non-dispatchable type of every handle
def GetHandleTypes(tree):
    # Extend OrderedDict with common handle operations
    class HandleDict(OrderedDict):
        def IsDispatchable(self, handle_type):
            return self.get(handle_type) == 'VK_DEFINE_HANDLE'
        def IsNonDispatchable(self, handle_type):
            return self.get(handle_type) == 'VK_DEFINE_NON_DISPATCHABLE_HANDLE'

    handles = HandleDict()
    for elem in tree.findall("types/type/[@category='handle']"):
        if not elem.get('alias'):
            name = elem.get('name')
            handles[name] = elem.find('type').text

    for elem in tree.findall("types/type/[@category='handle']"):
        if elem.get('alias'):
            name = elem.get('name')
            # Get the dispatchable/non-dispatchable type from the alias
            handles[name] = handles[elem.get('alias')]

    return handles

# Return a dict indicating whether a handle is an aliased type
def GetHandleAliased(tree):
    handles = OrderedDict()
    for elem in tree.findall("types/type/[@category='handle']"):
        name = elem.get('name')
        if elem.get('alias'):
            handles[name] = True
        else:
            handles[name] = False

    return handles

# Return a dict containing the parent of every handle
def GetHandleParents(tree):
    # Extend OrderedDict with common handle operations
    class HandleParentDict(OrderedDict):
        def IsParentDevice(self, handle_type):
            next_object = self.get(handle_type)
            while next_object != 'VkDevice' and next_object != 'VkInstance' and next_object != 'VkPhysicalDevice' and next_object is not None:
                next_object = self.get(next_object)
            return next_object == 'VkDevice'
        def GetHandleParent(self, handle_type):
            return self.get(handle_type)

    handle_parents = HandleParentDict()
    for elem in tree.findall("types/type/[@category='handle']"):
        if not elem.get('alias') or not elem.get('parent'):
            name = elem.get('name')
            handle_parents[name] = elem.get('parent')
    return handle_parents

# Return a dict containing the category attribute of every type
def GetTypeCategories(tree):
    type_categories = OrderedDict()
    for elem in tree.findall("types/type"):
        if not elem.get('alias'):
            # name is either an attribute or the text of a child <name> tag
            name = elem.get('name') or (elem.find("name") and elem.find('name').text)
            type_categories[name] = elem.get('category')
    return type_categories

# Return a dict containing platform guard for every type
def GetTypeGuards(tree):
    type_guards = OrderedDict()
    for ext_elem in tree.findall('extensions/extension'):
        ext_guard = platform_dict.get(ext_elem.get('platform'))
        if ext_guard:
            for type_elem in ext_elem.findall('require/type'):
                type_guards[type_elem.get('name')] = ext_guard
    return type_guards

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

# Wrap a statement with the supplied guard, preserving the newline-ness of the input statement
def Guarded(ifdef, value):
    if ifdef is not None:
        if value.endswith('\n'):
            value, trailing_newline = value[:-1], value[-1:]
        else:
            trailing_newline = ''
        return f'#ifdef {ifdef}\n' \
               f'{value}\n' \
               f'#endif{trailing_newline}'
    else:
        return value


# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))
