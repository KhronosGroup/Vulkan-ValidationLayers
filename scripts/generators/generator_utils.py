#!/usr/bin/python3 -i
#
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
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

from generators.vulkan_object import *

# TODO - Remove common_codegen.py
# This file is trying to replace common_codegen.py using new VulkanObject class

def fileIsGeneratedWarning(file: str) -> str:
    return f'// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n// See {file} for modifications\n'

def getProtectMacro(command: Command, ifdef: bool = False, endif: bool = False) -> str:
    result = ""
    if not isinstance(command.feature, Extension) or (command.feature.protect is None):
        return result
    macro = "ifdef" if ifdef else "endif //"
    result = "#{} {}\n".format(macro, command.feature.protect)
    return result

# Takes the `len` or `altlen` found in the XML and formats it in C++ friendly way
def getFormatedLength(length: str):
    result = None
    if length is not None and length != 'null-terminated':
        # For string arrays, 'len' can look like 'count,null-terminated', indicating that we
        # have a null terminated array of strings.  We strip the null-terminated from the
        # 'len' field and only return the parameter specifying the string count
        result = length if 'null-terminated' not in length else length.split(',')[0]
        # Spec has now notation for len attributes, using :: instead of platform specific pointer symbol
        result = result.replace('::', '->')
    return result