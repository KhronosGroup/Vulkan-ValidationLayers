/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 */

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include <algorithm>
#include <array>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <valarray>

#include "vk_loader_platform.h"
#include "vk_dispatch_table_helper.h"
#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "convert_to_renderpass2.h"
#include "core_validation.h"
#include "buffer_validation.h"
#include "shader_validation.h"
#include "vk_layer_utils.h"

// Array of command names indexed by CMD_TYPE enum
static const std::array<const char *, CMD_RANGE_SIZE> command_name_list = {{VUID_CMD_NAME_LIST}};

// These functions are defined *outside* the core_validation namespace as their type
// is also defined outside that namespace
size_t PipelineLayoutCompatDef::hash() const {
    hash_util::HashCombiner hc;
    // The set number is integral to the CompatDef's distinctiveness
    hc << set << push_constant_ranges.get();
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    for (uint32_t i = 0; i <= set; i++) {
        hc << descriptor_set_layouts[i].get();
    }
    return hc.Value();
}

bool PipelineLayoutCompatDef::operator==(const PipelineLayoutCompatDef &other) const {
    if ((set != other.set) || (push_constant_ranges != other.push_constant_ranges)) {
        return false;
    }

    if (set_layouts_id == other.set_layouts_id) {
        // if it's the same set_layouts_id, then *any* subset will match
        return true;
    }

    // They aren't exactly the same PipelineLayoutSetLayouts, so we need to check if the required subsets match
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    assert(set < descriptor_set_layouts.size());
    const auto &other_ds_layouts = *other.set_layouts_id.get();
    assert(set < other_ds_layouts.size());
    for (uint32_t i = 0; i <= set; i++) {
        if (descriptor_set_layouts[i] != other_ds_layouts[i]) {
            return false;
        }
    }
    return true;
}

using std::max;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

// Get the global maps of pending releases
const GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkImageMemoryBarrier>::Tag &type_tag) const {
    return qfo_release_image_barrier_map;
}
const GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag &type_tag) const {
    return qfo_release_buffer_barrier_map;
}
GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkImageMemoryBarrier>::Tag &type_tag) {
    return qfo_release_image_barrier_map;
}
GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag &type_tag) {
    return qfo_release_buffer_barrier_map;
}

ImageSubresourceLayoutMap::InitialLayoutState::InitialLayoutState(const CMD_BUFFER_STATE &cb_state,
                                                                  const IMAGE_VIEW_STATE *view_state)
    : image_view(VK_NULL_HANDLE), aspect_mask(0), label(cb_state.debug_label) {
    if (view_state) {
        image_view = view_state->image_view;
        aspect_mask = view_state->create_info.subresourceRange.aspectMask;
    }
}

std::string FormatDebugLabel(const char *prefix, const LoggingLabel &label) {
    if (label.Empty()) return std::string();
    std::string out;
    string_sprintf(&out, "%sVkDebugUtilsLabel(name='%s' color=[%g, %g %g, %g])", prefix, label.name.c_str(), label.color[0],
                   label.color[1], label.color[2], label.color[3]);
    return out;
}

// the ImageLayoutMap implementation bakes in the number of valid aspects -- we have to choose the correct one at construction time
template <uint32_t kThreshold>
static std::unique_ptr<ImageSubresourceLayoutMap> LayoutMapFactoryByAspect(const IMAGE_STATE &image_state) {
    ImageSubresourceLayoutMap *map = nullptr;
    switch (image_state.full_range.aspectMask) {
        case VK_IMAGE_ASPECT_COLOR_BIT:
            map = new ImageSubresourceLayoutMapImpl<ColorAspectTraits, kThreshold>(image_state);
            break;
        case VK_IMAGE_ASPECT_DEPTH_BIT:
            map = new ImageSubresourceLayoutMapImpl<DepthAspectTraits, kThreshold>(image_state);
            break;
        case VK_IMAGE_ASPECT_STENCIL_BIT:
            map = new ImageSubresourceLayoutMapImpl<StencilAspectTraits, kThreshold>(image_state);
            break;
        case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
            map = new ImageSubresourceLayoutMapImpl<DepthStencilAspectTraits, kThreshold>(image_state);
            break;
        case VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT:
            map = new ImageSubresourceLayoutMapImpl<Multiplane2AspectTraits, kThreshold>(image_state);
            break;
        case VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT:
            map = new ImageSubresourceLayoutMapImpl<Multiplane3AspectTraits, kThreshold>(image_state);
            break;
    }

    assert(map);  // We shouldn't be able to get here null unless the traits cases are incomplete
    return std::unique_ptr<ImageSubresourceLayoutMap>(map);
}

static std::unique_ptr<ImageSubresourceLayoutMap> LayoutMapFactory(const IMAGE_STATE &image_state) {
    std::unique_ptr<ImageSubresourceLayoutMap> map;
    const uint32_t kAlwaysDenseLimit = 16;  // About a cacheline on deskop architectures
    if (image_state.full_range.layerCount <= kAlwaysDenseLimit) {
        // Create a dense row map
        map = LayoutMapFactoryByAspect<0>(image_state);
    } else {
        // Create an initially sparse row map
        map = LayoutMapFactoryByAspect<kAlwaysDenseLimit>(image_state);
    }
    return map;
}

// The const variant only need the image as it is the key for the map
const ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(const CMD_BUFFER_STATE *cb_state, VkImage image) {
    auto it = cb_state->image_layout_map.find(image);
    if (it == cb_state->image_layout_map.cend()) {
        return nullptr;
    }
    return it->second.get();
}

// The non-const variant only needs the image state, as the factory requires it to construct a new entry
ImageSubresourceLayoutMap *GetImageSubresourceLayoutMap(CMD_BUFFER_STATE *cb_state, const IMAGE_STATE &image_state) {
    auto it = cb_state->image_layout_map.find(image_state.image);
    if (it == cb_state->image_layout_map.end()) {
        // Empty slot... fill it in.
        auto insert_pair = cb_state->image_layout_map.insert(std::make_pair(image_state.image, LayoutMapFactory(image_state)));
        assert(insert_pair.second);
        ImageSubresourceLayoutMap *new_map = insert_pair.first->second.get();
        assert(new_map);
        return new_map;
    }
    return it->second.get();
}

// For given mem object, verify that it is not null or UNBOUND, if it is, report error. Return skip value.
bool CoreChecks::VerifyBoundMemoryIsValid(VkDeviceMemory mem, const VulkanTypedHandle &typed_handle, const char *api_name,
                                          const char *error_code) const {
    bool result = false;
    auto type_name = object_string[typed_handle.type];
    if (VK_NULL_HANDLE == mem) {
        result = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, typed_handle.handle,
                         error_code, "%s: %s used with no memory bound. Memory should be bound by calling vkBind%sMemory().",
                         api_name, report_data->FormatHandle(typed_handle).c_str(), type_name + 2);
    } else if (MEMORY_UNBOUND == mem) {
        result = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, typed_handle.handle,
                         error_code,
                         "%s: %s used with no memory bound and previously bound memory was freed. Memory must not be freed "
                         "prior to this operation.",
                         api_name, report_data->FormatHandle(typed_handle).c_str());
    }
    return result;
}

// Check to see if memory was ever bound to this image
bool CoreChecks::ValidateMemoryIsBoundToImage(const IMAGE_STATE *image_state, const char *api_name, const char *error_code) const {
    bool result = false;
    if (image_state->create_from_swapchain != VK_NULL_HANDLE) {
        if (image_state->bind_swapchain == VK_NULL_HANDLE) {
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                    HandleToUint64(image_state->image), error_code,
                    "%s: %s is created by %s, and the image should be bound by calling vkBindImageMemory2(), and the pNext chain "
                    "includes VkBindImageMemorySwapchainInfoKHR.",
                    api_name, report_data->FormatHandle(image_state->image).c_str(),
                    report_data->FormatHandle(image_state->create_from_swapchain).c_str());
        } else if (image_state->create_from_swapchain != image_state->bind_swapchain) {
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                    HandleToUint64(image_state->image), error_code,
                    "%s: %s is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                    "swapchain",
                    api_name, report_data->FormatHandle(image_state->image).c_str(),
                    report_data->FormatHandle(image_state->create_from_swapchain).c_str(),
                    report_data->FormatHandle(image_state->bind_swapchain).c_str());
        }
    } else if (0 == (static_cast<uint32_t>(image_state->createInfo.flags) & VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
        result = VerifyBoundMemoryIsValid(image_state->binding.mem, VulkanTypedHandle(image_state->image, kVulkanObjectTypeImage),
                                          api_name, error_code);
    }
    return result;
}

// Check to see if memory was bound to this buffer
bool CoreChecks::ValidateMemoryIsBoundToBuffer(const BUFFER_STATE *buffer_state, const char *api_name,
                                               const char *error_code) const {
    bool result = false;
    if (0 == (static_cast<uint32_t>(buffer_state->createInfo.flags) & VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
        result = VerifyBoundMemoryIsValid(buffer_state->binding.mem,
                                          VulkanTypedHandle(buffer_state->buffer, kVulkanObjectTypeBuffer), api_name, error_code);
    }
    return result;
}

// Check to see if memory was bound to this acceleration structure
bool CoreChecks::ValidateMemoryIsBoundToAccelerationStructure(const ACCELERATION_STRUCTURE_STATE *as_state, const char *api_name,
                                                              const char *error_code) const {
    return VerifyBoundMemoryIsValid(as_state->binding.mem,
                                    VulkanTypedHandle(as_state->acceleration_structure, kVulkanObjectTypeAccelerationStructureNV),
                                    api_name, error_code);
}

// Valid usage checks for a call to SetMemBinding().
// For NULL mem case, output warning
// Make sure given object is in global object map
//  IF a previous binding existed, output validation error
//  Otherwise, add reference from objectInfo to memoryInfo
//  Add reference off of objInfo
// TODO: We may need to refactor or pass in multiple valid usage statements to handle multiple valid usage conditions.
bool CoreChecks::ValidateSetMemBinding(VkDeviceMemory mem, const VulkanTypedHandle &typed_handle, const char *apiName) const {
    bool skip = false;
    // It's an error to bind an object to NULL memory
    if (mem != VK_NULL_HANDLE) {
        const BINDABLE *mem_binding = ValidationStateTracker::GetObjectMemBinding(typed_handle);
        assert(mem_binding);
        if (mem_binding->sparse) {
            const char *error_code = "VUID-vkBindImageMemory-image-01045";
            const char *handle_type = "IMAGE";
            if (typed_handle.type == kVulkanObjectTypeBuffer) {
                error_code = "VUID-vkBindBufferMemory-buffer-01030";
                handle_type = "BUFFER";
            } else {
                assert(typed_handle.type == kVulkanObjectTypeImage);
            }
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                            HandleToUint64(mem), error_code,
                            "In %s, attempting to bind %s to %s which was created with sparse memory flags "
                            "(VK_%s_CREATE_SPARSE_*_BIT).",
                            apiName, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(typed_handle).c_str(),
                            handle_type);
        }
        const DEVICE_MEMORY_STATE *mem_info = ValidationStateTracker::GetDevMemState(mem);
        if (mem_info) {
            const DEVICE_MEMORY_STATE *prev_binding = ValidationStateTracker::GetDevMemState(mem_binding->binding.mem);
            if (prev_binding) {
                const char *error_code = "VUID-vkBindImageMemory-image-01044";
                if (typed_handle.type == kVulkanObjectTypeBuffer) {
                    error_code = "VUID-vkBindBufferMemory-buffer-01029";
                } else {
                    assert(typed_handle.type == kVulkanObjectTypeImage);
                }
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                HandleToUint64(mem), error_code,
                                "In %s, attempting to bind %s to %s which has already been bound to %s.", apiName,
                                report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(typed_handle).c_str(),
                                report_data->FormatHandle(prev_binding->mem).c_str());
            } else if (mem_binding->binding.mem == MEMORY_UNBOUND) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                HandleToUint64(mem), kVUID_Core_MemTrack_RebindObject,
                                "In %s, attempting to bind %s to %s which was previous bound to memory that has "
                                "since been freed. Memory bindings are immutable in "
                                "Vulkan so this attempt to bind to new memory is not allowed.",
                                apiName, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(typed_handle).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateDeviceQueueFamily(uint32_t queue_family, const char *cmd_name, const char *parameter_name,
                                           const char *error_code, bool optional = false) const {
    bool skip = false;
    if (!optional && queue_family == VK_QUEUE_FAMILY_IGNORED) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        error_code,
                        "%s: %s is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.",
                        cmd_name, parameter_name);
    } else if (queue_family_index_map.find(queue_family) == queue_family_index_map.end()) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), error_code,
            "%s: %s (= %" PRIu32
            ") is not one of the queue families given via VkDeviceQueueCreateInfo structures when the device was created.",
            cmd_name, parameter_name, queue_family);
    }

    return skip;
}

bool CoreChecks::ValidateQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families, const char *cmd_name,
                                       const char *array_parameter_name, const char *unique_error_code,
                                       const char *valid_error_code, bool optional = false) const {
    bool skip = false;
    if (queue_families) {
        std::unordered_set<uint32_t> set;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            std::string parameter_name = std::string(array_parameter_name) + "[" + std::to_string(i) + "]";

            if (set.count(queue_families[i])) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), unique_error_code, "%s: %s (=%" PRIu32 ") is not unique within %s array.",
                                cmd_name, parameter_name.c_str(), queue_families[i], array_parameter_name);
            } else {
                set.insert(queue_families[i]);
                skip |= ValidateDeviceQueueFamily(queue_families[i], cmd_name, parameter_name.c_str(), valid_error_code, optional);
            }
        }
    }
    return skip;
}

// Check object status for selected flag state
bool CoreChecks::ValidateStatus(const CMD_BUFFER_STATE *pNode, CBStatusFlags status_mask, VkFlags msg_flags, const char *fail_msg,
                                const char *msg_code) const {
    if (!(pNode->status & status_mask)) {
        return log_msg(report_data, msg_flags, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(pNode->commandBuffer),
                       msg_code, "%s: %s..", report_data->FormatHandle(pNode->commandBuffer).c_str(), fail_msg);
    }
    return false;
}

// Return true if for a given PSO, the given state enum is dynamic, else return false
static bool IsDynamic(const PIPELINE_STATE *pPipeline, const VkDynamicState state) {
    if (pPipeline && pPipeline->graphicsPipelineCI.pDynamicState) {
        for (uint32_t i = 0; i < pPipeline->graphicsPipelineCI.pDynamicState->dynamicStateCount; i++) {
            if (state == pPipeline->graphicsPipelineCI.pDynamicState->pDynamicStates[i]) return true;
        }
    }
    return false;
}

// Validate state stored as flags at time of draw call
bool CoreChecks::ValidateDrawStateFlags(const CMD_BUFFER_STATE *pCB, const PIPELINE_STATE *pPipe, bool indexed,
                                        const char *msg_code) const {
    bool result = false;
    if (pPipe->topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
        pPipe->topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) {
        result |= ValidateStatus(pCB, CBSTATUS_LINE_WIDTH_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic line width state not set for this command buffer", msg_code);
    }
    if (pPipe->graphicsPipelineCI.pRasterizationState &&
        (pPipe->graphicsPipelineCI.pRasterizationState->depthBiasEnable == VK_TRUE)) {
        result |= ValidateStatus(pCB, CBSTATUS_DEPTH_BIAS_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic depth bias state not set for this command buffer", msg_code);
    }
    if (pPipe->blendConstantsEnabled) {
        result |= ValidateStatus(pCB, CBSTATUS_BLEND_CONSTANTS_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic blend constants state not set for this command buffer", msg_code);
    }
    if (pPipe->graphicsPipelineCI.pDepthStencilState &&
        (pPipe->graphicsPipelineCI.pDepthStencilState->depthBoundsTestEnable == VK_TRUE)) {
        result |= ValidateStatus(pCB, CBSTATUS_DEPTH_BOUNDS_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic depth bounds state not set for this command buffer", msg_code);
    }
    if (pPipe->graphicsPipelineCI.pDepthStencilState &&
        (pPipe->graphicsPipelineCI.pDepthStencilState->stencilTestEnable == VK_TRUE)) {
        result |= ValidateStatus(pCB, CBSTATUS_STENCIL_READ_MASK_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic stencil read mask state not set for this command buffer", msg_code);
        result |= ValidateStatus(pCB, CBSTATUS_STENCIL_WRITE_MASK_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic stencil write mask state not set for this command buffer", msg_code);
        result |= ValidateStatus(pCB, CBSTATUS_STENCIL_REFERENCE_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Dynamic stencil reference state not set for this command buffer", msg_code);
    }
    if (indexed) {
        result |= ValidateStatus(pCB, CBSTATUS_INDEX_BUFFER_BOUND, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                 "Index buffer object not bound to this command buffer when Indexed Draw attempted", msg_code);
    }
    if (pPipe->topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
        pPipe->topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) {
        const auto *line_state =
            lvl_find_in_chain<VkPipelineRasterizationLineStateCreateInfoEXT>(pPipe->graphicsPipelineCI.pRasterizationState->pNext);
        if (line_state && line_state->stippledLineEnable) {
            result |= ValidateStatus(pCB, CBSTATUS_LINE_STIPPLE_SET, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                     "Dynamic line stipple state not set for this command buffer", msg_code);
        }
    }

    return result;
}

bool CoreChecks::LogInvalidAttachmentMessage(const char *type1_string, const RENDER_PASS_STATE *rp1_state, const char *type2_string,
                                             const RENDER_PASS_STATE *rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                             const char *msg, const char *caller, const char *error_code) const {
    return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                   HandleToUint64(rp1_state->renderPass), error_code,
                   "%s: RenderPasses incompatible between %s w/ %s and %s w/ %s Attachment %u is not "
                   "compatible with %u: %s.",
                   caller, type1_string, report_data->FormatHandle(rp1_state->renderPass).c_str(), type2_string,
                   report_data->FormatHandle(rp2_state->renderPass).c_str(), primary_attach, secondary_attach, msg);
}

bool CoreChecks::ValidateAttachmentCompatibility(const char *type1_string, const RENDER_PASS_STATE *rp1_state,
                                                 const char *type2_string, const RENDER_PASS_STATE *rp2_state,
                                                 uint32_t primary_attach, uint32_t secondary_attach, const char *caller,
                                                 const char *error_code) const {
    bool skip = false;
    const auto &primaryPassCI = rp1_state->createInfo;
    const auto &secondaryPassCI = rp2_state->createInfo;
    if (primaryPassCI.attachmentCount <= primary_attach) {
        primary_attach = VK_ATTACHMENT_UNUSED;
    }
    if (secondaryPassCI.attachmentCount <= secondary_attach) {
        secondary_attach = VK_ATTACHMENT_UNUSED;
    }
    if (primary_attach == VK_ATTACHMENT_UNUSED && secondary_attach == VK_ATTACHMENT_UNUSED) {
        return skip;
    }
    if (primary_attach == VK_ATTACHMENT_UNUSED) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "The first is unused while the second is not.", caller, error_code);
        return skip;
    }
    if (secondary_attach == VK_ATTACHMENT_UNUSED) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "The second is unused while the first is not.", caller, error_code);
        return skip;
    }
    if (primaryPassCI.pAttachments[primary_attach].format != secondaryPassCI.pAttachments[secondary_attach].format) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different formats.", caller, error_code);
    }
    if (primaryPassCI.pAttachments[primary_attach].samples != secondaryPassCI.pAttachments[secondary_attach].samples) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different samples.", caller, error_code);
    }
    if (primaryPassCI.pAttachments[primary_attach].flags != secondaryPassCI.pAttachments[secondary_attach].flags) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different flags.", caller, error_code);
    }

    return skip;
}

bool CoreChecks::ValidateSubpassCompatibility(const char *type1_string, const RENDER_PASS_STATE *rp1_state,
                                              const char *type2_string, const RENDER_PASS_STATE *rp2_state, const int subpass,
                                              const char *caller, const char *error_code) const {
    bool skip = false;
    const auto &primary_desc = rp1_state->createInfo.pSubpasses[subpass];
    const auto &secondary_desc = rp2_state->createInfo.pSubpasses[subpass];
    uint32_t maxInputAttachmentCount = std::max(primary_desc.inputAttachmentCount, secondary_desc.inputAttachmentCount);
    for (uint32_t i = 0; i < maxInputAttachmentCount; ++i) {
        uint32_t primary_input_attach = VK_ATTACHMENT_UNUSED, secondary_input_attach = VK_ATTACHMENT_UNUSED;
        if (i < primary_desc.inputAttachmentCount) {
            primary_input_attach = primary_desc.pInputAttachments[i].attachment;
        }
        if (i < secondary_desc.inputAttachmentCount) {
            secondary_input_attach = secondary_desc.pInputAttachments[i].attachment;
        }
        skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_input_attach,
                                                secondary_input_attach, caller, error_code);
    }
    uint32_t maxColorAttachmentCount = std::max(primary_desc.colorAttachmentCount, secondary_desc.colorAttachmentCount);
    for (uint32_t i = 0; i < maxColorAttachmentCount; ++i) {
        uint32_t primary_color_attach = VK_ATTACHMENT_UNUSED, secondary_color_attach = VK_ATTACHMENT_UNUSED;
        if (i < primary_desc.colorAttachmentCount) {
            primary_color_attach = primary_desc.pColorAttachments[i].attachment;
        }
        if (i < secondary_desc.colorAttachmentCount) {
            secondary_color_attach = secondary_desc.pColorAttachments[i].attachment;
        }
        skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_color_attach,
                                                secondary_color_attach, caller, error_code);
        if (rp1_state->createInfo.subpassCount > 1) {
            uint32_t primary_resolve_attach = VK_ATTACHMENT_UNUSED, secondary_resolve_attach = VK_ATTACHMENT_UNUSED;
            if (i < primary_desc.colorAttachmentCount && primary_desc.pResolveAttachments) {
                primary_resolve_attach = primary_desc.pResolveAttachments[i].attachment;
            }
            if (i < secondary_desc.colorAttachmentCount && secondary_desc.pResolveAttachments) {
                secondary_resolve_attach = secondary_desc.pResolveAttachments[i].attachment;
            }
            skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_resolve_attach,
                                                    secondary_resolve_attach, caller, error_code);
        }
    }
    uint32_t primary_depthstencil_attach = VK_ATTACHMENT_UNUSED, secondary_depthstencil_attach = VK_ATTACHMENT_UNUSED;
    if (primary_desc.pDepthStencilAttachment) {
        primary_depthstencil_attach = primary_desc.pDepthStencilAttachment[0].attachment;
    }
    if (secondary_desc.pDepthStencilAttachment) {
        secondary_depthstencil_attach = secondary_desc.pDepthStencilAttachment[0].attachment;
    }
    skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_depthstencil_attach,
                                            secondary_depthstencil_attach, caller, error_code);
    return skip;
}

// Verify that given renderPass CreateInfo for primary and secondary command buffers are compatible.
//  This function deals directly with the CreateInfo, there are overloaded versions below that can take the renderPass handle and
//  will then feed into this function
bool CoreChecks::ValidateRenderPassCompatibility(const char *type1_string, const RENDER_PASS_STATE *rp1_state,
                                                 const char *type2_string, const RENDER_PASS_STATE *rp2_state, const char *caller,
                                                 const char *error_code) const {
    bool skip = false;

    if (rp1_state->createInfo.subpassCount != rp2_state->createInfo.subpassCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                        HandleToUint64(rp1_state->renderPass), error_code,
                        "%s: RenderPasses incompatible between %s w/ %s with a subpassCount of %u and %s w/ "
                        "%s with a subpassCount of %u.",
                        caller, type1_string, report_data->FormatHandle(rp1_state->renderPass).c_str(),
                        rp1_state->createInfo.subpassCount, type2_string, report_data->FormatHandle(rp2_state->renderPass).c_str(),
                        rp2_state->createInfo.subpassCount);
    } else {
        for (uint32_t i = 0; i < rp1_state->createInfo.subpassCount; ++i) {
            skip |= ValidateSubpassCompatibility(type1_string, rp1_state, type2_string, rp2_state, i, caller, error_code);
        }
    }
    return skip;
}

// For given pipeline, return number of MSAA samples, or one if MSAA disabled
static VkSampleCountFlagBits GetNumSamples(PIPELINE_STATE const *pipe) {
    if (pipe->graphicsPipelineCI.pMultisampleState != NULL &&
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO == pipe->graphicsPipelineCI.pMultisampleState->sType) {
        return pipe->graphicsPipelineCI.pMultisampleState->rasterizationSamples;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

static void ListBits(std::ostream &s, uint32_t bits) {
    for (int i = 0; i < 32 && bits; i++) {
        if (bits & (1 << i)) {
            s << i;
            bits &= ~(1 << i);
            if (bits) {
                s << ",";
            }
        }
    }
}

// Validate draw-time state related to the PSO
bool CoreChecks::ValidatePipelineDrawtimeState(const LAST_BOUND_STATE &state, const CMD_BUFFER_STATE *pCB, CMD_TYPE cmd_type,
                                               const PIPELINE_STATE *pPipeline, const char *caller) const {
    bool skip = false;
    const auto &current_vtx_bfr_binding_info = pCB->current_vertex_buffer_binding_info.vertex_buffer_bindings;

    // Verify vertex binding
    if (pPipeline->vertex_binding_descriptions_.size() > 0) {
        for (size_t i = 0; i < pPipeline->vertex_binding_descriptions_.size(); i++) {
            const auto vertex_binding = pPipeline->vertex_binding_descriptions_[i].binding;
            if ((current_vtx_bfr_binding_info.size() < (vertex_binding + 1)) ||
                (current_vtx_bfr_binding_info[vertex_binding].buffer == VK_NULL_HANDLE)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_VtxIndexOutOfBounds,
                            "%s expects that this Command Buffer's vertex binding Index %u should be set via "
                            "vkCmdBindVertexBuffers. This is because VkVertexInputBindingDescription struct at "
                            "index " PRINTF_SIZE_T_SPECIFIER " of pVertexBindingDescriptions has a binding value of %u.",
                            report_data->FormatHandle(state.pipeline_state->pipeline).c_str(), vertex_binding, i, vertex_binding);
            }
        }

        // Verify vertex attribute address alignment
        for (size_t i = 0; i < pPipeline->vertex_attribute_descriptions_.size(); i++) {
            const auto &attribute_description = pPipeline->vertex_attribute_descriptions_[i];
            const auto vertex_binding = attribute_description.binding;
            const auto attribute_offset = attribute_description.offset;

            const auto &vertex_binding_map_it = pPipeline->vertex_binding_to_index_map_.find(vertex_binding);
            if ((vertex_binding_map_it != pPipeline->vertex_binding_to_index_map_.cend()) &&
                (vertex_binding < current_vtx_bfr_binding_info.size()) &&
                (current_vtx_bfr_binding_info[vertex_binding].buffer != VK_NULL_HANDLE)) {
                const auto vertex_buffer_stride = pPipeline->vertex_binding_descriptions_[vertex_binding_map_it->second].stride;
                const auto vertex_buffer_offset = current_vtx_bfr_binding_info[vertex_binding].offset;

                // Use 1 as vertex/instance index to use buffer stride as well
                const auto attrib_address = vertex_buffer_offset + vertex_buffer_stride + attribute_offset;

                VkDeviceSize vtx_attrib_req_alignment = pPipeline->vertex_attribute_alignments_[i];

                if (SafeModulo(attrib_address, vtx_attrib_req_alignment) != 0) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                                    HandleToUint64(current_vtx_bfr_binding_info[vertex_binding].buffer),
                                    kVUID_Core_DrawState_InvalidVtxAttributeAlignment,
                                    "Invalid attribAddress alignment for vertex attribute " PRINTF_SIZE_T_SPECIFIER
                                    " from %s and vertex %s.",
                                    i, report_data->FormatHandle(state.pipeline_state->pipeline).c_str(),
                                    report_data->FormatHandle(current_vtx_bfr_binding_info[vertex_binding].buffer).c_str());
                }
            }
        }
    } else {
        if ((!current_vtx_bfr_binding_info.empty()) && (!pCB->vertex_buffer_used)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_VtxIndexOutOfBounds,
                        "Vertex buffers are bound to %s but no vertex buffers are attached to %s.",
                        report_data->FormatHandle(pCB->commandBuffer).c_str(),
                        report_data->FormatHandle(state.pipeline_state->pipeline).c_str());
        }
    }

    // If Viewport or scissors are dynamic, verify that dynamic count matches PSO count.
    // Skip check if rasterization is disabled or there is no viewport.
    if ((!pPipeline->graphicsPipelineCI.pRasterizationState ||
         (pPipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) &&
        pPipeline->graphicsPipelineCI.pViewportState) {
        bool dynViewport = IsDynamic(pPipeline, VK_DYNAMIC_STATE_VIEWPORT);
        bool dynScissor = IsDynamic(pPipeline, VK_DYNAMIC_STATE_SCISSOR);

        if (dynViewport) {
            const auto requiredViewportsMask = (1 << pPipeline->graphicsPipelineCI.pViewportState->viewportCount) - 1;
            const auto missingViewportMask = ~pCB->viewportMask & requiredViewportsMask;
            if (missingViewportMask) {
                std::stringstream ss;
                ss << "Dynamic viewport(s) ";
                ListBits(ss, missingViewportMask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetViewport().";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_ViewportScissorMismatch, "%s", ss.str().c_str());
            }
        }

        if (dynScissor) {
            const auto requiredScissorMask = (1 << pPipeline->graphicsPipelineCI.pViewportState->scissorCount) - 1;
            const auto missingScissorMask = ~pCB->scissorMask & requiredScissorMask;
            if (missingScissorMask) {
                std::stringstream ss;
                ss << "Dynamic scissor(s) ";
                ListBits(ss, missingScissorMask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetScissor().";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_ViewportScissorMismatch, "%s", ss.str().c_str());
            }
        }
    }

    // Verify that any MSAA request in PSO matches sample# in bound FB
    // Skip the check if rasterization is disabled.
    if (!pPipeline->graphicsPipelineCI.pRasterizationState ||
        (pPipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
        VkSampleCountFlagBits pso_num_samples = GetNumSamples(pPipeline);
        if (pCB->activeRenderPass) {
            const auto render_pass_info = pCB->activeRenderPass->createInfo.ptr();
            const VkSubpassDescription2KHR *subpass_desc = &render_pass_info->pSubpasses[pCB->activeSubpass];
            uint32_t i;
            unsigned subpass_num_samples = 0;

            for (i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED)
                    subpass_num_samples |= (unsigned)render_pass_info->pAttachments[attachment].samples;
            }

            if (subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                subpass_num_samples |= (unsigned)render_pass_info->pAttachments[attachment].samples;
            }

            if (!(device_extensions.vk_amd_mixed_attachment_samples || device_extensions.vk_nv_framebuffer_mixed_samples) &&
                ((subpass_num_samples & static_cast<unsigned>(pso_num_samples)) != subpass_num_samples)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                HandleToUint64(pPipeline->pipeline), kVUID_Core_DrawState_NumSamplesMismatch,
                                "Num samples mismatch! At draw-time in %s with %u samples while current %s w/ "
                                "%u samples!",
                                report_data->FormatHandle(pPipeline->pipeline).c_str(), pso_num_samples,
                                report_data->FormatHandle(pCB->activeRenderPass->renderPass).c_str(), subpass_num_samples);
            }
        } else {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                        HandleToUint64(pPipeline->pipeline), kVUID_Core_DrawState_NoActiveRenderpass,
                        "No active render pass found at draw-time in %s!", report_data->FormatHandle(pPipeline->pipeline).c_str());
        }
    }
    // Verify that PSO creation renderPass is compatible with active renderPass
    if (pCB->activeRenderPass) {
        // TODO: Move all of the error codes common across different Draws into a LUT accessed by cmd_type
        // TODO: AMD extension codes are included here, but actual function entrypoints are not yet intercepted
        // Error codes for renderpass and subpass mismatches
        auto rp_error = "VUID-vkCmdDraw-renderPass-02684", sp_error = "VUID-vkCmdDraw-subpass-02685";
        switch (cmd_type) {
            case CMD_DRAWINDEXED:
                rp_error = "VUID-vkCmdDrawIndexed-renderPass-02684";
                sp_error = "VUID-vkCmdDrawIndexed-subpass-02685";
                break;
            case CMD_DRAWINDIRECT:
                rp_error = "VUID-vkCmdDrawIndirect-renderPass-02684";
                sp_error = "VUID-vkCmdDrawIndirect-subpass-02685";
                break;
            case CMD_DRAWINDIRECTCOUNTKHR:
                rp_error = "VUID-vkCmdDrawIndirectCountKHR-renderPass-02684";
                sp_error = "VUID-vkCmdDrawIndirectCountKHR-subpass-02685";
                break;
            case CMD_DRAWINDEXEDINDIRECT:
                rp_error = "VUID-vkCmdDrawIndexedIndirect-renderPass-02684";
                sp_error = "VUID-vkCmdDrawIndexedIndirect-subpass-02685";
                break;
            case CMD_DRAWINDEXEDINDIRECTCOUNTKHR:
                rp_error = "VUID-vkCmdDrawIndexedIndirectCountKHR-renderPass-02684";
                sp_error = "VUID-vkCmdDrawIndexedIndirectCountKHR-subpass-02685";
                break;
            case CMD_DRAWMESHTASKSNV:
                rp_error = "VUID-vkCmdDrawMeshTasksNV-renderPass-02684";
                sp_error = "VUID-vkCmdDrawMeshTasksNV-subpass-02685";
                break;
            case CMD_DRAWMESHTASKSINDIRECTNV:
                rp_error = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-02684";
                sp_error = "VUID-vkCmdDrawMeshTasksIndirectNV-subpass-02685";
                break;
            case CMD_DRAWMESHTASKSINDIRECTCOUNTNV:
                rp_error = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-02684";
                sp_error = "VUID-vkCmdDrawMeshTasksIndirectCountNV-subpass-02685";
                break;
            default:
                assert(CMD_DRAW == cmd_type);
                break;
        }
        if (pCB->activeRenderPass->renderPass != pPipeline->rp_state->renderPass) {
            // renderPass that PSO was created with must be compatible with active renderPass that PSO is being used with
            skip |= ValidateRenderPassCompatibility("active render pass", pCB->activeRenderPass, "pipeline state object",
                                                    pPipeline->rp_state.get(), caller, rp_error);
        }
        if (pPipeline->graphicsPipelineCI.subpass != pCB->activeSubpass) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                        HandleToUint64(pPipeline->pipeline), sp_error, "Pipeline was built for subpass %u but used in subpass %u.",
                        pPipeline->graphicsPipelineCI.subpass, pCB->activeSubpass);
        }
    }

    return skip;
}

// For given cvdescriptorset::DescriptorSet, verify that its Set is compatible w/ the setLayout corresponding to
// pipelineLayout[layoutIndex]
static bool VerifySetLayoutCompatibility(const debug_report_data *report_data, const cvdescriptorset::DescriptorSet *descriptor_set,
                                         PIPELINE_LAYOUT_STATE const *pipeline_layout, const uint32_t layoutIndex,
                                         string &errorMsg) {
    auto num_sets = pipeline_layout->set_layouts.size();
    if (layoutIndex >= num_sets) {
        stringstream errorStr;
        errorStr << report_data->FormatHandle(pipeline_layout->layout) << ") only contains " << num_sets
                 << " setLayouts corresponding to sets 0-" << num_sets - 1 << ", but you're attempting to bind set to index "
                 << layoutIndex;
        errorMsg = errorStr.str();
        return false;
    }
    if (descriptor_set->IsPushDescriptor()) return true;
    auto layout_node = pipeline_layout->set_layouts[layoutIndex].get();
    return cvdescriptorset::VerifySetLayoutCompatibility(report_data, layout_node, descriptor_set->GetLayout().get(), &errorMsg);
}

static const char *string_VuidNotCompatibleForSet(CMD_TYPE cmd_type) {
    const static std::map<CMD_TYPE, const char *> incompatible_for_set_vuid = {
        {CMD_DISPATCH, "VUID-vkCmdDispatch-None-02697"},
        {CMD_DISPATCHINDIRECT, "VUID-vkCmdDispatchIndirect-None-02697"},
        {CMD_DRAW, "VUID-vkCmdDraw-None-02697"},
        {CMD_DRAWINDEXED, "VUID-vkCmdDrawIndexed-None-02697"},
        {CMD_DRAWINDEXEDINDIRECT, "VUID-vkCmdDrawIndexedIndirect-None-02697"},
        {CMD_DRAWINDEXEDINDIRECTCOUNTKHR, "VUID-vkCmdDrawIndexedIndirectCountKHR-None-02697"},
        {CMD_DRAWINDIRECT, "VUID-vkCmdDrawIndirect-None-02697"},
        {CMD_DRAWINDIRECTCOUNTKHR, "VUID-vkCmdDrawIndirectCountKHR-None-02697"},
        {CMD_DRAWMESHTASKSINDIRECTCOUNTNV, "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02697"},
        {CMD_DRAWMESHTASKSINDIRECTNV, "VUID-vkCmdDrawMeshTasksIndirectNV-None-02697"},
        {CMD_DRAWMESHTASKSNV, "VUID-vkCmdDrawMeshTasksNV-None-02697"},

        // Not implemented on this path...
        // { CMD_DRAWDISPATCHBASE, "VUID-vkCmdDispatchBase-None-02697" },
        // { CMD_DRAWINDIRECTBYTECOUNTEXT, "VUID-vkCmdDrawIndirectByteCountEXT-None-02697"},
        {CMD_TRACERAYSNV, "VUID-vkCmdTraceRaysNV-None-02697"},
    };
    auto find_it = incompatible_for_set_vuid.find(cmd_type);
    if (find_it == incompatible_for_set_vuid.cend()) {
        assert(find_it != incompatible_for_set_vuid.cend());
        return "BAD VUID -- Unknown Command Type";
    }
    return find_it->second;
}
// Validate overall state at the time of a draw call
bool CoreChecks::ValidateCmdBufDrawState(const CMD_BUFFER_STATE *cb_node, CMD_TYPE cmd_type, const bool indexed,
                                         const VkPipelineBindPoint bind_point, const char *function, const char *pipe_err_code,
                                         const char *state_err_code) const {
    const auto last_bound_it = cb_node->lastBound.find(bind_point);
    const PIPELINE_STATE *pPipe = nullptr;
    if (last_bound_it != cb_node->lastBound.cend()) {
        pPipe = last_bound_it->second.pipeline_state;
    }

    if (nullptr == pPipe) {
        return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                       HandleToUint64(cb_node->commandBuffer), pipe_err_code,
                       "Must not call %s on this command buffer while there is no %s pipeline bound.", function,
                       bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS ? "Graphics" : "Compute");
    }

    bool result = false;
    auto const &state = last_bound_it->second;

    // First check flag states
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) result = ValidateDrawStateFlags(cb_node, pPipe, indexed, state_err_code);

    // Now complete other state checks
    string errorString;
    auto const &pipeline_layout = pPipe->pipeline_layout.get();

    // Check if the current pipeline is compatible for the maximum used set with the bound sets.
    if (pPipe->active_slots.size() > 0 && !CompatForSet(pPipe->max_active_slot, state, pipeline_layout->compat_for_set)) {
        result |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                          HandleToUint64(pPipe->pipeline), string_VuidNotCompatibleForSet(cmd_type),
                          "%s(): %s defined with %s is not compatible for maximum set statically used %" PRIu32
                          " with bound descriptor sets, last bound with %s",
                          command_name_list[cmd_type], report_data->FormatHandle(pPipe->pipeline).c_str(),
                          report_data->FormatHandle(pipeline_layout->layout).c_str(), pPipe->max_active_slot,
                          report_data->FormatHandle(state.pipeline_layout).c_str());
    }

    for (const auto &set_binding_pair : pPipe->active_slots) {
        uint32_t setIndex = set_binding_pair.first;
        // If valid set is not bound throw an error
        if ((state.per_set.size() <= setIndex) || (!state.per_set[setIndex].bound_descriptor_set)) {
            result |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_DescriptorSetNotBound,
                        "%s uses set #%u but that set is not bound.", report_data->FormatHandle(pPipe->pipeline).c_str(), setIndex);
        } else if (!VerifySetLayoutCompatibility(report_data, state.per_set[setIndex].bound_descriptor_set, pipeline_layout,
                                                 setIndex, errorString)) {
            // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
            VkDescriptorSet setHandle = state.per_set[setIndex].bound_descriptor_set->GetSet();
            result |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                              HandleToUint64(setHandle), kVUID_Core_DrawState_PipelineLayoutsIncompatible,
                              "%s bound as set #%u is not compatible with overlapping %s due to: %s",
                              report_data->FormatHandle(setHandle).c_str(), setIndex,
                              report_data->FormatHandle(pipeline_layout->layout).c_str(), errorString.c_str());
        } else {  // Valid set is bound and layout compatible, validate that it's updated
            // Pull the set node
            const cvdescriptorset::DescriptorSet *descriptor_set = state.per_set[setIndex].bound_descriptor_set;
            // Validate the draw-time state for this descriptor set
            std::string err_str;
            if (!descriptor_set->IsPushDescriptor()) {
                // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation checks.
                // Here, the currently bound pipeline determines whether an image validation check is redundant...
                // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline.
                cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second);
                const auto &binding_req_map = reduced_map.FilteredMap(*cb_node, *pPipe);

                // We can skip validating the descriptor set if "nothing" has changed since the last validation.
                // Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
                // any dynamic descriptors, always revalidate rather than caching the values. We currently only
                // apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
                // binding_req_map which could potentially be expensive.
                bool descriptor_set_changed =
                    !reduced_map.IsManyDescriptors() ||
                    // Revalidate each time if the set has dynamic offsets
                    state.per_set[setIndex].dynamicOffsets.size() > 0 ||
                    // Revalidate if descriptor set (or contents) has changed
                    state.per_set[setIndex].validated_set != descriptor_set ||
                    state.per_set[setIndex].validated_set_change_count != descriptor_set->GetChangeCount() ||
                    (!disabled.image_layout_validation &&
                     state.per_set[setIndex].validated_set_image_layout_change_count != cb_node->image_layout_change_count);
                bool need_validate = descriptor_set_changed ||
                                     // Revalidate if previous bindingReqMap doesn't include new bindingReqMap
                                     !std::includes(state.per_set[setIndex].validated_set_binding_req_map.begin(),
                                                    state.per_set[setIndex].validated_set_binding_req_map.end(),
                                                    binding_req_map.begin(), binding_req_map.end());

                if (need_validate) {
                    bool success;
                    if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                        // Only validate the bindings that haven't already been validated
                        BindingReqMap delta_reqs;
                        std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                            state.per_set[setIndex].validated_set_binding_req_map.begin(),
                                            state.per_set[setIndex].validated_set_binding_req_map.end(),
                                            std::inserter(delta_reqs, delta_reqs.begin()));
                        success = ValidateDrawState(descriptor_set, delta_reqs, state.per_set[setIndex].dynamicOffsets, cb_node,
                                                    function, &err_str);
                    } else {
                        success = ValidateDrawState(descriptor_set, binding_req_map, state.per_set[setIndex].dynamicOffsets,
                                                    cb_node, function, &err_str);
                    }
                    if (!success) {
                        auto set = descriptor_set->GetSet();
                        result |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                    HandleToUint64(set), kVUID_Core_DrawState_DescriptorSetNotUpdated,
                                    "%s bound as set #%u encountered the following validation error at %s time: %s",
                                    report_data->FormatHandle(set).c_str(), setIndex, function, err_str.c_str());
                    }
                }
            }
        }
    }

    // Check general pipeline state that needs to be validated at drawtime
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point)
        result |= ValidatePipelineDrawtimeState(state, cb_node, cmd_type, pPipe, function);

    return result;
}

bool CoreChecks::ValidatePipelineLocked(std::vector<std::shared_ptr<PIPELINE_STATE>> const &pPipelines, int pipelineIndex) const {
    bool skip = false;

    const PIPELINE_STATE *pPipeline = pPipelines[pipelineIndex].get();

    // If create derivative bit is set, check that we've specified a base
    // pipeline correctly, and that the base pipeline was created to allow
    // derivatives.
    if (pPipeline->graphicsPipelineCI.flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
        const PIPELINE_STATE *pBasePipeline = nullptr;
        if (!((pPipeline->graphicsPipelineCI.basePipelineHandle != VK_NULL_HANDLE) ^
              (pPipeline->graphicsPipelineCI.basePipelineIndex != -1))) {
            // This check is a superset of "VUID-VkGraphicsPipelineCreateInfo-flags-00724" and
            // "VUID-VkGraphicsPipelineCreateInfo-flags-00725"
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), kVUID_Core_DrawState_InvalidPipelineCreateState,
                            "Invalid Pipeline CreateInfo: exactly one of base pipeline index and handle must be specified");
        } else if (pPipeline->graphicsPipelineCI.basePipelineIndex != -1) {
            if (pPipeline->graphicsPipelineCI.basePipelineIndex >= pipelineIndex) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-vkCreateGraphicsPipelines-flags-00720",
                                "Invalid Pipeline CreateInfo: base pipeline must occur earlier in array than derivative pipeline.");
            } else {
                pBasePipeline = pPipelines[pPipeline->graphicsPipelineCI.basePipelineIndex].get();
            }
        } else if (pPipeline->graphicsPipelineCI.basePipelineHandle != VK_NULL_HANDLE) {
            pBasePipeline = GetPipelineState(pPipeline->graphicsPipelineCI.basePipelineHandle);
        }

        if (pBasePipeline && !(pBasePipeline->graphicsPipelineCI.flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), kVUID_Core_DrawState_InvalidPipelineCreateState,
                            "Invalid Pipeline CreateInfo: base pipeline does not allow derivatives.");
        }
    }

    return skip;
}

// UNLOCKED pipeline validation. DO NOT lookup objects in the CoreChecks->* maps in this function.
bool CoreChecks::ValidatePipelineUnlocked(const PIPELINE_STATE *pPipeline, uint32_t pipelineIndex) const {
    bool skip = false;

    // Ensure the subpass index is valid. If not, then ValidateGraphicsPipelineShaderState
    // produces nonsense errors that confuse users. Other layers should already
    // emit errors for renderpass being invalid.
    auto subpass_desc = &pPipeline->rp_state->createInfo.pSubpasses[pPipeline->graphicsPipelineCI.subpass];
    if (pPipeline->graphicsPipelineCI.subpass >= pPipeline->rp_state->createInfo.subpassCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-subpass-00759",
                        "Invalid Pipeline CreateInfo State: Subpass index %u is out of range for this renderpass (0..%u).",
                        pPipeline->graphicsPipelineCI.subpass, pPipeline->rp_state->createInfo.subpassCount - 1);
        subpass_desc = nullptr;
    }

    if (pPipeline->graphicsPipelineCI.pColorBlendState != NULL) {
        const safe_VkPipelineColorBlendStateCreateInfo *color_blend_state = pPipeline->graphicsPipelineCI.pColorBlendState;
        if (subpass_desc && color_blend_state->attachmentCount != subpass_desc->colorAttachmentCount) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-attachmentCount-00746",
                        "vkCreateGraphicsPipelines(): %s subpass %u has colorAttachmentCount of %u which doesn't "
                        "match the pColorBlendState->attachmentCount of %u.",
                        report_data->FormatHandle(pPipeline->rp_state->renderPass).c_str(), pPipeline->graphicsPipelineCI.subpass,
                        subpass_desc->colorAttachmentCount, color_blend_state->attachmentCount);
        }
        if (!enabled_features.core.independentBlend) {
            if (pPipeline->attachments.size() > 1) {
                const VkPipelineColorBlendAttachmentState *const pAttachments = &pPipeline->attachments[0];
                for (size_t i = 1; i < pPipeline->attachments.size(); i++) {
                    // Quoting the spec: "If [the independent blend] feature is not enabled, the VkPipelineColorBlendAttachmentState
                    // settings for all color attachments must be identical." VkPipelineColorBlendAttachmentState contains
                    // only attachment state, so memcmp is best suited for the comparison
                    if (memcmp(static_cast<const void *>(pAttachments), static_cast<const void *>(&pAttachments[i]),
                               sizeof(pAttachments[0]))) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                        HandleToUint64(device), "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605",
                                        "Invalid Pipeline CreateInfo: If independent blend feature not enabled, all elements of "
                                        "pAttachments must be identical.");
                        break;
                    }
                }
            }
        }
        if (!enabled_features.core.logicOp && (pPipeline->graphicsPipelineCI.pColorBlendState->logicOpEnable != VK_FALSE)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00606",
                        "Invalid Pipeline CreateInfo: If logic operations feature not enabled, logicOpEnable must be VK_FALSE.");
        }
        for (size_t i = 0; i < pPipeline->attachments.size(); i++) {
            if ((pPipeline->attachments[i].srcColorBlendFactor == VK_BLEND_FACTOR_SRC1_COLOR) ||
                (pPipeline->attachments[i].srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR) ||
                (pPipeline->attachments[i].srcColorBlendFactor == VK_BLEND_FACTOR_SRC1_ALPHA) ||
                (pPipeline->attachments[i].srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-00608",
                                "vkCreateGraphicsPipelines(): pPipelines[%d].pColorBlendState.pAttachments[" PRINTF_SIZE_T_SPECIFIER
                                "].srcColorBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                "enabled.",
                                pipelineIndex, i, pPipeline->attachments[i].srcColorBlendFactor);
                }
            }
            if ((pPipeline->attachments[i].dstColorBlendFactor == VK_BLEND_FACTOR_SRC1_COLOR) ||
                (pPipeline->attachments[i].dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR) ||
                (pPipeline->attachments[i].dstColorBlendFactor == VK_BLEND_FACTOR_SRC1_ALPHA) ||
                (pPipeline->attachments[i].dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-00609",
                                "vkCreateGraphicsPipelines(): pPipelines[%d].pColorBlendState.pAttachments[" PRINTF_SIZE_T_SPECIFIER
                                "].dstColorBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                "enabled.",
                                pipelineIndex, i, pPipeline->attachments[i].dstColorBlendFactor);
                }
            }
            if ((pPipeline->attachments[i].srcAlphaBlendFactor == VK_BLEND_FACTOR_SRC1_COLOR) ||
                (pPipeline->attachments[i].srcAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR) ||
                (pPipeline->attachments[i].srcAlphaBlendFactor == VK_BLEND_FACTOR_SRC1_ALPHA) ||
                (pPipeline->attachments[i].srcAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-00610",
                                "vkCreateGraphicsPipelines(): pPipelines[%d].pColorBlendState.pAttachments[" PRINTF_SIZE_T_SPECIFIER
                                "].srcAlphaBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                "enabled.",
                                pipelineIndex, i, pPipeline->attachments[i].srcAlphaBlendFactor);
                }
            }
            if ((pPipeline->attachments[i].dstAlphaBlendFactor == VK_BLEND_FACTOR_SRC1_COLOR) ||
                (pPipeline->attachments[i].dstAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR) ||
                (pPipeline->attachments[i].dstAlphaBlendFactor == VK_BLEND_FACTOR_SRC1_ALPHA) ||
                (pPipeline->attachments[i].dstAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-00611",
                                "vkCreateGraphicsPipelines(): pPipelines[%d].pColorBlendState.pAttachments[" PRINTF_SIZE_T_SPECIFIER
                                "].dstAlphaBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                "enabled.",
                                pipelineIndex, i, pPipeline->attachments[i].dstAlphaBlendFactor);
                }
            }
        }
    }

    if (ValidateGraphicsPipelineShaderState(pPipeline)) {
        skip = true;
    }
    // Each shader's stage must be unique
    if (pPipeline->duplicate_shaders) {
        for (uint32_t stage = VK_SHADER_STAGE_VERTEX_BIT; stage & VK_SHADER_STAGE_ALL_GRAPHICS; stage <<= 1) {
            if (pPipeline->duplicate_shaders & stage) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), kVUID_Core_DrawState_InvalidPipelineCreateState,
                                "Invalid Pipeline CreateInfo State: Multiple shaders provided for stage %s",
                                string_VkShaderStageFlagBits(VkShaderStageFlagBits(stage)));
            }
        }
    }
    if (device_extensions.vk_nv_mesh_shader) {
        // VS or mesh is required
        if (!(pPipeline->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_NV))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                            "Invalid Pipeline CreateInfo State: Vertex Shader or Mesh Shader required.");
        }
        // Can't mix mesh and VTG
        if ((pPipeline->active_shaders & (VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_TASK_BIT_NV)) &&
            (pPipeline->active_shaders &
             (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
              VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-pStages-02095",
                            "Invalid Pipeline CreateInfo State: Geometric shader stages must either be all mesh (mesh | task) "
                            "or all VTG (vertex, tess control, tess eval, geom).");
        }
    } else {
        // VS is required
        if (!(pPipeline->active_shaders & VK_SHADER_STAGE_VERTEX_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-stage-00727",
                            "Invalid Pipeline CreateInfo State: Vertex Shader required.");
        }
    }

    if (!enabled_features.mesh_shader.meshShader && (pPipeline->active_shaders & VK_SHADER_STAGE_MESH_BIT_NV)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                        "Invalid Pipeline CreateInfo State: Mesh Shader not supported.");
    }

    if (!enabled_features.mesh_shader.taskShader && (pPipeline->active_shaders & VK_SHADER_STAGE_TASK_BIT_NV)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkPipelineShaderStageCreateInfo-stage-02092",
                        "Invalid Pipeline CreateInfo State: Task Shader not supported.");
    }

    // Either both or neither TC/TE shaders should be defined
    bool has_control = (pPipeline->active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
    bool has_eval = (pPipeline->active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0;
    if (has_control && !has_eval) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-00729",
                        "Invalid Pipeline CreateInfo State: TE and TC shaders must be included or excluded as a pair.");
    }
    if (!has_control && has_eval) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-00730",
                        "Invalid Pipeline CreateInfo State: TE and TC shaders must be included or excluded as a pair.");
    }
    // Compute shaders should be specified independent of Gfx shaders
    if (pPipeline->active_shaders & VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-stage-00728",
                        "Invalid Pipeline CreateInfo State: Do not specify Compute Shader for Gfx Pipeline.");
    }

    if ((pPipeline->active_shaders & VK_SHADER_STAGE_VERTEX_BIT) && !pPipeline->graphicsPipelineCI.pInputAssemblyState) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-02098",
                        "Invalid Pipeline CreateInfo State: Missing pInputAssemblyState.");
    }

    // VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only valid for tessellation pipelines.
    // Mismatching primitive topology and tessellation fails graphics pipeline creation.
    if (has_control && has_eval &&
        (!pPipeline->graphicsPipelineCI.pInputAssemblyState ||
         pPipeline->graphicsPipelineCI.pInputAssemblyState->topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-00736",
                        "Invalid Pipeline CreateInfo State: VK_PRIMITIVE_TOPOLOGY_PATCH_LIST must be set as IA topology for "
                        "tessellation pipelines.");
    }
    if (pPipeline->graphicsPipelineCI.pInputAssemblyState) {
        if (pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
            if (!has_control || !has_eval) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-topology-00737",
                            "Invalid Pipeline CreateInfo State: VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only valid "
                            "for tessellation pipelines.");
            }
        }

        if ((pPipeline->graphicsPipelineCI.pInputAssemblyState->primitiveRestartEnable == VK_TRUE) &&
            (pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                            "topology is %s and primitiveRestartEnable is VK_TRUE. It is invalid.",
                            string_VkPrimitiveTopology(pPipeline->graphicsPipelineCI.pInputAssemblyState->topology));
        }
        if ((enabled_features.core.geometryShader == VK_FALSE) &&
            (pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
             pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429",
                            "topology is %s and geometry shaders feature is not enabled. It is invalid.",
                            string_VkPrimitiveTopology(pPipeline->graphicsPipelineCI.pInputAssemblyState->topology));
        }
        if ((enabled_features.core.tessellationShader == VK_FALSE) &&
            (pPipeline->graphicsPipelineCI.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430",
                            "topology is %s and tessellation shaders feature is not enabled. It is invalid.",
                            string_VkPrimitiveTopology(pPipeline->graphicsPipelineCI.pInputAssemblyState->topology));
        }
    }

    // If a rasterization state is provided...
    if (pPipeline->graphicsPipelineCI.pRasterizationState) {
        if ((pPipeline->graphicsPipelineCI.pRasterizationState->depthClampEnable == VK_TRUE) &&
            (!enabled_features.core.depthClamp)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkPipelineRasterizationStateCreateInfo-depthClampEnable-00782",
                            "vkCreateGraphicsPipelines(): the depthClamp device feature is disabled: the depthClampEnable member "
                            "of the VkPipelineRasterizationStateCreateInfo structure must be set to VK_FALSE.");
        }

        if (!IsDynamic(pPipeline, VK_DYNAMIC_STATE_DEPTH_BIAS) &&
            (pPipeline->graphicsPipelineCI.pRasterizationState->depthBiasClamp != 0.0) && (!enabled_features.core.depthBiasClamp)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), kVUID_Core_DrawState_InvalidFeature,
                            "vkCreateGraphicsPipelines(): the depthBiasClamp device feature is disabled: the depthBiasClamp member "
                            "of the VkPipelineRasterizationStateCreateInfo structure must be set to 0.0 unless the "
                            "VK_DYNAMIC_STATE_DEPTH_BIAS dynamic state is enabled");
        }

        // If rasterization is enabled...
        if (pPipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable == VK_FALSE) {
            if ((pPipeline->graphicsPipelineCI.pMultisampleState->alphaToOneEnable == VK_TRUE) &&
                (!enabled_features.core.alphaToOne)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineMultisampleStateCreateInfo-alphaToOneEnable-00785",
                                "vkCreateGraphicsPipelines(): the alphaToOne device feature is disabled: the alphaToOneEnable "
                                "member of the VkPipelineMultisampleStateCreateInfo structure must be set to VK_FALSE.");
            }

            // If subpass uses a depth/stencil attachment, pDepthStencilState must be a pointer to a valid structure
            if (subpass_desc && subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                if (!pPipeline->graphicsPipelineCI.pDepthStencilState) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                    HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00752",
                                    "Invalid Pipeline CreateInfo State: pDepthStencilState is NULL when rasterization is enabled "
                                    "and subpass uses a depth/stencil attachment.");

                } else if ((pPipeline->graphicsPipelineCI.pDepthStencilState->depthBoundsTestEnable == VK_TRUE) &&
                           (!enabled_features.core.depthBounds)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkPipelineDepthStencilStateCreateInfo-depthBoundsTestEnable-00598",
                                "vkCreateGraphicsPipelines(): the depthBounds device feature is disabled: the "
                                "depthBoundsTestEnable member of the VkPipelineDepthStencilStateCreateInfo structure must be "
                                "set to VK_FALSE.");
                }
            }

            // If subpass uses color attachments, pColorBlendState must be valid pointer
            if (subpass_desc) {
                uint32_t color_attachment_count = 0;
                for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                    if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                        ++color_attachment_count;
                    }
                }
                if (color_attachment_count > 0 && pPipeline->graphicsPipelineCI.pColorBlendState == nullptr) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                    HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00753",
                                    "Invalid Pipeline CreateInfo State: pColorBlendState is NULL when rasterization is enabled and "
                                    "subpass uses color attachments.");
                }
            }
        }
    }

    if ((pPipeline->active_shaders & VK_SHADER_STAGE_VERTEX_BIT) && !pPipeline->graphicsPipelineCI.pVertexInputState) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkGraphicsPipelineCreateInfo-pStages-02097",
                        "Invalid Pipeline CreateInfo State: Missing pVertexInputState.");
    }

    auto vi = pPipeline->graphicsPipelineCI.pVertexInputState;
    if (vi != NULL) {
        for (uint32_t j = 0; j < vi->vertexAttributeDescriptionCount; j++) {
            VkFormat format = vi->pVertexAttributeDescriptions[j].format;
            // Internal call to get format info.  Still goes through layers, could potentially go directly to ICD.
            VkFormatProperties properties;
            DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
            if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkVertexInputAttributeDescription-format-00623",
                            "vkCreateGraphicsPipelines: pCreateInfo[%d].pVertexInputState->vertexAttributeDescriptions[%d].format "
                            "(%s) is not a supported vertex buffer format.",
                            pipelineIndex, j, string_VkFormat(format));
            }
        }
    }

    if (subpass_desc && pPipeline->graphicsPipelineCI.pMultisampleState) {
        auto accumColorSamples = [subpass_desc, pPipeline](uint32_t &samples) {
            for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    samples |= static_cast<uint32_t>(pPipeline->rp_state->createInfo.pAttachments[attachment].samples);
                }
            }
        };

        if (!(device_extensions.vk_amd_mixed_attachment_samples || device_extensions.vk_nv_framebuffer_mixed_samples)) {
            uint32_t raster_samples = static_cast<uint32_t>(GetNumSamples(pPipeline));
            uint32_t subpass_num_samples = 0;

            accumColorSamples(subpass_num_samples);

            if (subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                subpass_num_samples |= static_cast<uint32_t>(pPipeline->rp_state->createInfo.pAttachments[attachment].samples);
            }

            // subpass_num_samples is 0 when the subpass has no attachments or if all attachments are VK_ATTACHMENT_UNUSED.
            // Only validate the value of subpass_num_samples if the subpass has attachments that are not VK_ATTACHMENT_UNUSED.
            if (subpass_num_samples && (!IsPowerOfTwo(subpass_num_samples) || (subpass_num_samples != raster_samples))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-subpass-00757",
                                "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->rasterizationSamples (%u) "
                                "does not match the number of samples of the RenderPass color and/or depth attachment.",
                                pipelineIndex, raster_samples);
            }
        }

        if (device_extensions.vk_amd_mixed_attachment_samples) {
            VkSampleCountFlagBits max_sample_count = static_cast<VkSampleCountFlagBits>(0);
            for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                    max_sample_count = std::max(
                        max_sample_count,
                        pPipeline->rp_state->createInfo.pAttachments[subpass_desc->pColorAttachments[i].attachment].samples);
                }
            }
            if (subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                max_sample_count = std::max(
                    max_sample_count,
                    pPipeline->rp_state->createInfo.pAttachments[subpass_desc->pDepthStencilAttachment->attachment].samples);
            }
            if ((pPipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable == VK_FALSE) &&
                (pPipeline->graphicsPipelineCI.pMultisampleState->rasterizationSamples != max_sample_count)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-subpass-01505",
                                "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->rasterizationSamples (%s) != max "
                                "attachment samples (%s) used in subpass %u.",
                                pipelineIndex,
                                string_VkSampleCountFlagBits(pPipeline->graphicsPipelineCI.pMultisampleState->rasterizationSamples),
                                string_VkSampleCountFlagBits(max_sample_count), pPipeline->graphicsPipelineCI.subpass);
            }
        }

        if (device_extensions.vk_nv_framebuffer_mixed_samples) {
            uint32_t raster_samples = static_cast<uint32_t>(GetNumSamples(pPipeline));
            uint32_t subpass_color_samples = 0;

            accumColorSamples(subpass_color_samples);

            if (subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                const uint32_t subpass_depth_samples =
                    static_cast<uint32_t>(pPipeline->rp_state->createInfo.pAttachments[attachment].samples);

                if (pPipeline->graphicsPipelineCI.pDepthStencilState) {
                    const bool ds_test_enabled =
                        (pPipeline->graphicsPipelineCI.pDepthStencilState->depthTestEnable == VK_TRUE) ||
                        (pPipeline->graphicsPipelineCI.pDepthStencilState->depthBoundsTestEnable == VK_TRUE) ||
                        (pPipeline->graphicsPipelineCI.pDepthStencilState->stencilTestEnable == VK_TRUE);

                    if (ds_test_enabled && (!IsPowerOfTwo(subpass_depth_samples) || (raster_samples != subpass_depth_samples))) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                        HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-subpass-01411",
                                        "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->rasterizationSamples (%u) "
                                        "does not match the number of samples of the RenderPass depth attachment (%u).",
                                        pipelineIndex, raster_samples, subpass_depth_samples);
                    }
                }
            }

            if (IsPowerOfTwo(subpass_color_samples)) {
                if (raster_samples < subpass_color_samples) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                    HandleToUint64(device), "VUID-VkGraphicsPipelineCreateInfo-subpass-01412",
                                    "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->rasterizationSamples (%u) "
                                    "is not greater or equal to the number of samples of the RenderPass color attachment (%u).",
                                    pipelineIndex, raster_samples, subpass_color_samples);
                }

                if (pPipeline->graphicsPipelineCI.pMultisampleState) {
                    if ((raster_samples > subpass_color_samples) &&
                        (pPipeline->graphicsPipelineCI.pMultisampleState->sampleShadingEnable == VK_TRUE)) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                    HandleToUint64(device), "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415",
                                    "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->sampleShadingEnable must be "
                                    "VK_FALSE when "
                                    "pCreateInfo[%d].pMultisampleState->rasterizationSamples (%u) is greater than the number of "
                                    "samples of the "
                                    "subpass color attachment (%u).",
                                    pipelineIndex, pipelineIndex, raster_samples, subpass_color_samples);
                    }

                    const auto *coverage_modulation_state = lvl_find_in_chain<VkPipelineCoverageModulationStateCreateInfoNV>(
                        pPipeline->graphicsPipelineCI.pMultisampleState->pNext);

                    if (coverage_modulation_state && (coverage_modulation_state->coverageModulationTableEnable == VK_TRUE)) {
                        if (coverage_modulation_state->coverageModulationTableCount != (raster_samples / subpass_color_samples)) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                        HandleToUint64(device),
                                        "VUID-VkPipelineCoverageModulationStateCreateInfoNV-coverageModulationTableEnable-01405",
                                        "vkCreateGraphicsPipelines: pCreateInfos[%d] VkPipelineCoverageModulationStateCreateInfoNV "
                                        "coverageModulationTableCount of %u is invalid.",
                                        pipelineIndex, coverage_modulation_state->coverageModulationTableCount);
                        }
                    }
                }
            }
        }

        if (device_extensions.vk_nv_fragment_coverage_to_color) {
            const auto coverage_to_color_state =
                lvl_find_in_chain<VkPipelineCoverageToColorStateCreateInfoNV>(pPipeline->graphicsPipelineCI.pMultisampleState);

            if (coverage_to_color_state && coverage_to_color_state->coverageToColorEnable == VK_TRUE) {
                bool attachment_is_valid = false;
                std::string error_detail;

                if (coverage_to_color_state->coverageToColorLocation < subpass_desc->colorAttachmentCount) {
                    const auto color_attachment_ref =
                        subpass_desc->pColorAttachments[coverage_to_color_state->coverageToColorLocation];
                    if (color_attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                        const auto color_attachment = pPipeline->rp_state->createInfo.pAttachments[color_attachment_ref.attachment];

                        switch (color_attachment.format) {
                            case VK_FORMAT_R8_UINT:
                            case VK_FORMAT_R8_SINT:
                            case VK_FORMAT_R16_UINT:
                            case VK_FORMAT_R16_SINT:
                            case VK_FORMAT_R32_UINT:
                            case VK_FORMAT_R32_SINT:
                                attachment_is_valid = true;
                                break;
                            default:
                                string_sprintf(&error_detail, "references an attachment with an invalid format (%s).",
                                               string_VkFormat(color_attachment.format));
                                break;
                        }
                    } else {
                        string_sprintf(&error_detail,
                                       "references an invalid attachment. The subpass pColorAttachments[%" PRIu32
                                       "].attachment has the value "
                                       "VK_ATTACHMENT_UNUSED.",
                                       coverage_to_color_state->coverageToColorLocation);
                    }
                } else {
                    string_sprintf(&error_detail,
                                   "references an non-existing attachment since the subpass colorAttachmentCount is %" PRIu32 ".",
                                   subpass_desc->colorAttachmentCount);
                }

                if (!attachment_is_valid) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                    HandleToUint64(device),
                                    "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRId32
                                    "].pMultisampleState VkPipelineCoverageToColorStateCreateInfoNV "
                                    "coverageToColorLocation = %" PRIu32 " %s",
                                    pipelineIndex, coverage_to_color_state->coverageToColorLocation, error_detail.c_str());
                }
            }
        }
    }

    return skip;
}

// Block of code at start here specifically for managing/tracking DSs

// Validate that given set is valid and that it's not being used by an in-flight CmdBuffer
// func_str is the name of the calling function
// Return false if no errors occur
// Return true if validation error occurs and callback returns true (to skip upcoming API call down the chain)
bool CoreChecks::ValidateIdleDescriptorSet(VkDescriptorSet set, const char *func_str) const {
    if (disabled.idle_descriptor_set) return false;
    bool skip = false;
    auto set_node = setMap.find(set);
    if (set_node == setMap.end()) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                        HandleToUint64(set), kVUID_Core_DrawState_DoubleDestroy,
                        "Cannot call %s() on %s that has not been allocated.", func_str, report_data->FormatHandle(set).c_str());
    } else {
        // TODO : This covers various error cases so should pass error enum into this function and use passed in enum here
        if (set_node->second->in_use.load()) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            HandleToUint64(set), "VUID-vkFreeDescriptorSets-pDescriptorSets-00309",
                            "Cannot call %s() on %s that is in use by a command buffer.", func_str,
                            report_data->FormatHandle(set).c_str());
        }
    }
    return skip;
}

// If a renderpass is active, verify that the given command type is appropriate for current subpass state
bool CoreChecks::ValidateCmdSubpassState(const CMD_BUFFER_STATE *pCB, const CMD_TYPE cmd_type) const {
    if (!pCB->activeRenderPass) return false;
    bool skip = false;
    if (pCB->activeSubpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS &&
        (cmd_type != CMD_EXECUTECOMMANDS && cmd_type != CMD_NEXTSUBPASS && cmd_type != CMD_ENDRENDERPASS &&
         cmd_type != CMD_NEXTSUBPASS2KHR && cmd_type != CMD_ENDRENDERPASS2KHR)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidCommandBuffer,
                        "Commands cannot be called in a subpass using secondary command buffers.");
    } else if (pCB->activeSubpassContents == VK_SUBPASS_CONTENTS_INLINE && cmd_type == CMD_EXECUTECOMMANDS) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidCommandBuffer,
                        "vkCmdExecuteCommands() cannot be called in a subpass using inline commands.");
    }
    return skip;
}

bool CoreChecks::ValidateCmdQueueFlags(const CMD_BUFFER_STATE *cb_node, const char *caller_name, VkQueueFlags required_flags,
                                       const char *error_code) const {
    auto pool = cb_node->command_pool.get();
    if (pool) {
        VkQueueFlags queue_flags = GetPhysicalDeviceState()->queue_family_properties[pool->queueFamilyIndex].queueFlags;
        if (!(required_flags & queue_flags)) {
            string required_flags_string;
            for (auto flag : {VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT}) {
                if (flag & required_flags) {
                    if (required_flags_string.size()) {
                        required_flags_string += " or ";
                    }
                    required_flags_string += string_VkQueueFlagBits(flag);
                }
            }
            return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                           HandleToUint64(cb_node->commandBuffer), error_code,
                           "Cannot call %s on a command buffer allocated from a pool without %s capabilities..", caller_name,
                           required_flags_string.c_str());
        }
    }
    return false;
}

static char const *GetCauseStr(VulkanTypedHandle obj) {
    if (obj.type == kVulkanObjectTypeDescriptorSet) return "destroyed or updated";
    if (obj.type == kVulkanObjectTypeCommandBuffer) return "destroyed or rerecorded";
    return "destroyed";
}

bool CoreChecks::ReportInvalidCommandBuffer(const CMD_BUFFER_STATE *cb_state, const char *call_source) const {
    bool skip = false;
    for (auto obj : cb_state->broken_bindings) {
        const char *cause_str = GetCauseStr(obj);
        string VUID;
        string_sprintf(&VUID, "%s-%s", kVUID_Core_DrawState_InvalidCommandBuffer, object_string[obj.type]);
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_state->commandBuffer), VUID.c_str(),
                    "You are adding %s to %s that is invalid because bound %s was %s.", call_source,
                    report_data->FormatHandle(cb_state->commandBuffer).c_str(), report_data->FormatHandle(obj).c_str(), cause_str);
    }
    return skip;
}

// 'commandBuffer must be in the recording state' valid usage error code for each command
// Autogenerated as part of the vk_validation_error_message.h codegen
static const std::array<const char *, CMD_RANGE_SIZE> must_be_recording_list = {{VUID_MUST_BE_RECORDING_LIST}};

// Validate the given command being added to the specified cmd buffer, flagging errors if CB is not in the recording state or if
// there's an issue with the Cmd ordering
bool CoreChecks::ValidateCmd(const CMD_BUFFER_STATE *cb_state, const CMD_TYPE cmd, const char *caller_name) const {
    switch (cb_state->state) {
        case CB_RECORDING:
            return ValidateCmdSubpassState(cb_state, cmd);

        case CB_INVALID_COMPLETE:
        case CB_INVALID_INCOMPLETE:
            return ReportInvalidCommandBuffer(cb_state, caller_name);

        default:
            assert(cmd != CMD_NONE);
            const auto error = must_be_recording_list[cmd];
            return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                           HandleToUint64(cb_state->commandBuffer), error,
                           "You must call vkBeginCommandBuffer() before this call to %s.", caller_name);
    }
}

bool CoreChecks::ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, VkDebugReportObjectTypeEXT VUID_handle_type,
                                                         uint64_t VUID_handle, const char *VUID) const {
    bool skip = false;
    uint32_t count = 1 << physical_device_count;
    if (count <= deviceMask) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VUID_handle_type, VUID_handle, VUID,
                        "deviceMask(0x%" PRIx32 ") is invaild. Physical device count is %" PRIu32 ".", deviceMask,
                        physical_device_count);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToZero(uint32_t deviceMask, VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle,
                                          const char *VUID) const {
    bool skip = false;
    if (deviceMask == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VUID_handle_type, VUID_handle, VUID,
                        "deviceMask(0x%" PRIx32 ") must be non-zero.", deviceMask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE *pCB, uint32_t deviceMask,
                                                   VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle,
                                                   const char *VUID) const {
    bool skip = false;
    if ((deviceMask & pCB->initial_device_mask) != deviceMask) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VUID_handle_type, VUID_handle, VUID,
                        "deviceMask(0x%" PRIx32 ") is not a subset of %s initial device mask(0x%" PRIx32 ").", deviceMask,
                        report_data->FormatHandle(pCB->commandBuffer).c_str(), pCB->initial_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE *pCB, uint32_t deviceMask,
                                                VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle,
                                                const char *VUID) const {
    bool skip = false;
    if ((deviceMask & pCB->active_render_pass_device_mask) != deviceMask) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VUID_handle_type, VUID_handle, VUID,
                        "deviceMask(0x%" PRIx32 ") is not a subset of %s device mask(0x%" PRIx32 ").", deviceMask,
                        report_data->FormatHandle(pCB->activeRenderPass->renderPass).c_str(), pCB->active_render_pass_device_mask);
    }
    return skip;
}

// Flags validation error if the associated call is made inside a render pass. The apiName routine should ONLY be called outside a
// render pass.
bool CoreChecks::InsideRenderPass(const CMD_BUFFER_STATE *pCB, const char *apiName, const char *msgCode) const {
    bool inside = false;
    if (pCB->activeRenderPass) {
        inside = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                         HandleToUint64(pCB->commandBuffer), msgCode, "%s: It is invalid to issue this call inside an active %s.",
                         apiName, report_data->FormatHandle(pCB->activeRenderPass->renderPass).c_str());
    }
    return inside;
}

// Flags validation error if the associated call is made outside a render pass. The apiName
// routine should ONLY be called inside a render pass.
bool CoreChecks::OutsideRenderPass(const CMD_BUFFER_STATE *pCB, const char *apiName, const char *msgCode) const {
    bool outside = false;
    if (((pCB->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) && (!pCB->activeRenderPass)) ||
        ((pCB->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) && (!pCB->activeRenderPass) &&
         !(pCB->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT))) {
        outside = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                          HandleToUint64(pCB->commandBuffer), msgCode, "%s: This call must be issued inside an active render pass.",
                          apiName);
    }
    return outside;
}

bool CoreChecks::ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t requested_queue_family,
                                          const char *err_code, const char *cmd_name, const char *queue_family_var_name) const {
    bool skip = false;

    if (requested_queue_family >= pd_state->queue_family_known_count) {
        const char *conditional_ext_cmd =
            instance_extensions.vk_khr_get_physical_device_properties_2 ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]" : "";

        const std::string count_note = (UNCALLED == pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState)
                                           ? "the pQueueFamilyPropertyCount was never obtained"
                                           : "i.e. is not less than " + std::to_string(pd_state->queue_family_known_count);

        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                        HandleToUint64(pd_state->phys_device), err_code,
                        "%s: %s (= %" PRIu32
                        ") is not less than any previously obtained pQueueFamilyPropertyCount from "
                        "vkGetPhysicalDeviceQueueFamilyProperties%s (%s).",
                        cmd_name, queue_family_var_name, requested_queue_family, conditional_ext_cmd, count_note.c_str());
    }
    return skip;
}

// Verify VkDeviceQueueCreateInfos
bool CoreChecks::ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t info_count,
                                                const VkDeviceQueueCreateInfo *infos) const {
    bool skip = false;

    std::unordered_set<uint32_t> queue_family_set;

    for (uint32_t i = 0; i < info_count; ++i) {
        const auto requested_queue_family = infos[i].queueFamilyIndex;

        std::string queue_family_var_name = "pCreateInfo->pQueueCreateInfos[" + std::to_string(i) + "].queueFamilyIndex";
        skip |= ValidateQueueFamilyIndex(pd_state, requested_queue_family, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                         "vkCreateDevice", queue_family_var_name.c_str());

        if (queue_family_set.insert(requested_queue_family).second == false) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(pd_state->phys_device), "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                            "CreateDevice(): %s (=%" PRIu32 ") is not unique within pQueueCreateInfos.",
                            queue_family_var_name.c_str(), requested_queue_family);
        }

        // Verify that requested queue count of queue family is known to be valid at this point in time
        if (requested_queue_family < pd_state->queue_family_known_count) {
            const auto requested_queue_count = infos[i].queueCount;
            const bool queue_family_has_props = requested_queue_family < pd_state->queue_family_properties.size();
            // spec guarantees at least one queue for each queue family
            const uint32_t available_queue_count =
                queue_family_has_props ? pd_state->queue_family_properties[requested_queue_family].queueCount : 1;
            const char *conditional_ext_cmd = instance_extensions.vk_khr_get_physical_device_properties_2
                                                  ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]"
                                                  : "";

            if (requested_queue_count > available_queue_count) {
                const std::string count_note =
                    queue_family_has_props
                        ? "i.e. is not less than or equal to " +
                              std::to_string(pd_state->queue_family_properties[requested_queue_family].queueCount)
                        : "the pQueueFamilyProperties[" + std::to_string(requested_queue_family) + "] was never obtained";

                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(pd_state->phys_device), "VUID-VkDeviceQueueCreateInfo-queueCount-00382",
                    "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].queueCount (=%" PRIu32
                    ") is not less than or equal to available queue count for this pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "].queueFamilyIndex} (=%" PRIu32 ") obtained previously from vkGetPhysicalDeviceQueueFamilyProperties%s (%s).",
                    i, requested_queue_count, i, requested_queue_family, conditional_ext_cmd, count_note.c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) const {
    bool skip = false;
    auto pd_state = GetPhysicalDeviceState(gpu);

    // TODO: object_tracker should perhaps do this instead
    //       and it does not seem to currently work anyway -- the loader just crashes before this point
    if (!pd_state) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, 0,
                        kVUID_Core_DevLimit_MustQueryCount,
                        "Invalid call to vkCreateDevice() w/o first calling vkEnumeratePhysicalDevices().");
    } else {
        skip |= ValidateDeviceQueueCreateInfos(pd_state, pCreateInfo->queueCreateInfoCount, pCreateInfo->pQueueCreateInfos);
    }
    return skip;
}

void CoreChecks::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    StateTracker::PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeCoreValidation);
    CoreChecks *core_checks = static_cast<CoreChecks *>(validation_data);
    core_checks->SetSetImageViewInitialLayoutCallback(
        [core_checks](CMD_BUFFER_STATE *cb_node, const IMAGE_VIEW_STATE &iv_state, VkImageLayout layout) -> void {
            core_checks->SetImageViewInitialLayout(cb_node, iv_state, layout);
        });
}

void CoreChecks::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;
    imageSubresourceMap.clear();
    imageLayoutMap.clear();

    StateTracker::PreCallRecordDestroyDevice(device, pAllocator);
}

// For given stage mask, if Geometry shader stage is on w/o GS being enabled, report geo_error_id
//   and if Tessellation Control or Evaluation shader stages are on w/o TS being enabled, report tess_error_id.
// Similarly for mesh and task shaders.
bool CoreChecks::ValidateStageMaskGsTsEnables(VkPipelineStageFlags stageMask, const char *caller, const char *geo_error_id,
                                              const char *tess_error_id, const char *mesh_error_id,
                                              const char *task_error_id) const {
    bool skip = false;
    if (!enabled_features.core.geometryShader && (stageMask & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, geo_error_id,
                        "%s call includes a stageMask with VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT bit set when device does not have "
                        "geometryShader feature enabled.",
                        caller);
    }
    if (!enabled_features.core.tessellationShader &&
        (stageMask & (VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, tess_error_id,
                        "%s call includes a stageMask with VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT and/or "
                        "VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT bit(s) set when device does not have "
                        "tessellationShader feature enabled.",
                        caller);
    }
    if (!enabled_features.mesh_shader.meshShader && (stageMask & VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, mesh_error_id,
                        "%s call includes a stageMask with VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV bit set when device does not have "
                        "VkPhysicalDeviceMeshShaderFeaturesNV::meshShader feature enabled.",
                        caller);
    }
    if (!enabled_features.mesh_shader.taskShader && (stageMask & VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, task_error_id,
                        "%s call includes a stageMask with VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV bit set when device does not have "
                        "VkPhysicalDeviceMeshShaderFeaturesNV::taskShader feature enabled.",
                        caller);
    }
    return skip;
}

// Note: This function assumes that the global lock is held by the calling thread.
// For the given queue, verify the queue state up to the given seq number.
// Currently the only check is to make sure that if there are events to be waited on prior to
//  a QueryReset, make sure that all such events have been signalled.
bool CoreChecks::VerifyQueueStateToSeq(const QUEUE_STATE *initial_queue, uint64_t initial_seq) const {
    bool skip = false;

    // sequence number we want to validate up to, per queue
    std::unordered_map<const QUEUE_STATE *, uint64_t> target_seqs{{initial_queue, initial_seq}};
    // sequence number we've completed validation for, per queue
    std::unordered_map<const QUEUE_STATE *, uint64_t> done_seqs;
    std::vector<const QUEUE_STATE *> worklist{initial_queue};

    while (worklist.size()) {
        auto queue = worklist.back();
        worklist.pop_back();

        auto target_seq = target_seqs[queue];
        auto seq = std::max(done_seqs[queue], queue->seq);
        auto sub_it = queue->submissions.begin() + int(seq - queue->seq);  // seq >= queue->seq

        for (; seq < target_seq; ++sub_it, ++seq) {
            for (auto &wait : sub_it->waitSemaphores) {
                auto other_queue = GetQueueState(wait.queue);

                if (other_queue == queue) continue;  // semaphores /always/ point backwards, so no point here.

                auto other_target_seq = std::max(target_seqs[other_queue], wait.seq);
                auto other_done_seq = std::max(done_seqs[other_queue], other_queue->seq);

                // if this wait is for another queue, and covers new sequence
                // numbers beyond what we've already validated, mark the new
                // target seq and (possibly-re)add the queue to the worklist.
                if (other_done_seq < other_target_seq) {
                    target_seqs[other_queue] = other_target_seq;
                    worklist.push_back(other_queue);
                }
            }
        }

        // finally mark the point we've now validated this queue to.
        done_seqs[queue] = seq;
    }

    return skip;
}

// When the given fence is retired, verify outstanding queue operations through the point of the fence
bool CoreChecks::VerifyQueueStateToFence(VkFence fence) const {
    auto fence_state = GetFenceState(fence);
    if (fence_state && fence_state->scope == kSyncScopeInternal && VK_NULL_HANDLE != fence_state->signaler.first) {
        return VerifyQueueStateToSeq(GetQueueState(fence_state->signaler.first), fence_state->signaler.second);
    }
    return false;
}

bool CoreChecks::ValidateCommandBufferSimultaneousUse(const CMD_BUFFER_STATE *pCB, int current_submit_count) const {
    bool skip = false;
    if ((pCB->in_use.load() || current_submit_count > 1) &&
        !(pCB->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                        "VUID-vkQueueSubmit-pCommandBuffers-00071", "%s is already in use and is not marked for simultaneous use.",
                        report_data->FormatHandle(pCB->commandBuffer).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateCommandBufferState(const CMD_BUFFER_STATE *cb_state, const char *call_source, int current_submit_count,
                                            const char *vu_id) const {
    bool skip = false;
    if (disabled.command_buffer_state) return skip;
    // Validate ONE_TIME_SUBMIT_BIT CB is not being submitted more than once
    if ((cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) &&
        (cb_state->submitCount + current_submit_count > 1)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                        kVUID_Core_DrawState_CommandBufferSingleSubmitViolation,
                        "%s was begun w/ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set, but has been submitted 0x%" PRIxLEAST64
                        "times.",
                        report_data->FormatHandle(cb_state->commandBuffer).c_str(), cb_state->submitCount + current_submit_count);
    }

    // Validate that cmd buffers have been updated
    switch (cb_state->state) {
        case CB_INVALID_INCOMPLETE:
        case CB_INVALID_COMPLETE:
            skip |= ReportInvalidCommandBuffer(cb_state, call_source);
            break;

        case CB_NEW:
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            (uint64_t)(cb_state->commandBuffer), vu_id,
                            "%s used in the call to %s is unrecorded and contains no commands.",
                            report_data->FormatHandle(cb_state->commandBuffer).c_str(), call_source);
            break;

        case CB_RECORDING:
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), kVUID_Core_DrawState_NoEndCommandBuffer,
                            "You must call vkEndCommandBuffer() on %s before this call to %s!",
                            report_data->FormatHandle(cb_state->commandBuffer).c_str(), call_source);
            break;

        default: /* recorded */
            break;
    }
    return skip;
}

// Check that the queue family index of 'queue' matches one of the entries in pQueueFamilyIndices
bool CoreChecks::ValidImageBufferQueue(const CMD_BUFFER_STATE *cb_node, const VulkanTypedHandle &object, uint32_t queueFamilyIndex,
                                       uint32_t count, const uint32_t *indices) const {
    bool found = false;
    bool skip = false;
    for (uint32_t i = 0; i < count; i++) {
        if (indices[i] == queueFamilyIndex) {
            found = true;
            break;
        }
    }

    if (!found) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[object.type], object.handle,
                       kVUID_Core_DrawState_InvalidQueueFamily,
                       "vkQueueSubmit: %s contains %s which was not created allowing concurrent access to "
                       "this queue family %d.",
                       report_data->FormatHandle(cb_node->commandBuffer).c_str(), report_data->FormatHandle(object).c_str(),
                       queueFamilyIndex);
    }
    return skip;
}

// Validate that queueFamilyIndices of primary command buffers match this queue
// Secondary command buffers were previously validated in vkCmdExecuteCommands().
bool CoreChecks::ValidateQueueFamilyIndices(const CMD_BUFFER_STATE *pCB, VkQueue queue) const {
    bool skip = false;
    auto pPool = pCB->command_pool.get();
    auto queue_state = GetQueueState(queue);

    if (pPool && queue_state) {
        if (pPool->queueFamilyIndex != queue_state->queueFamilyIndex) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCB->commandBuffer), "VUID-vkQueueSubmit-pCommandBuffers-00074",
                            "vkQueueSubmit: Primary %s created in queue family %d is being submitted on %s "
                            "from queue family %d.",
                            report_data->FormatHandle(pCB->commandBuffer).c_str(), pPool->queueFamilyIndex,
                            report_data->FormatHandle(queue).c_str(), queue_state->queueFamilyIndex);
        }

        // Ensure that any bound images or buffers created with SHARING_MODE_CONCURRENT have access to the current queue family
        for (const auto &object : pCB->object_bindings) {
            if (object.type == kVulkanObjectTypeImage) {
                auto image_state = object.node ? (IMAGE_STATE *)object.node : GetImageState(object.Cast<VkImage>());
                if (image_state && image_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(pCB, object, queue_state->queueFamilyIndex,
                                                  image_state->createInfo.queueFamilyIndexCount,
                                                  image_state->createInfo.pQueueFamilyIndices);
                }
            } else if (object.type == kVulkanObjectTypeBuffer) {
                auto buffer_state = object.node ? (BUFFER_STATE *)object.node : GetBufferState(object.Cast<VkBuffer>());
                if (buffer_state && buffer_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(pCB, object, queue_state->queueFamilyIndex,
                                                  buffer_state->createInfo.queueFamilyIndexCount,
                                                  buffer_state->createInfo.pQueueFamilyIndices);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBufferState(const CMD_BUFFER_STATE *pCB, int current_submit_count,
                                                   QFOTransferCBScoreboards<VkImageMemoryBarrier> *qfo_image_scoreboards,
                                                   QFOTransferCBScoreboards<VkBufferMemoryBarrier> *qfo_buffer_scoreboards) const {
    // Track in-use for resources off of primary and any secondary CBs
    bool skip = false;

    // If USAGE_SIMULTANEOUS_USE_BIT not set then CB cannot already be executing on device
    skip |= ValidateCommandBufferSimultaneousUse(pCB, current_submit_count);

    skip |= ValidateQueuedQFOTransfers(pCB, qfo_image_scoreboards, qfo_buffer_scoreboards);

    for (auto pSubCB : pCB->linkedCommandBuffers) {
        skip |= ValidateQueuedQFOTransfers(pSubCB, qfo_image_scoreboards, qfo_buffer_scoreboards);
        // TODO: replace with InvalidateCommandBuffers() at recording.
        if ((pSubCB->primaryCommandBuffer != pCB->commandBuffer) &&
            !(pSubCB->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                    "VUID-vkQueueSubmit-pCommandBuffers-00073",
                    "%s was submitted with secondary %s but that buffer has subsequently been bound to "
                    "primary %s and it does not have VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                    report_data->FormatHandle(pCB->commandBuffer).c_str(), report_data->FormatHandle(pSubCB->commandBuffer).c_str(),
                    report_data->FormatHandle(pSubCB->primaryCommandBuffer).c_str());
        }
    }

    skip |= ValidateCommandBufferState(pCB, "vkQueueSubmit()", current_submit_count, "VUID-vkQueueSubmit-pCommandBuffers-00072");

    return skip;
}

bool CoreChecks::ValidateFenceForSubmit(const FENCE_STATE *pFence) const {
    bool skip = false;

    if (pFence && pFence->scope == kSyncScopeInternal) {
        if (pFence->state == FENCE_INFLIGHT) {
            // TODO: opportunities for "VUID-vkQueueSubmit-fence-00064", "VUID-vkQueueBindSparse-fence-01114",
            // "VUID-vkAcquireNextImageKHR-fence-01287"
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,
                            HandleToUint64(pFence->fence), kVUID_Core_DrawState_InvalidFence,
                            "%s is already in use by another submission.", report_data->FormatHandle(pFence->fence).c_str());
        }

        else if (pFence->state == FENCE_RETIRED) {
            // TODO: opportunities for "VUID-vkQueueSubmit-fence-00063", "VUID-vkQueueBindSparse-fence-01113",
            // "VUID-vkAcquireNextImageKHR-fence-01287"
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,
                            HandleToUint64(pFence->fence), kVUID_Core_MemTrack_FenceState,
                            "%s submitted in SIGNALED state.  Fences must be reset before being submitted",
                            report_data->FormatHandle(pFence->fence).c_str());
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                           VkResult result) {
    StateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);

    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (cb_node) {
                for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                    UpdateCmdBufImageLayouts(secondaryCmdBuffer);
                    RecordQueuedQFOTransfers(secondaryCmdBuffer);
                }
                UpdateCmdBufImageLayouts(cb_node);
                RecordQueuedQFOTransfers(cb_node);
            }
        }
    }
}
bool CoreChecks::ValidateSemaphoresForSubmit(VkQueue queue, const VkSubmitInfo *submit,
                                             unordered_set<VkSemaphore> *unsignaled_sema_arg,
                                             unordered_set<VkSemaphore> *signaled_sema_arg,
                                             unordered_set<VkSemaphore> *internal_sema_arg) const {
    bool skip = false;
    unordered_set<VkSemaphore> &signaled_semaphores = *signaled_sema_arg;
    unordered_set<VkSemaphore> &unsignaled_semaphores = *unsignaled_sema_arg;
    unordered_set<VkSemaphore> &internal_semaphores = *internal_sema_arg;

    for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
        skip |=
            ValidateStageMaskGsTsEnables(submit->pWaitDstStageMask[i], "vkQueueSubmit()",
                                         "VUID-VkSubmitInfo-pWaitDstStageMask-00076", "VUID-VkSubmitInfo-pWaitDstStageMask-00077",
                                         "VUID-VkSubmitInfo-pWaitDstStageMask-02089", "VUID-VkSubmitInfo-pWaitDstStageMask-02090");
        VkSemaphore semaphore = submit->pWaitSemaphores[i];
        const auto *pSemaphore = GetSemaphoreState(semaphore);
        if (pSemaphore && (pSemaphore->scope == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
            if (unsignaled_semaphores.count(semaphore) || (!(signaled_semaphores.count(semaphore)) && !(pSemaphore->signaled))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                "%s is waiting on %s that has no way to be signaled.", report_data->FormatHandle(queue).c_str(),
                                report_data->FormatHandle(semaphore).c_str());
            } else {
                signaled_semaphores.erase(semaphore);
                unsignaled_semaphores.insert(semaphore);
            }
        }
        if (pSemaphore && pSemaphore->scope == kSyncScopeExternalTemporary) {
            internal_semaphores.insert(semaphore);
        }
    }
    for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit->pSignalSemaphores[i];
        const auto *pSemaphore = GetSemaphoreState(semaphore);
        if (pSemaphore && (pSemaphore->scope == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
            if (signaled_semaphores.count(semaphore) || (!(unsignaled_semaphores.count(semaphore)) && pSemaphore->signaled)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                "%s is signaling %s that was previously signaled by %s but has not since "
                                "been waited on by any queue.",
                                report_data->FormatHandle(queue).c_str(), report_data->FormatHandle(semaphore).c_str(),
                                report_data->FormatHandle(pSemaphore->signaler.first).c_str());
            } else {
                unsignaled_semaphores.erase(semaphore);
                signaled_semaphores.insert(semaphore);
            }
        }
    }

    return skip;
}
bool CoreChecks::ValidateCommandBuffersForSubmit(VkQueue queue, const VkSubmitInfo *submit,
                                                 ImageSubresPairLayoutMap *localImageLayoutMap_arg,
                                                 vector<VkCommandBuffer> *current_cmds_arg) const {
    bool skip = false;
    auto queue_state = GetQueueState(queue);

    ImageSubresPairLayoutMap &localImageLayoutMap = *localImageLayoutMap_arg;
    vector<VkCommandBuffer> &current_cmds = *current_cmds_arg;

    QFOTransferCBScoreboards<VkImageMemoryBarrier> qfo_image_scoreboards;
    QFOTransferCBScoreboards<VkBufferMemoryBarrier> qfo_buffer_scoreboards;
    QueryMap localQueryToStateMap;
    EventToStageMap localEventToStageMap;

    for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
        const auto *cb_node = GetCBState(submit->pCommandBuffers[i]);
        if (cb_node) {
            skip |= ValidateCmdBufImageLayouts(cb_node, imageLayoutMap, &localImageLayoutMap);
            current_cmds.push_back(submit->pCommandBuffers[i]);
            skip |= ValidatePrimaryCommandBufferState(
                cb_node, (int)std::count(current_cmds.begin(), current_cmds.end(), submit->pCommandBuffers[i]),
                &qfo_image_scoreboards, &qfo_buffer_scoreboards);
            skip |= ValidateQueueFamilyIndices(cb_node, queue);

            for (auto descriptorSet : cb_node->validate_descriptorsets_in_queuesubmit) {
                const cvdescriptorset::DescriptorSet *set_node = GetSetNode(descriptorSet.first);
                if (set_node) {
                    for (auto pipe : descriptorSet.second) {
                        for (auto binding : pipe.second) {
                            std::string error;
                            std::vector<uint32_t> dynamicOffsets;
                            // dynamic data isn't allowed in UPDATE_AFTER_BIND, so dynamicOffsets is always empty.
                            if (!ValidateDescriptorSetBindingData(cb_node, set_node, dynamicOffsets, binding.first, binding.second,
                                                                  "vkQueueSubmit()", &error)) {
                                skip |= log_msg(
                                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                    HandleToUint64(descriptorSet.first), kVUID_Core_DrawState_DescriptorSetNotUpdated,
                                    "%s bound the following validation error at %s time: %s",
                                    report_data->FormatHandle(descriptorSet.first).c_str(), "vkQueueSubmit()", error.c_str());
                            }
                        }
                    }
                }
            }

            // Potential early exit here as bad object state may crash in delayed function calls
            if (skip) {
                return true;
            }

            // Call submit-time functions to validate or update local mirrors of state
            // (to preserve const-ness at validate time)
            for (auto &function : cb_node->queue_submit_functions) {
                skip |= function(this, queue_state);
            }
            for (auto &function : cb_node->eventUpdates) {
                skip |= function(this, /*do_validate*/ true, &localEventToStageMap);
            }
            for (auto &function : cb_node->queryUpdates) {
                skip |= function(this, /*do_validate*/ true, &localQueryToStateMap);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                            VkFence fence) const {
    const auto *pFence = GetFenceState(fence);
    bool skip = ValidateFenceForSubmit(pFence);
    if (skip) {
        return true;
    }

    unordered_set<VkSemaphore> signaled_semaphores;
    unordered_set<VkSemaphore> unsignaled_semaphores;
    unordered_set<VkSemaphore> internal_semaphores;
    vector<VkCommandBuffer> current_cmds;
    ImageSubresPairLayoutMap localImageLayoutMap;
    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        skip |= ValidateSemaphoresForSubmit(queue, submit, &unsignaled_semaphores, &signaled_semaphores, &internal_semaphores);
        skip |= ValidateCommandBuffersForSubmit(queue, submit, &localImageLayoutMap, &current_cmds);

        auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupSubmitInfo>(submit->pNext);
        if (chained_device_group_struct && chained_device_group_struct->commandBufferCount > 0) {
            for (uint32_t i = 0; i < chained_device_group_struct->commandBufferCount; ++i) {
                skip |= ValidateDeviceMaskToPhysicalDeviceCount(chained_device_group_struct->pCommandBufferDeviceMasks[i],
                                                                VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, HandleToUint64(queue),
                                                                "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
            }
        }
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only on Android and only for NDK versions
// that support the VK_ANDROID_external_memory_android_hardware_buffer extension.
// This chunk could move into a seperate core_validation_android.cpp file... ?

// clang-format off

// Map external format and usage flags to/from equivalent Vulkan flags
// (Tables as of v1.1.92)

// AHardwareBuffer Format                       Vulkan Format
// ======================                       =============
// AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM        VK_FORMAT_R8G8B8A8_UNORM
// AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM        VK_FORMAT_R8G8B8A8_UNORM
// AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM          VK_FORMAT_R8G8B8_UNORM
// AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM          VK_FORMAT_R5G6B5_UNORM_PACK16
// AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT    VK_FORMAT_R16G16B16A16_SFLOAT
// AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM     VK_FORMAT_A2B10G10R10_UNORM_PACK32
// AHARDWAREBUFFER_FORMAT_D16_UNORM             VK_FORMAT_D16_UNORM
// AHARDWAREBUFFER_FORMAT_D24_UNORM             VK_FORMAT_X8_D24_UNORM_PACK32
// AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT     VK_FORMAT_D24_UNORM_S8_UINT
// AHARDWAREBUFFER_FORMAT_D32_FLOAT             VK_FORMAT_D32_SFLOAT
// AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT     VK_FORMAT_D32_SFLOAT_S8_UINT
// AHARDWAREBUFFER_FORMAT_S8_UINT               VK_FORMAT_S8_UINT

// The AHARDWAREBUFFER_FORMAT_* are an enum in the NDK headers, but get passed in to Vulkan
// as uint32_t. Casting the enums here avoids scattering casts around in the code.
std::map<uint32_t, VkFormat> ahb_format_map_a2v = {
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,        VK_FORMAT_R8G8B8A8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,        VK_FORMAT_R8G8B8A8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM,          VK_FORMAT_R8G8B8_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM,          VK_FORMAT_R5G6B5_UNORM_PACK16 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT,    VK_FORMAT_R16G16B16A16_SFLOAT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM,     VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D16_UNORM,             VK_FORMAT_D16_UNORM },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM,             VK_FORMAT_X8_D24_UNORM_PACK32 },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT,     VK_FORMAT_D24_UNORM_S8_UINT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT,             VK_FORMAT_D32_SFLOAT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT,     VK_FORMAT_D32_SFLOAT_S8_UINT },
    { (uint32_t)AHARDWAREBUFFER_FORMAT_S8_UINT,               VK_FORMAT_S8_UINT }
};

// AHardwareBuffer Usage                        Vulkan Usage or Creation Flag (Intermixed - Aargh!)
// =====================                        =================================================== 
// None                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT
// None                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT
// AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE      VK_IMAGE_USAGE_SAMPLED_BIT
// AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE      VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
// AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
// AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP           VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
// AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE    None 
// AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT      VK_IMAGE_CREATE_PROTECTED_BIT
// None                                         VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
// None                                         VK_IMAGE_CREATE_EXTENDED_USAGE_BIT

// Same casting rationale. De-mixing the table to prevent type confusion and aliasing 
std::map<uint64_t, VkImageUsageFlags> ahb_usage_map_a2v = {
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,    (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT,     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE,  0 },   // No equivalent 
};

std::map<uint64_t, VkImageCreateFlags> ahb_create_map_a2v = {
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP,         VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT },
    { (uint64_t)AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT,    VK_IMAGE_CREATE_PROTECTED_BIT },
    { (uint64_t)AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE,  0 },   // No equivalent 
};

std::map<VkImageUsageFlags, uint64_t> ahb_usage_map_v2a = {
    { VK_IMAGE_USAGE_SAMPLED_BIT,           (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE },
    { VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE },
    { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT  },
};

std::map<VkImageCreateFlags, uint64_t> ahb_create_map_v2a = {
    { VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,  (uint64_t)AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP },
    { VK_IMAGE_CREATE_PROTECTED_BIT,        (uint64_t)AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT },
};

// clang-format on

//
// AHB-extension new APIs
//
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties) const {
    bool skip = false;
    //  buffer must be a valid Android hardware buffer object with at least one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags.
    AHardwareBuffer_Desc ahb_desc;
    AHardwareBuffer_describe(buffer, &ahb_desc);
    uint32_t required_flags = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                              AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    if (0 == (ahb_desc.usage & required_flags)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884",
                        "vkGetAndroidHardwareBufferPropertiesANDROID: The AHardwareBuffer's AHardwareBuffer_Desc.usage (0x%" PRIx64
                        ") does not have any AHARDWAREBUFFER_USAGE_GPU_* flags set.",
                        ahb_desc.usage);
    }
    return skip;
}

void CoreChecks::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                         VkAndroidHardwareBufferPropertiesANDROID *pProperties,
                                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    auto ahb_format_props = lvl_find_in_chain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
    if (ahb_format_props) {
        ahb_ext_formats_set.insert(ahb_format_props->externalFormat);
    }
}

bool CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                      struct AHardwareBuffer **pBuffer) const {
    bool skip = false;
    const DEVICE_MEMORY_STATE *mem_info = GetDevMemState(pInfo->memory);

    // VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID must have been included in
    // VkExportMemoryAllocateInfoKHR::handleTypes when memory was created.
    if (!mem_info->is_export ||
        (0 == (mem_info->export_handle_type_flags & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882",
                        "vkGetMemoryAndroidHardwareBufferANDROID: %s was not allocated for export, or the "
                        "export handleTypes (0x%" PRIx32
                        ") did not contain VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                        report_data->FormatHandle(pInfo->memory).c_str(), mem_info->export_handle_type_flags);
    }

    // If the pNext chain of the VkMemoryAllocateInfo used to allocate memory included a VkMemoryDedicatedAllocateInfo
    // with non-NULL image member, then that image must already be bound to memory.
    if (mem_info->is_dedicated && (VK_NULL_HANDLE != mem_info->dedicated_image)) {
        const auto image_state = GetImageState(mem_info->dedicated_image);
        if ((nullptr == image_state) || (0 == (image_state->GetBoundMemory().count(pInfo->memory)))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883",
                            "vkGetMemoryAndroidHardwareBufferANDROID: %s was allocated using a dedicated "
                            "%s, but that image is not bound to the VkDeviceMemory object.",
                            report_data->FormatHandle(pInfo->memory).c_str(),
                            report_data->FormatHandle(mem_info->dedicated_image).c_str());
        }
    }

    return skip;
}

//
// AHB-specific validation within non-AHB APIs
//
bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *alloc_info) const {
    bool skip = false;
    auto import_ahb_info = lvl_find_in_chain<VkImportAndroidHardwareBufferInfoANDROID>(alloc_info->pNext);
    auto exp_mem_alloc_info = lvl_find_in_chain<VkExportMemoryAllocateInfo>(alloc_info->pNext);
    auto mem_ded_alloc_info = lvl_find_in_chain<VkMemoryDedicatedAllocateInfo>(alloc_info->pNext);

    if ((import_ahb_info) && (NULL != import_ahb_info->buffer)) {
        // This is an import with handleType of VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID
        AHardwareBuffer_Desc ahb_desc = {};
        AHardwareBuffer_describe(import_ahb_info->buffer, &ahb_desc);

        //  If buffer is not NULL, it must be a valid Android hardware buffer object with AHardwareBuffer_Desc::format and
        //  AHardwareBuffer_Desc::usage compatible with Vulkan as described in Android Hardware Buffers.
        //
        //  BLOB & GPU_DATA_BUFFER combo specifically allowed
        if ((AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) || (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
            // Otherwise, must be a combination from the AHardwareBuffer Format and Usage Equivalence tables
            // Usage must have at least one bit from the table. It may have additional bits not in the table
            uint64_t ahb_equiv_usage_bits = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                                            AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                                            AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
            if ((0 == (ahb_desc.usage & ahb_equiv_usage_bits)) || (0 == ahb_format_map_a2v.count(ahb_desc.format))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881",
                                "vkAllocateMemory: The AHardwareBuffer_Desc's format ( %u ) and/or usage ( 0x%" PRIx64
                                " ) are not compatible with Vulkan.",
                                ahb_desc.format, ahb_desc.usage);
            }
        }

        // Collect external buffer info
        VkPhysicalDeviceExternalBufferInfo pdebi = {};
        pdebi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
        pdebi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE];
        }
        if (AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT & ahb_desc.usage) {
            pdebi.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT];
        }
        VkExternalBufferProperties ext_buf_props = {};
        ext_buf_props.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;

        DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &pdebi, &ext_buf_props);

        // Collect external format info
        VkPhysicalDeviceExternalImageFormatInfo pdeifi = {};
        pdeifi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
        pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        VkPhysicalDeviceImageFormatInfo2 pdifi2 = {};
        pdifi2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        pdifi2.pNext = &pdeifi;
        if (0 < ahb_format_map_a2v.count(ahb_desc.format)) pdifi2.format = ahb_format_map_a2v[ahb_desc.format];
        pdifi2.type = VK_IMAGE_TYPE_2D;           // Seems likely
        pdifi2.tiling = VK_IMAGE_TILING_OPTIMAL;  // Ditto
        if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
            pdifi2.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE];
        }
        if (AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT & ahb_desc.usage) {
            pdifi2.usage |= ahb_usage_map_a2v[AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT];
        }
        if (AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP & ahb_desc.usage) {
            pdifi2.flags |= ahb_create_map_a2v[AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP];
        }
        if (AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT & ahb_desc.usage) {
            pdifi2.flags |= ahb_create_map_a2v[AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT];
        }

        VkExternalImageFormatProperties ext_img_fmt_props = {};
        ext_img_fmt_props.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
        VkImageFormatProperties2 ifp2 = {};
        ifp2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        ifp2.pNext = &ext_img_fmt_props;

        VkResult fmt_lookup_result = DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &pdifi2, &ifp2);

        //  If buffer is not NULL, Android hardware buffers must be supported for import, as reported by
        //  VkExternalImageFormatProperties or VkExternalBufferProperties.
        if (0 == (ext_buf_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
            if ((VK_SUCCESS != fmt_lookup_result) || (0 == (ext_img_fmt_props.externalMemoryProperties.externalMemoryFeatures &
                                                            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01880",
                                "vkAllocateMemory: Neither the VkExternalImageFormatProperties nor the VkExternalBufferProperties "
                                "structs for the AHardwareBuffer include the VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT flag.");
            }
        }

        // Retrieve buffer and format properties of the provided AHardwareBuffer
        VkAndroidHardwareBufferFormatPropertiesANDROID ahb_format_props = {};
        ahb_format_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
        VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
        ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
        ahb_props.pNext = &ahb_format_props;
        DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &ahb_props);

        // allocationSize must be the size returned by vkGetAndroidHardwareBufferPropertiesANDROID for the Android hardware buffer
        if (alloc_info->allocationSize != ahb_props.allocationSize) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkMemoryAllocateInfo-allocationSize-02383",
                            "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                            "struct, allocationSize (%" PRId64
                            ") does not match the AHardwareBuffer's reported allocationSize (%" PRId64 ").",
                            alloc_info->allocationSize, ahb_props.allocationSize);
        }

        // memoryTypeIndex must be one of those returned by vkGetAndroidHardwareBufferPropertiesANDROID for the AHardwareBuffer
        // Note: memoryTypeIndex is an index, memoryTypeBits is a bitmask
        uint32_t mem_type_bitmask = 1 << alloc_info->memoryTypeIndex;
        if (0 == (mem_type_bitmask & ahb_props.memoryTypeBits)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385",
                            "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                            "struct, memoryTypeIndex (%" PRId32
                            ") does not correspond to a bit set in AHardwareBuffer's reported "
                            "memoryTypeBits bitmask (0x%" PRIx32 ").",
                            alloc_info->memoryTypeIndex, ahb_props.memoryTypeBits);
        }

        // Checks for allocations without a dedicated allocation requirement
        if ((nullptr == mem_ded_alloc_info) || (VK_NULL_HANDLE == mem_ded_alloc_info->image)) {
            // the Android hardware buffer must have a format of AHARDWAREBUFFER_FORMAT_BLOB and a usage that includes
            // AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER
            if (((uint64_t)AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) ||
                (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkMemoryAllocateInfo-pNext-02384",
                    "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID "
                    "struct without a dedicated allocation requirement, while the AHardwareBuffer_Desc's format ( %u ) is not "
                    "AHARDWAREBUFFER_FORMAT_BLOB or usage (0x%" PRIx64 ") does not include AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER.",
                    ahb_desc.format, ahb_desc.usage);
            }
        } else {  // Checks specific to import with a dedicated allocation requirement
            const VkImageCreateInfo *ici = &(GetImageState(mem_ded_alloc_info->image)->createInfo);

            // The Android hardware buffer's usage must include at least one of AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT or
            // AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
            if (0 == (ahb_desc.usage & (AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE))) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkMemoryAllocateInfo-pNext-02386",
                    "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID and a "
                    "dedicated allocation requirement, while the AHardwareBuffer's usage (0x%" PRIx64
                    ") contains neither AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT nor AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE.",
                    ahb_desc.usage);
            }

            //  the format of image must be VK_FORMAT_UNDEFINED or the format returned by
            //  vkGetAndroidHardwareBufferPropertiesANDROID
            if ((ici->format != ahb_format_props.format) && (VK_FORMAT_UNDEFINED != ici->format)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-02387",
                                "vkAllocateMemory: VkMemoryAllocateInfo struct with chained "
                                "VkImportAndroidHardwareBufferInfoANDROID, the dedicated allocation image's "
                                "format (%s) is not VK_FORMAT_UNDEFINED and does not match the AHardwareBuffer's format (%s).",
                                string_VkFormat(ici->format), string_VkFormat(ahb_format_props.format));
            }

            // The width, height, and array layer dimensions of image and the Android hardwarebuffer must be identical
            if ((ici->extent.width != ahb_desc.width) || (ici->extent.height != ahb_desc.height) ||
                (ici->arrayLayers != ahb_desc.layers)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-02388",
                                "vkAllocateMemory: VkMemoryAllocateInfo struct with chained "
                                "VkImportAndroidHardwareBufferInfoANDROID, the dedicated allocation image's "
                                "width, height, and arrayLayers (%" PRId32 " %" PRId32 " %" PRId32
                                ") do not match those of the AHardwareBuffer (%" PRId32 " %" PRId32 " %" PRId32 ").",
                                ici->extent.width, ici->extent.height, ici->arrayLayers, ahb_desc.width, ahb_desc.height,
                                ahb_desc.layers);
            }

            // If the Android hardware buffer's usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE, the image must
            // have either a full mipmap chain or exactly 1 mip level.
            //
            // NOTE! The language of this VUID contradicts the language in the spec (1.1.93), which says "The
            // AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE flag does not correspond to a Vulkan image usage or creation flag. Instead,
            // its presence indicates that the Android hardware buffer contains a complete mipmap chain, and its absence indicates
            // that the Android hardware buffer contains only a single mip level."
            //
            // TODO: This code implements the VUID's meaning, but it seems likely that the spec text is actually correct.
            // Clarification requested.
            if ((ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) && (ici->mipLevels != 1) &&
                (ici->mipLevels != FullMipChainLevels(ici->extent))) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-02389",
                            "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID, "
                            "usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE but mipLevels (%" PRId32
                            ") is neither 1 nor full mip "
                            "chain levels (%" PRId32 ").",
                            ici->mipLevels, FullMipChainLevels(ici->extent));
            }

            // each bit set in the usage of image must be listed in AHardwareBuffer Usage Equivalence, and if there is a
            // corresponding AHARDWAREBUFFER_USAGE bit listed that bit must be included in the Android hardware buffer's
            // AHardwareBuffer_Desc::usage
            if (ici->usage &
                ~(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-02390",
                            "vkAllocateMemory: VkMemoryAllocateInfo struct with chained VkImportAndroidHardwareBufferInfoANDROID, "
                            "dedicated image usage bits include one or more with no AHardwareBuffer equivalent.");
            }

            bool illegal_usage = false;
            std::vector<VkImageUsageFlags> usages = {VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
            for (VkImageUsageFlags ubit : usages) {
                if (ici->usage & ubit) {
                    uint64_t ahb_usage = ahb_usage_map_v2a[ubit];
                    if (0 == (ahb_usage & ahb_desc.usage)) illegal_usage = true;
                }
            }
            if (illegal_usage) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-02390",
                                "vkAllocateMemory: VkMemoryAllocateInfo struct with chained "
                                "VkImportAndroidHardwareBufferInfoANDROID, one or more AHardwareBuffer usage bits equivalent to "
                                "the provided image's usage bits are missing from AHardwareBuffer_Desc.usage.");
            }
        }
    } else {  // Not an import
        if ((exp_mem_alloc_info) && (mem_ded_alloc_info) &&
            (0 != (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID & exp_mem_alloc_info->handleTypes)) &&
            (VK_NULL_HANDLE != mem_ded_alloc_info->image)) {
            // This is an Android HW Buffer export
            if (0 != alloc_info->allocationSize) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkMemoryAllocateInfo-pNext-01874",
                                "vkAllocateMemory: pNext chain indicates a dedicated Android Hardware Buffer export allocation, "
                                "but allocationSize is non-zero.");
            }
        } else {
            if (0 == alloc_info->allocationSize) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkMemoryAllocateInfo-pNext-01874",
                    "vkAllocateMemory: pNext chain does not indicate a dedicated export allocation, but allocationSize is 0.");
            };
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirements2ANDROID(const VkImage image) const {
    bool skip = false;

    const IMAGE_STATE *image_state = GetImageState(image);
    if (image_state->imported_ahb && (0 == image_state->GetBoundMemory().size())) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-VkImageMemoryRequirementsInfo2-image-01897",
                        "vkGetImageMemoryRequirements2: Attempt to query layout from an image created with "
                        "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType, which has not yet been "
                        "bound to memory.");
    }
    return skip;
}

static bool ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const debug_report_data *report_data,
                                                                   const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                   const VkImageFormatProperties2 *pImageFormatProperties) {
    bool skip = false;
    const VkAndroidHardwareBufferUsageANDROID *ahb_usage =
        lvl_find_in_chain<VkAndroidHardwareBufferUsageANDROID>(pImageFormatProperties->pNext);
    if (nullptr != ahb_usage) {
        const VkPhysicalDeviceExternalImageFormatInfo *pdeifi =
            lvl_find_in_chain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
        if ((nullptr == pdeifi) || (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID != pdeifi->handleType)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868",
                            "vkGetPhysicalDeviceImageFormatProperties2: pImageFormatProperties includes a chained "
                            "VkAndroidHardwareBufferUsageANDROID struct, but pImageFormatInfo does not include a chained "
                            "VkPhysicalDeviceExternalImageFormatInfo struct with handleType "
                            "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info) const {
    const VkExternalFormatANDROID *ext_format_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if ((nullptr != ext_format_android) && (0 != ext_format_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT, 0,
                           "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904",
                           "vkCreateSamplerYcbcrConversion[KHR]: CreateInfo format is not VK_FORMAT_UNDEFINED while "
                           "there is a chained VkExternalFormatANDROID struct.");
        }
    } else if (VK_FORMAT_UNDEFINED == create_info->format) {
        return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT, 0,
                       "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904",
                       "vkCreateSamplerYcbcrConversion[KHR]: CreateInfo format is VK_FORMAT_UNDEFINED with no chained "
                       "VkExternalFormatANDROID struct.");
    }
    return false;
}

#else  // !VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *alloc_info) const { return false; }

static bool ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const debug_report_data *report_data,
                                                                   const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                   const VkImageFormatProperties2 *pImageFormatProperties) {
    return false;
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info) const {
    return false;
}

bool CoreChecks::ValidateGetImageMemoryRequirements2ANDROID(const VkImage image) const { return false; }

#endif  // VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) const {
    bool skip = false;
    if (memObjMap.size() >= phys_dev_props.limits.maxMemoryAllocationCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        kVUIDUndefined, "Number of currently valid memory objects is not less than the maximum allowed (%u).",
                        phys_dev_props.limits.maxMemoryAllocationCount);
    }

    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateAllocateMemoryANDROID(pAllocateInfo);
    } else {
        if (0 == pAllocateInfo->allocationSize) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkMemoryAllocateInfo-allocationSize-00638", "vkAllocateMemory: allocationSize is 0.");
        };
    }

    auto chained_flags_struct = lvl_find_in_chain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    if (chained_flags_struct && chained_flags_struct->flags == VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT) {
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(chained_flags_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                                        HandleToUint64(device), "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
        skip |= ValidateDeviceMaskToZero(chained_flags_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                         HandleToUint64(device), "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    }
    // TODO: VUIDs ending in 00643, 00644, 00646, 00647, 01742, 01743, 01745, 00645, 00648, 01744
    return skip;
}

// For given obj node, if it is use, flag a validation error and return callback result, else return false
bool CoreChecks::ValidateObjectNotInUse(const BASE_NODE *obj_node, const VulkanTypedHandle &obj_struct, const char *caller_name,
                                        const char *error_code) const {
    if (disabled.object_in_use) return false;
    bool skip = false;
    if (obj_node->in_use.load()) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[obj_struct.type], obj_struct.handle,
                        error_code, "Cannot call %s on %s that is currently in use by a command buffer.", caller_name,
                        report_data->FormatHandle(obj_struct).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) const {
    const DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    const VulkanTypedHandle obj_struct(mem, kVulkanObjectTypeDeviceMemory);
    bool skip = false;
    if (mem_info) {
        skip |= ValidateObjectNotInUse(mem_info, obj_struct, "vkFreeMemory", "VUID-vkFreeMemory-memory-00677");
        for (const auto &obj : mem_info->obj_bindings) {
            log_msg(report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, get_debug_report_enum[obj.type], obj.handle,
                    kVUID_Core_MemTrack_FreedMemRef, "%s still has a reference to %s.", report_data->FormatHandle(obj).c_str(),
                    report_data->FormatHandle(mem_info->mem).c_str());
        }
    }
    return skip;
}

// Validate that given Map memory range is valid. This means that the memory should not already be mapped,
//  and that the size of the map range should be:
//  1. Not zero
//  2. Within the size of the memory allocation
bool CoreChecks::ValidateMapMemRange(const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize offset, VkDeviceSize size) const {
    bool skip = false;
    assert(mem_info);
    const auto mem = mem_info->mem;
    if (size == 0) {
        skip =
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, HandleToUint64(mem),
                    kVUID_Core_MemTrack_InvalidMap, "VkMapMemory: Attempting to map memory range of size zero");
    }

    // It is an application error to call VkMapMemory on an object that is already mapped
    if (mem_info->mapped_range.size != 0) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                       HandleToUint64(mem), kVUID_Core_MemTrack_InvalidMap,
                       "VkMapMemory: Attempting to map memory on an already-mapped %s.", report_data->FormatHandle(mem).c_str());
    }

    // Validate that offset + size is within object's allocationSize
    if (size == VK_WHOLE_SIZE) {
        if (offset >= mem_info->alloc_info.allocationSize) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                           HandleToUint64(mem), kVUID_Core_MemTrack_InvalidMap,
                           "Mapping Memory from 0x%" PRIx64 " to 0x%" PRIx64
                           " with size of VK_WHOLE_SIZE oversteps total array size 0x%" PRIx64,
                           offset, mem_info->alloc_info.allocationSize, mem_info->alloc_info.allocationSize);
        }
    } else {
        if ((offset + size) > mem_info->alloc_info.allocationSize) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                           HandleToUint64(mem), "VUID-vkMapMemory-size-00681",
                           "Mapping Memory from 0x%" PRIx64 " to 0x%" PRIx64 " oversteps total array size 0x%" PRIx64 ".", offset,
                           size + offset, mem_info->alloc_info.allocationSize);
        }
    }
    return skip;
}

// Guard value for pad data
static char NoncoherentMemoryFillValue = 0xb;

void CoreChecks::InitializeShadowMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        uint32_t index = mem_info->alloc_info.memoryTypeIndex;
        if (phys_dev_mem_props.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            mem_info->shadow_copy = 0;
        } else {
            if (size == VK_WHOLE_SIZE) {
                size = mem_info->alloc_info.allocationSize - offset;
            }
            mem_info->shadow_pad_size = phys_dev_props.limits.minMemoryMapAlignment;
            assert(SafeModulo(mem_info->shadow_pad_size, phys_dev_props.limits.minMemoryMapAlignment) == 0);
            // Ensure start of mapped region reflects hardware alignment constraints
            uint64_t map_alignment = phys_dev_props.limits.minMemoryMapAlignment;

            // From spec: (ppData - offset) must be aligned to at least limits::minMemoryMapAlignment.
            uint64_t start_offset = offset % map_alignment;
            // Data passed to driver will be wrapped by a guardband of data to detect over- or under-writes.
            mem_info->shadow_copy_base =
                malloc(static_cast<size_t>(2 * mem_info->shadow_pad_size + size + map_alignment + start_offset));

            mem_info->shadow_copy =
                reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(mem_info->shadow_copy_base) + map_alignment) &
                                         ~(map_alignment - 1)) +
                start_offset;
            assert(SafeModulo(reinterpret_cast<uintptr_t>(mem_info->shadow_copy) + mem_info->shadow_pad_size - start_offset,
                              map_alignment) == 0);

            memset(mem_info->shadow_copy, NoncoherentMemoryFillValue, static_cast<size_t>(2 * mem_info->shadow_pad_size + size));
            *ppData = static_cast<char *>(mem_info->shadow_copy) + mem_info->shadow_pad_size;
        }
    }
}

bool CoreChecks::PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                              uint64_t timeout) const {
    // Verify fence status of submitted fences
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; i++) {
        skip |= VerifyQueueStateToFence(pFences[i]);
    }
    return skip;
}

bool CoreChecks::ValidateGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue, const char *valid_qfi_vuid,
                                        const char *qfi_in_range_vuid) const {
    bool skip = false;

    skip |= ValidateDeviceQueueFamily(queueFamilyIndex, "vkGetDeviceQueue", "queueFamilyIndex", valid_qfi_vuid);
    const auto &queue_data = queue_family_index_map.find(queueFamilyIndex);
    if (queue_data != queue_family_index_map.end() && queue_data->second <= queueIndex) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        qfi_in_range_vuid,
                        "vkGetDeviceQueue: queueIndex (=%" PRIu32
                        ") is not less than the number of queues requested from queueFamilyIndex (=%" PRIu32
                        ") when the device was created (i.e. is not less than %" PRIu32 ").",
                        queueIndex, queueFamilyIndex, queue_data->second);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                               VkQueue *pQueue) const {
    return ValidateGetDeviceQueue(queueFamilyIndex, queueIndex, pQueue, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384",
                                  "VUID-vkGetDeviceQueue-queueIndex-00385");
}

bool CoreChecks::PreCallValidateQueueWaitIdle(VkQueue queue) const {
    const QUEUE_STATE *queue_state = GetQueueState(queue);
    return VerifyQueueStateToSeq(queue_state, queue_state->seq + queue_state->submissions.size());
}

bool CoreChecks::PreCallValidateDeviceWaitIdle(VkDevice device) const {
    bool skip = false;
    const auto &const_queue_map = queueMap;
    for (auto &queue : const_queue_map) {
        skip |= VerifyQueueStateToSeq(&queue.second, queue.second.seq + queue.second.submissions.size());
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) const {
    const FENCE_STATE *fence_node = GetFenceState(fence);
    bool skip = false;
    if (fence_node) {
        if (fence_node->scope == kSyncScopeInternal && fence_node->state == FENCE_INFLIGHT) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, HandleToUint64(fence),
                        "VUID-vkDestroyFence-fence-01120", "%s is in use.", report_data->FormatHandle(fence).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                 const VkAllocationCallbacks *pAllocator) const {
    const SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    const VulkanTypedHandle obj_struct(semaphore, kVulkanObjectTypeSemaphore);
    bool skip = false;
    if (sema_node) {
        skip |= ValidateObjectNotInUse(sema_node, obj_struct, "vkDestroySemaphore", "VUID-vkDestroySemaphore-semaphore-01137");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) const {
    const EVENT_STATE *event_state = GetEventState(event);
    const VulkanTypedHandle obj_struct(event, kVulkanObjectTypeEvent);
    bool skip = false;
    if (event_state) {
        skip |= ValidateObjectNotInUse(event_state, obj_struct, "vkDestroyEvent", "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                 const VkAllocationCallbacks *pAllocator) const {
    if (disabled.query_validation) return false;
    const QUERY_POOL_STATE *qp_state = GetQueryPoolState(queryPool);
    const VulkanTypedHandle obj_struct(queryPool, kVulkanObjectTypeQueryPool);
    bool skip = false;
    if (qp_state) {
        skip |= ValidateObjectNotInUse(qp_state, obj_struct, "vkDestroyQueryPool", "VUID-vkDestroyQueryPool-queryPool-00793");
    }
    return skip;
}

bool CoreChecks::ValidateGetQueryPoolResultsFlags(VkQueryPool queryPool, VkQueryResultFlags flags) const {
    bool skip = false;
    const auto query_pool_state = GetQueryPoolState(queryPool);
    if (query_pool_state) {
        if ((query_pool_state->createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, HandleToUint64(queryPool),
                "VUID-vkGetQueryPoolResults-queryType-00818",
                "%s was created with a queryType of VK_QUERY_TYPE_TIMESTAMP but flags contains VK_QUERY_RESULT_PARTIAL_BIT.",
                report_data->FormatHandle(queryPool).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetQueryPoolResultsQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    bool skip = false;
    QueryObject query_obj{queryPool, 0u};
    for (uint32_t i = 0; i < queryCount; ++i) {
        query_obj.query = firstQuery + i;
        if (queryToStateMap.count(query_obj) == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT,
                            HandleToUint64(queryPool), kVUID_Core_DrawState_InvalidQuery,
                            "vkGetQueryPoolResults() on %s and query %" PRIu32 ": unknown query",
                            report_data->FormatHandle(queryPool).c_str(), query_obj.query);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags) const {
    if (disabled.query_validation) return false;
    bool skip = false;
    skip |= ValidateQueryPoolStride("VUID-vkGetQueryPoolResults-flags-00814", "VUID-vkGetQueryPoolResults-flags-00815", stride,
                                    "dataSize", dataSize, flags);
    skip |= ValidateGetQueryPoolResultsFlags(queryPool, flags);
    skip |= ValidateGetQueryPoolResultsQueries(queryPool, firstQuery, queryCount);

    return skip;
}

bool CoreChecks::ValidateInsertMemoryRange(const VulkanTypedHandle &typed_handle, const DEVICE_MEMORY_STATE *mem_info,
                                           VkDeviceSize memoryOffset, const VkMemoryRequirements &memRequirements, bool is_linear,
                                           const char *api_name) const {
    bool skip = false;

    if (memoryOffset >= mem_info->alloc_info.allocationSize) {
        const char *error_code = nullptr;
        if (typed_handle.type == kVulkanObjectTypeBuffer) {
            error_code = "VUID-vkBindBufferMemory-memoryOffset-01031";
        } else if (typed_handle.type == kVulkanObjectTypeImage) {
            error_code = "VUID-vkBindImageMemory-memoryOffset-01046";
        } else if (typed_handle.type == kVulkanObjectTypeAccelerationStructureNV) {
            error_code = "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-02451";
        } else {
            // Unsupported object type
            assert(false);
        }

        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                       HandleToUint64(mem_info->mem), error_code,
                       "In %s, attempting to bind %s to %s, memoryOffset=0x%" PRIxLEAST64
                       " must be less than the memory allocation size 0x%" PRIxLEAST64 ".",
                       api_name, report_data->FormatHandle(mem_info->mem).c_str(), report_data->FormatHandle(typed_handle).c_str(),
                       memoryOffset, mem_info->alloc_info.allocationSize);
    }

    return skip;
}

bool CoreChecks::ValidateInsertImageMemoryRange(VkImage image, const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                const VkMemoryRequirements &mem_reqs, bool is_linear, const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(image, kVulkanObjectTypeImage), mem_info, mem_offset, mem_reqs, is_linear,
                                     api_name);
}

bool CoreChecks::ValidateInsertBufferMemoryRange(VkBuffer buffer, const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                 const VkMemoryRequirements &mem_reqs, const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer), mem_info, mem_offset, mem_reqs, true,
                                     api_name);
}

bool CoreChecks::ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const DEVICE_MEMORY_STATE *mem_info,
                                                                VkDeviceSize mem_offset, const VkMemoryRequirements &mem_reqs,
                                                                const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV), mem_info, mem_offset,
                                     mem_reqs, true, api_name);
}

bool CoreChecks::ValidateMemoryTypes(const DEVICE_MEMORY_STATE *mem_info, const uint32_t memory_type_bits, const char *funcName,
                                     const char *msgCode) const {
    bool skip = false;
    if (((1 << mem_info->alloc_info.memoryTypeIndex) & memory_type_bits) == 0) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                       HandleToUint64(mem_info->mem), msgCode,
                       "%s(): MemoryRequirements->memoryTypeBits (0x%X) for this object type are not compatible with the memory "
                       "type (0x%X) of %s.",
                       funcName, memory_type_bits, mem_info->alloc_info.memoryTypeIndex,
                       report_data->FormatHandle(mem_info->mem).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                          const char *api_name) const {
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);

    bool skip = false;
    if (buffer_state) {
        // Track objects tied to memory
        uint64_t buffer_handle = HandleToUint64(buffer);
        const VulkanTypedHandle obj_struct(buffer, kVulkanObjectTypeBuffer);
        skip = ValidateSetMemBinding(mem, obj_struct, api_name);

        // Validate bound memory range information
        const auto mem_info = GetDevMemState(mem);
        if (mem_info) {
            skip |= ValidateInsertBufferMemoryRange(buffer, mem_info, memoryOffset, buffer_state->requirements, api_name);
            skip |= ValidateMemoryTypes(mem_info, buffer_state->requirements.memoryTypeBits, api_name,
                                        "VUID-vkBindBufferMemory-memory-01035");
        }

        // Validate memory requirements alignment
        if (SafeModulo(memoryOffset, buffer_state->requirements.alignment) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, buffer_handle,
                            "VUID-vkBindBufferMemory-memoryOffset-01036",
                            "%s: memoryOffset is 0x%" PRIxLEAST64
                            " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                            ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                            api_name, memoryOffset, buffer_state->requirements.alignment);
        }

        if (mem_info) {
            // Validate memory requirements size
            if (buffer_state->requirements.size > (mem_info->alloc_info.allocationSize - memoryOffset)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, buffer_handle,
                                "VUID-vkBindBufferMemory-size-01037",
                                "%s: memory size minus memoryOffset is 0x%" PRIxLEAST64
                                " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                                ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                                api_name, mem_info->alloc_info.allocationSize - memoryOffset, buffer_state->requirements.size);
            }

            // Validate dedicated allocation
            if (mem_info->is_dedicated && ((mem_info->dedicated_buffer != buffer) || (memoryOffset != 0))) {
                // TODO: Add vkBindBufferMemory2KHR error message when added to spec.
                auto validation_error = kVUIDUndefined;
                if (strcmp(api_name, "vkBindBufferMemory()") == 0) {
                    validation_error = "VUID-vkBindBufferMemory-memory-01508";
                }
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, buffer_handle,
                                validation_error,
                                "%s: for dedicated %s, VkMemoryDedicatedAllocateInfoKHR::buffer %s must be equal "
                                "to %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                api_name, report_data->FormatHandle(mem).c_str(),
                                report_data->FormatHandle(mem_info->dedicated_buffer).c_str(),
                                report_data->FormatHandle(buffer).c_str(), memoryOffset);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                                 VkDeviceSize memoryOffset) const {
    const char *api_name = "vkBindBufferMemory()";
    return ValidateBindBufferMemory(buffer, mem, memoryOffset, api_name);
}

bool CoreChecks::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                  const VkBindBufferMemoryInfoKHR *pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset, api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfoKHR *pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset, api_name);
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2 *pInfo) const {
    bool skip = false;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateGetImageMemoryRequirements2ANDROID(pInfo->image);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                            VkMemoryRequirements2 *pMemoryRequirements) const {
    return ValidateGetImageMemoryRequirements2(pInfo);
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                               VkMemoryRequirements2 *pMemoryRequirements) const {
    return ValidateGetImageMemoryRequirements2(pInfo);
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        VkImageFormatProperties2 *pImageFormatProperties) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(report_data, pImageFormatInfo, pImageFormatProperties);
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                           VkImageFormatProperties2 *pImageFormatProperties) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(report_data, pImageFormatInfo, pImageFormatProperties);
    return skip;
}

bool CoreChecks::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                const VkAllocationCallbacks *pAllocator) const {
    const PIPELINE_STATE *pipeline_state = GetPipelineState(pipeline);
    const VulkanTypedHandle obj_struct(pipeline, kVulkanObjectTypePipeline);
    bool skip = false;
    if (pipeline_state) {
        skip |= ValidateObjectNotInUse(pipeline_state, obj_struct, "vkDestroyPipeline", "VUID-vkDestroyPipeline-pipeline-00765");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) const {
    const SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
    const VulkanTypedHandle obj_struct(sampler, kVulkanObjectTypeSampler);
    bool skip = false;
    if (sampler_state) {
        skip |= ValidateObjectNotInUse(sampler_state, obj_struct, "vkDestroySampler", "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks *pAllocator) const {
    const DESCRIPTOR_POOL_STATE *desc_pool_state = GetDescriptorPoolState(descriptorPool);
    const VulkanTypedHandle obj_struct(descriptorPool, kVulkanObjectTypeDescriptorPool);
    bool skip = false;
    if (desc_pool_state) {
        skip |= ValidateObjectNotInUse(desc_pool_state, obj_struct, "vkDestroyDescriptorPool",
                                       "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    }
    return skip;
}

// Verify cmdBuffer in given cb_node is not in global in-flight set, and return skip result
//  If this is a secondary command buffer, then make sure its primary is also in-flight
//  If primary is not in-flight, then remove secondary from global in-flight set
// This function is only valid at a point when cmdBuffer is being reset or freed
bool CoreChecks::CheckCommandBufferInFlight(const CMD_BUFFER_STATE *cb_node, const char *action, const char *error_code) const {
    bool skip = false;
    if (cb_node->in_use.load()) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), error_code, "Attempt to %s %s which is in use.", action,
                        report_data->FormatHandle(cb_node->commandBuffer).c_str());
    }
    return skip;
}

// Iterate over all cmdBuffers in given commandPool and verify that each is not in use
bool CoreChecks::CheckCommandBuffersInFlight(const COMMAND_POOL_STATE *pPool, const char *action, const char *error_code) const {
    bool skip = false;
    for (auto cmd_buffer : pPool->commandBuffers) {
        skip |= CheckCommandBufferInFlight(GetCBState(cmd_buffer), action, error_code);
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer *pCommandBuffers) const {
    bool skip = false;
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        const auto *cb_node = GetCBState(pCommandBuffers[i]);
        // Delete CB information structure, and remove from commandBufferMap
        if (cb_node) {
            skip |= CheckCommandBufferInFlight(cb_node, "free", "VUID-vkFreeCommandBuffers-pCommandBuffers-00047");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) const {
    return ValidateDeviceQueueFamily(pCreateInfo->queueFamilyIndex, "vkCreateCommandPool", "pCreateInfo->queueFamilyIndex",
                                     "VUID-vkCreateCommandPool-queueFamilyIndex-01937");
}

bool CoreChecks::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) const {
    if (disabled.query_validation) return false;
    bool skip = false;
    if (pCreateInfo && pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        if (!enabled_features.core.pipelineStatisticsQuery) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, 0,
                            "VUID-VkQueryPoolCreateInfo-queryType-00791",
                            "Query pool with type VK_QUERY_TYPE_PIPELINE_STATISTICS created on a device with "
                            "VkDeviceCreateInfo.pEnabledFeatures.pipelineStatisticsQuery == VK_FALSE.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks *pAllocator) const {
    const COMMAND_POOL_STATE *cp_state = GetCommandPoolState(commandPool);
    bool skip = false;
    if (cp_state) {
        // Verify that command buffers in pool are complete (not in-flight)
        skip |= CheckCommandBuffersInFlight(cp_state, "destroy command pool with", "VUID-vkDestroyCommandPool-commandPool-00041");
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
    const auto *command_pool_state = GetCommandPoolState(commandPool);
    return CheckCommandBuffersInFlight(command_pool_state, "reset command pool with", "VUID-vkResetCommandPool-commandPool-00040");
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) const {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        const auto pFence = GetFenceState(pFences[i]);
        if (pFence && pFence->scope == kSyncScopeInternal && pFence->state == FENCE_INFLIGHT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,
                            HandleToUint64(pFences[i]), "VUID-vkResetFences-pFences-01123", "%s is in use.",
                            report_data->FormatHandle(pFences[i]).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks *pAllocator) const {
    const FRAMEBUFFER_STATE *framebuffer_state = GetFramebufferState(framebuffer);
    const VulkanTypedHandle obj_struct(framebuffer, kVulkanObjectTypeFramebuffer);
    bool skip = false;
    if (framebuffer_state) {
        skip |= ValidateObjectNotInUse(framebuffer_state, obj_struct, "vkDestroyFramebuffer",
                                       "VUID-vkDestroyFramebuffer-framebuffer-00892");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                  const VkAllocationCallbacks *pAllocator) const {
    const RENDER_PASS_STATE *rp_state = GetRenderPassState(renderPass);
    const VulkanTypedHandle obj_struct(renderPass, kVulkanObjectTypeRenderPass);
    bool skip = false;
    if (rp_state) {
        skip |= ValidateObjectNotInUse(rp_state, obj_struct, "vkDestroyRenderPass", "VUID-vkDestroyRenderPass-renderPass-00873");
    }
    return skip;
}

// Access helper functions for external modules
VkFormatProperties CoreChecks::GetPDFormatProperties(const VkFormat format) const {
    VkFormatProperties format_properties;
    DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
    return format_properties;
}

bool CoreChecks::ValidatePipelineVertexDivisors(std::vector<std::shared_ptr<PIPELINE_STATE>> const &pipe_state_vec,
                                                const uint32_t count, const VkGraphicsPipelineCreateInfo *pipe_cis) const {
    bool skip = false;
    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;

    for (uint32_t i = 0; i < count; i++) {
        auto pvids_ci = lvl_find_in_chain<VkPipelineVertexInputDivisorStateCreateInfoEXT>(pipe_cis[i].pVertexInputState->pNext);
        if (nullptr == pvids_ci) continue;

        const PIPELINE_STATE *pipe_state = pipe_state_vec[i].get();
        for (uint32_t j = 0; j < pvids_ci->vertexBindingDivisorCount; j++) {
            const VkVertexInputBindingDivisorDescriptionEXT *vibdd = &(pvids_ci->pVertexBindingDivisors[j]);
            if (vibdd->binding >= device_limits->maxVertexInputBindings) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkVertexInputBindingDivisorDescriptionEXT-binding-01869",
                    "vkCreateGraphicsPipelines(): Pipeline[%1u] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] binding index of (%1u) exceeds device maxVertexInputBindings (%1u).",
                    i, j, vibdd->binding, device_limits->maxVertexInputBindings);
            }
            if (vibdd->divisor > phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkVertexInputBindingDivisorDescriptionEXT-divisor-01870",
                    "vkCreateGraphicsPipelines(): Pipeline[%1u] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] divisor of (%1u) exceeds extension maxVertexAttribDivisor (%1u).",
                    i, j, vibdd->divisor, phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor);
            }
            if ((0 == vibdd->divisor) && !enabled_features.vtx_attrib_divisor_features.vertexAttributeInstanceRateZeroDivisor) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateZeroDivisor-02228",
                    "vkCreateGraphicsPipelines(): Pipeline[%1u] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] divisor must not be 0 when vertexAttributeInstanceRateZeroDivisor feature is not "
                    "enabled.",
                    i, j);
            }
            if ((1 != vibdd->divisor) && !enabled_features.vtx_attrib_divisor_features.vertexAttributeInstanceRateDivisor) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateDivisor-02229",
                    "vkCreateGraphicsPipelines(): Pipeline[%1u] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] divisor (%1u) must be 1 when vertexAttributeInstanceRateDivisor feature is not "
                    "enabled.",
                    i, j, vibdd->divisor);
            }

            // Find the corresponding binding description and validate input rate setting
            bool failed_01871 = true;
            for (size_t k = 0; k < pipe_state->vertex_binding_descriptions_.size(); k++) {
                if ((vibdd->binding == pipe_state->vertex_binding_descriptions_[k].binding) &&
                    (VK_VERTEX_INPUT_RATE_INSTANCE == pipe_state->vertex_binding_descriptions_[k].inputRate)) {
                    failed_01871 = false;
                    break;
                }
            }
            if (failed_01871) {  // Description not found, or has incorrect inputRate value
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871",
                    "vkCreateGraphicsPipelines(): Pipeline[%1u] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] specifies binding index (%1u), but that binding index's "
                    "VkVertexInputBindingDescription.inputRate member is not VK_VERTEX_INPUT_RATE_INSTANCE.",
                    i, j, vibdd->binding);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        void *cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                     pPipelines, cgpl_state_data);
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);

    for (uint32_t i = 0; i < count; i++) {
        skip |= ValidatePipelineLocked(cgpl_state->pipe_state, i);
    }

    for (uint32_t i = 0; i < count; i++) {
        skip |= ValidatePipelineUnlocked(cgpl_state->pipe_state[i].get(), i);
    }

    if (device_extensions.vk_ext_vertex_attribute_divisor) {
        skip |= ValidatePipelineVertexDivisors(cgpl_state->pipe_state, count, pCreateInfos);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       void *ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                    pPipelines, ccpl_state_data);

    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        // TODO: Add Compute Pipeline Verification
        skip |= ValidateComputePipeline(ccpl_state->pipe_state.back().get());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                         pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        skip |= ValidateRayTracingPipelineNV(crtpl_state->pipe_state[i].get());
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo,
                                                                   uint32_t *pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR *pProperties) const {
    bool skip = false;

    if (!enabled_features.pipeline_exe_props_features.pipelineExecutableInfo) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableProperties-03270",
                        "vkGetPipelineExecutablePropertiesKHR called when pipelineExecutableInfo feature is not enabled.");
    }

    return skip;
}

bool CoreChecks::ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo) const {
    bool skip = false;

    if (!enabled_features.pipeline_exe_props_features.pipelineExecutableInfo) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272",
                        "vkGetPipelineExecutableStatisticsKHR called when pipelineExecutableInfo feature is not enabled.");
    }

    VkPipelineInfoKHR pi = {};
    pi.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
    pi.pipeline = pExecutableInfo->pipeline;

    // We could probably cache this instead of fetching it every time
    uint32_t executableCount = 0;
    DispatchGetPipelineExecutablePropertiesKHR(device, &pi, &executableCount, NULL);

    if (pExecutableInfo->executableIndex >= executableCount) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                    HandleToUint64(pExecutableInfo->pipeline), "VUID-VkPipelineExecutableInfoKHR-executableIndex-03275",
                    "VkPipelineExecutableInfo::executableIndex (%1u) must be less than the number of executables associated with "
                    "the pipeline (%1u) as returned by vkGetPipelineExecutablePropertiessKHR",
                    pExecutableInfo->executableIndex, executableCount);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                                   uint32_t *pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR *pStatistics) const {
    bool skip = ValidatePipelineExecutableInfo(device, pExecutableInfo);

    const PIPELINE_STATE *pipeline_state = GetPipelineState(pExecutableInfo->pipeline);
    if (!(pipeline_state->getPipelineCreateFlags() & VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                        HandleToUint64(pExecutableInfo->pipeline), "VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274",
                        "vkGetPipelineExecutableStatisticsKHR called on a pipeline created without the "
                        "VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR *pStatistics) const {
    bool skip = ValidatePipelineExecutableInfo(device, pExecutableInfo);

    const PIPELINE_STATE *pipeline_state = GetPipelineState(pExecutableInfo->pipeline);
    if (!(pipeline_state->getPipelineCreateFlags() & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                        HandleToUint64(pExecutableInfo->pipeline),
                        "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278",
                        "vkGetPipelineExecutableInternalRepresentationsKHR called on a pipeline created without the "
                        "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorSetLayout *pSetLayout) const {
    return cvdescriptorset::ValidateDescriptorSetLayoutCreateInfo(
        report_data, pCreateInfo, device_extensions.vk_khr_push_descriptor, phys_dev_ext_props.max_push_descriptors,
        device_extensions.vk_ext_descriptor_indexing, &enabled_features.descriptor_indexing, &enabled_features.inline_uniform_block,
        &phys_dev_ext_props.inline_uniform_block_props, &device_extensions);
}

// Used by CreatePipelineLayout and CmdPushConstants.
// Note that the index argument is optional and only used by CreatePipelineLayout.
bool CoreChecks::ValidatePushConstantRange(const uint32_t offset, const uint32_t size, const char *caller_name,
                                           uint32_t index = 0) const {
    if (disabled.push_constant_range) return false;
    uint32_t const maxPushConstantsSize = phys_dev_props.limits.maxPushConstantsSize;
    bool skip = false;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if ((offset >= maxPushConstantsSize) || (size > maxPushConstantsSize - offset)) {
        // This is a pain just to adapt the log message to the caller, but better to sort it out only when there is a problem.
        if (0 == strcmp(caller_name, "vkCreatePipelineLayout()")) {
            if (offset >= maxPushConstantsSize) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPushConstantRange-offset-00294",
                    "%s call has push constants index %u with offset %u that exceeds this device's maxPushConstantSize of %u.",
                    caller_name, index, offset, maxPushConstantsSize);
            }
            if (size > maxPushConstantsSize - offset) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkPushConstantRange-size-00298",
                                "%s call has push constants index %u with offset %u and size %u that exceeds this device's "
                                "maxPushConstantSize of %u.",
                                caller_name, index, offset, size, maxPushConstantsSize);
            }
        } else if (0 == strcmp(caller_name, "vkCmdPushConstants()")) {
            if (offset >= maxPushConstantsSize) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-vkCmdPushConstants-offset-00370",
                    "%s call has push constants index %u with offset %u that exceeds this device's maxPushConstantSize of %u.",
                    caller_name, index, offset, maxPushConstantsSize);
            }
            if (size > maxPushConstantsSize - offset) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-vkCmdPushConstants-size-00371",
                                "%s call has push constants index %u with offset %u and size %u that exceeds this device's "
                                "maxPushConstantSize of %u.",
                                caller_name, index, offset, size, maxPushConstantsSize);
            }
        } else {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_DrawState_InternalError, "%s caller not supported.", caller_name);
        }
    }
    // size needs to be non-zero and a multiple of 4.
    if ((size == 0) || ((size & 0x3) != 0)) {
        if (0 == strcmp(caller_name, "vkCreatePipelineLayout()")) {
            if (size == 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkPushConstantRange-size-00296",
                                "%s call has push constants index %u with size %u. Size must be greater than zero.", caller_name,
                                index, size);
            }
            if (size & 0x3) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkPushConstantRange-size-00297",
                                "%s call has push constants index %u with size %u. Size must be a multiple of 4.", caller_name,
                                index, size);
            }
        } else if (0 == strcmp(caller_name, "vkCmdPushConstants()")) {
            if (size == 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-vkCmdPushConstants-size-arraylength",
                                "%s call has push constants index %u with size %u. Size must be greater than zero.", caller_name,
                                index, size);
            }
            if (size & 0x3) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-vkCmdPushConstants-size-00369",
                                "%s call has push constants index %u with size %u. Size must be a multiple of 4.", caller_name,
                                index, size);
            }
        } else {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_DrawState_InternalError, "%s caller not supported.", caller_name);
        }
    }
    // offset needs to be a multiple of 4.
    if ((offset & 0x3) != 0) {
        if (0 == strcmp(caller_name, "vkCreatePipelineLayout()")) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPushConstantRange-offset-00295",
                            "%s call has push constants index %u with offset %u. Offset must be a multiple of 4.", caller_name,
                            index, offset);
        } else if (0 == strcmp(caller_name, "vkCmdPushConstants()")) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-vkCmdPushConstants-offset-00368",
                            "%s call has push constants with offset %u. Offset must be a multiple of 4.", caller_name, offset);
        } else {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_DrawState_InternalError, "%s caller not supported.", caller_name);
        }
    }
    return skip;
}

enum DSL_DESCRIPTOR_GROUPS {
    DSL_TYPE_SAMPLERS = 0,
    DSL_TYPE_UNIFORM_BUFFERS,
    DSL_TYPE_STORAGE_BUFFERS,
    DSL_TYPE_SAMPLED_IMAGES,
    DSL_TYPE_STORAGE_IMAGES,
    DSL_TYPE_INPUT_ATTACHMENTS,
    DSL_TYPE_INLINE_UNIFORM_BLOCK,
    DSL_NUM_DESCRIPTOR_GROUPS
};

// Used by PreCallValidateCreatePipelineLayout.
// Returns an array of size DSL_NUM_DESCRIPTOR_GROUPS of the maximum number of descriptors used in any single pipeline stage
std::valarray<uint32_t> GetDescriptorCountMaxPerStage(
    const DeviceFeatures *enabled_features,
    const std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    // Identify active pipeline stages
    std::vector<VkShaderStageFlags> stage_flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   VK_SHADER_STAGE_COMPUTE_BIT};
    if (enabled_features->core.geometryShader) {
        stage_flags.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if (enabled_features->core.tessellationShader) {
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }

    // Allow iteration over enum values
    std::vector<DSL_DESCRIPTOR_GROUPS> dsl_groups = {
        DSL_TYPE_SAMPLERS,       DSL_TYPE_UNIFORM_BUFFERS,   DSL_TYPE_STORAGE_BUFFERS,     DSL_TYPE_SAMPLED_IMAGES,
        DSL_TYPE_STORAGE_IMAGES, DSL_TYPE_INPUT_ATTACHMENTS, DSL_TYPE_INLINE_UNIFORM_BLOCK};

    // Sum by layouts per stage, then pick max of stages per type
    std::valarray<uint32_t> max_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // max descriptor sum among all pipeline stages
    for (auto stage : stage_flags) {
        std::valarray<uint32_t> stage_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // per-stage sums
        for (auto dsl : set_layouts) {
            if (skip_update_after_bind &&
                (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
                continue;
            }

            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (0 != (stage & binding->stageFlags) && binding->descriptorCount > 0) {
                    switch (binding->descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_UNIFORM_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_STORAGE_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_STORAGE_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            stage_sum[DSL_TYPE_INPUT_ATTACHMENTS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                            // count one block per binding. descriptorCount is number of bytes
                            stage_sum[DSL_TYPE_INLINE_UNIFORM_BLOCK]++;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for (auto type : dsl_groups) {
            max_sum[type] = std::max(stage_sum[type], max_sum[type]);
        }
    }
    return max_sum;
}

// Used by PreCallValidateCreatePipelineLayout.
// Returns a map indexed by VK_DESCRIPTOR_TYPE_* enum of the summed descriptors by type.
// Note: descriptors only count against the limit once even if used by multiple stages.
std::map<uint32_t, uint32_t> GetDescriptorSum(
    const std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    std::map<uint32_t, uint32_t> sum_by_type;
    for (auto dsl : set_layouts) {
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorCount > 0) {
                if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
                    // count one block per binding. descriptorCount is number of bytes
                    sum_by_type[binding->descriptorType]++;
                } else {
                    sum_by_type[binding->descriptorType] += binding->descriptorCount;
                }
            }
        }
    }
    return sum_by_type;
}

bool CoreChecks::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkPipelineLayout *pPipelineLayout) const {
    bool skip = false;

    // Validate layout count against device physical limit
    if (pCreateInfo->setLayoutCount > phys_dev_props.limits.maxBoundDescriptorSets) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286",
                        "vkCreatePipelineLayout(): setLayoutCount (%d) exceeds physical device maxBoundDescriptorSets limit (%d).",
                        pCreateInfo->setLayoutCount, phys_dev_props.limits.maxBoundDescriptorSets);
    }

    // Validate Push Constant ranges
    uint32_t i, j;
    for (i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        skip |= ValidatePushConstantRange(pCreateInfo->pPushConstantRanges[i].offset, pCreateInfo->pPushConstantRanges[i].size,
                                          "vkCreatePipelineLayout()", i);
        if (0 == pCreateInfo->pPushConstantRanges[i].stageFlags) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPushConstantRange-stageFlags-requiredbitmask",
                            "vkCreatePipelineLayout() call has no stageFlags set.");
        }
    }

    // As of 1.0.28, there is a VU that states that a stage flag cannot appear more than once in the list of push constant ranges.
    for (i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
        for (j = i + 1; j < pCreateInfo->pushConstantRangeCount; ++j) {
            if (0 != (pCreateInfo->pPushConstantRanges[i].stageFlags & pCreateInfo->pPushConstantRanges[j].stageFlags)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                                "vkCreatePipelineLayout() Duplicate stage flags found in ranges %d and %d.", i, j);
            }
        }
    }

    // Early-out
    if (skip) return skip;

    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts(pCreateInfo->setLayoutCount, nullptr);
    unsigned int push_descriptor_set_count = 0;
    {
        for (i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            set_layouts[i] = GetDescriptorSetLayoutShared(pCreateInfo->pSetLayouts[i]);
            if (set_layouts[i]->IsPushDescriptor()) ++push_descriptor_set_count;
        }
    }

    if (push_descriptor_set_count > 1) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00293",
                        "vkCreatePipelineLayout() Multiple push descriptor sets found.");
    }

    // Max descriptors by type, within a single pipeline stage
    std::valarray<uint32_t> max_descriptors_per_stage = GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, true);
    // Samplers
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLERS] > phys_dev_props.limits.maxPerStageDescriptorSamplers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00287",
                        "vkCreatePipelineLayout(): max per-stage sampler bindings count (%d) exceeds device "
                        "maxPerStageDescriptorSamplers limit (%d).",
                        max_descriptors_per_stage[DSL_TYPE_SAMPLERS], phys_dev_props.limits.maxPerStageDescriptorSamplers);
    }

    // Uniform buffers
    if (max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorUniformBuffers) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00288",
                    "vkCreatePipelineLayout(): max per-stage uniform buffer bindings count (%d) exceeds device "
                    "maxPerStageDescriptorUniformBuffers limit (%d).",
                    max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS], phys_dev_props.limits.maxPerStageDescriptorUniformBuffers);
    }

    // Storage buffers
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorStorageBuffers) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00289",
                    "vkCreatePipelineLayout(): max per-stage storage buffer bindings count (%d) exceeds device "
                    "maxPerStageDescriptorStorageBuffers limit (%d).",
                    max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS], phys_dev_props.limits.maxPerStageDescriptorStorageBuffers);
    }

    // Sampled images
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorSampledImages) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00290",
                    "vkCreatePipelineLayout(): max per-stage sampled image bindings count (%d) exceeds device "
                    "maxPerStageDescriptorSampledImages limit (%d).",
                    max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES], phys_dev_props.limits.maxPerStageDescriptorSampledImages);
    }

    // Storage images
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorStorageImages) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00291",
                    "vkCreatePipelineLayout(): max per-stage storage image bindings count (%d) exceeds device "
                    "maxPerStageDescriptorStorageImages limit (%d).",
                    max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES], phys_dev_props.limits.maxPerStageDescriptorStorageImages);
    }

    // Input attachments
    if (max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS] > phys_dev_props.limits.maxPerStageDescriptorInputAttachments) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01676",
                        "vkCreatePipelineLayout(): max per-stage input attachment bindings count (%d) exceeds device "
                        "maxPerStageDescriptorInputAttachments limit (%d).",
                        max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS],
                        phys_dev_props.limits.maxPerStageDescriptorInputAttachments);
    }

    // Inline uniform blocks
    if (max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
        phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-descriptorType-02214",
                        "vkCreatePipelineLayout(): max per-stage inline uniform block bindings count (%d) exceeds device "
                        "maxPerStageDescriptorInlineUniformBlocks limit (%d).",
                        max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                        phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks);
    }

    // Total descriptors by type
    //
    std::map<uint32_t, uint32_t> sum_all_stages = GetDescriptorSum(set_layouts, true);
    // Samplers
    uint32_t sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLER] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSamplers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01677",
                        "vkCreatePipelineLayout(): sum of sampler bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetSamplers limit (%d).",
                        sum, phys_dev_props.limits.maxDescriptorSetSamplers);
    }

    // Uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] > phys_dev_props.limits.maxDescriptorSetUniformBuffers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01678",
                        "vkCreatePipelineLayout(): sum of uniform buffer bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetUniformBuffers limit (%d).",
                        sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER], phys_dev_props.limits.maxDescriptorSetUniformBuffers);
    }

    // Dynamic uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01679",
                        "vkCreatePipelineLayout(): sum of dynamic uniform buffer bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetUniformBuffersDynamic limit (%d).",
                        sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                        phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic);
    }

    // Storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] > phys_dev_props.limits.maxDescriptorSetStorageBuffers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01680",
                        "vkCreatePipelineLayout(): sum of storage buffer bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetStorageBuffers limit (%d).",
                        sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER], phys_dev_props.limits.maxDescriptorSetStorageBuffers);
    }

    // Dynamic storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01681",
                        "vkCreatePipelineLayout(): sum of dynamic storage buffer bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetStorageBuffersDynamic limit (%d).",
                        sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                        phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic);
    }

    //  Sampled images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
          sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSampledImages) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682",
                        "vkCreatePipelineLayout(): sum of sampled image bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetSampledImages limit (%d).",
                        sum, phys_dev_props.limits.maxDescriptorSetSampledImages);
    }

    //  Storage images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetStorageImages) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01683",
                        "vkCreatePipelineLayout(): sum of storage image bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetStorageImages limit (%d).",
                        sum, phys_dev_props.limits.maxDescriptorSetStorageImages);
    }

    // Input attachments
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] > phys_dev_props.limits.maxDescriptorSetInputAttachments) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01684",
                    "vkCreatePipelineLayout(): sum of input attachment bindings among all stages (%d) exceeds device "
                    "maxDescriptorSetInputAttachments limit (%d).",
                    sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT], phys_dev_props.limits.maxDescriptorSetInputAttachments);
    }

    // Inline uniform blocks
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT] >
        phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkPipelineLayoutCreateInfo-descriptorType-02216",
                        "vkCreatePipelineLayout(): sum of inline uniform block bindings among all stages (%d) exceeds device "
                        "maxDescriptorSetInlineUniformBlocks limit (%d).",
                        sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT],
                        phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks);
    }

    if (device_extensions.vk_ext_descriptor_indexing) {
        // XXX TODO: replace with correct VU messages

        // Max descriptors by type, within a single pipeline stage
        std::valarray<uint32_t> max_descriptors_per_stage_update_after_bind =
            GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, false);
        // Samplers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022",
                            "vkCreatePipelineLayout(): max per-stage sampler bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindSamplers limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03023",
                            "vkCreatePipelineLayout(): max per-stage uniform buffer bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindUniformBuffers limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
        }

        // Storage buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03024",
                            "vkCreatePipelineLayout(): max per-stage storage buffer bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindStorageBuffers limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
        }

        // Sampled images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025",
                            "vkCreatePipelineLayout(): max per-stage sampled image bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindSampledImages limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages);
        }

        // Storage images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03026",
                            "vkCreatePipelineLayout(): max per-stage storage image bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindStorageImages limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS] >
            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-03027",
                            "vkCreatePipelineLayout(): max per-stage input attachment bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindInputAttachments limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS],
                            phys_dev_ext_props.descriptor_indexing_props.maxPerStageDescriptorUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
            phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-02215",
                            "vkCreatePipelineLayout(): max per-stage inline uniform block bindings count (%d) exceeds device "
                            "maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks limit (%d).",
                            max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                            phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
        }

        // Total descriptors by type, summed across all pipeline stages
        //
        std::map<uint32_t, uint32_t> sum_all_stages_update_after_bind = GetDescriptorSum(set_layouts, false);
        // Samplers
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
        if (sum > phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindSamplers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036",
                            "vkCreatePipelineLayout(): sum of sampler bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindSamplers limit (%d).",
                            sum, phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] >
            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037",
                            "vkCreatePipelineLayout(): sum of uniform buffer bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindUniformBuffers limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER],
                            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindUniformBuffers);
        }

        // Dynamic uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038",
                            "vkCreatePipelineLayout(): sum of dynamic uniform buffer bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindUniformBuffersDynamic limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
        }

        // Storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] >
            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039",
                            "vkCreatePipelineLayout(): sum of storage buffer bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindStorageBuffers limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER],
                            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageBuffers);
        }

        // Dynamic storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040",
                            "vkCreatePipelineLayout(): sum of dynamic storage buffer bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindStorageBuffersDynamic limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
        }

        //  Sampled images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
        if (sum > phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindSampledImages) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041",
                            "vkCreatePipelineLayout(): sum of sampled image bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindSampledImages limit (%d).",
                            sum, phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindSampledImages);
        }

        //  Storage images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
        if (sum > phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageImages) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042",
                            "vkCreatePipelineLayout(): sum of storage image bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindStorageImages limit (%d).",
                            sum, phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] >
            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindInputAttachments) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043",
                            "vkCreatePipelineLayout(): sum of input attachment bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindInputAttachments limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT],
                            phys_dev_ext_props.descriptor_indexing_props.maxDescriptorSetUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT] >
            phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkPipelineLayoutCreateInfo-descriptorType-02217",
                            "vkCreatePipelineLayout(): sum of inline uniform block bindings among all stages (%d) exceeds device "
                            "maxDescriptorSetUpdateAfterBindInlineUniformBlocks limit (%d).",
                            sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT],
                            phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags) const {
    // Make sure sets being destroyed are not currently in-use
    if (disabled.idle_descriptor_set) return false;
    bool skip = false;
    const DESCRIPTOR_POOL_STATE *pPool = GetDescriptorPoolState(descriptorPool);
    if (pPool != nullptr) {
        for (auto ds : pPool->sets) {
            if (ds && ds->in_use.load()) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                                HandleToUint64(descriptorPool), "VUID-vkResetDescriptorPool-descriptorPool-00313",
                                "It is invalid to call vkResetDescriptorPool() with descriptor sets in use by a command buffer.");
                if (skip) break;
            }
        }
    }
    return skip;
}

// Ensure the pool contains enough descriptors and descriptor sets to satisfy
// an allocation request. Fills common_data with the total number of descriptors of each type required,
// as well as DescriptorSetLayout ptrs used for later update.
bool CoreChecks::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets, void *ads_state_data) const {
    StateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, ads_state_data);

    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    // All state checks for AllocateDescriptorSets is done in single function
    return ValidateAllocateDescriptorSets(pAllocateInfo, ads_state);
}

bool CoreChecks::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                   const VkDescriptorSet *pDescriptorSets) const {
    // Make sure that no sets being destroyed are in-flight
    bool skip = false;
    // First make sure sets being destroyed are not currently in-use
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateIdleDescriptorSet(pDescriptorSets[i], "vkFreeDescriptorSets");
        }
    }
    const DESCRIPTOR_POOL_STATE *pool_state = GetDescriptorPoolState(descriptorPool);
    if (pool_state && !(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT & pool_state->createInfo.flags)) {
        // Can't Free from a NON_FREE pool
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                        HandleToUint64(descriptorPool), "VUID-vkFreeDescriptorSets-descriptorPool-00312",
                        "It is invalid to call vkFreeDescriptorSets() with a pool created without setting "
                        "VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet *pDescriptorCopies) const {
    // First thing to do is perform map look-ups.
    // NOTE : UpdateDescriptorSets is somewhat unique in that it's operating on a number of DescriptorSets
    //  so we can't just do a single map look-up up-front, but do them individually in functions below

    // Now make call(s) that validate state, but don't perform state updates in this function
    // Note, here DescriptorSets is unique in that we don't yet have an instance. Using a helper function in the
    //  namespace which will parse params and make calls into specific class instances
    return ValidateUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies,
                                        "vkUpdateDescriptorSets()");
}

bool CoreChecks::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    if (cb_state->in_use.load()) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkBeginCommandBuffer-commandBuffer-00049",
                        "Calling vkBeginCommandBuffer() on active %s before it has completed. You must check "
                        "command buffer fence before this call.",
                        report_data->FormatHandle(commandBuffer).c_str());
    }
    if (cb_state->createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        // Secondary Command Buffer
        const VkCommandBufferInheritanceInfo *pInfo = pBeginInfo->pInheritanceInfo;
        if (!pInfo) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkBeginCommandBuffer-commandBuffer-00051",
                            "vkBeginCommandBuffer(): Secondary %s must have inheritance info.",
                            report_data->FormatHandle(commandBuffer).c_str());
        } else {
            if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                assert(pInfo->renderPass);
                const auto *framebuffer = GetFramebufferState(pInfo->framebuffer);
                if (framebuffer) {
                    if (framebuffer->createInfo.renderPass != pInfo->renderPass) {
                        const auto *render_pass = GetRenderPassState(pInfo->renderPass);
                        // renderPass that framebuffer was created with must be compatible with local renderPass
                        skip |= ValidateRenderPassCompatibility("framebuffer", framebuffer->rp_state.get(), "command buffer",
                                                                render_pass, "vkBeginCommandBuffer()",
                                                                "VUID-VkCommandBufferBeginInfo-flags-00055");
                    }
                }
            }
            if ((pInfo->occlusionQueryEnable == VK_FALSE || enabled_features.core.occlusionQueryPrecise == VK_FALSE) &&
                (pInfo->queryFlags & VK_QUERY_CONTROL_PRECISE_BIT)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkBeginCommandBuffer-commandBuffer-00052",
                                "vkBeginCommandBuffer(): Secondary %s must not have VK_QUERY_CONTROL_PRECISE_BIT if "
                                "occulusionQuery is disabled or the device does not support precise occlusion queries.",
                                report_data->FormatHandle(commandBuffer).c_str());
            }
        }
        if (pInfo && pInfo->renderPass != VK_NULL_HANDLE) {
            const auto *renderPass = GetRenderPassState(pInfo->renderPass);
            if (renderPass) {
                if (pInfo->subpass >= renderPass->createInfo.subpassCount) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-VkCommandBufferBeginInfo-flags-00054",
                                    "vkBeginCommandBuffer(): Secondary %s must have a subpass index (%d) that is "
                                    "less than the number of subpasses (%d).",
                                    report_data->FormatHandle(commandBuffer).c_str(), pInfo->subpass,
                                    renderPass->createInfo.subpassCount);
                }
            }
        }
    }
    if (CB_RECORDING == cb_state->state) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkBeginCommandBuffer-commandBuffer-00049",
                        "vkBeginCommandBuffer(): Cannot call Begin on %s in the RECORDING state. Must first call "
                        "vkEndCommandBuffer().",
                        report_data->FormatHandle(commandBuffer).c_str());
    } else if (CB_RECORDED == cb_state->state || CB_INVALID_COMPLETE == cb_state->state) {
        VkCommandPool cmdPool = cb_state->createInfo.commandPool;
        const auto *pPool = cb_state->command_pool.get();
        if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pPool->createFlags)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkBeginCommandBuffer-commandBuffer-00050",
                            "Call to vkBeginCommandBuffer() on %s attempts to implicitly reset cmdBuffer created from "
                            "%s that does NOT have the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set.",
                            report_data->FormatHandle(commandBuffer).c_str(), report_data->FormatHandle(cmdPool).c_str());
        }
    }
    auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(
            chained_device_group_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(commandBuffer),
            "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
        skip |=
            ValidateDeviceMaskToZero(chained_device_group_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                     HandleToUint64(commandBuffer), "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    }
    return skip;
}

bool CoreChecks::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    if ((VK_COMMAND_BUFFER_LEVEL_PRIMARY == cb_state->createInfo.level) ||
        !(cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
        // This needs spec clarification to update valid usage, see comments in PR:
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/165
        skip |= InsideRenderPass(cb_state, "vkEndCommandBuffer()", "VUID-vkEndCommandBuffer-commandBuffer-00060");
    }

    skip |= ValidateCmd(cb_state, CMD_ENDCOMMANDBUFFER, "vkEndCommandBuffer()");
    for (auto query : cb_state->activeQueries) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkEndCommandBuffer-commandBuffer-00061",
                        "Ending command buffer with in progress query: %s, query %d.",
                        report_data->FormatHandle(query.pool).c_str(), query.query);
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const {
    bool skip = false;
    const CMD_BUFFER_STATE *pCB = GetCBState(commandBuffer);
    if (!pCB) return false;
    VkCommandPool cmdPool = pCB->createInfo.commandPool;
    const auto *pPool = pCB->command_pool.get();

    if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pPool->createFlags)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkResetCommandBuffer-commandBuffer-00046",
                        "Attempt to reset %s created from %s that does NOT have the "
                        "VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set.",
                        report_data->FormatHandle(commandBuffer).c_str(), report_data->FormatHandle(cmdPool).c_str());
    }
    skip |= CheckCommandBufferInFlight(pCB, "reset", "VUID-vkResetCommandBuffer-commandBuffer-00045");

    return skip;
}

static const char *GetPipelineTypeName(VkPipelineBindPoint pipelineBindPoint) {
    switch (pipelineBindPoint) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            return "graphics";
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return "compute";
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_NV:
            return "ray-tracing";
        default:
            return "unknown";
    }
}

bool CoreChecks::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBindPipeline()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdBindPipeline-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_BINDPIPELINE, "vkCmdBindPipeline()");
    static const std::map<VkPipelineBindPoint, std::string> bindpoint_errors = {
        std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdBindPipeline-pipelineBindPoint-00777"),
        std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdBindPipeline-pipelineBindPoint-00778"),
        std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, "VUID-vkCmdBindPipeline-pipelineBindPoint-02391")};

    skip |= ValidatePipelineBindPoint(cb_state, pipelineBindPoint, "vkCmdBindPipeline()", bindpoint_errors);

    const auto *pipeline_state = GetPipelineState(pipeline);
    assert(pipeline_state);

    const auto &pipeline_state_bind_point = pipeline_state->getPipelineType();

    if (pipelineBindPoint != pipeline_state_bind_point) {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBindPipeline-pipelineBindPoint-00779",
                            "Cannot bind a pipeline of type %s to the graphics pipeline bind point",
                            GetPipelineTypeName(pipeline_state_bind_point));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBindPipeline-pipelineBindPoint-00780",
                            "Cannot bind a pipeline of type %s to the compute pipeline bind point",
                            GetPipelineTypeName(pipeline_state_bind_point));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBindPipeline-pipelineBindPoint-02392",
                            "Cannot bind a pipeline of type %s to the ray-tracing pipeline bind point",
                            GetPipelineTypeName(pipeline_state_bind_point));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport *pViewports) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip =
        ValidateCmdQueueFlags(cb_state, "vkCmdSetViewport()", VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewport-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETVIEWPORT, "vkCmdSetViewport()");
    if (cb_state->static_status & CBSTATUS_VIEWPORT_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewport-None-01221",
                        "vkCmdSetViewport(): pipeline was created without VK_DYNAMIC_STATE_VIEWPORT flag.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D *pScissors) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip =
        ValidateCmdQueueFlags(cb_state, "vkCmdSetScissor()", VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetScissor-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETSCISSOR, "vkCmdSetScissor()");
    if (cb_state->static_status & CBSTATUS_SCISSOR_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetScissor-None-00590",
                        "vkCmdSetScissor(): pipeline was created without VK_DYNAMIC_STATE_SCISSOR flag..");
    }
    return skip;
}

void ValidationStateTracker ::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                         uint32_t scissorCount, const VkRect2D *pScissors) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->scissorMask |= ((1u << scissorCount) - 1u) << firstScissor;
    cb_state->status |= CBSTATUS_SCISSOR_SET;
}

bool CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetExclusiveScissorNV()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETEXCLUSIVESCISSORNV, "vkCmdSetExclusiveScissorNV()");
    if (cb_state->static_status & CBSTATUS_EXCLUSIVE_SCISSOR_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-None-02032",
                        "vkCmdSetExclusiveScissorNV(): pipeline was created without VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV flag.");
    }

    if (!enabled_features.exclusive_scissor.exclusiveScissor) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetExclusiveScissorNV-None-02031",
                        "vkCmdSetExclusiveScissorNV: The exclusiveScissor feature is disabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBindShadingRateImageNV()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdBindShadingRateImageNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_BINDSHADINGRATEIMAGENV, "vkCmdBindShadingRateImageNV()");

    if (!enabled_features.shading_rate_image.shadingRateImage) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBindShadingRateImageNV-None-02058",
                        "vkCmdBindShadingRateImageNV: The shadingRateImage feature is disabled.");
    }

    if (imageView != VK_NULL_HANDLE) {
        const auto view_state = GetImageViewState(imageView);
        auto &ivci = view_state->create_info;

        if (!view_state || (ivci.viewType != VK_IMAGE_VIEW_TYPE_2D && ivci.viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                            HandleToUint64(imageView), "VUID-vkCmdBindShadingRateImageNV-imageView-02059",
                            "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must be a valid "
                            "VkImageView handle of type VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
        }

        if (view_state && ivci.format != VK_FORMAT_R8_UINT) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, HandleToUint64(imageView),
                "VUID-vkCmdBindShadingRateImageNV-imageView-02060",
                "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must have a format of VK_FORMAT_R8_UINT.");
        }

        const VkImageCreateInfo *ici = view_state ? &GetImageState(view_state->create_info.image)->createInfo : nullptr;
        if (ici && !(ici->usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                            HandleToUint64(imageView), "VUID-vkCmdBindShadingRateImageNV-imageView-02061",
                            "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, the image must have been "
                            "created with VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV set.");
        }

        if (view_state) {
            const auto image_state = GetImageState(view_state->create_info.image);
            bool hit_error = false;

            // XXX TODO: While the VUID says "each subresource", only the base mip level is
            // actually used. Since we don't have an existing convenience function to iterate
            // over all mip levels, just don't bother with non-base levels.
            const VkImageSubresourceRange &range = view_state->create_info.subresourceRange;
            VkImageSubresourceLayers subresource = {range.aspectMask, range.baseMipLevel, range.baseArrayLayer, range.layerCount};

            if (image_state) {
                skip |= VerifyImageLayout(cb_state, image_state, subresource, imageLayout, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
                                          "vkCmdCopyImage()", "VUID-vkCmdBindShadingRateImageNV-imageLayout-02063",
                                          "VUID-vkCmdBindShadingRateImageNV-imageView-02062", &hit_error);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV *pShadingRatePalettes) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetViewportShadingRatePaletteNV()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_SETVIEWPORTSHADINGRATEPALETTENV, "vkCmdSetViewportShadingRatePaletteNV()");

    if (!enabled_features.shading_rate_image.shadingRateImage) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-None-02064",
                        "vkCmdSetViewportShadingRatePaletteNV: The shadingRateImage feature is disabled.");
    }

    if (cb_state->static_status & CBSTATUS_SHADING_RATE_PALETTE_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportShadingRatePaletteNV-None-02065",
                        "vkCmdSetViewportShadingRatePaletteNV(): pipeline was created without "
                        "VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV flag.");
    }

    for (uint32_t i = 0; i < viewportCount; ++i) {
        auto *palette = &pShadingRatePalettes[i];
        if (palette->shadingRatePaletteEntryCount == 0 ||
            palette->shadingRatePaletteEntryCount > phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                HandleToUint64(commandBuffer), "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071",
                "vkCmdSetViewportShadingRatePaletteNV: shadingRatePaletteEntryCount must be between 1 and shadingRatePaletteSize.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkDebugReportObjectTypeEXT object_type,
                                             uint64_t object_handle, const char *func_name) const {
    bool skip = false;

    const BUFFER_STATE *vb_state = GetBufferState(triangles.vertexData);
    if (vb_state != nullptr && vb_state->binding.size <= triangles.vertexOffset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-vertexOffset-02428", "%s", func_name);
    }

    const BUFFER_STATE *ib_state = GetBufferState(triangles.indexData);
    if (ib_state != nullptr && ib_state->binding.size <= triangles.indexOffset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-indexOffset-02431", "%s", func_name);
    }

    const BUFFER_STATE *td_state = GetBufferState(triangles.transformData);
    if (td_state != nullptr && td_state->binding.size <= triangles.transformOffset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryTrianglesNV-transformOffset-02437", "%s", func_name);
    }

    return skip;
}

bool CoreChecks::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, VkDebugReportObjectTypeEXT object_type,
                                        uint64_t object_handle, const char *func_name) const {
    bool skip = false;

    const BUFFER_STATE *aabb_state = GetBufferState(aabbs.aabbData);
    if (aabb_state != nullptr && aabb_state->binding.size > 0 && aabb_state->binding.size <= aabbs.offset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
                        "VUID-VkGeometryAABBNV-offset-02439", "%s", func_name);
    }

    return skip;
}

bool CoreChecks::ValidateGeometryNV(const VkGeometryNV &geometry, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                    const char *func_name) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip = ValidateGeometryTrianglesNV(geometry.geometry.triangles, object_type, object_handle, func_name);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip = ValidateGeometryAABBNV(geometry.geometry.aabbs, object_type, object_handle, func_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureNV *pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo != nullptr && pCreateInfo->info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pCreateInfo->info.geometryCount; i++) {
            skip |= ValidateGeometryNV(pCreateInfo->info.pGeometries[i], VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                       HandleToUint64(device), "vkCreateAccelerationStructureNV():");
        }
    }
    return skip;
}

bool CoreChecks::ValidateBindAccelerationStructureMemoryNV(VkDevice device,
                                                           const VkBindAccelerationStructureMemoryInfoNV &info) const {
    bool skip = false;

    const ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(info.accelerationStructure);
    if (!as_state) {
        return skip;
    }
    uint64_t as_handle = HandleToUint64(info.accelerationStructure);
    if (!as_state->GetBoundMemory().empty()) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                    as_handle, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-02450",
                    "vkBindAccelerationStructureMemoryNV(): accelerationStructure must not already be backed by a memory object.");
    }

    // Validate bound memory range information
    const auto mem_info = GetDevMemState(info.memory);
    if (mem_info) {
        skip |= ValidateInsertAccelerationStructureMemoryRange(info.accelerationStructure, mem_info, info.memoryOffset,
                                                               as_state->memory_requirements.memoryRequirements,
                                                               "vkBindAccelerationStructureMemoryNV()");
        skip |= ValidateMemoryTypes(mem_info, as_state->memory_requirements.memoryRequirements.memoryTypeBits,
                                    "vkBindAccelerationStructureMemoryNV()",
                                    "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-02593");
    }

    // Validate memory requirements alignment
    if (SafeModulo(info.memoryOffset, as_state->memory_requirements.memoryRequirements.alignment) != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                        as_handle, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-02594",
                        "vkBindAccelerationStructureMemoryNV(): memoryOffset is 0x%" PRIxLEAST64
                        " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                        ", returned from a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure"
                        "and type of VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV.",
                        info.memoryOffset, as_state->memory_requirements.memoryRequirements.alignment);
    }

    if (mem_info) {
        // Validate memory requirements size
        if (as_state->memory_requirements.memoryRequirements.size > (mem_info->alloc_info.allocationSize - info.memoryOffset)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, as_handle,
                            "VUID-VkBindAccelerationStructureMemoryInfoNV-size-02595",
                            "vkBindAccelerationStructureMemoryNV(): memory size minus memoryOffset is 0x%" PRIxLEAST64
                            " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                            ", returned from a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure"
                            "and type of VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV.",
                            mem_info->alloc_info.allocationSize - info.memoryOffset,
                            as_state->memory_requirements.memoryRequirements.size);
        }
    }

    return skip;
}
bool CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV *pBindInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        skip |= ValidateBindAccelerationStructureMemoryNV(device, pBindInfos[i]);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void *pData) const {
    bool skip = false;

    const ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(accelerationStructure);
    if (as_state != nullptr) {
        // TODO: update the fake VUID below once the real one is generated.
        skip = ValidateMemoryIsBoundToAccelerationStructure(
            as_state, "vkGetAccelerationStructureHandleNV",
            "UNASSIGNED-vkGetAccelerationStructureHandleNV-accelerationStructure-XXXX");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBuildAccelerationStructureNV()", VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_BUILDACCELERATIONSTRUCTURENV, "vkCmdBuildAccelerationStructureNV()");

    if (pInfo != nullptr && pInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            skip |= ValidateGeometryNV(pInfo->pGeometries[i], VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                                       HandleToUint64(device), "vkCmdBuildAccelerationStructureNV():");
        }
    }

    if (pInfo != nullptr && pInfo->geometryCount > phys_dev_ext_props.ray_tracing_props.maxGeometryCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241",
                        "vkCmdBuildAccelerationStructureNV(): geometryCount [%d] must be less than or equal to "
                        "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.",
                        pInfo->geometryCount);
    }

    const ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
    const ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);
    const BUFFER_STATE *scratch_buffer_state = GetBufferState(scratch);

    if (dst_as_state != nullptr && pInfo != nullptr) {
        if (dst_as_state->create_info.info.type != pInfo->type) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                            "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::type"
                            "[%s] must be identical to build info VkAccelerationStructureInfoNV::type [%s].",
                            string_VkAccelerationStructureTypeNV(dst_as_state->create_info.info.type),
                            string_VkAccelerationStructureTypeNV(pInfo->type));
        }
        if (dst_as_state->create_info.info.flags != pInfo->flags) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                            "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::flags"
                            "[0x%X] must be identical to build info VkAccelerationStructureInfoNV::flags [0x%X].",
                            dst_as_state->create_info.info.flags, pInfo->flags);
        }
        if (dst_as_state->create_info.info.instanceCount < pInfo->instanceCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                            "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::instanceCount "
                            "[%d] must be greater than or equal to build info VkAccelerationStructureInfoNV::instanceCount [%d].",
                            dst_as_state->create_info.info.instanceCount, pInfo->instanceCount);
        }
        if (dst_as_state->create_info.info.geometryCount < pInfo->geometryCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                            "vkCmdBuildAccelerationStructureNV(): create info VkAccelerationStructureInfoNV::geometryCount"
                            "[%d] must be greater than or equal to build info VkAccelerationStructureInfoNV::geometryCount [%d].",
                            dst_as_state->create_info.info.geometryCount, pInfo->geometryCount);
        } else {
            for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
                const VkGeometryDataNV &create_geometry_data = dst_as_state->create_info.info.pGeometries[i].geometry;
                const VkGeometryDataNV &build_geometry_data = pInfo->pGeometries[i].geometry;
                if (create_geometry_data.triangles.vertexCount < build_geometry_data.triangles.vertexCount) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                        "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.triangles.vertexCount [%d]"
                        "must be greater than or equal to build info pGeometries[%d].geometry.triangles.vertexCount [%d].",
                        i, create_geometry_data.triangles.vertexCount, i, build_geometry_data.triangles.vertexCount);
                    break;
                }
                if (create_geometry_data.triangles.indexCount < build_geometry_data.triangles.indexCount) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                        "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.triangles.indexCount [%d]"
                        "must be greater than or equal to build info pGeometries[%d].geometry.triangles.indexCount [%d].",
                        i, create_geometry_data.triangles.indexCount, i, build_geometry_data.triangles.indexCount);
                    break;
                }
                if (create_geometry_data.aabbs.numAABBs < build_geometry_data.aabbs.numAABBs) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-dst-02488",
                                    "vkCmdBuildAccelerationStructureNV(): create info pGeometries[%d].geometry.aabbs.numAABBs [%d]"
                                    "must be greater than or equal to build info pGeometries[%d].geometry.aabbs.numAABBs [%d].",
                                    i, create_geometry_data.aabbs.numAABBs, i, build_geometry_data.aabbs.numAABBs);
                    break;
                }
            }
        }
    }

    if (dst_as_state != nullptr) {
        skip |= ValidateMemoryIsBoundToAccelerationStructure(
            dst_as_state, "vkCmdBuildAccelerationStructureNV()",
            "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkAccelerationStructureNV");
    }

    if (update == VK_TRUE) {
        if (src == VK_NULL_HANDLE) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-update-02489",
                            "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, src must not be VK_NULL_HANDLE.");
        } else {
            if (src_as_state == nullptr || !src_as_state->built ||
                !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-update-02489",
                                "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, src must have been built before "
                                "with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV set in "
                                "VkAccelerationStructureInfoNV::flags.");
            }
        }
        if (dst_as_state != nullptr && !dst_as_state->update_scratch_memory_requirements_checked) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                            HandleToUint64(dst), kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery,
                            "vkCmdBuildAccelerationStructureNV(): Updating %s but vkGetAccelerationStructureMemoryRequirementsNV() "
                            "has not been called for update scratch memory.",
                            report_data->FormatHandle(dst_as_state->acceleration_structure).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr &&
            dst_as_state->update_scratch_memory_requirements.memoryRequirements.size >
                (scratch_buffer_state->binding.size - scratchOffset)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-update-02492",
                            "vkCmdBuildAccelerationStructureNV(): If update is VK_TRUE, The size member of the "
                            "VkMemoryRequirements structure returned from a call to "
                            "vkGetAccelerationStructureMemoryRequirementsNV with "
                            "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                            "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                            "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV must be less than "
                            "or equal to the size of scratch minus scratchOffset");
        }
    } else {
        if (dst_as_state != nullptr && !dst_as_state->build_scratch_memory_requirements_checked) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                            HandleToUint64(dst), kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery,
                            "vkCmdBuildAccelerationStructureNV(): Assigning scratch buffer to %s but "
                            "vkGetAccelerationStructureMemoryRequirementsNV() has not been called for scratch memory.",
                            report_data->FormatHandle(dst_as_state->acceleration_structure).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr &&
            dst_as_state->build_scratch_memory_requirements.memoryRequirements.size >
                (scratch_buffer_state->binding.size - scratchOffset)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-update-02491",
                            "vkCmdBuildAccelerationStructureNV(): If update is VK_FALSE, The size member of the "
                            "VkMemoryRequirements structure returned from a call to "
                            "vkGetAccelerationStructureMemoryRequirementsNV with "
                            "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                            "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                            "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV must be less than "
                            "or equal to the size of scratch minus scratchOffset");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeNV mode) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdCopyAccelerationStructureNV()", VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_COPYACCELERATIONSTRUCTURENV, "vkCmdCopyAccelerationStructureNV()");

    const ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
    const ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);

    if (dst_as_state != nullptr) {
        skip |= ValidateMemoryIsBoundToAccelerationStructure(
            dst_as_state, "vkCmdBuildAccelerationStructureNV()",
            "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkAccelerationStructureNV");
    }

    if (mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV) {
        if (src_as_state != nullptr &&
            (!src_as_state->built || !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdCopyAccelerationStructureNV-src-02497",
                            "vkCmdCopyAccelerationStructureNV(): src must have been built with "
                            "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV if mode is "
                            "VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator) const {
    const ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(accelerationStructure);
    const VulkanTypedHandle obj_struct(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV);
    bool skip = false;
    if (as_state) {
        skip |= ValidateObjectNotInUse(as_state, obj_struct, "vkDestroyAccelerationStructureNV",
                                       "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-02442");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
                                                         const VkViewportWScalingNV *pViewportWScalings) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdmdSetViewportWScalingNV()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdmdSetViewportWScalingNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_SETVIEWPORTWSCALINGNV, "vkCmdmdSetViewportWScalingNV()");

    if (cb_state->static_status & CBSTATUS_VIEWPORT_W_SCALING_SET) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(commandBuffer), "VUID-vkCmdSetViewportWScalingNV-None-01322",
                    "vkCmdmdSetViewportWScalingNV(): pipeline was created without VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV flag.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetLineWidth()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetLineWidth-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETLINEWIDTH, "vkCmdSetLineWidth()");

    if (cb_state->static_status & CBSTATUS_LINE_WIDTH_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetLineWidth-None-00787",
                        "vkCmdSetLineWidth called but pipeline was created without VK_DYNAMIC_STATE_LINE_WIDTH flag.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetLineStippleEXT()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetLineStippleEXT-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETLINESTIPPLEEXT, "vkCmdSetLineStippleEXT()");

    if (cb_state->static_status & CBSTATUS_LINE_STIPPLE_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetLineStippleEXT-None-02775",
                        "vkCmdSetLineStippleEXT called but pipeline was created without VK_DYNAMIC_STATE_LINE_STIPPLE_EXT flag.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetDepthBias()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetDepthBias-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETDEPTHBIAS, "vkCmdSetDepthBias()");
    if (cb_state->static_status & CBSTATUS_DEPTH_BIAS_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetDepthBias-None-00789",
                        "vkCmdSetDepthBias(): pipeline was created without VK_DYNAMIC_STATE_DEPTH_BIAS flag..");
    }
    if ((depthBiasClamp != 0.0) && (!enabled_features.core.depthBiasClamp)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetDepthBias-depthBiasClamp-00790",
                        "vkCmdSetDepthBias(): the depthBiasClamp device feature is disabled: the depthBiasClamp parameter must "
                        "be set to 0.0.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetBlendConstants()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetBlendConstants-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETBLENDCONSTANTS, "vkCmdSetBlendConstants()");
    if (cb_state->static_status & CBSTATUS_BLEND_CONSTANTS_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetBlendConstants-None-00612",
                        "vkCmdSetBlendConstants(): pipeline was created without VK_DYNAMIC_STATE_BLEND_CONSTANTS flag..");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetDepthBounds()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetDepthBounds-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETDEPTHBOUNDS, "vkCmdSetDepthBounds()");
    if (cb_state->static_status & CBSTATUS_DEPTH_BOUNDS_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetDepthBounds-None-00599",
                        "vkCmdSetDepthBounds(): pipeline was created without VK_DYNAMIC_STATE_DEPTH_BOUNDS flag..");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetStencilCompareMask()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetStencilCompareMask-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETSTENCILCOMPAREMASK, "vkCmdSetStencilCompareMask()");
    if (cb_state->static_status & CBSTATUS_STENCIL_READ_MASK_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetStencilCompareMask-None-00602",
                        "vkCmdSetStencilCompareMask(): pipeline was created without VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK flag..");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetStencilWriteMask()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetStencilWriteMask-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETSTENCILWRITEMASK, "vkCmdSetStencilWriteMask()");
    if (cb_state->static_status & CBSTATUS_STENCIL_WRITE_MASK_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetStencilWriteMask-None-00603",
                        "vkCmdSetStencilWriteMask(): pipeline was created without VK_DYNAMIC_STATE_STENCIL_WRITE_MASK flag..");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetStencilReference()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdSetStencilReference-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETSTENCILREFERENCE, "vkCmdSetStencilReference()");
    if (cb_state->static_status & CBSTATUS_STENCIL_REFERENCE_SET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdSetStencilReference-None-00604",
                        "vkCmdSetStencilReference(): pipeline was created without VK_DYNAMIC_STATE_STENCIL_REFERENCE flag..");
    }
    return skip;
}

static bool ValidateDynamicOffsetAlignment(const debug_report_data *report_data, const VkDescriptorSetLayoutBinding *binding,
                                           VkDescriptorType test_type, VkDeviceSize alignment, const uint32_t *pDynamicOffsets,
                                           const char *err_msg, const char *limit_name, uint32_t *offset_idx) {
    bool skip = false;
    if (binding->descriptorType == test_type) {
        const auto end_idx = *offset_idx + binding->descriptorCount;
        for (uint32_t current_idx = *offset_idx; current_idx < end_idx; current_idx++) {
            if (SafeModulo(pDynamicOffsets[current_idx], alignment) != 0) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, 0, err_msg,
                    "vkCmdBindDescriptorSets(): pDynamicOffsets[%d] is %d but must be a multiple of device limit %s 0x%" PRIxLEAST64
                    ".",
                    current_idx, pDynamicOffsets[current_idx], limit_name, alignment);
            }
        }
        *offset_idx = end_idx;
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                      const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                      const uint32_t *pDynamicOffsets) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdBindDescriptorSets()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdBindDescriptorSets-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_BINDDESCRIPTORSETS, "vkCmdBindDescriptorSets()");
    // Track total count of dynamic descriptor types to make sure we have an offset for each one
    uint32_t total_dynamic_descriptors = 0;
    string error_string = "";

    const auto *pipeline_layout = GetPipelineLayout(layout);
    for (uint32_t set_idx = 0; set_idx < setCount; set_idx++) {
        const cvdescriptorset::DescriptorSet *descriptor_set = GetSetNode(pDescriptorSets[set_idx]);
        if (descriptor_set) {
            // Verify that set being bound is compatible with overlapping setLayout of pipelineLayout
            if (!VerifySetLayoutCompatibility(report_data, descriptor_set, pipeline_layout, set_idx + firstSet, error_string)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            HandleToUint64(pDescriptorSets[set_idx]), "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358",
                            "descriptorSet #%u being bound is not compatible with overlapping descriptorSetLayout at index %u of "
                            "%s due to: %s.",
                            set_idx, set_idx + firstSet, report_data->FormatHandle(layout).c_str(), error_string.c_str());
            }

            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            if (set_dynamic_descriptor_count) {
                // First make sure we won't overstep bounds of pDynamicOffsets array
                if ((total_dynamic_descriptors + set_dynamic_descriptor_count) > dynamicOffsetCount) {
                    // Test/report this here, such that we don't run past the end of pDynamicOffsets in the else clause
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                HandleToUint64(pDescriptorSets[set_idx]), "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359",
                                "descriptorSet #%u (%s) requires %u dynamicOffsets, but only %u dynamicOffsets are left in "
                                "pDynamicOffsets array. There must be one dynamic offset for each dynamic descriptor being bound.",
                                set_idx, report_data->FormatHandle(pDescriptorSets[set_idx]).c_str(),
                                descriptor_set->GetDynamicDescriptorCount(), (dynamicOffsetCount - total_dynamic_descriptors));
                    // Set the number found to the maximum to prevent duplicate messages, or subsquent descriptor sets from
                    // testing against the "short tail" we're skipping below.
                    total_dynamic_descriptors = dynamicOffsetCount;
                } else {  // Validate dynamic offsets and Dynamic Offset Minimums
                    uint32_t cur_dyn_offset = total_dynamic_descriptors;
                    const auto dsl = descriptor_set->GetLayout();
                    const auto binding_count = dsl->GetBindingCount();
                    const auto &limits = phys_dev_props.limits;
                    for (uint32_t binding_idx = 0; binding_idx < binding_count; binding_idx++) {
                        const auto *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                        skip |= ValidateDynamicOffsetAlignment(report_data, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                               limits.minUniformBufferOffsetAlignment, pDynamicOffsets,
                                                               "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01971",
                                                               "minUniformBufferOffsetAlignment", &cur_dyn_offset);
                        skip |= ValidateDynamicOffsetAlignment(report_data, binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                                               limits.minStorageBufferOffsetAlignment, pDynamicOffsets,
                                                               "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01972",
                                                               "minStorageBufferOffsetAlignment", &cur_dyn_offset);
                    }
                    // Keep running total of dynamic descriptor count to verify at the end
                    total_dynamic_descriptors += set_dynamic_descriptor_count;
                }
            }
        } else {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            HandleToUint64(pDescriptorSets[set_idx]), kVUID_Core_DrawState_InvalidSet,
                            "Attempt to bind %s that doesn't exist!", report_data->FormatHandle(pDescriptorSets[set_idx]).c_str());
        }
    }
    //  dynamicOffsetCount must equal the total number of dynamic descriptors in the sets being bound
    if (total_dynamic_descriptors != dynamicOffsetCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359",
                        "Attempting to bind %u descriptorSets with %u dynamic descriptors, but dynamicOffsetCount is %u. It should "
                        "exactly match the number of dynamic descriptors.",
                        setCount, total_dynamic_descriptors, dynamicOffsetCount);
    }
    return skip;
}

// Validates that the supplied bind point is supported for the command buffer (vis. the command pool)
// Takes array of error codes as some of the VUID's (e.g. vkCmdBindPipeline) are written per bindpoint
// TODO add vkCmdBindPipeline bind_point validation using this call.
bool CoreChecks::ValidatePipelineBindPoint(const CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point, const char *func_name,
                                           const std::map<VkPipelineBindPoint, std::string> &bind_errors) const {
    bool skip = false;
    auto pool = cb_state->command_pool.get();
    if (pool) {  // The loss of a pool in a recording cmd is reported in DestroyCommandPool
        static const std::map<VkPipelineBindPoint, VkQueueFlags> flag_mask = {
            std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<VkQueueFlags>(VK_QUEUE_COMPUTE_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                           static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)),
        };
        const auto &qfp = GetPhysicalDeviceState()->queue_family_properties[pool->queueFamilyIndex];
        if (0 == (qfp.queueFlags & flag_mask.at(bind_point))) {
            const std::string &error = bind_errors.at(bind_point);
            auto cb_u64 = HandleToUint64(cb_state->commandBuffer);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, cb_u64,
                            error, "%s: %s was allocated from %s that does not support bindpoint %s.", func_name,
                            report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                            report_data->FormatHandle(cb_state->createInfo.commandPool).c_str(),
                            string_VkPipelineBindPoint(bind_point));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    const char *func_name = "vkCmdPushDescriptorSetKHR()";
    bool skip = false;
    skip |= ValidateCmd(cb_state, CMD_PUSHDESCRIPTORSETKHR, func_name);
    skip |= ValidateCmdQueueFlags(cb_state, func_name, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT),
                                  "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-cmdpool");

    static const std::map<VkPipelineBindPoint, std::string> bind_errors = {
        std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363"),
        std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363"),
        std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363")};

    skip |= ValidatePipelineBindPoint(cb_state, pipelineBindPoint, func_name, bind_errors);
    const auto layout_data = GetPipelineLayout(layout);

    // Validate the set index points to a push descriptor set and is in range
    if (layout_data) {
        const auto &set_layouts = layout_data->set_layouts;
        const auto layout_u64 = HandleToUint64(layout);
        if (set < set_layouts.size()) {
            const auto dsl = set_layouts[set];
            if (dsl) {
                if (!dsl->IsPushDescriptor()) {
                    skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                   layout_u64, "VUID-vkCmdPushDescriptorSetKHR-set-00365",
                                   "%s: Set index %" PRIu32 " does not match push descriptor set layout index for %s.", func_name,
                                   set, report_data->FormatHandle(layout).c_str());
                } else {
                    // Create an empty proxy in order to use the existing descriptor set update validation
                    // TODO move the validation (like this) that doesn't need descriptor set state to the DSL object so we
                    // don't have to do this.
                    cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, nullptr, report_data);
                    skip |= ValidatePushDescriptorsUpdate(&proxy_ds, descriptorWriteCount, pDescriptorWrites, func_name);
                }
            }
        } else {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, layout_u64,
                           "VUID-vkCmdPushDescriptorSetKHR-set-00364",
                           "%s: Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", func_name, set,
                           report_data->FormatHandle(layout).c_str(), static_cast<uint32_t>(set_layouts.size()));
        }
    }

    return skip;
}

static VkDeviceSize GetIndexAlignment(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        case VK_INDEX_TYPE_UINT8_EXT:
            return 1;
        default:
            // Not a real index type. Express no alignment requirement here; we expect upper layer
            // to have already picked up on the enum being nonsense.
            return 1;
    }
}

bool CoreChecks::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType) const {
    const auto buffer_state = GetBufferState(buffer);
    const auto cb_node = GetCBState(commandBuffer);
    assert(buffer_state);
    assert(cb_node);

    bool skip =
        ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true, "VUID-vkCmdBindIndexBuffer-buffer-00433",
                                 "vkCmdBindIndexBuffer()", "VK_BUFFER_USAGE_INDEX_BUFFER_BIT");
    skip |= ValidateCmdQueueFlags(cb_node, "vkCmdBindIndexBuffer()", VK_QUEUE_GRAPHICS_BIT,
                                  "VUID-vkCmdBindIndexBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_BINDINDEXBUFFER, "vkCmdBindIndexBuffer()");
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdBindIndexBuffer()", "VUID-vkCmdBindIndexBuffer-buffer-00434");
    const auto offset_align = GetIndexAlignment(indexType);
    if (offset % offset_align) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBindIndexBuffer-offset-00432",
                        "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") does not fall on alignment (%s) boundary.", offset,
                        string_VkIndexType(indexType));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) const {
    const auto cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBindVertexBuffers()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdBindVertexBuffers-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_BINDVERTEXBUFFERS, "vkCmdBindVertexBuffers()");
    for (uint32_t i = 0; i < bindingCount; ++i) {
        const auto buffer_state = GetBufferState(pBuffers[i]);
        assert(buffer_state);
        skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                         "VUID-vkCmdBindVertexBuffers-pBuffers-00627", "vkCmdBindVertexBuffers()",
                                         "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
        skip |=
            ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdBindVertexBuffers()", "VUID-vkCmdBindVertexBuffers-pBuffers-00628");
        if (pOffsets[i] >= buffer_state->createInfo.size) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-vkCmdBindVertexBuffers-pOffsets-00626",
                            "vkCmdBindVertexBuffers() offset (0x%" PRIxLEAST64 ") is beyond the end of the buffer.", pOffsets[i]);
        }
    }
    return skip;
}

// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(const IMAGE_STATE *image_state, VkSampleCountFlagBits sample_count, const char *location,
                                          const std::string &msgCode) const {
    bool skip = false;
    if (image_state->createInfo.samples != sample_count) {
        skip =
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                    HandleToUint64(image_state->image), msgCode, "%s for %s was created with a sample count of %s but must be %s.",
                    location, report_data->FormatHandle(image_state->image).c_str(),
                    string_VkSampleCountFlagBits(image_state->createInfo.samples), string_VkSampleCountFlagBits(sample_count));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize dataSize, const void *pData) const {
    const auto cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    const auto dst_buffer_state = GetBufferState(dstBuffer);
    assert(dst_buffer_state);

    bool skip = false;
    skip |= ValidateMemoryIsBoundToBuffer(dst_buffer_state, "vkCmdUpdateBuffer()", "VUID-vkCmdUpdateBuffer-dstBuffer-00035");
    // Validate that DST buffer has correct usage flags set
    skip |=
        ValidateBufferUsageFlags(dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, "VUID-vkCmdUpdateBuffer-dstBuffer-00034",
                                 "vkCmdUpdateBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |=
        ValidateCmdQueueFlags(cb_state, "vkCmdUpdateBuffer()", VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                              "VUID-vkCmdUpdateBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_UPDATEBUFFER, "vkCmdUpdateBuffer()");
    skip |= InsideRenderPass(cb_state, "vkCmdUpdateBuffer()", "VUID-vkCmdUpdateBuffer-renderpass");
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdSetEvent()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdSetEvent-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_SETEVENT, "vkCmdSetEvent()");
    skip |= InsideRenderPass(cb_state, "vkCmdSetEvent()", "VUID-vkCmdSetEvent-renderpass");
    skip |= ValidateStageMaskGsTsEnables(stageMask, "vkCmdSetEvent()", "VUID-vkCmdSetEvent-stageMask-01150",
                                         "VUID-vkCmdSetEvent-stageMask-01151", "VUID-vkCmdSetEvent-stageMask-02107",
                                         "VUID-vkCmdSetEvent-stageMask-02108");
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdResetEvent()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdResetEvent-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_RESETEVENT, "vkCmdResetEvent()");
    skip |= InsideRenderPass(cb_state, "vkCmdResetEvent()", "VUID-vkCmdResetEvent-renderpass");
    skip |= ValidateStageMaskGsTsEnables(stageMask, "vkCmdResetEvent()", "VUID-vkCmdResetEvent-stageMask-01154",
                                         "VUID-vkCmdResetEvent-stageMask-01155", "VUID-vkCmdResetEvent-stageMask-02109",
                                         "VUID-vkCmdResetEvent-stageMask-02110");
    return skip;
}

// Return input pipeline stage flags, expanded for individual bits if VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT is set
static VkPipelineStageFlags ExpandPipelineStageFlags(const DeviceExtensions &extensions, VkPipelineStageFlags inflags) {
    if (~inflags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) return inflags;

    return (inflags & ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) |
           (VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
            (extensions.vk_nv_mesh_shader ? (VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV | VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV) : 0) |
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
            VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT |
            (extensions.vk_ext_conditional_rendering ? VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT : 0) |
            (extensions.vk_ext_transform_feedback ? VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT : 0) |
            (extensions.vk_nv_shading_rate_image ? VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV : 0) |
            (extensions.vk_ext_fragment_density_map ? VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT : 0));
}

static bool HasNonFramebufferStagePipelineStageFlags(VkPipelineStageFlags inflags) {
    return (inflags & ~(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)) != 0;
}

static int GetGraphicsPipelineStageLogicalOrdinal(VkPipelineStageFlagBits flag) {
    // Note that the list (and lookup) ignore invalid-for-enabled-extension condition.  This should be checked elsewhere
    // and would greatly complicate this intentionally simple implementation
    // clang-format off
    const VkPipelineStageFlagBits ordered_array[] = {
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
        VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,

        // Including the task/mesh shaders here is not technically correct, as they are in a
        // separate logical pipeline - but it works for the case this is currently used, and
        // fixing it would require significant rework and end up with the code being far more
        // verbose for no practical gain.
        // However, worth paying attention to this if using this function in a new way.
        VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
        VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,

        VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    };
    // clang-format on

    const int ordered_array_length = sizeof(ordered_array) / sizeof(VkPipelineStageFlagBits);

    for (int i = 0; i < ordered_array_length; ++i) {
        if (ordered_array[i] == flag) {
            return i;
        }
    }

    return -1;
}

// The following two functions technically have O(N^2) complexity, but it's for a value of O that's largely
// stable and also rather tiny - this could definitely be rejigged to work more efficiently, but the impact
// on runtime is currently negligible, so it wouldn't gain very much.
// If we add a lot more graphics pipeline stages, this set of functions should be rewritten to accomodate.
static VkPipelineStageFlagBits GetLogicallyEarliestGraphicsPipelineStage(VkPipelineStageFlags inflags) {
    VkPipelineStageFlagBits earliest_bit = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    int earliest_bit_order = GetGraphicsPipelineStageLogicalOrdinal(earliest_bit);

    for (std::size_t i = 0; i < sizeof(VkPipelineStageFlagBits); ++i) {
        VkPipelineStageFlagBits current_flag = (VkPipelineStageFlagBits)((inflags & 0x1u) << i);
        if (current_flag) {
            int new_order = GetGraphicsPipelineStageLogicalOrdinal(current_flag);
            if (new_order != -1 && new_order < earliest_bit_order) {
                earliest_bit_order = new_order;
                earliest_bit = current_flag;
            }
        }
        inflags = inflags >> 1;
    }
    return earliest_bit;
}

static VkPipelineStageFlagBits GetLogicallyLatestGraphicsPipelineStage(VkPipelineStageFlags inflags) {
    VkPipelineStageFlagBits latest_bit = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    int latest_bit_order = GetGraphicsPipelineStageLogicalOrdinal(latest_bit);

    for (std::size_t i = 0; i < sizeof(VkPipelineStageFlagBits); ++i) {
        if (inflags & 0x1u) {
            int new_order = GetGraphicsPipelineStageLogicalOrdinal((VkPipelineStageFlagBits)((inflags & 0x1u) << i));
            if (new_order != -1 && new_order > latest_bit_order) {
                latest_bit_order = new_order;
                latest_bit = (VkPipelineStageFlagBits)((inflags & 0x1u) << i);
            }
        }
        inflags = inflags >> 1;
    }
    return latest_bit;
}

// Verify image barrier image state and that the image is consistent with FB image
bool CoreChecks::ValidateImageBarrierAttachment(const char *funcName, CMD_BUFFER_STATE const *cb_state, VkFramebuffer framebuffer,
                                                uint32_t active_subpass, const safe_VkSubpassDescription2KHR &sub_desc,
                                                const VulkanTypedHandle &rp_handle, uint32_t img_index,
                                                const VkImageMemoryBarrier &img_barrier) const {
    bool skip = false;
    const auto &fb_state = GetFramebufferState(framebuffer);
    assert(fb_state);
    const auto img_bar_image = img_barrier.image;
    bool image_match = false;
    bool sub_image_found = false;  // Do we find a corresponding subpass description
    VkImageLayout sub_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t attach_index = 0;
    // Verify that a framebuffer image matches barrier image
    const auto attachmentCount = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachmentCount; ++attachment) {
        auto view_state = GetAttachmentImageViewState(fb_state, attachment);
        if (view_state && (img_bar_image == view_state->create_info.image)) {
            image_match = true;
            attach_index = attachment;
            break;
        }
    }
    if (image_match) {  // Make sure subpass is referring to matching attachment
        if (sub_desc.pDepthStencilAttachment && sub_desc.pDepthStencilAttachment->attachment == attach_index) {
            sub_image_layout = sub_desc.pDepthStencilAttachment->layout;
            sub_image_found = true;
        }
        if (!sub_image_found && device_extensions.vk_khr_depth_stencil_resolve) {
            const auto *resolve = lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolveKHR>(sub_desc.pNext);
            if (resolve && resolve->pDepthStencilResolveAttachment &&
                resolve->pDepthStencilResolveAttachment->attachment == attach_index) {
                sub_image_layout = resolve->pDepthStencilResolveAttachment->layout;
                sub_image_found = true;
            }
        }
        if (!sub_image_found) {
            for (uint32_t j = 0; j < sub_desc.colorAttachmentCount; ++j) {
                if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pColorAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
                if (!sub_image_found && sub_desc.pResolveAttachments &&
                    sub_desc.pResolveAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pResolveAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
            }
        }
        if (!sub_image_found) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-image-02635",
                            "%s: Barrier pImageMemoryBarriers[%d].%s is not referenced by the VkSubpassDescription for "
                            "active subpass (%d) of current %s.",
                            funcName, img_index, report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                            report_data->FormatHandle(rp_handle).c_str());
        }
    } else {  // !image_match
        auto const fb_handle = HandleToUint64(fb_state->framebuffer);
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, fb_handle,
                        "VUID-vkCmdPipelineBarrier-image-02635",
                        "%s: Barrier pImageMemoryBarriers[%d].%s does not match an image from the current %s.", funcName, img_index,
                        report_data->FormatHandle(img_bar_image).c_str(), report_data->FormatHandle(fb_state->framebuffer).c_str());
    }
    if (img_barrier.oldLayout != img_barrier.newLayout) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-oldLayout-01181",
                        "%s: As the Image Barrier for %s is being executed within a render pass instance, oldLayout must "
                        "equal newLayout yet they are %s and %s.",
                        funcName, report_data->FormatHandle(img_barrier.image).c_str(), string_VkImageLayout(img_barrier.oldLayout),
                        string_VkImageLayout(img_barrier.newLayout));
    } else {
        if (sub_image_found && sub_image_layout != img_barrier.oldLayout) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-oldLayout-02636",
                            "%s: Barrier pImageMemoryBarriers[%d].%s is referenced by the VkSubpassDescription for active "
                            "subpass (%d) of current %s as having layout %s, but image barrier has layout %s.",
                            funcName, img_index, report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                            report_data->FormatHandle(rp_handle).c_str(), string_VkImageLayout(sub_image_layout),
                            string_VkImageLayout(img_barrier.oldLayout));
        }
    }
    return skip;
}

// Validate image barriers within a renderPass
bool CoreChecks::ValidateRenderPassImageBarriers(const char *funcName, const CMD_BUFFER_STATE *cb_state, uint32_t active_subpass,
                                                 const safe_VkSubpassDescription2KHR &sub_desc, const VulkanTypedHandle &rp_handle,
                                                 const safe_VkSubpassDependency2KHR *dependencies,
                                                 const std::vector<uint32_t> &self_dependencies, uint32_t image_mem_barrier_count,
                                                 const VkImageMemoryBarrier *image_barriers) const {
    bool skip = false;
    for (uint32_t i = 0; i < image_mem_barrier_count; ++i) {
        const auto &img_barrier = image_barriers[i];
        const auto &img_src_access_mask = img_barrier.srcAccessMask;
        const auto &img_dst_access_mask = img_barrier.dstAccessMask;
        bool access_mask_match = false;
        for (const auto self_dep_index : self_dependencies) {
            const auto &sub_dep = dependencies[self_dep_index];
            access_mask_match = (img_src_access_mask == (sub_dep.srcAccessMask & img_src_access_mask)) &&
                                (img_dst_access_mask == (sub_dep.dstAccessMask & img_dst_access_mask));
            if (access_mask_match) break;
        }
        if (!access_mask_match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                            "%s: Barrier pImageMemoryBarriers[%d].srcAccessMask(0x%X) is not a subset of VkSubpassDependency "
                            "srcAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                            funcName, i, img_src_access_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                            self_dep_ss.str().c_str());
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                            "%s: Barrier pImageMemoryBarriers[%d].dstAccessMask(0x%X) is not a subset of VkSubpassDependency "
                            "dstAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                            funcName, i, img_dst_access_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                            self_dep_ss.str().c_str());
        }
        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182",
                            "%s: Barrier pImageMemoryBarriers[%d].srcQueueFamilyIndex is %d and "
                            "pImageMemoryBarriers[%d].dstQueueFamilyIndex is %d but both must be VK_QUEUE_FAMILY_IGNORED.",
                            funcName, i, img_barrier.srcQueueFamilyIndex, i, img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(funcName, cb_state, cb_state->activeFramebuffer, active_subpass, sub_desc,
                                                   rp_handle, i, img_barrier);
        }
    }
    return skip;
}

// Validate VUs for Pipeline Barriers that are within a renderPass
// Pre: cb_state->activeRenderPass must be a pointer to valid renderPass state
bool CoreChecks::ValidateRenderPassPipelineBarriers(const char *funcName, const CMD_BUFFER_STATE *cb_state,
                                                    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                                    VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                                    const VkMemoryBarrier *mem_barriers, uint32_t buffer_mem_barrier_count,
                                                    const VkBufferMemoryBarrier *buffer_mem_barriers,
                                                    uint32_t image_mem_barrier_count,
                                                    const VkImageMemoryBarrier *image_barriers) const {
    bool skip = false;
    const auto rp_state = cb_state->activeRenderPass;
    const auto active_subpass = cb_state->activeSubpass;
    const VulkanTypedHandle rp_handle(rp_state->renderPass, kVulkanObjectTypeRenderPass);
    const auto &self_dependencies = rp_state->self_dependencies[active_subpass];
    const auto &dependencies = rp_state->createInfo.pDependencies;
    if (self_dependencies.size() == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, rp_handle.handle,
                        "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                        "%s: Barriers cannot be set during subpass %d of %s with no self-dependency specified.", funcName,
                        active_subpass, report_data->FormatHandle(rp_handle).c_str());
    } else {
        // Grab ref to current subpassDescription up-front for use below
        const auto &sub_desc = rp_state->createInfo.pSubpasses[active_subpass];
        // Look for matching mask in any self-dependency
        bool stage_mask_match = false;
        for (const auto self_dep_index : self_dependencies) {
            const auto &sub_dep = dependencies[self_dep_index];
            const auto &sub_src_stage_mask = ExpandPipelineStageFlags(device_extensions, sub_dep.srcStageMask);
            const auto &sub_dst_stage_mask = ExpandPipelineStageFlags(device_extensions, sub_dep.dstStageMask);
            stage_mask_match = ((sub_src_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                                (src_stage_mask == (sub_src_stage_mask & src_stage_mask))) &&
                               ((sub_dst_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                                (dst_stage_mask == (sub_dst_stage_mask & dst_stage_mask)));
            if (stage_mask_match) break;
        }
        if (!stage_mask_match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                            "%s: Barrier srcStageMask(0x%X) is not a subset of VkSubpassDependency srcStageMask of any "
                            "self-dependency of subpass %d of %s for which dstStageMask is also a subset. "
                            "Candidate VkSubpassDependency are pDependencies entries [%s].",
                            funcName, src_stage_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                            self_dep_ss.str().c_str());
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                            "%s: Barrier dstStageMask(0x%X) is not a subset of VkSubpassDependency dstStageMask of any "
                            "self-dependency of subpass %d of %s for which srcStageMask is also a subset. "
                            "Candidate VkSubpassDependency are pDependencies entries [%s].",
                            funcName, dst_stage_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                            self_dep_ss.str().c_str());
        }

        if (0 != buffer_mem_barrier_count) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178",
                            "%s: bufferMemoryBarrierCount is non-zero (%d) for subpass %d of %s.", funcName,
                            buffer_mem_barrier_count, active_subpass, report_data->FormatHandle(rp_handle).c_str());
        }
        for (uint32_t i = 0; i < mem_barrier_count; ++i) {
            const auto &mb_src_access_mask = mem_barriers[i].srcAccessMask;
            const auto &mb_dst_access_mask = mem_barriers[i].dstAccessMask;
            bool access_mask_match = false;
            for (const auto self_dep_index : self_dependencies) {
                const auto &sub_dep = dependencies[self_dep_index];
                access_mask_match = (mb_src_access_mask == (sub_dep.srcAccessMask & mb_src_access_mask)) &&
                                    (mb_dst_access_mask == (sub_dep.dstAccessMask & mb_dst_access_mask));
                if (access_mask_match) break;
            }

            if (!access_mask_match) {
                std::stringstream self_dep_ss;
                stream_join(self_dep_ss, ", ", self_dependencies);
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, rp_handle.handle,
                    "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                    "%s: Barrier pMemoryBarriers[%d].srcAccessMask(0x%X) is not a subset of VkSubpassDependency srcAccessMask "
                    "for any self-dependency of subpass %d of %s for which dstAccessMask is also a subset. "
                    "Candidate VkSubpassDependency are pDependencies entries [%s].",
                    funcName, i, mb_src_access_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                    self_dep_ss.str().c_str());
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, rp_handle.handle,
                    "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                    "%s: Barrier pMemoryBarriers[%d].dstAccessMask(0x%X) is not a subset of VkSubpassDependency dstAccessMask "
                    "for any self-dependency of subpass %d of %s for which srcAccessMask is also a subset. "
                    "Candidate VkSubpassDependency are pDependencies entries [%s].",
                    funcName, i, mb_dst_access_mask, active_subpass, report_data->FormatHandle(rp_handle).c_str(),
                    self_dep_ss.str().c_str());
            }
        }

        skip |= ValidateRenderPassImageBarriers(funcName, cb_state, active_subpass, sub_desc, rp_handle, dependencies,
                                                self_dependencies, image_mem_barrier_count, image_barriers);

        bool flag_match = false;
        for (const auto self_dep_index : self_dependencies) {
            const auto &sub_dep = dependencies[self_dep_index];
            flag_match = sub_dep.dependencyFlags == dependency_flags;
            if (flag_match) break;
        }
        if (!flag_match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            rp_handle.handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                            "%s: dependencyFlags param (0x%X) does not equal VkSubpassDependency dependencyFlags value for any "
                            "self-dependency of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                            funcName, dependency_flags, cb_state->activeSubpass, report_data->FormatHandle(rp_handle).c_str(),
                            self_dep_ss.str().c_str());
        }
    }
    return skip;
}

// Array to mask individual accessMask to corresponding stageMask
//  accessMask active bit position (0-31) maps to index
const static VkPipelineStageFlags AccessMaskToPipeStage[28] = {
    // VK_ACCESS_INDIRECT_COMMAND_READ_BIT = 0
    VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
    // VK_ACCESS_INDEX_READ_BIT = 1
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
    // VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT = 2
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
    // VK_ACCESS_UNIFORM_READ_BIT = 3
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
        VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV |
        VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
    // VK_ACCESS_INPUT_ATTACHMENT_READ_BIT = 4
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    // VK_ACCESS_SHADER_READ_BIT = 5
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
        VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV |
        VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
    // VK_ACCESS_SHADER_WRITE_BIT = 6
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
        VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV |
        VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
    // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT = 7
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    // VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT = 8
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT = 9
    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 10
    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    // VK_ACCESS_TRANSFER_READ_BIT = 11
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    // VK_ACCESS_TRANSFER_WRITE_BIT = 12
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    // VK_ACCESS_HOST_READ_BIT = 13
    VK_PIPELINE_STAGE_HOST_BIT,
    // VK_ACCESS_HOST_WRITE_BIT = 14
    VK_PIPELINE_STAGE_HOST_BIT,
    // VK_ACCESS_MEMORY_READ_BIT = 15
    VK_ACCESS_FLAG_BITS_MAX_ENUM,  // Always match
    // VK_ACCESS_MEMORY_WRITE_BIT = 16
    VK_ACCESS_FLAG_BITS_MAX_ENUM,  // Always match
    // VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX = 17
    VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
    // VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX = 18
    VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
    // VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT = 19
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    // VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT = 20
    VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
    // VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV = 21
    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
    // VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV = 22
    VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
    // VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV = 23
    VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
    // VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT = 24
    VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
    // VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT = 25
    VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
    // VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT = 26
    VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
    // VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT = 27
    VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
};

// Verify that all bits of access_mask are supported by the src_stage_mask
static bool ValidateAccessMaskPipelineStage(const DeviceExtensions &extensions, VkAccessFlags access_mask,
                                            VkPipelineStageFlags stage_mask) {
    // Early out if all commands set, or access_mask NULL
    if ((stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) || (0 == access_mask)) return true;

    stage_mask = ExpandPipelineStageFlags(extensions, stage_mask);
    int index = 0;
    // for each of the set bits in access_mask, make sure that supporting stage mask bit(s) are set
    while (access_mask) {
        index = (u_ffs(access_mask) - 1);
        assert(index >= 0);
        // Must have "!= 0" compare to prevent warning from MSVC
        if ((AccessMaskToPipeStage[index] & stage_mask) == 0) return false;  // early out
        access_mask &= ~(1 << index);                                        // Mask off bit that's been checked
    }
    return true;
}

namespace barrier_queue_families {
enum VuIndex {
    kSrcOrDstMustBeIgnore,
    kSpecialOrIgnoreOnly,
    kSrcIgnoreRequiresDstIgnore,
    kDstValidOrSpecialIfNotIgnore,
    kSrcValidOrSpecialIfNotIgnore,
    kSrcAndDestMustBeIgnore,
    kBothIgnoreOrBothValid,
    kSubmitQueueMustMatchSrcOrDst
};
static const char *vu_summary[] = {"Source or destination queue family must be ignored.",
                                   "Source or destination queue family must be special or ignored.",
                                   "Destination queue family must be ignored if source queue family is.",
                                   "Destination queue family must be valid, ignored, or special.",
                                   "Source queue family must be valid, ignored, or special.",
                                   "Source and destination queue family must both be ignored.",
                                   "Source and destination queue family must both be ignore or both valid.",
                                   "Source or destination queue family must match submit queue family, if not ignored."};

static const std::string image_error_codes[] = {
    "VUID-VkImageMemoryBarrier-image-01381",  //   kSrcOrDstMustBeIgnore
    "VUID-VkImageMemoryBarrier-image-01766",  //   kSpecialOrIgnoreOnly
    "VUID-VkImageMemoryBarrier-image-01201",  //   kSrcIgnoreRequiresDstIgnore
    "VUID-VkImageMemoryBarrier-image-01768",  //   kDstValidOrSpecialIfNotIgnore
    "VUID-VkImageMemoryBarrier-image-01767",  //   kSrcValidOrSpecialIfNotIgnore
    "VUID-VkImageMemoryBarrier-image-01199",  //   kSrcAndDestMustBeIgnore
    "VUID-VkImageMemoryBarrier-image-01200",  //   kBothIgnoreOrBothValid
    "VUID-VkImageMemoryBarrier-image-01205",  //   kSubmitQueueMustMatchSrcOrDst
};

static const std::string buffer_error_codes[] = {
    "VUID-VkBufferMemoryBarrier-buffer-01191",  //  kSrcOrDstMustBeIgnore
    "VUID-VkBufferMemoryBarrier-buffer-01763",  //  kSpecialOrIgnoreOnly
    "VUID-VkBufferMemoryBarrier-buffer-01193",  //  kSrcIgnoreRequiresDstIgnore
    "VUID-VkBufferMemoryBarrier-buffer-01765",  //  kDstValidOrSpecialIfNotIgnore
    "VUID-VkBufferMemoryBarrier-buffer-01764",  //  kSrcValidOrSpecialIfNotIgnore
    "VUID-VkBufferMemoryBarrier-buffer-01190",  //  kSrcAndDestMustBeIgnore
    "VUID-VkBufferMemoryBarrier-buffer-01192",  //  kBothIgnoreOrBothValid
    "VUID-VkBufferMemoryBarrier-buffer-01196",  //  kSubmitQueueMustMatchSrcOrDst
};

class ValidatorState {
  public:
    ValidatorState(const ValidationStateTracker *device_data, const char *func_name, const CMD_BUFFER_STATE *cb_state,
                   const VulkanTypedHandle &barrier_handle, const VkSharingMode sharing_mode)
        : report_data_(device_data->report_data),
          func_name_(func_name),
          cb_handle64_(HandleToUint64(cb_state->commandBuffer)),
          barrier_handle_(barrier_handle),
          sharing_mode_(sharing_mode),
          val_codes_(barrier_handle.type == kVulkanObjectTypeImage ? image_error_codes : buffer_error_codes),
          limit_(static_cast<uint32_t>(device_data->physical_device_state->queue_family_properties.size())),
          mem_ext_(device_data->device_extensions.vk_khr_external_memory) {}

    // Log the messages using boilerplate from object state, and Vu specific information from the template arg
    // One and two family versions, in the single family version, Vu holds the name of the passed parameter
    bool LogMsg(VuIndex vu_index, uint32_t family, const char *param_name) const {
        const std::string &val_code = val_codes_[vu_index];
        const char *annotation = GetFamilyAnnotation(family);
        return log_msg(report_data_, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, cb_handle64_,
                       val_code, "%s: Barrier using %s %s created with sharingMode %s, has %s %u%s. %s", func_name_,
                       GetTypeString(), report_data_->FormatHandle(barrier_handle_).c_str(), GetModeString(), param_name, family,
                       annotation, vu_summary[vu_index]);
    }

    bool LogMsg(VuIndex vu_index, uint32_t src_family, uint32_t dst_family) const {
        const std::string &val_code = val_codes_[vu_index];
        const char *src_annotation = GetFamilyAnnotation(src_family);
        const char *dst_annotation = GetFamilyAnnotation(dst_family);
        return log_msg(
            report_data_, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, cb_handle64_, val_code,
            "%s: Barrier using %s %s created with sharingMode %s, has srcQueueFamilyIndex %u%s and dstQueueFamilyIndex %u%s. %s",
            func_name_, GetTypeString(), report_data_->FormatHandle(barrier_handle_).c_str(), GetModeString(), src_family,
            src_annotation, dst_family, dst_annotation, vu_summary[vu_index]);
    }

    // This abstract Vu can only be tested at submit time, thus we need a callback from the closure containing the needed
    // data. Note that the mem_barrier is copied to the closure as the lambda lifespan exceed the guarantees of validity for
    // application input.
    static bool ValidateAtQueueSubmit(const QUEUE_STATE *queue_state, const ValidationStateTracker *device_data,
                                      uint32_t src_family, uint32_t dst_family, const ValidatorState &val) {
        uint32_t queue_family = queue_state->queueFamilyIndex;
        if ((src_family != queue_family) && (dst_family != queue_family)) {
            const std::string &val_code = val.val_codes_[kSubmitQueueMustMatchSrcOrDst];
            const char *src_annotation = val.GetFamilyAnnotation(src_family);
            const char *dst_annotation = val.GetFamilyAnnotation(dst_family);
            return log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                           HandleToUint64(queue_state->queue), val_code,
                           "%s: Barrier submitted to queue with family index %u, using %s %s created with sharingMode %s, has "
                           "srcQueueFamilyIndex %u%s and dstQueueFamilyIndex %u%s. %s",
                           "vkQueueSubmit", queue_family, val.GetTypeString(),
                           device_data->report_data->FormatHandle(val.barrier_handle_).c_str(), val.GetModeString(), src_family,
                           src_annotation, dst_family, dst_annotation, vu_summary[kSubmitQueueMustMatchSrcOrDst]);
        }
        return false;
    }
    // Logical helpers for semantic clarity
    inline bool KhrExternalMem() const { return mem_ext_; }
    inline bool IsValid(uint32_t queue_family) const { return (queue_family < limit_); }
    inline bool IsValidOrSpecial(uint32_t queue_family) const {
        return IsValid(queue_family) || (mem_ext_ && QueueFamilyIsSpecial(queue_family));
    }

    // Helpers for LogMsg (and log_msg)
    const char *GetModeString() const { return string_VkSharingMode(sharing_mode_); }

    // Descriptive text for the various types of queue family index
    const char *GetFamilyAnnotation(uint32_t family) const {
        const char *external = " (VK_QUEUE_FAMILY_EXTERNAL_KHR)";
        const char *foreign = " (VK_QUEUE_FAMILY_FOREIGN_EXT)";
        const char *ignored = " (VK_QUEUE_FAMILY_IGNORED)";
        const char *valid = " (VALID)";
        const char *invalid = " (INVALID)";
        switch (family) {
            case VK_QUEUE_FAMILY_EXTERNAL_KHR:
                return external;
            case VK_QUEUE_FAMILY_FOREIGN_EXT:
                return foreign;
            case VK_QUEUE_FAMILY_IGNORED:
                return ignored;
            default:
                if (IsValid(family)) {
                    return valid;
                }
                return invalid;
        };
    }
    const char *GetTypeString() const { return object_string[barrier_handle_.type]; }
    VkSharingMode GetSharingMode() const { return sharing_mode_; }

  protected:
    const debug_report_data *const report_data_;
    const char *const func_name_;
    const uint64_t cb_handle64_;
    const VulkanTypedHandle barrier_handle_;
    const VkSharingMode sharing_mode_;
    const std::string *val_codes_;
    const uint32_t limit_;
    const bool mem_ext_;
};

bool Validate(const CoreChecks *device_data, const char *func_name, const CMD_BUFFER_STATE *cb_state, const ValidatorState &val,
              const uint32_t src_queue_family, const uint32_t dst_queue_family) {
    bool skip = false;

    const bool mode_concurrent = val.GetSharingMode() == VK_SHARING_MODE_CONCURRENT;
    const bool src_ignored = QueueFamilyIsIgnored(src_queue_family);
    const bool dst_ignored = QueueFamilyIsIgnored(dst_queue_family);
    if (val.KhrExternalMem()) {
        if (mode_concurrent) {
            if (!(src_ignored || dst_ignored)) {
                skip |= val.LogMsg(kSrcOrDstMustBeIgnore, src_queue_family, dst_queue_family);
            }
            if ((src_ignored && !(dst_ignored || QueueFamilyIsSpecial(dst_queue_family))) ||
                (dst_ignored && !(src_ignored || QueueFamilyIsSpecial(src_queue_family)))) {
                skip |= val.LogMsg(kSpecialOrIgnoreOnly, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if (src_ignored && !dst_ignored) {
                skip |= val.LogMsg(kSrcIgnoreRequiresDstIgnore, src_queue_family, dst_queue_family);
            }
            if (!dst_ignored && !val.IsValidOrSpecial(dst_queue_family)) {
                skip |= val.LogMsg(kDstValidOrSpecialIfNotIgnore, dst_queue_family, "dstQueueFamilyIndex");
            }
            if (!src_ignored && !val.IsValidOrSpecial(src_queue_family)) {
                skip |= val.LogMsg(kSrcValidOrSpecialIfNotIgnore, src_queue_family, "srcQueueFamilyIndex");
            }
        }
    } else {
        // No memory extension
        if (mode_concurrent) {
            if (!src_ignored || !dst_ignored) {
                skip |= val.LogMsg(kSrcAndDestMustBeIgnore, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if (!((src_ignored && dst_ignored) || (val.IsValid(src_queue_family) && val.IsValid(dst_queue_family)))) {
                skip |= val.LogMsg(kBothIgnoreOrBothValid, src_queue_family, dst_queue_family);
            }
        }
    }
    return skip;
}
}  // namespace barrier_queue_families

bool CoreChecks::ValidateConcurrentBarrierAtSubmit(const ValidationStateTracker *state_data, const QUEUE_STATE *queue_state,
                                                   const char *func_name, const CMD_BUFFER_STATE *cb_state,
                                                   const VulkanTypedHandle &typed_handle, uint32_t src_queue_family,
                                                   uint32_t dst_queue_family) {
    using barrier_queue_families::ValidatorState;
    ValidatorState val(state_data, func_name, cb_state, typed_handle, VK_SHARING_MODE_CONCURRENT);
    return ValidatorState::ValidateAtQueueSubmit(queue_state, state_data, src_queue_family, dst_queue_family, val);
}

// Type specific wrapper for image barriers
bool CoreChecks::ValidateBarrierQueueFamilies(const char *func_name, const CMD_BUFFER_STATE *cb_state,
                                              const VkImageMemoryBarrier &barrier, const IMAGE_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the image state
    barrier_queue_families::ValidatorState val(this, func_name, cb_state, VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage),
                                               state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, func_name, cb_state, val, src_queue_family, dst_queue_family);
}

// Type specific wrapper for buffer barriers
bool CoreChecks::ValidateBarrierQueueFamilies(const char *func_name, const CMD_BUFFER_STATE *cb_state,
                                              const VkBufferMemoryBarrier &barrier, const BUFFER_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the buffer state
    barrier_queue_families::ValidatorState val(
        this, func_name, cb_state, VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer), state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, func_name, cb_state, val, src_queue_family, dst_queue_family);
}

bool CoreChecks::ValidateBarriers(const char *funcName, const CMD_BUFFER_STATE *cb_state, VkPipelineStageFlags src_stage_mask,
                                  VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount,
                                  const VkMemoryBarrier *pMemBarriers, uint32_t bufferBarrierCount,
                                  const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                  const VkImageMemoryBarrier *pImageMemBarriers) const {
    bool skip = false;
    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        const auto &mem_barrier = pMemBarriers[i];
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.srcAccessMask, src_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184",
                            "%s: pMemBarriers[%d].srcAccessMask (0x%X) is not supported by srcStageMask (0x%X).", funcName, i,
                            mem_barrier.srcAccessMask, src_stage_mask);
        }
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.dstAccessMask, dst_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185",
                            "%s: pMemBarriers[%d].dstAccessMask (0x%X) is not supported by dstStageMask (0x%X).", funcName, i,
                            mem_barrier.dstAccessMask, dst_stage_mask);
        }
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
        const auto &mem_barrier = pImageMemBarriers[i];
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.srcAccessMask, src_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184",
                            "%s: pImageMemBarriers[%d].srcAccessMask (0x%X) is not supported by srcStageMask (0x%X).", funcName, i,
                            mem_barrier.srcAccessMask, src_stage_mask);
        }
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.dstAccessMask, dst_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185",
                            "%s: pImageMemBarriers[%d].dstAccessMask (0x%X) is not supported by dstStageMask (0x%X).", funcName, i,
                            mem_barrier.dstAccessMask, dst_stage_mask);
        }

        auto image_data = GetImageState(mem_barrier.image);
        skip |= ValidateBarrierQueueFamilies(funcName, cb_state, mem_barrier, image_data);

        if (mem_barrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED || mem_barrier.newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-VkImageMemoryBarrier-newLayout-01198",
                            "%s: Image Layout cannot be transitioned to UNDEFINED or PREINITIALIZED.", funcName);
        }

        if (image_data) {
            // There is no VUID for this, but there is blanket text:
            //     "Non-sparse resources must be bound completely and contiguously to a single VkDeviceMemory object before
            //     recording commands in a command buffer."
            // TODO: Update this when VUID is defined
            skip |= ValidateMemoryIsBoundToImage(image_data, funcName, kVUIDUndefined);

            const auto aspect_mask = mem_barrier.subresourceRange.aspectMask;
            skip |= ValidateImageAspectMask(image_data->image, image_data->createInfo.format, aspect_mask, funcName);

            const std::string param_name = "pImageMemoryBarriers[" + std::to_string(i) + "].subresourceRange";
            skip |= ValidateImageBarrierSubresourceRange(image_data, mem_barrier.subresourceRange, funcName, param_name.c_str());
        }
    }

    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        const auto &mem_barrier = pBufferMemBarriers[i];

        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.srcAccessMask, src_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184",
                            "%s: pBufferMemBarriers[%d].srcAccessMask (0x%X) is not supported by srcStageMask (0x%X).", funcName, i,
                            mem_barrier.srcAccessMask, src_stage_mask);
        }
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier.dstAccessMask, dst_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185",
                            "%s: pBufferMemBarriers[%d].dstAccessMask (0x%X) is not supported by dstStageMask (0x%X).", funcName, i,
                            mem_barrier.dstAccessMask, dst_stage_mask);
        }
        // Validate buffer barrier queue family indices
        auto buffer_state = GetBufferState(mem_barrier.buffer);
        skip |= ValidateBarrierQueueFamilies(funcName, cb_state, mem_barrier, buffer_state);

        if (buffer_state) {
            // There is no VUID for this, but there is blanket text:
            //     "Non-sparse resources must be bound completely and contiguously to a single VkDeviceMemory object before
            //     recording commands in a command buffer"
            // TODO: Update this when VUID is defined
            skip |= ValidateMemoryIsBoundToBuffer(buffer_state, funcName, kVUIDUndefined);

            auto buffer_size = buffer_state->createInfo.size;
            if (mem_barrier.offset >= buffer_size) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-VkBufferMemoryBarrier-offset-01187",
                                "%s: Buffer Barrier %s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                                funcName, report_data->FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset),
                                HandleToUint64(buffer_size));
            } else if (mem_barrier.size != VK_WHOLE_SIZE && (mem_barrier.offset + mem_barrier.size > buffer_size)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-VkBufferMemoryBarrier-size-01189",
                                "%s: Buffer Barrier %s has offset 0x%" PRIx64 " and size 0x%" PRIx64
                                " whose sum is greater than total size 0x%" PRIx64 ".",
                                funcName, report_data->FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset),
                                HandleToUint64(mem_barrier.size), HandleToUint64(buffer_size));
            }
        }
    }

    skip |= ValidateBarriersQFOTransferUniqueness(funcName, cb_state, bufferBarrierCount, pBufferMemBarriers, imageMemBarrierCount,
                                                  pImageMemBarriers);

    return skip;
}

bool CoreChecks::ValidateEventStageMask(const ValidationStateTracker *state_data, const CMD_BUFFER_STATE *pCB, size_t eventCount,
                                        size_t firstEventIndex, VkPipelineStageFlags sourceStageMask,
                                        EventToStageMap *localEventToStageMap) {
    bool skip = false;
    VkPipelineStageFlags stageMask = 0;
    const auto max_event = std::min((firstEventIndex + eventCount), pCB->events.size());
    for (size_t event_index = firstEventIndex; event_index < max_event; ++event_index) {
        auto event = pCB->events[event_index];
        auto event_data = localEventToStageMap->find(event);
        if (event_data != localEventToStageMap->end()) {
            stageMask |= event_data->second;
        } else {
            auto global_event_data = state_data->GetEventState(event);
            if (!global_event_data) {
                skip |= log_msg(state_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,
                                HandleToUint64(event), kVUID_Core_DrawState_InvalidEvent,
                                "%s cannot be waited on if it has never been set.",
                                state_data->report_data->FormatHandle(event).c_str());
            } else {
                stageMask |= global_event_data->stageMask;
            }
        }
    }
    // TODO: Need to validate that host_bit is only set if set event is called
    // but set event can be called at any time.
    if (sourceStageMask != stageMask && sourceStageMask != (stageMask | VK_PIPELINE_STAGE_HOST_BIT)) {
        skip |= log_msg(state_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), "VUID-vkCmdWaitEvents-srcStageMask-parameter",
                        "Submitting cmdbuffer with call to VkCmdWaitEvents using srcStageMask 0x%X which must be the bitwise OR of "
                        "the stageMask parameters used in calls to vkCmdSetEvent and VK_PIPELINE_STAGE_HOST_BIT if used with "
                        "vkSetEvent but instead is 0x%X.",
                        sourceStageMask, stageMask);
    }
    return skip;
}

// Note that we only check bits that HAVE required queueflags -- don't care entries are skipped
static std::unordered_map<VkPipelineStageFlags, VkQueueFlags> supported_pipeline_stages_table = {
    {VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT},
    {VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT},
    {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_QUEUE_GRAPHICS_BIT},
    {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_QUEUE_COMPUTE_BIT},
    {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT},
    {VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT}};

static const VkPipelineStageFlags stage_flag_bit_array[] = {VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
                                                            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                                                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                                            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                            VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};

bool CoreChecks::CheckStageMaskQueueCompatibility(VkCommandBuffer command_buffer, VkPipelineStageFlags stage_mask,
                                                  VkQueueFlags queue_flags, const char *function, const char *src_or_dest,
                                                  const char *error_code) const {
    bool skip = false;
    // Lookup each bit in the stagemask and check for overlap between its table bits and queue_flags
    for (const auto &item : stage_flag_bit_array) {
        if (stage_mask & item) {
            if ((supported_pipeline_stages_table[item] & queue_flags) == 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(command_buffer), error_code,
                                "%s(): %s flag %s is not compatible with the queue family properties of this command buffer.",
                                function, src_or_dest, string_VkPipelineStageFlagBits(static_cast<VkPipelineStageFlagBits>(item)));
            }
        }
    }
    return skip;
}

// Check if all barriers are of a given operation type.
template <typename Barrier, typename OpCheck>
bool AllTransferOp(const COMMAND_POOL_STATE *pool, OpCheck &op_check, uint32_t count, const Barrier *barriers) {
    if (!pool) return false;

    for (uint32_t b = 0; b < count; b++) {
        if (!op_check(pool, barriers + b)) return false;
    }
    return true;
}

// Look at the barriers to see if we they are all release or all acquire, the result impacts queue properties validation
BarrierOperationsType CoreChecks::ComputeBarrierOperationsType(const CMD_BUFFER_STATE *cb_state, uint32_t buffer_barrier_count,
                                                               const VkBufferMemoryBarrier *buffer_barriers,
                                                               uint32_t image_barrier_count,
                                                               const VkImageMemoryBarrier *image_barriers) const {
    auto pool = cb_state->command_pool.get();
    BarrierOperationsType op_type = kGeneral;

    // Look at the barrier details only if they exist
    // Note: AllTransferOp returns true for count == 0
    if ((buffer_barrier_count + image_barrier_count) != 0) {
        if (AllTransferOp(pool, TempIsReleaseOp<VkBufferMemoryBarrier>, buffer_barrier_count, buffer_barriers) &&
            AllTransferOp(pool, TempIsReleaseOp<VkImageMemoryBarrier>, image_barrier_count, image_barriers)) {
            op_type = kAllRelease;
        } else if (AllTransferOp(pool, IsAcquireOp<VkBufferMemoryBarrier>, buffer_barrier_count, buffer_barriers) &&
                   AllTransferOp(pool, IsAcquireOp<VkImageMemoryBarrier>, image_barrier_count, image_barriers)) {
            op_type = kAllAcquire;
        }
    }

    return op_type;
}

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(const CMD_BUFFER_STATE *cb_state,
                                                            VkPipelineStageFlags source_stage_mask,
                                                            VkPipelineStageFlags dest_stage_mask,
                                                            BarrierOperationsType barrier_op_type, const char *function,
                                                            const char *error_code) const {
    bool skip = false;
    uint32_t queue_family_index = cb_state->command_pool->queueFamilyIndex;
    auto physical_device_state = GetPhysicalDeviceState();

    // Any pipeline stage included in srcStageMask or dstStageMask must be supported by the capabilities of the queue family
    // specified by the queueFamilyIndex member of the VkCommandPoolCreateInfo structure that was used to create the VkCommandPool
    // that commandBuffer was allocated from, as specified in the table of supported pipeline stages.

    if (queue_family_index < physical_device_state->queue_family_properties.size()) {
        VkQueueFlags specified_queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;

        // Only check the source stage mask if any barriers aren't "acquire ownership"
        if ((barrier_op_type != kAllAcquire) && (source_stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) == 0) {
            skip |= CheckStageMaskQueueCompatibility(cb_state->commandBuffer, source_stage_mask, specified_queue_flags, function,
                                                     "srcStageMask", error_code);
        }
        // Only check the dest stage mask if any barriers aren't "release ownership"
        if ((barrier_op_type != kAllRelease) && (dest_stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) == 0) {
            skip |= CheckStageMaskQueueCompatibility(cb_state->commandBuffer, dest_stage_mask, specified_queue_flags, function,
                                                     "dstStageMask", error_code);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount,
                                              const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    auto barrier_op_type = ComputeBarrierOperationsType(cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                        imageMemoryBarrierCount, pImageMemoryBarriers);
    bool skip = ValidateStageMasksAgainstQueueCapabilities(cb_state, sourceStageMask, dstStageMask, barrier_op_type,
                                                           "vkCmdWaitEvents", "VUID-vkCmdWaitEvents-srcStageMask-01164");
    skip |= ValidateStageMaskGsTsEnables(sourceStageMask, "vkCmdWaitEvents()", "VUID-vkCmdWaitEvents-srcStageMask-01159",
                                         "VUID-vkCmdWaitEvents-srcStageMask-01161", "VUID-vkCmdWaitEvents-srcStageMask-02111",
                                         "VUID-vkCmdWaitEvents-srcStageMask-02112");
    skip |= ValidateStageMaskGsTsEnables(dstStageMask, "vkCmdWaitEvents()", "VUID-vkCmdWaitEvents-dstStageMask-01160",
                                         "VUID-vkCmdWaitEvents-dstStageMask-01162", "VUID-vkCmdWaitEvents-dstStageMask-02113",
                                         "VUID-vkCmdWaitEvents-dstStageMask-02114");
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdWaitEvents()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdWaitEvents-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_WAITEVENTS, "vkCmdWaitEvents()");
    skip |= ValidateBarriersToImages(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, "vkCmdWaitEvents()");
    skip |= ValidateBarriers("vkCmdWaitEvents()", cb_state, sourceStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

void CoreChecks::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                            uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                            uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                            uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // The StateTracker added will add to the events vector.
    auto first_event_index = cb_state->events.size();
    StateTracker::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                             imageMemoryBarrierCount, pImageMemoryBarriers);
    auto event_added_count = cb_state->events.size() - first_event_index;

    const CMD_BUFFER_STATE *cb_state_const = cb_state;
    cb_state->eventUpdates.emplace_back(
        [cb_state_const, event_added_count, first_event_index, sourceStageMask](
            const ValidationStateTracker *device_data, bool do_validate, EventToStageMap *localEventToStageMap) {
            if (!do_validate) return false;
            return ValidateEventStageMask(device_data, cb_state_const, event_added_count, first_event_index, sourceStageMask,
                                          localEventToStageMap);
        });
    TransitionImageLayouts(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordBarrierValidationInfo("vkCmdWaitEvents", cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                   uint32_t bufferMemoryBarrierCount,
                                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                   uint32_t imageMemoryBarrierCount,
                                                   const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    bool skip = false;
    auto barrier_op_type = ComputeBarrierOperationsType(cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                        imageMemoryBarrierCount, pImageMemoryBarriers);
    skip |= ValidateStageMasksAgainstQueueCapabilities(cb_state, srcStageMask, dstStageMask, barrier_op_type,
                                                       "vkCmdPipelineBarrier", "VUID-vkCmdPipelineBarrier-srcStageMask-01183");
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdPipelineBarrier()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdPipelineBarrier-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_PIPELINEBARRIER, "vkCmdPipelineBarrier()");
    skip |=
        ValidateStageMaskGsTsEnables(srcStageMask, "vkCmdPipelineBarrier()", "VUID-vkCmdPipelineBarrier-srcStageMask-01168",
                                     "VUID-vkCmdPipelineBarrier-srcStageMask-01170", "VUID-vkCmdPipelineBarrier-srcStageMask-02115",
                                     "VUID-vkCmdPipelineBarrier-srcStageMask-02116");
    skip |=
        ValidateStageMaskGsTsEnables(dstStageMask, "vkCmdPipelineBarrier()", "VUID-vkCmdPipelineBarrier-dstStageMask-01169",
                                     "VUID-vkCmdPipelineBarrier-dstStageMask-01171", "VUID-vkCmdPipelineBarrier-dstStageMask-02117",
                                     "VUID-vkCmdPipelineBarrier-dstStageMask-02118");
    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers("vkCmdPipelineBarrier()", cb_state, srcStageMask, dstStageMask, dependencyFlags,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    }
    skip |= ValidateBarriersToImages(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, "vkCmdPipelineBarrier()");
    skip |= ValidateBarriers("vkCmdPipelineBarrier()", cb_state, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

void CoreChecks::EnqueueSubmitTimeValidateImageBarrierAttachment(const char *func_name, CMD_BUFFER_STATE *cb_state,
                                                                 uint32_t imageMemBarrierCount,
                                                                 const VkImageMemoryBarrier *pImageMemBarriers) {
    // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
    if ((cb_state->activeRenderPass) && (VK_NULL_HANDLE == cb_state->activeFramebuffer) &&
        (VK_COMMAND_BUFFER_LEVEL_SECONDARY == cb_state->createInfo.level)) {
        const auto active_subpass = cb_state->activeSubpass;
        const auto rp_state = cb_state->activeRenderPass;
        const auto &sub_desc = rp_state->createInfo.pSubpasses[active_subpass];
        const VulkanTypedHandle rp_handle(rp_state->renderPass, kVulkanObjectTypeRenderPass);
        for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
            const auto &img_barrier = pImageMemBarriers[i];
            // Secondary CB case w/o FB specified delay validation
            cb_state->cmd_execute_commands_functions.emplace_back([=](const CMD_BUFFER_STATE *primary_cb, VkFramebuffer fb) {
                return ValidateImageBarrierAttachment(func_name, cb_state, fb, active_subpass, sub_desc, rp_handle, i, img_barrier);
            });
        }
    }
}

void CoreChecks::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    const char *func_name = "vkCmdPipelineBarrier";

    RecordBarrierValidationInfo(func_name, cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                pImageMemoryBarriers);

    EnqueueSubmitTimeValidateImageBarrierAttachment(func_name, cb_state, imageMemoryBarrierCount, pImageMemoryBarriers);
    TransitionImageLayouts(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool CoreChecks::ValidateBeginQuery(const CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj, VkFlags flags, CMD_TYPE cmd,
                                    const char *cmd_name, const char *vuid_queue_flags, const char *vuid_queue_feedback,
                                    const char *vuid_queue_occlusion, const char *vuid_precise,
                                    const char *vuid_query_count) const {
    bool skip = false;
    const auto &query_pool_ci = GetQueryPoolState(query_obj.pool)->createInfo;

    // There are tighter queue constraints to test for certain query pools
    if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
        skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT, vuid_queue_feedback);
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_OCCLUSION) {
        skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT, vuid_queue_occlusion);
    }

    skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, vuid_queue_flags);

    if (flags & VK_QUERY_CONTROL_PRECISE_BIT) {
        if (!enabled_features.core.occlusionQueryPrecise) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), vuid_precise,
                            "%s: VK_QUERY_CONTROL_PRECISE_BIT provided, but precise occlusion queries not enabled on the device.",
                            cmd_name);
        }

        if (query_pool_ci.queryType != VK_QUERY_TYPE_OCCLUSION) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), vuid_precise,
                        "%s: VK_QUERY_CONTROL_PRECISE_BIT provided, but pool query type is not VK_QUERY_TYPE_OCCLUSION", cmd_name);
        }
    }

    if (query_obj.query >= query_pool_ci.queryCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), vuid_query_count,
                        "%s: Query index %" PRIu32 " must be less than query count %" PRIu32 " of %s.", cmd_name, query_obj.query,
                        query_pool_ci.queryCount, report_data->FormatHandle(query_obj.pool).c_str());
    }

    skip |= ValidateCmd(cb_state, cmd, cmd_name);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                              VkFlags flags) const {
    if (disabled.query_validation) return false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    QueryObject query_obj(queryPool, slot);
    return ValidateBeginQuery(cb_state, query_obj, flags, CMD_BEGINQUERY, "vkCmdBeginQuery()",
                              "VUID-vkCmdBeginQuery-commandBuffer-cmdpool", "VUID-vkCmdBeginQuery-queryType-02327",
                              "VUID-vkCmdBeginQuery-queryType-00803", "VUID-vkCmdBeginQuery-queryType-00800",
                              "VUID-vkCmdBeginQuery-query-00802");
}

bool CoreChecks::VerifyQueryIsReset(const ValidationStateTracker *state_data, VkCommandBuffer commandBuffer, QueryObject query_obj,
                                    const char *func_name, QueryMap *localQueryToStateMap) {
    bool skip = false;

    QueryState state = state_data->GetQueryState(localQueryToStateMap, query_obj.pool, query_obj.query);
    if (state != QUERYSTATE_RESET) {
        skip |= log_msg(state_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), kVUID_Core_DrawState_QueryNotReset,
                        "%s: %s and query %" PRIu32
                        ": query not reset. "
                        "After query pool creation, each query must be reset before it is used. "
                        "Queries must also be reset between uses.",
                        func_name, state_data->report_data->FormatHandle(query_obj.pool).c_str(), query_obj.query);
    }

    return skip;
}

void CoreChecks::EnqueueVerifyBeginQuery(VkCommandBuffer command_buffer, const QueryObject &query_obj, const char *func_name) {
    CMD_BUFFER_STATE *cb_state = GetCBState(command_buffer);

    // Enqueue the submit time validation here, ahead of the submit time state update in the StateTracker's PostCallRecord
    cb_state->queryUpdates.emplace_back([command_buffer, query_obj, func_name](const ValidationStateTracker *device_data,
                                                                               bool do_validate, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(device_data, command_buffer, query_obj, func_name, localQueryToStateMap);
    });
}

void CoreChecks::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) {
    if (disabled.query_validation) return;
    QueryObject query_obj = {queryPool, slot};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, "vkCmdBeginQuery()");
}

bool CoreChecks::ValidateCmdEndQuery(const CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj, CMD_TYPE cmd,
                                     const char *cmd_name, const char *vuid_queue_flags, const char *vuid_active_queries) const {
    bool skip = false;
    if (!cb_state->activeQueries.count(query_obj)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), vuid_active_queries,
                        "%s: Ending a query before it was started: %s, index %d.", cmd_name,
                        report_data->FormatHandle(query_obj.pool).c_str(), query_obj.query);
    }
    skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, vuid_queue_flags);
    skip |= ValidateCmd(cb_state, cmd, cmd_name);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) const {
    if (disabled.query_validation) return false;
    QueryObject query_obj = {queryPool, slot};
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmdEndQuery(cb_state, query_obj, CMD_ENDQUERY, "vkCmdEndQuery()", "VUID-vkCmdEndQuery-commandBuffer-cmdpool",
                               "VUID-vkCmdEndQuery-None-01923");
}

bool CoreChecks::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) const {
    if (disabled.query_validation) return false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    bool skip = InsideRenderPass(cb_state, "vkCmdResetQueryPool()", "VUID-vkCmdResetQueryPool-renderpass");
    skip |= ValidateCmd(cb_state, CMD_RESETQUERYPOOL, "VkCmdResetQueryPool()");
    skip |= ValidateCmdQueueFlags(cb_state, "VkCmdResetQueryPool()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdResetQueryPool-commandBuffer-cmdpool");
    return skip;
}

static QueryResultType GetQueryResultType(QueryState state, VkQueryResultFlags flags) {
    switch (state) {
        case QUERYSTATE_UNKNOWN:
            return QUERYRESULT_UNKNOWN;
        case QUERYSTATE_RESET:
        case QUERYSTATE_RUNNING:
            if (flags & VK_QUERY_RESULT_WAIT_BIT) {
                return ((state == QUERYSTATE_RESET) ? QUERYRESULT_WAIT_ON_RESET : QUERYRESULT_WAIT_ON_RUNNING);
            } else if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
                return QUERYRESULT_SOME_DATA;
            } else {
                return QUERYRESULT_NO_DATA;
            }
        case QUERYSTATE_ENDED:
            if ((flags & VK_QUERY_RESULT_WAIT_BIT) || (flags & VK_QUERY_RESULT_PARTIAL_BIT) ||
                (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
                return QUERYRESULT_SOME_DATA;
            } else {
                return QUERYRESULT_MAYBE_NO_DATA;
            }
        case QUERYSTATE_AVAILABLE:
            return QUERYRESULT_SOME_DATA;
    }
    assert(false);
    return QUERYRESULT_UNKNOWN;
}

bool CoreChecks::ValidateCopyQueryPoolResults(const ValidationStateTracker *state_data, VkCommandBuffer commandBuffer,
                                              VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                              VkQueryResultFlags flags, QueryMap *localQueryToStateMap) {
    bool skip = false;
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryState state = state_data->GetQueryState(localQueryToStateMap, queryPool, firstQuery + i);
        QueryResultType result_type = GetQueryResultType(state, flags);
        if (result_type != QUERYRESULT_SOME_DATA) {
            skip |= log_msg(state_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), kVUID_Core_DrawState_InvalidQuery,
                            "vkCmdCopyQueryPoolResults(): Requesting a copy from query to buffer on %s query %" PRIu32 ": %s",
                            state_data->report_data->FormatHandle(queryPool).c_str(), firstQuery + i,
                            string_QueryResultType(result_type));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags) const {
    if (disabled.query_validation) return false;
    const auto cb_state = GetCBState(commandBuffer);
    const auto dst_buff_state = GetBufferState(dstBuffer);
    assert(cb_state);
    assert(dst_buff_state);
    bool skip = ValidateMemoryIsBoundToBuffer(dst_buff_state, "vkCmdCopyQueryPoolResults()",
                                              "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00826");
    skip |= ValidateQueryPoolStride("VUID-vkCmdCopyQueryPoolResults-flags-00822", "VUID-vkCmdCopyQueryPoolResults-flags-00823",
                                    stride, "dstOffset", dstOffset, flags);
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(dst_buff_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00825", "vkCmdCopyQueryPoolResults()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdCopyQueryPoolResults()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdCopyQueryPoolResults-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_COPYQUERYPOOLRESULTS, "vkCmdCopyQueryPoolResults()");
    skip |= InsideRenderPass(cb_state, "vkCmdCopyQueryPoolResults()", "VUID-vkCmdCopyQueryPoolResults-renderpass");
    return skip;
}

void CoreChecks::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                      uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                      VkDeviceSize stride, VkQueryResultFlags flags) {
    if (disabled.query_validation) return;
    auto cb_state = GetCBState(commandBuffer);
    cb_state->queryUpdates.emplace_back(
        [commandBuffer, queryPool, firstQuery, queryCount, flags](const ValidationStateTracker *device_data, bool do_validate,
                                                                  QueryMap *localQueryToStateMap) {
            if (!do_validate) return false;
            return ValidateCopyQueryPoolResults(device_data, commandBuffer, queryPool, firstQuery, queryCount, flags,
                                                localQueryToStateMap);
        });
}

bool CoreChecks::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                 const void *pValues) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdPushConstants()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdPushConstants-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_PUSHCONSTANTS, "vkCmdPushConstants()");
    skip |= ValidatePushConstantRange(offset, size, "vkCmdPushConstants()");
    if (0 == stageFlags) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdPushConstants-stageFlags-requiredbitmask",
                        "vkCmdPushConstants() call has no stageFlags set.");
    }

    // Check if pipeline_layout VkPushConstantRange(s) overlapping offset, size have stageFlags set for each stage in the command
    // stageFlags argument, *and* that the command stageFlags argument has bits set for the stageFlags in each overlapping range.
    if (!skip) {
        const auto &ranges = *GetPipelineLayout(layout)->push_constant_ranges;
        VkShaderStageFlags found_stages = 0;
        for (const auto &range : ranges) {
            if ((offset >= range.offset) && (offset + size <= range.offset + range.size)) {
                VkShaderStageFlags matching_stages = range.stageFlags & stageFlags;
                if (matching_stages != range.stageFlags) {
                    // "VUID-vkCmdPushConstants-offset-01796" VUID-vkCmdPushConstants-offset-01796
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-vkCmdPushConstants-offset-01796",
                                    "vkCmdPushConstants(): stageFlags (0x%" PRIx32 ", offset (%" PRIu32 "), and size (%" PRIu32
                                    "),  must contain all stages in overlapping VkPushConstantRange stageFlags (0x%" PRIx32
                                    "), offset (%" PRIu32 "), and size (%" PRIu32 ") in %s.",
                                    (uint32_t)stageFlags, offset, size, (uint32_t)range.stageFlags, range.offset, range.size,
                                    report_data->FormatHandle(layout).c_str());
                }

                // Accumulate all stages we've found
                found_stages = matching_stages | found_stages;
            }
        }
        if (found_stages != stageFlags) {
            // "VUID-vkCmdPushConstants-offset-01795" VUID-vkCmdPushConstants-offset-01795
            uint32_t missing_stages = ~found_stages & stageFlags;
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), "VUID-vkCmdPushConstants-offset-01795",
                            "vkCmdPushConstants(): stageFlags = 0x%" PRIx32
                            ", VkPushConstantRange in %s overlapping offset = %d and size = %d, do not contain "
                            "stageFlags 0x%" PRIx32 ".",
                            (uint32_t)stageFlags, report_data->FormatHandle(layout).c_str(), offset, size, missing_stages);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkQueryPool queryPool, uint32_t slot) const {
    if (disabled.query_validation) return false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdWriteTimestamp()",
                                      VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
                                      "VUID-vkCmdWriteTimestamp-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_WRITETIMESTAMP, "vkCmdWriteTimestamp()");
    return skip;
}

void CoreChecks::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkQueryPool queryPool, uint32_t slot) {
    if (disabled.query_validation) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    QueryObject query = {queryPool, slot};
    const char *func_name = "vkCmdWriteTimestamp()";
    cb_state->queryUpdates.emplace_back([commandBuffer, query, func_name](const ValidationStateTracker *device_data,
                                                                          bool do_validate, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(device_data, commandBuffer, query, func_name, localQueryToStateMap);
    });
}

bool CoreChecks::MatchUsage(uint32_t count, const VkAttachmentReference2KHR *attachments, const VkFramebufferCreateInfo *fbci,
                            VkImageUsageFlagBits usage_flag, const char *error_code) const {
    bool skip = false;

    if (attachments) {
        for (uint32_t attach = 0; attach < count; attach++) {
            if (attachments[attach].attachment != VK_ATTACHMENT_UNUSED) {
                // Attachment counts are verified elsewhere, but prevent an invalid access
                if (attachments[attach].attachment < fbci->attachmentCount) {
                    if ((fbci->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
                        const VkImageView *image_view = &fbci->pAttachments[attachments[attach].attachment];
                        auto view_state = GetImageViewState(*image_view);
                        if (view_state) {
                            const VkImageCreateInfo *ici = &GetImageState(view_state->create_info.image)->createInfo;
                            if (ici != nullptr) {
                                if ((ici->usage & usage_flag) == 0) {
                                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, error_code,
                                                    "vkCreateFramebuffer:  Framebuffer Attachment (%d) conflicts with the image's "
                                                    "IMAGE_USAGE flags (%s).",
                                                    attachments[attach].attachment, string_VkImageUsageFlagBits(usage_flag));
                                }
                            }
                        }
                    } else {
                        const VkFramebufferAttachmentsCreateInfoKHR *fbaci =
                            lvl_find_in_chain<VkFramebufferAttachmentsCreateInfoKHR>(fbci->pNext);
                        if (fbaci != nullptr && fbaci->pAttachmentImageInfos != nullptr &&
                            fbaci->attachmentImageInfoCount > attachments[attach].attachment) {
                            uint32_t image_usage = fbaci->pAttachmentImageInfos[attachments[attach].attachment].usage;
                            if ((image_usage & usage_flag) == 0) {
                                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                                                0, error_code,
                                                "vkCreateFramebuffer:  Framebuffer attachment info (%d) conflicts with the image's "
                                                "IMAGE_USAGE flags (%s).",
                                                attachments[attach].attachment, string_VkImageUsageFlagBits(usage_flag));
                            }
                        }
                    }
                }
            }
        }
    }
    return skip;
}

// Validate VkFramebufferCreateInfo which includes:
// 1. attachmentCount equals renderPass attachmentCount
// 2. corresponding framebuffer and renderpass attachments have matching formats
// 3. corresponding framebuffer and renderpass attachments have matching sample counts
// 4. fb attachments only have a single mip level
// 5. fb attachment dimensions are each at least as large as the fb
// 6. fb attachments use idenity swizzle
// 7. fb attachments used by renderPass for color/input/ds have correct usage bit set
// 8. fb dimensions are within physical device limits
bool CoreChecks::ValidateFramebufferCreateInfo(const VkFramebufferCreateInfo *pCreateInfo) const {
    bool skip = false;

    const VkFramebufferAttachmentsCreateInfoKHR *pFramebufferAttachmentsCreateInfo =
        lvl_find_in_chain<VkFramebufferAttachmentsCreateInfoKHR>(pCreateInfo->pNext);
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) != 0) {
        if (!enabled_features.imageless_framebuffer_features.imagelessFramebuffer) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-flags-03189",
                        "vkCreateFramebuffer(): VkFramebufferCreateInfo flags includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR, "
                        "but the imagelessFramebuffer feature is not enabled.");
        }

        if (pFramebufferAttachmentsCreateInfo == nullptr) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-flags-03190",
                        "vkCreateFramebuffer(): VkFramebufferCreateInfo flags includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR, "
                        "but no instance of VkFramebufferAttachmentsCreateInfoKHR is present in the pNext chain.");
        } else {
            if (pFramebufferAttachmentsCreateInfo->attachmentImageInfoCount != 0 &&
                pFramebufferAttachmentsCreateInfo->attachmentImageInfoCount != pCreateInfo->attachmentCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkFramebufferCreateInfo-flags-03191",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachmentCount is %u, but "
                                "VkFramebufferAttachmentsCreateInfoKHR attachmentImageInfoCount is %u.",
                                pCreateInfo->attachmentCount, pFramebufferAttachmentsCreateInfo->attachmentImageInfoCount);
            }
        }
    }

    auto rp_state = GetRenderPassState(pCreateInfo->renderPass);
    if (rp_state) {
        const VkRenderPassCreateInfo2KHR *rpci = rp_state->createInfo.ptr();
        if (rpci->attachmentCount != pCreateInfo->attachmentCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(pCreateInfo->renderPass), "VUID-VkFramebufferCreateInfo-attachmentCount-00876",
                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachmentCount of %u does not match attachmentCount "
                            "of %u of %s being used to create Framebuffer.",
                            pCreateInfo->attachmentCount, rpci->attachmentCount,
                            report_data->FormatHandle(pCreateInfo->renderPass).c_str());
        } else {
            // attachmentCounts match, so make sure corresponding attachment details line up
            if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
                const VkImageView *image_views = pCreateInfo->pAttachments;
                for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
                    auto view_state = GetImageViewState(image_views[i]);
                    if (view_state == nullptr) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                                    HandleToUint64(image_views[i]), "VUID-VkFramebufferCreateInfo-flags-03188",
                                    "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u is not a valid VkImageView.", i);
                    } else {
                        auto &ivci = view_state->create_info;
                        if (ivci.format != rpci->pAttachments[i].format) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                HandleToUint64(pCreateInfo->renderPass), "VUID-VkFramebufferCreateInfo-pAttachments-00880",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has format of %s that does not "
                                "match the format of %s used by the corresponding attachment for %s.",
                                i, string_VkFormat(ivci.format), string_VkFormat(rpci->pAttachments[i].format),
                                report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                        }
                        const VkImageCreateInfo *ici = &GetImageState(ivci.image)->createInfo;
                        if (ici->samples != rpci->pAttachments[i].samples) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pCreateInfo->renderPass), "VUID-VkFramebufferCreateInfo-pAttachments-00881",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has %s samples that do not "
                                        "match the %s "
                                        "samples used by the corresponding attachment for %s.",
                                        i, string_VkSampleCountFlagBits(ici->samples),
                                        string_VkSampleCountFlagBits(rpci->pAttachments[i].samples),
                                        report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                        }
                        // Verify that view only has a single mip level
                        if (ivci.subresourceRange.levelCount != 1) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkFramebufferCreateInfo-pAttachments-00883",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has mip levelCount of %u but "
                                "only a single mip level (levelCount ==  1) is allowed when creating a Framebuffer.",
                                i, ivci.subresourceRange.levelCount);
                        }
                        const uint32_t mip_level = ivci.subresourceRange.baseMipLevel;
                        uint32_t mip_width = max(1u, ici->extent.width >> mip_level);
                        uint32_t mip_height = max(1u, ici->extent.height >> mip_level);
                        if (!(rpci->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT ||
                              rpci->pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT)) {
                            if ((ivci.subresourceRange.layerCount < pCreateInfo->layers) || (mip_width < pCreateInfo->width) ||
                                (mip_height < pCreateInfo->height)) {
                                skip |= log_msg(
                                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkFramebufferCreateInfo-pAttachments-00882",
                                    "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has dimensions "
                                    "smaller than the corresponding framebuffer dimensions. Here are the respective dimensions for "
                                    "attachment #%u, framebuffer:\n"
                                    "width: %u, %u\n"
                                    "height: %u, %u\n"
                                    "layerCount: %u, %u\n",
                                    i, ivci.subresourceRange.baseMipLevel, i, mip_width, pCreateInfo->width, mip_height,
                                    pCreateInfo->height, ivci.subresourceRange.layerCount, pCreateInfo->layers);
                            }
                        } else {
                            if (device_extensions.vk_ext_fragment_density_map) {
                                uint32_t ceiling_width = (uint32_t)ceil(
                                    (float)pCreateInfo->width /
                                    std::max((float)phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width,
                                             1.0f));
                                if (mip_width < ceiling_width) {
                                    skip |= log_msg(
                                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkFramebufferCreateInfo-pAttachments-02555",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has width "
                                        "smaller than the corresponding the ceiling of framebuffer width / "
                                        "maxFragmentDensityTexelSize.width "
                                        "Here are the respective dimensions for attachment #%u, the ceiling value:\n "
                                        "attachment #%u, framebuffer:\n"
                                        "width: %u, the ceiling value: %u\n",
                                        i, ivci.subresourceRange.baseMipLevel, i, i, mip_width, ceiling_width);
                                }
                                uint32_t ceiling_height = (uint32_t)ceil(
                                    (float)pCreateInfo->height /
                                    std::max(
                                        (float)phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height,
                                        1.0f));
                                if (mip_height < ceiling_height) {
                                    skip |= log_msg(
                                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkFramebufferCreateInfo-pAttachments-02556",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has height "
                                        "smaller than the corresponding the ceiling of framebuffer height / "
                                        "maxFragmentDensityTexelSize.height "
                                        "Here are the respective dimensions for attachment #%u, the ceiling value:\n "
                                        "attachment #%u, framebuffer:\n"
                                        "height: %u, the ceiling value: %u\n",
                                        i, ivci.subresourceRange.baseMipLevel, i, i, mip_height, ceiling_height);
                                }
                            }
                        }
                        if (((ivci.components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                             (ivci.components.r != VK_COMPONENT_SWIZZLE_R)) ||
                            ((ivci.components.g != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                             (ivci.components.g != VK_COMPONENT_SWIZZLE_G)) ||
                            ((ivci.components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                             (ivci.components.b != VK_COMPONENT_SWIZZLE_B)) ||
                            ((ivci.components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                             (ivci.components.a != VK_COMPONENT_SWIZZLE_A))) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkFramebufferCreateInfo-pAttachments-00884",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has non-identy swizzle. All "
                                "framebuffer attachments must have been created with the identity swizzle. Here are the actual "
                                "swizzle values:\n"
                                "r swizzle = %s\n"
                                "g swizzle = %s\n"
                                "b swizzle = %s\n"
                                "a swizzle = %s\n",
                                i, string_VkComponentSwizzle(ivci.components.r), string_VkComponentSwizzle(ivci.components.g),
                                string_VkComponentSwizzle(ivci.components.b), string_VkComponentSwizzle(ivci.components.a));
                        }
                    }
                }
            } else if (pFramebufferAttachmentsCreateInfo) {
                // VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR is set
                for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
                    auto &aii = pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[i];
                    bool formatFound = false;
                    for (uint32_t j = 0; j < aii.viewFormatCount; ++j) {
                        if (aii.pViewFormats[j] == rpci->pAttachments[i].format) {
                            formatFound = true;
                        }
                    }
                    if (!formatFound) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pCreateInfo->renderPass), "VUID-VkFramebufferCreateInfo-flags-03205",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u does not include "
                                        "format %s used "
                                        "by the corresponding attachment for renderPass (%s).",
                                        i, string_VkFormat(rpci->pAttachments[i].format),
                                        report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                    }

                    const char *mismatchedLayersNoMultiviewVuid = device_extensions.vk_khr_multiview
                                                                      ? "VUID-VkFramebufferCreateInfo-renderPass-03199"
                                                                      : "VUID-VkFramebufferCreateInfo-flags-03200";
                    if ((rpci->subpassCount == 0) || (rpci->pSubpasses[0].viewMask == 0)) {
                        if (aii.layerCount < pCreateInfo->layers) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        mismatchedLayersNoMultiviewVuid,
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has only #%u layers, "
                                        "but framebuffer has #%u layers.",
                                        i, aii.layerCount, pCreateInfo->layers);
                        }
                    }

                    if (!device_extensions.vk_ext_fragment_density_map) {
                        if (aii.width < pCreateInfo->width) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkFramebufferCreateInfo-flags-03192",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has a width of only #%u, "
                                "but framebuffer has a width of #%u.",
                                i, aii.width, pCreateInfo->width);
                        }

                        if (aii.height < pCreateInfo->height) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkFramebufferCreateInfo-flags-03193",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has a height of only #%u, "
                                "but framebuffer has a height of #%u.",
                                i, aii.height, pCreateInfo->height);
                        }
                    }
                }

                // Validate image usage
                uint32_t attachment_index = VK_ATTACHMENT_UNUSED;
                for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
                    skip |= MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pColorAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201");
                    skip |=
                        MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pResolveAttachments, pCreateInfo,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201");
                    skip |= MatchUsage(1, rpci->pSubpasses[i].pDepthStencilAttachment, pCreateInfo,
                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03202");
                    skip |= MatchUsage(rpci->pSubpasses[i].inputAttachmentCount, rpci->pSubpasses[i].pInputAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03204");

                    const VkSubpassDescriptionDepthStencilResolveKHR *pDepthStencilResolve =
                        lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolveKHR>(rpci->pSubpasses[i].pNext);
                    if (device_extensions.vk_khr_depth_stencil_resolve && pDepthStencilResolve != nullptr) {
                        skip |= MatchUsage(1, pDepthStencilResolve->pDepthStencilResolveAttachment, pCreateInfo,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03203");
                    }
                }

                if (device_extensions.vk_khr_multiview) {
                    if ((rpci->subpassCount > 0) && (rpci->pSubpasses[0].viewMask != 0)) {
                        for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
                            const VkSubpassDescriptionDepthStencilResolveKHR *pDepthStencilResolve =
                                lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolveKHR>(rpci->pSubpasses[i].pNext);
                            uint32_t view_bits = rpci->pSubpasses[i].viewMask;
                            uint32_t highest_view_bit = 0;

                            for (int j = 0; j < 32; ++j) {
                                if (((view_bits >> j) & 1) != 0) {
                                    highest_view_bit = j;
                                }
                            }

                            for (uint32_t j = 0; j < rpci->pSubpasses[i].colorAttachmentCount; ++j) {
                                attachment_index = rpci->pSubpasses[i].pColorAttachments[j].attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    uint32_t layer_count =
                                        pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= log_msg(
                                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                            HandleToUint64(pCreateInfo->renderPass),
                                            "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %u layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %u, with that attachment specified as a color attachment %u.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                    }
                                }
                                if (rpci->pSubpasses[i].pResolveAttachments) {
                                    attachment_index = rpci->pSubpasses[i].pResolveAttachments[j].attachment;
                                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                        uint32_t layer_count =
                                            pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[attachment_index].layerCount;
                                        if (layer_count <= highest_view_bit) {
                                            skip |= log_msg(
                                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                                HandleToUint64(pCreateInfo->renderPass),
                                                "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                                "only specifies %u layers, but the view mask for subpass %u in renderPass (%s) "
                                                "includes layer %u, with that attachment specified as a resolve attachment %u.",
                                                attachment_index, layer_count, i,
                                                report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                        }
                                    }
                                }
                            }

                            for (uint32_t j = 0; j < rpci->pSubpasses[i].inputAttachmentCount; ++j) {
                                attachment_index = rpci->pSubpasses[i].pInputAttachments[j].attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    uint32_t layer_count =
                                        pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= log_msg(
                                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                            HandleToUint64(pCreateInfo->renderPass),
                                            "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %u layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %u, with that attachment specified as an input attachment %u.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                    }
                                }
                            }

                            if (rpci->pSubpasses[i].pDepthStencilAttachment != nullptr) {
                                attachment_index = rpci->pSubpasses[i].pDepthStencilAttachment->attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    uint32_t layer_count =
                                        pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= log_msg(
                                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                            HandleToUint64(pCreateInfo->renderPass),
                                            "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %u layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %u, with that attachment specified as a depth/stencil attachment.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
                                    }
                                }

                                if (device_extensions.vk_khr_depth_stencil_resolve && pDepthStencilResolve != nullptr &&
                                    pDepthStencilResolve->pDepthStencilResolveAttachment != nullptr) {
                                    attachment_index = pDepthStencilResolve->pDepthStencilResolveAttachment->attachment;
                                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                        uint32_t layer_count =
                                            pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[attachment_index].layerCount;
                                        if (layer_count <= highest_view_bit) {
                                            skip |= log_msg(
                                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                                HandleToUint64(pCreateInfo->renderPass),
                                                "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                                "only specifies %u layers, but the view mask for subpass %u in renderPass (%s) "
                                                "includes layer %u, with that attachment specified as a depth/stencil resolve "
                                                "attachment.",
                                                attachment_index, layer_count, i,
                                                report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
                // Verify correct attachment usage flags
                for (uint32_t subpass = 0; subpass < rpci->subpassCount; subpass++) {
                    // Verify input attachments:
                    skip |= MatchUsage(rpci->pSubpasses[subpass].inputAttachmentCount, rpci->pSubpasses[subpass].pInputAttachments,
                                       pCreateInfo, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                       "VUID-VkFramebufferCreateInfo-pAttachments-00879");
                    // Verify color attachments:
                    skip |= MatchUsage(rpci->pSubpasses[subpass].colorAttachmentCount, rpci->pSubpasses[subpass].pColorAttachments,
                                       pCreateInfo, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                       "VUID-VkFramebufferCreateInfo-pAttachments-00877");
                    // Verify depth/stencil attachments:
                    skip |=
                        MatchUsage(1, rpci->pSubpasses[subpass].pDepthStencilAttachment, pCreateInfo,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-02633");
                }
            }
        }
    }
    // Verify FB dimensions are within physical device limits
    if (pCreateInfo->width > phys_dev_props.limits.maxFramebufferWidth) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-width-00886",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo width exceeds physical device limits. Requested "
                        "width: %u, device max: %u\n",
                        pCreateInfo->width, phys_dev_props.limits.maxFramebufferWidth);
    }
    if (pCreateInfo->height > phys_dev_props.limits.maxFramebufferHeight) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-height-00888",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo height exceeds physical device limits. Requested "
                        "height: %u, device max: %u\n",
                        pCreateInfo->height, phys_dev_props.limits.maxFramebufferHeight);
    }
    if (pCreateInfo->layers > phys_dev_props.limits.maxFramebufferLayers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-layers-00890",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo layers exceeds physical device limits. Requested "
                        "layers: %u, device max: %u\n",
                        pCreateInfo->layers, phys_dev_props.limits.maxFramebufferLayers);
    }
    // Verify FB dimensions are greater than zero
    if (pCreateInfo->width <= 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-width-00885",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo width must be greater than zero.");
    }
    if (pCreateInfo->height <= 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-height-00887",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo height must be greater than zero.");
    }
    if (pCreateInfo->layers <= 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkFramebufferCreateInfo-layers-00889",
                        "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo layers must be greater than zero.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) const {
    // TODO : Verify that renderPass FB is created with is compatible with FB
    bool skip = false;
    skip |= ValidateFramebufferCreateInfo(pCreateInfo);
    return skip;
}

static bool FindDependency(const uint32_t index, const uint32_t dependent, const std::vector<DAGNode> &subpass_to_node,
                           std::unordered_set<uint32_t> &processed_nodes) {
    // If we have already checked this node we have not found a dependency path so return false.
    if (processed_nodes.count(index)) return false;
    processed_nodes.insert(index);
    const DAGNode &node = subpass_to_node[index];
    // Look for a dependency path. If one exists return true else recurse on the previous nodes.
    if (std::find(node.prev.begin(), node.prev.end(), dependent) == node.prev.end()) {
        for (auto elem : node.prev) {
            if (FindDependency(elem, dependent, subpass_to_node, processed_nodes)) return true;
        }
    } else {
        return true;
    }
    return false;
}

bool CoreChecks::IsImageLayoutReadOnly(VkImageLayout layout) const {
    if ((layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) || (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
        (layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
        (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)) {
        return true;
    }
    return false;
}

bool CoreChecks::CheckDependencyExists(const uint32_t subpass, const VkImageLayout layout,
                                       const std::vector<SubpassLayout> &dependent_subpasses,
                                       const std::vector<DAGNode> &subpass_to_node, bool &skip) const {
    bool result = true;
    bool bImageLayoutReadOnly = IsImageLayoutReadOnly(layout);
    // Loop through all subpasses that share the same attachment and make sure a dependency exists
    for (uint32_t k = 0; k < dependent_subpasses.size(); ++k) {
        const SubpassLayout &sp = dependent_subpasses[k];
        if (subpass == sp.index) continue;
        if (bImageLayoutReadOnly && IsImageLayoutReadOnly(sp.layout)) continue;

        const DAGNode &node = subpass_to_node[subpass];
        // Check for a specified dependency between the two nodes. If one exists we are done.
        auto prev_elem = std::find(node.prev.begin(), node.prev.end(), sp.index);
        auto next_elem = std::find(node.next.begin(), node.next.end(), sp.index);
        if (prev_elem == node.prev.end() && next_elem == node.next.end()) {
            // If no dependency exits an implicit dependency still might. If not, throw an error.
            std::unordered_set<uint32_t> processed_nodes;
            if (!(FindDependency(subpass, sp.index, subpass_to_node, processed_nodes) ||
                  FindDependency(sp.index, subpass, subpass_to_node, processed_nodes))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_InvalidRenderpass,
                                "A dependency between subpasses %d and %d must exist but one is not specified.", subpass, sp.index);
                result = false;
            }
        }
    }
    return result;
}

bool CoreChecks::CheckPreserved(const VkRenderPassCreateInfo2KHR *pCreateInfo, const int index, const uint32_t attachment,
                                const std::vector<DAGNode> &subpass_to_node, int depth, bool &skip) const {
    const DAGNode &node = subpass_to_node[index];
    // If this node writes to the attachment return true as next nodes need to preserve the attachment.
    const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[index];
    for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
        if (attachment == subpass.pColorAttachments[j].attachment) return true;
    }
    for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
        if (attachment == subpass.pInputAttachments[j].attachment) return true;
    }
    if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
        if (attachment == subpass.pDepthStencilAttachment->attachment) return true;
    }
    bool result = false;
    // Loop through previous nodes and see if any of them write to the attachment.
    for (auto elem : node.prev) {
        result |= CheckPreserved(pCreateInfo, elem, attachment, subpass_to_node, depth + 1, skip);
    }
    // If the attachment was written to by a previous node than this node needs to preserve it.
    if (result && depth > 0) {
        bool has_preserved = false;
        for (uint32_t j = 0; j < subpass.preserveAttachmentCount; ++j) {
            if (subpass.pPreserveAttachments[j] == attachment) {
                has_preserved = true;
                break;
            }
        }
        if (!has_preserved) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_DrawState_InvalidRenderpass,
                            "Attachment %d is used by a later subpass and must be preserved in subpass %d.", attachment, index);
        }
    }
    return result;
}

template <class T>
bool IsRangeOverlapping(T offset1, T size1, T offset2, T size2) {
    return (((offset1 + size1) > offset2) && ((offset1 + size1) < (offset2 + size2))) ||
           ((offset1 > offset2) && (offset1 < (offset2 + size2)));
}

bool IsRegionOverlapping(VkImageSubresourceRange range1, VkImageSubresourceRange range2) {
    return (IsRangeOverlapping(range1.baseMipLevel, range1.levelCount, range2.baseMipLevel, range2.levelCount) &&
            IsRangeOverlapping(range1.baseArrayLayer, range1.layerCount, range2.baseArrayLayer, range2.layerCount));
}

bool CoreChecks::ValidateDependencies(FRAMEBUFFER_STATE const *framebuffer, RENDER_PASS_STATE const *renderPass) const {
    bool skip = false;
    auto const pFramebufferInfo = framebuffer->createInfo.ptr();
    auto const pCreateInfo = renderPass->createInfo.ptr();
    auto const &subpass_to_node = renderPass->subpassToNode;

    struct Attachment {
        std::vector<SubpassLayout> outputs;
        std::vector<SubpassLayout> inputs;
        std::vector<uint32_t> overlapping;
    };

    std::vector<Attachment> attachments(pCreateInfo->attachmentCount);

    // Find overlapping attachments
    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        for (uint32_t j = i + 1; j < pCreateInfo->attachmentCount; ++j) {
            VkImageView viewi = pFramebufferInfo->pAttachments[i];
            VkImageView viewj = pFramebufferInfo->pAttachments[j];
            if (viewi == viewj) {
                attachments[i].overlapping.emplace_back(j);
                attachments[j].overlapping.emplace_back(i);
                continue;
            }
            auto view_state_i = GetImageViewState(viewi);
            auto view_state_j = GetImageViewState(viewj);
            if (!view_state_i || !view_state_j) {
                continue;
            }
            auto view_ci_i = view_state_i->create_info;
            auto view_ci_j = view_state_j->create_info;
            if (view_ci_i.image == view_ci_j.image && IsRegionOverlapping(view_ci_i.subresourceRange, view_ci_j.subresourceRange)) {
                attachments[i].overlapping.emplace_back(j);
                attachments[j].overlapping.emplace_back(i);
                continue;
            }
            auto image_data_i = GetImageState(view_ci_i.image);
            auto image_data_j = GetImageState(view_ci_j.image);
            if (!image_data_i || !image_data_j) {
                continue;
            }
            if (image_data_i->binding.mem == image_data_j->binding.mem &&
                IsRangeOverlapping(image_data_i->binding.offset, image_data_i->binding.size, image_data_j->binding.offset,
                                   image_data_j->binding.size)) {
                attachments[i].overlapping.emplace_back(j);
                attachments[j].overlapping.emplace_back(i);
            }
        }
    }
    // Find for each attachment the subpasses that use them.
    unordered_set<uint32_t> attachmentIndices;
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];
        attachmentIndices.clear();
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            uint32_t attachment = subpass.pInputAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            SubpassLayout sp = {i, subpass.pInputAttachments[j].layout};
            attachments[attachment].inputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].inputs.emplace_back(sp);
            }
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            SubpassLayout sp = {i, subpass.pColorAttachments[j].layout};
            attachments[attachment].outputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].outputs.emplace_back(sp);
            }
            attachmentIndices.insert(attachment);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            uint32_t attachment = subpass.pDepthStencilAttachment->attachment;
            SubpassLayout sp = {i, subpass.pDepthStencilAttachment->layout};
            attachments[attachment].outputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].outputs.emplace_back(sp);
            }

            if (attachmentIndices.count(attachment)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_DrawState_InvalidRenderpass,
                            "Cannot use same attachment (%u) as both color and depth output in same subpass (%u).", attachment, i);
            }
        }
    }
    // If there is a dependency needed make sure one exists
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];
        // If the attachment is an input then all subpasses that output must have a dependency relationship
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            uint32_t attachment = subpass.pInputAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            CheckDependencyExists(i, subpass.pInputAttachments[j].layout, attachments[attachment].outputs, subpass_to_node, skip);
        }
        // If the attachment is an output then all subpasses that use the attachment must have a dependency relationship
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            CheckDependencyExists(i, subpass.pColorAttachments[j].layout, attachments[attachment].outputs, subpass_to_node, skip);
            CheckDependencyExists(i, subpass.pColorAttachments[j].layout, attachments[attachment].inputs, subpass_to_node, skip);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            const uint32_t &attachment = subpass.pDepthStencilAttachment->attachment;
            CheckDependencyExists(i, subpass.pDepthStencilAttachment->layout, attachments[attachment].outputs, subpass_to_node,
                                  skip);
            CheckDependencyExists(i, subpass.pDepthStencilAttachment->layout, attachments[attachment].inputs, subpass_to_node,
                                  skip);
        }
    }
    // Loop through implicit dependencies, if this pass reads make sure the attachment is preserved for all passes after it was
    // written.
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            CheckPreserved(pCreateInfo, i, subpass.pInputAttachments[j].attachment, subpass_to_node, 0, skip);
        }
    }
    return skip;
}

bool CoreChecks::ValidateRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR *pCreateInfo) const {
    bool skip = false;
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const VkSubpassDependency2KHR &dependency = pCreateInfo->pDependencies[i];
        VkPipelineStageFlagBits latest_src_stage = GetLogicallyLatestGraphicsPipelineStage(dependency.srcStageMask);
        VkPipelineStageFlagBits earliest_dst_stage = GetLogicallyEarliestGraphicsPipelineStage(dependency.dstStageMask);

        // The first subpass here serves as a good proxy for "is multiview enabled" - since all view masks need to be non-zero if
        // any are, which enables multiview.
        if (use_rp2 && (dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) && (pCreateInfo->pSubpasses[0].viewMask == 0)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkRenderPassCreateInfo2KHR-viewMask-03059",
                "Dependency %u specifies the VK_DEPENDENCY_VIEW_LOCAL_BIT, but multiview is not enabled for this render pass.", i);
        } else if (use_rp2 && !(dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) && dependency.viewOffset != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDependency2KHR-dependencyFlags-03092",
                            "Dependency %u specifies the VK_DEPENDENCY_VIEW_LOCAL_BIT, but also specifies a view offset of %u.", i,
                            dependency.viewOffset);
        } else if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL || dependency.dstSubpass == VK_SUBPASS_EXTERNAL) {
            if (dependency.srcSubpass == dependency.dstSubpass) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcSubpass-03085" : "VUID-VkSubpassDependency-srcSubpass-00865";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "The src and dst subpasses in dependency %u are both external.", i);
            } else if (dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
                if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL) {
                    vuid = "VUID-VkSubpassDependency-dependencyFlags-02520";
                } else {  // dependency.dstSubpass == VK_SUBPASS_EXTERNAL
                    vuid = "VUID-VkSubpassDependency-dependencyFlags-02521";
                }
                if (use_rp2) {
                    // Create render pass 2 distinguishes between source and destination external dependencies.
                    if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL) {
                        vuid = "VUID-VkSubpassDependency2KHR-dependencyFlags-03090";
                    } else {
                        vuid = "VUID-VkSubpassDependency2KHR-dependencyFlags-03091";
                    }
                }
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "Dependency %u specifies an external dependency but also specifies VK_DEPENDENCY_VIEW_LOCAL_BIT.", i);
            }
        } else if (dependency.srcSubpass > dependency.dstSubpass) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcSubpass-03084" : "VUID-VkSubpassDependency-srcSubpass-00864";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "Dependency %u specifies a dependency from a later subpass (%u) to an earlier subpass (%u), which is "
                            "disallowed to prevent cyclic dependencies.",
                            i, dependency.srcSubpass, dependency.dstSubpass);
        } else if (dependency.srcSubpass == dependency.dstSubpass) {
            if (dependency.viewOffset != 0) {
                vuid = use_rp2 ? kVUID_Core_DrawState_InvalidRenderpass : "VUID-VkRenderPassCreateInfo-pNext-01930";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "Dependency %u specifies a self-dependency but has a non-zero view offset of %u", i,
                                dependency.viewOffset);
            } else if ((dependency.dependencyFlags | VK_DEPENDENCY_VIEW_LOCAL_BIT) != dependency.dependencyFlags &&
                       pCreateInfo->pSubpasses[dependency.srcSubpass].viewMask > 1) {
                vuid =
                    use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03060" : "VUID-VkSubpassDependency-srcSubpass-00872";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "Dependency %u specifies a self-dependency for subpass %u with a non-zero view mask, but does not "
                                "specify VK_DEPENDENCY_VIEW_LOCAL_BIT.",
                                i, dependency.srcSubpass);
            } else if ((HasNonFramebufferStagePipelineStageFlags(dependency.srcStageMask) ||
                        HasNonFramebufferStagePipelineStageFlags(dependency.dstStageMask)) &&
                       (GetGraphicsPipelineStageLogicalOrdinal(latest_src_stage) >
                        GetGraphicsPipelineStageLogicalOrdinal(earliest_dst_stage))) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcSubpass-03087" : "VUID-VkSubpassDependency-srcSubpass-00867";
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                    "Dependency %u specifies a self-dependency from logically-later stage (%s) to a logically-earlier stage (%s).",
                    i, string_VkPipelineStageFlagBits(latest_src_stage), string_VkPipelineStageFlagBits(earliest_dst_stage));
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateAttachmentIndex(RenderPassCreateVersion rp_version, uint32_t attachment, uint32_t attachment_count,
                                         const char *type) const {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    if (attachment >= attachment_count && attachment != VK_ATTACHMENT_UNUSED) {
        const char *vuid =
            use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-attachment-03051" : "VUID-VkRenderPassCreateInfo-attachment-00834";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "%s: %s attachment %d must be less than the total number of attachments %d.", type, function_name,
                        attachment, attachment_count);
    }
    return skip;
}

enum AttachmentType {
    ATTACHMENT_COLOR = 1,
    ATTACHMENT_DEPTH = 2,
    ATTACHMENT_INPUT = 4,
    ATTACHMENT_PRESERVE = 8,
    ATTACHMENT_RESOLVE = 16,
};

char const *StringAttachmentType(uint8_t type) {
    switch (type) {
        case ATTACHMENT_COLOR:
            return "color";
        case ATTACHMENT_DEPTH:
            return "depth";
        case ATTACHMENT_INPUT:
            return "input";
        case ATTACHMENT_PRESERVE:
            return "preserve";
        case ATTACHMENT_RESOLVE:
            return "resolve";
        default:
            return "(multiple)";
    }
}

bool CoreChecks::AddAttachmentUse(RenderPassCreateVersion rp_version, uint32_t subpass, std::vector<uint8_t> &attachment_uses,
                                  std::vector<VkImageLayout> &attachment_layouts, uint32_t attachment, uint8_t new_use,
                                  VkImageLayout new_layout) const {
    if (attachment >= attachment_uses.size()) return false; /* out of range, but already reported */

    bool skip = false;
    auto &uses = attachment_uses[attachment];
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    if (uses & new_use) {
        if (attachment_layouts[attachment] != new_layout) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-layout-02528" : "VUID-VkSubpassDescription-layout-02519";
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                    "%s: subpass %u already uses attachment %u with a different image layout (%s vs %s).", function_name, subpass,
                    attachment, string_VkImageLayout(attachment_layouts[attachment]), string_VkImageLayout(new_layout));
        }
    } else if (uses & ~ATTACHMENT_INPUT || (uses && (new_use == ATTACHMENT_RESOLVE || new_use == ATTACHMENT_PRESERVE))) {
        /* Note: input attachments are assumed to be done first. */
        vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pPreserveAttachments-03074"
                       : "VUID-VkSubpassDescription-pPreserveAttachments-00854";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "%s: subpass %u uses attachment %u as both %s and %s attachment.", function_name, subpass, attachment,
                        StringAttachmentType(uses), StringAttachmentType(new_use));
    } else {
        attachment_layouts[attachment] = new_layout;
        uses |= new_use;
    }

    return skip;
}

bool CoreChecks::ValidateRenderpassAttachmentUsage(RenderPassCreateVersion rp_version,
                                                   const VkRenderPassCreateInfo2KHR *pCreateInfo) const {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];
        std::vector<uint8_t> attachment_uses(pCreateInfo->attachmentCount);
        std::vector<VkImageLayout> attachment_layouts(pCreateInfo->attachmentCount);

        if (subpass.pipelineBindPoint != VK_PIPELINE_BIND_POINT_GRAPHICS) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pipelineBindPoint-03062"
                           : "VUID-VkSubpassDescription-pipelineBindPoint-00844";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s: Pipeline bind point for subpass %d must be VK_PIPELINE_BIND_POINT_GRAPHICS.", function_name, i);
        }

        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            auto const &attachment_ref = subpass.pInputAttachments[j];
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                skip |= ValidateAttachmentIndex(rp_version, attachment_ref.attachment, pCreateInfo->attachmentCount, "Input");

                if (attachment_ref.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
                    vuid =
                        use_rp2 ? kVUID_Core_DrawState_InvalidRenderpass : "VUID-VkInputAttachmentAspectReference-aspectMask-01964";
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "%s: Aspect mask for input attachment reference %d in subpass %d includes VK_IMAGE_ASPECT_METADATA_BIT.",
                        function_name, i, j);
                }

                if (attachment_ref.attachment < pCreateInfo->attachmentCount) {
                    skip |= AddAttachmentUse(rp_version, i, attachment_uses, attachment_layouts, attachment_ref.attachment,
                                             ATTACHMENT_INPUT, attachment_ref.layout);

                    vuid = use_rp2 ? kVUID_Core_DrawState_InvalidRenderpass : "VUID-VkRenderPassCreateInfo-pNext-01963";
                    skip |= ValidateImageAspectMask(VK_NULL_HANDLE, pCreateInfo->pAttachments[attachment_ref.attachment].format,
                                                    attachment_ref.aspectMask, function_name, vuid);
                }

                if (rp_version == RENDER_PASS_VERSION_2) {
                    // These are validated automatically as part of parameter validation for create renderpass 1
                    // as they are in a struct that only applies to input attachments - not so for v2.

                    // Check for 0
                    if (attachment_ref.aspectMask == 0) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkSubpassDescription2KHR-attachment-02800",
                                        "%s:  Input attachment (%d) aspect mask must not be 0.", function_name, j);
                    } else {
                        const VkImageAspectFlags valid_bits =
                            (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT |
                             VK_IMAGE_ASPECT_METADATA_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT |
                             VK_IMAGE_ASPECT_PLANE_2_BIT | VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT |
                             VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT |
                             VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT);

                        // Check for valid aspect mask bits
                        if (attachment_ref.aspectMask & ~valid_bits) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                            "VUID-VkSubpassDescription2KHR-attachment-02799",
                                            "%s:  Input attachment (%d) aspect mask (0x%" PRIx32 ")is invalid.", function_name, j,
                                            attachment_ref.aspectMask);
                        }
                    }
                }
            }
        }

        for (uint32_t j = 0; j < subpass.preserveAttachmentCount; ++j) {
            uint32_t attachment = subpass.pPreserveAttachments[j];
            if (attachment == VK_ATTACHMENT_UNUSED) {
                vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-attachment-03073" : "VUID-VkSubpassDescription-attachment-00853";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "%s:  Preserve attachment (%d) must not be VK_ATTACHMENT_UNUSED.", function_name, j);
            } else {
                skip |= ValidateAttachmentIndex(rp_version, attachment, pCreateInfo->attachmentCount, "Preserve");
                if (attachment < pCreateInfo->attachmentCount) {
                    skip |= AddAttachmentUse(rp_version, i, attachment_uses, attachment_layouts, attachment, ATTACHMENT_PRESERVE,
                                             VkImageLayout(0) /* preserve doesn't have any layout */);
                }
            }
        }

        bool subpass_performs_resolve = false;

        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            if (subpass.pResolveAttachments) {
                auto const &attachment_ref = subpass.pResolveAttachments[j];
                if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                    skip |= ValidateAttachmentIndex(rp_version, attachment_ref.attachment, pCreateInfo->attachmentCount, "Resolve");

                    if (attachment_ref.attachment < pCreateInfo->attachmentCount) {
                        skip |= AddAttachmentUse(rp_version, i, attachment_uses, attachment_layouts, attachment_ref.attachment,
                                                 ATTACHMENT_RESOLVE, attachment_ref.layout);

                        subpass_performs_resolve = true;

                        if (pCreateInfo->pAttachments[attachment_ref.attachment].samples != VK_SAMPLE_COUNT_1_BIT) {
                            vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pResolveAttachments-03067"
                                           : "VUID-VkSubpassDescription-pResolveAttachments-00849";
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "%s:  Subpass %u requests multisample resolve into attachment %u, which must "
                                "have VK_SAMPLE_COUNT_1_BIT but has %s.",
                                function_name, i, attachment_ref.attachment,
                                string_VkSampleCountFlagBits(pCreateInfo->pAttachments[attachment_ref.attachment].samples));
                        }
                    }
                }
            }
        }

        if (subpass.pDepthStencilAttachment) {
            if (subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                skip |= ValidateAttachmentIndex(rp_version, subpass.pDepthStencilAttachment->attachment,
                                                pCreateInfo->attachmentCount, "Depth");
                if (subpass.pDepthStencilAttachment->attachment < pCreateInfo->attachmentCount) {
                    skip |= AddAttachmentUse(rp_version, i, attachment_uses, attachment_layouts,
                                             subpass.pDepthStencilAttachment->attachment, ATTACHMENT_DEPTH,
                                             subpass.pDepthStencilAttachment->layout);
                }
            }
        }

        uint32_t last_sample_count_attachment = VK_ATTACHMENT_UNUSED;
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            auto const &attachment_ref = subpass.pColorAttachments[j];
            skip |= ValidateAttachmentIndex(rp_version, attachment_ref.attachment, pCreateInfo->attachmentCount, "Color");
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED && attachment_ref.attachment < pCreateInfo->attachmentCount) {
                skip |= AddAttachmentUse(rp_version, i, attachment_uses, attachment_layouts, attachment_ref.attachment,
                                         ATTACHMENT_COLOR, attachment_ref.layout);

                VkSampleCountFlagBits current_sample_count = pCreateInfo->pAttachments[attachment_ref.attachment].samples;
                if (last_sample_count_attachment != VK_ATTACHMENT_UNUSED) {
                    VkSampleCountFlagBits last_sample_count =
                        pCreateInfo->pAttachments[subpass.pColorAttachments[last_sample_count_attachment].attachment].samples;
                    if (current_sample_count != last_sample_count) {
                        vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pColorAttachments-03069"
                                       : "VUID-VkSubpassDescription-pColorAttachments-01417";
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "%s:  Subpass %u attempts to render to color attachments with inconsistent sample counts."
                                    "Color attachment ref %u has sample count %s, whereas previous color attachment ref %u has "
                                    "sample count %s.",
                                    function_name, i, j, string_VkSampleCountFlagBits(current_sample_count),
                                    last_sample_count_attachment, string_VkSampleCountFlagBits(last_sample_count));
                    }
                }
                last_sample_count_attachment = j;

                if (subpass_performs_resolve && current_sample_count == VK_SAMPLE_COUNT_1_BIT) {
                    vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pResolveAttachments-03066"
                                   : "VUID-VkSubpassDescription-pResolveAttachments-00848";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "%s:  Subpass %u requests multisample resolve from attachment %u which has "
                                    "VK_SAMPLE_COUNT_1_BIT.",
                                    function_name, i, attachment_ref.attachment);
                }

                if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED &&
                    subpass.pDepthStencilAttachment->attachment < pCreateInfo->attachmentCount) {
                    const auto depth_stencil_sample_count =
                        pCreateInfo->pAttachments[subpass.pDepthStencilAttachment->attachment].samples;

                    if (device_extensions.vk_amd_mixed_attachment_samples) {
                        if (pCreateInfo->pAttachments[attachment_ref.attachment].samples > depth_stencil_sample_count) {
                            vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pColorAttachments-03070"
                                           : "VUID-VkSubpassDescription-pColorAttachments-01506";
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "%s:  Subpass %u pColorAttachments[%u] has %s which is larger than "
                                "depth/stencil attachment %s.",
                                function_name, i, j,
                                string_VkSampleCountFlagBits(pCreateInfo->pAttachments[attachment_ref.attachment].samples),
                                string_VkSampleCountFlagBits(depth_stencil_sample_count));
                            break;
                        }
                    }

                    if (!device_extensions.vk_amd_mixed_attachment_samples && !device_extensions.vk_nv_framebuffer_mixed_samples &&
                        current_sample_count != depth_stencil_sample_count) {
                        vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pDepthStencilAttachment-03071"
                                       : "VUID-VkSubpassDescription-pDepthStencilAttachment-01418";
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s:  Subpass %u attempts to render to use a depth/stencil attachment with sample count that differs "
                            "from color attachment %u."
                            "The depth attachment ref has sample count %s, whereas color attachment ref %u has sample count %s.",
                            function_name, i, j, string_VkSampleCountFlagBits(depth_stencil_sample_count), j,
                            string_VkSampleCountFlagBits(current_sample_count));
                        break;
                    }
                }
            }

            if (subpass_performs_resolve && subpass.pResolveAttachments[j].attachment != VK_ATTACHMENT_UNUSED &&
                subpass.pResolveAttachments[j].attachment < pCreateInfo->attachmentCount) {
                if (attachment_ref.attachment == VK_ATTACHMENT_UNUSED) {
                    vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pResolveAttachments-03065"
                                   : "VUID-VkSubpassDescription-pResolveAttachments-00847";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "%s:  Subpass %u requests multisample resolve from attachment %u which has "
                                    "attachment=VK_ATTACHMENT_UNUSED.",
                                    function_name, i, attachment_ref.attachment);
                } else {
                    const auto &color_desc = pCreateInfo->pAttachments[attachment_ref.attachment];
                    const auto &resolve_desc = pCreateInfo->pAttachments[subpass.pResolveAttachments[j].attachment];
                    if (color_desc.format != resolve_desc.format) {
                        vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-pResolveAttachments-03068"
                                       : "VUID-VkSubpassDescription-pResolveAttachments-00850";
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "%s:  Subpass %u pColorAttachments[%u] resolves to an attachment with a "
                                    "different format. color format: %u, resolve format: %u.",
                                    function_name, i, j, color_desc.format, resolve_desc.format);
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCreateRenderPass(VkDevice device, RenderPassCreateVersion rp_version,
                                          const VkRenderPassCreateInfo2KHR *pCreateInfo) const {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    // TODO: As part of wrapping up the mem_tracker/core_validation merge the following routine should be consolidated with
    //       ValidateLayouts.
    skip |= ValidateRenderpassAttachmentUsage(rp_version, pCreateInfo);

    skip |= ValidateRenderPassDAG(rp_version, pCreateInfo);

    // Validate multiview correlation and view masks
    bool viewMaskZero = false;
    bool viewMaskNonZero = false;

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];
        if (subpass.viewMask != 0) {
            viewMaskNonZero = true;
        } else {
            viewMaskZero = true;
        }

        if ((subpass.flags & VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX) != 0 &&
            (subpass.flags & VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX) == 0) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-flags-03076" : "VUID-VkSubpassDescription-flags-00856";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s: The flags parameter of subpass description %u includes "
                            "VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX but does not also include "
                            "VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX.",
                            function_name, i);
        }
    }

    if (rp_version == RENDER_PASS_VERSION_2) {
        if (viewMaskNonZero && viewMaskZero) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkRenderPassCreateInfo2KHR-viewMask-03058",
                            "%s: Some view masks are non-zero whilst others are zero.", function_name);
        }

        if (viewMaskZero && pCreateInfo->correlatedViewMaskCount != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkRenderPassCreateInfo2KHR-viewMask-03057",
                            "%s: Multiview is not enabled but correlation masks are still provided", function_name);
        }
    }
    uint32_t aggregated_cvms = 0;
    for (uint32_t i = 0; i < pCreateInfo->correlatedViewMaskCount; ++i) {
        if (aggregated_cvms & pCreateInfo->pCorrelatedViewMasks[i]) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pCorrelatedViewMasks-03056"
                           : "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s: pCorrelatedViewMasks[%u] contains a previously appearing view bit.", function_name, i);
        }
        aggregated_cvms |= pCreateInfo->pCorrelatedViewMasks[i];
    }

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        auto const &dependency = pCreateInfo->pDependencies[i];
        if (rp_version == RENDER_PASS_VERSION_2) {
            skip |= ValidateStageMaskGsTsEnables(
                dependency.srcStageMask, function_name, "VUID-VkSubpassDependency2KHR-srcStageMask-03080",
                "VUID-VkSubpassDependency2KHR-srcStageMask-03082", "VUID-VkSubpassDependency2KHR-srcStageMask-02103",
                "VUID-VkSubpassDependency2KHR-srcStageMask-02104");
            skip |= ValidateStageMaskGsTsEnables(
                dependency.dstStageMask, function_name, "VUID-VkSubpassDependency2KHR-dstStageMask-03081",
                "VUID-VkSubpassDependency2KHR-dstStageMask-03083", "VUID-VkSubpassDependency2KHR-dstStageMask-02105",
                "VUID-VkSubpassDependency2KHR-dstStageMask-02106");
        } else {
            skip |= ValidateStageMaskGsTsEnables(
                dependency.srcStageMask, function_name, "VUID-VkSubpassDependency-srcStageMask-00860",
                "VUID-VkSubpassDependency-srcStageMask-00862", "VUID-VkSubpassDependency-srcStageMask-02099",
                "VUID-VkSubpassDependency-srcStageMask-02100");
            skip |= ValidateStageMaskGsTsEnables(
                dependency.dstStageMask, function_name, "VUID-VkSubpassDependency-dstStageMask-00861",
                "VUID-VkSubpassDependency-dstStageMask-00863", "VUID-VkSubpassDependency-dstStageMask-02101",
                "VUID-VkSubpassDependency-dstStageMask-02102");
        }

        if (!ValidateAccessMaskPipelineStage(device_extensions, dependency.srcAccessMask, dependency.srcStageMask)) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcAccessMask-03088" : "VUID-VkSubpassDependency-srcAccessMask-00868";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s: pDependencies[%u].srcAccessMask (0x%" PRIx32 ") is not supported by srcStageMask (0x%" PRIx32 ").",
                            function_name, i, dependency.srcAccessMask, dependency.srcStageMask);
        }

        if (!ValidateAccessMaskPipelineStage(device_extensions, dependency.dstAccessMask, dependency.dstStageMask)) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-dstAccessMask-03089" : "VUID-VkSubpassDependency-dstAccessMask-00869";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "%s: pDependencies[%u].dstAccessMask (0x%" PRIx32 ") is not supported by dstStageMask (0x%" PRIx32 ").",
                            function_name, i, dependency.dstAccessMask, dependency.dstStageMask);
        }
    }
    if (!skip) {
        skip |= ValidateLayouts(rp_version, device, pCreateInfo);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const {
    bool skip = false;
    // Handle extension structs from KHR_multiview and KHR_maintenance2 that can only be validated for RP1 (indices out of bounds)
    const VkRenderPassMultiviewCreateInfo *pMultiviewInfo = lvl_find_in_chain<VkRenderPassMultiviewCreateInfo>(pCreateInfo->pNext);
    if (pMultiviewInfo) {
        if (pMultiviewInfo->subpassCount && pMultiviewInfo->subpassCount != pCreateInfo->subpassCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkRenderPassCreateInfo-pNext-01928",
                            "Subpass count is %u but multiview info has a subpass count of %u.", pCreateInfo->subpassCount,
                            pMultiviewInfo->subpassCount);
        } else if (pMultiviewInfo->dependencyCount && pMultiviewInfo->dependencyCount != pCreateInfo->dependencyCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkRenderPassCreateInfo-pNext-01929",
                            "Dependency count is %u but multiview info has a dependency count of %u.", pCreateInfo->dependencyCount,
                            pMultiviewInfo->dependencyCount);
        }
    }
    const VkRenderPassInputAttachmentAspectCreateInfo *pInputAttachmentAspectInfo =
        lvl_find_in_chain<VkRenderPassInputAttachmentAspectCreateInfo>(pCreateInfo->pNext);
    if (pInputAttachmentAspectInfo) {
        for (uint32_t i = 0; i < pInputAttachmentAspectInfo->aspectReferenceCount; ++i) {
            uint32_t subpass = pInputAttachmentAspectInfo->pAspectReferences[i].subpass;
            uint32_t attachment = pInputAttachmentAspectInfo->pAspectReferences[i].inputAttachmentIndex;
            if (subpass >= pCreateInfo->subpassCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkRenderPassCreateInfo-pNext-01926",
                                "Subpass index %u specified by input attachment aspect info %u is greater than the subpass "
                                "count of %u for this render pass.",
                                subpass, i, pCreateInfo->subpassCount);
            } else if (pCreateInfo->pSubpasses && attachment >= pCreateInfo->pSubpasses[subpass].inputAttachmentCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkRenderPassCreateInfo-pNext-01927",
                                "Input attachment index %u specified by input attachment aspect info %u is greater than the "
                                "input attachment count of %u for this subpass.",
                                attachment, i, pCreateInfo->pSubpasses[subpass].inputAttachmentCount);
            }
        }
    }
    const VkRenderPassFragmentDensityMapCreateInfoEXT *pFragmentDensityMapInfo =
        lvl_find_in_chain<VkRenderPassFragmentDensityMapCreateInfoEXT>(pCreateInfo->pNext);
    if (pFragmentDensityMapInfo) {
        if (pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment != VK_ATTACHMENT_UNUSED) {
            if (pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment >= pCreateInfo->attachmentCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02547",
                                "fragmentDensityMapAttachment %u must be less than attachmentCount %u of for this render pass.",
                                pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment, pCreateInfo->attachmentCount);
            } else {
                if (!(pFragmentDensityMapInfo->fragmentDensityMapAttachment.layout ==
                          VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT ||
                      pFragmentDensityMapInfo->fragmentDensityMapAttachment.layout == VK_IMAGE_LAYOUT_GENERAL)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549",
                                    "Layout of fragmentDensityMapAttachment %u' must be equal to "
                                    "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT, or VK_IMAGE_LAYOUT_GENERAL.",
                                    pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment);
                }
                if (!(pCreateInfo->pAttachments[pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment].loadOp ==
                          VK_ATTACHMENT_LOAD_OP_LOAD ||
                      pCreateInfo->pAttachments[pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment].loadOp ==
                          VK_ATTACHMENT_LOAD_OP_DONT_CARE)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550",
                                    "FragmentDensityMapAttachment %u' must reference an attachment with a loadOp "
                                    "equal to VK_ATTACHMENT_LOAD_OP_LOAD or VK_ATTACHMENT_LOAD_OP_DONT_CARE.",
                                    pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment);
                }
                if (pCreateInfo->pAttachments[pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment].storeOp !=
                    VK_ATTACHMENT_STORE_OP_DONT_CARE) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551",
                                    "FragmentDensityMapAttachment %u' must reference an attachment with a storeOp "
                                    "equal to VK_ATTACHMENT_STORE_OP_DONT_CARE.",
                                    pFragmentDensityMapInfo->fragmentDensityMapAttachment.attachment);
                }
            }
        }
    }

    if (!skip) {
        safe_VkRenderPassCreateInfo2KHR create_info_2;
        ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo, &create_info_2);
        skip |= ValidateCreateRenderPass(device, RENDER_PASS_VERSION_1, create_info_2.ptr());
    }

    return skip;
}

static bool ValidateDepthStencilResolve(const debug_report_data *report_data,
                                        const VkPhysicalDeviceDepthStencilResolvePropertiesKHR &depth_stencil_resolve_props,
                                        const VkRenderPassCreateInfo2KHR *pCreateInfo) {
    bool skip = false;

    // If the pNext list of VkSubpassDescription2KHR includes a VkSubpassDescriptionDepthStencilResolveKHR structure,
    // then that structure describes depth/stencil resolve operations for the subpass.
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; i++) {
        VkSubpassDescription2KHR subpass = pCreateInfo->pSubpasses[i];
        const auto *resolve = lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolveKHR>(subpass.pNext);

        if (resolve == nullptr) {
            continue;
        }

        if (resolve->pDepthStencilResolveAttachment != nullptr &&
            resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            if (subpass.pDepthStencilAttachment->attachment == VK_ATTACHMENT_UNUSED) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03177",
                                "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                                "structure with resolve attachment %u, but pDepthStencilAttachment=VK_ATTACHMENT_UNUSED.",
                                i, resolve->pDepthStencilResolveAttachment->attachment);
            }
            if (resolve->depthResolveMode == VK_RESOLVE_MODE_NONE_KHR && resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE_KHR) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03178",
                                "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                                "structure with resolve attachment %u, but both depth and stencil resolve modes are "
                                "VK_RESOLVE_MODE_NONE_KHR.",
                                i, resolve->pDepthStencilResolveAttachment->attachment);
            }
        }

        if (resolve->pDepthStencilResolveAttachment != nullptr &&
            pCreateInfo->pAttachments[subpass.pDepthStencilAttachment->attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03179",
                "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                "structure with resolve attachment %u. However pDepthStencilAttachment has sample count=VK_SAMPLE_COUNT_1_BIT.",
                i, resolve->pDepthStencilResolveAttachment->attachment);
        }

        if (pCreateInfo->pAttachments[resolve->pDepthStencilResolveAttachment->attachment].samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03180",
                            "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                            "structure with resolve attachment %u which has sample count=VK_SAMPLE_COUNT_1_BIT.",
                            i, resolve->pDepthStencilResolveAttachment->attachment);
        }

        VkFormat pDepthStencilAttachmentFormat = pCreateInfo->pAttachments[subpass.pDepthStencilAttachment->attachment].format;
        VkFormat pDepthStencilResolveAttachmentFormat =
            pCreateInfo->pAttachments[resolve->pDepthStencilResolveAttachment->attachment].format;

        if ((FormatDepthSize(pDepthStencilAttachmentFormat) != FormatDepthSize(pDepthStencilResolveAttachmentFormat)) ||
            (FormatDepthNumericalType(pDepthStencilAttachmentFormat) !=
             FormatDepthNumericalType(pDepthStencilResolveAttachmentFormat))) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03181",
                        "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                        "structure with resolve attachment %u which has a depth component (size %u). The depth component "
                        "of pDepthStencilAttachment must have the same number of bits (currently %u) and the same numerical type.",
                        i, resolve->pDepthStencilResolveAttachment->attachment,
                        FormatDepthSize(pDepthStencilResolveAttachmentFormat), FormatDepthSize(pDepthStencilAttachmentFormat));
        }

        if ((FormatStencilSize(pDepthStencilAttachmentFormat) != FormatStencilSize(pDepthStencilResolveAttachmentFormat)) ||
            (FormatStencilNumericalType(pDepthStencilAttachmentFormat) !=
             FormatStencilNumericalType(pDepthStencilResolveAttachmentFormat))) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03182",
                        "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                        "structure with resolve attachment %u which has a stencil component (size %u). The stencil component "
                        "of pDepthStencilAttachment must have the same number of bits (currently %u) and the same numerical type.",
                        i, resolve->pDepthStencilResolveAttachment->attachment,
                        FormatStencilSize(pDepthStencilResolveAttachmentFormat), FormatStencilSize(pDepthStencilAttachmentFormat));
        }

        if (!(resolve->depthResolveMode == VK_RESOLVE_MODE_NONE_KHR ||
              resolve->depthResolveMode & depth_stencil_resolve_props.supportedDepthResolveModes)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescriptionDepthStencilResolveKHR-depthResolveMode-03183",
                            "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                            "structure with invalid depthResolveMode=%u.",
                            i, resolve->depthResolveMode);
        }

        if (!(resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE_KHR ||
              resolve->stencilResolveMode & depth_stencil_resolve_props.supportedStencilResolveModes)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescriptionDepthStencilResolveKHR-stencilResolveMode-03184",
                            "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                            "structure with invalid stencilResolveMode=%u.",
                            i, resolve->stencilResolveMode);
        }

        if (FormatIsDepthAndStencil(pDepthStencilResolveAttachmentFormat) &&
            depth_stencil_resolve_props.independentResolve == VK_FALSE &&
            depth_stencil_resolve_props.independentResolveNone == VK_FALSE &&
            !(resolve->depthResolveMode == resolve->stencilResolveMode)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03185",
                            "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                            "structure. The values of depthResolveMode (%u) and stencilResolveMode (%u) must be identical.",
                            i, resolve->depthResolveMode, resolve->stencilResolveMode);
        }

        if (FormatIsDepthAndStencil(pDepthStencilResolveAttachmentFormat) &&
            depth_stencil_resolve_props.independentResolve == VK_FALSE &&
            depth_stencil_resolve_props.independentResolveNone == VK_TRUE &&
            !(resolve->depthResolveMode == resolve->stencilResolveMode || resolve->depthResolveMode == VK_RESOLVE_MODE_NONE_KHR ||
              resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE_KHR)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkSubpassDescriptionDepthStencilResolveKHR-pDepthStencilResolveAttachment-03186",
                            "vkCreateRenderPass2KHR(): Subpass %u includes a VkSubpassDescriptionDepthStencilResolveKHR "
                            "structure. The values of depthResolveMode (%u) and stencilResolveMode (%u) must be identical, or "
                            "one of them must be %u.",
                            i, resolve->depthResolveMode, resolve->stencilResolveMode, VK_RESOLVE_MODE_NONE_KHR);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) const {
    bool skip = false;

    if (device_extensions.vk_khr_depth_stencil_resolve) {
        skip |= ValidateDepthStencilResolve(report_data, phys_dev_ext_props.depth_stencil_resolve_props, pCreateInfo);
    }

    safe_VkRenderPassCreateInfo2KHR create_info_2(pCreateInfo);
    skip |= ValidateCreateRenderPass(device, RENDER_PASS_VERSION_2, create_info_2.ptr());

    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBuffer(const CMD_BUFFER_STATE *pCB, char const *cmd_name, const char *error_code) const {
    bool skip = false;
    if (pCB->createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), error_code, "Cannot execute command %s on a secondary command buffer.",
                        cmd_name);
    }
    return skip;
}

bool CoreChecks::VerifyRenderAreaBounds(const VkRenderPassBeginInfo *pRenderPassBegin) const {
    bool skip = false;
    const safe_VkFramebufferCreateInfo *pFramebufferInfo = &GetFramebufferState(pRenderPassBegin->framebuffer)->createInfo;
    if (pRenderPassBegin->renderArea.offset.x < 0 ||
        (pRenderPassBegin->renderArea.offset.x + pRenderPassBegin->renderArea.extent.width) > pFramebufferInfo->width ||
        pRenderPassBegin->renderArea.offset.y < 0 ||
        (pRenderPassBegin->renderArea.offset.y + pRenderPassBegin->renderArea.extent.height) > pFramebufferInfo->height) {
        skip |= static_cast<bool>(log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            kVUID_Core_DrawState_InvalidRenderArea,
            "Cannot execute a render pass with renderArea not within the bound of the framebuffer. RenderArea: x %d, y %d, width "
            "%d, height %d. Framebuffer: width %d, height %d.",
            pRenderPassBegin->renderArea.offset.x, pRenderPassBegin->renderArea.offset.y, pRenderPassBegin->renderArea.extent.width,
            pRenderPassBegin->renderArea.extent.height, pFramebufferInfo->width, pFramebufferInfo->height));
    }
    return skip;
}

bool CoreChecks::VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo *pRenderPassBeginInfo) const {
    bool skip = false;
    const VkRenderPassAttachmentBeginInfoKHR *pRenderPassAttachmentBeginInfo =
        lvl_find_in_chain<VkRenderPassAttachmentBeginInfoKHR>(pRenderPassBeginInfo->pNext);

    if (pRenderPassAttachmentBeginInfo && pRenderPassAttachmentBeginInfo->attachmentCount != 0) {
        const safe_VkFramebufferCreateInfo *pFramebufferCreateInfo =
            &GetFramebufferState(pRenderPassBeginInfo->framebuffer)->createInfo;
        const VkFramebufferAttachmentsCreateInfoKHR *pFramebufferAttachmentsCreateInfo =
            lvl_find_in_chain<VkFramebufferAttachmentsCreateInfoKHR>(pFramebufferCreateInfo->pNext);
        if ((pFramebufferCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(pRenderPassBeginInfo->renderPass), "VUID-VkRenderPassBeginInfo-framebuffer-03207",
                            "VkRenderPassBeginInfo: Image views specified at render pass begin, but framebuffer not created with "
                            "VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR");
        } else if (pFramebufferAttachmentsCreateInfo) {
            if (pFramebufferAttachmentsCreateInfo->attachmentImageInfoCount != pRenderPassAttachmentBeginInfo->attachmentCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                HandleToUint64(pRenderPassBeginInfo->renderPass), "VUID-VkRenderPassBeginInfo-framebuffer-03208",
                                "VkRenderPassBeginInfo: %u image views specified at render pass begin, but framebuffer "
                                "created expecting %u attachments",
                                pRenderPassAttachmentBeginInfo->attachmentCount,
                                pFramebufferAttachmentsCreateInfo->attachmentImageInfoCount);
            } else {
                const safe_VkRenderPassCreateInfo2KHR *pRenderPassCreateInfo =
                    &GetRenderPassState(pRenderPassBeginInfo->renderPass)->createInfo;
                for (uint32_t i = 0; i < pRenderPassAttachmentBeginInfo->attachmentCount; ++i) {
                    const VkImageViewCreateInfo *pImageViewCreateInfo =
                        &GetImageViewState(pRenderPassAttachmentBeginInfo->pAttachments[i])->create_info;
                    const VkFramebufferAttachmentImageInfoKHR *pFramebufferAttachmentImageInfo =
                        &pFramebufferAttachmentsCreateInfo->pAttachmentImageInfos[i];
                    const VkImageCreateInfo *pImageCreateInfo = &GetImageState(pImageViewCreateInfo->image)->createInfo;

                    if (pFramebufferAttachmentImageInfo->flags != pImageCreateInfo->flags) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03209",
                                        "VkRenderPassBeginInfo: Image view #%u created from an image with flags set as 0x%X, "
                                        "but image info #%u used to create the framebuffer had flags set as 0x%X",
                                        i, pImageCreateInfo->flags, i, pFramebufferAttachmentImageInfo->flags);
                    }

                    if (pFramebufferAttachmentImageInfo->usage != pImageCreateInfo->usage) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03210",
                                        "VkRenderPassBeginInfo: Image view #%u created from an image with usage set as 0x%X, "
                                        "but image info #%u used to create the framebuffer had usage set as 0x%X",
                                        i, pImageCreateInfo->usage, i, pFramebufferAttachmentImageInfo->usage);
                    }

                    if (pFramebufferAttachmentImageInfo->width != pImageCreateInfo->extent.width) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03211",
                                        "VkRenderPassBeginInfo: Image view #%u created from an image with width set as %u, "
                                        "but image info #%u used to create the framebuffer had width set as %u",
                                        i, pImageCreateInfo->extent.width, i, pFramebufferAttachmentImageInfo->width);
                    }

                    if (pFramebufferAttachmentImageInfo->height != pImageCreateInfo->extent.height) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03212",
                                        "VkRenderPassBeginInfo: Image view #%u created from an image with height set as %u, "
                                        "but image info #%u used to create the framebuffer had height set as %u",
                                        i, pImageCreateInfo->extent.height, i, pFramebufferAttachmentImageInfo->height);
                    }

                    if (pFramebufferAttachmentImageInfo->layerCount != pImageViewCreateInfo->subresourceRange.layerCount) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(pRenderPassBeginInfo->renderPass), "VUID-VkRenderPassBeginInfo-framebuffer-03213",
                            "VkRenderPassBeginInfo: Image view #%u created with a subresource range with a layerCount of %u, "
                            "but image info #%u used to create the framebuffer had layerCount set as %u",
                            i, pImageViewCreateInfo->subresourceRange.layerCount, i, pFramebufferAttachmentImageInfo->layerCount);
                    }

                    const VkImageFormatListCreateInfoKHR *pImageFormatListCreateInfo =
                        lvl_find_in_chain<VkImageFormatListCreateInfoKHR>(pImageCreateInfo->pNext);
                    if (pImageFormatListCreateInfo) {
                        if (pImageFormatListCreateInfo->viewFormatCount != pFramebufferAttachmentImageInfo->viewFormatCount) {
                            skip |= log_msg(
                                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                HandleToUint64(pRenderPassBeginInfo->renderPass), "VUID-VkRenderPassBeginInfo-framebuffer-03214",
                                "VkRenderPassBeginInfo: Image view #%u created with an image with a viewFormatCount of %u, "
                                "but image info #%u used to create the framebuffer had viewFormatCount set as %u",
                                i, pImageFormatListCreateInfo->viewFormatCount, i,
                                pFramebufferAttachmentImageInfo->viewFormatCount);
                        }

                        for (uint32_t j = 0; j < pImageFormatListCreateInfo->viewFormatCount; ++j) {
                            bool formatFound = false;
                            for (uint32_t k = 0; k < pFramebufferAttachmentImageInfo->viewFormatCount; ++k) {
                                if (pImageFormatListCreateInfo->pViewFormats[j] ==
                                    pFramebufferAttachmentImageInfo->pViewFormats[k]) {
                                    formatFound = true;
                                }
                            }
                            if (!formatFound) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                            HandleToUint64(pRenderPassBeginInfo->renderPass),
                                            "VUID-VkRenderPassBeginInfo-framebuffer-03215",
                                            "VkRenderPassBeginInfo: Image view #%u created with an image including the format "
                                            "%s in its view format list, "
                                            "but image info #%u used to create the framebuffer does not include this format",
                                            i, string_VkFormat(pImageFormatListCreateInfo->pViewFormats[j]), i);
                            }
                        }
                    }

                    if (pRenderPassCreateInfo->pAttachments[i].format != pImageViewCreateInfo->format) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03216",
                                        "VkRenderPassBeginInfo: Image view #%u created with a format of %s, "
                                        "but render pass attachment description #%u created with a format of %s",
                                        i, string_VkFormat(pImageViewCreateInfo->format), i,
                                        string_VkFormat(pRenderPassCreateInfo->pAttachments[i].format));
                    }

                    if (pRenderPassCreateInfo->pAttachments[i].samples != pImageCreateInfo->samples) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                        HandleToUint64(pRenderPassBeginInfo->renderPass),
                                        "VUID-VkRenderPassBeginInfo-framebuffer-03217",
                                        "VkRenderPassBeginInfo: Image view #%u created with an image with %s samples, "
                                        "but render pass attachment description #%u created with %s samples",
                                        i, string_VkSampleCountFlagBits(pImageCreateInfo->samples), i,
                                        string_VkSampleCountFlagBits(pRenderPassCreateInfo->pAttachments[i].samples));
                    }

                    if (pImageViewCreateInfo->subresourceRange.levelCount != 1) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                                        HandleToUint64(pRenderPassAttachmentBeginInfo->pAttachments[i]),
                                        "VUID-VkRenderPassAttachmentBeginInfoKHR-pAttachments-03218",
                                        "VkRenderPassAttachmentBeginInfo: Image view #%u created with multiple (%u) mip levels.", i,
                                        pImageViewCreateInfo->subresourceRange.levelCount);
                    }

                    if (((pImageViewCreateInfo->components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                         (pImageViewCreateInfo->components.r != VK_COMPONENT_SWIZZLE_R)) ||
                        ((pImageViewCreateInfo->components.g != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                         (pImageViewCreateInfo->components.g != VK_COMPONENT_SWIZZLE_G)) ||
                        ((pImageViewCreateInfo->components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                         (pImageViewCreateInfo->components.b != VK_COMPONENT_SWIZZLE_B)) ||
                        ((pImageViewCreateInfo->components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
                         (pImageViewCreateInfo->components.a != VK_COMPONENT_SWIZZLE_A))) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                                    HandleToUint64(pRenderPassAttachmentBeginInfo->pAttachments[i]),
                                    "VUID-VkRenderPassAttachmentBeginInfoKHR-pAttachments-03219",
                                    "VkRenderPassAttachmentBeginInfo: Image view #%u created with non-identity swizzle. All "
                                    "framebuffer attachments must have been created with the identity swizzle. Here are the actual "
                                    "swizzle values:\n"
                                    "r swizzle = %s\n"
                                    "g swizzle = %s\n"
                                    "b swizzle = %s\n"
                                    "a swizzle = %s\n",
                                    i, string_VkComponentSwizzle(pImageViewCreateInfo->components.r),
                                    string_VkComponentSwizzle(pImageViewCreateInfo->components.g),
                                    string_VkComponentSwizzle(pImageViewCreateInfo->components.b),
                                    string_VkComponentSwizzle(pImageViewCreateInfo->components.a));
                    }
                }
            }
        }
    }

    return skip;
}

// If this is a stencil format, make sure the stencil[Load|Store]Op flag is checked, while if it is a depth/color attachment the
// [load|store]Op flag must be checked
// TODO: The memory valid flag in DEVICE_MEMORY_STATE should probably be split to track the validity of stencil memory separately.
template <typename T>
static bool FormatSpecificLoadAndStoreOpSettings(VkFormat format, T color_depth_op, T stencil_op, T op) {
    if (color_depth_op != op && stencil_op != op) {
        return false;
    }
    bool check_color_depth_load_op = !FormatIsStencilOnly(format);
    bool check_stencil_load_op = FormatIsDepthAndStencil(format) || !check_color_depth_load_op;

    return ((check_color_depth_load_op && (color_depth_op == op)) || (check_stencil_load_op && (stencil_op == op)));
}

bool CoreChecks::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                            const VkRenderPassBeginInfo *pRenderPassBegin) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    auto render_pass_state = pRenderPassBegin ? GetRenderPassState(pRenderPassBegin->renderPass) : nullptr;
    auto framebuffer = pRenderPassBegin ? GetFramebufferState(pRenderPassBegin->framebuffer) : nullptr;

    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCmdBeginRenderPass2KHR()" : "vkCmdBeginRenderPass()";

    if (render_pass_state) {
        uint32_t clear_op_size = 0;  // Make sure pClearValues is at least as large as last LOAD_OP_CLEAR

        // Handle extension struct from EXT_sample_locations
        const VkRenderPassSampleLocationsBeginInfoEXT *pSampleLocationsBeginInfo =
            lvl_find_in_chain<VkRenderPassSampleLocationsBeginInfoEXT>(pRenderPassBegin->pNext);
        if (pSampleLocationsBeginInfo) {
            for (uint32_t i = 0; i < pSampleLocationsBeginInfo->attachmentInitialSampleLocationsCount; ++i) {
                if (pSampleLocationsBeginInfo->pAttachmentInitialSampleLocations[i].attachmentIndex >=
                    render_pass_state->createInfo.attachmentCount) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkAttachmentSampleLocationsEXT-attachmentIndex-01531",
                                    "Attachment index %u specified by attachment sample locations %u is greater than the "
                                    "attachment count of %u for the render pass being begun.",
                                    pSampleLocationsBeginInfo->pAttachmentInitialSampleLocations[i].attachmentIndex, i,
                                    render_pass_state->createInfo.attachmentCount);
                }
            }

            for (uint32_t i = 0; i < pSampleLocationsBeginInfo->postSubpassSampleLocationsCount; ++i) {
                if (pSampleLocationsBeginInfo->pPostSubpassSampleLocations[i].subpassIndex >=
                    render_pass_state->createInfo.subpassCount) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkSubpassSampleLocationsEXT-subpassIndex-01532",
                                    "Subpass index %u specified by subpass sample locations %u is greater than the subpass count "
                                    "of %u for the render pass being begun.",
                                    pSampleLocationsBeginInfo->pPostSubpassSampleLocations[i].subpassIndex, i,
                                    render_pass_state->createInfo.subpassCount);
                }
            }
        }

        for (uint32_t i = 0; i < render_pass_state->createInfo.attachmentCount; ++i) {
            auto pAttachment = &render_pass_state->createInfo.pAttachments[i];
            if (FormatSpecificLoadAndStoreOpSettings(pAttachment->format, pAttachment->loadOp, pAttachment->stencilLoadOp,
                                                     VK_ATTACHMENT_LOAD_OP_CLEAR)) {
                clear_op_size = static_cast<uint32_t>(i) + 1;
            }
        }

        if (clear_op_size > pRenderPassBegin->clearValueCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(render_pass_state->renderPass), "VUID-VkRenderPassBeginInfo-clearValueCount-00902",
                            "In %s the VkRenderPassBeginInfo struct has a clearValueCount of %u but there "
                            "must be at least %u entries in pClearValues array to account for the highest index attachment in "
                            "%s that uses VK_ATTACHMENT_LOAD_OP_CLEAR is %u. Note that the pClearValues array is indexed by "
                            "attachment number so even if some pClearValues entries between 0 and %u correspond to attachments "
                            "that aren't cleared they will be ignored.",
                            function_name, pRenderPassBegin->clearValueCount, clear_op_size,
                            report_data->FormatHandle(render_pass_state->renderPass).c_str(), clear_op_size, clear_op_size - 1);
        }
        skip |= VerifyFramebufferAndRenderPassImageViews(pRenderPassBegin);
        skip |= VerifyRenderAreaBounds(pRenderPassBegin);
        skip |= VerifyFramebufferAndRenderPassLayouts(rp_version, cb_state, pRenderPassBegin,
                                                      GetFramebufferState(pRenderPassBegin->framebuffer));
        if (framebuffer->rp_state->renderPass != render_pass_state->renderPass) {
            skip |= ValidateRenderPassCompatibility("render pass", render_pass_state, "framebuffer", framebuffer->rp_state.get(),
                                                    function_name, "VUID-VkRenderPassBeginInfo-renderPass-00904");
        }

        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-renderpass" : "VUID-vkCmdBeginRenderPass-renderpass";
        skip |= InsideRenderPass(cb_state, function_name, vuid);
        skip |= ValidateDependencies(framebuffer, render_pass_state);

        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-bufferlevel" : "VUID-vkCmdBeginRenderPass-bufferlevel";
        skip |= ValidatePrimaryCommandBuffer(cb_state, function_name, vuid);

        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-commandBuffer-cmdpool" : "VUID-vkCmdBeginRenderPass-commandBuffer-cmdpool";
        skip |= ValidateCmdQueueFlags(cb_state, function_name, VK_QUEUE_GRAPHICS_BIT, vuid);

        const CMD_TYPE cmd_type = use_rp2 ? CMD_BEGINRENDERPASS2KHR : CMD_BEGINRENDERPASS;
        skip |= ValidateCmd(cb_state, cmd_type, function_name);
    }

    auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
    if (chained_device_group_struct) {
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(
            chained_device_group_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
            HandleToUint64(pRenderPassBegin->renderPass), "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
        skip |= ValidateDeviceMaskToZero(chained_device_group_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                         HandleToUint64(pRenderPassBegin->renderPass),
                                         "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
        skip |= ValidateDeviceMaskToCommandBuffer(
            cb_state, chained_device_group_struct->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
            HandleToUint64(pRenderPassBegin->renderPass), "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");

        if (chained_device_group_struct->deviceRenderAreaCount != 0 &&
            chained_device_group_struct->deviceRenderAreaCount != physical_device_count) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(pRenderPassBegin->renderPass),
                            "VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908",
                            "deviceRenderAreaCount[%" PRIu32 "] is invaild. Physical device count is %" PRIu32 ".",
                            chained_device_group_struct->deviceRenderAreaCount, physical_device_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   VkSubpassContents contents) const {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

void CoreChecks::RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 const VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto render_pass_state = pRenderPassBegin ? GetRenderPassState(pRenderPassBegin->renderPass) : nullptr;
    auto framebuffer = pRenderPassBegin ? GetFramebufferState(pRenderPassBegin->framebuffer) : nullptr;
    if (render_pass_state) {
        // transition attachments to the correct layouts for beginning of renderPass and first subpass
        TransitionBeginRenderPassLayouts(cb_state, render_pass_state, framebuffer);
    }
}

void CoreChecks::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 VkSubpassContents contents) {
    StateTracker::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, contents);
}

void CoreChecks::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     const VkSubpassBeginInfoKHR *pSubpassBeginInfo) {
    StateTracker::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

bool CoreChecks::ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCmdNextSubpass2KHR()" : "vkCmdNextSubpass()";

    vuid = use_rp2 ? "VUID-vkCmdNextSubpass2KHR-bufferlevel" : "VUID-vkCmdNextSubpass-bufferlevel";
    skip |= ValidatePrimaryCommandBuffer(cb_state, function_name, vuid);

    vuid = use_rp2 ? "VUID-vkCmdNextSubpass2KHR-commandBuffer-cmdpool" : "VUID-vkCmdNextSubpass-commandBuffer-cmdpool";
    skip |= ValidateCmdQueueFlags(cb_state, function_name, VK_QUEUE_GRAPHICS_BIT, vuid);
    const CMD_TYPE cmd_type = use_rp2 ? CMD_NEXTSUBPASS2KHR : CMD_NEXTSUBPASS;
    skip |= ValidateCmd(cb_state, cmd_type, function_name);

    vuid = use_rp2 ? "VUID-vkCmdNextSubpass2KHR-renderpass" : "VUID-vkCmdNextSubpass-renderpass";
    skip |= OutsideRenderPass(cb_state, function_name, vuid);

    auto subpassCount = cb_state->activeRenderPass->createInfo.subpassCount;
    if (cb_state->activeSubpass == subpassCount - 1) {
        vuid = use_rp2 ? "VUID-vkCmdNextSubpass2KHR-None-03102" : "VUID-vkCmdNextSubpass-None-00909";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), vuid, "%s: Attempted to advance beyond final subpass.", function_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_1, commandBuffer);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                   const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_2, commandBuffer);
}

void CoreChecks::RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    TransitionSubpassLayouts(cb_state, cb_state->activeRenderPass, cb_state->activeSubpass,
                             GetFramebufferState(cb_state->activeRenderPassBeginInfo.framebuffer));
}

void CoreChecks::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    RecordCmdNextSubpassLayouts(commandBuffer, contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                  const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

bool CoreChecks::ValidateCmdEndRenderPass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCmdEndRenderPass2KHR()" : "vkCmdEndRenderPass()";

    RENDER_PASS_STATE *rp_state = cb_state->activeRenderPass;
    if (rp_state) {
        if (cb_state->activeSubpass != rp_state->createInfo.subpassCount - 1) {
            vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2KHR-None-03103" : "VUID-vkCmdEndRenderPass-None-00910";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(commandBuffer), vuid, "%s: Called before reaching final subpass.", function_name);
        }
    }

    vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2KHR-renderpass" : "VUID-vkCmdEndRenderPass-renderpass";
    skip |= OutsideRenderPass(cb_state, function_name, vuid);

    vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2KHR-bufferlevel" : "VUID-vkCmdEndRenderPass-bufferlevel";
    skip |= ValidatePrimaryCommandBuffer(cb_state, function_name, vuid);

    vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2KHR-commandBuffer-cmdpool" : "VUID-vkCmdEndRenderPass-commandBuffer-cmdpool";
    skip |= ValidateCmdQueueFlags(cb_state, function_name, VK_QUEUE_GRAPHICS_BIT, vuid);

    const CMD_TYPE cmd_type = use_rp2 ? CMD_ENDRENDERPASS2KHR : CMD_ENDRENDERPASS;
    skip |= ValidateCmd(cb_state, cmd_type, function_name);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = ValidateCmdEndRenderPass(RENDER_PASS_VERSION_1, commandBuffer);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                     const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = ValidateCmdEndRenderPass(RENDER_PASS_VERSION_2, commandBuffer);
    return skip;
}

void CoreChecks::RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    FRAMEBUFFER_STATE *framebuffer = GetFramebufferState(cb_state->activeFramebuffer);
    TransitionFinalSubpassLayouts(cb_state, &cb_state->activeRenderPassBeginInfo, framebuffer);
}

void CoreChecks::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    // Record the end at the CoreLevel to ensure StateTracker cleanup doesn't step on anything we need.
    RecordCmdEndRenderPassLayouts(commandBuffer);
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer);
}

void CoreChecks::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    RecordCmdEndRenderPassLayouts(commandBuffer);
}

bool CoreChecks::ValidateFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE *pCB, VkCommandBuffer secondaryBuffer,
                                     const CMD_BUFFER_STATE *pSubCB, const char *caller) const {
    bool skip = false;
    if (!pSubCB->beginInfo.pInheritanceInfo) {
        return skip;
    }
    VkFramebuffer primary_fb = pCB->activeFramebuffer;
    VkFramebuffer secondary_fb = pSubCB->beginInfo.pInheritanceInfo->framebuffer;
    if (secondary_fb != VK_NULL_HANDLE) {
        if (primary_fb != secondary_fb) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(primaryBuffer), "VUID-vkCmdExecuteCommands-pCommandBuffers-00099",
                            "vkCmdExecuteCommands() called w/ invalid secondary %s which has a %s"
                            " that is not the same as the primary command buffer's current active %s.",
                            report_data->FormatHandle(secondaryBuffer).c_str(), report_data->FormatHandle(secondary_fb).c_str(),
                            report_data->FormatHandle(primary_fb).c_str());
        }
        auto fb = GetFramebufferState(secondary_fb);
        if (!fb) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(primaryBuffer), kVUID_Core_DrawState_InvalidSecondaryCommandBuffer,
                            "vkCmdExecuteCommands() called w/ invalid %s which has invalid %s.",
                            report_data->FormatHandle(secondaryBuffer).c_str(), report_data->FormatHandle(secondary_fb).c_str());
            return skip;
        }
    }
    return skip;
}

bool CoreChecks::ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE *pCB, const CMD_BUFFER_STATE *pSubCB) const {
    bool skip = false;
    unordered_set<int> activeTypes;
    if (!disabled.query_validation) {
        for (auto queryObject : pCB->activeQueries) {
            auto query_pool_state = GetQueryPoolState(queryObject.pool);
            if (query_pool_state) {
                if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS &&
                    pSubCB->beginInfo.pInheritanceInfo) {
                    VkQueryPipelineStatisticFlags cmdBufStatistics = pSubCB->beginInfo.pInheritanceInfo->pipelineStatistics;
                    if ((cmdBufStatistics & query_pool_state->createInfo.pipelineStatistics) != cmdBufStatistics) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCB->commandBuffer), "VUID-vkCmdExecuteCommands-commandBuffer-00104",
                            "vkCmdExecuteCommands() called w/ invalid %s which has invalid active %s"
                            ". Pipeline statistics is being queried so the command buffer must have all bits set on the queryPool.",
                            report_data->FormatHandle(pCB->commandBuffer).c_str(),
                            report_data->FormatHandle(queryObject.pool).c_str());
                    }
                }
                activeTypes.insert(query_pool_state->createInfo.queryType);
            }
        }
        for (auto queryObject : pSubCB->startedQueries) {
            auto query_pool_state = GetQueryPoolState(queryObject.pool);
            if (query_pool_state && activeTypes.count(query_pool_state->createInfo.queryType)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidSecondaryCommandBuffer,
                                "vkCmdExecuteCommands() called w/ invalid %s which has invalid active %s"
                                " of type %d but a query of that type has been started on secondary %s.",
                                report_data->FormatHandle(pCB->commandBuffer).c_str(),
                                report_data->FormatHandle(queryObject.pool).c_str(), query_pool_state->createInfo.queryType,
                                report_data->FormatHandle(pSubCB->commandBuffer).c_str());
            }
        }
    }
    auto primary_pool = pCB->command_pool.get();
    auto secondary_pool = pSubCB->command_pool.get();
    if (primary_pool && secondary_pool && (primary_pool->queueFamilyIndex != secondary_pool->queueFamilyIndex)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pSubCB->commandBuffer), kVUID_Core_DrawState_InvalidQueueFamily,
                        "vkCmdExecuteCommands(): Primary %s created in queue family %d has secondary "
                        "%s created in queue family %d.",
                        report_data->FormatHandle(pCB->commandBuffer).c_str(), primary_pool->queueFamilyIndex,
                        report_data->FormatHandle(pSubCB->commandBuffer).c_str(), secondary_pool->queueFamilyIndex);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                   const VkCommandBuffer *pCommandBuffers) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = false;
    const CMD_BUFFER_STATE *sub_cb_state = NULL;
    std::unordered_set<const CMD_BUFFER_STATE *> linked_command_buffers;

    for (uint32_t i = 0; i < commandBuffersCount; i++) {
        sub_cb_state = GetCBState(pCommandBuffers[i]);
        assert(sub_cb_state);
        if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == sub_cb_state->createInfo.level) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCommandBuffers[i]), "VUID-vkCmdExecuteCommands-pCommandBuffers-00088",
                            "vkCmdExecuteCommands() called w/ Primary %s in element %u of pCommandBuffers array. All "
                            "cmd buffers in pCommandBuffers array must be secondary.",
                            report_data->FormatHandle(pCommandBuffers[i]).c_str(), i);
        } else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == sub_cb_state->createInfo.level) {
            if (sub_cb_state->beginInfo.pInheritanceInfo != nullptr) {
                const auto secondary_rp_state = GetRenderPassState(sub_cb_state->beginInfo.pInheritanceInfo->renderPass);
                if (cb_state->activeRenderPass &&
                    !(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(pCommandBuffers[i]), "VUID-vkCmdExecuteCommands-pCommandBuffers-00096",
                                    "vkCmdExecuteCommands(): Secondary %s is executed within a %s "
                                    "instance scope, but the Secondary Command Buffer does not have the "
                                    "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                    "the vkBeginCommandBuffer() was called.",
                                    report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    report_data->FormatHandle(cb_state->activeRenderPass->renderPass).c_str());
                } else if (!cb_state->activeRenderPass &&
                           (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(pCommandBuffers[i]), "VUID-vkCmdExecuteCommands-pCommandBuffers-00100",
                                    "vkCmdExecuteCommands(): Secondary %s is executed outside a render pass "
                                    "instance scope, but the Secondary Command Buffer does have the "
                                    "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                    "the vkBeginCommandBuffer() was called.",
                                    report_data->FormatHandle(pCommandBuffers[i]).c_str());
                } else if (cb_state->activeRenderPass &&
                           (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    // Make sure render pass is compatible with parent command buffer pass if has continue
                    if (cb_state->activeRenderPass->renderPass != secondary_rp_state->renderPass) {
                        skip |= ValidateRenderPassCompatibility(
                            "primary command buffer", cb_state->activeRenderPass, "secondary command buffer", secondary_rp_state,
                            "vkCmdExecuteCommands()", "VUID-vkCmdExecuteCommands-pInheritanceInfo-00098");
                    }
                    //  If framebuffer for secondary CB is not NULL, then it must match active FB from primaryCB
                    skip |=
                        ValidateFramebuffer(commandBuffer, cb_state, pCommandBuffers[i], sub_cb_state, "vkCmdExecuteCommands()");
                    if (!sub_cb_state->cmd_execute_commands_functions.empty()) {
                        //  Inherit primary's activeFramebuffer and while running validate functions
                        for (auto &function : sub_cb_state->cmd_execute_commands_functions) {
                            skip |= function(cb_state, cb_state->activeFramebuffer);
                        }
                    }
                }
            }
        }

        // TODO(mlentine): Move more logic into this method
        skip |= ValidateSecondaryCommandBufferState(cb_state, sub_cb_state);
        skip |= ValidateCommandBufferState(sub_cb_state, "vkCmdExecuteCommands()", 0,
                                           "VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
        if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (sub_cb_state->in_use.load()) {
                // TODO: Find some way to differentiate between the -00090 and -00091 conditions
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdExecuteCommands-pCommandBuffers-00090",
                                "Cannot execute pending %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                report_data->FormatHandle(sub_cb_state->commandBuffer).c_str());
            }
            // We use an const_cast, because one cannot query a container keyed on a non-const pointer using a const pointer
            if (cb_state->linkedCommandBuffers.count(const_cast<CMD_BUFFER_STATE *>(sub_cb_state))) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdExecuteCommands-pCommandBuffers-00092",
                    "Cannot execute %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set if previously executed in %s",
                    report_data->FormatHandle(sub_cb_state->commandBuffer).c_str(),
                    report_data->FormatHandle(cb_state->commandBuffer).c_str());
            }

            const auto insert_pair = linked_command_buffers.insert(sub_cb_state);
            if (!insert_pair.second) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdExecuteCommands-pCommandBuffers-00093",
                                "Cannot duplicate %s in pCommandBuffers without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                report_data->FormatHandle(cb_state->commandBuffer).c_str());
            }

            if (cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // Warn that non-simultaneous secondary cmd buffer renders primary non-simultaneous
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(pCommandBuffers[i]), kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse,
                                "vkCmdExecuteCommands(): Secondary %s does not have "
                                "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set and will cause primary "
                                "%s to be treated as if it does not have "
                                "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set, even though it does.",
                                report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                report_data->FormatHandle(cb_state->commandBuffer).c_str());
            }
        }
        if (!cb_state->activeQueries.empty() && !enabled_features.core.inheritedQueries) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCommandBuffers[i]), "VUID-vkCmdExecuteCommands-commandBuffer-00101",
                            "vkCmdExecuteCommands(): Secondary %s cannot be submitted with a query in flight and "
                            "inherited queries not supported on this device.",
                            report_data->FormatHandle(pCommandBuffers[i]).c_str());
        }
        // Validate initial layout uses vs. the primary cmd buffer state
        // Novel Valid usage: "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001"
        // initial layout usage of secondary command buffers resources must match parent command buffer
        const auto *const_cb_state = static_cast<const CMD_BUFFER_STATE *>(cb_state);
        for (const auto &sub_layout_map_entry : sub_cb_state->image_layout_map) {
            const auto image = sub_layout_map_entry.first;
            const auto *image_state = GetImageState(image);
            if (!image_state) continue;  // Can't set layouts of a dead image

            const auto *cb_subres_map = GetImageSubresourceLayoutMap(const_cb_state, image);
            // Const getter can be null in which case we have nothing to check against for this image...
            if (!cb_subres_map) continue;

            const auto &sub_cb_subres_map = sub_layout_map_entry.second;
            // Validate the initial_uses, that they match the current state of the primary cb, or absent a current state,
            // that the match any initial_layout.
            for (auto it_init = sub_cb_subres_map->BeginInitialUse(); !it_init.AtEnd(); ++it_init) {
                const auto &sub_layout = (*it_init).layout;
                if (VK_IMAGE_LAYOUT_UNDEFINED == sub_layout) continue;  // secondary doesn't care about current or initial
                const auto &subresource = (*it_init).subresource;
                // Look up the current layout (if any)
                VkImageLayout cb_layout = cb_subres_map->GetSubresourceLayout(subresource);
                const char *layout_type = "current";
                if (cb_layout == kInvalidLayout) {
                    // Find initial layout (if any)
                    cb_layout = cb_subres_map->GetSubresourceInitialLayout(subresource);
                    layout_type = "initial";
                }
                if ((cb_layout != kInvalidLayout) && (cb_layout != sub_layout)) {
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCommandBuffers[i]), "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001",
                            "%s: Executed secondary command buffer using %s (subresource: aspectMask 0x%X array layer %u, "
                            "mip level %u) which expects layout %s--instead, image %s layout is %s.",
                            "vkCmdExecuteCommands():", report_data->FormatHandle(image).c_str(), subresource.aspectMask,
                            subresource.arrayLayer, subresource.mipLevel, string_VkImageLayout(sub_layout), layout_type,
                            string_VkImageLayout(cb_layout));
                }
            }
        }
    }

    skip |= ValidatePrimaryCommandBuffer(cb_state, "vkCmdExecuteCommands()", "VUID-vkCmdExecuteCommands-bufferlevel");
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdExecuteCommands()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdExecuteCommands-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_EXECUTECOMMANDS, "vkCmdExecuteCommands()");
    return skip;
}

bool CoreChecks::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                          VkFlags flags, void **ppData) const {
    bool skip = false;
    const DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    if (mem_info) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                           HandleToUint64(mem), "VUID-vkMapMemory-memory-00682",
                           "Mapping Memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set: %s.",
                           report_data->FormatHandle(mem).c_str());
        }
        skip |= ValidateMapMemRange(mem_info, offset, size);
    }
    return skip;
}

void CoreChecks::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                         void **ppData, VkResult result) {
    if (VK_SUCCESS != result) return;
    StateTracker::PostCallRecordMapMemory(device, mem, offset, size, flags, ppData, result);
    InitializeShadowMemory(mem, offset, size, ppData);
}

bool CoreChecks::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem) const {
    bool skip = false;
    const auto mem_info = GetDevMemState(mem);
    if (mem_info && !mem_info->mapped_range.size) {
        // Valid Usage: memory must currently be mapped
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                        HandleToUint64(mem), "VUID-vkUnmapMemory-memory-00689", "Unmapping Memory without memory being mapped: %s.",
                        report_data->FormatHandle(mem).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    // Only core checks uses the shadow copy, clear that up here
    auto mem_info = GetDevMemState(mem);
    if (mem_info && mem_info->shadow_copy_base) {
        free(mem_info->shadow_copy_base);
        mem_info->shadow_copy_base = nullptr;
        mem_info->shadow_copy = nullptr;
        mem_info->shadow_pad_size = 0;
    }
    StateTracker::PreCallRecordUnmapMemory(device, mem);
}

bool CoreChecks::ValidateMemoryIsMapped(const char *funcName, uint32_t memRangeCount, const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    for (uint32_t i = 0; i < memRangeCount; ++i) {
        auto mem_info = GetDevMemState(pMemRanges[i].memory);
        if (mem_info) {
            if (pMemRanges[i].size == VK_WHOLE_SIZE) {
                if (mem_info->mapped_range.offset > pMemRanges[i].offset) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                    HandleToUint64(pMemRanges[i].memory), "VUID-VkMappedMemoryRange-size-00686",
                                    "%s: Flush/Invalidate offset (" PRINTF_SIZE_T_SPECIFIER
                                    ") is less than Memory Object's offset (" PRINTF_SIZE_T_SPECIFIER ").",
                                    funcName, static_cast<size_t>(pMemRanges[i].offset),
                                    static_cast<size_t>(mem_info->mapped_range.offset));
                }
            } else {
                const uint64_t data_end = (mem_info->mapped_range.size == VK_WHOLE_SIZE)
                                              ? mem_info->alloc_info.allocationSize
                                              : (mem_info->mapped_range.offset + mem_info->mapped_range.size);
                if ((mem_info->mapped_range.offset > pMemRanges[i].offset) ||
                    (data_end < (pMemRanges[i].offset + pMemRanges[i].size))) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                    HandleToUint64(pMemRanges[i].memory), "VUID-VkMappedMemoryRange-size-00685",
                                    "%s: Flush/Invalidate size or offset (" PRINTF_SIZE_T_SPECIFIER ", " PRINTF_SIZE_T_SPECIFIER
                                    ") exceed the Memory Object's upper-bound (" PRINTF_SIZE_T_SPECIFIER ").",
                                    funcName, static_cast<size_t>(pMemRanges[i].offset + pMemRanges[i].size),
                                    static_cast<size_t>(pMemRanges[i].offset), static_cast<size_t>(data_end));
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateAndCopyNoncoherentMemoryToDriver(uint32_t mem_range_count, const VkMappedMemoryRange *mem_ranges) const {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        auto mem_info = GetDevMemState(mem_ranges[i].memory);
        if (mem_info) {
            if (mem_info->shadow_copy) {
                VkDeviceSize size = (mem_info->mapped_range.size != VK_WHOLE_SIZE)
                                        ? mem_info->mapped_range.size
                                        : (mem_info->alloc_info.allocationSize - mem_info->mapped_range.offset);
                char *data = static_cast<char *>(mem_info->shadow_copy);
                for (uint64_t j = 0; j < mem_info->shadow_pad_size; ++j) {
                    if (data[j] != NoncoherentMemoryFillValue) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                        HandleToUint64(mem_ranges[i].memory), kVUID_Core_MemTrack_InvalidMap,
                                        "Memory underflow was detected on %s.",
                                        report_data->FormatHandle(mem_ranges[i].memory).c_str());
                    }
                }
                for (uint64_t j = (size + mem_info->shadow_pad_size); j < (2 * mem_info->shadow_pad_size + size); ++j) {
                    if (data[j] != NoncoherentMemoryFillValue) {
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                    HandleToUint64(mem_ranges[i].memory), kVUID_Core_MemTrack_InvalidMap,
                                    "Memory overflow was detected on %s.", report_data->FormatHandle(mem_ranges[i].memory).c_str());
                    }
                }
                memcpy(mem_info->p_driver_data, static_cast<void *>(data + mem_info->shadow_pad_size), (size_t)(size));
            }
        }
    }
    return skip;
}

void CoreChecks::CopyNoncoherentMemoryFromDriver(uint32_t mem_range_count, const VkMappedMemoryRange *mem_ranges) {
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        auto mem_info = GetDevMemState(mem_ranges[i].memory);
        if (mem_info && mem_info->shadow_copy) {
            VkDeviceSize size = (mem_info->mapped_range.size != VK_WHOLE_SIZE)
                                    ? mem_info->mapped_range.size
                                    : (mem_info->alloc_info.allocationSize - mem_ranges[i].offset);
            char *data = static_cast<char *>(mem_info->shadow_copy);
            memcpy(data + mem_info->shadow_pad_size, mem_info->p_driver_data, (size_t)(size));
        }
    }
}

bool CoreChecks::ValidateMappedMemoryRangeDeviceLimits(const char *func_name, uint32_t mem_range_count,
                                                       const VkMappedMemoryRange *mem_ranges) const {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        uint64_t atom_size = phys_dev_props.limits.nonCoherentAtomSize;
        if (SafeModulo(mem_ranges[i].offset, atom_size) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                            HandleToUint64(mem_ranges->memory), "VUID-VkMappedMemoryRange-offset-00687",
                            "%s: Offset in pMemRanges[%d] is 0x%" PRIxLEAST64
                            ", which is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x%" PRIxLEAST64 ").",
                            func_name, i, mem_ranges[i].offset, atom_size);
        }
        auto mem_info = GetDevMemState(mem_ranges[i].memory);
        if (mem_info) {
            if ((mem_ranges[i].size != VK_WHOLE_SIZE) &&
                (mem_ranges[i].size + mem_ranges[i].offset != mem_info->alloc_info.allocationSize) &&
                (SafeModulo(mem_ranges[i].size, atom_size) != 0)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                HandleToUint64(mem_ranges->memory), "VUID-VkMappedMemoryRange-size-01390",
                                "%s: Size in pMemRanges[%d] is 0x%" PRIxLEAST64
                                ", which is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x%" PRIxLEAST64 ").",
                                func_name, i, mem_ranges[i].size, atom_size);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                        const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    skip |= ValidateAndCopyNoncoherentMemoryToDriver(memRangeCount, pMemRanges);
    skip |= ValidateMemoryIsMapped("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    return skip;
}

bool CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                             const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits("vkInvalidateMappedMemoryRanges", memRangeCount, pMemRanges);
    skip |= ValidateMemoryIsMapped("vkInvalidateMappedMemoryRanges", memRangeCount, pMemRanges);
    return skip;
}

void CoreChecks::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                            const VkMappedMemoryRange *pMemRanges, VkResult result) {
    if (VK_SUCCESS == result) {
        // Update our shadow copy with modified driver data
        CopyNoncoherentMemoryFromDriver(memRangeCount, pMemRanges);
    }
}

bool CoreChecks::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize *pCommittedMem) const {
    bool skip = false;
    const auto mem_info = GetDevMemState(mem);

    if (mem_info) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                           HandleToUint64(mem), "VUID-vkGetDeviceMemoryCommitment-memory-00690",
                           "Querying commitment for memory without VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT set: %s.",
                           report_data->FormatHandle(mem).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateBindImageMemory(const VkBindImageMemoryInfo &bindInfo, const char *api_name) const {
    bool skip = false;
    const IMAGE_STATE *image_state = GetImageState(bindInfo.image);
    if (image_state) {
        // Track objects tied to memory
        uint64_t image_handle = HandleToUint64(bindInfo.image);
        skip = ValidateSetMemBinding(bindInfo.memory, VulkanTypedHandle(bindInfo.image, kVulkanObjectTypeImage), api_name);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        if (image_state->external_format_android) {
            if (image_state->memory_requirements_checked) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                kVUID_Core_BindImage_InvalidMemReqQuery,
                                "%s: Must not call vkGetImageMemoryRequirements on %s that will be bound to an external "
                                "Android hardware buffer.",
                                api_name, report_data->FormatHandle(bindInfo.image).c_str());
            }
            return skip;
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR

        // Validate bound memory range information
        const auto mem_info = GetDevMemState(bindInfo.memory);
        if (mem_info) {
            skip |= ValidateInsertImageMemoryRange(bindInfo.image, mem_info, bindInfo.memoryOffset, image_state->requirements,
                                                   image_state->createInfo.tiling == VK_IMAGE_TILING_LINEAR, api_name);
            skip |= ValidateMemoryTypes(mem_info, image_state->requirements.memoryTypeBits, api_name,
                                        "VUID-vkBindImageMemory-memory-01047");
        }

        // Validate memory requirements alignment
        if (SafeModulo(bindInfo.memoryOffset, image_state->requirements.alignment) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            "VUID-vkBindImageMemory-memoryOffset-01048",
                            "%s: memoryOffset is 0x%" PRIxLEAST64
                            " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                            ", returned from a call to vkGetImageMemoryRequirements with image.",
                            api_name, bindInfo.memoryOffset, image_state->requirements.alignment);
        }

        if (mem_info) {
            // Validate memory requirements size
            if (image_state->requirements.size > mem_info->alloc_info.allocationSize - bindInfo.memoryOffset) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            "VUID-vkBindImageMemory-size-01049",
                            "%s: memory size minus memoryOffset is 0x%" PRIxLEAST64
                            " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                            ", returned from a call to vkGetImageMemoryRequirements with image.",
                            api_name, mem_info->alloc_info.allocationSize - bindInfo.memoryOffset, image_state->requirements.size);
            }

            // Validate dedicated allocation
            if (mem_info->is_dedicated) {
                if (enabled_features.dedicated_allocation_image_aliasing_features.dedicatedAllocationImageAliasing) {
                    const auto orig_image_state = GetImageState(mem_info->dedicated_image);
                    const auto current_image_state = GetImageState(bindInfo.image);
                    if ((bindInfo.memoryOffset != 0) || !orig_image_state || !current_image_state ||
                        !current_image_state->IsCreateInfoDedicatedAllocationImageAliasingCompatible(
                            orig_image_state->createInfo)) {
                        const char *validation_error;
                        if (strcmp(api_name, "vkBindImageMemory()") == 0) {
                            validation_error = "VUID-vkBindImageMemory-memory-02629";
                        } else {
                            validation_error = "VUID-VkBindImageMemoryInfo-memory-02631";
                        }
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                    validation_error,
                                    "%s: for dedicated memory allocation %s, VkMemoryDedicatedAllocateInfoKHR:: %s must compatible "
                                    "with %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                    api_name, report_data->FormatHandle(bindInfo.memory).c_str(),
                                    report_data->FormatHandle(mem_info->dedicated_image).c_str(),
                                    report_data->FormatHandle(bindInfo.image).c_str(), bindInfo.memoryOffset);
                    }
                } else {
                    if ((bindInfo.memoryOffset != 0) || (mem_info->dedicated_image != bindInfo.image)) {
                        const char *validation_error;
                        if (strcmp(api_name, "vkBindImageMemory()") == 0) {
                            validation_error = "VUID-vkBindImageMemory-memory-01509";
                        } else {
                            validation_error = "VUID-VkBindImageMemoryInfo-memory-01903";
                        }
                        skip |=
                            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                    validation_error,
                                    "%s: for dedicated memory allocation %s, VkMemoryDedicatedAllocateInfoKHR:: %s must be equal "
                                    "to %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                    api_name, report_data->FormatHandle(bindInfo.memory).c_str(),
                                    report_data->FormatHandle(mem_info->dedicated_image).c_str(),
                                    report_data->FormatHandle(bindInfo.image).c_str(), bindInfo.memoryOffset);
                    }
                }
            }
        }

        const auto swapchain_info = lvl_find_in_chain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            if (bindInfo.memory != VK_NULL_HANDLE) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                "VUID-VkBindImageMemoryInfo-pNext-01631", "%s: %s is not VK_NULL_HANDLE.", api_name,
                                report_data->FormatHandle(bindInfo.memory).c_str());
            }
            if (image_state->create_from_swapchain != swapchain_info->swapchain) {
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), kVUID_Core_BindImageMemory_Swapchain,
                        "%s: %s is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                        "swapchain",
                        api_name, report_data->FormatHandle(image_state->image).c_str(),
                        report_data->FormatHandle(image_state->create_from_swapchain).c_str(),
                        report_data->FormatHandle(swapchain_info->swapchain).c_str());
            }
            const auto swapchain_state = GetSwapchainState(swapchain_info->swapchain);
            if (swapchain_state && swapchain_state->images.size() <= swapchain_info->imageIndex) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            "VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644",
                            "%s: imageIndex (%i) is out of bounds of %s images (size: %i)", api_name, swapchain_info->imageIndex,
                            report_data->FormatHandle(swapchain_info->swapchain).c_str(), (int)swapchain_state->images.size());
            }
        } else {
            if (image_state->create_from_swapchain) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                "VUID-VkBindImageMemoryInfo-image-01630",
                                "%s: pNext of VkBindImageMemoryInfo doesn't include VkBindImageMemorySwapchainInfoKHR.", api_name);
            }
            if (!mem_info) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                "VUID-VkBindImageMemoryInfo-pNext-01632", "%s: %s is invalid.", api_name,
                                report_data->FormatHandle(bindInfo.memory).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                VkDeviceSize memoryOffset) const {
    VkBindImageMemoryInfo bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindInfo.image = image;
    bindInfo.memory = mem;
    bindInfo.memoryOffset = memoryOffset;
    return ValidateBindImageMemory(bindInfo, "vkBindImageMemory()");
}

bool CoreChecks::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindImageMemoryInfoKHR *pBindInfos) const {
    bool skip = false;
    char api_name[128];
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i], api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfoKHR *pBindInfos) const {
    bool skip = false;
    char api_name[128];
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i], api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event) const {
    bool skip = false;
    const auto event_state = GetEventState(event);
    if (event_state) {
        if (event_state->write_in_use) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,
                            HandleToUint64(event), kVUID_Core_DrawState_QueueForwardProgress,
                            "Cannot call vkSetEvent() on %s that is already in use by a command buffer.",
                            report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                VkFence fence) const {
    const auto queue_data = GetQueueState(queue);
    const auto pFence = GetFenceState(fence);
    bool skip = ValidateFenceForSubmit(pFence);
    if (skip) {
        return true;
    }

    const auto queueFlags = GetPhysicalDeviceState()->queue_family_properties[queue_data->queueFamilyIndex].queueFlags;
    if (!(queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, HandleToUint64(queue),
            "VUID-vkQueueBindSparse-queuetype",
            "Attempting vkQueueBindSparse on a non-memory-management capable queue -- VK_QUEUE_SPARSE_BINDING_BIT not set.");
    }

    unordered_set<VkSemaphore> signaled_semaphores;
    unordered_set<VkSemaphore> unsignaled_semaphores;
    unordered_set<VkSemaphore> internal_semaphores;
    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; ++bindIdx) {
        const VkBindSparseInfo &bindInfo = pBindInfo[bindIdx];

        std::vector<SEMAPHORE_WAIT> semaphore_waits;
        std::vector<VkSemaphore> semaphore_signals;
        for (uint32_t i = 0; i < bindInfo.waitSemaphoreCount; ++i) {
            VkSemaphore semaphore = bindInfo.pWaitSemaphores[i];
            const auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore && (pSemaphore->scope == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
                if (unsignaled_semaphores.count(semaphore) ||
                    (!(signaled_semaphores.count(semaphore)) && !(pSemaphore->signaled))) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                    HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                    "%s is waiting on %s that has no way to be signaled.", report_data->FormatHandle(queue).c_str(),
                                    report_data->FormatHandle(semaphore).c_str());
                } else {
                    signaled_semaphores.erase(semaphore);
                    unsignaled_semaphores.insert(semaphore);
                }
            }
            if (pSemaphore && pSemaphore->scope == kSyncScopeExternalTemporary) {
                internal_semaphores.insert(semaphore);
            }
        }
        for (uint32_t i = 0; i < bindInfo.signalSemaphoreCount; ++i) {
            VkSemaphore semaphore = bindInfo.pSignalSemaphores[i];
            const auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore && pSemaphore->scope == kSyncScopeInternal) {
                if (signaled_semaphores.count(semaphore) || (!(unsignaled_semaphores.count(semaphore)) && pSemaphore->signaled)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                    HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                    "%s is signaling %s that was previously signaled by %s but has not since "
                                    "been waited on by any queue.",
                                    report_data->FormatHandle(queue).c_str(), report_data->FormatHandle(semaphore).c_str(),
                                    report_data->FormatHandle(pSemaphore->signaler.first).c_str());
                } else {
                    unsignaled_semaphores.erase(semaphore);
                    signaled_semaphores.insert(semaphore);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateImportSemaphore(VkSemaphore semaphore, const char *caller_name) const {
    bool skip = false;
    const SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    if (sema_node) {
        const VulkanTypedHandle obj_struct(semaphore, kVulkanObjectTypeSemaphore);
        skip |= ValidateObjectNotInUse(sema_node, obj_struct, caller_name, kVUIDUndefined);
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo) const {
    return ValidateImportSemaphore(pImportSemaphoreWin32HandleInfo->semaphore, "vkImportSemaphoreWin32HandleKHR");
}

#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                                                     const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) const {
    return ValidateImportSemaphore(pImportSemaphoreFdInfo->semaphore, "vkImportSemaphoreFdKHR");
}

bool CoreChecks::ValidateImportFence(VkFence fence, const char *caller_name) const {
    const FENCE_STATE *fence_node = GetFenceState(fence);
    bool skip = false;
    if (fence_node && fence_node->scope == kSyncScopeInternal && fence_node->state == FENCE_INFLIGHT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, HandleToUint64(fence),
                        kVUIDUndefined, "Cannot call %s on %s that is currently in use.", caller_name,
                        report_data->FormatHandle(fence).c_str());
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportFenceWin32HandleKHR(
    VkDevice device, const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo) const {
    return ValidateImportFence(pImportFenceWin32HandleInfo->fence, "vkImportFenceWin32HandleKHR");
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo) const {
    return ValidateImportFence(pImportFenceFdInfo->fence, "vkImportFenceFdKHR");
}

bool CoreChecks::ValidateCreateSwapchain(const char *func_name, VkSwapchainCreateInfoKHR const *pCreateInfo,
                                         const SURFACE_STATE *surface_state, const SWAPCHAIN_NODE *old_swapchain_state) const {
    // All physical devices and queue families are required to be able to present to any native window on Android; require the
    // application to have established support on any other platform.
    if (!instance_extensions.vk_khr_android_surface) {
        auto support_predicate = [this](decltype(surface_state->gpu_queue_support)::value_type qs) -> bool {
            // TODO: should restrict search only to queue families of VkDeviceQueueCreateInfos, not whole phys. device
            return (qs.first.gpu == physical_device) && qs.second;
        };
        const auto &support = surface_state->gpu_queue_support;
        bool is_supported = std::any_of(support.begin(), support.end(), support_predicate);

        if (!is_supported) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkSwapchainCreateInfoKHR-surface-01270",
                        "%s: pCreateInfo->surface is not known at this time to be supported for presentation by this device. The "
                        "vkGetPhysicalDeviceSurfaceSupportKHR() must be called beforehand, and it must return VK_TRUE support with "
                        "this surface for at least one queue family of this device.",
                        func_name))
                return true;
        }
    }

    if (old_swapchain_state) {
        if (old_swapchain_state->createInfo.surface != pCreateInfo->surface) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                        HandleToUint64(pCreateInfo->oldSwapchain), "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-01933",
                        "%s: pCreateInfo->oldSwapchain's surface is not pCreateInfo->surface", func_name))
                return true;
        }
        if (old_swapchain_state->retired) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                        HandleToUint64(pCreateInfo->oldSwapchain), "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-01933",
                        "%s: pCreateInfo->oldSwapchain is retired", func_name))
                return true;
        }
    }

    if ((pCreateInfo->imageExtent.width == 0) || (pCreateInfo->imageExtent.height == 0)) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-imageExtent-01689", "%s: pCreateInfo->imageExtent = (%d, %d) which is illegal.",
                    func_name, pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height))
            return true;
    }

    auto physical_device_state = GetPhysicalDeviceState();
    bool skip = false;
    VkSurfaceTransformFlagBitsKHR currentTransform = physical_device_state->surfaceCapabilities.currentTransform;
    if ((pCreateInfo->preTransform & currentTransform) != pCreateInfo->preTransform) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                        HandleToUint64(physical_device), kVUID_Core_Swapchain_PreTransform,
                        "%s: pCreateInfo->preTransform (%s) doesn't match the currentTransform (%s) returned by "
                        "vkGetPhysicalDeviceSurfaceCapabilitiesKHR, the presentation engine will transform the image "
                        "content as part of the presentation operation.",
                        func_name, string_VkSurfaceTransformFlagBitsKHR(pCreateInfo->preTransform),
                        string_VkSurfaceTransformFlagBitsKHR(currentTransform));
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_state->phys_device, pCreateInfo->surface, &capabilities);
    // Validate pCreateInfo->minImageCount against VkSurfaceCapabilitiesKHR::{min|max}ImageCount:
    if (pCreateInfo->minImageCount < capabilities.minImageCount) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-minImageCount-01271",
                    "%s called with minImageCount = %d, which is outside the bounds returned by "
                    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() (i.e. minImageCount = %d, maxImageCount = %d).",
                    func_name, pCreateInfo->minImageCount, capabilities.minImageCount, capabilities.maxImageCount))
            return true;
    }

    if ((capabilities.maxImageCount > 0) && (pCreateInfo->minImageCount > capabilities.maxImageCount)) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-minImageCount-01272",
                    "%s called with minImageCount = %d, which is outside the bounds returned by "
                    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() (i.e. minImageCount = %d, maxImageCount = %d).",
                    func_name, pCreateInfo->minImageCount, capabilities.minImageCount, capabilities.maxImageCount))
            return true;
    }

    // Validate pCreateInfo->imageExtent against VkSurfaceCapabilitiesKHR::{current|min|max}ImageExtent:
    if ((pCreateInfo->imageExtent.width < capabilities.minImageExtent.width) ||
        (pCreateInfo->imageExtent.width > capabilities.maxImageExtent.width) ||
        (pCreateInfo->imageExtent.height < capabilities.minImageExtent.height) ||
        (pCreateInfo->imageExtent.height > capabilities.maxImageExtent.height)) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-imageExtent-01274",
                    "%s called with imageExtent = (%d,%d), which is outside the bounds returned by "
                    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): currentExtent = (%d,%d), minImageExtent = (%d,%d), "
                    "maxImageExtent = (%d,%d).",
                    func_name, pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height, capabilities.currentExtent.width,
                    capabilities.currentExtent.height, capabilities.minImageExtent.width, capabilities.minImageExtent.height,
                    capabilities.maxImageExtent.width, capabilities.maxImageExtent.height))
            return true;
    }
    // pCreateInfo->preTransform should have exactly one bit set, and that bit must also be set in
    // VkSurfaceCapabilitiesKHR::supportedTransforms.
    if (!pCreateInfo->preTransform || (pCreateInfo->preTransform & (pCreateInfo->preTransform - 1)) ||
        !(pCreateInfo->preTransform & capabilities.supportedTransforms)) {
        // This is an error situation; one for which we'd like to give the developer a helpful, multi-line error message.  Build
        // it up a little at a time, and then log it:
        std::string errorString = "";
        char str[1024];
        // Here's the first part of the message:
        sprintf(str, "%s called with a non-supported pCreateInfo->preTransform (i.e. %s).  Supported values are:\n", func_name,
                string_VkSurfaceTransformFlagBitsKHR(pCreateInfo->preTransform));
        errorString += str;
        for (int i = 0; i < 32; i++) {
            // Build up the rest of the message:
            if ((1 << i) & capabilities.supportedTransforms) {
                const char *newStr = string_VkSurfaceTransformFlagBitsKHR((VkSurfaceTransformFlagBitsKHR)(1 << i));
                sprintf(str, "    %s\n", newStr);
                errorString += str;
            }
        }
        // Log the message that we've built up:
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-preTransform-01279", "%s.", errorString.c_str()))
            return true;
    }

    // pCreateInfo->compositeAlpha should have exactly one bit set, and that bit must also be set in
    // VkSurfaceCapabilitiesKHR::supportedCompositeAlpha
    if (!pCreateInfo->compositeAlpha || (pCreateInfo->compositeAlpha & (pCreateInfo->compositeAlpha - 1)) ||
        !((pCreateInfo->compositeAlpha) & capabilities.supportedCompositeAlpha)) {
        // This is an error situation; one for which we'd like to give the developer a helpful, multi-line error message.  Build
        // it up a little at a time, and then log it:
        std::string errorString = "";
        char str[1024];
        // Here's the first part of the message:
        sprintf(str, "%s called with a non-supported pCreateInfo->compositeAlpha (i.e. %s).  Supported values are:\n", func_name,
                string_VkCompositeAlphaFlagBitsKHR(pCreateInfo->compositeAlpha));
        errorString += str;
        for (int i = 0; i < 32; i++) {
            // Build up the rest of the message:
            if ((1 << i) & capabilities.supportedCompositeAlpha) {
                const char *newStr = string_VkCompositeAlphaFlagBitsKHR((VkCompositeAlphaFlagBitsKHR)(1 << i));
                sprintf(str, "    %s\n", newStr);
                errorString += str;
            }
        }
        // Log the message that we've built up:
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-compositeAlpha-01280", "%s.", errorString.c_str()))
            return true;
    }
    // Validate pCreateInfo->imageArrayLayers against VkSurfaceCapabilitiesKHR::maxImageArrayLayers:
    if (pCreateInfo->imageArrayLayers > capabilities.maxImageArrayLayers) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275",
                    "%s called with a non-supported imageArrayLayers (i.e. %d).  Maximum value is %d.", func_name,
                    pCreateInfo->imageArrayLayers, capabilities.maxImageArrayLayers))
            return true;
    }
    // Validate pCreateInfo->imageUsage against VkSurfaceCapabilitiesKHR::supportedUsageFlags:
    if (pCreateInfo->imageUsage != (pCreateInfo->imageUsage & capabilities.supportedUsageFlags)) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-imageUsage-01276",
                    "%s called with a non-supported pCreateInfo->imageUsage (i.e. 0x%08x).  Supported flag bits are 0x%08x.",
                    func_name, pCreateInfo->imageUsage, capabilities.supportedUsageFlags))
            return true;
    }

    if (device_extensions.vk_khr_surface_protected_capabilities && (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)) {
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR};
        surfaceInfo.surface = pCreateInfo->surface;
        VkSurfaceProtectedCapabilitiesKHR surfaceProtectedCapabilities = {VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR};
        VkSurfaceCapabilities2KHR surfaceCapabilities = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};
        surfaceCapabilities.pNext = &surfaceProtectedCapabilities;
        DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device_state->phys_device, &surfaceInfo, &surfaceCapabilities);

        if (!surfaceProtectedCapabilities.supportsProtected) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkSwapchainCreateInfoKHR-flags-03187",
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR but the surface "
                        "capabilities does not have VkSurfaceProtectedCapabilitiesKHR.supportsProtected set to VK_TRUE.",
                        func_name))
                return true;
        }
    }

    std::vector<VkSurfaceFormatKHR> surface_formats;
    const auto *surface_formats_ref = &surface_formats;

    // Validate pCreateInfo values with the results of vkGetPhysicalDeviceSurfaceFormatsKHR():
    if (physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
        uint32_t surface_format_count = 0;
        DispatchGetPhysicalDeviceSurfaceFormatsKHR(physical_device, pCreateInfo->surface, &surface_format_count, nullptr);
        surface_formats.resize(surface_format_count);
        DispatchGetPhysicalDeviceSurfaceFormatsKHR(physical_device, pCreateInfo->surface, &surface_format_count,
                                                   &surface_formats[0]);
    } else {
        surface_formats_ref = &physical_device_state->surface_formats;
    }

    {
        // Validate pCreateInfo->imageFormat against VkSurfaceFormatKHR::format:
        bool foundFormat = false;
        bool foundColorSpace = false;
        bool foundMatch = false;
        for (auto const &format : *surface_formats_ref) {
            if (pCreateInfo->imageFormat == format.format) {
                // Validate pCreateInfo->imageColorSpace against VkSurfaceFormatKHR::colorSpace:
                foundFormat = true;
                if (pCreateInfo->imageColorSpace == format.colorSpace) {
                    foundMatch = true;
                    break;
                }
            } else {
                if (pCreateInfo->imageColorSpace == format.colorSpace) {
                    foundColorSpace = true;
                }
            }
        }
        if (!foundMatch) {
            if (!foundFormat) {
                if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkSwapchainCreateInfoKHR-imageFormat-01273",
                            "%s called with a non-supported pCreateInfo->imageFormat (i.e. %d).", func_name,
                            pCreateInfo->imageFormat))
                    return true;
            }
            if (!foundColorSpace) {
                if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkSwapchainCreateInfoKHR-imageFormat-01273",
                            "%s called with a non-supported pCreateInfo->imageColorSpace (i.e. %d).", func_name,
                            pCreateInfo->imageColorSpace))
                    return true;
            }
        }
    }

    std::vector<VkPresentModeKHR> present_modes;
    const auto *present_modes_ref = &present_modes;

    // Validate pCreateInfo values with the results of vkGetPhysicalDeviceSurfacePresentModesKHR():
    if (physical_device_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS) {
        uint32_t present_mode_count = 0;
        DispatchGetPhysicalDeviceSurfacePresentModesKHR(physical_device_state->phys_device, pCreateInfo->surface,
                                                        &present_mode_count, nullptr);
        present_modes.resize(present_mode_count);
        DispatchGetPhysicalDeviceSurfacePresentModesKHR(physical_device_state->phys_device, pCreateInfo->surface,
                                                        &present_mode_count, &present_modes[0]);
    } else {
        present_modes_ref = &physical_device_state->present_modes;
    }

    // Validate pCreateInfo->presentMode against vkGetPhysicalDeviceSurfacePresentModesKHR():
    bool foundMatch =
        std::find(present_modes_ref->begin(), present_modes_ref->end(), pCreateInfo->presentMode) != present_modes_ref->end();
    if (!foundMatch) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    "VUID-VkSwapchainCreateInfoKHR-presentMode-01281", "%s called with a non-supported presentMode (i.e. %s).",
                    func_name, string_VkPresentModeKHR(pCreateInfo->presentMode)))
            return true;
    }

    // Validate state for shared presentable case
    if (VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == pCreateInfo->presentMode ||
        VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == pCreateInfo->presentMode) {
        if (!device_extensions.vk_khr_shared_presentable_image) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        kVUID_Core_DrawState_ExtensionNotEnabled,
                        "%s called with presentMode %s which requires the VK_KHR_shared_presentable_image extension, which has not "
                        "been enabled.",
                        func_name, string_VkPresentModeKHR(pCreateInfo->presentMode)))
                return true;
        } else if (pCreateInfo->minImageCount != 1) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkSwapchainCreateInfoKHR-minImageCount-01383",
                        "%s called with presentMode %s, but minImageCount value is %d. For shared presentable image, minImageCount "
                        "must be 1.",
                        func_name, string_VkPresentModeKHR(pCreateInfo->presentMode), pCreateInfo->minImageCount))
                return true;
        }
    }

    if (pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) {
        if (!device_extensions.vk_khr_swapchain_mutable_format) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        kVUID_Core_DrawState_ExtensionNotEnabled,
                        "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR which requires the "
                        "VK_KHR_swapchain_mutable_format extension, which has not been enabled.",
                        func_name))
                return true;
        } else {
            const auto *image_format_list = lvl_find_in_chain<VkImageFormatListCreateInfoKHR>(pCreateInfo->pNext);
            if (image_format_list == nullptr) {
                if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                            "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the pNext chain of "
                            "pCreateInfo does not contain an instance of VkImageFormatListCreateInfoKHR.",
                            func_name))
                    return true;
            } else if (image_format_list->viewFormatCount == 0) {
                if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                            "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the viewFormatCount "
                            "member of VkImageFormatListCreateInfoKHR in the pNext chain is zero.",
                            func_name))
                    return true;
            } else {
                bool found_base_format = false;
                for (uint32_t i = 0; i < image_format_list->viewFormatCount; ++i) {
                    if (image_format_list->pViewFormats[i] == pCreateInfo->imageFormat) {
                        found_base_format = true;
                        break;
                    }
                }
                if (!found_base_format) {
                    if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                                HandleToUint64(device), "VUID-VkSwapchainCreateInfoKHR-flags-03168",
                                "%s: pCreateInfo->flags contains VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but none of the "
                                "elements of the pViewFormats member of VkImageFormatListCreateInfoKHR match "
                                "pCreateInfo->imageFormat.",
                                func_name))
                        return true;
                }
            }
        }
    }

    if ((pCreateInfo->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && pCreateInfo->pQueueFamilyIndices) {
        bool skip1 =
            ValidateQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices, "vkCreateBuffer",
                                  "pCreateInfo->pQueueFamilyIndices", "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01428",
                                  "VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01428", false);
        if (skip1) return true;
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) const {
    const auto surface_state = GetSurfaceState(pCreateInfo->surface);
    const auto old_swapchain_state = GetSwapchainState(pCreateInfo->oldSwapchain);
    return ValidateCreateSwapchain("vkCreateSwapchainKHR()", pCreateInfo, surface_state, old_swapchain_state);
}

void CoreChecks::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  const VkAllocationCallbacks *pAllocator) {
    if (swapchain) {
        auto swapchain_data = GetSwapchainState(swapchain);
        if (swapchain_data) {
            for (const auto &swapchain_image : swapchain_data->images) {
                auto image_sub = imageSubresourceMap.find(swapchain_image.image);
                if (image_sub != imageSubresourceMap.end()) {
                    for (auto imgsubpair : image_sub->second) {
                        auto image_item = imageLayoutMap.find(imgsubpair);
                        if (image_item != imageLayoutMap.end()) {
                            imageLayoutMap.erase(image_item);
                        }
                    }
                    imageSubresourceMap.erase(image_sub);
                }
                EraseQFOImageRelaseBarriers(swapchain_image.image);
            }
        }
    }
    StateTracker::PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
}

bool CoreChecks::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                      VkImage *pSwapchainImages) const {
    auto swapchain_state = GetSwapchainState(swapchain);
    bool skip = false;
    if (swapchain_state && pSwapchainImages) {
        if (*pSwapchainImageCount > swapchain_state->get_swapchain_image_count) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        kVUID_Core_Swapchain_InvalidCount,
                        "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount, and with pSwapchainImages set to a "
                        "value (%d) that is greater than the value (%d) that was returned when pSwapchainImageCount was NULL.",
                        *pSwapchainImageCount, swapchain_state->get_swapchain_image_count);
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                     VkImage *pSwapchainImages, VkResult result) {
    // Usually we'd call the StateTracker first, but
    //     a) none of the new state needed below is from the StateTracker
    //     b) StateTracker *will* update swapchain_state->images which we use to guard against double initialization
    // so we'll do it in the opposite order -- CoreChecks then StateTracker.
    //
    // Note, this will get trickier if we start storing image shared pointers in the image layout data, at which point
    // we'll have to reverse the order *back* and find some other scheme to prevent double initialization.

    if (((result == VK_SUCCESS) || (result == VK_INCOMPLETE)) && pSwapchainImages) {
        // Initialze image layout tracking data
        auto swapchain_state = GetSwapchainState(swapchain);
        const auto image_vector_size = swapchain_state->images.size();
        IMAGE_LAYOUT_STATE image_layout_node;
        image_layout_node.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_layout_node.format = swapchain_state->createInfo.imageFormat;

        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            // This is check makes sure that we don't have an image initialized for this swapchain index, but
            // given that it's StateTracker that stores this information, need to protect against non-extant entries in the vector
            if ((i < image_vector_size) && (swapchain_state->images[i].image != VK_NULL_HANDLE)) continue;

            ImageSubresourcePair subpair = {pSwapchainImages[i], false, VkImageSubresource()};
            imageSubresourceMap[pSwapchainImages[i]].push_back(subpair);
            imageLayoutMap[subpair] = image_layout_node;
        }
    }

    // Now call the base class
    StateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
}

bool CoreChecks::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const {
    bool skip = false;
    const auto queue_state = GetQueueState(queue);

    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        const auto pSemaphore = GetSemaphoreState(pPresentInfo->pWaitSemaphores[i]);
        if (pSemaphore && !pSemaphore->signaled) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                            kVUID_Core_DrawState_QueueForwardProgress, "%s is waiting on %s that has no way to be signaled.",
                            report_data->FormatHandle(queue).c_str(),
                            report_data->FormatHandle(pPresentInfo->pWaitSemaphores[i]).c_str());
        }
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        const auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
        if (swapchain_data) {
            if (pPresentInfo->pImageIndices[i] >= swapchain_data->images.size()) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(pPresentInfo->pSwapchains[i]), kVUID_Core_DrawState_SwapchainInvalidImage,
                            "vkQueuePresentKHR: Swapchain image index too large (%u). There are only %u images in this swapchain.",
                            pPresentInfo->pImageIndices[i], (uint32_t)swapchain_data->images.size());
            } else {
                auto image = swapchain_data->images[pPresentInfo->pImageIndices[i]].image;
                const auto image_state = GetImageState(image);

                if (!image_state->acquired) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                    HandleToUint64(pPresentInfo->pSwapchains[i]), kVUID_Core_DrawState_SwapchainImageNotAcquired,
                                    "vkQueuePresentKHR: Swapchain image index %u has not been acquired.",
                                    pPresentInfo->pImageIndices[i]);
                }

                vector<VkImageLayout> layouts;
                if (FindLayouts(image, layouts)) {
                    for (auto layout : layouts) {
                        if ((layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) && (!device_extensions.vk_khr_shared_presentable_image ||
                                                                            (layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR))) {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                                            HandleToUint64(queue), "VUID-VkPresentInfoKHR-pImageIndices-01296",
                                            "Images passed to present must be in layout VK_IMAGE_LAYOUT_PRESENT_SRC_KHR or "
                                            "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR but is in %s.",
                                            string_VkImageLayout(layout));
                        }
                    }
                }
            }

            // All physical devices and queue families are required to be able to present to any native window on Android; require
            // the application to have established support on any other platform.
            if (!instance_extensions.vk_khr_android_surface) {
                const auto surface_state = GetSurfaceState(swapchain_data->createInfo.surface);
                auto support_it = surface_state->gpu_queue_support.find({physical_device, queue_state->queueFamilyIndex});

                if (support_it == surface_state->gpu_queue_support.end()) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                    HandleToUint64(pPresentInfo->pSwapchains[i]), kVUID_Core_DrawState_SwapchainUnsupportedQueue,
                                    "vkQueuePresentKHR: Presenting image without calling vkGetPhysicalDeviceSurfaceSupportKHR");
                } else if (!support_it->second) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                    HandleToUint64(pPresentInfo->pSwapchains[i]), "VUID-vkQueuePresentKHR-pSwapchains-01292",
                                    "vkQueuePresentKHR: Presenting image on queue that cannot present to this surface.");
                }
            }
        }
    }
    if (pPresentInfo->pNext) {
        // Verify ext struct
        const auto *present_regions = lvl_find_in_chain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                const auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
                assert(swapchain_data);
                VkPresentRegionKHR region = present_regions->pRegions[i];
                for (uint32_t j = 0; j < region.rectangleCount; ++j) {
                    VkRectLayerKHR rect = region.pRectangles[j];
                    if ((rect.offset.x + rect.extent.width) > swapchain_data->createInfo.imageExtent.width) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                        HandleToUint64(pPresentInfo->pSwapchains[i]), "VUID-VkRectLayerKHR-offset-01261",
                                        "vkQueuePresentKHR(): For VkPresentRegionKHR down pNext chain, "
                                        "pRegion[%i].pRectangles[%i], the sum of offset.x (%i) and extent.width (%i) is greater "
                                        "than the corresponding swapchain's imageExtent.width (%i).",
                                        i, j, rect.offset.x, rect.extent.width, swapchain_data->createInfo.imageExtent.width);
                    }
                    if ((rect.offset.y + rect.extent.height) > swapchain_data->createInfo.imageExtent.height) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                        HandleToUint64(pPresentInfo->pSwapchains[i]), "VUID-VkRectLayerKHR-offset-01261",
                                        "vkQueuePresentKHR(): For VkPresentRegionKHR down pNext chain, "
                                        "pRegion[%i].pRectangles[%i], the sum of offset.y (%i) and extent.height (%i) is greater "
                                        "than the corresponding swapchain's imageExtent.height (%i).",
                                        i, j, rect.offset.y, rect.extent.height, swapchain_data->createInfo.imageExtent.height);
                    }
                    if (rect.layer > swapchain_data->createInfo.imageArrayLayers) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(pPresentInfo->pSwapchains[i]), "VUID-VkRectLayerKHR-layer-01262",
                            "vkQueuePresentKHR(): For VkPresentRegionKHR down pNext chain, pRegion[%i].pRectangles[%i], the layer "
                            "(%i) is greater than the corresponding swapchain's imageArrayLayers (%i).",
                            i, j, rect.layer, swapchain_data->createInfo.imageArrayLayers);
                    }
                }
            }
        }

        const auto *present_times_info = lvl_find_in_chain<VkPresentTimesInfoGOOGLE>(pPresentInfo->pNext);
        if (present_times_info) {
            if (pPresentInfo->swapchainCount != present_times_info->swapchainCount) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(pPresentInfo->pSwapchains[0]), "VUID-VkPresentTimesInfoGOOGLE-swapchainCount-01247",
                            "vkQueuePresentKHR(): VkPresentTimesInfoGOOGLE.swapchainCount is %i but pPresentInfo->swapchainCount "
                            "is %i. For VkPresentTimesInfoGOOGLE down pNext chain of VkPresentInfoKHR, "
                            "VkPresentTimesInfoGOOGLE.swapchainCount must equal VkPresentInfoKHR.swapchainCount.",
                            present_times_info->swapchainCount, pPresentInfo->swapchainCount);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkSwapchainKHR *pSwapchains) const {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            const auto surface_state = GetSurfaceState(pCreateInfos[i].surface);
            const auto old_swapchain_state = GetSwapchainState(pCreateInfos[i].oldSwapchain);
            std::stringstream func_name;
            func_name << "vkCreateSharedSwapchainsKHR[" << swapchainCount << "]()";
            skip |= ValidateCreateSwapchain(func_name.str().c_str(), &pCreateInfos[i], surface_state, old_swapchain_state);
        }
    }
    return skip;
}

bool CoreChecks::ValidateAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                          VkFence fence, uint32_t *pImageIndex, const char *func_name) const {
    bool skip = false;
    if (fence == VK_NULL_HANDLE && semaphore == VK_NULL_HANDLE) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkAcquireNextImageKHR-semaphore-01780",
                        "%s: Semaphore and fence cannot both be VK_NULL_HANDLE. There would be no way to "
                        "determine the completion of this operation.",
                        func_name);
    }

    auto pSemaphore = GetSemaphoreState(semaphore);
    if (pSemaphore && pSemaphore->scope == kSyncScopeInternal && pSemaphore->signaled) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                        HandleToUint64(semaphore), "VUID-vkAcquireNextImageKHR-semaphore-01286",
                        "%s: Semaphore must not be currently signaled or in a wait state.", func_name);
    }

    auto pFence = GetFenceState(fence);
    if (pFence) {
        skip |= ValidateFenceForSubmit(pFence);
    }

    const auto swapchain_data = GetSwapchainState(swapchain);
    if (swapchain_data) {
        if (swapchain_data->retired) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(swapchain), "VUID-vkAcquireNextImageKHR-swapchain-01285",
                            "%s: This swapchain has been retired. The application can still present any images it "
                            "has acquired, but cannot acquire any more.",
                            func_name);
        }

        auto physical_device_state = GetPhysicalDeviceState();
        if (physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState != UNCALLED) {
            uint64_t acquired_images = std::count_if(swapchain_data->images.begin(), swapchain_data->images.end(),
                                                     [=](SWAPCHAIN_IMAGE image) { return GetImageState(image.image)->acquired; });
            if (acquired_images > swapchain_data->images.size() - physical_device_state->surfaceCapabilities.minImageCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                HandleToUint64(swapchain), kVUID_Core_DrawState_SwapchainTooManyImages,
                                "%s: Application has already acquired the maximum number of images (0x%" PRIxLEAST64 ")", func_name,
                                acquired_images);
            }
        }

        if (swapchain_data->images.size() == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(swapchain), kVUID_Core_DrawState_SwapchainImagesNotFound,
                            "%s: No images found to acquire from. Application probably did not call "
                            "vkGetSwapchainImagesKHR after swapchain creation.",
                            func_name);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) const {
    return ValidateAcquireNextImage(device, swapchain, timeout, semaphore, fence, pImageIndex, "vkAcquireNextImageKHR");
}

bool CoreChecks::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                     uint32_t *pImageIndex) const {
    bool skip = false;
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(pAcquireInfo->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                                    HandleToUint64(pAcquireInfo->swapchain),
                                                    "VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
    skip |= ValidateDeviceMaskToZero(pAcquireInfo->deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                                     HandleToUint64(pAcquireInfo->swapchain), "VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
    skip |= ValidateAcquireNextImage(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                     pAcquireInfo->fence, pImageIndex, "vkAcquireNextImage2KHR");
    return skip;
}

bool CoreChecks::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                  const VkAllocationCallbacks *pAllocator) const {
    const auto surface_state = GetSurfaceState(surface);
    bool skip = false;
    if ((surface_state) && (surface_state->swapchain)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                        HandleToUint64(instance), "VUID-vkDestroySurfaceKHR-surface-01266",
                        "vkDestroySurfaceKHR() called before its associated VkSwapchainKHR was destroyed.");
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                               uint32_t queueFamilyIndex,
                                                                               struct wl_display *display) const {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-queueFamilyIndex-01306",
                                    "vkGetPhysicalDeviceWaylandPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t queueFamilyIndex) const {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-queueFamilyIndex-01309",
                                    "vkGetPhysicalDeviceWin32PresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, xcb_connection_t *connection,
                                                                           xcb_visualid_t visual_id) const {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-queueFamilyIndex-01312",
                                    "vkGetPhysicalDeviceXcbPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecks::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex, Display *dpy,
                                                                            VisualID visualID) const {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-queueFamilyIndex-01315",
                                    "vkGetPhysicalDeviceXlibPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   VkSurfaceKHR surface, VkBool32 *pSupported) const {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(physical_device_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-queueFamilyIndex-01269",
                                    "vkGetPhysicalDeviceSurfaceSupportKHR", "queueFamilyIndex");
}

bool CoreChecks::ValidateDescriptorUpdateTemplate(const char *func_name,
                                                  const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo) const {
    bool skip = false;
    const auto layout = GetDescriptorSetLayoutShared(pCreateInfo->descriptorSetLayout);
    if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType && !layout) {
        const VulkanTypedHandle ds_typed(pCreateInfo->descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout);
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, ds_typed.handle,
                    "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350",
                    "%s: Invalid pCreateInfo->descriptorSetLayout (%s)", func_name, report_data->FormatHandle(ds_typed).c_str());
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR == pCreateInfo->templateType) {
        auto bind_point = pCreateInfo->pipelineBindPoint;
        bool valid_bp = (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) || (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE);
        if (!valid_bp) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351",
                        "%s: Invalid pCreateInfo->pipelineBindPoint (%" PRIu32 ").", func_name, static_cast<uint32_t>(bind_point));
        }
        const auto pipeline_layout = GetPipelineLayout(pCreateInfo->pipelineLayout);
        if (!pipeline_layout) {
            const VulkanTypedHandle pl_typed(pCreateInfo->pipelineLayout, kVulkanObjectTypePipelineLayout);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                            pl_typed.handle, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352",
                            "%s: Invalid pCreateInfo->pipelineLayout (%s)", func_name, report_data->FormatHandle(pl_typed).c_str());
        } else {
            const uint32_t pd_set = pCreateInfo->set;
            if ((pd_set >= pipeline_layout->set_layouts.size()) || !pipeline_layout->set_layouts[pd_set] ||
                !pipeline_layout->set_layouts[pd_set]->IsPushDescriptor()) {
                const VulkanTypedHandle pl_typed(pCreateInfo->pipelineLayout, kVulkanObjectTypePipelineLayout);
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                pl_typed.handle, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353",
                                "%s: pCreateInfo->set (%" PRIu32
                                ") does not refer to the push descriptor set layout for pCreateInfo->pipelineLayout (%s).",
                                func_name, pd_set, report_data->FormatHandle(pl_typed).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) const {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplate()", pCreateInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) const {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplateKHR()", pCreateInfo);
    return skip;
}

bool CoreChecks::ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                         const void *pData) const {
    bool skip = false;
    auto const template_map_entry = desc_template_map.find(descriptorUpdateTemplate);
    if ((template_map_entry == desc_template_map.end()) || (template_map_entry->second.get() == nullptr)) {
        // Object tracker will report errors for invalid descriptorUpdateTemplate values, avoiding a crash in release builds
        // but retaining the assert as template support is new enough to want to investigate these in debug builds.
        assert(0);
    } else {
        const TEMPLATE_STATE *template_state = template_map_entry->second.get();
        // TODO: Validate template push descriptor updates
        if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
            skip = ValidateUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state, pData);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const void *pData) const {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                   const void *pData) const {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set,
                                                                    const void *pData) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    const char *const func_name = "vkPushDescriptorSetWithTemplateKHR()";
    bool skip = false;
    skip |= ValidateCmd(cb_state, CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR, func_name);

    const auto layout_data = GetPipelineLayout(layout);
    const auto dsl = GetDslFromPipelineLayout(layout_data, set);
    const VulkanTypedHandle layout_typed(layout, kVulkanObjectTypePipelineLayout);

    // Validate the set index points to a push descriptor set and is in range
    if (dsl) {
        if (!dsl->IsPushDescriptor()) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                           layout_typed.handle, "VUID-vkCmdPushDescriptorSetKHR-set-00365",
                           "%s: Set index %" PRIu32 " does not match push descriptor set layout index for %s.", func_name, set,
                           report_data->FormatHandle(layout_typed).c_str());
        }
    } else if (layout_data && (set >= layout_data->set_layouts.size())) {
        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                       layout_typed.handle, "VUID-vkCmdPushDescriptorSetKHR-set-00364",
                       "%s: Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", func_name, set,
                       report_data->FormatHandle(layout_typed).c_str(), static_cast<uint32_t>(layout_data->set_layouts.size()));
    }

    const auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    if (template_state) {
        const auto &template_ci = template_state->create_info;
        static const std::map<VkPipelineBindPoint, std::string> bind_errors = {
            std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366"),
            std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366"),
            std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366")};
        skip |= ValidatePipelineBindPoint(cb_state, template_ci.pipelineBindPoint, func_name, bind_errors);

        if (template_ci.templateType != VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), kVUID_Core_PushDescriptorUpdate_TemplateType,
                            "%s: descriptorUpdateTemplate %s was not created with flag "
                            "VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR.",
                            func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str());
        }
        if (template_ci.set != set) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), kVUID_Core_PushDescriptorUpdate_Template_SetMismatched,
                            "%s: descriptorUpdateTemplate %s created with set %" PRIu32
                            " does not match command parameter set %" PRIu32 ".",
                            func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str(), template_ci.set, set);
        }
        if (!CompatForSet(set, layout_data, GetPipelineLayout(template_ci.pipelineLayout))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), kVUID_Core_PushDescriptorUpdate_Template_LayoutMismatched,
                            "%s: descriptorUpdateTemplate %s created with %s is incompatible with command parameter "
                            "%s for set %" PRIu32,
                            func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str(),
                            report_data->FormatHandle(template_ci.pipelineLayout).c_str(),
                            report_data->FormatHandle(layout).c_str(), set);
        }
    }

    if (dsl && template_state) {
        // Create an empty proxy in order to use the existing descriptor set update validation
        cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, nullptr, report_data);
        // Decode the template into a set of write updates
        cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state, pData,
                                                                dsl->GetDescriptorSetLayout());
        // Validate the decoded update against the proxy_ds
        skip |= ValidatePushDescriptorsUpdate(&proxy_ds, static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                              decoded_template.desc_writes.data(), func_name);
    }

    return skip;
}

bool CoreChecks::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                         const char *api_name) const {
    bool skip = false;
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    if (physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState == UNCALLED) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                        HandleToUint64(physicalDevice), kVUID_Core_Swapchain_GetSupportedDisplaysWithoutQuery,
                        "Potential problem with calling %s() without first retrieving properties from "
                        "vkGetPhysicalDeviceDisplayPlanePropertiesKHR or vkGetPhysicalDeviceDisplayPlaneProperties2KHR.",
                        api_name);
    } else {
        if (planeIndex >= physical_device_state->display_plane_property_count) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                HandleToUint64(physicalDevice), "VUID-vkGetDisplayPlaneSupportedDisplaysKHR-planeIndex-01249",
                "%s(): planeIndex must be in the range [0, %d] that was returned by vkGetPhysicalDeviceDisplayPlanePropertiesKHR "
                "or vkGetPhysicalDeviceDisplayPlaneProperties2KHR. Do you have the plane index hardcoded?",
                api_name, physical_device_state->display_plane_property_count - 1);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                    uint32_t *pDisplayCount, VkDisplayKHR *pDisplays) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex,
                                                                    "vkGetDisplayPlaneSupportedDisplaysKHR");
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                               uint32_t planeIndex,
                                                               VkDisplayPlaneCapabilitiesKHR *pCapabilities) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex, "vkGetDisplayPlaneCapabilitiesKHR");
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                                VkDisplayPlaneCapabilities2KHR *pCapabilities) const {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, pDisplayPlaneInfo->planeIndex,
                                                                    "vkGetDisplayPlaneCapabilities2KHR");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                                       const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmd(cb_state, CMD_DEBUGMARKERBEGINEXT, "vkCmdDebugMarkerBeginEXT()");
}

bool CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmd(cb_state, CMD_DEBUGMARKERENDEXT, "vkCmdDebugMarkerEndEXT()");
}

bool CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                        VkQueryControlFlags flags, uint32_t index) const {
    if (disabled.query_validation) return false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    QueryObject query_obj(queryPool, query, index);
    const char *cmd_name = "vkCmdBeginQueryIndexedEXT()";
    bool skip = ValidateBeginQuery(
        cb_state, query_obj, flags, CMD_BEGINQUERYINDEXEDEXT, cmd_name, "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-cmdpool",
        "VUID-vkCmdBeginQueryIndexedEXT-queryType-02338", "VUID-vkCmdBeginQueryIndexedEXT-queryType-00803",
        "VUID-vkCmdBeginQueryIndexedEXT-queryType-00800", "VUID-vkCmdBeginQueryIndexedEXT-query-00802");

    // Extension specific VU's
    const auto &query_pool_ci = GetQueryPoolState(query_obj.pool)->createInfo;
    if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
        if (device_extensions.vk_ext_transform_feedback &&
            (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBeginQueryIndexedEXT-queryType-02339",
                "%s: index %" PRIu32
                " must be less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32 ".",
                cmd_name, index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
    } else if (index != 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdBeginQueryIndexedEXT-queryType-02340",
                        "%s: index %" PRIu32
                        " must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT.",
                        cmd_name, index, report_data->FormatHandle(queryPool).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      VkQueryControlFlags flags, uint32_t index) {
    if (disabled.query_validation) return;
    QueryObject query_obj = {queryPool, query, index};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, "vkCmdBeginQueryIndexedEXT()");
}

bool CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      uint32_t index) const {
    if (disabled.query_validation) return false;
    QueryObject query_obj = {queryPool, query, index};
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmdEndQuery(cb_state, query_obj, CMD_ENDQUERYINDEXEDEXT, "vkCmdEndQueryIndexedEXT()",
                               "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-cmdpool", "VUID-vkCmdEndQueryIndexedEXT-None-02342");
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount,
                                                          const VkRect2D *pDiscardRectangles) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // Minimal validation for command buffer state
    return ValidateCmd(cb_state, CMD_SETDISCARDRECTANGLEEXT, "vkCmdSetDiscardRectangleEXT()");
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT *pSampleLocationsInfo) const {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // Minimal validation for command buffer state
    return ValidateCmd(cb_state, CMD_SETSAMPLELOCATIONSEXT, "vkCmdSetSampleLocationsEXT()");
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversion(const char *func_name,
                                                      const VkSamplerYcbcrConversionCreateInfo *create_info) const {
    bool skip = false;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateCreateSamplerYcbcrConversionANDROID(create_info);
    } else {  // Not android hardware buffer
        if (VK_FORMAT_UNDEFINED == create_info->format) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT, 0,
                            "VUID-VkSamplerYcbcrConversionCreateInfo-format-01649",
                            "%s: CreateInfo format type is VK_FORMAT_UNDEFINED.", func_name);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversion()", pCreateInfo);
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversionKHR()", pCreateInfo);
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo) const {
    bool skip = false;

    if (!enabled_features.buffer_address.bufferDeviceAddress) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                        HandleToUint64(pInfo->buffer), "VUID-vkGetBufferDeviceAddressEXT-None-02598",
                        "The bufferDeviceAddress feature must: be enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.buffer_address.bufferDeviceAddressMultiDevice) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                        HandleToUint64(pInfo->buffer), "VUID-vkGetBufferDeviceAddressEXT-device-02599",
                        "If device was created with multiple physical devices, then the "
                        "bufferDeviceAddressMultiDevice feature must: be enabled.");
    }

    const auto buffer_state = GetBufferState(pInfo->buffer);
    if (buffer_state) {
        if (!(buffer_state->createInfo.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkGetBufferDeviceAddressEXT()",
                                                  "VUID-VkBufferDeviceAddressInfoEXT-buffer-02600");
        }

        skip |= ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT, true,
                                         "VUID-VkBufferDeviceAddressInfoEXT-buffer-02601", "vkGetBufferDeviceAddressEXT()",
                                         "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT");
    }

    return skip;
}

bool CoreChecks::ValidateQueryRange(VkDevice device, VkQueryPool queryPool, uint32_t totalCount, uint32_t firstQuery,
                                    uint32_t queryCount, const char *vuid_badfirst, const char *vuid_badrange) const {
    bool skip = false;

    if (firstQuery >= totalCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        vuid_badfirst, "firstQuery (%" PRIu32 ") greater than or equal to query pool count (%" PRIu32 ") for %s",
                        firstQuery, totalCount, report_data->FormatHandle(queryPool).c_str());
    }

    if ((firstQuery + queryCount) > totalCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        vuid_badrange, "Query range [%" PRIu32 ", %" PRIu32 ") goes beyond query pool count (%" PRIu32 ") for %s",
                        firstQuery, firstQuery + queryCount, totalCount, report_data->FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) const {
    if (disabled.query_validation) return false;

    bool skip = false;

    if (!enabled_features.host_query_reset_features.hostQueryReset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkResetQueryPoolEXT-None-02665", "Host query reset not enabled for device");
    }

    const auto query_pool_state = GetQueryPoolState(queryPool);
    if (query_pool_state) {
        skip |= ValidateQueryRange(device, queryPool, query_pool_state->createInfo.queryCount, firstQuery, queryCount,
                                   "VUID-vkResetQueryPoolEXT-firstQuery-02666", "VUID-vkResetQueryPoolEXT-firstQuery-02667");
    }

    return skip;
}

VkResult CoreChecks::CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       VkValidationCacheEXT *pValidationCache) {
    *pValidationCache = ValidationCache::Create(pCreateInfo);
    return *pValidationCache ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

void CoreChecks::CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks *pAllocator) {
    delete CastFromHandle<ValidationCache *>(validationCache);
}

VkResult CoreChecks::CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize,
                                                        void *pData) {
    size_t inSize = *pDataSize;
    CastFromHandle<ValidationCache *>(validationCache)->Write(pDataSize, pData);
    return (pData && *pDataSize != inSize) ? VK_INCOMPLETE : VK_SUCCESS;
}

VkResult CoreChecks::CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                       const VkValidationCacheEXT *pSrcCaches) {
    bool skip = false;
    auto dst = CastFromHandle<ValidationCache *>(dstCache);
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < srcCacheCount; i++) {
        auto src = CastFromHandle<const ValidationCache *>(pSrcCaches[i]);
        if (src == dst) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT, 0,
                            "VUID-vkMergeValidationCachesEXT-dstCache-01536",
                            "vkMergeValidationCachesEXT: dstCache (0x%" PRIx64 ") must not appear in pSrcCaches array.",
                            HandleToUint64(dstCache));
            result = VK_ERROR_VALIDATION_FAILED_EXT;
        }
        if (!skip) {
            dst->Merge(src);
        }
    }

    return result;
}

bool CoreChecks::ValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const char *func_name) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    skip |= ValidateCmd(cb_state, CMD_SETDEVICEMASK, func_name);
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                                    HandleToUint64(commandBuffer), "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    skip |= ValidateDeviceMaskToZero(deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(commandBuffer),
                                     "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    skip |= ValidateDeviceMaskToCommandBuffer(cb_state, deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                              HandleToUint64(commandBuffer), "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    if (cb_state->activeRenderPass) {
        skip |= ValidateDeviceMaskToRenderPass(cb_state, deviceMask, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                               HandleToUint64(commandBuffer), "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    return ValidateCmdSetDeviceMask(commandBuffer, deviceMask, "vkSetDeviceMask()");
}

bool CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    return ValidateCmdSetDeviceMask(commandBuffer, deviceMask, "vkSetDeviceMaskKHR()");
}

bool CoreChecks::ValidateQueryPoolStride(const std::string &vuid_not_64, const std::string &vuid_64, const VkDeviceSize stride,
                                         const char *parameter_name, const uint64_t parameter_value,
                                         const VkQueryResultFlags flags) const {
    bool skip = false;
    if (flags & VK_QUERY_RESULT_64_BIT) {
        static const int condition_multiples = 0b0111;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid_64,
                            "stride %" PRIx64 " or %s %" PRIx64 " is invalid.", stride, parameter_name, parameter_value);
        }
    } else {
        static const int condition_multiples = 0b0011;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid_not_64,
                            "stride %" PRIx64 " or %s %" PRIx64 " is invalid.", stride, parameter_name, parameter_value);
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawStrideWithStruct(VkCommandBuffer commandBuffer, const std::string &vuid, const uint32_t stride,
                                                 const char *struct_name, const uint32_t struct_size) const {
    bool skip = false;
    static const int condition_multiples = 0b0011;
    if ((stride & condition_multiples) || (stride < struct_size)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), vuid, "stride %d is invalid or less than sizeof(%s) %d.", stride,
                        struct_name, struct_size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawStrideWithBuffer(VkCommandBuffer commandBuffer, const std::string &vuid, const uint32_t stride,
                                                 const char *struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                                 const VkDeviceSize offset, const BUFFER_STATE *buffer_state) const {
    bool skip = false;
    uint64_t validation_value = stride * (drawCount - 1) + offset + struct_size;
    if (validation_value > buffer_state->createInfo.size) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), vuid,
                        "stride[%d] * (drawCount[%d] - 1) + offset[%" PRIx64 "] + sizeof(%s)[%d] = %" PRIx64
                        " is greater than the size[%" PRIx64 "] of %s.",
                        stride, drawCount, offset, struct_name, struct_size, validation_value, buffer_state->createInfo.size,
                        report_data->FormatHandle(buffer_state->buffer).c_str());
    }
    return skip;
}

void PIPELINE_STATE::initGraphicsPipeline(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                          std::shared_ptr<const RENDER_PASS_STATE> &&rpstate) {
    reset();
    bool uses_color_attachment = false;
    bool uses_depthstencil_attachment = false;
    if (pCreateInfo->subpass < rpstate->createInfo.subpassCount) {
        const auto &subpass = rpstate->createInfo.pSubpasses[pCreateInfo->subpass];

        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            if (subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                uses_color_attachment = true;
                break;
            }
        }

        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            uses_depthstencil_attachment = true;
        }
    }
    graphicsPipelineCI.initialize(pCreateInfo, uses_color_attachment, uses_depthstencil_attachment);
    if (graphicsPipelineCI.pInputAssemblyState) {
        topology_at_rasterizer = graphicsPipelineCI.pInputAssemblyState->topology;
    }

    stage_state.resize(pCreateInfo->stageCount);
    for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
        const VkPipelineShaderStageCreateInfo *pPSSCI = &pCreateInfo->pStages[i];
        this->duplicate_shaders |= this->active_shaders & pPSSCI->stage;
        this->active_shaders |= pPSSCI->stage;
        state_data->RecordPipelineShaderStage(pPSSCI, this, &stage_state[i]);
    }

    if (graphicsPipelineCI.pVertexInputState) {
        const auto pVICI = graphicsPipelineCI.pVertexInputState;
        if (pVICI->vertexBindingDescriptionCount) {
            this->vertex_binding_descriptions_ = std::vector<VkVertexInputBindingDescription>(
                pVICI->pVertexBindingDescriptions, pVICI->pVertexBindingDescriptions + pVICI->vertexBindingDescriptionCount);

            this->vertex_binding_to_index_map_.reserve(pVICI->vertexBindingDescriptionCount);
            for (uint32_t i = 0; i < pVICI->vertexBindingDescriptionCount; ++i) {
                this->vertex_binding_to_index_map_[pVICI->pVertexBindingDescriptions[i].binding] = i;
            }
        }
        if (pVICI->vertexAttributeDescriptionCount) {
            this->vertex_attribute_descriptions_ = std::vector<VkVertexInputAttributeDescription>(
                pVICI->pVertexAttributeDescriptions, pVICI->pVertexAttributeDescriptions + pVICI->vertexAttributeDescriptionCount);
            for (uint32_t i = 0; i < pVICI->vertexAttributeDescriptionCount; ++i) {
                const auto attribute_format = pVICI->pVertexAttributeDescriptions[i].format;
                VkDeviceSize vtx_attrib_req_alignment = FormatElementSize(attribute_format);
                if (FormatElementIsTexel(attribute_format)) {
                    vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, FormatChannelCount(attribute_format));
                }
                this->vertex_attribute_alignments_.push_back(vtx_attrib_req_alignment);
            }
        }
    }
    if (graphicsPipelineCI.pColorBlendState) {
        const auto pCBCI = graphicsPipelineCI.pColorBlendState;
        if (pCBCI->attachmentCount) {
            this->attachments =
                std::vector<VkPipelineColorBlendAttachmentState>(pCBCI->pAttachments, pCBCI->pAttachments + pCBCI->attachmentCount);
        }
    }
    rp_state = rpstate;
}

void PIPELINE_STATE::initComputePipeline(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo) {
    reset();
    computePipelineCI.initialize(pCreateInfo);
    switch (computePipelineCI.stage.stage) {
        case VK_SHADER_STAGE_COMPUTE_BIT:
            this->active_shaders |= VK_SHADER_STAGE_COMPUTE_BIT;
            stage_state.resize(1);
            state_data->RecordPipelineShaderStage(&pCreateInfo->stage, this, &stage_state[0]);
            break;
        default:
            // TODO : Flag error
            break;
    }
}

void PIPELINE_STATE::initRayTracingPipelineNV(const ValidationStateTracker *state_data,
                                              const VkRayTracingPipelineCreateInfoNV *pCreateInfo) {
    reset();
    raytracingPipelineCI.initialize(pCreateInfo);

    stage_state.resize(pCreateInfo->stageCount);
    for (uint32_t stage_index = 0; stage_index < pCreateInfo->stageCount; stage_index++) {
        const auto &shader_stage = pCreateInfo->pStages[stage_index];
        switch (shader_stage.stage) {
            case VK_SHADER_STAGE_RAYGEN_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_RAYGEN_BIT_NV;
                break;
            case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_ANY_HIT_BIT_NV;
                break;
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
                break;
            case VK_SHADER_STAGE_MISS_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_MISS_BIT_NV;
                break;
            case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_INTERSECTION_BIT_NV;
                break;
            case VK_SHADER_STAGE_CALLABLE_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_CALLABLE_BIT_NV;
                break;
            default:
                // TODO : Flag error
                break;
        }
        state_data->RecordPipelineShaderStage(&shader_stage, this, &stage_state[stage_index]);
    }
}
