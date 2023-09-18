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
from generators.base_generator import BaseGenerator

class ObjectTypesOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        # Helper for VkDebugReportObjectTypeEXT
        # Maps [ 'VkBuffer' : 'VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT' ]
        # Will be 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT' if no type
        debugReportObject = dict()

        # Search all fields of the Enum to see if has a DEBUG_REPORT_OBJECT
        for handle in self.vk.handles.values():
            debugObjects = ([enum.name for enum in self.vk.enums['VkDebugReportObjectTypeEXT'].fields if f'{handle.type[3:]}_EXT' in enum.name])
            object = 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT' if len(debugObjects) == 0 else debugObjects[0]
            debugReportObject[handle.name] = object

        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

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
        out.append('// clang-format off')
        out.append('''
#pragma once
#include "utils/cast_utils.h"
\n''')

        out.append('''
// Object Type enum for validation layer internal object handling
typedef enum VulkanObjectType {
    kVulkanObjectTypeUnknown = 0,\n''')
        for count, handle in enumerate(self.vk.handles.values(), start=1):
            out.append(f'    kVulkanObjectType{handle.name[2:]} = {count},\n')
        out.append(f'    kVulkanObjectTypeMax = {len(self.vk.handles) + 1}\n')
        out.append('} VulkanObjectType;\n')

        out.append('''
// Array of object name strings for OBJECT_TYPE enum conversion
static const char * const object_string[kVulkanObjectTypeMax] = {
    "VkNonDispatchableHandle",\n''')
        out.extend([f'    "{handle.name}",\n' for handle in self.vk.handles.values()])
        out.append('};\n')

        out.append('''
// Helper array to get Vulkan VK_EXT_debug_report object type enum from the internal layers version
const VkDebugReportObjectTypeEXT get_debug_report_enum[] = {
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, // kVulkanObjectTypeUnknown\n''')
        out.extend([f'    {debugReportObject[handle.name]},   // kVulkanObjectType{handle.name[2:]}\n' for handle in self.vk.handles.values()])
        out.append('};\n')

        out.append('''
// Helper function to get Official Vulkan VkObjectType enum from the internal layers version
static inline VkObjectType ConvertVulkanObjectToCoreObject(VulkanObjectType internal_type) {
    switch (internal_type) {\n''')
        out.extend([f'        case kVulkanObjectType{handle.name[2:]}: return {handle.type};\n' for handle in self.vk.handles.values()])
        out.append('''        default: return VK_OBJECT_TYPE_UNKNOWN;
    }
}\n''')

        out.append('''
// Helper function to get internal layers object ids from the official Vulkan VkObjectType enum
static inline VulkanObjectType ConvertCoreObjectToVulkanObject(VkObjectType vulkan_object_type) {
    switch (vulkan_object_type) {\n''')
        out.extend([f'        case {handle.type}: return kVulkanObjectType{handle.name[2:]};\n' for handle in self.vk.handles.values()])
        out.append('''        default: return kVulkanObjectTypeUnknown;
    }
}\n''')

        out.append('''
static inline VkDebugReportObjectTypeEXT convertCoreObjectToDebugReportObject(VkObjectType core_report_obj) {
    switch (core_report_obj) {
        case VK_OBJECT_TYPE_UNKNOWN: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;\n''')
        for handle in self.vk.handles.values():
            object = debugReportObject[handle.name]
            if object != 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT':
                out.append(f'        case {handle.type}: return {object};\n')
        out.append('''        default: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    }
}\n''')

        out.append('''
// Traits objects from each type statically map from Vk<handleType> to the various enums
template <typename VkType> struct VkHandleInfo {};
template <VulkanObjectType id> struct VulkanObjectTypeInfo {};

// The following line must match the vulkan_core.h condition guarding VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define TYPESAFE_NONDISPATCHABLE_HANDLES
#else
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkNonDispatchableHandle)

template <> struct VkHandleInfo<VkNonDispatchableHandle> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeUnknown;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_UNKNOWN;
    static const char* Typename() {
        return "VkNonDispatchableHandle";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeUnknown> {
    typedef VkNonDispatchableHandle Type;
};

#endif //  VK_DEFINE_HANDLE logic duplication\n''')

        for handle in [x for x in self.vk.handles.values() if x.dispatchable]:
            out.extend([f'#ifdef {handle.protect}\n'] if handle.protect else [])
            out.append(f'''
template <> struct VkHandleInfo<{handle.name}> {{
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectType{handle.name[2:]};
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = {debugReportObject[handle.name]};
    static const VkObjectType kVkObjectType = {handle.type};
    static const char* Typename() {{
        return "{handle.name}";
    }}
}};
template <> struct VulkanObjectTypeInfo<kVulkanObjectType{handle.name[2:]}> {{
    typedef {handle.name} Type;
}};\n''')
            out.extend([f'#endif //{handle.protect}\n'] if handle.protect else [])
        out.append('#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES\n')

        for handle in [x for x in self.vk.handles.values() if not x.dispatchable]:
            out.extend([f'#ifdef {handle.protect}\n'] if handle.protect else [])
            out.append(f'''
template <> struct VkHandleInfo<{handle.name}> {{
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectType{handle.name[2:]};
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = {debugReportObject[handle.name]};
    static const VkObjectType kVkObjectType = {handle.type};
    static const char* Typename() {{
        return "{handle.name}";
    }}
}};
template <> struct VulkanObjectTypeInfo<kVulkanObjectType{handle.name[2:]}> {{
    typedef {handle.name} Type;
}};\n''')
            out.extend([f'#endif //{handle.protect}\n'] if handle.protect else [])
        out.append('#endif // TYPESAFE_NONDISPATCHABLE_HANDLES\n')

        out.append('''
struct VulkanTypedHandle {
    uint64_t handle;
    VulkanObjectType type;
    template <typename Handle>
    VulkanTypedHandle(Handle handle_, VulkanObjectType type_) :
        handle(CastToUint64(handle_)),
        type(type_) {
#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
        // For 32 bit it's not always safe to check for traits <-> type
        // as all non-dispatchable handles have the same type-id and thus traits,
        // but on 64 bit we can validate the passed type matches the passed handle
        assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
#endif // TYPESAFE_NONDISPATCHABLE_HANDLES
    }
    template <typename Handle>
    Handle Cast() const {
#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
        assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
#endif // TYPESAFE_NONDISPATCHABLE_HANDLES
        return CastFromUint64<Handle>(handle);
    }
    VulkanTypedHandle() :
        handle(CastToUint64(VK_NULL_HANDLE)),
        type(kVulkanObjectTypeUnknown) {}
    operator bool() const { return handle != 0; }
};\n''')
        out.append('// clang-format on')
        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))
