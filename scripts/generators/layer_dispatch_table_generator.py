#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
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

import sys
import os
from generators.generator_utils import (fileIsGeneratedWarning)
from generators.base_generator import BaseGenerator

class LayerDispatchTableOutputGenerator(BaseGenerator):
    """Generate dispatch tables header based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)

    def generate(self):
        out = []
        out.append(f'''{fileIsGeneratedWarning(os.path.basename(__file__))}
/***************************************************************************
*
* Copyright (c) 2015-2023 The Khronos Group Inc.
* Copyright (c) 2015-2023 Valve Corporation
* Copyright (c) 2015-2023 LunarG, Inc.
* Copyright (c) 2015-2023 Google Inc.
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
****************************************************************************/\n''')
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        out.append('''
#pragma once

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);
''')
        out.append('''
// Instance function pointer dispatch table
typedef struct VkLayerInstanceDispatchTable_ {
    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;

''')
        for command in [x for x in self.vk.commands.values() if x.instance]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
            out.extend([f'#endif //{command.protect}\n'] if command.protect else [])
        out.append('} VkLayerInstanceDispatchTable;\n')

        out.append('''
// Device function pointer dispatch table
typedef struct VkLayerDispatchTable_ {
''')
        for command in [x for x in self.vk.commands.values() if x.device]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
            out.extend([f'#endif //{command.protect}\n'] if command.protect else [])
        out.append('} VkLayerDispatchTable;\n')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))