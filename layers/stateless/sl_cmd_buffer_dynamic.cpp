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

bool StatelessValidation::ValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                          const VkViewport *pViewports, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    if (!physical_device_features.multiViewport) {
        if (viewportCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCount-viewportCount-03395",
                             "%s: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.", api_call,
                             viewportCount);
        }
    } else {  // multiViewport enabled
        if (viewportCount < 1 || viewportCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWithCount-viewportCount-03394",
                             "%s:  viewportCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             api_call, viewportCount, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            skip |= manual_PreCallValidateViewport(
                viewport, api_call, ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}), commandBuffer);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                           const VkViewport *pViewports) const {
    bool skip = false;
    skip = ValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNTEXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                        const VkViewport *pViewports) const {
    bool skip = false;
    skip = ValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNT);
    return skip;
}

bool StatelessValidation::ValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                         const VkRect2D *pScissors, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    if (!physical_device_features.multiViewport) {
        if (scissorCount != 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03398",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "be 1 when the multiViewport feature is disabled.",
                             api_call, scissorCount);
        }
    } else {  // multiViewport enabled
        if (scissorCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03397",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "be great than zero.",
                             api_call, scissorCount);
        } else if (scissorCount > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-scissorCount-03397",
                             "%s: scissorCount (=%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             api_call, scissorCount, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-x-03399",
                                 "%s: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", api_call, scissor_i,
                                 scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-x-03399",
                                 "%s: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", api_call, scissor_i,
                                 scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-offset-03400",
                                 "%s: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64 ") of pScissors[%" PRIu32
                                 "] will overflow int32_t.",
                                 api_call, scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissorWithCount-offset-03401",
                                 "%s: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64 ") of pScissors[%" PRIu32
                                 "] will overflow int32_t.",
                                 api_call, scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                          const VkRect2D *pScissors) const {
    bool skip = false;
    skip = ValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNTEXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                       const VkRect2D *pScissors) const {
    bool skip = false;
    skip = ValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) const {
    bool skip = false;
    const auto *vertex_attribute_divisor_features =
        LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(device_createinfo_pnext);

    if (vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
        skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791",
                         "vkCmdSetVertexInputEXT(): vertexBindingDescriptionCount (%" PRIu32
                         ") is greater than the maxVertexInputBindings limit (%" PRIu32 ").",
                         vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
    }

    if (vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
        skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792",
                         "vkCmdSetVertexInputEXT(): vertexAttributeDescriptionCount (%" PRIu32
                         ") is greater than the maxVertexInputAttributes limit (%" PRIu32 ").",
                         vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        bool binding_found = false;
        for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
            if (pVertexAttributeDescriptions[attribute].binding == pVertexBindingDescriptions[binding].binding) {
                binding_found = true;
                break;
            }
        }
        if (!binding_found) {
            skip |= LogError(
                device, "VUID-vkCmdSetVertexInputEXT-binding-04793",
                "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32 "] references an unspecified binding", attribute);
        }
    }

    // check for distinct values
    {
        vvl::unordered_set<uint32_t> vertex_bindings(vertexBindingDescriptionCount);
        for (uint32_t i = 0; i < vertexBindingDescriptionCount; ++i) {
            const uint32_t binding = pVertexBindingDescriptions[i].binding;
            auto const &binding_it = vertex_bindings.find(binding);
            if (binding_it != vertex_bindings.cend()) {
                skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794",
                                 "vkCmdSetVertexInputEXT(): binding description for pVertexBindingDescriptions[%" PRIu32
                                 "] is already in pVertexBindingDescriptions[%" PRIu32 "]",
                                 binding, *binding_it);
                break;
            }
            vertex_bindings.insert(binding);
        }

        vvl::unordered_set<uint32_t> vertex_locations(vertexAttributeDescriptionCount);
        for (uint32_t i = 0; i < vertexAttributeDescriptionCount; ++i) {
            const uint32_t location = pVertexAttributeDescriptions[i].location;
            auto const &location_it = vertex_locations.find(location);
            if (location_it != vertex_locations.cend()) {
                skip |= LogError(device, "VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795",
                                 "vkCmdSetVertexInputEXT(): attribute location for pVertexAttributeDescriptions[%" PRIu32
                                 "] is already in pVertexAttributeDescriptions[%" PRIu32 "]",
                                 location, *location_it);
                break;
            }
            vertex_locations.insert(location);
        }
    }

    for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
        if (pVertexBindingDescriptions[binding].binding > device_limits.maxVertexInputBindings) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-binding-04796",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].binding is greater than maxVertexInputBindings",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].stride > device_limits.maxVertexInputBindingStride) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-stride-04797",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].stride is greater than maxVertexInputBindingStride",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].divisor == 0 &&
            (!vertex_attribute_divisor_features || !vertex_attribute_divisor_features->vertexAttributeInstanceRateZeroDivisor)) {
            skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-04798",
                             "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                             "].divisor is zero but "
                             "vertexAttributeInstanceRateZeroDivisor is not enabled",
                             binding);
        }

        if (pVertexBindingDescriptions[binding].divisor > 1) {
            if (!vertex_attribute_divisor_features || !vertex_attribute_divisor_features->vertexAttributeInstanceRateDivisor) {
                skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-04799",
                                 "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                 "].divisor is greater than one but "
                                 "vertexAttributeInstanceRateDivisor is not enabled",
                                 binding);
            } else {
                if (pVertexBindingDescriptions[binding].divisor >
                    phys_dev_ext_props.vertex_attribute_divisor_props.maxVertexAttribDivisor) {
                    skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-06226",
                                     "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                     "].divisor is greater than maxVertexAttribDivisor",
                                     binding);
                }

                if (pVertexBindingDescriptions[binding].inputRate != VK_VERTEX_INPUT_RATE_INSTANCE) {
                    skip |= LogError(device, "VUID-VkVertexInputBindingDescription2EXT-divisor-06227",
                                     "vkCmdSetVertexInputEXT(): pVertexBindingDescriptions[%" PRIu32
                                     "].divisor is greater than 1 but inputRate "
                                     "is not VK_VERTEX_INPUT_RATE_INSTANCE",
                                     binding);
                }
            }
        }
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        if (pVertexAttributeDescriptions[attribute].location > device_limits.maxVertexInputAttributes) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-location-06228",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].location is greater than maxVertexInputAttributes",
                             attribute);
        }

        if (pVertexAttributeDescriptions[attribute].binding > device_limits.maxVertexInputBindings) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-binding-06229",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].binding is greater than maxVertexInputBindings",
                             attribute);
        }

        if (pVertexAttributeDescriptions[attribute].offset > device_limits.maxVertexInputAttributeOffset) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-offset-06230",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].offset is greater than maxVertexInputAttributeOffset",
                             attribute);
        }

        VkFormatProperties properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pVertexAttributeDescriptions[attribute].format, &properties);
        if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
            skip |= LogError(device, "VUID-VkVertexInputAttributeDescription2EXT-format-04805",
                             "vkCmdSetVertexInputEXT(): pVertexAttributeDescriptions[%" PRIu32
                             "].format is not a "
                             "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT supported format. (supported bufferFeatures: %s)",
                             attribute, string_VkFormatFeatureFlags2(properties.bufferFeatures).c_str());
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                                                          uint32_t firstDiscardRectangle,
                                                                          uint32_t discardRectangleCount,
                                                                          const VkRect2D *pDiscardRectangles) const {
    bool skip = false;

    if (pDiscardRectangles) {
        for (uint32_t i = 0; i < discardRectangleCount; ++i) {
            const int64_t x_sum =
                static_cast<int64_t>(pDiscardRectangles[i].offset.x) + static_cast<int64_t>(pDiscardRectangles[i].extent.width);
            if (x_sum > std::numeric_limits<int32_t>::max()) {
                skip |= LogError(device, "VUID-vkCmdSetDiscardRectangleEXT-offset-00588",
                                 "vkCmdSetDiscardRectangleEXT(): offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pDiscardRectangles[%" PRIu32 "] will overflow int32_t.",
                                 pDiscardRectangles[i].offset.x, pDiscardRectangles[i].extent.width, x_sum, i);
            }

            const int64_t y_sum =
                static_cast<int64_t>(pDiscardRectangles[i].offset.y) + static_cast<int64_t>(pDiscardRectangles[i].extent.height);
            if (y_sum > std::numeric_limits<int32_t>::max()) {
                skip |= LogError(device, "VUID-vkCmdSetDiscardRectangleEXT-offset-00589",
                                 "vkCmdSetDiscardRectangleEXT(): offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pDiscardRectangles[%" PRIu32 "] will overflow int32_t.",
                                 pDiscardRectangles[i].offset.y, pDiscardRectangles[i].extent.height, y_sum, i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
                                                                                VkBool32 discardRectangleEnable) const {
    bool skip = false;
    if (discard_rectangles_extension_version < 2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetDiscardRectangleEnableEXT-specVersion-07851",
                         "vkCmdSetDiscardRectangleEnableEXT: Requires support for version 2 of VK_EXT_discard_rectangles.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetDiscardRectangleModeEXT(
    VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode) const {
    bool skip = false;
    if (discard_rectangles_extension_version < 2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetDiscardRectangleModeEXT-specVersion-07852",
                         "vkCmdSetDiscardRectangleModeEXT: Requires support for version 2 of VK_EXT_discard_rectangles.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer,
                                                                               uint32_t firstExclusiveScissor,
                                                                               uint32_t exclusiveScissorCount,
                                                                               const VkBool32 *pExclusiveScissorEnables) const {
    bool skip = false;
    if (scissor_exclusive_extension_version < 2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorEnableNV-exclusiveScissor-07853",
                         "vkCmdSetExclusiveScissorEnableNV: Requires support for version 2 of VK_NV_scissor_exclusive.");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer,
                                                                         uint32_t firstExclusiveScissor,
                                                                         uint32_t exclusiveScissorCount,
                                                                         const VkRect2D *pExclusiveScissors) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstExclusiveScissor != 0) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035",
                         "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but firstExclusiveScissor (=%" PRIu32
                         ") is not 0.",
                         firstExclusiveScissor);
        }
        if (exclusiveScissorCount > 1) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036",
                         "vkCmdSetExclusiveScissorNV: The multiViewport feature is disabled, but exclusiveScissorCount (=%" PRIu32
                         ") is not 1.",
                         exclusiveScissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstExclusiveScissor) + static_cast<uint64_t>(exclusiveScissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02034",
                             "vkCmdSetExclusiveScissorNV: firstExclusiveScissor + exclusiveScissorCount (=%" PRIu32 " + %" PRIu32
                             " = %" PRIu64 ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstExclusiveScissor, exclusiveScissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pExclusiveScissors) {
        for (uint32_t scissor_i = 0; scissor_i < exclusiveScissorCount; ++scissor_i) {
            const auto &scissor = pExclusiveScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                 "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.",
                                 scissor_i, scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-x-02037",
                                 "vkCmdSetExclusiveScissorNV: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.",
                                 scissor_i, scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-offset-02038",
                                 "vkCmdSetExclusiveScissorNV: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetExclusiveScissorNV-offset-02039",
                                 "vkCmdSetExclusiveScissorNV: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                         uint32_t viewportCount,
                                                                         const VkViewportWScalingNV *pViewportWScalings) const {
    bool skip = false;
    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if ((sum < 1) || (sum > device_limits.maxViewports)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportWScalingNV-firstViewport-01324",
                         "vkCmdSetViewportWScalingNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                         ") must be between 1 and VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 "), inculsive.",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
    const VkShadingRatePaletteNV *pShadingRatePalettes) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068",
                         "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but firstViewport (=%" PRIu32
                         ") is not 0.",
                         firstViewport);
        }
        if (viewportCount > 1) {
            skip |=
                LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069",
                         "vkCmdSetViewportShadingRatePaletteNV: The multiViewport feature is disabled, but viewportCount (=%" PRIu32
                         ") is not 1.",
                         viewportCount);
        }
    }

    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if (sum > device_limits.maxViewports) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067",
                         "vkCmdSetViewportShadingRatePaletteNV: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32
                         " = %" PRIu64 ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetCoarseSampleOrderNV(
    VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) const {
    bool skip = false;

    if (sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV && customSampleOrderCount != 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081",
                         "vkCmdSetCoarseSampleOrderNV: If sampleOrderType is not VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, "
                         "customSampleOrderCount must be 0.");
    }

    for (uint32_t order_i = 0; order_i < customSampleOrderCount; ++order_i) {
        skip |= ValidateCoarseSampleOrderCustomNV(&pCustomSampleOrders[order_i]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                               uint32_t viewportCount, const VkViewport *pViewports) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstViewport != 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-firstViewport-01224",
                             "vkCmdSetViewport: The multiViewport feature is disabled, but firstViewport (=%" PRIu32 ") is not 0.",
                             firstViewport);
        }
        if (viewportCount > 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-viewportCount-01225",
                             "vkCmdSetViewport: The multiViewport feature is disabled, but viewportCount (=%" PRIu32 ") is not 1.",
                             viewportCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetViewport-firstViewport-01223",
                             "vkCmdSetViewport: firstViewport + viewportCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstViewport, viewportCount, sum, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            const char *fn_name = "vkCmdSetViewport";
            skip |= manual_PreCallValidateViewport(
                viewport, fn_name, ParameterName("pViewports[%i]", ParameterName::IndexVector{viewport_i}), commandBuffer);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                              uint32_t scissorCount, const VkRect2D *pScissors) const {
    bool skip = false;

    if (!physical_device_features.multiViewport) {
        if (firstScissor != 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-firstScissor-00593",
                             "vkCmdSetScissor: The multiViewport feature is disabled, but firstScissor (=%" PRIu32 ") is not 0.",
                             firstScissor);
        }
        if (scissorCount > 1) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-scissorCount-00594",
                             "vkCmdSetScissor: The multiViewport feature is disabled, but scissorCount (=%" PRIu32 ") is not 1.",
                             scissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstScissor) + static_cast<uint64_t>(scissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-firstScissor-00592",
                             "vkCmdSetScissor: firstScissor + scissorCount (=%" PRIu32 " + %" PRIu32 " = %" PRIu64
                             ") is greater than VkPhysicalDeviceLimits::maxViewports (=%" PRIu32 ").",
                             firstScissor, scissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-x-00595",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.x (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-x-00595",
                                 "vkCmdSetScissor: pScissors[%" PRIu32 "].offset.y (=%" PRIi32 ") is negative.", scissor_i,
                                 scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-offset-00596",
                                 "vkCmdSetScissor: offset.x + extent.width (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum, scissor_i);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError(commandBuffer, "VUID-vkCmdSetScissor-offset-00597",
                                 "vkCmdSetScissor: offset.y + extent.height (=%" PRIi32 " + %" PRIu32 " = %" PRIi64
                                 ") of pScissors[%" PRIu32 "] will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum, scissor_i);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    bool skip = false;

    if (!physical_device_features.wideLines && (lineWidth != 1.0f)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetLineWidth-lineWidth-00788",
                         "VkPhysicalDeviceFeatures::wideLines is disabled, but lineWidth (=%f) is not 1.0.", lineWidth);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                     uint16_t lineStipplePattern) const {
    bool skip = false;

    if (lineStippleFactor < 1 || lineStippleFactor > 256) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetLineStippleEXT-lineStippleFactor-02776",
                         "vkCmdSetLineStippleEXT::lineStippleFactor=%" PRIu32 " is not in [1,256].", lineStippleFactor);
    }

    return skip;
}
