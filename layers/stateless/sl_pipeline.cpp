/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

bool StatelessValidation::manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPipelineLayout *pPipelineLayout) const {
    bool skip = false;
    // Validate layout count against device physical limit
    if (pCreateInfo->setLayoutCount > device_limits.maxBoundDescriptorSets) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286",
                         "vkCreatePipelineLayout(): setLayoutCount (%" PRIu32
                         ") exceeds physical device maxBoundDescriptorSets limit (%" PRIu32 ").",
                         pCreateInfo->setLayoutCount, device_limits.maxBoundDescriptorSets);
    }

    if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-06561",
                                 "vkCreatePipelineLayout(): pSetLayouts[%" PRIu32
                                 "] is VK_NULL_HANDLE, but VK_EXT_graphics_pipeline_library is not enabled.",
                                 i);
            }
        }
    }

    // Validate Push Constant ranges
    for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        const uint32_t offset = pCreateInfo->pPushConstantRanges[i].offset;
        const uint32_t size = pCreateInfo->pPushConstantRanges[i].size;
        const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
        // Check that offset + size don't exceed the max.
        // Prevent arithetic overflow here by avoiding addition and testing in this order.
        if (offset >= max_push_constants_size) {
            skip |= LogError(device, "VUID-VkPushConstantRange-offset-00294",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].offset (%" PRIu32
                             ") that exceeds this "
                             "device's maxPushConstantSize of %" PRIu32 ".",
                             i, offset, max_push_constants_size);
        }
        if (size > max_push_constants_size - offset) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00298",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "] offset (%" PRIu32
                             ") and size (%" PRIu32
                             ") "
                             "together exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                             i, offset, size, max_push_constants_size);
        }

        // size needs to be non-zero and a multiple of 4.
        if (size == 0) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00296",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].size (%" PRIu32
                             ") is not greater than zero.",
                             i, size);
        }
        if (size & 0x3) {
            skip |= LogError(device, "VUID-VkPushConstantRange-size-00297",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].size (%" PRIu32
                             ") is not a multiple of 4.",
                             i, size);
        }

        // offset needs to be a multiple of 4.
        if ((offset & 0x3) != 0) {
            skip |= LogError(device, "VUID-VkPushConstantRange-offset-00295",
                             "vkCreatePipelineLayout(): pCreateInfo->pPushConstantRanges[%" PRIu32 "].offset (%" PRIu32
                             ") is not a multiple of 4.",
                             i, offset);
        }
    }

    // As of 1.0.28, there is a VU that states that a stage flag cannot appear more than once in the list of push constant ranges.
    for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        for (uint32_t j = i + 1; j < pCreateInfo->pushConstantRangeCount; ++j) {
            if (0 != (pCreateInfo->pPushConstantRanges[i].stageFlags & pCreateInfo->pPushConstantRanges[j].stageFlags)) {
                skip |=
                    LogError(device, "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                             "vkCreatePipelineLayout() Duplicate stage flags found in ranges %" PRIu32 " and %" PRIu32 ".", i, j);
                break;  // Only need to report the first range mismatch
            }
        }
    }
    return skip;
}

