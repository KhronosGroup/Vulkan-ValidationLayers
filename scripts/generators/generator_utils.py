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
import os
import sys
import json

# Build a set of all vuid text strings found in validusage.json
def buildListVUID(valid_usage_file: str) -> set:

    # Walk the JSON-derived dict and find all "vuid" key values
    def ExtractVUIDs(vuid_dict):
        if hasattr(vuid_dict, 'items'):
            for key, value in vuid_dict.items():
                if key == "vuid":
                    yield value
                elif isinstance(value, dict):
                    for vuid in ExtractVUIDs(value):
                        yield vuid
                elif isinstance (value, list):
                    for listValue in value:
                        for vuid in ExtractVUIDs(listValue):
                            yield vuid

    valid_vuids = set()
    if not os.path.isfile(valid_usage_file):
        print(f'Error: Could not find, or error loading {valid_usage_file}')
        sys.exit(1)
    json_file = open(valid_usage_file, 'r', encoding='utf-8')
    vuid_dict = json.load(json_file)
    json_file.close()
    if len(vuid_dict) == 0:
        print(f'Error: Failed to load {valid_usage_file}')
        sys.exit(1)
    for json_vuid_string in ExtractVUIDs(vuid_dict):
        valid_vuids.add(json_vuid_string)

    # List of VUs that should exists, but have a spec bug
    for vuid in [
        # https://gitlab.khronos.org/vulkan/vulkan/-/issues/3582
        "VUID-VkCopyImageToImageInfoEXT-commonparent",
        "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parent",
        "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-parent",
        "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-parent",
        "VUID-vkGetDescriptorSetHostMappingVALVE-descriptorSet-parent",
        ]:
        valid_vuids.add(vuid)

    return valid_vuids

# Will do a sanity check the VUID exists
def getVUID(valid_vuids: set, vuid: str, quotes: bool = True) -> str:
    if vuid not in valid_vuids:
        print(f'Warning: Could not find {vuid} in validusage.json')
        vuid = vuid.replace('VUID-', 'UNASSIGNED-')
    return vuid if not quotes else f'"{vuid}"'
