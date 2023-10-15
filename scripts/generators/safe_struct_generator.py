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
import re
from generators.vulkan_object import Struct, Member
from generators.base_generator import BaseGenerator

# Determine if a structure needs a safe_struct helper function
# That is, it has an sType or one of its members is a pointer
def needSafeStruct(struct: Struct) -> bool:
    if 'VkBase' in struct.name:
        return False #  Ingore structs like VkBaseOutStructure
    if struct.sType is not None:
        return True
    for member in struct.members:
        if member.pointer:
            return True
    return False

class SafeStructOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.custom_construct_params = {
            # safe_VkGraphicsPipelineCreateInfo needs to know if subpass has color and\or depth\stencil attachments to use its pointers
            'VkGraphicsPipelineCreateInfo' :
                ', const bool uses_color_attachment, const bool uses_depthstencil_attachment',
            # safe_VkPipelineViewportStateCreateInfo needs to know if viewport and scissor is dynamic to use its pointers
            'VkPipelineViewportStateCreateInfo' :
                ', const bool is_dynamic_viewports, const bool is_dynamic_scissors',
            # safe_VkAccelerationStructureBuildGeometryInfoKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureBuildGeometryInfoKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_infos',
            # safe_VkAccelerationStructureGeometryKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureGeometryKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_info',
            # safe_VkDescriptorDataEXT needs to know what field of union is intialized
            'VkDescriptorDataEXT' :
                ', const VkDescriptorType type',
            'VkPipelineRenderingCreateInfo' : ''
        }

    def containsObjectHandle(self, member: Member) -> bool:
        if member.type in self.vk.handles:
            return True
        if member.type in self.vk.structs:
            for subMember in self.vk.structs[member.type].members:
                if self.containsObjectHandle(subMember):
                    return True
        return False

    def typeContainsObjectHandle(self, handle_type: str, dispatchable: bool) -> bool:
        if handle_type in self.vk.handles:
            if dispatchable == self.vk.handles[handle_type].dispatchable:
                return True
        # if handle_type is a struct, search its members
        if handle_type in self.vk.structs:
            struct = self.vk.structs[handle_type]
            for member in [x for x in struct.members if x.type in self.vk.handles]:
                if dispatchable == self.vk.handles[member.type].dispatchable:
                    return True
        return False

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
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
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'vk_safe_struct.h':
            self.generateHeader()
        elif self.filename == 'vk_safe_struct_utils.cpp':
            self.generateUtil()
        elif self.filename.startswith('vk_safe_struct_'):
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan.h>
            #include <cstdlib>
            #include <algorithm>
            #include <functional>

            // State that elements in a pNext chain may need to be aware of
            struct PNextCopyState {
                // Custom initialization function. Returns true if the structure passed to init was initialized, false otherwise
                std::function<bool(VkBaseOutStructure* /* safe_sruct */, const VkBaseOutStructure* /* in_struct */)> init;
            };

            void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state = {});
            void FreePnextChain(const void *pNext);
            char *SafeStringCopy(const char *in_string);
            \n''')

        for struct in [x for x in self.vk.structs.values() if needSafeStruct(x)]:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])
            out.append(f'{"union" if struct.union else "struct"} safe_{struct.name} {{\n')
            # Can only initialize first member of an Union
            canInitialize = True
            copy_pnext = ', bool copy_pnext = true' if struct.sType is not None else ''
            for member in struct.members:
                if member.type in self.vk.structs:
                    if needSafeStruct(self.vk.structs[member.type]):
                        if member.pointer:
                            pointer = '*' * member.cDeclaration.count('*')
                            brackets = '' if struct.union else '{}'
                            out.append(f'safe_{member.type}{pointer} {member.name}{brackets};\n')
                        else:
                            out.append(f'safe_{member.type} {member.name};\n')
                        continue

                explicitInitialize = member.pointer  and 'PFN_' not in member.type and canInitialize
                initialize = '{}' if explicitInitialize else ''
                # Prevents union from initializing agian
                canInitialize = not struct.union if explicitInitialize else canInitialize

                if member.length and self.containsObjectHandle(member) and not member.fixedSizeArray:
                    out.append(f'    {member.type}* {member.name}{initialize};\n')
                else:
                    out.append(f'{member.cDeclaration}{initialize};\n')

            if (struct.name == 'VkDescriptorDataEXT'):
                out.append('char type_at_end[sizeof(VkDescriptorDataEXT)+sizeof(VkDescriptorGetInfoEXT::type)];')

            constructParam = self.custom_construct_params.get(struct.name, '')
            out.append(f'''
                safe_{struct.name}(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}}{copy_pnext});
                safe_{struct.name}(const safe_{struct.name}& copy_src);
                safe_{struct.name}& operator=(const safe_{struct.name}& copy_src);
                safe_{struct.name}();
                ~safe_{struct.name}();
                void initialize(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}});
                void initialize(const safe_{struct.name}* copy_src, PNextCopyState* copy_state = {{}});
                {struct.name} *ptr() {{ return reinterpret_cast<{struct.name} *>(this); }}
                {struct.name} const *ptr() const {{ return reinterpret_cast<{struct.name} const *>(this); }}
                ''')

            if struct.name == 'VkShaderModuleCreateInfo':
                out.append('''
                    // Primarily intended for use by GPUAV when replacing shader module code with instrumented code
                    template<typename Container>
                    void SetCode(const Container &code) {
                        delete[] pCode;
                        codeSize = static_cast<uint32_t>(code.size() * sizeof(uint32_t));
                        pCode = new uint32_t[code.size()];
                        std::copy(&code.front(), &code.back() + 1, const_cast<uint32_t*>(pCode));
                    }
                    ''')
            out.append('};\n')
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])
        self.write("".join(out))

    def generateUtil(self):
        out = []
        out.append('''
            #include "vk_safe_struct.h"
            #include "utils/vk_layer_utils.h"

            #include <vector>

            extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

            char *SafeStringCopy(const char *in_string) {
                if (nullptr == in_string) return nullptr;
                char* dest = new char[std::strlen(in_string) + 1];
                return std::strcpy(dest, in_string);
            }

            ''')
        out.append('''