bool StatelessValidation::ValidatePipelineShaderStageCreateInfo(const char *func_name, const char *msg,
                                                                const VkPipelineShaderStageCreateInfo *pCreateInfo) const {
    bool skip = false;

    const auto *required_subgroup_size_features =
        LvlFindInChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(pCreateInfo->pNext);

    if (required_subgroup_size_features) {
        if ((pCreateInfo->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageCreateInfo-pNext-02754",
                "%s(): %s->flags (0x%x) includes VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT while "
                "VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT is included in the pNext chain.",
                func_name, msg, pCreateInfo->flags);
        }
    }
    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo &info,
                                                                      uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pTessellationState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO};

    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pTessellationState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineTessellationDomainOriginStateCreateInfo", info.pNext, allowed_structs.size(), allowed_structs.data(),
        GeneratedVulkanHeaderVersion, "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineTessellationStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pTessellationState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineTessellationStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo &info,
                                                                     uint32_t index) const {
    bool skip = false;

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT};
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pVertexInputState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineVertexInputDivisorStateCreateInfoEXT", info.pNext, allowed_structs.size(), allowed_structs.data(),
        GeneratedVulkanHeaderVersion, "VUID-VkPipelineVertexInputStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineVertexInputStateCreateInfo-sType-unique");
    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pVertexInputState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineVertexInputStateCreateInfo-sType-sType");
    skip |= ValidateArray(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pVertexInputState->vertexBindingDescriptionCount", ParameterName::IndexVector{index}),
        "pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions", info.vertexBindingDescriptionCount,
        &info.pVertexBindingDescriptions, false, true, kVUIDUndefined,
        "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-parameter");

    skip |= ValidateArray(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pVertexInputState->vertexAttributeDescriptionCount", ParameterName::IndexVector{index}),
        "pCreateInfos[i]->pVertexAttributeDescriptions", info.vertexAttributeDescriptionCount, &info.pVertexAttributeDescriptions,
        false, true, kVUIDUndefined, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-parameter");

    if (info.pVertexBindingDescriptions != nullptr) {
        for (uint32_t vertex_binding_description_index = 0; vertex_binding_description_index < info.vertexBindingDescriptionCount;
             ++vertex_binding_description_index) {
            skip |=
                ValidateRangedEnum("vkCreateGraphicsPipelines",
                                   ParameterName("pCreateInfos[%i].pVertexInputState->pVertexBindingDescriptions[%i].inputRate",
                                                 ParameterName::IndexVector{index, vertex_binding_description_index}),
                                   "VkVertexInputRate", info.pVertexBindingDescriptions[vertex_binding_description_index].inputRate,
                                   "VUID-VkVertexInputBindingDescription-inputRate-parameter");
        }
    }

    if (info.pVertexAttributeDescriptions != nullptr) {
        for (uint32_t vertex_attribute_description_index = 0;
             vertex_attribute_description_index < info.vertexAttributeDescriptionCount; ++vertex_attribute_description_index) {
            const VkFormat format = info.pVertexAttributeDescriptions[vertex_attribute_description_index].format;
            skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                                       ParameterName("pCreateInfos[%i].pVertexInputState->pVertexAttributeDescriptions[%i].format",
                                                     ParameterName::IndexVector{index, vertex_attribute_description_index}),
                                       "VkFormat", info.pVertexAttributeDescriptions[vertex_attribute_description_index].format,
                                       "VUID-VkVertexInputAttributeDescription-format-parameter");
            if (FormatIsDepthOrStencil(format)) {
                // Should never hopefully get here, but there are known driver advertising the wrong feature flags
                // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
                skip |= LogError(device, kVUID_Core_invalidDepthStencilFormat,
                                 "vkCreateGraphicsPipelines: "
                                 "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                 "].format is a "
                                 "depth/stencil format (%s) but depth/stencil formats do not have a defined sizes for "
                                 "alignment, replace with a color format.",
                                 index, vertex_attribute_description_index, string_VkFormat(format));
            }
        }
    }

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pVertexInputState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineVertexInputStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo &info,
                                                                  uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pViewportState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineViewportStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV,
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT,
    };
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pViewportState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineViewportSwizzleStateCreateInfoNV, VkPipelineViewportWScalingStateCreateInfoNV, "
        "VkPipelineViewportExclusiveScissorStateCreateInfoNV, VkPipelineViewportShadingRateImageStateCreateInfoNV, "
        "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV, VkPipelineViewportDepthClipControlCreateInfoEXT",
        info.pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
        "VUID-VkPipelineViewportStateCreateInfo-pNext-pNext", "VUID-VkPipelineViewportStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pViewportState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineViewportStateCreateInfo-flags-zerobitmask");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &info,
                                                                     uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pMultisampleState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineMultisampleStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV,
                                            VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT};
    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pMultisampleState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineCoverageModulationStateCreateInfoNV, VkPipelineCoverageReductionStateCreateInfoNV, "
        "VkPipelineCoverageToColorStateCreateInfoNV, VkPipelineSampleLocationsStateCreateInfoEXT",
        info.pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
        "VUID-VkPipelineMultisampleStateCreateInfo-pNext-pNext", "VUID-VkPipelineMultisampleStateCreateInfo-sType-unique");

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pMultisampleState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineMultisampleStateCreateInfo-flags-zerobitmask");

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pMultisampleState->sampleShadingEnable", ParameterName::IndexVector{index}),
                       info.sampleShadingEnable);

    skip |=
        ValidateArray("vkCreateGraphicsPipelines",
                      ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{index}),
                      ParameterName("pCreateInfos[%i].pMultisampleState->pSampleMask", ParameterName::IndexVector{index}),
                      info.rasterizationSamples, &info.pSampleMask, true, false, kVUIDUndefined,
                      "VUID-VkPipelineMultisampleStateCreateInfo-pSampleMask-parameter");

    skip |=
        ValidateFlags("vkCreateGraphicsPipelines",
                      ParameterName("pCreateInfos[%i].pMultisampleState->rasterizationSamples", ParameterName::IndexVector{index}),
                      "VkSampleCountFlagBits", AllVkSampleCountFlagBits, info.rasterizationSamples, kRequiredSingleBit,
                      "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pMultisampleState->alphaToCoverageEnable", ParameterName::IndexVector{index}),
        info.alphaToCoverageEnable);

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pMultisampleState->alphaToOneEnable", ParameterName::IndexVector{index}),
                           info.alphaToOneEnable);
    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &attachment_state,
                                                                    uint32_t pipe_index, uint32_t attachment_index) const {
    bool skip = false;

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].blendEnable",
                                         ParameterName::IndexVector{pipe_index, attachment_index}),
                           attachment_state.blendEnable);

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcColorBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.srcColorBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstColorBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.dstColorBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorBlendOp",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendOp", attachment_state.colorBlendOp,
                               "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].srcAlphaBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.srcAlphaBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].dstAlphaBlendFactor",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendFactor", attachment_state.dstAlphaBlendFactor,
                               "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].alphaBlendOp",
                                             ParameterName::IndexVector{pipe_index, attachment_index}),
                               "VkBlendOp", attachment_state.alphaBlendOp,
                               "VUID-VkPipelineColorBlendAttachmentState-alphaBlendOp-parameter");

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments[%i].colorWriteMask",
                                        ParameterName::IndexVector{pipe_index, attachment_index}),
                          "VkColorComponentFlagBits", AllVkColorComponentFlagBits, attachment_state.colorWriteMask, kOptionalFlags,
                          "VUID-VkPipelineColorBlendAttachmentState-colorWriteMask-parameter");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo &info,
                                                                    uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pColorBlendState", ParameterName::IndexVector{index}),
        "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO", &info, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        false, kVUIDUndefined, "VUID-VkPipelineColorBlendStateCreateInfo-sType-sType");

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT,
                                            VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT};

    skip |= ValidateStructPnext(
        "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].pColorBlendState->pNext", ParameterName::IndexVector{index}),
        "VkPipelineColorBlendAdvancedStateCreateInfoEXT, VkPipelineColorWriteCreateInfoEXT", info.pNext, allowed_structs.size(),
        allowed_structs.data(), GeneratedVulkanHeaderVersion, "VUID-VkPipelineColorBlendStateCreateInfo-pNext-pNext",
        "VUID-VkPipelineColorBlendStateCreateInfo-sType-unique");

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pColorBlendState->flags", ParameterName::IndexVector{index}),
                          "VkPipelineColorBlendStateCreateFlagBits", AllVkPipelineColorBlendStateCreateFlagBits, info.flags,
                          kOptionalFlags, "VUID-VkPipelineColorBlendStateCreateInfo-flags-parameter");

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pColorBlendState->logicOpEnable", ParameterName::IndexVector{index}),
                           info.logicOpEnable);

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo &info,
                                                                      uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineDepthStencilStateCreateInfo-sType-sType");

    skip |= ValidateStructPnext("vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pDepthStencilState->pNext", ParameterName::IndexVector{index}),
                                nullptr, info.pNext, 0, nullptr, GeneratedVulkanHeaderVersion,
                                "VUID-VkPipelineDepthStencilStateCreateInfo-pNext-pNext", nullptr);

    skip |= ValidateFlags("vkCreateGraphicsPipelines",
                          ParameterName("pCreateInfos[%i].pDepthStencilState->flags", ParameterName::IndexVector{index}),
                          "VkPipelineDepthStencilStateCreateFlagBits", AllVkPipelineDepthStencilStateCreateFlagBits, info.flags,
                          kOptionalFlags, "VUID-VkPipelineDepthStencilStateCreateInfo-flags-parameter");

    skip |= ValidateBool32("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->depthTestEnable", ParameterName::IndexVector{index}),
                           info.depthTestEnable);

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pDepthStencilState->depthWriteEnable", ParameterName::IndexVector{index}),
                       info.depthWriteEnable);

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->depthCompareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.depthCompareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->depthBoundsTestEnable", ParameterName::IndexVector{index}),
        info.depthBoundsTestEnable);

    skip |=
        ValidateBool32("vkCreateGraphicsPipelines",
                       ParameterName("pCreateInfos[%i].pDepthStencilState->stencilTestEnable", ParameterName::IndexVector{index}),
                       info.stencilTestEnable);

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->front.failOp", ParameterName::IndexVector{index}),
                           "VkStencilOp", info.front.failOp, "VUID-VkStencilOpState-failOp-parameter");

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pDepthStencilState->front.passOp", ParameterName::IndexVector{index}),
                           "VkStencilOp", info.front.passOp, "VUID-VkStencilOpState-passOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->front.depthFailOp", ParameterName::IndexVector{index}), "VkStencilOp",
        info.front.depthFailOp, "VUID-VkStencilOpState-depthFailOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->front.compareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.front.compareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState->back.failOp", ParameterName::IndexVector{index}),
                               "VkStencilOp", info.back.failOp, "VUID-VkStencilOpState-failOp-parameter");

    skip |= ValidateRangedEnum("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pDepthStencilState->back.passOp", ParameterName::IndexVector{index}),
                               "VkStencilOp", info.back.passOp, "VUID-VkStencilOpState-passOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->back.depthFailOp", ParameterName::IndexVector{index}), "VkStencilOp",
        info.back.depthFailOp, "VUID-VkStencilOpState-depthFailOp-parameter");

    skip |= ValidateRangedEnum(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pDepthStencilState->back.compareOp", ParameterName::IndexVector{index}), "VkCompareOp",
        info.back.compareOp, "VUID-VkPipelineDepthStencilStateCreateInfo-depthCompareOp-parameter");

    return skip;
}

