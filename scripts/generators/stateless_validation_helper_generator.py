#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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

import os
import sys

from generators.generator_utils import (fileIsGeneratedWarning, getVUID)
from generators.vulkan_object import (Struct)
from generators.base_generator import BaseGenerator

class StatelessValidationHelperOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)

        # Commands to ignore
        self.blacklist = [
            'vkGetInstanceProcAddr',
            'vkGetDeviceProcAddr',
            'vkEnumerateInstanceVersion',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateDeviceExtensionProperties',
            'vkGetDeviceGroupSurfacePresentModes2EXT'
        ]

    def generate(self):
        copyright = f'''{fileIsGeneratedWarning(os.path.basename(__file__))}
/***************************************************************************
*
* Copyright (c) 2015-2023 The Khronos Group Inc.
* Copyright (c) 2015-2023 Valve Corporation
* Copyright (c) 2015-2023 LunarG, Inc.
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
****************************************************************************/\n'''
        self.write(copyright)
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'stateless_validation_helper.h':
            self.generateHeader()
        elif self.filename == 'stateless_validation_helper.cpp':
            self.generateSource()
        elif self.filename == 'stateless_validation_pnext_struct.cpp':
            self.generatePnextStruct()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('#pragma once\n')
        for command in [x for x in self.vk.commands.values() if x.name not in self.blacklist]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            prototype = command.cPrototype.split('VKAPI_CALL ')[1]
            prototype = f'bool PreCallValidate{prototype[2:]}'
            prototype = prototype.replace(');', ') const override;\n')
            if 'ValidationCache' in command.name:
                prototype = prototype.replace('const override', 'const')
            out.append(prototype)
            out.extend([f'#endif // {command.protect}\n'] if command.protect else [])
        self.write("".join(out))

    # TODO - Not being used currently
    def getStructValidation(self, struct: Struct):
        out = []
        for member in [x for x in struct.members if x.name != "pNext" and x.name != "sType" and not x.noAutoValidity]:
            # Switch state that decides which Validate*() call by return type
            if member.type == 'uint32_t' or member.type == 'uint64_t':
                continue # nothing to validate
            if member.type == 'VkBool32':
                if member.length:
                    out.append(f'skip |= ValidateBool32Array("{struct.name}", "{member.length}", "{member.name}", structure->{member.length}, structure->{member.name}, false, true);\n')
                else:
                    out.append(f'skip |= ValidateBool32("{struct.name}", "{member.name}", structure->{member.name});\n')

            elif member.type in self.vk.bitmasks:
                bitmask = self.vk.bitmasks[member.type]
                arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')

                out.append(f'skip |= ValidateFlags("{struct.name}", "{member.name}", "{bitmask.flagName}", All{bitmask.flagName}, structure->{member.name}, {"kOptionalSingleBit" if member.optional else "kRequiredSingleBit"}, {arrayVUID});\n')

            elif member.type.replace('Flags', 'FlagBits') in self.vk.bitmasks:
                bitmask = self.vk.bitmasks[member.type.replace('Flags', 'FlagBits')]
                if len(bitmask.flags) == 0:
                    zeroVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-zerobitmask')

                    out.append(f'skip |= ValidateReservedFlags("{struct.name}", "{member.name}", structure->{member.name}, "{zeroVUID}");\n')
                elif member.length:
                    arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')
                    out.append(f'skip |= ValidateFlagsArray("{struct.name}", "{member.length}", "{member.name}", "VkDescriptorBindingFlagBits", All{bitmask.flagName}, structure->{member.length}, structure->{member.name}, false, {arrayVUID});\n')
                else:
                    arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')
                    bitmaskVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-requiredbitmask') if not member.optional and not member.optionalPointer else 'nullptr'

                    out.append(f'skip |= ValidateFlags("{struct.name}", "{member.name}", "{bitmask.flagName}", All{bitmask.flagName}, structure->{member.name}, {"kOptionalFlags" if member.optional else "kRequiredFlags"}, {arrayVUID}, {bitmaskVUID});\n')

            elif member.type in self.vk.handles and not member.optional:
                if member.length:
                    hasCount = [x.optional for x in struct.members if x.name == member.length][0] == False
                    countVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.length}-arraylength') if hasCount else 'kVUIDUndefined'
                    arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')

                    out.append(f'skip |= ValidateArray("{struct.name}", "{member.length}", "{member.name}", structure->{member.length}, &structure->{member.name}, {str(hasCount).lower()}, true, {countVUID}, {arrayVUID});\n')
                else:
                    out.append(f'skip |= ValidateRequiredHandle("{struct.name}", "{member.name}", structure->{member.name});\n')

            elif member.type in self.vk.structs and self.vk.structs[member.type].sType:
                # TODO - should ValidateStructType not just be another call to ValidatePnextStructContents?
                subStruct = self.vk.structs[member.type]
                stypeVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-sType-sType')
                arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')
                if member.length:
                    hasCount = [x.optional for x in struct.members if x.name == member.length][0] == False
                    countVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.length}-arraylength') if hasCount else 'kVUIDUndefined'

                    out.append(f'skip |= ValidateStructTypeArray("{struct.name}", "{member.name}", "{member.length}", "{subStruct.sType}", structure->{member.name}, structure->{member.length}, {subStruct.sType}, {str(hasCount).lower()}, true, {stypeVUID}, {arrayVUID}, {countVUID});\n')
                else:
                    out.append(f'skip |= ValidateStructType("{struct.name}", "{member.name}", "{subStruct.sType}", structure->{member.name}, structure->{member.length}, {subStruct.sType}, false, {arrayVUID}, {stypeVUID});\n')

            elif member.type in self.vk.enums:
                enum = self.vk.enums[member.type]
                if member.length:
                    hasCount = [x.optional for x in struct.members if x.name == member.length][0] == False
                    out.append(f'skip |= ValidateRangedEnumArray("{struct.name}", "{member.length}", "{member.name}", "{enum.name}", structure->{member.length}, structure->{member.name}, true, {str(hasCount).lower()});\n')
                else:
                    arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')

                    out.append(f'skip |= ValidateRangedEnum("{struct.name}", "{member.name}", "{enum.name}", structure->{member.name}, {arrayVUID});\n')

            # Check for pointer lasts, means not a enum/struct/bitmask/etc.
            elif member.pointer and not member.optional:
                arrayVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.name}-parameter')
                if member.length:
                    hasCount = [x.optional for x in struct.members if x.name == member.length][0] == False
                    countVUID = getVUID(self.valid_vuids, f'VUID-{struct.name}-{member.length}-arraylength') if hasCount else 'kVUIDUndefined'

                    out.append(f'skip |= ValidateArray("{struct.name}", "{member.length}", "{member.name}", structure->{member.length}, &structure->{member.name}, {str(hasCount).lower()}, true, {countVUID}, {arrayVUID});\n')
                else:
                    out.append(f'skip |= ValidateRequiredPointer("{struct.name}", "{member.name}", structure->{member.name}, {arrayVUID});\n')

            # This logic is handled after, in thoery would could have called into functions,
            # but instead inline the check here as we don't currently have a good way to pass in
            # "where" in the struct we are indexing
            if member.type in self.vk.structs and not self.vk.structs[member.type].sType:
                subStruct = self.vk.structs[member.type]
                body = self.getStructValidation(subStruct)
                if len(body) > 0:
                    out.append(f'if (structure->{member.name} != nullptr) {{')
                    out.extend([body])
                    out.append('}')

        return out if len(out) > 0 else []

    # TODO - Not being used currently
    def generatePnextStruct(self):
        out = []
        out.append('''
#include "chassis.h"
#include "stateless/stateless_validation.h"
#include "enum_flag_bits.h"

bool StatelessValidation::ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name,
                                                      const VkBaseOutStructure* header, const char *pnext_vuid,
                                                      bool is_physdev_api, bool is_const_param) const {
    bool skip = false;
    switch(header->sType) {
''')

        for struct in [x for x in self.vk.structs.values() if x.extends is not None and not x.returnedOnly]:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])
            out.append(f'        case {struct.sType}: {{ // Covers VUID-{struct.name}-sType-sType\n')

            if struct.version and not struct.extensions:
                out.append(f'''            if (api_version < {struct.version.nameApi}) {{
                skip |= LogError(
                           instance, pnext_vuid,
                           "%s: Includes a pNext pointer (%s) to a VkStructureType ({struct.sType}) which was added in {struct.version.nameApi} but the "
                           "current effective API version is %s.",
                           api_name, parameter_name.get_name().c_str(), StringAPIVersion(api_version).c_str());
            }}\n''')

            elif struct.extensions:
                # TODO - Need to handle few cases where there are more then one extension
                # ex. VkPipelineShaderStageRequiredSubgroupSizeCreateInfo and VkDeviceQueueGlobalPriorityCreateInfoKHR
                extension = struct.extensions[0]
                out.append('            if (is_const_param) {\n')
                if extension.device:
                    out.append(f'                if ((is_physdev_api && !SupportedByPdev(physical_device, {extension.nameString})) || (!is_physdev_api && !IsExtEnabled(device_extensions.{extension.name.lower()}))) {{')
                elif extension.instance:
                    out.append(f'                if (!instance_extensions.{extension.name.lower()}) {{')
                out.append(f'''                        skip |= LogError(
                               instance, pnext_vuid,
                               "%s: Includes a pNext pointer (%s) to a VkStructureType ({struct.sType}), but its parent extension "
                               "{extension.name} has not been enabled.",
                               api_name, parameter_name.get_name().c_str());
                }}
            }}\n''')

                body = self.getStructValidation(struct)
                if len(body) > 0:
                    out.append(f'''
            if (is_const_param) {{
                {struct.name} *structure = ({struct.name} *) header;
{"".join('                {}'.format(x) for x in body)}            }}\n''')

            else:
                out.append(f'            // No Validation code for {struct.name} structure members\n')

            out.append('        } break;\n')
            out.extend([f'#endif //{struct.protect}\n'] if struct.protect else [])

        out.append('''
        default:
            skip = false;
    }
    return skip;
}
''')

        self.write("".join(out))

    def generateSource(self):
        out = []
        self.write("".join(out))