// clang-format off
void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state) {
    void *first_pNext{};
    VkBaseOutStructure *prev_pNext{};
    void *safe_pNext{};

    while (pNext) {
        const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

        switch (header->sType) {
            // Add special-case code to copy beloved secret loader structs
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
                VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;
                // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            // Special-case Loader Device Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;
                // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }''')

        for struct in [x for x in self.vk.structs.values() if x.extends]:
            out.extend([f'\n#ifdef {struct.protect}'] if struct.protect else [])
            out.append(f'''
            case {struct.sType}:
                safe_pNext = new safe_{struct.name}(reinterpret_cast<const {struct.name} *>(pNext), copy_state, false);
                break;''')
            out.extend([f'\n#endif // {struct.protect}'] if struct.protect else [])

        out.append('''
            default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain
                // If sType is in custom list, construct blind copy
                for (auto item : custom_stype_info) {
                    if (item.first == header->sType) {
                        safe_pNext = malloc(item.second);
                        memcpy(safe_pNext, header, item.second);
                    }
                }
                break;
        }
        if (!first_pNext) {
            first_pNext = safe_pNext;
        }
        pNext = header->pNext;
        if (prev_pNext && safe_pNext) {
            prev_pNext->pNext = (VkBaseOutStructure*)safe_pNext;
        }
        if (safe_pNext) {
            prev_pNext = (VkBaseOutStructure*)safe_pNext;
        }
        safe_pNext = nullptr;
    }

    return first_pNext;
}

void FreePnextChain(const void *pNext) {
    if (!pNext) return;

    auto header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

    switch (header->sType) {
        // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerInstanceCreateInfo *>(pNext);
            break;
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerDeviceCreateInfo *>(pNext);
            break;
''')

        for struct in [x for x in self.vk.structs.values() if x.extends]:
            out.extend([f'\n#ifdef {struct.protect}'] if struct.protect else [])
            out.append(f'''
        case {struct.sType}:
            delete reinterpret_cast<const safe_{struct.name} *>(header);
            break;''')
            out.extend([f'\n#endif // {struct.protect}'] if struct.protect else [])

        out.append('''
        default: // Encountered an unknown sType
            // If sType is in custom list, free custom struct memory and clean up
            for (auto item : custom_stype_info) {
                if (item.first == header->sType) {
                    if (header->pNext) {
                        FreePnextChain(header->pNext);
                    }
                    free(const_cast<void *>(pNext));
                    pNext = nullptr;
                    break;
                }
            }
            if (pNext) {
                FreePnextChain(header->pNext);
            }
            break;
    }
}''')
        out.append('// clang-format on\n')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "vk_safe_struct.h"
            #include <vulkan/utility/vk_struct_helper.hpp>
            #include "utils/vk_layer_utils.h"

            #include <cstddef>
            #include <cassert>
            #include <cstring>
            #include <vector>

            #include <vulkan/vk_layer.h>
            ''')

        custom_definitions = {
            # as_geom_khr_host_alloc maps a VkAccelerationStructureGeometryKHR to its host allocated instance array, if the user supplied such an array.
            'VkAccelerationStructureGeometryKHR':
            """
            struct ASGeomKHRExtraData {
                ASGeomKHRExtraData(uint8_t *alloc, uint32_t primOffset, uint32_t primCount) :
                    ptr(alloc),
                    primitiveOffset(primOffset),
                    primitiveCount(primCount)
                {}
                ~ASGeomKHRExtraData() {
                    if (ptr)
                        delete[] ptr;
                }
                uint8_t *ptr;
                uint32_t primitiveOffset;
                uint32_t primitiveCount;
            };

            vl_concurrent_unordered_map<const safe_VkAccelerationStructureGeometryKHR*, ASGeomKHRExtraData*, 4> as_geom_khr_host_alloc;"""
            }

        custom_defeault_construct_txt = {
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '    *pType = VK_DESCRIPTOR_TYPE_MAX_ENUM;\n'
            }

        custom_construct_txt = {
                # VkWriteDescriptorSet is special case because pointers may be non-null but ignored
                'VkWriteDescriptorSet' :
                    '    switch (descriptorType) {\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:\n'
                    '        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:\n'
                    '        if (descriptorCount && in_struct->pImageInfo) {\n'
                    '            pImageInfo = new VkDescriptorImageInfo[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pImageInfo[i] = in_struct->pImageInfo[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:\n'
                    '        if (descriptorCount && in_struct->pBufferInfo) {\n'
                    '            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pBufferInfo[i] = in_struct->pBufferInfo[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:\n'
                    '        if (descriptorCount && in_struct->pTexelBufferView) {\n'
                    '            pTexelBufferView = new VkBufferView[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pTexelBufferView[i] = in_struct->pTexelBufferView[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        default:\n'
                    '        break;\n'
                    '    }\n',
                'VkShaderModuleCreateInfo' :
                    '    if (in_struct->pCode) {\n'
                    '        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);\n'
                    '        memcpy((void *)pCode, (void *)in_struct->pCode, codeSize);\n'
                    '    }\n',
                # VkGraphicsPipelineCreateInfo is special case because its pointers may be non-null but ignored
                'VkGraphicsPipelineCreateInfo' :
                    '    const bool is_graphics_library = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(in_struct->pNext) != nullptr;\n'
                    '    if (stageCount && in_struct->pStages) {\n'
                    '        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];\n'
                    '        for (uint32_t i = 0; i < stageCount; ++i) {\n'
                    '            pStages[i].initialize(&in_struct->pStages[i]);\n'
                    '        }\n'
                    '    }\n'
                    '    if (in_struct->pVertexInputState)\n'
                    '        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);\n'
                    '    else\n'
                    '        pVertexInputState = nullptr;\n'
                    '    if (in_struct->pInputAssemblyState)\n'
                    '        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);\n'
                    '    else\n'
                    '        pInputAssemblyState = nullptr;\n'
                    '    bool has_tessellation_stage = false;\n'
                    '    if (stageCount && pStages)\n'
                    '        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)\n'
                    '            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)\n'
                    '                has_tessellation_stage = true;\n'
                    '    if (in_struct->pTessellationState && has_tessellation_stage)\n'
                    '        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);\n'
                    '    else\n'
                    '        pTessellationState = nullptr; // original pTessellationState pointer ignored\n'
                    '    bool is_dynamic_has_rasterization = false;\n'
                    '    if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {\n'
                    '        for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)\n'
                    '            if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)\n'
                    '                is_dynamic_has_rasterization = true;\n'
                    '    }\n'
                    '    const bool has_rasterization = in_struct->pRasterizationState ? (is_dynamic_has_rasterization || !in_struct->pRasterizationState->rasterizerDiscardEnable) : false;\n'
                    '    if (in_struct->pViewportState && (has_rasterization || is_graphics_library)) {\n'
                    '        bool is_dynamic_viewports = false;\n'
                    '        bool is_dynamic_scissors = false;\n'
                    '        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {\n'
                    '            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)\n'
                    '                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT)\n'
                    '                    is_dynamic_viewports = true;\n'
                    '            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)\n'
                    '                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR)\n'
                    '                    is_dynamic_scissors = true;\n'
                    '        }\n'
                    '        pViewportState = new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);\n'
                    '    } else\n'
                    '        pViewportState = nullptr; // original pViewportState pointer ignored\n'
                    '    if (in_struct->pRasterizationState)\n'
                    '        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);\n'
                    '    else\n'
                    '        pRasterizationState = nullptr;\n'
                    '    if (in_struct->pMultisampleState && (renderPass != VK_NULL_HANDLE || has_rasterization || is_graphics_library))\n'
                    '        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);\n'
                    '    else\n'
                    '        pMultisampleState = nullptr; // original pMultisampleState pointer ignored\n'
                    '    // needs a tracked subpass state uses_depthstencil_attachment\n'
                    '    if (in_struct->pDepthStencilState && ((has_rasterization && uses_depthstencil_attachment) || is_graphics_library))\n'
                    '        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);\n'
                    '    else\n'
                    '        pDepthStencilState = nullptr; // original pDepthStencilState pointer ignored\n'
                    '    // needs a tracked subpass state usesColorAttachment\n'
                    '    if (in_struct->pColorBlendState && ((has_rasterization && uses_color_attachment) || is_graphics_library))\n'
                    '        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);\n'
                    '    else\n'
                    '        pColorBlendState = nullptr; // original pColorBlendState pointer ignored\n'
                    '    if (in_struct->pDynamicState)\n'
                    '        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);\n'
                    '    else\n'
                    '        pDynamicState = nullptr;\n',
                 # VkPipelineViewportStateCreateInfo is special case because its pointers may be non-null but ignored
                'VkPipelineViewportStateCreateInfo' :
                    '    if (in_struct->pViewports && !is_dynamic_viewports) {\n'
                    '        pViewports = new VkViewport[in_struct->viewportCount];\n'
                    '        memcpy ((void *)pViewports, (void *)in_struct->pViewports, sizeof(VkViewport)*in_struct->viewportCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pViewports = nullptr;\n'
                    '    if (in_struct->pScissors && !is_dynamic_scissors) {\n'
                    '        pScissors = new VkRect2D[in_struct->scissorCount];\n'
                    '        memcpy ((void *)pScissors, (void *)in_struct->pScissors, sizeof(VkRect2D)*in_struct->scissorCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pScissors = nullptr;\n',
                # VkFrameBufferCreateInfo is special case because its pAttachments pointer may be non-null but ignored
                'VkFramebufferCreateInfo' :
                    '    if (attachmentCount && in_struct->pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {\n'
                    '        pAttachments = new VkImageView[attachmentCount];\n'
                    '        for (uint32_t i = 0; i < attachmentCount; ++i) {\n'
                    '            pAttachments[i] = in_struct->pAttachments[i];\n'
                    '        }\n'
                    '    }\n',
                # VkDescriptorSetLayoutBinding is special case because its pImmutableSamplers pointer may be non-null but ignored
                'VkDescriptorSetLayoutBinding' :
                    '    const bool sampler_type = in_struct->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || in_struct->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;\n'
                    '    if (descriptorCount && in_struct->pImmutableSamplers && sampler_type) {\n'
                    '        pImmutableSamplers = new VkSampler[descriptorCount];\n'
                    '        for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '            pImmutableSamplers[i] = in_struct->pImmutableSamplers[i];\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureBuildGeometryInfoKHR':
                    '    if (geometryCount) {\n'
                    '        if ( in_struct->ppGeometries) {\n'
                    '            ppGeometries = new safe_VkAccelerationStructureGeometryKHR *[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);\n'
                    '            }\n'
                    '        } else {\n'
                    '            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                (pGeometries)[i] = safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);\n'
                    '            }\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {\n'
                    '        if (geometry.instances.arrayOfPointers) {\n'
                    '            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);\n'
                    '            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            VkAccelerationStructureInstanceKHR **ppInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR **>(allocation + build_range_info->primitiveOffset);\n'
                    '            VkAccelerationStructureInstanceKHR *pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(allocation + build_range_info->primitiveOffset + pp_array_size);\n'
                    '            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {\n'
                    '                const uint8_t *byte_ptr = reinterpret_cast<const uint8_t *>(in_struct->geometry.instances.data.hostAddress);\n'
                    '                pInstances[i] = *(reinterpret_cast<VkAccelerationStructureInstanceKHR * const*>(byte_ptr + build_range_info->primitiveOffset)[i]);\n'
                    '                ppInstances[i] = &pInstances[i];\n'
                    '            }\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));\n'
                    '        } else {\n'
                    '            const auto primitive_offset = build_range_info->primitiveOffset;\n'
                    '            const auto primitive_count = build_range_info->primitiveCount;\n'
                    '            size_t array_size = primitive_offset + primitive_count * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            auto host_address = static_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);\n'
                    '            memcpy(allocation + primitive_offset, host_address + primitive_offset, primitive_count * sizeof(VkAccelerationStructureInstanceKHR));\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));\n'
                    '        }\n'
                    '    }\n',
                'VkMicromapBuildInfoEXT':
                    '    if (in_struct->pUsageCounts) {\n'
                    '        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];\n'
                    '        memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*in_struct->usageCountsCount);\n'
                    '    }\n'
                    '    if (in_struct->ppUsageCounts) {\n'
                    '        VkMicromapUsageEXT** pointer_array  = new VkMicromapUsageEXT*[in_struct->usageCountsCount];\n'
                    '        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {\n'
                    '            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);\n'
                    '        }\n'
                    '        ppUsageCounts = pointer_array;\n'
                    '    }\n',
                'VkAccelerationStructureTrianglesOpacityMicromapEXT':
                    '    if (in_struct->pUsageCounts) {\n'
                    '        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];\n'
                    '        memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*in_struct->usageCountsCount);\n'
                    '    }\n'
                    '    if (in_struct->ppUsageCounts) {\n'
                    '        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];\n'
                    '        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {\n'
                    '            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);\n'
                    '        }\n'
                    '        ppUsageCounts = pointer_array;\n'
                    '    }\n',
                'VkAccelerationStructureTrianglesDisplacementMicromapNV':
                    '    if (in_struct->pUsageCounts) {\n'
                    '        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];\n'
                    '        memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*in_struct->usageCountsCount);\n'
                    '    }\n'
                    '    if (in_struct->ppUsageCounts) {\n'
                    '        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];\n'
                    '        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {\n'
                    '            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);\n'
                    '        }\n'
                    '        ppUsageCounts = pointer_array;\n'
                    '    }\n',
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (type)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    pSampler              = new VkSampler(*in_struct->pSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     pCombinedImageSampler = new VkDescriptorImageInfo(*in_struct->pCombinedImageSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              pSampledImage         = in_struct->pSampledImage ? new VkDescriptorImageInfo(*in_struct->pSampledImage) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              pStorageImage         = in_struct->pStorageImage ? new VkDescriptorImageInfo(*in_struct->pStorageImage) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           pInputAttachmentImage = new VkDescriptorImageInfo(*in_struct->pInputAttachmentImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       pUniformTexelBuffer   = in_struct->pUniformTexelBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pUniformTexelBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       pStorageTexelBuffer   = in_struct->pStorageTexelBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pStorageTexelBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             pUniformBuffer        = in_struct->pUniformBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pUniformBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             pStorageBuffer        = in_struct->pStorageBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pStorageBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = in_struct->accelerationStructure; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = in_struct->accelerationStructure; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    *pType = type;\n',
                'VkPipelineRenderingCreateInfo': '''
                    bool custom_init = copy_state && copy_state->init;
                    if (custom_init) {
                        custom_init = copy_state->init(reinterpret_cast<VkBaseOutStructure*>(this), reinterpret_cast<const VkBaseOutStructure*>(in_struct));
                    }
                    if (!custom_init) {
                        // The custom iniitalization was not used, so do the regular initialization
                        if (in_struct->pColorAttachmentFormats) {
                            pColorAttachmentFormats = new VkFormat[in_struct->colorAttachmentCount];
                            memcpy ((void *)pColorAttachmentFormats, (void *)in_struct->pColorAttachmentFormats, sizeof(VkFormat)*in_struct->colorAttachmentCount);
                        }
                    }
                '''
            }

        custom_copy_txt = {
                # VkGraphicsPipelineCreateInfo is special case because it has custom construct parameters
                'VkGraphicsPipelineCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    const bool is_graphics_library = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(copy_src.pNext);\n'
                    '    if (stageCount && copy_src.pStages) {\n'
                    '        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];\n'
                    '        for (uint32_t i = 0; i < stageCount; ++i) {\n'
                    '            pStages[i].initialize(&copy_src.pStages[i]);\n'
                    '        }\n'
                    '    }\n'
                    '    if (copy_src.pVertexInputState)\n'
                    '        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);\n'
                    '    else\n'
                    '        pVertexInputState = nullptr;\n'
                    '    if (copy_src.pInputAssemblyState)\n'
                    '        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*copy_src.pInputAssemblyState);\n'
                    '    else\n'
                    '        pInputAssemblyState = nullptr;\n'
                    '    bool has_tessellation_stage = false;\n'
                    '    if (stageCount && pStages)\n'
                    '        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)\n'
                    '            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)\n'
                    '                has_tessellation_stage = true;\n'
                    '    if (copy_src.pTessellationState && has_tessellation_stage)\n'
                    '        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);\n'
                    '    else\n'
                    '        pTessellationState = nullptr; // original pTessellationState pointer ignored\n'
                    '    bool is_dynamic_has_rasterization = false;\n'
                    '    if (copy_src.pDynamicState && copy_src.pDynamicState->pDynamicStates) {\n'
                    '        for (uint32_t i = 0; i < copy_src.pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)\n'
                    '            if (copy_src.pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)\n'
                    '                is_dynamic_has_rasterization = true;\n'
                    '    }\n'
                    '    const bool has_rasterization = copy_src.pRasterizationState ? (is_dynamic_has_rasterization || !copy_src.pRasterizationState->rasterizerDiscardEnable) : false;\n'
                    '    if (copy_src.pViewportState && (has_rasterization || is_graphics_library)) {\n'
                    '        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*copy_src.pViewportState);\n'
                    '    } else\n'
                    '        pViewportState = nullptr; // original pViewportState pointer ignored\n'
                    '    if (copy_src.pRasterizationState)\n'
                    '        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*copy_src.pRasterizationState);\n'
                    '    else\n'
                    '        pRasterizationState = nullptr;\n'
                    '    if (copy_src.pMultisampleState && (has_rasterization || is_graphics_library))\n'
                    '        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*copy_src.pMultisampleState);\n'
                    '    else\n'
                    '        pMultisampleState = nullptr; // original pMultisampleState pointer ignored\n'
                    '    if (copy_src.pDepthStencilState && (has_rasterization || is_graphics_library))\n'
                    '        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*copy_src.pDepthStencilState);\n'
                    '    else\n'
                    '        pDepthStencilState = nullptr; // original pDepthStencilState pointer ignored\n'
                    '    if (copy_src.pColorBlendState && (has_rasterization || is_graphics_library))\n'
                    '        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*copy_src.pColorBlendState);\n'
                    '    else\n'
                    '        pColorBlendState = nullptr; // original pColorBlendState pointer ignored\n'
                    '    if (copy_src.pDynamicState)\n'
                    '        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);\n'
                    '    else\n'
                    '        pDynamicState = nullptr;\n',
                 # VkPipelineViewportStateCreateInfo is special case because it has custom construct parameters
                'VkPipelineViewportStateCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    if (copy_src.pViewports) {\n'
                    '        pViewports = new VkViewport[copy_src.viewportCount];\n'
                    '        memcpy ((void *)pViewports, (void *)copy_src.pViewports, sizeof(VkViewport)*copy_src.viewportCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pViewports = nullptr;\n'
                    '    if (copy_src.pScissors) {\n'
                    '        pScissors = new VkRect2D[copy_src.scissorCount];\n'
                    '        memcpy ((void *)pScissors, (void *)copy_src.pScissors, sizeof(VkRect2D)*copy_src.scissorCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pScissors = nullptr;\n',
                'VkFramebufferCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    if (attachmentCount && copy_src.pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {\n'
                    '        pAttachments = new VkImageView[attachmentCount];\n'
                    '        for (uint32_t i = 0; i < attachmentCount; ++i) {\n'
                    '            pAttachments[i] = copy_src.pAttachments[i];\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureBuildGeometryInfoKHR':
                    '    if (geometryCount) {\n'
                    '        if ( copy_src.ppGeometries) {\n'
                    '            ppGeometries = new safe_VkAccelerationStructureGeometryKHR *[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);\n'
                    '            }\n'
                    '        } else {\n'
                    '            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);\n'
                    '            }\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    auto src_iter = as_geom_khr_host_alloc.find(&copy_src);\n'
                    '    if (src_iter != as_geom_khr_host_alloc.end()) {\n'
                    '        auto &src_alloc = src_iter->second;\n'
                    '        if (geometry.instances.arrayOfPointers) {\n'
                    '            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);\n'
                    '            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            VkAccelerationStructureInstanceKHR **ppInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR **>(allocation + src_alloc->primitiveOffset);\n'
                    '            VkAccelerationStructureInstanceKHR *pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(allocation + src_alloc->primitiveOffset + pp_array_size);\n'
                    '            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {\n'
                    '                pInstances[i] = *(reinterpret_cast<VkAccelerationStructureInstanceKHR * const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);\n'
                    '                ppInstances[i] = &pInstances[i];\n'
                    '            }\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));\n'
                    '        } else {\n'
                    '            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            memcpy(allocation, src_alloc->ptr, array_size);\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));\n'
                    '        }\n'
                    '    }\n',
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '    VkDescriptorType type = *(VkDescriptorType*)&copy_src.type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (type)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    pSampler              = new VkSampler(*copy_src.pSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     pCombinedImageSampler = new VkDescriptorImageInfo(*copy_src.pCombinedImageSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              pSampledImage         = new VkDescriptorImageInfo(*copy_src.pSampledImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              pStorageImage         = new VkDescriptorImageInfo(*copy_src.pStorageImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           pInputAttachmentImage = new VkDescriptorImageInfo(*copy_src.pInputAttachmentImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       pUniformTexelBuffer   = new safe_VkDescriptorAddressInfoEXT(*copy_src.pUniformTexelBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       pStorageTexelBuffer   = new safe_VkDescriptorAddressInfoEXT(*copy_src.pStorageTexelBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             pUniformBuffer        = new safe_VkDescriptorAddressInfoEXT(*copy_src.pUniformBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             pStorageBuffer        = new safe_VkDescriptorAddressInfoEXT(*copy_src.pStorageBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = copy_src.accelerationStructure; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = copy_src.accelerationStructure; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    *pType = type;\n',
                'VkPipelineRenderingCreateInfo': '''
                    if (copy_src.pColorAttachmentFormats) {
                        pColorAttachmentFormats = new VkFormat[copy_src.colorAttachmentCount];
                        memcpy ((void *)pColorAttachmentFormats, (void *)copy_src.pColorAttachmentFormats, sizeof(VkFormat)*copy_src.colorAttachmentCount);
                    }
                '''
            }

        custom_destruct_txt = {
                'VkShaderModuleCreateInfo' :
                    '    if (pCode)\n'
                    '        delete[] reinterpret_cast<const uint8_t *>(pCode);\n',
                'VkAccelerationStructureBuildGeometryInfoKHR' :
                    '    if (ppGeometries) {\n'
                    '        for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '             delete ppGeometries[i];\n'
                    '        }\n'
                    '        delete[] ppGeometries;\n'
                    '    } else if(pGeometries) {\n'
                    '        delete[] pGeometries;\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    auto iter = as_geom_khr_host_alloc.pop(this);\n'
                    '    if (iter != as_geom_khr_host_alloc.end()) {\n'
                    '        delete iter->second;\n'
                    '    }\n',
                'VkMicromapBuildInfoEXT':
                    '    if (pUsageCounts)\n'
                    '        delete[] pUsageCounts;\n'
                    '    if (ppUsageCounts) {\n'
                    '        for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '             delete ppUsageCounts[i];\n'
                    '        }\n'
                    '        delete[] ppUsageCounts;\n'
                    '    }\n',
                'VkAccelerationStructureTrianglesOpacityMicromapEXT':
                    '    if (pUsageCounts)\n'
                    '        delete[] pUsageCounts;\n'
                    '    if (ppUsageCounts) {\n'
                    '        for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '             delete ppUsageCounts[i];\n'
                    '        }\n'
                    '        delete[] ppUsageCounts;\n'
                    '    }\n',
                'VkAccelerationStructureTrianglesDisplacementMicromapNV':
                    '    if (pUsageCounts)\n'
                    '        delete[] pUsageCounts;\n'
                    '    if (ppUsageCounts) {\n'
                    '        for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '             delete ppUsageCounts[i];\n'
                    '        }\n'
                    '        delete[] ppUsageCounts;\n'
                    '    }\n',
                'VkDescriptorDataEXT' :
                    '\n'
                    '    VkDescriptorType& thisType = *(VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (thisType)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    delete pSampler;              pSampler              = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     delete pCombinedImageSampler; pCombinedImageSampler = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              delete pSampledImage;         pSampledImage         = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              delete pStorageImage;         pStorageImage         = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           delete pInputAttachmentImage; pInputAttachmentImage = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       delete pUniformTexelBuffer;   pUniformTexelBuffer   = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       delete pStorageTexelBuffer;   pStorageTexelBuffer   = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             delete pUniformBuffer;        pUniformBuffer        = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             delete pStorageBuffer;        pStorageBuffer        = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = 0ull; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = 0ull; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    thisType = VK_DESCRIPTOR_TYPE_MAX_ENUM;\n',
            }

        wsiStructs = [
            'VkXlibSurfaceCreateInfoKHR',
            'VkXcbSurfaceCreateInfoKHR',
            'VkWaylandSurfaceCreateInfoKHR',
            'VkAndroidSurfaceCreateInfoKHR',
            'VkWin32SurfaceCreateInfoKHR',
            'VkIOSSurfaceCreateInfoMVK',
            'VkMacOSSurfaceCreateInfoMVK',
            'VkMetalSurfaceCreateInfoEXT'
        ]

        member_init_transforms = {
            'queueFamilyIndexCount': lambda m: f'{m.name}(0)'
        }

        def qfi_construct(item, member):
            true_index_setter = lambda i: f'{i}queueFamilyIndexCount = in_struct->queueFamilyIndexCount;\n'
            false_index_setter = lambda i: f'{i}queueFamilyIndexCount = 0;\n'
            if item.name == 'VkSwapchainCreateInfoKHR':
                return (f'(in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)
            else:
                return (f'(in_struct->sharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)

        # map of:
        #  <member name>: function(item, member) -> (condition, true statement, false statement)
        member_construct_conditions = {
            'pQueueFamilyIndices': qfi_construct
        }

        # For abstract types just want to save the pointer away
        # since we cannot make a copy.
        abstractTypes = [
            'AHardwareBuffer',
            'ANativeWindow',
            'CAMetalLayer'
        ]

        # Find what types of safe structs need to be generated based on output file name
        splitRegex = r'.*';
        if self.filename.endswith('_khr.cpp'):
            splitRegex = r'.*KHR$'
        elif self.filename.endswith('_ext.cpp'):
            splitRegex = r'.*EXT$'
        elif self.filename.endswith('_vendor.cpp'):
            splitRegex = r'^(?!.*(KHR|EXT)$).*[A-Z]$' # Matches all words finishing with an upper case letter, but not ending with KHRor EXT
        else: # elif self.filename.endswith('_core.cpp'):
            splitRegex = r'.*[a-z0-9]$'

        for struct in [x for x in self.vk.structs.values() if needSafeStruct(x) and x.name not in wsiStructs and re.match(splitRegex, x.name)]:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])

            init_list = ''          # list of members in struct constructor initializer
            default_init_list = ''  # Default constructor just inits ptrs to nullptr in initializer
            init_func_txt = ''      # Txt for initialize() function that takes struct ptr and inits members
            construct_txt = ''      # Body of constuctor as well as body of initialize() func following init_func_txt
            destruct_txt = ''

            has_pnext = struct.sType is not None
            copy_pnext = ''
            copy_pnext_if = ''
            copy_strings = ''
            for member in struct.members:
                m_type = member.type
                if member.name == 'pNext':
                    copy_pnext = 'pNext = SafePnextCopy(in_struct->pNext, copy_state);\n'
                    copy_pnext_if = '''
                    if (copy_pnext) {
                        pNext = SafePnextCopy(in_struct->pNext, copy_state);
                    }'''
                if member.type in self.vk.structs and needSafeStruct(self.vk.structs[member.type]):
                    m_type = f'safe_{member.type}'
                if member.pointer and 'safe_' not in m_type and 'PFN_' not in member.type and not self.typeContainsObjectHandle(member.type, False):
                    # Ptr types w/o a safe_struct, for non-null case need to allocate new ptr and copy data in
                    if m_type in ['void', 'char']:
                        if member.name != 'pNext':
                            if m_type == 'char':
                                # Create deep copies of strings
                                if member.length:
                                    copy_strings += f'''
                                        char **tmp_{member.name} = new char *[in_struct->{member.length}];
                                        for (uint32_t i = 0; i < {member.length}; ++i) {{
                                            tmp_{member.name}[i] = SafeStringCopy(in_struct->{member.name}[i]);
                                        }}
                                        {member.name} = tmp_{member.name};'''

                                    destruct_txt += f'''
                                        if ({member.name}) {{
                                            for (uint32_t i = 0; i < {member.length}; ++i) {{
                                                delete [] {member.name}[i];
                                            }}
                                            delete [] {member.name};
                                        }}'''
                                else:
                                    copy_strings += f'{member.name} = SafeStringCopy(in_struct->{member.name});\n'
                                    destruct_txt += f'if ({member.name}) delete [] {member.name};\n'
                            else:
                                # We need a deep copy of pData / dataSize combos
                                if member.name == 'pData':
                                    init_list += f'\n    {member.name}(nullptr),'
                                    construct_txt += '''
                                        if (in_struct->pData != nullptr) {
                                            auto temp = new std::byte[in_struct->dataSize];
                                            std::memcpy(temp, in_struct->pData, in_struct->dataSize);
                                            pData = temp;
                                        }
                                        '''

                                    destruct_txt  += '''
                                        if (pData != nullptr) {
                                            auto temp = reinterpret_cast<const std::byte*>(pData);
                                            delete [] temp;
                                        }
                                        '''
                                else:
                                    init_list += f'\n{member.name}(in_struct->{member.name}),'
                                    init_func_txt += f'{member.name} = in_struct->{member.name};\n'
                        default_init_list += f'\n{member.name}(nullptr),'
                    else:
                        default_init_list += f'\n{member.name}(nullptr),'
                        init_list += f'\n{member.name}(nullptr),'
                        if m_type in abstractTypes:
                            construct_txt += f'{member.name} = in_struct->{member.name};\n'
                        else:
                            init_func_txt += f'{member.name} = nullptr;\n'
                            if not member.fixedSizeArray and (member.length is None or '/' in member.length):
                                construct_txt += f'''
                                    if (in_struct->{member.name}) {{
                                            {member.name} = new {m_type}(*in_struct->{member.name});
                                        }}
                                    '''
                                destruct_txt += f'if ({member.name})\n'
                                destruct_txt += f'    delete {member.name};\n'
                            else:
                                # Prepend struct members with struct name
                                decorated_length = member.length
                                for other_member in struct.members:
                                    decorated_length = re.sub(r'\b({})\b'.format(other_member.name), r'in_struct->\1', decorated_length)
                                try:
                                    concurrent_clause = member_construct_conditions[member.name](struct, member)
                                except:
                                    concurrent_clause = (f'in_struct->{member.name}', lambda x: '')
                                construct_txt += f'''
                                    if ({concurrent_clause[0]}) {{
                                        {member.name} = new {m_type}[{decorated_length}];
                                        memcpy ((void *){member.name}, (void *)in_struct->{member.name}, sizeof({m_type})*{decorated_length});
                                        {concurrent_clause[1]('        ')}'''
                                if len(concurrent_clause) > 2:
                                    construct_txt += '} else {\n'
                                    construct_txt += concurrent_clause[2]('        ')
                                construct_txt += '}\n'
                                destruct_txt += f'if ({member.name})\n'
                                destruct_txt += f'    delete[] {member.name};\n'
                elif member.fixedSizeArray or member.length is not None:
                    if member.fixedSizeArray:
                        construct_txt += f'''
                            for (uint32_t i = 0; i < {member.fixedSizeArray[0]}; ++i) {{
                                    {member.name}[i] = in_struct->{member.name}[i];
                                }}
                            '''
                    else:
                        # Init array ptr to NULL
                        default_init_list += f'\n{member.name}(nullptr),'
                        init_list += f'\n{member.name}(nullptr),'
                        init_func_txt += f'{member.name} = nullptr;\n'
                        array_element = f'in_struct->{member.name}[i]'
                        if member.type in self.vk.structs and needSafeStruct(self.vk.structs[member.type]):
                            array_element = f'{member.type}(&in_struct->safe_{member.name}[i])'
                        construct_txt += f'if ({member.length} && in_struct->{member.name}) {{\n'
                        construct_txt += f'    {member.name} = new {m_type}[{member.length}];\n'
                        destruct_txt += f'if ({member.name})\n'
                        destruct_txt += f'    delete[] {member.name};\n'
                        construct_txt += f'for (uint32_t i = 0; i < {member.length}; ++i) {{\n'
                        if 'safe_' in m_type:
                            construct_txt += f'{member.name}[i].initialize(&in_struct->{member.name}[i]);\n'
                        else:
                            construct_txt += f'{member.name}[i] = {array_element};\n'
                        construct_txt += '}\n'
                        construct_txt += '}\n'
                elif member.pointer and 'PFN_' not in member.type:
                    default_init_list += f'\n{member.name}(nullptr),'
                    init_list += f'\n{member.name}(nullptr),'
                    init_func_txt += f'{member.name} = nullptr;\n'
                    construct_txt += f'if (in_struct->{member.name})\n'
                    construct_txt += f'    {member.name} = new {m_type}(in_struct->{member.name});\n'
                    destruct_txt += f'if ({member.name})\n'
                    destruct_txt += f'    delete {member.name};\n'
                elif 'safe_' in m_type and member.type == 'VkDescriptorDataEXT':
                    init_list += f'\n{member.name}(&in_struct->{member.name}, in_struct->type),'
                    init_func_txt += f'{member.name}.initialize(&in_struct->{member.name}, in_struct->type);\n'
                elif 'safe_' in m_type:
                    init_list += f'\n{member.name}(&in_struct->{member.name}),'
                    init_func_txt += f'{member.name}.initialize(&in_struct->{member.name});\n'
                else:
                    try:
                        init_list += f'\n{member_init_transforms[member.name](member)},'
                    except:
                        init_list += f'\n{member.name}(in_struct->{member.name}),'
                        init_func_txt += f'{member.name} = in_struct->{member.name};\n'
                    if not struct.union:
                        if member.name == 'sType' and struct.sType:
                            default_init_list += f'\n{member.name}({struct.sType}),'
                        else:
                            default_init_list += f'\n{member.name}(),'
            if '' != init_list:
                init_list = init_list[:-1] # hack off final comma

            if struct.name in custom_definitions:
                out.append(custom_definitions[struct.name])

            if struct.name in custom_construct_txt:
                construct_txt = custom_construct_txt[struct.name]

            construct_txt = copy_strings + construct_txt

            if struct.name in custom_destruct_txt:
                destruct_txt = custom_destruct_txt[struct.name]

            copy_pnext_param = ''
            if has_pnext:
                copy_pnext_param = ', bool copy_pnext'
                destruct_txt += '    FreePnextChain(pNext);\n'

            if struct.union:
                if (struct.name == 'VkDescriptorDataEXT'):
                    default_init_list = ' type_at_end {0},'
                    out.append(f'''
                        safe_{struct.name}::safe_{struct.name}(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, [[maybe_unused]] PNextCopyState* copy_state{copy_pnext_param})
                        {{
                        {copy_pnext + construct_txt}}}
                        ''')
                else:
                    # Unions don't allow multiple members in the initialization list, so just call initialize
                    out.append(f'''
                        safe_{struct.name}::safe_{struct.name}(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, PNextCopyState*)
                        {{
                            initialize(in_struct);
                        }}
                        ''')
            else:
                out.append(f'''
                    safe_{struct.name}::safe_{struct.name}(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, [[maybe_unused]] PNextCopyState* copy_state{copy_pnext_param}) :{init_list}
                    {{
                    {copy_pnext_if + construct_txt}}}
                    ''')
            if '' != default_init_list:
                default_init_list = f' :{default_init_list[:-1]}'
            default_init_body = '\n' + custom_defeault_construct_txt[struct.name] if struct.name in custom_defeault_construct_txt else ''
            out.append(f'''
                safe_{struct.name}::safe_{struct.name}(){default_init_list}
                {{{default_init_body}}}
                ''')
            # Create slight variation of init and construct txt for copy constructor that takes a copy_src object reference vs. struct ptr
            construct_txt = copy_pnext + construct_txt
            copy_construct_init = init_func_txt.replace('in_struct->', 'copy_src.')
            copy_construct_init = copy_construct_init.replace(', copy_state', '')
            if struct.name == 'VkDescriptorGetInfoEXT':
                copy_construct_init = copy_construct_init.replace(', copy_src.type', '')
            # Pass object to copy constructors
            copy_construct_txt = re.sub('(new \\w+)\\(in_struct->', '\\1(*copy_src.', construct_txt)
            # Modify remaining struct refs for copy_src object
            copy_construct_txt = copy_construct_txt.replace('in_struct->', 'copy_src.')
            # Modify remaining struct refs for copy_src object
            copy_construct_txt = copy_construct_txt .replace(', copy_state', '')
            if struct.name in custom_copy_txt:
                copy_construct_txt = custom_copy_txt[struct.name]
            copy_assign_txt = '    if (&copy_src == this) return *this;\n\n' + destruct_txt + '\n' + copy_construct_init + copy_construct_txt + '\n    return *this;'
            # Copy constructor
            out.append(f'''
                safe_{struct.name}::safe_{struct.name}(const safe_{struct.name}& copy_src)
                {{
                {copy_construct_init}{copy_construct_txt}}}
                ''')
            # Copy assignment operator
            out.append(f'''
                safe_{struct.name}& safe_{struct.name}::operator=(const safe_{struct.name}& copy_src)\n{{
                {copy_assign_txt}
                }}
                ''')
            out.append(f'''
                safe_{struct.name}::~safe_{struct.name}()
                {{
                {destruct_txt}}}
                ''')
            out.append(f'''
                void safe_{struct.name}::initialize(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, [[maybe_unused]] PNextCopyState* copy_state)
                {{
                {destruct_txt}{init_func_txt}{construct_txt}}}
                ''')
            # Copy initializer uses same txt as copy constructor but has a ptr and not a reference
            init_copy = copy_construct_init.replace('copy_src.', 'copy_src->')
            # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            init_copy = re.sub(r'&copy_src(?!->)', 'copy_src', init_copy)
            init_construct = copy_construct_txt.replace('copy_src.', 'copy_src->')
            # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            init_construct = re.sub(r'&copy_src(?!->)', 'copy_src', init_construct)
            out.append(f'''
                void safe_{struct.name}::initialize(const safe_{struct.name}* copy_src, [[maybe_unused]] PNextCopyState* copy_state)
                {{
                {init_copy}{init_construct}}}
                ''')
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])

        self.write("".join(out))