// TODO Issue 4847 - Move logic to be autogenerated
bool StatelessValidation::ValidatePipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo &info,
                                                                       uint32_t index) const {
    bool skip = false;

    skip |= ValidateStructType("vkCreateGraphicsPipelines",
                               ParameterName("pCreateInfos[%i].pInputAssemblyState", ParameterName::IndexVector{index}),
                               "VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO", &info,
                               VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, false, kVUIDUndefined,
                               "VUID-VkPipelineInputAssemblyStateCreateInfo-sType-sType");

    skip |= ValidateStructPnext("vkCreateGraphicsPipelines",
                                ParameterName("pCreateInfos[%i].pInputAssemblyState->pNext", ParameterName::IndexVector{index}),
                                nullptr, info.pNext, 0, nullptr, GeneratedVulkanHeaderVersion,
                                "VUID-VkPipelineInputAssemblyStateCreateInfo-pNext-pNext", nullptr);

    skip |= ValidateReservedFlags("vkCreateGraphicsPipelines",
                                  ParameterName("pCreateInfos[%i].pInputAssemblyState->flags", ParameterName::IndexVector{index}),
                                  info.flags, "VUID-VkPipelineInputAssemblyStateCreateInfo-flags-zerobitmask");

    skip |=
        ValidateRangedEnum("vkCreateGraphicsPipelines",
                           ParameterName("pCreateInfos[%i].pInputAssemblyState->topology", ParameterName::IndexVector{index}),
                           "VkPrimitiveTopology", info.topology, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-parameter");

    skip |= ValidateBool32(
        "vkCreateGraphicsPipelines",
        ParameterName("pCreateInfos[%i].pInputAssemblyState->primitiveRestartEnable", ParameterName::IndexVector{index}),
        info.primitiveRestartEnable);

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t createInfoCount,
                                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines) const {
    bool skip = false;

    if (pCreateInfos != nullptr) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            bool has_pre_raster_state = true;
            // Create a copy of create_info and set non-included sub-state to null
            auto create_info = pCreateInfos[i];
            const auto *graphics_lib_info = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
            if (graphics_lib_info) {
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT)) {
                    create_info.pVertexInputState = nullptr;
                    create_info.pInputAssemblyState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT)) {
                    has_pre_raster_state = false;
                    create_info.pViewportState = nullptr;
                    create_info.pRasterizationState = nullptr;
                    create_info.pTessellationState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
                    create_info.pDepthStencilState = nullptr;
                }
                if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
                    create_info.pColorBlendState = nullptr;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                    create_info.pMultisampleState = nullptr;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT))) {
                    create_info.layout = VK_NULL_HANDLE;
                }
                if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                    create_info.renderPass = VK_NULL_HANDLE;
                    create_info.subpass = 0;
                }
            }

            // Values needed from either dynamic rendering or the subpass description
            uint32_t color_attachment_count = 0;

            if (!create_info.renderPass) {
                if (create_info.pColorBlendState && create_info.pMultisampleState) {
                    const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
                    // Pipeline has fragment output state
                    if (rendering_struct) {
                        color_attachment_count = rendering_struct->colorAttachmentCount;

                        if ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED)) {
                            skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "stencilAttachmentFormat", "VkFormat",
                                                       rendering_struct->stencilAttachmentFormat,
                                                       "VUID-VkGraphicsPipelineCreateInfo-renderPass-06583");

                            if (!FormatHasDepth(rendering_struct->depthAttachmentFormat)) {
                                skip |= LogError(
                                    device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06587",
                                    "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                    "]: VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s) does not have a depth aspect.",
                                    i, string_VkFormat(rendering_struct->depthAttachmentFormat));
                            }
                        }

                        if ((rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)) {
                            skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "stencilAttachmentFormat", "VkFormat",
                                                       rendering_struct->stencilAttachmentFormat,
                                                       "VUID-VkGraphicsPipelineCreateInfo-renderPass-06584");
                            if (!FormatHasStencil(rendering_struct->stencilAttachmentFormat)) {
                                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06588",
                                                 "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                                 "]: VkPipelineRenderingCreateInfo::stencilAttachmentFormat  (%s) does not have a "
                                                 "stencil aspect.",
                                                 i, string_VkFormat(rendering_struct->stencilAttachmentFormat));
                            }
                        }

                        if (color_attachment_count != 0) {
                            skip |= ValidateRangedEnumArray(
                                "VkPipelineRenderingCreateInfo", "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579",
                                "colorAttachmentCount", "pColorAttachmentFormats", "VkFormat", color_attachment_count,
                                rendering_struct->pColorAttachmentFormats, true, true);
                        }

                        if (rendering_struct->pColorAttachmentFormats) {
                            for (uint32_t j = 0; j < color_attachment_count; ++j) {
                                skip |= ValidateRangedEnum("VkPipelineRenderingCreateInfo", "pColorAttachmentFormats", "VkFormat",
                                                           rendering_struct->pColorAttachmentFormats[j],
                                                           "VUID-VkGraphicsPipelineCreateInfo-renderPass-06580");
                            }
                        }
                    }

                    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
                    auto attachment_sample_count_info = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(create_info.pNext);
                    if (attachment_sample_count_info && attachment_sample_count_info->pColorAttachmentSamples) {
                        color_attachment_count = attachment_sample_count_info->colorAttachmentCount;

                        for (uint32_t j = 0; j < color_attachment_count; ++j) {
                            skip |= ValidateFlags("vkCreateGraphicsPipelines",
                                                  ParameterName("VkAttachmentSampleCountInfoAMD->pColorAttachmentSamples"),
                                                  "VkSampleCountFlagBits", AllVkSampleCountFlagBits,
                                                  attachment_sample_count_info->pColorAttachmentSamples[j], kRequiredFlags,
                                                  "VUID-VkGraphicsPipelineCreateInfo-pColorAttachmentSamples-06592");
                        }
                    }
                }
            }

            const VkPipelineCreateFlags flags = create_info.flags;
            if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
                if (create_info.stageCount == 0) {
                    skip |=
                        LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stageCount-06604",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "].stageCount is 0, but %s is not enabled", i,
                                 VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
                }
                if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03371",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "]->flags (0x%x) must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                                     i, flags);
                }

                // TODO while PRIu32 should probably be used instead of %i below, %i is necessary due to
                // ParameterName::IndexFormatSpecifier
                skip |= ValidateStructTypeArray(
                    "vkCreateGraphicsPipelines", ParameterName("pCreateInfos[%i].stageCount", ParameterName::IndexVector{i}),
                    ParameterName("pCreateInfos[%i].pStages", ParameterName::IndexVector{i}),
                    "VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO", create_info.stageCount, create_info.pStages,
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, true, true,
                    "VUID-VkPipelineShaderStageCreateInfo-sType-sType", "VUID-VkGraphicsPipelineCreateInfo-pStages-06600",
                    "VUID-VkGraphicsPipelineCreateInfo-pStages-06600");
                skip |=
                    ValidateStructType("vkCreateGraphicsPipelines",
                                       ParameterName("pCreateInfos[%i].pRasterizationState", ParameterName::IndexVector{i}),
                                       "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO",
                                       create_info.pRasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                       true, "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-06601",
                                       "VUID-VkPipelineRasterizationStateCreateInfo-sType-sType");
            }

            // <VkDynamicState, index in pDynamicStates, hash for enum key>
            vvl::unordered_map<VkDynamicState, uint32_t, std::hash<int>> dynamic_state_map;
            // TODO probably should check dynamic state from graphics libraries, at least when creating an "executable pipeline"
            if (create_info.pDynamicState != nullptr) {
                const auto &dynamic_state_info = *create_info.pDynamicState;
                for (uint32_t state_index = 0; state_index < dynamic_state_info.dynamicStateCount; ++state_index) {
                    const VkDynamicState dynamic_state = dynamic_state_info.pDynamicStates[state_index];

                    if (vvl::Contains(dynamic_state_map, dynamic_state)) {
                        skip |= LogError(device, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442",
                                         "vkCreateGraphicsPipelines: %s was listed twice in the "
                                         "pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32
                                         "] and pDynamicStates[%" PRIu32 "]",
                                         string_VkDynamicState(dynamic_state), i, dynamic_state_map[dynamic_state], state_index);
                    }

                    dynamic_state_map[dynamic_state] = state_index;
                }
            }

            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
                // Not allowed for graphics pipelines
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03578",
                                 "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR was listed the "
                                 "pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates[%" PRIu32
                                 "] but not allowed in graphic pipelines.",
                                 i, dynamic_state_map[VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR]);
            }

            if (has_pre_raster_state && vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) &&
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04132",
                                 "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT and "
                                 "VK_DYNAMIC_STATE_VIEWPORT both listed in pCreateInfos[%" PRIu32
                                 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32 "] and pDynamicStates[%" PRIu32
                                 "] respectfully.",
                                 i, dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT],
                                 dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT]);
            }

            if (has_pre_raster_state && vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) &&
                vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR)) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04133",
                    "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT and VK_DYNAMIC_STATE_SCISSOR "
                    "both listed in pCreateInfos[%" PRIu32 "].pDynamicState->pDynamicStates array at pDynamicStates[%" PRIu32
                    "] and pDynamicStates[%" PRIu32 "] respectfully.",
                    i, dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT], dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR]);
            }

            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT) &&
                discard_rectangles_extension_version < 2) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07855",
                    "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT is listed in pCreateInfos[%" PRIu32
                    "].pDynamicState->pDynamicStates[%" PRIu32 "] without support for version 2 of VK_EXT_discard_rectangles.",
                    i, dynamic_state_map[VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT]);
            }
            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT) &&
                discard_rectangles_extension_version < 2) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07856",
                    "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT is listed in pCreateInfos[%" PRIu32
                    "].pDynamicState->pDynamicStates[%" PRIu32 "] without support for version 2 of VK_EXT_discard_rectangles.",
                    i, dynamic_state_map[VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT]);
            }
            if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV) &&
                scissor_exclusive_extension_version < 2) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07854",
                    "vkCreateGraphicsPipelines: VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV is listed in pCreateInfos[%" PRIu32
                    "].pDynamicState->pDynamicStates[%" PRIu32 "] without support for version 2 of VK_NV_scissor_exclusive.",
                    i, dynamic_state_map[VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV]);
            }

            auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(create_info.pNext);
            if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0 &&
                                                 feedback_struct->pipelineStageCreationFeedbackCount != create_info.stageCount)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594",
                                 "vkCreateGraphicsPipelines(): in pCreateInfo[%" PRIu32
                                 "], VkPipelineCreationFeedbackEXT::pipelineStageCreationFeedbackCount"
                                 "(=%" PRIu32 ") must equal VkGraphicsPipelineCreateInfo::stageCount(=%" PRIu32 ").",
                                 i, feedback_struct->pipelineStageCreationFeedbackCount, create_info.stageCount);
            }

            // helpers for bool used multiple times below
            const bool has_dynamic_viewport = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT);
            const bool has_dynamic_scissor = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR);
            const bool has_dynamic_viewport_w_scaling_nv = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
            const bool has_dynamic_exclusive_scissor_nv = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
            const bool has_dynamic_viewport_with_count = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
            const bool has_dynamic_scissor_with_count = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
            const bool has_dynamic_vertex_input = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);

            // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml

            // Collect active stages and other information
            // Only want to loop through pStages once
            uint32_t active_shaders = 0;
            if (create_info.pStages != nullptr) {
                for (uint32_t stage_index = 0; stage_index < create_info.stageCount; ++stage_index) {
                    active_shaders |= create_info.pStages[stage_index].stage;

                    skip |= ValidateRequiredPointer(
                        "vkCreateGraphicsPipelines",
                        ParameterName("pCreateInfos[%i].stage[%i].pName", ParameterName::IndexVector{i, stage_index}),
                        create_info.pStages[stage_index].pName, "VUID-VkPipelineShaderStageCreateInfo-pName-parameter");

                    if (create_info.pStages[stage_index].pName) {
                        skip |= ValidateString(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pStages[%i].pName", ParameterName::IndexVector{i, stage_index}),
                            "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", create_info.pStages[stage_index].pName);
                    }

                    std::stringstream msg;
                    msg << "pCreateInfos[%" << i << "].pStages[%" << stage_index << "]";
                    ValidatePipelineShaderStageCreateInfo("vkCreateGraphicsPipelines", msg.str().c_str(),
                                                          &create_info.pStages[stage_index]);
                }
            }

            if (has_pre_raster_state && (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
                (active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
                if (create_info.pTessellationState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00731",
                                     "vkCreateGraphicsPipelines: if pCreateInfos[%" PRIu32
                                     "].pStages includes a tessellation control "
                                     "shader stage and a tessellation evaluation shader stage, "
                                     "pCreateInfos[%" PRIu32 "].pTessellationState must not be NULL.",
                                     i, i);
                } else {
                    skip |= ValidatePipelineTessellationStateCreateInfo(*create_info.pTessellationState, i);

                    if (create_info.pTessellationState->patchControlPoints == 0 ||
                        create_info.pTessellationState->patchControlPoints > device_limits.maxTessellationPatchSize) {
                        skip |=
                            LogError(device, "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214",
                                     "vkCreateGraphicsPipelines: invalid parameter "
                                     "pCreateInfos[%" PRIu32 "].pTessellationState->patchControlPoints value %" PRIu32
                                     ". patchControlPoints "
                                     "should be >0 and <=%" PRIu32 ".",
                                     i, create_info.pTessellationState->patchControlPoints, device_limits.maxTessellationPatchSize);
                    }
                }
            }

            if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) && (create_info.pInputAssemblyState != nullptr)) {
                skip |= ValidatePipelineInputAssemblyStateCreateInfo(*create_info.pInputAssemblyState, i);
            }

            if (!has_dynamic_vertex_input && !(active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) &&
                (create_info.pVertexInputState != nullptr)) {
                auto const &vertex_input_state = create_info.pVertexInputState;

                skip |= ValidatePipelineVertexInputStateCreateInfo(*vertex_input_state, i);

                if (vertex_input_state->vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
                    skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                     "vkCreateGraphicsPipelines: pararameter "
                                     "pCreateInfo[%" PRIu32 "].pVertexInputState->vertexBindingDescriptionCount (%" PRIu32
                                     ") is "
                                     "greater than VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                     i, vertex_input_state->vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
                }

                if (vertex_input_state->vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
                    skip |=
                        LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                                 "vkCreateGraphicsPipelines: pararameter "
                                 "pCreateInfo[%" PRIu32 "].pVertexInputState->vertexAttributeDescriptionCount (%" PRIu32
                                 ") is "
                                 "greater than VkPhysicalDeviceLimits::maxVertexInputAttributes (%" PRIu32 ").",
                                 i, vertex_input_state->vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
                }

                vvl::unordered_set<uint32_t> vertex_bindings(vertex_input_state->vertexBindingDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexBindingDescriptionCount; ++d) {
                    auto const &vertex_bind_desc = vertex_input_state->pVertexBindingDescriptions[d];
                    auto const &binding_it = vertex_bindings.find(vertex_bind_desc.binding);
                    if (binding_it != vertex_bindings.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexBindingDescription[%" PRIu32
                                         "].binding "
                                         "(%" PRIu32 ") is already in pVertexBindingDescription[%" PRIu32 "]",
                                         i, d, vertex_bind_desc.binding, *binding_it);
                    }
                    vertex_bindings.insert(vertex_bind_desc.binding);

                    if (vertex_bind_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputBindingDescription-binding-00618",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexBindingDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                         i, d, vertex_bind_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_bind_desc.stride > device_limits.maxVertexInputBindingStride) {
                        skip |= LogError(device, "VUID-VkVertexInputBindingDescription-stride-00619",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexBindingDescriptions[%" PRIu32
                                         "].stride (%" PRIu32
                                         ") is greater "
                                         "than VkPhysicalDeviceLimits::maxVertexInputBindingStride (%" PRIu32 ").",
                                         i, d, vertex_bind_desc.stride, device_limits.maxVertexInputBindingStride);
                    }
                }

                vvl::unordered_set<uint32_t> attribute_locations(vertex_input_state->vertexAttributeDescriptionCount);
                for (uint32_t d = 0; d < vertex_input_state->vertexAttributeDescriptionCount; ++d) {
                    auto const &vertex_attrib_desc = vertex_input_state->pVertexAttributeDescriptions[d];
                    auto const &location_it = attribute_locations.find(vertex_attrib_desc.location);
                    if (location_it != attribute_locations.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].location (%" PRIu32 ") is already in pVertexAttributeDescriptions[%" PRIu32 "].",
                                         i, d, vertex_attrib_desc.location, *location_it);
                    }
                    attribute_locations.insert(vertex_attrib_desc.location);

                    auto const &binding_it = vertex_bindings.find(vertex_attrib_desc.binding);
                    if (binding_it == vertex_bindings.cend()) {
                        skip |= LogError(device, "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                                         "vkCreateGraphicsPipelines: parameter "
                                         " pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") does not exist "
                                         "in any pCreateInfo[%" PRIu32 "].pVertexInputState->pVertexBindingDescription.",
                                         i, d, vertex_attrib_desc.binding, i);
                    }

                    if (vertex_attrib_desc.location >= device_limits.maxVertexInputAttributes) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-location-00620",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].location (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.location, device_limits.maxVertexInputAttributes);
                    }

                    if (vertex_attrib_desc.binding >= device_limits.maxVertexInputBindings) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-binding-00621",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].binding (%" PRIu32
                                         ") is "
                                         "greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.binding, device_limits.maxVertexInputBindings);
                    }

                    if (vertex_attrib_desc.offset > device_limits.maxVertexInputAttributeOffset) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-offset-00622",
                                         "vkCreateGraphicsPipelines: parameter "
                                         "pCreateInfos[%" PRIu32 "].pVertexInputState->pVertexAttributeDescriptions[%" PRIu32
                                         "].offset (%" PRIu32
                                         ") is "
                                         "greater than VkPhysicalDeviceLimits::maxVertexInputAttributeOffset (%" PRIu32 ").",
                                         i, d, vertex_attrib_desc.offset, device_limits.maxVertexInputAttributeOffset);
                    }

                    VkFormatProperties properties;
                    DispatchGetPhysicalDeviceFormatProperties(physical_device, vertex_attrib_desc.format, &properties);
                    if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-format-00623",
                                         "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                         "].pVertexInputState->vertexAttributeDescriptions[%d].format "
                                         "(%s) is not a supported vertex buffer format. (supported bufferFeatures: %s)",
                                         i, d, string_VkFormat(vertex_attrib_desc.format),
                                         string_VkFormatFeatureFlags2(properties.bufferFeatures).c_str());
                    }
                }
            }

            // pViewportState, pMultisampleState, pDepthStencilState, and pColorBlendState ignored when rasterization is disabled
            if ((create_info.pRasterizationState != nullptr) &&
                (create_info.pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
                // Everything in here has a pre-rasterization shader state
                if (create_info.pViewportState == nullptr) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00750",
                                     "vkCreateGraphicsPipelines: Rasterization is enabled (pCreateInfos[%" PRIu32
                                     "].pRasterizationState->rasterizerDiscardEnable is VK_FALSE), but pCreateInfos[%" PRIu32
                                     "].pViewportState (=NULL) is not a valid pointer.",
                                     i, i);
                } else {
                    const auto &viewport_state = *create_info.pViewportState;
                    skip |= ValidatePipelineViewportStateCreateInfo(*create_info.pViewportState, i);

                    auto exclusive_scissor_struct =
                        LvlFindInChain<VkPipelineViewportExclusiveScissorStateCreateInfoNV>(viewport_state.pNext);
                    auto shading_rate_image_struct =
                        LvlFindInChain<VkPipelineViewportShadingRateImageStateCreateInfoNV>(viewport_state.pNext);
                    auto coarse_sample_order_struct =
                        LvlFindInChain<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV>(viewport_state.pNext);
                    const auto vp_swizzle_struct = LvlFindInChain<VkPipelineViewportSwizzleStateCreateInfoNV>(viewport_state.pNext);
                    const auto vp_w_scaling_struct =
                        LvlFindInChain<VkPipelineViewportWScalingStateCreateInfoNV>(viewport_state.pNext);
                    const auto depth_clip_control_struct =
                        LvlFindInChain<VkPipelineViewportDepthClipControlCreateInfoEXT>(viewport_state.pNext);

                    if (!physical_device_features.multiViewport) {
                        if (exclusive_scissor_struct && (exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                                                         exclusive_scissor_struct->exclusiveScissorCount != 1)) {
                            skip |= LogError(
                                device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
                                "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                "disabled, but pCreateInfos[%" PRIu32
                                "] VkPipelineViewportExclusiveScissorStateCreateInfoNV::exclusiveScissorCount (=%" PRIu32
                                ") is not 1.",
                                i, exclusive_scissor_struct->exclusiveScissorCount);
                        }

                        if (shading_rate_image_struct &&
                            (shading_rate_image_struct->viewportCount != 0 && shading_rate_image_struct->viewportCount != 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32
                                             "] VkPipelineViewportShadingRateImageStateCreateInfoNV::viewportCount (=%" PRIu32
                                             ") is neither 0 nor 1.",
                                             i, shading_rate_image_struct->viewportCount);
                        }
                    }

                    // Viewport count
                    if (viewport_state.viewportCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, viewport_state.viewportCount, device_limits.maxViewports);
                    }
                    if (has_dynamic_viewport_with_count) {
                        if (viewport_state.viewportCount != 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->viewportCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT is used.",
                                             i, viewport_state.viewportCount);
                        }
                    } else {
                        if (viewport_state.viewportCount == 0) {
                            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state)
                                                   ? "VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135"
                                                   : "VUID-VkPipelineViewportStateCreateInfo-viewportCount-arraylength";
                            skip |= LogError(
                                device, vuid,
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "].pViewportState->viewportCount can't be 0 unless VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT is used.",
                                i);
                        }

                        if (!physical_device_features.multiViewport && (viewport_state.viewportCount > 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.viewportCount);
                        }
                    }

                    // Scissor count
                    if (viewport_state.scissorCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, viewport_state.scissorCount, device_limits.maxViewports);
                    }
                    if (has_dynamic_scissor_with_count) {
                        if (viewport_state.scissorCount != 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState->scissorCount (=%" PRIu32
                                             ") must be zero when VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT is used.",
                                             i, viewport_state.viewportCount);
                        }
                    } else {
                        if (viewport_state.scissorCount == 0) {
                            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state)
                                                   ? "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136"
                                                   : "VUID-VkPipelineViewportStateCreateInfo-scissorCount-arraylength";
                            skip |= LogError(
                                device, vuid,
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "].pViewportState->scissorCount can't be 0 unless VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT is used.",
                                i);
                        }

                        if (!physical_device_features.multiViewport && (viewport_state.scissorCount > 1)) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217",
                                             "vkCreateGraphicsPipelines: The VkPhysicalDeviceFeatures::multiViewport feature is "
                                             "disabled, but pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                             ") is not 1.",
                                             i, viewport_state.scissorCount);
                        }
                    }

                    if (!has_dynamic_scissor && viewport_state.pScissors) {
                        for (uint32_t scissor_i = 0; scissor_i < viewport_state.scissorCount; ++scissor_i) {
                            const auto &scissor = viewport_state.pScissors[scissor_i];

                            if (scissor.offset.x < 0) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-x-02821",
                                                 "vkCreateGraphicsPipelines: offset.x (=%" PRIi32 ") of pCreateInfos[%" PRIu32
                                                 "].pViewportState->pScissors[%" PRIu32 "] is negative.",
                                                 scissor.offset.x, i, scissor_i);
                            }

                            if (scissor.offset.y < 0) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-x-02821",
                                                 "vkCreateGraphicsPipelines: offset.y (=%" PRIi32 ") of pCreateInfos[%" PRIu32
                                                 "].pViewportState->pScissors[%" PRIu32 "] is negative.",
                                                 scissor.offset.y, i, scissor_i);
                            }

                            const int64_t x_sum =
                                static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
                            if (x_sum > std::numeric_limits<int32_t>::max()) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-offset-02822",
                                                 "vkCreateGraphicsPipelines: offset.x + extent.width (=%" PRIi32 " + %" PRIu32
                                                 " = %" PRIi64 ") of pCreateInfos[%" PRIu32 "].pViewportState->pScissors[%" PRIu32
                                                 "] will overflow int32_t.",
                                                 scissor.offset.x, scissor.extent.width, x_sum, i, scissor_i);
                            }

                            const int64_t y_sum =
                                static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
                            if (y_sum > std::numeric_limits<int32_t>::max()) {
                                skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-offset-02823",
                                                 "vkCreateGraphicsPipelines: offset.y + extent.height (=%" PRIi32 " + %" PRIu32
                                                 " = %" PRIi64 ") of pCreateInfos[%" PRIu32 "].pViewportState->pScissors[%" PRIu32
                                                 "] will overflow int32_t.",
                                                 scissor.offset.y, scissor.extent.height, y_sum, i, scissor_i);
                            }
                        }
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount > device_limits.maxViewports) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                     ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                     i, exclusive_scissor_struct->exclusiveScissorCount, device_limits.maxViewports);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->viewportCount > device_limits.maxViewports) {
                        skip |= LogError(device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02055",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "] VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                                         ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                                         i, shading_rate_image_struct->viewportCount, device_limits.maxViewports);
                    }

                    if (viewport_state.scissorCount != viewport_state.viewportCount) {
                        if (!IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state) ||
                            (!has_dynamic_viewport_with_count && !has_dynamic_scissor_with_count)) {
                            skip |= LogError(
                                device, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "].pViewportState->scissorCount (=%" PRIu32
                                ") is not identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                                i, viewport_state.scissorCount, i, viewport_state.viewportCount);
                        }
                    }

                    if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                        exclusive_scissor_struct->exclusiveScissorCount != viewport_state.viewportCount) {
                        skip |=
                            LogError(device, "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32 "] exclusiveScissorCount (=%" PRIu32
                                     ") must be zero or identical to pCreateInfos[%" PRIu32
                                     "].pViewportState->viewportCount (=%" PRIu32 ").",
                                     i, exclusive_scissor_struct->exclusiveScissorCount, i, viewport_state.viewportCount);
                    }

                    if (shading_rate_image_struct && shading_rate_image_struct->shadingRateImageEnable &&
                        shading_rate_image_struct->viewportCount != viewport_state.viewportCount) {
                        skip |= LogError(
                            device, "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056",
                            "vkCreateGraphicsPipelines: If shadingRateImageEnable is enabled, pCreateInfos[%" PRIu32
                            "] "
                            "VkPipelineViewportShadingRateImageStateCreateInfoNV viewportCount (=%" PRIu32
                            ") must identical to pCreateInfos[%" PRIu32 "].pViewportState->viewportCount (=%" PRIu32 ").",
                            i, shading_rate_image_struct->viewportCount, i, viewport_state.viewportCount);
                    }

                    if (!has_dynamic_viewport && !has_dynamic_viewport_with_count && viewport_state.viewportCount > 0 &&
                        viewport_state.pViewports == nullptr) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130",
                                         "vkCreateGraphicsPipelines: The viewport state is not dynamic but pCreateInfos[%" PRIu32
                                         "].pViewportState->pViewports is an invalid pointer.",
                                         i);
                    }

                    if (!has_dynamic_scissor && !has_dynamic_scissor_with_count && viewport_state.scissorCount > 0 &&
                        viewport_state.pScissors == nullptr) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131",
                                         "vkCreateGraphicsPipelines: The scissor state is is not dynamic, but pCreateInfos[%" PRIu32
                                         "].pViewportState->pScissors is an invalid pointer.",
                                         i);
                    }

                    if (!has_dynamic_exclusive_scissor_nv && exclusive_scissor_struct &&
                        exclusive_scissor_struct->exclusiveScissorCount > 0 &&
                        exclusive_scissor_struct->pExclusiveScissors == nullptr) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04056",
                                     "vkCreateGraphicsPipelines: The exclusive scissor state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV), but "
                                     "pCreateInfos[%" PRIu32 "] pExclusiveScissors (=NULL) is an invalid pointer.",
                                     i, i);
                    }

                    if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV) &&
                        shading_rate_image_struct && shading_rate_image_struct->viewportCount > 0 &&
                        shading_rate_image_struct->pShadingRatePalettes == nullptr) {
                        skip |= LogError(
                            device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057",
                            "vkCreateGraphicsPipelines: The shading rate palette state is static (pCreateInfos[%" PRIu32
                            "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV), "
                            "but pCreateInfos[%" PRIu32 "] pShadingRatePalettes (=NULL) is an invalid pointer.",
                            i, i);
                    }

                    if (vp_swizzle_struct) {
                        const auto swizzle_viewport_count = vp_swizzle_struct->viewportCount;
                        const auto viewport_count = viewport_state.viewportCount;
                        if (swizzle_viewport_count < viewport_count) {
                            skip |= LogError(device, "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215",
                                             "vkCreateGraphicsPipelines: SwizzleState viewportCount (%" PRIu32
                                             ") less than ViewportState viewportCount (%" PRIu32 ")!",
                                             swizzle_viewport_count, viewport_count);
                        }
                    }

                    // validate the VkViewports
                    if (!has_dynamic_viewport && viewport_state.pViewports) {
                        for (uint32_t viewport_i = 0; viewport_i < viewport_state.viewportCount; ++viewport_i) {
                            const auto &viewport = viewport_state.pViewports[viewport_i];  // will crash on invalid ptr
                            const char *fn_name = "vkCreateGraphicsPipelines";
                            skip |= manual_PreCallValidateViewport(viewport, fn_name,
                                                                   ParameterName("pCreateInfos[%i].pViewportState->pViewports[%i]",
                                                                                 ParameterName::IndexVector{i, viewport_i}),
                                                                   VkCommandBuffer(0));
                        }
                    }

                    if (has_dynamic_viewport_w_scaling_nv && !IsExtEnabled(device_extensions.vk_nv_clip_space_w_scaling)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, but "
                                         "VK_NV_clip_space_w_scaling extension is not enabled.",
                                         i);
                    }

                    if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT) &&
                        !IsExtEnabled(device_extensions.vk_ext_discard_rectangles)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, but "
                                         "VK_EXT_discard_rectangles extension is not enabled.",
                                         i);
                    }

                    if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
                        !IsExtEnabled(device_extensions.vk_ext_sample_locations)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but "
                                         "VK_EXT_sample_locations extension is not enabled.",
                                         i);
                    }

                    if (has_dynamic_exclusive_scissor_nv && !IsExtEnabled(device_extensions.vk_nv_scissor_exclusive)) {
                        skip |= LogError(device, kVUID_PVError_ExtensionNotEnabled,
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "].pDynamicState->pDynamicStates contains VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, but "
                                         "VK_NV_scissor_exclusive extension is not enabled.",
                                         i);
                    }

                    if (coarse_sample_order_struct &&
                        coarse_sample_order_struct->sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV &&
                        coarse_sample_order_struct->customSampleOrderCount != 0) {
                        skip |= LogError(device, "VUID-VkPipelineViewportCoarseSampleOrderStateCreateInfoNV-sampleOrderType-02072",
                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                         "] "
                                         "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV sampleOrderType is not "
                                         "VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV and customSampleOrderCount is not 0.",
                                         i);
                    }

                    if (coarse_sample_order_struct) {
                        for (uint32_t order_i = 0; order_i < coarse_sample_order_struct->customSampleOrderCount; ++order_i) {
                            skip |= ValidateCoarseSampleOrderCustomNV(&coarse_sample_order_struct->pCustomSampleOrders[order_i]);
                        }
                    }

                    if (vp_w_scaling_struct && (vp_w_scaling_struct->viewportWScalingEnable == VK_TRUE)) {
                        if (vp_w_scaling_struct->viewportCount != viewport_state.viewportCount) {
                            skip |= LogError(device, "VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "] "
                                             "VkPipelineViewportWScalingStateCreateInfoNV.viewportCount (=%" PRIu32
                                             ") "
                                             "is not equal to VkPipelineViewportStateCreateInfo.viewportCount (=%" PRIu32 ").",
                                             i, vp_w_scaling_struct->viewportCount, viewport_state.viewportCount);
                        }
                        if (!has_dynamic_viewport_w_scaling_nv && !vp_w_scaling_struct->pViewportWScalings) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "] "
                                "VkPipelineViewportWScalingStateCreateInfoNV.pViewportWScalings (=NULL) is not a valid array.",
                                i);
                        }
                    }

                    if (depth_clip_control_struct) {
                        const auto *depth_clip_control_features =
                            LvlFindInChain<VkPhysicalDeviceDepthClipControlFeaturesEXT>(device_createinfo_pnext);
                        const bool enabled_depth_clip_control =
                            depth_clip_control_features && depth_clip_control_features->depthClipControl;
                        if (depth_clip_control_struct->negativeOneToOne && !enabled_depth_clip_control) {
                            skip |= LogError(device, "VUID-VkPipelineViewportDepthClipControlCreateInfoEXT-negativeOneToOne-06470",
                                             "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                             "].pViewportState has negativeOneToOne set to VK_TRUE in the pNext chain, but the "
                                             "depthClipControl feature is not enabled. ",
                                             i);
                        }
                    }
                }

                // It is possible for pCreateInfos[i].pMultisampleState to be null when creating a graphics library
                if (create_info.pMultisampleState) {
                    skip |= ValidatePipelineMultisampleStateCreateInfo(*create_info.pMultisampleState, i);

                    if (create_info.pMultisampleState->sampleShadingEnable == VK_TRUE) {
                        if (!physical_device_features.sampleRateShading) {
                            skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784",
                                             "vkCreateGraphicsPipelines(): parameter "
                                             "pCreateInfos[%" PRIu32 "].pMultisampleState->sampleShadingEnable.",
                                             i);
                        }
                        // TODO Add documentation issue about when minSampleShading must be in range and when it is ignored
                        // For now a "least noise" test *only* when sampleShadingEnable is VK_TRUE.
                        if (!IsBetweenInclusive(create_info.pMultisampleState->minSampleShading, 0.F, 1.0F)) {
                            skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786",
                                             "vkCreateGraphicsPipelines(): parameter pCreateInfos[%" PRIu32
                                             "].pMultisampleState->minSampleShading.",
                                             i);
                        }
                    }
                }

                const auto *line_state =
                    LvlFindInChain<VkPipelineRasterizationLineStateCreateInfoEXT>(create_info.pRasterizationState->pNext);
                const bool dynamic_line_raster_mode =
                    vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
                const bool dynamic_line_stipple_enable = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
                if (line_state) {
                    if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                        if (line_state->lineStippleFactor < 1 || line_state->lineStippleFactor > 256) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] lineStippleFactor = %" PRIu32
                                             " must be in the range [1,256].",
                                             i, line_state->lineStippleFactor);
                        }
                    }

                    if (!dynamic_line_raster_mode) {
                        if ((line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT ||
                             line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT)) {
                            if (create_info.pMultisampleState && create_info.pMultisampleState->alphaToCoverageEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%" PRIu32 "].pMultisampleState->alphaToCoverageEnable == VK_TRUE.",
                                             i);
                            }
                            if (create_info.pMultisampleState && create_info.pMultisampleState->alphaToOneEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%" PRIu32 "].pMultisampleState->alphaToOneEnable == VK_TRUE.",
                                             i);
                            }
                            if (create_info.pMultisampleState && create_info.pMultisampleState->sampleShadingEnable) {
                                skip |=
                                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766",
                                             "vkCreateGraphicsPipelines(): Bresenham/Smooth line rasterization not supported with "
                                             "pCreateInfos[%" PRIu32 "].pMultisampleState->sampleShadingEnable == VK_TRUE.",
                                             i);
                            }
                        }

                        const auto *line_features =
                            LvlFindInChain<VkPhysicalDeviceLineRasterizationFeaturesEXT>(device_createinfo_pnext);
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                            (!line_features || !line_features->rectangularLines)) {
                            skip |= LogError(
                                device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02768",
                                "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                "] lineRasterizationMode = "
                                "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT but the rectangularLines feature is not enabled.",
                                i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                            (!line_features || !line_features->bresenhamLines)) {
                            skip |=
                                LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02769",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] lineRasterizationMode = "
                                         "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT but the bresenhamLines feature is not enabled.",
                                         i);
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                            (!line_features || !line_features->smoothLines)) {
                            skip |= LogError(
                                device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-lineRasterizationMode-02770",
                                "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                "] lineRasterizationMode = "
                                "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT but the smoothLines feature is not enabled.",
                                i);
                        }
                        if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                                (!line_features || !line_features->stippledRectangularLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02771",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT and stippledLineEnable is "
                                             "VK_TRUE, but the stippledRectangularLines feature is not enabled.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                                (!line_features || !line_features->stippledBresenhamLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02772",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT and stippledLineEnable is VK_TRUE, "
                                             "but the stippledBresenhamLines feature is not enabled.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                                (!line_features || !line_features->stippledSmoothLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02773",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT and stippledLineEnable is "
                                             "VK_TRUE, but the stippledSmoothLines feature is not enabled.",
                                             i);
                            }
                            if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT &&
                                (!line_features || !line_features->stippledRectangularLines || !device_limits.strictLines)) {
                                skip |=
                                    LogError(device, "VUID-VkPipelineRasterizationLineStateCreateInfoEXT-stippledLineEnable-02774",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] lineRasterizationMode = "
                                             "VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT and stippledLineEnable is VK_TRUE, "
                                             "but the stippledRectangularLines feature is not enabled or strictLines is VK_FALSE.",
                                             i);
                            }
                        }
                    }
                }

                bool uses_color_attachment = false;
                bool uses_depthstencil_attachment = false;
                VkSubpassDescriptionFlags subpass_flags = 0;
                {
                    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
                    const auto subpasses_uses_it = renderpasses_states.find(create_info.renderPass);
                    if (subpasses_uses_it != renderpasses_states.end()) {
                        const auto &subpasses_uses = subpasses_uses_it->second;
                        if (subpasses_uses.subpasses_using_color_attachment.count(create_info.subpass)) {
                            uses_color_attachment = true;
                        }
                        if (subpasses_uses.subpasses_using_depthstencil_attachment.count(create_info.subpass)) {
                            uses_depthstencil_attachment = true;
                        }
                        subpass_flags = subpasses_uses.subpasses_flags[create_info.subpass];

                        color_attachment_count = subpasses_uses.color_attachment_count;
                    }
                    lock.unlock();
                }

                if (create_info.pDepthStencilState != nullptr && uses_depthstencil_attachment) {
                    skip |= ValidatePipelineDepthStencilStateCreateInfo(*create_info.pDepthStencilState, i);

                    if ((create_info.pDepthStencilState->flags &
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_depth_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderDepthAttachmentAccess == VK_TRUE;
                        if (!rasterization_order_depth_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderDepthAttachmentAccess-06463",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationOrderDepthAttachmentAccess == VK_FALSE, but "
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-flags-06485",
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s but "
                                "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str(),
                                string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }

                    if ((create_info.pDepthStencilState->flags &
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_stencil_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderStencilAttachmentAccess == VK_TRUE;
                        if (!rasterization_order_stencil_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device,
                                "VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderStencilAttachmentAccess-06464",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationOrderStencilAttachmentAccess == VK_FALSE, but "
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-flags-06486",
                                "VkPipelineDepthStencilStateCreateInfo::flags == %s but "
                                "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                string_VkPipelineDepthStencilStateCreateFlags(create_info.pDepthStencilState->flags).c_str(),
                                string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }
                }

                if (create_info.pColorBlendState != nullptr && uses_color_attachment) {
                    auto const &color_blend_state = *create_info.pColorBlendState;
                    skip |= ValidatePipelineColorBlendStateCreateInfo(color_blend_state, i);

                    if ((color_blend_state.flags &
                         VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM) != 0) {
                        const auto *rasterization_order_attachment_access_feature =
                            LvlFindInChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM>(device_createinfo_pnext);
                        const bool rasterization_order_color_attachment_access_feature_enabled =
                            rasterization_order_attachment_access_feature &&
                            rasterization_order_attachment_access_feature->rasterizationOrderColorAttachmentAccess == VK_TRUE;

                        if (!rasterization_order_color_attachment_access_feature_enabled) {
                            skip |= LogError(
                                device, "VUID-VkPipelineColorBlendStateCreateInfo-rasterizationOrderColorAttachmentAccess-06465",
                                "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM::"
                                "rasterizationColorAttachmentAccess == VK_FALSE, but "
                                "VkPipelineColorBlendStateCreateInfo::flags == %s",
                                string_VkPipelineColorBlendStateCreateFlags(color_blend_state.flags).c_str());
                        }

                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_ARM) == 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06484",
                                             "VkPipelineColorBlendStateCreateInfo::flags == %s but "
                                             "VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                             string_VkPipelineColorBlendStateCreateFlags(color_blend_state.flags).c_str(),
                                             string_VkSubpassDescriptionFlags(subpass_flags).c_str());
                        }
                    }

                    if (color_blend_state.pAttachments != nullptr) {
                        const VkBlendOp first_color_blend_op = color_blend_state.pAttachments[0].colorBlendOp;
                        const VkBlendOp first_alpha_blend_op = color_blend_state.pAttachments[0].alphaBlendOp;
                        for (uint32_t attachment_index = 0; attachment_index < color_blend_state.attachmentCount;
                             ++attachment_index) {
                            const VkPipelineColorBlendAttachmentState attachment_state =
                                color_blend_state.pAttachments[attachment_index];

                            skip |= ValidatePipelineColorBlendAttachmentState(attachment_state, i, attachment_index);

                            // if blendEnabled is false, these values are ignored
                            if (attachment_state.blendEnable) {
                                bool advance_blend = false;
                                if (IsAdvanceBlendOperation(attachment_state.colorBlendOp)) {
                                    advance_blend = true;
                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendAllOperations == VK_FALSE) {
                                        // This VUID checks if a subset of advance blend ops are allowed
                                        switch (attachment_state.colorBlendOp) {
                                            case VK_BLEND_OP_ZERO_EXT:
                                            case VK_BLEND_OP_SRC_EXT:
                                            case VK_BLEND_OP_DST_EXT:
                                            case VK_BLEND_OP_SRC_OVER_EXT:
                                            case VK_BLEND_OP_DST_OVER_EXT:
                                            case VK_BLEND_OP_SRC_IN_EXT:
                                            case VK_BLEND_OP_DST_IN_EXT:
                                            case VK_BLEND_OP_SRC_OUT_EXT:
                                            case VK_BLEND_OP_DST_OUT_EXT:
                                            case VK_BLEND_OP_SRC_ATOP_EXT:
                                            case VK_BLEND_OP_DST_ATOP_EXT:
                                            case VK_BLEND_OP_XOR_EXT:
                                            case VK_BLEND_OP_INVERT_EXT:
                                            case VK_BLEND_OP_INVERT_RGB_EXT:
                                            case VK_BLEND_OP_LINEARDODGE_EXT:
                                            case VK_BLEND_OP_LINEARBURN_EXT:
                                            case VK_BLEND_OP_VIVIDLIGHT_EXT:
                                            case VK_BLEND_OP_LINEARLIGHT_EXT:
                                            case VK_BLEND_OP_PINLIGHT_EXT:
                                            case VK_BLEND_OP_HARDMIX_EXT:
                                            case VK_BLEND_OP_PLUS_EXT:
                                            case VK_BLEND_OP_PLUS_CLAMPED_EXT:
                                            case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
                                            case VK_BLEND_OP_PLUS_DARKER_EXT:
                                            case VK_BLEND_OP_MINUS_EXT:
                                            case VK_BLEND_OP_MINUS_CLAMPED_EXT:
                                            case VK_BLEND_OP_CONTRAST_EXT:
                                            case VK_BLEND_OP_INVERT_OVG_EXT:
                                            case VK_BLEND_OP_RED_EXT:
                                            case VK_BLEND_OP_GREEN_EXT:
                                            case VK_BLEND_OP_BLUE_EXT: {
                                                skip |= LogError(
                                                    device,
                                                    "VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409",
                                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                                    "].pColorBlendState->pAttachments[%" PRIu32
                                                    "].colorBlendOp (%s) is not valid when "
                                                    "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::"
                                                    "advancedBlendAllOperations is "
                                                    "VK_FALSE",
                                                    i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp));
                                                break;
                                            }
                                            default:
                                                break;
                                        }
                                    }

                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend ==
                                            VK_FALSE &&
                                        attachment_state.colorBlendOp != first_color_blend_op) {
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01407",
                                            "vkCreateGraphicsPipelines: advancedBlendIndependentBlend is set to VK_FALSE, but "
                                            "pCreateInfos[%" PRIu32 "].pColorBlendState->pAttachments[%" PRIu32
                                            "].colorBlendOp (%s) is not same the other attachments (%s).",
                                            i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                            string_VkBlendOp(first_color_blend_op));
                                    }
                                }

                                if (IsAdvanceBlendOperation(attachment_state.alphaBlendOp)) {
                                    advance_blend = true;
                                    if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend ==
                                            VK_FALSE &&
                                        attachment_state.alphaBlendOp != first_alpha_blend_op) {
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01408",
                                            "vkCreateGraphicsPipelines: advancedBlendIndependentBlend is set to VK_FALSE, but "
                                            "pCreateInfos[%" PRIu32 "].pColorBlendState->pAttachments[%" PRIu32
                                            "].alphaBlendOp (%s) is not same the other attachments (%s).",
                                            i, attachment_index, string_VkBlendOp(attachment_state.alphaBlendOp),
                                            string_VkBlendOp(first_alpha_blend_op));
                                    }
                                }

                                if (advance_blend) {
                                    if (attachment_state.colorBlendOp != attachment_state.alphaBlendOp) {
                                        skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01406",
                                                         "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                                         "].pColorBlendState->pAttachments[%" PRIu32
                                                         "] has different colorBlendOp (%s) and alphaBlendOp (%s) but one of "
                                                         "them is an advance blend operation.",
                                                         i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                                         string_VkBlendOp(attachment_state.alphaBlendOp));
                                    } else if (color_attachment_count >
                                               phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments) {
                                        // color_attachment_count is found one of multiple spots above
                                        //
                                        // error can guarantee it is the same VkBlendOp
                                        skip |= LogError(
                                            device, "VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410",
                                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                            "].pColorBlendState->pAttachments[%" PRIu32
                                            "] has an advance blend operation (%s) but the colorAttachmentCount (%" PRIu32
                                            ") for the subpass is greater than advancedBlendMaxColorAttachments (%" PRIu32 ").",
                                            i, attachment_index, string_VkBlendOp(attachment_state.colorBlendOp),
                                            color_attachment_count,
                                            phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments);
                                        break;  // if this fails once, will fail every iteration
                                    }
                                }
                            }
                        }
                    }

                    // If logicOpEnable is VK_TRUE, logicOp must be a valid VkLogicOp value
                    if (color_blend_state.logicOpEnable == VK_TRUE) {
                        skip |= ValidateRangedEnum(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pColorBlendState->logicOp", ParameterName::IndexVector{i}), "VkLogicOp",
                            color_blend_state.logicOp, "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00607");
                    }

                    const bool dynamic_not_set = (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) ||
                                                  !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) ||
                                                  !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) ||
                                                  !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT));

                    // If any of the dynamic states are not set still need a valid array
                    if ((color_blend_state.attachmentCount > 0) && dynamic_not_set) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353"
                                               : "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07354";

                        skip |= ValidateArray(
                            "vkCreateGraphicsPipelines",
                            ParameterName("pCreateInfos[%i].pColorBlendState->attachmentCount", ParameterName::IndexVector{i}),
                            ParameterName("pCreateInfos[%i].pColorBlendState->pAttachments", ParameterName::IndexVector{i}),
                            color_blend_state.attachmentCount, &color_blend_state.pAttachments, false, true, kVUIDUndefined, vuid);
                    }

                    auto color_write = LvlFindInChain<VkPipelineColorWriteCreateInfoEXT>(color_blend_state.pNext);
                    if (color_write && (color_write->attachmentCount != color_blend_state.attachmentCount) && dynamic_not_set) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-07608"
                                               : "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-04802";
                        skip |= LogError(device, vuid,
                                         "vkCreateGraphicsPipelines(): VkPipelineColorWriteCreateInfoEXT in the pNext chain of "
                                         "pCreateInfo[%" PRIu32 "].pColorBlendState has different attachmentCount (%" PRIu32
                                         ") than pColorBlendState.attachmentCount (%" PRIu32 ").",
                                         i, color_write->attachmentCount, color_blend_state.attachmentCount);
                    }
                }
            }

            if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
                if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                    if (create_info.basePipelineIndex != -1) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-07986",
                                         "vkCreateGraphicsPipelines pCreateInfos[%" PRIu32 "]->basePipelineIndex is %" PRIu32
                                         " and basePipelineHandle is not VK_NULL_HANDLE.",
                                         i, pCreateInfos[i].basePipelineIndex);
                    }
                } else {
                    if (static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-07985",
                                     "vkCreateGraphicsPipelines parameter pCreateInfos[%" PRIu32 "]->basePipelineIndex (%" PRId32
                                     ") must be a valid"
                                     "index into the pCreateInfos array, of size %" PRIu32 ".",
                                     i, create_info.basePipelineIndex, createInfoCount);
                    }
                }
            }

            if (create_info.pRasterizationState) {
                if (!IsExtEnabled(device_extensions.vk_nv_fill_rectangle)) {
                    if (create_info.pRasterizationState->polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_FILL_RECTANGLE_NV "
                                     "if the extension VK_NV_fill_rectangle is not enabled.");
                    } else if ((create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                               (physical_device_features.fillModeNonSolid == false)) {
                        skip |= LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01413",
                                         "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                         "pCreateInfos[%" PRIu32
                                         "]->pRasterizationState->polygonMode cannot be VK_POLYGON_MODE_POINT or "
                                         "VK_POLYGON_MODE_LINE if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                         i);
                    }
                } else {
                    if ((create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                        (create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL_RECTANGLE_NV) &&
                        (physical_device_features.fillModeNonSolid == false)) {
                        skip |=
                            LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507",
                                     "vkCreateGraphicsPipelines parameter, VkPolygonMode "
                                     "pCreateInfos[%" PRIu32
                                     "]->pRasterizationState->polygonMode must be VK_POLYGON_MODE_FILL or "
                                     "VK_POLYGON_MODE_FILL_RECTANGLE_NV if VkPhysicalDeviceFeatures->fillModeNonSolid is false.",
                                     i);
                    }
                }

                if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_WIDTH) && !physical_device_features.wideLines &&
                    (create_info.pRasterizationState->lineWidth != 1.0f)) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749",
                                     "The line width state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                     "VkPhysicalDeviceFeatures::wideLines is disabled, but pCreateInfos[%" PRIu32
                                     "].pRasterizationState->lineWidth (=%f) is not 1.0.",
                                     i, i, create_info.pRasterizationState->lineWidth);
                }
            }

            // Validate no flags not allowed are used
            if ((flags & VK_PIPELINE_CREATE_DISPATCH_BASE) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-00764",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_DISPATCH_BASE.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03372",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03373",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03374",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03375",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03376",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03377",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-03577",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                                 i, flags);
            }
            if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-04947",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]->flags (0x%x) must not include "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                                 i, flags);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                       uint32_t createInfoCount,
                                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkPipeline *pPipelines) const {
    bool skip = false;
    for (uint32_t i = 0; i < createInfoCount; i++) {
        skip |=
            ValidateString("vkCreateComputePipelines", ParameterName("pCreateInfos[%i].stage.pName", ParameterName::IndexVector{i}),
                           "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", pCreateInfos[i].stage.pName);
        auto feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext);
        if (feedback_struct) {
            const uint32_t feedback_count = feedback_struct->pipelineStageCreationFeedbackCount;
            if ((feedback_count != 0) && (feedback_count != 1)) {
                skip |= LogError(
                    device, "VUID-VkComputePipelineCreateInfo-pipelineStageCreationFeedbackCount-06566",
                    "vkCreateComputePipelines(): VkPipelineCreationFeedbackCreateInfo::pipelineStageCreationFeedbackCount (%" PRIu32
                    ") is not 0 or 1 in pCreateInfos[%" PRIu32 "].",
                    feedback_count, i);
            }
        }

        // Make sure compute stage is selected
        if (pCreateInfos[i].stage.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-stage-00701",
                             "vkCreateComputePipelines(): the pCreateInfo[%" PRIu32
                             "].stage.stage (%s) is not VK_SHADER_STAGE_COMPUTE_BIT",
                             i, string_VkShaderStageFlagBits(pCreateInfos[i].stage.stage));
        }

        const VkPipelineCreateFlags flags = pCreateInfos[i].flags;
        // Validate no flags not allowed are used
        if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03364",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03365",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03366",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03367",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03368",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03369",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03370",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-03576",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-04945",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                             i, flags);
        }
        if ((flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) != 0) {
            skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-02874",
                             "vkCreateComputePipelines(): pCreateInfos[%" PRIu32
                             "]->flags (0x%x) must not include "
                             "VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV.",
                             i, flags);
        }
        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (pCreateInfos[i].basePipelineHandle != VK_NULL_HANDLE) {
                if (pCreateInfos[i].basePipelineIndex != -1) {
                    skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-07986",
                                     "vkCreateComputePipelines pCreateInfos[%" PRIu32 "]->basePipelineIndex is %" PRIu32
                                     " and basePipelineHandle is not VK_NULL_HANDLE.",
                                     i, pCreateInfos[i].basePipelineIndex);
                }
            } else {
                if (static_cast<uint32_t>(pCreateInfos[i].basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(device, "VUID-VkComputePipelineCreateInfo-flags-07985",
                                     "vkCreateComputePipelines parameter pCreateInfos[%" PRIu32 "]->basePipelineIndex (%" PRIi32
                                     ") must be a valid index into the pCreateInfos array, of size %" PRIu32 ".",
                                     i, pCreateInfos[i].basePipelineIndex, createInfoCount);
                }
            }
        }

        std::stringstream msg;
        msg << "pCreateInfos[%" << i << "].stage";
        ValidatePipelineShaderStageCreateInfo("vkCreateComputePipelines", msg.str().c_str(), &pCreateInfos[i].stage);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                                    uint32_t srcCacheCount,
                                                                    const VkPipelineCache *pSrcCaches) const {
    bool skip = false;
    if (pSrcCaches) {
        for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
            if (pSrcCaches[index0] == dstCache) {
                skip |= LogError(instance, "VUID-vkMergePipelineCaches-dstCache-00770",
                                 "vkMergePipelineCaches(): dstCache %s is in pSrcCaches list.",
                                 report_data->FormatHandle(dstCache).c_str());
                break;
            }
        }
    }
    return skip;
}