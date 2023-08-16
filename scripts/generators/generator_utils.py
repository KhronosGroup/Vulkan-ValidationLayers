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

# TODO - This is a temporary design to allow us to slowly increase functions to use
#  ErrorObject until we feel confident with the design to roll it out everywhere
# (this allows use to see what works well before spending time refactoring each call)
error_object_functions = [
    'vkCreateRenderPass',
    'vkCreateRenderPass2',
    'vkCreateRenderPass2KHR',
    'vkDestroyRenderPass',
    'vkCreateFramebuffer',
    'vkDestroyFramebuffer',
    'vkCmdBeginRenderPass',
    'vkCmdBeginRenderPass2',
    'vkCmdBeginRenderPass2KHR',
    'vkCmdEndRenderPass',
    'vkCmdEndRenderPass2',
    'vkCmdEndRenderPass2KHR',
    'vkCmdBeginRendering',
    'vkCmdBeginRenderingKHR',
    'vkCmdEndRendering',
    'vkCmdEndRenderingKHR',
    'vkCmdNextSubpass',
    'vkCmdNextSubpass2',
    'vkCmdNextSubpass2KHR',
    'vkCmdPushConstants',
    'vkQueueBindSparse',
    'vkQueueSubmit',
    'vkQueueSubmit2',
    'vkQueueSubmit2KHR',
    'vkCreateSamplerYcbcrConversion',
    'vkCreateSamplerYcbcrConversionKHR',
    'vkCmdExecuteCommands',
    'vkAllocateMemory',
    'vkFreeMemory',
    'vkMapMemory',
    'vkMapMemory2KHR',
    'vkUnmapMemory',
    'vkUnmapMemory2KHR',
    'vkFlushMappedMemoryRanges',
    'vkInvalidateMappedMemoryRanges',
    'vkGetDeviceMemoryCommitment',
    'vkGetDeviceImageMemoryRequirementsKHR',
    'vkGetDeviceImageSparseMemoryRequirementsKHR',
    'vkGetImageMemoryRequirements',
    'vkGetImageMemoryRequirements2',
    'vkGetImageMemoryRequirements2KHR',
    'vkGetBufferOpaqueCaptureAddress',
    'vkGetBufferOpaqueCaptureAddressKHR',
    'vkGetDeviceMemoryOpaqueCaptureAddress',
    'vkGetDeviceMemoryOpaqueCaptureAddressKHR',
    'vkGetBufferDeviceAddress',
    'vkGetBufferDeviceAddressEXT',
    'vkGetBufferDeviceAddressKHR',
    'vkBindBufferMemory',
    'vkBindBufferMemory2',
    'vkBindBufferMemory2KHR',
    'vkBindImageMemory',
    'vkBindImageMemory2',
    'vkBindImageMemory2KHR',
    'vkCreateImage',
    'vkCreateImageView',
    'vkCreateBuffer',
    'vkCreateBufferView',
    'vkCmdFillBuffer',
    "vkCmdDraw",
    "vkCmdDrawIndexed",
    "vkCmdDrawIndirect",
    "vkCmdDrawIndexedIndirect",
    "vkCmdDrawIndirectCount",
    "vkCmdDrawIndirectCountAMD",
    "vkCmdDrawIndirectCountKHR",
    "vkCmdDrawIndexedIndirectCount",
    "vkCmdDrawIndexedIndirectCountAMD",
    "vkCmdDrawIndexedIndirectCountKHR",
    "vkCmdDrawIndirectByteCountEXT",
    "vkCmdDrawMeshTasksNV",
    "vkCmdDrawMeshTasksIndirectNV",
    "vkCmdDrawMeshTasksIndirectCountNV",
    "vkCmdDrawMultiEXT",
    "vkCmdDrawMultiIndexedEXT",
    "vkCmdDrawClusterHUAWEI",
    "vkCmdDrawClusterIndirectHUAWEI",
    "vkCmdDrawMeshTasksEXT",
    "vkCmdDrawMeshTasksIndirectEXT",
    "vkCmdDrawMeshTasksIndirectCountEXT",
    'vkCmdDispatch',
    'vkCmdDispatchIndirect',
    'vkCmdDispatchBase',
    'vkCmdDispatchBaseKHR',
    "vkCmdDispatchGraphAMDX",
    "vkCmdDispatchGraphIndirectAMDX",
    "vkCmdDispatchGraphIndirectCountAMDX",
    "vkCmdTraceRaysNV",
    "vkCmdTraceRaysKHR",
    "vkCmdTraceRaysIndirectKHR",
    "vkCmdTraceRaysIndirect2KHR",
    "vkGetQueryPoolResults",
    "vkCreateQueryPool",
    "vkCmdBeginQuery",
    "vkCmdEndQuery",
    "vkCmdResetQueryPool",
    "vkCmdCopyQueryPoolResults",
    "vkCmdWriteTimestamp",
    "vkCmdWriteTimestamp2",
    "vkCmdWriteTimestamp2KHR",
    "vkCmdBeginQueryIndexedEXT",
    "vkCmdEndQueryIndexedEXT",
    "vkResetQueryPool",
    "vkResetQueryPoolEXT",
    "vkReleaseProfilingLockKHR",
    "vkCreateSwapchainKHR",
    "vkQueuePresentKHR",
    "vkReleaseSwapchainImagesEXT",
    "vkCreateSharedSwapchainsKHR",
    "vkAcquireNextImageKHR",
    "vkAcquireNextImage2KHR",
    "vkWaitForPresentKHR",
    "vkDestroySurfaceKHR",
    "vkGetPhysicalDeviceWaylandPresentationSupportKHR",
    "vkGetPhysicalDeviceWin32PresentationSupportKHR",
    "vkGetPhysicalDeviceXcbPresentationSupportKHR",
    "vkGetPhysicalDeviceXlibPresentationSupportKHR",
    "vkGetPhysicalDeviceScreenPresentationSupportQNX",
    "vkGetPhysicalDeviceSurfaceSupportKHR",
    "vkGetDisplayPlaneSupportedDisplaysKHR",
    "vkGetDisplayPlaneCapabilitiesKHR",
    "vkGetDisplayPlaneCapabilities2KHR",
    "vkCreateDisplayPlaneSurfaceKHR",
    "vkAcquireFullScreenExclusiveModeEXT",
    "vkReleaseFullScreenExclusiveModeEXT",
    "vkGetDeviceGroupSurfacePresentModes2EXT",
    "vkGetPhysicalDeviceSurfacePresentModes2EXT",
    "vkGetDeviceGroupSurfacePresentModesKHR",
    "vkGetPhysicalDevicePresentRectanglesKHR",
    "vkGetPhysicalDeviceSurfaceCapabilities2EXT",
    "vkGetPhysicalDeviceSurfaceCapabilities2KHR",
    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
    "vkGetPhysicalDeviceSurfaceFormats2KHR",
    "vkGetPhysicalDeviceSurfaceFormatsKHR",
    "vkGetPhysicalDeviceSurfacePresentModesKHR",
    "vkCreateDisplayModeKHR",
    "vkCreateXlibSurfaceKHR",
    "vkCreateXcbSurfaceKHR",
    "vkCreateWin32SurfaceKHR",
    "vkCreateWaylandSurfaceKHR",
]

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

INDENT_SPACES = 4
def incIndent(indent: str) -> str:
    inc = ' ' * INDENT_SPACES
    return indent + inc if indent else inc

def decIndent(indent: str) -> str:
    return indent[:-INDENT_SPACES] if indent and (len(indent) > INDENT_SPACES) else ''

# Add the indent to each line of the input
def addIndent(indent: str, input: str) -> str:
    out = ''
    lines = input.split('\n')
    for line in lines:
        out += f'{indent}{line}\n'
    return out