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

// WSI Image Objects bypass usual Image Object creation methods.  A special Memory
// Object value will be used to identify them internally.
static const VkDeviceMemory MEMTRACKER_SWAP_CHAIN_IMAGE_KEY = (VkDeviceMemory)(-1);
// 2nd special memory handle used to flag object as unbound from memory
static const VkDeviceMemory MEMORY_UNBOUND = VkDeviceMemory(~((uint64_t)(0)) - 1);

// Get the global map of pending releases
GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkImageMemoryBarrier>::Tag &type_tag) {
    return qfo_release_image_barrier_map;
}
GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> &CoreChecks::GetGlobalQFOReleaseBarrierMap(
    const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag &type_tag) {
    return qfo_release_buffer_barrier_map;
}

// Get the image viewstate for a given framebuffer attachment
IMAGE_VIEW_STATE *ValidationStateTracker::GetAttachmentImageViewState(FRAMEBUFFER_STATE *framebuffer, uint32_t index) {
    assert(framebuffer && (index < framebuffer->createInfo.attachmentCount));
    const VkImageView &image_view = framebuffer->createInfo.pAttachments[index];
    return GetImageViewState(image_view);
}

EVENT_STATE *ValidationStateTracker::GetEventState(VkEvent event) {
    auto it = eventMap.find(event);
    if (it == eventMap.end()) {
        return nullptr;
    }
    return &it->second;
}

QUEUE_STATE *ValidationStateTracker::GetQueueState(VkQueue queue) {
    auto it = queueMap.find(queue);
    if (it == queueMap.end()) {
        return nullptr;
    }
    return &it->second;
}

PHYSICAL_DEVICE_STATE *ValidationStateTracker::GetPhysicalDeviceState(VkPhysicalDevice phys) {
    auto *phys_dev_map = ((physical_device_map.size() > 0) ? &physical_device_map : &instance_state->physical_device_map);
    auto it = phys_dev_map->find(phys);
    if (it == phys_dev_map->end()) {
        return nullptr;
    }
    return &it->second;
}

PHYSICAL_DEVICE_STATE *ValidationStateTracker::GetPhysicalDeviceState() { return physical_device_state; }
const PHYSICAL_DEVICE_STATE *ValidationStateTracker::GetPhysicalDeviceState() const { return physical_device_state; }

// Return ptr to memory binding for given handle of specified type
BINDABLE *ValidationStateTracker::GetObjectMemBinding(const VulkanTypedHandle &typed_handle) {
    switch (typed_handle.type) {
        case kVulkanObjectTypeImage:
            return GetImageState(VkImage(typed_handle.handle));
        case kVulkanObjectTypeBuffer:
            return GetBufferState(VkBuffer(typed_handle.handle));
        case kVulkanObjectTypeAccelerationStructureNV:
            return GetAccelerationStructureState(VkAccelerationStructureNV(typed_handle.handle));
        default:
            break;
    }
    return nullptr;
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

void ValidationStateTracker::AddMemObjInfo(void *object, const VkDeviceMemory mem, const VkMemoryAllocateInfo *pAllocateInfo) {
    assert(object != NULL);

    auto *mem_info = new DEVICE_MEMORY_STATE(object, mem, pAllocateInfo);
    memObjMap[mem] = unique_ptr<DEVICE_MEMORY_STATE>(mem_info);

    auto dedicated = lvl_find_in_chain<VkMemoryDedicatedAllocateInfoKHR>(pAllocateInfo->pNext);
    if (dedicated) {
        mem_info->is_dedicated = true;
        mem_info->dedicated_buffer = dedicated->buffer;
        mem_info->dedicated_image = dedicated->image;
    }
    auto export_info = lvl_find_in_chain<VkExportMemoryAllocateInfo>(pAllocateInfo->pNext);
    if (export_info) {
        mem_info->is_export = true;
        mem_info->export_handle_type_flags = export_info->handleTypes;
    }
}

// Create binding link between given sampler and command buffer node
void CoreChecks::AddCommandBufferBindingSampler(CMD_BUFFER_STATE *cb_node, SAMPLER_STATE *sampler_state) {
    auto inserted = cb_node->object_bindings.emplace(sampler_state->sampler, kVulkanObjectTypeSampler);
    if (inserted.second) {
        // Only need to complete the cross-reference if this is a new item
        sampler_state->cb_bindings.insert(cb_node);
    }
}

// Create binding link between given image node and command buffer node
void ValidationStateTracker::AddCommandBufferBindingImage(CMD_BUFFER_STATE *cb_node, IMAGE_STATE *image_state) {
    // Skip validation if this image was created through WSI
    if (image_state->binding.mem != MEMTRACKER_SWAP_CHAIN_IMAGE_KEY) {
        // First update cb binding for image
        auto image_inserted = cb_node->object_bindings.emplace(image_state->image, kVulkanObjectTypeImage);
        if (image_inserted.second) {
            // Only need to continue if this is a new item (the rest of the work would have be done previous)
            image_state->cb_bindings.insert(cb_node);
            // Now update CB binding in MemObj mini CB list
            for (auto mem_binding : image_state->GetBoundMemory()) {
                DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
                if (pMemInfo) {
                    // Now update CBInfo's Mem reference list
                    auto mem_inserted = cb_node->memObjs.insert(mem_binding);
                    if (mem_inserted.second) {
                        // Only need to complete the cross-reference if this is a new item
                        pMemInfo->cb_bindings.insert(cb_node);
                    }
                }
            }
        }
    }
}

// Create binding link between given image view node and its image with command buffer node
void ValidationStateTracker::AddCommandBufferBindingImageView(CMD_BUFFER_STATE *cb_node, IMAGE_VIEW_STATE *view_state) {
    // First add bindings for imageView
    auto inserted = cb_node->object_bindings.emplace(view_state->image_view, kVulkanObjectTypeImageView);
    if (inserted.second) {
        // Only need to continue if this is a new item
        view_state->cb_bindings.insert(cb_node);
        auto image_state = GetImageState(view_state->create_info.image);
        // Add bindings for image within imageView
        if (image_state) {
            AddCommandBufferBindingImage(cb_node, image_state);
        }
    }
}

// Create binding link between given buffer node and command buffer node
void ValidationStateTracker::AddCommandBufferBindingBuffer(CMD_BUFFER_STATE *cb_node, BUFFER_STATE *buffer_state) {
    // First update cb binding for buffer
    auto buffer_inserted = cb_node->object_bindings.emplace(buffer_state->buffer, kVulkanObjectTypeBuffer);
    if (buffer_inserted.second) {
        // Only need to continue if this is a new item
        buffer_state->cb_bindings.insert(cb_node);
        // Now update CB binding in MemObj mini CB list
        for (auto mem_binding : buffer_state->GetBoundMemory()) {
            DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
            if (pMemInfo) {
                // Now update CBInfo's Mem reference list
                auto inserted = cb_node->memObjs.insert(mem_binding);
                if (inserted.second) {
                    // Only need to complete the cross-reference if this is a new item
                    pMemInfo->cb_bindings.insert(cb_node);
                }
            }
        }
    }
}

// Create binding link between given buffer view node and its buffer with command buffer node
void ValidationStateTracker::AddCommandBufferBindingBufferView(CMD_BUFFER_STATE *cb_node, BUFFER_VIEW_STATE *view_state) {
    // First add bindings for bufferView
    auto inserted = cb_node->object_bindings.emplace(view_state->buffer_view, kVulkanObjectTypeBufferView);
    if (inserted.second) {
        // Only need to complete the cross-reference if this is a new item
        view_state->cb_bindings.insert(cb_node);
        auto buffer_state = GetBufferState(view_state->create_info.buffer);
        // Add bindings for buffer within bufferView
        if (buffer_state) {
            AddCommandBufferBindingBuffer(cb_node, buffer_state);
        }
    }
}

// Create binding link between given acceleration structure and command buffer node
void CoreChecks::AddCommandBufferBindingAccelerationStructure(CMD_BUFFER_STATE *cb_node, ACCELERATION_STRUCTURE_STATE *as_state) {
    auto as_inserted = cb_node->object_bindings.emplace(as_state->acceleration_structure, kVulkanObjectTypeAccelerationStructureNV);
    if (as_inserted.second) {
        // Only need to complete the cross-reference if this is a new item
        as_state->cb_bindings.insert(cb_node);
        // Now update CB binding in MemObj mini CB list
        for (auto mem_binding : as_state->GetBoundMemory()) {
            DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
            if (pMemInfo) {
                // Now update CBInfo's Mem reference list
                auto mem_inserted = cb_node->memObjs.insert(mem_binding);
                if (mem_inserted.second) {
                    // Only need to complete the cross-reference if this is a new item
                    pMemInfo->cb_bindings.insert(cb_node);
                }
            }
        }
    }
}

// For every mem obj bound to particular CB, free bindings related to that CB
void ValidationStateTracker::ClearCmdBufAndMemReferences(CMD_BUFFER_STATE *cb_node) {
    if (cb_node) {
        if (cb_node->memObjs.size() > 0) {
            for (auto mem : cb_node->memObjs) {
                DEVICE_MEMORY_STATE *pInfo = GetDevMemState(mem);
                if (pInfo) {
                    pInfo->cb_bindings.erase(cb_node);
                }
            }
            cb_node->memObjs.clear();
        }
    }
}

// Clear a single object binding from given memory object
void ValidationStateTracker::ClearMemoryObjectBinding(const VulkanTypedHandle &typed_handle, VkDeviceMemory mem) {
    DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    // This obj is bound to a memory object. Remove the reference to this object in that memory object's list
    if (mem_info) {
        mem_info->obj_bindings.erase(typed_handle);
    }
}

// ClearMemoryObjectBindings clears the binding of objects to memory
//  For the given object it pulls the memory bindings and makes sure that the bindings
//  no longer refer to the object being cleared. This occurs when objects are destroyed.
void ValidationStateTracker::ClearMemoryObjectBindings(const VulkanTypedHandle &typed_handle) {
    BINDABLE *mem_binding = GetObjectMemBinding(typed_handle);
    if (mem_binding) {
        if (!mem_binding->sparse) {
            ClearMemoryObjectBinding(typed_handle, mem_binding->binding.mem);
        } else {  // Sparse, clear all bindings
            for (auto &sparse_mem_binding : mem_binding->sparse_bindings) {
                ClearMemoryObjectBinding(typed_handle, sparse_mem_binding.mem);
            }
        }
    }
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

// SetMemBinding is used to establish immutable, non-sparse binding between a single image/buffer object and memory object.
// Corresponding valid usage checks are in ValidateSetMemBinding().
void CoreChecks::SetMemBinding(VkDeviceMemory mem, BINDABLE *mem_binding, VkDeviceSize memory_offset,
                               const VulkanTypedHandle &typed_handle) {
    assert(mem_binding);
    mem_binding->binding.mem = mem;
    mem_binding->UpdateBoundMemorySet();  // force recreation of cached set
    mem_binding->binding.offset = memory_offset;
    mem_binding->binding.size = mem_binding->requirements.size;

    if (mem != VK_NULL_HANDLE) {
        DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
        if (mem_info) {
            mem_info->obj_bindings.insert(typed_handle);
            // For image objects, make sure default memory state is correctly set
            // TODO : What's the best/correct way to handle this?
            if (kVulkanObjectTypeImage == typed_handle.type) {
                auto const image_state = reinterpret_cast<const IMAGE_STATE *>(mem_binding);
                if (image_state) {
                    VkImageCreateInfo ici = image_state->createInfo;
                    if (ici.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                        // TODO::  More memory state transition stuff.
                    }
                }
            }
        }
    }
}

// Valid usage checks for a call to SetMemBinding().
// For NULL mem case, output warning
// Make sure given object is in global object map
//  IF a previous binding existed, output validation error
//  Otherwise, add reference from objectInfo to memoryInfo
//  Add reference off of objInfo
// TODO: We may need to refactor or pass in multiple valid usage statements to handle multiple valid usage conditions.
bool CoreChecks::ValidateSetMemBinding(VkDeviceMemory mem, const VulkanTypedHandle &typed_handle, const char *apiName) {
    bool skip = false;
    // It's an error to bind an object to NULL memory
    if (mem != VK_NULL_HANDLE) {
        BINDABLE *mem_binding = GetObjectMemBinding(typed_handle);
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
        DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
        if (mem_info) {
            DEVICE_MEMORY_STATE *prev_binding = GetDevMemState(mem_binding->binding.mem);
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

// For NULL mem case, clear any previous binding Else...
// Make sure given object is in its object map
//  IF a previous binding existed, update binding
//  Add reference from objectInfo to memoryInfo
//  Add reference off of object's binding info
// Return VK_TRUE if addition is successful, VK_FALSE otherwise
bool CoreChecks::SetSparseMemBinding(MEM_BINDING binding, const VulkanTypedHandle &typed_handle) {
    bool skip = VK_FALSE;
    // Handle NULL case separately, just clear previous binding & decrement reference
    if (binding.mem == VK_NULL_HANDLE) {
        // TODO : This should cause the range of the resource to be unbound according to spec
    } else {
        BINDABLE *mem_binding = GetObjectMemBinding(typed_handle);
        assert(mem_binding);
        if (mem_binding) {  // Invalid handles are reported by object tracker, but Get returns NULL for them, so avoid SEGV here
            assert(mem_binding->sparse);
            DEVICE_MEMORY_STATE *mem_info = GetDevMemState(binding.mem);
            if (mem_info) {
                mem_info->obj_bindings.insert(typed_handle);
                // Need to set mem binding for this object
                mem_binding->sparse_bindings.insert(binding);
                mem_binding->UpdateBoundMemorySet();
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateDeviceQueueFamily(uint32_t queue_family, const char *cmd_name, const char *parameter_name,
                                           const char *error_code, bool optional = false) {
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
                                       const char *valid_error_code, bool optional = false) {
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
bool CoreChecks::ValidateStatus(CMD_BUFFER_STATE *pNode, CBStatusFlags status_mask, VkFlags msg_flags, const char *fail_msg,
                                const char *msg_code) {
    if (!(pNode->status & status_mask)) {
        return log_msg(report_data, msg_flags, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(pNode->commandBuffer),
                       msg_code, "%s: %s..", report_data->FormatHandle(pNode->commandBuffer).c_str(), fail_msg);
    }
    return false;
}

RENDER_PASS_STATE *ValidationStateTracker::GetRenderPassState(VkRenderPass renderpass) {
    auto it = renderPassMap.find(renderpass);
    if (it == renderPassMap.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::shared_ptr<RENDER_PASS_STATE> ValidationStateTracker::GetRenderPassStateSharedPtr(VkRenderPass renderpass) {
    auto it = renderPassMap.find(renderpass);
    if (it == renderPassMap.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> const GetDescriptorSetLayout(const ValidationStateTracker *state_data,
                                                                                         VkDescriptorSetLayout dsLayout) {
    auto it = state_data->descriptorSetLayoutMap.find(dsLayout);
    if (it == state_data->descriptorSetLayoutMap.end()) {
        return nullptr;
    }
    return it->second;
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
bool CoreChecks::ValidateDrawStateFlags(CMD_BUFFER_STATE *pCB, const PIPELINE_STATE *pPipe, bool indexed, const char *msg_code) {
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
bool CoreChecks::ValidatePipelineDrawtimeState(LAST_BOUND_STATE const &state, const CMD_BUFFER_STATE *pCB, CMD_TYPE cmd_type,
                                               PIPELINE_STATE const *pPipeline, const char *caller) {
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
            const auto attribute_format = attribute_description.format;

            const auto &vertex_binding_map_it = pPipeline->vertex_binding_to_index_map_.find(vertex_binding);
            if ((vertex_binding_map_it != pPipeline->vertex_binding_to_index_map_.cend()) &&
                (vertex_binding < current_vtx_bfr_binding_info.size()) &&
                (current_vtx_bfr_binding_info[vertex_binding].buffer != VK_NULL_HANDLE)) {
                const auto vertex_buffer_stride = pPipeline->vertex_binding_descriptions_[vertex_binding_map_it->second].stride;
                const auto vertex_buffer_offset = current_vtx_bfr_binding_info[vertex_binding].offset;
                const auto buffer_state = GetBufferState(current_vtx_bfr_binding_info[vertex_binding].buffer);

                // Use only memory binding offset as base memory should be properly aligned by the driver
                const auto buffer_binding_address = buffer_state->binding.offset + vertex_buffer_offset;
                // Use 1 as vertex/instance index to use buffer stride as well
                const auto attrib_address = buffer_binding_address + vertex_buffer_stride + attribute_offset;

                uint32_t vtx_attrib_req_alignment = FormatElementSize(attribute_format);
                if (FormatElementIsTexel(attribute_format)) {
                    vtx_attrib_req_alignment /= FormatChannelCount(attribute_format);
                }

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
static bool VerifySetLayoutCompatibility(const cvdescriptorset::DescriptorSet *descriptor_set,
                                         PIPELINE_LAYOUT_STATE const *pipeline_layout, const uint32_t layoutIndex,
                                         string &errorMsg) {
    auto num_sets = pipeline_layout->set_layouts.size();
    if (layoutIndex >= num_sets) {
        stringstream errorStr;
        errorStr << "VkPipelineLayout (" << pipeline_layout->layout << ") only contains " << num_sets
                 << " setLayouts corresponding to sets 0-" << num_sets - 1 << ", but you're attempting to bind set to index "
                 << layoutIndex;
        errorMsg = errorStr.str();
        return false;
    }
    if (descriptor_set->IsPushDescriptor()) return true;
    auto layout_node = pipeline_layout->set_layouts[layoutIndex];
    return cvdescriptorset::VerifySetLayoutCompatibility(layout_node.get(), descriptor_set->GetLayout().get(), &errorMsg);
}

// Validate overall state at the time of a draw call
bool CoreChecks::ValidateCmdBufDrawState(CMD_BUFFER_STATE *cb_node, CMD_TYPE cmd_type, const bool indexed,
                                         const VkPipelineBindPoint bind_point, const char *function, const char *pipe_err_code,
                                         const char *state_err_code) {
    bool result = false;
    auto const &state = cb_node->lastBound[bind_point];
    PIPELINE_STATE *pPipe = state.pipeline_state;
    if (nullptr == pPipe) {
        return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                       HandleToUint64(cb_node->commandBuffer), pipe_err_code,
                       "Must not call %s on this command buffer while there is no %s pipeline bound.", function,
                       bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS ? "Graphics" : "Compute");
    }

    // First check flag states
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) result = ValidateDrawStateFlags(cb_node, pPipe, indexed, state_err_code);

    // Now complete other state checks
    string errorString;
    auto const &pipeline_layout = pPipe->pipeline_layout;

    for (const auto &set_binding_pair : pPipe->active_slots) {
        uint32_t setIndex = set_binding_pair.first;
        // If valid set is not bound throw an error
        if ((state.boundDescriptorSets.size() <= setIndex) || (!state.boundDescriptorSets[setIndex])) {
            result |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_DescriptorSetNotBound,
                        "%s uses set #%u but that set is not bound.", report_data->FormatHandle(pPipe->pipeline).c_str(), setIndex);
        } else if (!VerifySetLayoutCompatibility(state.boundDescriptorSets[setIndex], &pipeline_layout, setIndex, errorString)) {
            // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
            VkDescriptorSet setHandle = state.boundDescriptorSets[setIndex]->GetSet();
            result |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                              HandleToUint64(setHandle), kVUID_Core_DrawState_PipelineLayoutsIncompatible,
                              "%s bound as set #%u is not compatible with overlapping %s due to: %s",
                              report_data->FormatHandle(setHandle).c_str(), setIndex,
                              report_data->FormatHandle(pipeline_layout.layout).c_str(), errorString.c_str());
        } else {  // Valid set is bound and layout compatible, validate that it's updated
            // Pull the set node
            cvdescriptorset::DescriptorSet *descriptor_set = state.boundDescriptorSets[setIndex];
            // Validate the draw-time state for this descriptor set
            std::string err_str;
            if (!descriptor_set->IsPushDescriptor()) {
                // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation checks.
                // Here, the currently bound pipeline determines whether an image validation check is redundant...
                // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline.
                const cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second, cb_node,
                                                                           pPipe);
                const auto &binding_req_map = reduced_map.Map();

                if (!ValidateDrawState(descriptor_set, binding_req_map, state.dynamicOffsets[setIndex], cb_node, function,
                                       &err_str)) {
                    auto set = descriptor_set->GetSet();
                    result |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                      HandleToUint64(set), kVUID_Core_DrawState_DescriptorSetNotUpdated,
                                      "%s bound as set #%u encountered the following validation error at %s time: %s",
                                      report_data->FormatHandle(set).c_str(), setIndex, function, err_str.c_str());
                }
            }
        }
    }

    // Check general pipeline state that needs to be validated at drawtime
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point)
        result |= ValidatePipelineDrawtimeState(state, cb_node, cmd_type, pPipe, function);

    return result;
}

void CoreChecks::UpdateDrawState(CMD_BUFFER_STATE *cb_state, const VkPipelineBindPoint bind_point) {
    auto const &state = cb_state->lastBound[bind_point];
    PIPELINE_STATE *pPipe = state.pipeline_state;
    if (VK_NULL_HANDLE != state.pipeline_layout) {
        for (const auto &set_binding_pair : pPipe->active_slots) {
            uint32_t setIndex = set_binding_pair.first;
            // Pull the set node
            cvdescriptorset::DescriptorSet *descriptor_set = state.boundDescriptorSets[setIndex];
            if (!descriptor_set->IsPushDescriptor()) {
                // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor binding
                const cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second, cb_state);
                const auto &binding_req_map = reduced_map.Map();

                // Bind this set and its active descriptor resources to the command buffer
                descriptor_set->UpdateDrawState(this, cb_state, binding_req_map);
                // For given active slots record updated images & buffers
                descriptor_set->GetStorageUpdates(binding_req_map, &cb_state->updateBuffers, &cb_state->updateImages);
            }
        }
    }
    if (!pPipe->vertex_binding_descriptions_.empty()) {
        cb_state->vertex_buffer_used = true;
    }
}

bool CoreChecks::ValidatePipelineLocked(std::vector<std::unique_ptr<PIPELINE_STATE>> const &pPipelines, int pipelineIndex) const {
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
        if (color_blend_state->attachmentCount != subpass_desc->colorAttachmentCount) {
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

        if (subpass_desc->pDepthStencilAttachment && subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
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
                max_sample_count =
                    std::max(max_sample_count,
                             pPipeline->rp_state->createInfo.pAttachments[subpass_desc->pColorAttachments[i].attachment].samples);
            }
        }
        if (subpass_desc->pDepthStencilAttachment && subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            max_sample_count =
                std::max(max_sample_count,
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

        if (subpass_desc->pDepthStencilAttachment && subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
            const uint32_t subpass_depth_samples =
                static_cast<uint32_t>(pPipeline->rp_state->createInfo.pAttachments[attachment].samples);

            if (pPipeline->graphicsPipelineCI.pDepthStencilState) {
                const bool ds_test_enabled = (pPipeline->graphicsPipelineCI.pDepthStencilState->depthTestEnable == VK_TRUE) ||
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
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415",
                        "vkCreateGraphicsPipelines: pCreateInfo[%d].pMultisampleState->sampleShadingEnable must be VK_FALSE when "
                        "pCreateInfo[%d].pMultisampleState->rasterizationSamples (%u) is greater than the number of samples of the "
                        "subpass color attachment (%u).",
                        pipelineIndex, pipelineIndex, raster_samples, subpass_color_samples);
                }

                const auto *coverage_modulation_state = lvl_find_in_chain<VkPipelineCoverageModulationStateCreateInfoNV>(
                    pPipeline->graphicsPipelineCI.pMultisampleState->pNext);

                if (coverage_modulation_state && (coverage_modulation_state->coverageModulationTableEnable == VK_TRUE)) {
                    if (coverage_modulation_state->coverageModulationTableCount != (raster_samples / subpass_color_samples)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
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
                const auto color_attachment_ref = subpass_desc->pColorAttachments[coverage_to_color_state->coverageToColorLocation];
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
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404",
                            "vkCreateGraphicsPipelines: pCreateInfos[%" PRId32
                            "].pMultisampleState VkPipelineCoverageToColorStateCreateInfoNV "
                            "coverageToColorLocation = %" PRIu32 " %s",
                            pipelineIndex, coverage_to_color_state->coverageToColorLocation, error_detail.c_str());
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
bool CoreChecks::ValidateIdleDescriptorSet(VkDescriptorSet set, const char *func_str) {
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

// Remove set from setMap and delete the set
void ValidationStateTracker::FreeDescriptorSet(cvdescriptorset::DescriptorSet *descriptor_set) {
    setMap.erase(descriptor_set->GetSet());
}

// Free all DS Pools including their Sets & related sub-structs
// NOTE : Calls to this function should be wrapped in mutex
void ValidationStateTracker::DeleteDescriptorSetPools() {
    for (auto ii = descriptorPoolMap.begin(); ii != descriptorPoolMap.end();) {
        // Remove this pools' sets from setMap and delete them
        for (auto ds : ii->second->sets) {
            FreeDescriptorSet(ds);
        }
        ii->second->sets.clear();
        ii = descriptorPoolMap.erase(ii);
    }
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
    auto pool = GetCommandPoolState(cb_node->createInfo.commandPool);
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
                                                   const char *VUID) {
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
                                                const char *VUID) {
    bool skip = false;
    if ((deviceMask & pCB->active_render_pass_device_mask) != deviceMask) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VUID_handle_type, VUID_handle, VUID,
                        "deviceMask(0x%" PRIx32 ") is not a subset of %s device mask(0x%" PRIx32 ").", deviceMask,
                        report_data->FormatHandle(pCB->activeRenderPass->renderPass).c_str(), pCB->active_render_pass_device_mask);
    }
    return skip;
}

// For given object struct return a ptr of BASE_NODE type for its wrapping struct
BASE_NODE *ValidationStateTracker::GetStateStructPtrFromObject(const VulkanTypedHandle &object_struct) {
    BASE_NODE *base_ptr = nullptr;
    switch (object_struct.type) {
        case kVulkanObjectTypeDescriptorSet: {
            base_ptr = GetSetNode(object_struct.Cast<VkDescriptorSet>());
            break;
        }
        case kVulkanObjectTypeSampler: {
            base_ptr = GetSamplerState(object_struct.Cast<VkSampler>());
            break;
        }
        case kVulkanObjectTypeQueryPool: {
            base_ptr = GetQueryPoolState(object_struct.Cast<VkQueryPool>());
            break;
        }
        case kVulkanObjectTypePipeline: {
            base_ptr = GetPipelineState(object_struct.Cast<VkPipeline>());
            break;
        }
        case kVulkanObjectTypeBuffer: {
            base_ptr = GetBufferState(object_struct.Cast<VkBuffer>());
            break;
        }
        case kVulkanObjectTypeBufferView: {
            base_ptr = GetBufferViewState(object_struct.Cast<VkBufferView>());
            break;
        }
        case kVulkanObjectTypeImage: {
            base_ptr = GetImageState(object_struct.Cast<VkImage>());
            break;
        }
        case kVulkanObjectTypeImageView: {
            base_ptr = GetImageViewState(object_struct.Cast<VkImageView>());
            break;
        }
        case kVulkanObjectTypeEvent: {
            base_ptr = GetEventState(object_struct.Cast<VkEvent>());
            break;
        }
        case kVulkanObjectTypeDescriptorPool: {
            base_ptr = GetDescriptorPoolState(object_struct.Cast<VkDescriptorPool>());
            break;
        }
        case kVulkanObjectTypeCommandPool: {
            base_ptr = GetCommandPoolState(object_struct.Cast<VkCommandPool>());
            break;
        }
        case kVulkanObjectTypeFramebuffer: {
            base_ptr = GetFramebufferState(object_struct.Cast<VkFramebuffer>());
            break;
        }
        case kVulkanObjectTypeRenderPass: {
            base_ptr = GetRenderPassState(object_struct.Cast<VkRenderPass>());
            break;
        }
        case kVulkanObjectTypeDeviceMemory: {
            base_ptr = GetDevMemState(object_struct.Cast<VkDeviceMemory>());
            break;
        }
        case kVulkanObjectTypeAccelerationStructureNV: {
            base_ptr = GetAccelerationStructureState(object_struct.Cast<VkAccelerationStructureNV>());
            break;
        }
        default:
            // TODO : Any other objects to be handled here?
            assert(0);
            break;
    }
    return base_ptr;
}

// Tie the VulkanTypedHandle to the cmd buffer which includes:
//  Add object_binding to cmd buffer
//  Add cb_binding to object
static void AddCommandBufferBinding(std::unordered_set<CMD_BUFFER_STATE *> *cb_bindings, const VulkanTypedHandle &obj,
                                    CMD_BUFFER_STATE *cb_node) {
    cb_bindings->insert(cb_node);
    cb_node->object_bindings.insert(obj);
}
// For a given object, if cb_node is in that objects cb_bindings, remove cb_node
void ValidationStateTracker::RemoveCommandBufferBinding(VulkanTypedHandle const &object, CMD_BUFFER_STATE *cb_node) {
    BASE_NODE *base_obj = GetStateStructPtrFromObject(object);
    if (base_obj) base_obj->cb_bindings.erase(cb_node);
}
// Reset the command buffer state
//  Maintain the createInfo and set state to CB_NEW, but clear all other state
void ValidationStateTracker::ResetCommandBufferState(const VkCommandBuffer cb) {
    CMD_BUFFER_STATE *pCB = GetCBState(cb);
    if (pCB) {
        pCB->in_use.store(0);
        // Reset CB state (note that createInfo is not cleared)
        pCB->commandBuffer = cb;
        memset(&pCB->beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        memset(&pCB->inheritanceInfo, 0, sizeof(VkCommandBufferInheritanceInfo));
        pCB->hasDrawCmd = false;
        pCB->hasTraceRaysCmd = false;
        pCB->state = CB_NEW;
        pCB->submitCount = 0;
        pCB->image_layout_change_count = 1;  // Start at 1. 0 is insert value for validation cache versions, s.t. new == dirty
        pCB->status = 0;
        pCB->static_status = 0;
        pCB->viewportMask = 0;
        pCB->scissorMask = 0;

        for (auto &item : pCB->lastBound) {
            item.second.reset();
        }

        memset(&pCB->activeRenderPassBeginInfo, 0, sizeof(pCB->activeRenderPassBeginInfo));
        pCB->activeRenderPass = nullptr;
        pCB->activeSubpassContents = VK_SUBPASS_CONTENTS_INLINE;
        pCB->activeSubpass = 0;
        pCB->broken_bindings.clear();
        pCB->waitedEvents.clear();
        pCB->events.clear();
        pCB->writeEventsBeforeWait.clear();
        pCB->queryToStateMap.clear();
        pCB->activeQueries.clear();
        pCB->startedQueries.clear();
        pCB->image_layout_map.clear();
        pCB->eventToStageMap.clear();
        pCB->cb_vertex_buffer_binding_info.clear();
        pCB->current_vertex_buffer_binding_info.vertex_buffer_bindings.clear();
        pCB->vertex_buffer_used = false;
        pCB->primaryCommandBuffer = VK_NULL_HANDLE;
        // If secondary, invalidate any primary command buffer that may call us.
        if (pCB->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
            InvalidateCommandBuffers(pCB->linkedCommandBuffers, VulkanTypedHandle(cb, kVulkanObjectTypeCommandBuffer));
        }

        // Remove reverse command buffer links.
        for (auto pSubCB : pCB->linkedCommandBuffers) {
            pSubCB->linkedCommandBuffers.erase(pCB);
        }
        pCB->linkedCommandBuffers.clear();
        pCB->updateImages.clear();
        pCB->updateBuffers.clear();
        ClearCmdBufAndMemReferences(pCB);
        pCB->queue_submit_functions.clear();
        pCB->cmd_execute_commands_functions.clear();
        pCB->eventUpdates.clear();
        pCB->queryUpdates.clear();

        // Remove object bindings
        for (const auto &obj : pCB->object_bindings) {
            RemoveCommandBufferBinding(obj, pCB);
        }
        pCB->object_bindings.clear();
        // Remove this cmdBuffer's reference from each FrameBuffer's CB ref list
        for (auto framebuffer : pCB->framebuffers) {
            auto fb_state = GetFramebufferState(framebuffer);
            if (fb_state) fb_state->cb_bindings.erase(pCB);
        }
        pCB->framebuffers.clear();
        pCB->activeFramebuffer = VK_NULL_HANDLE;
        memset(&pCB->index_buffer_binding, 0, sizeof(pCB->index_buffer_binding));

        pCB->qfo_transfer_image_barriers.Reset();
        pCB->qfo_transfer_buffer_barriers.Reset();

        // Clean up the label data
        ResetCmdDebugUtilsLabel(report_data, pCB->commandBuffer);
        pCB->debug_label.Reset();
    }
    if (command_buffer_reset_callback) {
        (*command_buffer_reset_callback)(cb);
    }
}

CBStatusFlags MakeStaticStateMask(VkPipelineDynamicStateCreateInfo const *ds) {
    // initially assume everything is static state
    CBStatusFlags flags = CBSTATUS_ALL_STATE_SET;

    if (ds) {
        for (uint32_t i = 0; i < ds->dynamicStateCount; i++) {
            switch (ds->pDynamicStates[i]) {
                case VK_DYNAMIC_STATE_LINE_WIDTH:
                    flags &= ~CBSTATUS_LINE_WIDTH_SET;
                    break;
                case VK_DYNAMIC_STATE_DEPTH_BIAS:
                    flags &= ~CBSTATUS_DEPTH_BIAS_SET;
                    break;
                case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
                    flags &= ~CBSTATUS_BLEND_CONSTANTS_SET;
                    break;
                case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
                    flags &= ~CBSTATUS_DEPTH_BOUNDS_SET;
                    break;
                case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
                    flags &= ~CBSTATUS_STENCIL_READ_MASK_SET;
                    break;
                case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
                    flags &= ~CBSTATUS_STENCIL_WRITE_MASK_SET;
                    break;
                case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
                    flags &= ~CBSTATUS_STENCIL_REFERENCE_SET;
                    break;
                case VK_DYNAMIC_STATE_SCISSOR:
                    flags &= ~CBSTATUS_SCISSOR_SET;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT:
                    flags &= ~CBSTATUS_VIEWPORT_SET;
                    break;
                case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
                    flags &= ~CBSTATUS_EXCLUSIVE_SCISSOR_SET;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
                    flags &= ~CBSTATUS_SHADING_RATE_PALETTE_SET;
                    break;
                default:
                    break;
            }
        }
    }

    return flags;
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

void CoreChecks::InitGpuValidation() {
    // Process the layer settings file.
    enum CoreValidationGpuFlagBits {
        CORE_VALIDATION_GPU_VALIDATION_ALL_BIT = 0x00000001,
        CORE_VALIDATION_GPU_VALIDATION_RESERVE_BINDING_SLOT_BIT = 0x00000002,
    };
    typedef VkFlags CoreGPUFlags;
    static const std::unordered_map<std::string, VkFlags> gpu_flags_option_definitions = {
        {std::string("all"), CORE_VALIDATION_GPU_VALIDATION_ALL_BIT},
        {std::string("reserve_binding_slot"), CORE_VALIDATION_GPU_VALIDATION_RESERVE_BINDING_SLOT_BIT},
    };
    std::string gpu_flags_key = "lunarg_core_validation.gpu_validation";
    CoreGPUFlags gpu_flags = GetLayerOptionFlags(gpu_flags_key, gpu_flags_option_definitions, 0);
    gpu_flags_key = "khronos_validation.gpu_validation";
    gpu_flags |= GetLayerOptionFlags(gpu_flags_key, gpu_flags_option_definitions, 0);
    if (gpu_flags & CORE_VALIDATION_GPU_VALIDATION_ALL_BIT) {
        instance_state->enabled.gpu_validation = true;
    }
    if (gpu_flags & CORE_VALIDATION_GPU_VALIDATION_RESERVE_BINDING_SLOT_BIT) {
        instance_state->enabled.gpu_validation_reserve_binding_slot = true;
    }
}

void CoreChecks::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance, VkResult result) {
    if (VK_SUCCESS != result) return;
    InitGpuValidation();
}

bool CoreChecks::ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t requested_queue_family,
                                          const char *err_code, const char *cmd_name, const char *queue_family_var_name) {
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
                                                const VkDeviceQueueCreateInfo *infos) {
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
                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    bool skip = false;
    auto pd_state = GetPhysicalDeviceState(gpu);

    // TODO: object_tracker should perhaps do this instead
    //       and it does not seem to currently work anyway -- the loader just crashes before this point
    if (!pd_state) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, 0,
                        kVUID_Core_DevLimit_MustQueryCount,
                        "Invalid call to vkCreateDevice() w/o first calling vkEnumeratePhysicalDevices().");
    }
    skip |= ValidateDeviceQueueCreateInfos(pd_state, pCreateInfo->queueCreateInfoCount, pCreateInfo->pQueueCreateInfos);
    return skip;
}

void CoreChecks::PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                           std::unique_ptr<safe_VkDeviceCreateInfo> &modified_create_info) {
    // GPU Validation can possibly turn on device features, so give it a chance to change the create info.
    if (enabled.gpu_validation) {
        VkPhysicalDeviceFeatures supported_features;
        DispatchGetPhysicalDeviceFeatures(gpu, &supported_features);
        GpuPreCallRecordCreateDevice(gpu, modified_create_info, &supported_features);
    }
}

void CoreChecks::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    StateTracker::PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);

    if (enabled.gpu_validation) {
        // The only CoreCheck specific init is for gpu_validation
        ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
        ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeCoreValidation);
        CoreChecks *core_checks = static_cast<CoreChecks *>(validation_data);
        core_checks->GpuPostCallRecordCreateDevice(&enabled, pCreateInfo);
        core_checks->SetCommandBufferResetCallback(
            [core_checks](VkCommandBuffer command_buffer) -> void { core_checks->GpuResetCommandBuffer(command_buffer); });
    }
}

void ValidationStateTracker::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                        VkResult result) {
    if (VK_SUCCESS != result) return;

    const VkPhysicalDeviceFeatures *enabled_features_found = pCreateInfo->pEnabledFeatures;
    if (nullptr == enabled_features_found) {
        const auto *features2 = lvl_find_in_chain<VkPhysicalDeviceFeatures2KHR>(pCreateInfo->pNext);
        if (features2) {
            enabled_features_found = &(features2->features);
        }
    }

    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeCoreValidation);
    ValidationStateTracker *state_tracker = static_cast<ValidationStateTracker *>(validation_data);

    if (nullptr == enabled_features_found) {
        state_tracker->enabled_features.core = {};
    } else {
        state_tracker->enabled_features.core = *enabled_features_found;
    }

    // Make sure that queue_family_properties are obtained for this device's physical_device, even if the app has not
    // previously set them through an explicit API call.
    uint32_t count;
    auto pd_state = GetPhysicalDeviceState(gpu);
    DispatchGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
    pd_state->queue_family_properties.resize(std::max(static_cast<uint32_t>(pd_state->queue_family_properties.size()), count));
    DispatchGetPhysicalDeviceQueueFamilyProperties(gpu, &count, &pd_state->queue_family_properties[0]);
    // Save local link to this device's physical device state
    state_tracker->physical_device_state = pd_state;

    const auto *device_group_ci = lvl_find_in_chain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
    state_tracker->physical_device_count =
        device_group_ci && device_group_ci->physicalDeviceCount > 0 ? device_group_ci->physicalDeviceCount : 1;

    const auto *descriptor_indexing_features = lvl_find_in_chain<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>(pCreateInfo->pNext);
    if (descriptor_indexing_features) {
        state_tracker->enabled_features.descriptor_indexing = *descriptor_indexing_features;
    }

    const auto *eight_bit_storage_features = lvl_find_in_chain<VkPhysicalDevice8BitStorageFeaturesKHR>(pCreateInfo->pNext);
    if (eight_bit_storage_features) {
        state_tracker->enabled_features.eight_bit_storage = *eight_bit_storage_features;
    }

    const auto *exclusive_scissor_features = lvl_find_in_chain<VkPhysicalDeviceExclusiveScissorFeaturesNV>(pCreateInfo->pNext);
    if (exclusive_scissor_features) {
        state_tracker->enabled_features.exclusive_scissor = *exclusive_scissor_features;
    }

    const auto *shading_rate_image_features = lvl_find_in_chain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);
    if (shading_rate_image_features) {
        state_tracker->enabled_features.shading_rate_image = *shading_rate_image_features;
    }

    const auto *mesh_shader_features = lvl_find_in_chain<VkPhysicalDeviceMeshShaderFeaturesNV>(pCreateInfo->pNext);
    if (mesh_shader_features) {
        state_tracker->enabled_features.mesh_shader = *mesh_shader_features;
    }

    const auto *inline_uniform_block_features =
        lvl_find_in_chain<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(pCreateInfo->pNext);
    if (inline_uniform_block_features) {
        state_tracker->enabled_features.inline_uniform_block = *inline_uniform_block_features;
    }

    const auto *transform_feedback_features = lvl_find_in_chain<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(pCreateInfo->pNext);
    if (transform_feedback_features) {
        state_tracker->enabled_features.transform_feedback_features = *transform_feedback_features;
    }

    const auto *float16_int8_features = lvl_find_in_chain<VkPhysicalDeviceFloat16Int8FeaturesKHR>(pCreateInfo->pNext);
    if (float16_int8_features) {
        state_tracker->enabled_features.float16_int8 = *float16_int8_features;
    }

    const auto *vtx_attrib_div_features = lvl_find_in_chain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
    if (vtx_attrib_div_features) {
        state_tracker->enabled_features.vtx_attrib_divisor_features = *vtx_attrib_div_features;
    }

    const auto *uniform_buffer_standard_layout_features =
        lvl_find_in_chain<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR>(pCreateInfo->pNext);
    if (uniform_buffer_standard_layout_features) {
        state_tracker->enabled_features.uniform_buffer_standard_layout = *uniform_buffer_standard_layout_features;
    }

    const auto *scalar_block_layout_features = lvl_find_in_chain<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT>(pCreateInfo->pNext);
    if (scalar_block_layout_features) {
        state_tracker->enabled_features.scalar_block_layout_features = *scalar_block_layout_features;
    }

    const auto *buffer_address = lvl_find_in_chain<VkPhysicalDeviceBufferAddressFeaturesEXT>(pCreateInfo->pNext);
    if (buffer_address) {
        state_tracker->enabled_features.buffer_address = *buffer_address;
    }

    const auto *cooperative_matrix_features = lvl_find_in_chain<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(pCreateInfo->pNext);
    if (cooperative_matrix_features) {
        state_tracker->enabled_features.cooperative_matrix_features = *cooperative_matrix_features;
    }

    const auto *float_controls_features = lvl_find_in_chain<VkPhysicalDeviceFloatControlsPropertiesKHR>(pCreateInfo->pNext);
    if (float_controls_features) {
        state_tracker->enabled_features.float_controls = *float_controls_features;
    }

    const auto *host_query_reset_features = lvl_find_in_chain<VkPhysicalDeviceHostQueryResetFeaturesEXT>(pCreateInfo->pNext);
    if (host_query_reset_features) {
        state_tracker->enabled_features.host_query_reset_features = *host_query_reset_features;
    }

    const auto *compute_shader_derivatives_features =
        lvl_find_in_chain<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV>(pCreateInfo->pNext);
    if (compute_shader_derivatives_features) {
        state_tracker->enabled_features.compute_shader_derivatives_features = *compute_shader_derivatives_features;
    }

    const auto *fragment_shader_barycentric_features =
        lvl_find_in_chain<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV>(pCreateInfo->pNext);
    if (fragment_shader_barycentric_features) {
        state_tracker->enabled_features.fragment_shader_barycentric_features = *fragment_shader_barycentric_features;
    }

    const auto *shader_image_footprint_features =
        lvl_find_in_chain<VkPhysicalDeviceShaderImageFootprintFeaturesNV>(pCreateInfo->pNext);
    if (shader_image_footprint_features) {
        state_tracker->enabled_features.shader_image_footprint_features = *shader_image_footprint_features;
    }

    const auto *fragment_shader_interlock_features =
        lvl_find_in_chain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(pCreateInfo->pNext);
    if (fragment_shader_interlock_features) {
        state_tracker->enabled_features.fragment_shader_interlock_features = *fragment_shader_interlock_features;
    }

    const auto *demote_to_helper_invocation_features =
        lvl_find_in_chain<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>(pCreateInfo->pNext);
    if (demote_to_helper_invocation_features) {
        state_tracker->enabled_features.demote_to_helper_invocation_features = *demote_to_helper_invocation_features;
    }

    const auto *texel_buffer_alignment_features =
        lvl_find_in_chain<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>(pCreateInfo->pNext);
    if (texel_buffer_alignment_features) {
        state_tracker->enabled_features.texel_buffer_alignment_features = *texel_buffer_alignment_features;
    }

    const auto *imageless_framebuffer_features =
        lvl_find_in_chain<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>(pCreateInfo->pNext);
    if (imageless_framebuffer_features) {
        state_tracker->enabled_features.imageless_framebuffer_features = *imageless_framebuffer_features;
    }

    // Store physical device properties and physical device mem limits into CoreChecks structs
    DispatchGetPhysicalDeviceMemoryProperties(gpu, &state_tracker->phys_dev_mem_props);
    DispatchGetPhysicalDeviceProperties(gpu, &state_tracker->phys_dev_props);

    const auto &dev_ext = state_tracker->device_extensions;
    auto *phys_dev_props = &state_tracker->phys_dev_ext_props;

    if (dev_ext.vk_khr_push_descriptor) {
        // Get the needed push_descriptor limits
        VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_prop;
        GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_push_descriptor, &push_descriptor_prop);
        phys_dev_props->max_push_descriptors = push_descriptor_prop.maxPushDescriptors;
    }

    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_descriptor_indexing, &phys_dev_props->descriptor_indexing_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_shading_rate_image, &phys_dev_props->shading_rate_image_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_mesh_shader, &phys_dev_props->mesh_shader_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_inline_uniform_block, &phys_dev_props->inline_uniform_block_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_vertex_attribute_divisor, &phys_dev_props->vtx_attrib_divisor_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_depth_stencil_resolve, &phys_dev_props->depth_stencil_resolve_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_transform_feedback, &phys_dev_props->transform_feedback_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_ray_tracing, &phys_dev_props->ray_tracing_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_texel_buffer_alignment, &phys_dev_props->texel_buffer_alignment_props);
    if (state_tracker->device_extensions.vk_nv_cooperative_matrix) {
        // Get the needed cooperative_matrix properties
        auto cooperative_matrix_props = lvl_init_struct<VkPhysicalDeviceCooperativeMatrixPropertiesNV>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&cooperative_matrix_props);
        instance_dispatch_table.GetPhysicalDeviceProperties2KHR(gpu, &prop2);
        state_tracker->phys_dev_ext_props.cooperative_matrix_props = cooperative_matrix_props;

        uint32_t numCooperativeMatrixProperties = 0;
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &numCooperativeMatrixProperties, NULL);
        state_tracker->cooperative_matrix_properties.resize(numCooperativeMatrixProperties,
                                                            lvl_init_struct<VkCooperativeMatrixPropertiesNV>());

        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &numCooperativeMatrixProperties,
                                                                               state_tracker->cooperative_matrix_properties.data());
    }
    if (state_tracker->api_version >= VK_API_VERSION_1_1) {
        // Get the needed subgroup limits
        auto subgroup_prop = lvl_init_struct<VkPhysicalDeviceSubgroupProperties>();
        auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&subgroup_prop);
        instance_dispatch_table.GetPhysicalDeviceProperties2(gpu, &prop2);

        state_tracker->phys_dev_ext_props.subgroup_props = subgroup_prop;
    }

    // Store queue family data
    if ((pCreateInfo != nullptr) && (pCreateInfo->pQueueCreateInfos != nullptr)) {
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            state_tracker->queue_family_index_map.insert(
                std::make_pair(pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, pCreateInfo->pQueueCreateInfos[i].queueCount));
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    pipelineMap.clear();
    renderPassMap.clear();
    commandBufferMap.clear();
    // This will also delete all sets in the pool & remove them from setMap
    DeleteDescriptorSetPools();
    // All sets should be removed
    assert(setMap.empty());
    descriptorSetLayoutMap.clear();
    imageViewMap.clear();
    imageMap.clear();
    imageLayoutMap.clear();
    bufferViewMap.clear();
    bufferMap.clear();
    // Queues persist until device is destroyed
    queueMap.clear();
    layer_debug_utils_destroy_device(device);
}
void CoreChecks::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;
    if (enabled.gpu_validation) {
        GpuPreCallRecordDestroyDevice();
    }
    imageSubresourceMap.clear();

    StateTracker::PreCallRecordDestroyDevice(device, pAllocator);
}

// For given stage mask, if Geometry shader stage is on w/o GS being enabled, report geo_error_id
//   and if Tessellation Control or Evaluation shader stages are on w/o TS being enabled, report tess_error_id.
// Similarly for mesh and task shaders.
bool CoreChecks::ValidateStageMaskGsTsEnables(VkPipelineStageFlags stageMask, const char *caller, const char *geo_error_id,
                                              const char *tess_error_id, const char *mesh_error_id, const char *task_error_id) {
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

// Loop through bound objects and increment their in_use counts.
void CoreChecks::IncrementBoundObjects(CMD_BUFFER_STATE const *cb_node) {
    for (auto obj : cb_node->object_bindings) {
        auto base_obj = GetStateStructPtrFromObject(obj);
        if (base_obj) {
            base_obj->in_use.fetch_add(1);
        }
    }
}
// Track which resources are in-flight by atomically incrementing their "in_use" count
void CoreChecks::IncrementResources(CMD_BUFFER_STATE *cb_node) {
    cb_node->submitCount++;
    cb_node->in_use.fetch_add(1);

    // First Increment for all "generic" objects bound to cmd buffer, followed by special-case objects below
    IncrementBoundObjects(cb_node);
    // TODO : We should be able to remove the NULL look-up checks from the code below as long as
    //  all the corresponding cases are verified to cause CB_INVALID state and the CB_INVALID state
    //  should then be flagged prior to calling this function
    for (auto event : cb_node->writeEventsBeforeWait) {
        auto event_state = GetEventState(event);
        if (event_state) event_state->write_in_use++;
    }
}

// Note: This function assumes that the global lock is held by the calling thread.
// For the given queue, verify the queue state up to the given seq number.
// Currently the only check is to make sure that if there are events to be waited on prior to
//  a QueryReset, make sure that all such events have been signalled.
bool CoreChecks::VerifyQueueStateToSeq(QUEUE_STATE *initial_queue, uint64_t initial_seq) {
    bool skip = false;

    // sequence number we want to validate up to, per queue
    std::unordered_map<QUEUE_STATE *, uint64_t> target_seqs{{initial_queue, initial_seq}};
    // sequence number we've completed validation for, per queue
    std::unordered_map<QUEUE_STATE *, uint64_t> done_seqs;
    std::vector<QUEUE_STATE *> worklist{initial_queue};

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
bool CoreChecks::VerifyQueueStateToFence(VkFence fence) {
    auto fence_state = GetFenceState(fence);
    if (fence_state && fence_state->scope == kSyncScopeInternal && VK_NULL_HANDLE != fence_state->signaler.first) {
        return VerifyQueueStateToSeq(GetQueueState(fence_state->signaler.first), fence_state->signaler.second);
    }
    return false;
}

// Decrement in-use count for objects bound to command buffer
void CoreChecks::DecrementBoundResources(CMD_BUFFER_STATE const *cb_node) {
    BASE_NODE *base_obj = nullptr;
    for (auto obj : cb_node->object_bindings) {
        base_obj = GetStateStructPtrFromObject(obj);
        if (base_obj) {
            base_obj->in_use.fetch_sub(1);
        }
    }
}

void CoreChecks::RetireWorkOnQueue(QUEUE_STATE *pQueue, uint64_t seq) {
    std::unordered_map<VkQueue, uint64_t> otherQueueSeqs;

    // Roll this queue forward, one submission at a time.
    while (pQueue->seq < seq) {
        auto &submission = pQueue->submissions.front();

        for (auto &wait : submission.waitSemaphores) {
            auto pSemaphore = GetSemaphoreState(wait.semaphore);
            if (pSemaphore) {
                pSemaphore->in_use.fetch_sub(1);
            }
            auto &lastSeq = otherQueueSeqs[wait.queue];
            lastSeq = std::max(lastSeq, wait.seq);
        }

        for (auto &semaphore : submission.signalSemaphores) {
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                pSemaphore->in_use.fetch_sub(1);
            }
        }

        for (auto &semaphore : submission.externalSemaphores) {
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                pSemaphore->in_use.fetch_sub(1);
            }
        }

        for (auto cb : submission.cbs) {
            auto cb_node = GetCBState(cb);
            if (!cb_node) {
                continue;
            }
            // First perform decrement on general case bound objects
            DecrementBoundResources(cb_node);
            for (auto event : cb_node->writeEventsBeforeWait) {
                auto eventNode = eventMap.find(event);
                if (eventNode != eventMap.end()) {
                    eventNode->second.write_in_use--;
                }
            }
            for (auto queryStatePair : cb_node->queryToStateMap) {
                queryToStateMap[queryStatePair.first] = queryStatePair.second;
            }
            for (auto eventStagePair : cb_node->eventToStageMap) {
                eventMap[eventStagePair.first].stageMask = eventStagePair.second;
            }

            cb_node->in_use.fetch_sub(1);
        }

        auto pFence = GetFenceState(submission.fence);
        if (pFence && pFence->scope == kSyncScopeInternal) {
            pFence->state = FENCE_RETIRED;
        }

        pQueue->submissions.pop_front();
        pQueue->seq++;
    }

    // Roll other queues forward to the highest seq we saw a wait for
    for (auto qs : otherQueueSeqs) {
        RetireWorkOnQueue(GetQueueState(qs.first), qs.second);
    }
}

// Submit a fence to a queue, delimiting previous fences and previous untracked
// work by it.
static void SubmitFence(QUEUE_STATE *pQueue, FENCE_STATE *pFence, uint64_t submitCount) {
    pFence->state = FENCE_INFLIGHT;
    pFence->signaler.first = pQueue->queue;
    pFence->signaler.second = pQueue->seq + pQueue->submissions.size() + submitCount;
}

bool CoreChecks::ValidateCommandBufferSimultaneousUse(CMD_BUFFER_STATE *pCB, int current_submit_count) {
    bool skip = false;
    if ((pCB->in_use.load() || current_submit_count > 1) &&
        !(pCB->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                        "VUID-vkQueueSubmit-pCommandBuffers-00071", "%s is already in use and is not marked for simultaneous use.",
                        report_data->FormatHandle(pCB->commandBuffer).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateCommandBufferState(CMD_BUFFER_STATE *cb_state, const char *call_source, int current_submit_count,
                                            const char *vu_id) {
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
bool CoreChecks::ValidImageBufferQueue(CMD_BUFFER_STATE *cb_node, const VulkanTypedHandle &object, VkQueue queue, uint32_t count,
                                       const uint32_t *indices) {
    bool found = false;
    bool skip = false;
    auto queue_state = GetQueueState(queue);
    if (queue_state) {
        for (uint32_t i = 0; i < count; i++) {
            if (indices[i] == queue_state->queueFamilyIndex) {
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
                           queue_state->queueFamilyIndex);
        }
    }
    return skip;
}

// Validate that queueFamilyIndices of primary command buffers match this queue
// Secondary command buffers were previously validated in vkCmdExecuteCommands().
bool CoreChecks::ValidateQueueFamilyIndices(CMD_BUFFER_STATE *pCB, VkQueue queue) {
    bool skip = false;
    auto pPool = GetCommandPoolState(pCB->createInfo.commandPool);
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
                auto image_state = GetImageState(object.Cast<VkImage>());
                if (image_state && image_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(pCB, object, queue, image_state->createInfo.queueFamilyIndexCount,
                                                  image_state->createInfo.pQueueFamilyIndices);
                }
            } else if (object.type == kVulkanObjectTypeBuffer) {
                auto buffer_state = GetBufferState(object.Cast<VkBuffer>());
                if (buffer_state && buffer_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(pCB, object, queue, buffer_state->createInfo.queueFamilyIndexCount,
                                                  buffer_state->createInfo.pQueueFamilyIndices);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBufferState(CMD_BUFFER_STATE *pCB, int current_submit_count,
                                                   QFOTransferCBScoreboards<VkImageMemoryBarrier> *qfo_image_scoreboards,
                                                   QFOTransferCBScoreboards<VkBufferMemoryBarrier> *qfo_buffer_scoreboards) {
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

bool CoreChecks::ValidateFenceForSubmit(FENCE_STATE *pFence) {
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
    uint64_t early_retire_seq = 0;
    auto pQueue = GetQueueState(queue);
    auto pFence = GetFenceState(fence);

    if (pFence) {
        if (pFence->scope == kSyncScopeInternal) {
            // Mark fence in use
            SubmitFence(pQueue, pFence, std::max(1u, submitCount));
            if (!submitCount) {
                // If no submissions, but just dropping a fence on the end of the queue,
                // record an empty submission with just the fence, so we can determine
                // its completion.
                pQueue->submissions.emplace_back(std::vector<VkCommandBuffer>(), std::vector<SEMAPHORE_WAIT>(),
                                                 std::vector<VkSemaphore>(), std::vector<VkSemaphore>(), fence);
            }
        } else {
            // Retire work up until this fence early, we will not see the wait that corresponds to this signal
            early_retire_seq = pQueue->seq + pQueue->submissions.size();
            if (!external_sync_warning) {
                external_sync_warning = true;
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, HandleToUint64(fence),
                        kVUID_Core_DrawState_QueueForwardProgress,
                        "vkQueueSubmit(): Signaling external %s on %s will disable validation of preceding command "
                        "buffer lifecycle states and the in-use status of associated objects.",
                        report_data->FormatHandle(fence).c_str(), report_data->FormatHandle(queue).c_str());
            }
        }
    }

    // Now process each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        std::vector<VkCommandBuffer> cbs;
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        vector<SEMAPHORE_WAIT> semaphore_waits;
        vector<VkSemaphore> semaphore_signals;
        vector<VkSemaphore> semaphore_externals;
        for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
            VkSemaphore semaphore = submit->pWaitSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                if (pSemaphore->scope == kSyncScopeInternal) {
                    if (pSemaphore->signaler.first != VK_NULL_HANDLE) {
                        semaphore_waits.push_back({semaphore, pSemaphore->signaler.first, pSemaphore->signaler.second});
                        pSemaphore->in_use.fetch_add(1);
                    }
                    pSemaphore->signaler.first = VK_NULL_HANDLE;
                    pSemaphore->signaled = false;
                } else {
                    semaphore_externals.push_back(semaphore);
                    pSemaphore->in_use.fetch_add(1);
                    if (pSemaphore->scope == kSyncScopeExternalTemporary) {
                        pSemaphore->scope = kSyncScopeInternal;
                    }
                }
            }
        }
        for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
            VkSemaphore semaphore = submit->pSignalSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                if (pSemaphore->scope == kSyncScopeInternal) {
                    pSemaphore->signaler.first = queue;
                    pSemaphore->signaler.second = pQueue->seq + pQueue->submissions.size() + 1;
                    pSemaphore->signaled = true;
                    pSemaphore->in_use.fetch_add(1);
                    semaphore_signals.push_back(semaphore);
                } else {
                    // Retire work up until this submit early, we will not see the wait that corresponds to this signal
                    early_retire_seq = std::max(early_retire_seq, pQueue->seq + pQueue->submissions.size() + 1);
                    if (!external_sync_warning) {
                        external_sync_warning = true;
                        log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                "vkQueueSubmit(): Signaling external %s on %s will disable validation of preceding "
                                "command buffer lifecycle states and the in-use status of associated objects.",
                                report_data->FormatHandle(semaphore).c_str(), report_data->FormatHandle(queue).c_str());
                    }
                }
            }
        }
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (cb_node) {
                cbs.push_back(submit->pCommandBuffers[i]);
                for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                    cbs.push_back(secondaryCmdBuffer->commandBuffer);
                    UpdateCmdBufImageLayouts(secondaryCmdBuffer);
                    IncrementResources(secondaryCmdBuffer);
                    RecordQueuedQFOTransfers(secondaryCmdBuffer);
                }
                UpdateCmdBufImageLayouts(cb_node);
                IncrementResources(cb_node);
                RecordQueuedQFOTransfers(cb_node);
            }
        }
        pQueue->submissions.emplace_back(cbs, semaphore_waits, semaphore_signals, semaphore_externals,
                                         submit_idx == submitCount - 1 ? fence : (VkFence)VK_NULL_HANDLE);
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(pQueue, early_retire_seq);
    }

    if (enabled.gpu_validation) {
        GpuPostCallQueueSubmit(queue, submitCount, pSubmits, fence);
    }
}

bool CoreChecks::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    auto pFence = GetFenceState(fence);
    bool skip = ValidateFenceForSubmit(pFence);
    if (skip) {
        return true;
    }

    unordered_set<VkSemaphore> signaled_semaphores;
    unordered_set<VkSemaphore> unsignaled_semaphores;
    unordered_set<VkSemaphore> internal_semaphores;
    vector<VkCommandBuffer> current_cmds;
    unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_STATE> localImageLayoutMap;
    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
            skip |= ValidateStageMaskGsTsEnables(
                submit->pWaitDstStageMask[i], "vkQueueSubmit()", "VUID-VkSubmitInfo-pWaitDstStageMask-00076",
                "VUID-VkSubmitInfo-pWaitDstStageMask-00077", "VUID-VkSubmitInfo-pWaitDstStageMask-02089",
                "VUID-VkSubmitInfo-pWaitDstStageMask-02090");
            VkSemaphore semaphore = submit->pWaitSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
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
        for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
            VkSemaphore semaphore = submit->pSignalSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
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
        QFOTransferCBScoreboards<VkImageMemoryBarrier> qfo_image_scoreboards;
        QFOTransferCBScoreboards<VkBufferMemoryBarrier> qfo_buffer_scoreboards;

        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (cb_node) {
                skip |= ValidateCmdBufImageLayouts(cb_node, imageLayoutMap, localImageLayoutMap);
                current_cmds.push_back(submit->pCommandBuffers[i]);
                skip |= ValidatePrimaryCommandBufferState(
                    cb_node, (int)std::count(current_cmds.begin(), current_cmds.end(), submit->pCommandBuffers[i]),
                    &qfo_image_scoreboards, &qfo_buffer_scoreboards);
                skip |= ValidateQueueFamilyIndices(cb_node, queue);

                // Potential early exit here as bad object state may crash in delayed function calls
                if (skip) {
                    return true;
                }

                // Call submit-time functions to validate/update state
                for (auto &function : cb_node->queue_submit_functions) {
                    skip |= function();
                }
                for (auto &function : cb_node->eventUpdates) {
                    skip |= function(queue);
                }
                for (auto &function : cb_node->queryUpdates) {
                    skip |= function(queue);
                }
            }
        }
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
void CoreChecks::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    if (enabled.gpu_validation && device_extensions.vk_ext_descriptor_indexing) {
        GpuPreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    }
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
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                          VkAndroidHardwareBufferPropertiesANDROID *pProperties) {
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
                                                                      struct AHardwareBuffer **pBuffer) {
    bool skip = false;
    DEVICE_MEMORY_STATE *mem_info = GetDevMemState(pInfo->memory);

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
        auto image_state = GetImageState(mem_info->dedicated_image);
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

        VkResult fmt_lookup_result = GetPDImageFormatProperties2(&pdifi2, &ifp2);

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

bool CoreChecks::ValidateCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info) {
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

void CoreChecks::RecordCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                           VkSamplerYcbcrConversion ycbcr_conversion) {
    const VkExternalFormatANDROID *ext_format_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_format_android && (0 != ext_format_android->externalFormat)) {
        ycbcr_conversion_ahb_fmt_map.emplace(ycbcr_conversion, ext_format_android->externalFormat);
    }
};

void CoreChecks::RecordDestroySamplerYcbcrConversionANDROID(VkSamplerYcbcrConversion ycbcr_conversion) {
    ycbcr_conversion_ahb_fmt_map.erase(ycbcr_conversion);
};

#else  // !VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo *alloc_info) const { return false; }

static bool ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const debug_report_data *report_data,
                                                                   const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                   const VkImageFormatProperties2 *pImageFormatProperties) {
    return false;
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info) {
    return false;
}

bool CoreChecks::ValidateGetImageMemoryRequirements2ANDROID(const VkImage image) const { return false; }

void CoreChecks::RecordCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                           VkSamplerYcbcrConversion ycbcr_conversion){};

void CoreChecks::RecordDestroySamplerYcbcrConversionANDROID(VkSamplerYcbcrConversion ycbcr_conversion){};

#endif  // VK_USE_PLATFORM_ANDROID_KHR

bool CoreChecks::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
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

void ValidationStateTracker::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                          VkResult result) {
    if (VK_SUCCESS == result) {
        AddMemObjInfo(device, *pMemory, pAllocateInfo);
    }
    return;
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

bool CoreChecks::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
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

void ValidationStateTracker::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
    if (!mem) return;
    DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    const VulkanTypedHandle obj_struct(mem, kVulkanObjectTypeDeviceMemory);

    // Clear mem binding for any bound objects
    for (const auto &obj : mem_info->obj_bindings) {
        BINDABLE *bindable_state = nullptr;
        switch (obj.type) {
            case kVulkanObjectTypeImage:
                bindable_state = GetImageState(obj.Cast<VkImage>());
                break;
            case kVulkanObjectTypeBuffer:
                bindable_state = GetBufferState(obj.Cast<VkBuffer>());
                break;
            case kVulkanObjectTypeAccelerationStructureNV:
                bindable_state = GetAccelerationStructureState(obj.Cast<VkAccelerationStructureNV>());
                break;

            default:
                // Should only have acceleration structure, buffer, or image objects bound to memory
                assert(0);
        }

        assert(bindable_state);
        bindable_state->binding.mem = MEMORY_UNBOUND;
        bindable_state->UpdateBoundMemorySet();
    }
    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(mem_info->cb_bindings, obj_struct);
    memObjMap.erase(mem);
}

// Validate that given Map memory range is valid. This means that the memory should not already be mapped,
//  and that the size of the map range should be:
//  1. Not zero
//  2. Within the size of the memory allocation
bool CoreChecks::ValidateMapMemRange(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size) {
    bool skip = false;

    if (size == 0) {
        skip =
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, HandleToUint64(mem),
                    kVUID_Core_MemTrack_InvalidMap, "VkMapMemory: Attempting to map memory range of size zero");
    }

    auto mem_element = memObjMap.find(mem);
    if (mem_element != memObjMap.end()) {
        auto mem_info = mem_element->second.get();
        // It is an application error to call VkMapMemory on an object that is already mapped
        if (mem_info->mem_range.size != 0) {
            skip =
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
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
                               "Mapping Memory from 0x%" PRIx64 " to 0x%" PRIx64 " oversteps total array size 0x%" PRIx64 ".",
                               offset, size + offset, mem_info->alloc_info.allocationSize);
            }
        }
    }
    return skip;
}

void CoreChecks::StoreMemRanges(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->mem_range.offset = offset;
        mem_info->mem_range.size = size;
    }
}

// Guard value for pad data
static char NoncoherentMemoryFillValue = 0xb;

void CoreChecks::InitializeAndTrackMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->p_driver_data = *ppData;
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

void CoreChecks::RetireFence(VkFence fence) {
    auto pFence = GetFenceState(fence);
    if (pFence && pFence->scope == kSyncScopeInternal) {
        if (pFence->signaler.first != VK_NULL_HANDLE) {
            // Fence signaller is a queue -- use this as proof that prior operations on that queue have completed.
            RetireWorkOnQueue(GetQueueState(pFence->signaler.first), pFence->signaler.second);
        } else {
            // Fence signaller is the WSI. We're not tracking what the WSI op actually /was/ in CV yet, but we need to mark
            // the fence as retired.
            pFence->state = FENCE_RETIRED;
        }
    }
}

bool CoreChecks::PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                              uint64_t timeout) {
    // Verify fence status of submitted fences
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; i++) {
        skip |= VerifyQueueStateToFence(pFences[i]);
    }
    return skip;
}

void CoreChecks::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                             uint64_t timeout, VkResult result) {
    if (VK_SUCCESS != result) return;

    // When we know that all fences are complete we can clean/remove their CBs
    if ((VK_TRUE == waitAll) || (1 == fenceCount)) {
        for (uint32_t i = 0; i < fenceCount; i++) {
            RetireFence(pFences[i]);
        }
    }
    // NOTE : Alternate case not handled here is when some fences have completed. In
    //  this case for app to guarantee which fences completed it will have to call
    //  vkGetFenceStatus() at which point we'll clean/remove their CBs if complete.
}

void CoreChecks::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) {
    if (VK_SUCCESS != result) return;
    RetireFence(fence);
}

void CoreChecks::RecordGetDeviceQueueState(uint32_t queue_family_index, VkQueue queue) {
    // Add queue to tracking set only if it is new
    auto queue_is_new = queues.emplace(queue);
    if (queue_is_new.second == true) {
        QUEUE_STATE *queue_state = &queueMap[queue];
        queue_state->queue = queue;
        queue_state->queueFamilyIndex = queue_family_index;
        queue_state->seq = 0;
    }
}

bool CoreChecks::ValidateGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue, const char *valid_qfi_vuid,
                                        const char *qfi_in_range_vuid) {
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

bool CoreChecks::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    return ValidateGetDeviceQueue(queueFamilyIndex, queueIndex, pQueue, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384",
                                  "VUID-vkGetDeviceQueue-queueIndex-00385");
}

void CoreChecks::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    RecordGetDeviceQueueState(queueFamilyIndex, *pQueue);
}

void CoreChecks::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, *pQueue);
}

bool CoreChecks::PreCallValidateQueueWaitIdle(VkQueue queue) {
    QUEUE_STATE *queue_state = GetQueueState(queue);
    return VerifyQueueStateToSeq(queue_state, queue_state->seq + queue_state->submissions.size());
}

void CoreChecks::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    if (VK_SUCCESS != result) return;
    QUEUE_STATE *queue_state = GetQueueState(queue);
    RetireWorkOnQueue(queue_state, queue_state->seq + queue_state->submissions.size());
}

bool CoreChecks::PreCallValidateDeviceWaitIdle(VkDevice device) {
    bool skip = false;
    for (auto &queue : queueMap) {
        skip |= VerifyQueueStateToSeq(&queue.second, queue.second.seq + queue.second.submissions.size());
    }
    return skip;
}

void CoreChecks::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (auto &queue : queueMap) {
        RetireWorkOnQueue(&queue.second, queue.second.seq + queue.second.submissions.size());
    }
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    FENCE_STATE *fence_node = GetFenceState(fence);
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

void CoreChecks::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    if (!fence) return;
    fenceMap.erase(fence);
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator) {
    SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    const VulkanTypedHandle obj_struct(semaphore, kVulkanObjectTypeSemaphore);
    bool skip = false;
    if (sema_node) {
        skip |= ValidateObjectNotInUse(sema_node, obj_struct, "vkDestroySemaphore", "VUID-vkDestroySemaphore-semaphore-01137");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator) {
    if (!semaphore) return;
    semaphoreMap.erase(semaphore);
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    EVENT_STATE *event_state = GetEventState(event);
    const VulkanTypedHandle obj_struct(event, kVulkanObjectTypeEvent);
    bool skip = false;
    if (event_state) {
        skip |= ValidateObjectNotInUse(event_state, obj_struct, "vkDestroyEvent", "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    if (!event) return;
    EVENT_STATE *event_state = GetEventState(event);
    const VulkanTypedHandle obj_struct(event, kVulkanObjectTypeEvent);
    InvalidateCommandBuffers(event_state->cb_bindings, obj_struct);
    eventMap.erase(event);
}

bool CoreChecks::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator) {
    if (disabled.query_validation) return false;
    QUERY_POOL_STATE *qp_state = GetQueryPoolState(queryPool);
    const VulkanTypedHandle obj_struct(queryPool, kVulkanObjectTypeQueryPool);
    bool skip = false;
    if (qp_state) {
        skip |= ValidateObjectNotInUse(qp_state, obj_struct, "vkDestroyQueryPool", "VUID-vkDestroyQueryPool-queryPool-00793");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator) {
    if (!queryPool) return;
    QUERY_POOL_STATE *qp_state = GetQueryPoolState(queryPool);
    const VulkanTypedHandle obj_struct(queryPool, kVulkanObjectTypeQueryPool);
    InvalidateCommandBuffers(qp_state->cb_bindings, obj_struct);
    queryPoolMap.erase(queryPool);
}

bool CoreChecks::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags) {
    if (disabled.query_validation) return false;
    bool skip = false;
    skip |= ValidateQueryPoolStride("VUID-vkGetQueryPoolResults-flags-00814", "VUID-vkGetQueryPoolResults-flags-00815", stride,
                                    "dataSize", dataSize, flags);
    auto query_pool_state = queryPoolMap.find(queryPool);
    if (query_pool_state != queryPoolMap.end()) {
        if ((query_pool_state->second->createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, HandleToUint64(queryPool),
                "VUID-vkGetQueryPoolResults-queryType-00818",
                "%s was created with a queryType of VK_QUERY_TYPE_TIMESTAMP but flags contains VK_QUERY_RESULT_PARTIAL_BIT.",
                report_data->FormatHandle(queryPool).c_str());
        }
    }

    QueryObject query_obj{queryPool, 0u};
    QueryResultType result_type;

    for (uint32_t i = 0; i < queryCount; ++i) {
        query_obj.query = firstQuery + i;
        auto query_data = queryToStateMap.find(query_obj);

        if (query_data != queryToStateMap.end()) {
            result_type = GetQueryResultType(query_data->second, flags);
        } else {
            result_type = QUERYRESULT_UNKNOWN;
        }

        if (result_type != QUERYRESULT_SOME_DATA) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT,
                            HandleToUint64(queryPool), kVUID_Core_DrawState_InvalidQuery,
                            "vkGetQueryPoolResults() on %s and query %" PRIu32 ": %s", report_data->FormatHandle(queryPool).c_str(),
                            query_obj.query, string_QueryResultType(result_type));
        }
    }

    return skip;
}

// Return true if given ranges intersect, else false
// Prereq : For both ranges, range->end - range->start > 0. This case should have already resulted
//  in an error so not checking that here
// pad_ranges bool indicates a linear and non-linear comparison which requires padding
// In the case where padding is required, if an alias is encountered then a validation error is reported and skip
//  may be set by the callback function so caller should merge in skip value if padding case is possible.
// This check can be skipped by passing skip_checks=true, for call sites outside the validation path.
bool CoreChecks::RangesIntersect(MEMORY_RANGE const *range1, MEMORY_RANGE const *range2, bool *skip, bool skip_checks) {
    *skip = false;
    auto r1_start = range1->start;
    auto r1_end = range1->end;
    auto r2_start = range2->start;
    auto r2_end = range2->end;
    VkDeviceSize pad_align = 1;
    if (range1->linear != range2->linear) {
        pad_align = phys_dev_props.limits.bufferImageGranularity;
    }
    if ((r1_end & ~(pad_align - 1)) < (r2_start & ~(pad_align - 1))) return false;
    if ((r1_start & ~(pad_align - 1)) > (r2_end & ~(pad_align - 1))) return false;

    if (!skip_checks && (range1->linear != range2->linear)) {
        // In linear vs. non-linear case, warn of aliasing
        const char *r1_linear_str = range1->linear ? "Linear" : "Non-linear";
        const char *r2_linear_str = range2->linear ? "linear" : "non-linear";
        auto obj_type = range1->image ? VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT : VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
        *skip |= log_msg(
            report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, obj_type, range1->handle, kVUID_Core_MemTrack_InvalidAliasing,
            "%s %s is aliased with %s %s which may indicate a bug. For further info refer to the Buffer-Image Granularity "
            "section of the Vulkan specification. "
            "(https://www.khronos.org/registry/vulkan/specs/1.0-extensions/xhtml/vkspec.html#resources-bufferimagegranularity)",
            r1_linear_str, report_data->FormatHandle(MemoryRangeTypedHandle(*range1)).c_str(), r2_linear_str,
            report_data->FormatHandle(MemoryRangeTypedHandle(*range2)).c_str());
    }
    // Ranges intersect
    return true;
}
// Simplified RangesIntersect that calls above function to check range1 for intersection with offset & end addresses
bool CoreChecks::RangesIntersect(MEMORY_RANGE const *range1, VkDeviceSize offset, VkDeviceSize end) {
    // Create a local MEMORY_RANGE struct to wrap offset/size
    MEMORY_RANGE range_wrap;
    // Synch linear with range1 to avoid padding and potential validation error case
    range_wrap.linear = range1->linear;
    range_wrap.start = offset;
    range_wrap.end = end;
    bool tmp_bool;
    return RangesIntersect(range1, &range_wrap, &tmp_bool, true);
}

bool CoreChecks::ValidateInsertMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize memoryOffset,
                                           VkMemoryRequirements memRequirements, VulkanObjectType object_type, bool is_linear,
                                           const char *api_name) {
    bool skip = false;

    MEMORY_RANGE range;
    range.image = object_type == kVulkanObjectTypeImage;
    range.handle = handle;
    range.linear = is_linear;
    range.memory = mem_info->mem;
    range.start = memoryOffset;
    range.size = memRequirements.size;
    range.end = memoryOffset + memRequirements.size - 1;
    range.aliases.clear();

    // Check for aliasing problems.
    for (auto &obj_range_pair : mem_info->bound_ranges) {
        auto check_range = &obj_range_pair.second;
        bool intersection_error = false;
        if (RangesIntersect(&range, check_range, &intersection_error, false)) {
            skip |= intersection_error;
            range.aliases.insert(check_range);
        }
    }

    if (memoryOffset >= mem_info->alloc_info.allocationSize) {
        const char *error_code = nullptr;
        if (object_type == kVulkanObjectTypeBuffer) {
            error_code = "VUID-vkBindBufferMemory-memoryOffset-01031";
        } else if (object_type == kVulkanObjectTypeImage) {
            error_code = "VUID-vkBindImageMemory-memoryOffset-01046";
        } else if (object_type == kVulkanObjectTypeAccelerationStructureNV) {
            error_code = "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-02451";
        } else {
            // Unsupported object type
            assert(false);
        }

        skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                       HandleToUint64(mem_info->mem), error_code,
                       "In %s, attempting to bind %s to %s, memoryOffset=0x%" PRIxLEAST64
                       " must be less than the memory allocation size 0x%" PRIxLEAST64 ".",
                       api_name, report_data->FormatHandle(mem_info->mem).c_str(),
                       report_data->FormatHandle(MemoryRangeTypedHandle(range)).c_str(), memoryOffset,
                       mem_info->alloc_info.allocationSize);
    }

    return skip;
}

// Object with given handle is being bound to memory w/ given mem_info struct.
//  Track the newly bound memory range with given memoryOffset
//  Also scan any previous ranges, track aliased ranges with new range, and flag an error if a linear
//  and non-linear range incorrectly overlap.
// Return true if an error is flagged and the user callback returns "true", otherwise false
// is_image indicates an image object, otherwise handle is for a buffer
// is_linear indicates a buffer or linear image
void CoreChecks::InsertMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize memoryOffset,
                                   VkMemoryRequirements memRequirements, VulkanObjectType object_type, bool is_linear) {
    MEMORY_RANGE range;

    range.image = object_type == kVulkanObjectTypeImage;
    range.handle = handle;
    range.linear = is_linear;
    range.memory = mem_info->mem;
    range.start = memoryOffset;
    range.size = memRequirements.size;
    range.end = memoryOffset + memRequirements.size - 1;
    range.aliases.clear();
    // Update Memory aliasing
    // Save aliased ranges so we can copy into final map entry below. Can't do it in loop b/c we don't yet have final ptr. If we
    // inserted into map before loop to get the final ptr, then we may enter loop when not needed & we check range against itself
    std::unordered_set<MEMORY_RANGE *> tmp_alias_ranges;
    for (auto &obj_range_pair : mem_info->bound_ranges) {
        auto check_range = &obj_range_pair.second;
        bool intersection_error = false;
        if (RangesIntersect(&range, check_range, &intersection_error, true)) {
            range.aliases.insert(check_range);
            tmp_alias_ranges.insert(check_range);
        }
    }
    mem_info->bound_ranges[handle] = std::move(range);
    for (auto tmp_range : tmp_alias_ranges) {
        tmp_range->aliases.insert(&mem_info->bound_ranges[handle]);
    }

    if (object_type == kVulkanObjectTypeImage) {
        mem_info->bound_images.insert(handle);
    } else if (object_type == kVulkanObjectTypeBuffer) {
        mem_info->bound_buffers.insert(handle);
    } else if (object_type == kVulkanObjectTypeAccelerationStructureNV) {
        mem_info->bound_acceleration_structures.insert(handle);
    } else {
        // Unsupported object type
        assert(false);
    }
}

bool CoreChecks::ValidateInsertImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                VkMemoryRequirements mem_reqs, bool is_linear, const char *api_name) {
    return ValidateInsertMemoryRange(HandleToUint64(image), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeImage, is_linear,
                                     api_name);
}
void CoreChecks::InsertImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                        VkMemoryRequirements mem_reqs, bool is_linear) {
    InsertMemoryRange(HandleToUint64(image), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeImage, is_linear);
}

bool CoreChecks::ValidateInsertBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                 VkMemoryRequirements mem_reqs, const char *api_name) {
    return ValidateInsertMemoryRange(HandleToUint64(buffer), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeBuffer, true,
                                     api_name);
}
void CoreChecks::InsertBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                         VkMemoryRequirements mem_reqs) {
    InsertMemoryRange(HandleToUint64(buffer), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeBuffer, true);
}

bool CoreChecks::ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE *mem_info,
                                                                VkDeviceSize mem_offset, VkMemoryRequirements mem_reqs,
                                                                const char *api_name) {
    return ValidateInsertMemoryRange(HandleToUint64(as), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeAccelerationStructureNV,
                                     true, api_name);
}
void CoreChecks::InsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE *mem_info,
                                                        VkDeviceSize mem_offset, VkMemoryRequirements mem_reqs) {
    InsertMemoryRange(HandleToUint64(as), mem_info, mem_offset, mem_reqs, kVulkanObjectTypeAccelerationStructureNV, true);
}

// Remove MEMORY_RANGE struct for give handle from bound_ranges of mem_info
//  is_image indicates if handle is for image or buffer
//  This function will also remove the handle-to-index mapping from the appropriate
//  map and clean up any aliases for range being removed.
static void RemoveMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info, VulkanObjectType object_type) {
    auto erase_range = &mem_info->bound_ranges[handle];
    for (auto alias_range : erase_range->aliases) {
        alias_range->aliases.erase(erase_range);
    }
    erase_range->aliases.clear();
    mem_info->bound_ranges.erase(handle);
    if (object_type == kVulkanObjectTypeImage) {
        mem_info->bound_images.erase(handle);
    } else if (object_type == kVulkanObjectTypeBuffer) {
        mem_info->bound_buffers.erase(handle);
    } else if (object_type == kVulkanObjectTypeAccelerationStructureNV) {
        mem_info->bound_acceleration_structures.erase(handle);
    } else {
        // Unsupported object type
        assert(false);
    }
}

void ValidationStateTracker::RemoveBufferMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(handle, mem_info, kVulkanObjectTypeBuffer);
}

void ValidationStateTracker::RemoveImageMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(handle, mem_info, kVulkanObjectTypeImage);
}

void ValidationStateTracker::RemoveAccelerationStructureMemoryRange(uint64_t handle, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(handle, mem_info, kVulkanObjectTypeAccelerationStructureNV);
}

bool CoreChecks::ValidateMemoryTypes(const DEVICE_MEMORY_STATE *mem_info, const uint32_t memory_type_bits, const char *funcName,
                                     const char *msgCode) {
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

bool CoreChecks::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset, const char *api_name) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);

    bool skip = false;
    if (buffer_state) {
        // Track objects tied to memory
        uint64_t buffer_handle = HandleToUint64(buffer);
        const VulkanTypedHandle obj_struct(buffer, kVulkanObjectTypeBuffer);
        skip = ValidateSetMemBinding(mem, obj_struct, api_name);
        if (!buffer_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetBufferMemoryRequirements() prior to calling
            // BindBufferMemory, but it's implied in that memory being bound must conform with VkMemoryRequirements from
            // vkGetBufferMemoryRequirements()
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, buffer_handle,
                            kVUID_Core_DrawState_InvalidBuffer,
                            "%s: Binding memory to %s but vkGetBufferMemoryRequirements() has not been called on that buffer.",
                            api_name, report_data->FormatHandle(buffer).c_str());
            // Make the call for them so we can verify the state
            DispatchGetBufferMemoryRequirements(device, buffer, &buffer_state->requirements);
        }

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

bool CoreChecks::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    const char *api_name = "vkBindBufferMemory()";
    return ValidateBindBufferMemory(buffer, mem, memoryOffset, api_name);
}

void CoreChecks::UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        // Track bound memory range information
        auto mem_info = GetDevMemState(mem);
        if (mem_info) {
            InsertBufferMemoryRange(buffer, mem_info, memoryOffset, buffer_state->requirements);
        }
        // Track objects tied to memory
        SetMemBinding(mem, buffer_state, memoryOffset, VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer));
    }
}

void CoreChecks::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    UpdateBindBufferMemoryState(buffer, mem, memoryOffset);
}

bool CoreChecks::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                  const VkBindBufferMemoryInfoKHR *pBindInfos) {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset, api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfoKHR *pBindInfos) {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset, api_name);
    }
    return skip;
}

void CoreChecks::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfoKHR *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void CoreChecks::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfoKHR *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void CoreChecks::RecordGetBufferMemoryRequirementsState(VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        buffer_state->requirements = *pMemoryRequirements;
        buffer_state->memory_requirements_checked = true;
    }
}

void CoreChecks::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                           VkMemoryRequirements *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(buffer, pMemoryRequirements);
}

void CoreChecks::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                            VkMemoryRequirements2KHR *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

void CoreChecks::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                               VkMemoryRequirements2KHR *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

bool CoreChecks::ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2 *pInfo) {
    bool skip = false;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateGetImageMemoryRequirements2ANDROID(pInfo->image);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                            VkMemoryRequirements2 *pMemoryRequirements) {
    return ValidateGetImageMemoryRequirements2(pInfo);
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                               VkMemoryRequirements2 *pMemoryRequirements) {
    return ValidateGetImageMemoryRequirements2(pInfo);
}

void CoreChecks::RecordGetImageMemoryRequiementsState(VkImage image, VkMemoryRequirements *pMemoryRequirements) {
    IMAGE_STATE *image_state = GetImageState(image);
    if (image_state) {
        image_state->requirements = *pMemoryRequirements;
        image_state->memory_requirements_checked = true;
    }
}

void CoreChecks::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                          VkMemoryRequirements *pMemoryRequirements) {
    RecordGetImageMemoryRequiementsState(image, pMemoryRequirements);
}

void CoreChecks::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                           VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetImageMemoryRequiementsState(pInfo->image, &pMemoryRequirements->memoryRequirements);
}

void CoreChecks::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                              VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetImageMemoryRequiementsState(pInfo->image, &pMemoryRequirements->memoryRequirements);
}

static void RecordGetImageSparseMemoryRequirementsState(IMAGE_STATE *image_state,
                                                        VkSparseImageMemoryRequirements *sparse_image_memory_requirements) {
    image_state->sparse_requirements.emplace_back(*sparse_image_memory_requirements);
    if (sparse_image_memory_requirements->formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
        image_state->sparse_metadata_required = true;
    }
}

void CoreChecks::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                uint32_t *pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    auto image_state = GetImageState(image);
    image_state->get_sparse_reqs_called = true;
    if (!pSparseMemoryRequirements) return;
    for (uint32_t i = 0; i < *pSparseMemoryRequirementCount; i++) {
        RecordGetImageSparseMemoryRequirementsState(image_state, &pSparseMemoryRequirements[i]);
    }
}

void CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                 const VkImageSparseMemoryRequirementsInfo2KHR *pInfo,
                                                                 uint32_t *pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements) {
    auto image_state = GetImageState(pInfo->image);
    image_state->get_sparse_reqs_called = true;
    if (!pSparseMemoryRequirements) return;
    for (uint32_t i = 0; i < *pSparseMemoryRequirementCount; i++) {
        assert(!pSparseMemoryRequirements[i].pNext);  // TODO: If an extension is ever added here we need to handle it
        RecordGetImageSparseMemoryRequirementsState(image_state, &pSparseMemoryRequirements[i].memoryRequirements);
    }
}

void CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements) {
    auto image_state = GetImageState(pInfo->image);
    image_state->get_sparse_reqs_called = true;
    if (!pSparseMemoryRequirements) return;
    for (uint32_t i = 0; i < *pSparseMemoryRequirementCount; i++) {
        assert(!pSparseMemoryRequirements[i].pNext);  // TODO: If an extension is ever added here we need to handle it
        RecordGetImageSparseMemoryRequirementsState(image_state, &pSparseMemoryRequirements[i].memoryRequirements);
    }
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        VkImageFormatProperties2 *pImageFormatProperties) {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(report_data, pImageFormatInfo, pImageFormatProperties);
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                           VkImageFormatProperties2 *pImageFormatProperties) {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(report_data, pImageFormatInfo, pImageFormatProperties);
    return skip;
}

void ValidationStateTracker::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                              const VkAllocationCallbacks *pAllocator) {
    if (!shaderModule) return;
    shaderModuleMap.erase(shaderModule);
}

bool CoreChecks::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    PIPELINE_STATE *pipeline_state = GetPipelineState(pipeline);
    const VulkanTypedHandle obj_struct(pipeline, kVulkanObjectTypePipeline);
    bool skip = false;
    if (pipeline_state) {
        skip |= ValidateObjectNotInUse(pipeline_state, obj_struct, "vkDestroyPipeline", "VUID-vkDestroyPipeline-pipeline-00765");
    }
    return skip;
}

void ValidationStateTracker::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                          const VkAllocationCallbacks *pAllocator) {
    if (!pipeline) return;
    PIPELINE_STATE *pipeline_state = GetPipelineState(pipeline);
    const VulkanTypedHandle obj_struct(pipeline, kVulkanObjectTypePipeline);
    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(pipeline_state->cb_bindings, obj_struct);
    pipelineMap.erase(pipeline);
}

void CoreChecks::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    if (pipeline && enabled.gpu_validation) {
        GpuPreCallRecordDestroyPipeline(pipeline);
    }

    StateTracker::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
}

void ValidationStateTracker::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!pipelineLayout) return;
    pipelineLayoutMap.erase(pipelineLayout);
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) {
    SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
    const VulkanTypedHandle obj_struct(sampler, kVulkanObjectTypeSampler);
    bool skip = false;
    if (sampler_state) {
        skip |= ValidateObjectNotInUse(sampler_state, obj_struct, "vkDestroySampler", "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) {
    if (!sampler) return;
    SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
    const VulkanTypedHandle obj_struct(sampler, kVulkanObjectTypeSampler);
    // Any bound cmd buffers are now invalid
    if (sampler_state) {
        InvalidateCommandBuffers(sampler_state->cb_bindings, obj_struct);
    }
    samplerMap.erase(sampler);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                     const VkAllocationCallbacks *pAllocator) {
    if (!descriptorSetLayout) return;
    auto layout_it = descriptorSetLayoutMap.find(descriptorSetLayout);
    if (layout_it != descriptorSetLayoutMap.end()) {
        layout_it->second.get()->MarkDestroyed();
        descriptorSetLayoutMap.erase(layout_it);
    }
}

bool CoreChecks::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks *pAllocator) {
    DESCRIPTOR_POOL_STATE *desc_pool_state = GetDescriptorPoolState(descriptorPool);
    const VulkanTypedHandle obj_struct(descriptorPool, kVulkanObjectTypeDescriptorPool);
    bool skip = false;
    if (desc_pool_state) {
        skip |= ValidateObjectNotInUse(desc_pool_state, obj_struct, "vkDestroyDescriptorPool",
                                       "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    }
    return skip;
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!descriptorPool) return;
    DESCRIPTOR_POOL_STATE *desc_pool_state = GetDescriptorPoolState(descriptorPool);
    const VulkanTypedHandle obj_struct(descriptorPool, kVulkanObjectTypeDescriptorPool);
    if (desc_pool_state) {
        // Any bound cmd buffers are now invalid
        InvalidateCommandBuffers(desc_pool_state->cb_bindings, obj_struct);
        // Free sets that were in this pool
        for (auto ds : desc_pool_state->sets) {
            FreeDescriptorSet(ds);
        }
        descriptorPoolMap.erase(descriptorPool);
    }
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

// Free all command buffers in given list, removing all references/links to them using ResetCommandBufferState
void ValidationStateTracker::FreeCommandBufferStates(COMMAND_POOL_STATE *pool_state, const uint32_t command_buffer_count,
                                                     const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < command_buffer_count; i++) {
        auto cb_state = GetCBState(command_buffers[i]);
        // Remove references to command buffer's state and delete
        if (cb_state) {
            // reset prior to delete, removing various references to it.
            // TODO: fix this, it's insane.
            ResetCommandBufferState(cb_state->commandBuffer);
            // Remove the cb_state's references from COMMAND_POOL_STATEs
            pool_state->commandBuffers.erase(command_buffers[i]);
            // Remove the cb debug labels
            EraseCmdDebugUtilsLabel(report_data, cb_state->commandBuffer);
            // Remove CBState from CB map
            commandBufferMap.erase(cb_state->commandBuffer);
        }
    }
}

bool CoreChecks::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer *pCommandBuffers) {
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

void ValidationStateTracker::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                             uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    auto pPool = GetCommandPoolState(commandPool);
    FreeCommandBufferStates(pPool, commandBufferCount, pCommandBuffers);
}

bool CoreChecks::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) {
    return ValidateDeviceQueueFamily(pCreateInfo->queueFamilyIndex, "vkCreateCommandPool", "pCreateInfo->queueFamilyIndex",
                                     "VUID-vkCreateCommandPool-queueFamilyIndex-01937");
}

void CoreChecks::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    std::unique_ptr<COMMAND_POOL_STATE> cmd_pool_state(new COMMAND_POOL_STATE{});
    cmd_pool_state->createFlags = pCreateInfo->flags;
    cmd_pool_state->queueFamilyIndex = pCreateInfo->queueFamilyIndex;
    commandPoolMap[*pCommandPool] = std::move(cmd_pool_state);
}

bool CoreChecks::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
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

void CoreChecks::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool, VkResult result) {
    if (VK_SUCCESS != result) return;
    std::unique_ptr<QUERY_POOL_STATE> query_pool_state(new QUERY_POOL_STATE{});
    query_pool_state->createInfo = *pCreateInfo;
    queryPoolMap[*pQueryPool] = std::move(query_pool_state);

    QueryObject query_obj{*pQueryPool, 0u};
    for (uint32_t i = 0; i < pCreateInfo->queryCount; ++i) {
        query_obj.query = i;
        queryToStateMap[query_obj] = QUERYSTATE_UNKNOWN;
    }
}

bool CoreChecks::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks *pAllocator) {
    COMMAND_POOL_STATE *cp_state = GetCommandPoolState(commandPool);
    bool skip = false;
    if (cp_state) {
        // Verify that command buffers in pool are complete (not in-flight)
        skip |= CheckCommandBuffersInFlight(cp_state, "destroy command pool with", "VUID-vkDestroyCommandPool-commandPool-00041");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                 const VkAllocationCallbacks *pAllocator) {
    if (!commandPool) return;
    COMMAND_POOL_STATE *cp_state = GetCommandPoolState(commandPool);
    // Remove cmdpool from cmdpoolmap, after freeing layer data for the command buffers
    // "When a pool is destroyed, all command buffers allocated from the pool are freed."
    if (cp_state) {
        // Create a vector, as FreeCommandBufferStates deletes from cp_state->commandBuffers during iteration.
        std::vector<VkCommandBuffer> cb_vec{cp_state->commandBuffers.begin(), cp_state->commandBuffers.end()};
        FreeCommandBufferStates(cp_state, static_cast<uint32_t>(cb_vec.size()), cb_vec.data());
        commandPoolMap.erase(commandPool);
    }
}

bool CoreChecks::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    const auto *command_pool_state = GetCommandPoolState(commandPool);
    return CheckCommandBuffersInFlight(command_pool_state, "reset command pool with", "VUID-vkResetCommandPool-commandPool-00040");
}

void ValidationStateTracker::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                                            VkCommandPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    // Reset all of the CBs allocated from this pool
    auto command_pool_state = GetCommandPoolState(commandPool);
    for (auto cmdBuffer : command_pool_state->commandBuffers) {
        ResetCommandBufferState(cmdBuffer);
    }
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto pFence = GetFenceState(pFences[i]);
        if (pFence && pFence->scope == kSyncScopeInternal && pFence->state == FENCE_INFLIGHT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,
                            HandleToUint64(pFences[i]), "VUID-vkResetFences-pFences-01123", "%s is in use.",
                            report_data->FormatHandle(pFences[i]).c_str());
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkResult result) {
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto pFence = GetFenceState(pFences[i]);
        if (pFence) {
            if (pFence->scope == kSyncScopeInternal) {
                pFence->state = FENCE_UNSIGNALED;
            } else if (pFence->scope == kSyncScopeExternalTemporary) {
                pFence->scope = kSyncScopeInternal;
            }
        }
    }
}

// For given cb_nodes, invalidate them and track object causing invalidation
void ValidationStateTracker::InvalidateCommandBuffers(std::unordered_set<CMD_BUFFER_STATE *> const &cb_nodes,
                                                      const VulkanTypedHandle &obj) {
    for (auto cb_node : cb_nodes) {
        if (cb_node->state == CB_RECORDING) {
            cb_node->state = CB_INVALID_INCOMPLETE;
        } else if (cb_node->state == CB_RECORDED) {
            cb_node->state = CB_INVALID_COMPLETE;
        }
        cb_node->broken_bindings.push_back(obj);

        // if secondary, then propagate the invalidation to the primaries that will call us.
        if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
            InvalidateCommandBuffers(cb_node->linkedCommandBuffers, obj);
        }
    }
}

bool CoreChecks::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks *pAllocator) {
    FRAMEBUFFER_STATE *framebuffer_state = GetFramebufferState(framebuffer);
    const VulkanTypedHandle obj_struct(framebuffer, kVulkanObjectTypeFramebuffer);
    bool skip = false;
    if (framebuffer_state) {
        skip |= ValidateObjectNotInUse(framebuffer_state, obj_struct, "vkDestroyFramebuffer",
                                       "VUID-vkDestroyFramebuffer-framebuffer-00892");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                 const VkAllocationCallbacks *pAllocator) {
    if (!framebuffer) return;
    FRAMEBUFFER_STATE *framebuffer_state = GetFramebufferState(framebuffer);
    const VulkanTypedHandle obj_struct(framebuffer, kVulkanObjectTypeFramebuffer);
    InvalidateCommandBuffers(framebuffer_state->cb_bindings, obj_struct);
    frameBufferMap.erase(framebuffer);
}

bool CoreChecks::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                  const VkAllocationCallbacks *pAllocator) {
    RENDER_PASS_STATE *rp_state = GetRenderPassState(renderPass);
    const VulkanTypedHandle obj_struct(renderPass, kVulkanObjectTypeRenderPass);
    bool skip = false;
    if (rp_state) {
        skip |= ValidateObjectNotInUse(rp_state, obj_struct, "vkDestroyRenderPass", "VUID-vkDestroyRenderPass-renderPass-00873");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) {
    if (!renderPass) return;
    RENDER_PASS_STATE *rp_state = GetRenderPassState(renderPass);
    const VulkanTypedHandle obj_struct(renderPass, kVulkanObjectTypeRenderPass);
    InvalidateCommandBuffers(rp_state->cb_bindings, obj_struct);
    renderPassMap.erase(renderPass);
}

// Access helper functions for external modules
VkFormatProperties CoreChecks::GetPDFormatProperties(const VkFormat format) const {
    VkFormatProperties format_properties;
    DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
    return format_properties;
}

VkResult CoreChecks::GetPDImageFormatProperties(const VkImageCreateInfo *image_ci,
                                                VkImageFormatProperties *pImageFormatProperties) {
    return DispatchGetPhysicalDeviceImageFormatProperties(physical_device, image_ci->format, image_ci->imageType, image_ci->tiling,
                                                          image_ci->usage, image_ci->flags, pImageFormatProperties);
}

VkResult CoreChecks::GetPDImageFormatProperties2(const VkPhysicalDeviceImageFormatInfo2 *phys_dev_image_fmt_info,
                                                 VkImageFormatProperties2 *pImageFormatProperties) const {
    if (!instance_extensions.vk_khr_get_physical_device_properties_2) return VK_ERROR_EXTENSION_NOT_PRESENT;
    return DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, phys_dev_image_fmt_info, pImageFormatProperties);
}

void CoreChecks::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkFence *pFence, VkResult result) {
    if (VK_SUCCESS != result) return;
    std::unique_ptr<FENCE_STATE> fence_state(new FENCE_STATE{});
    fence_state->fence = *pFence;
    fence_state->createInfo = *pCreateInfo;
    fence_state->state = (pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED;
    fenceMap[*pFence] = std::move(fence_state);
}

// Validation cache:
// CV is the bottommost implementor of this extension. Don't pass calls down.
// utility function to set collective state for pipeline
void SetPipelineState(PIPELINE_STATE *pPipe) {
    // If any attachment used by this pipeline has blendEnable, set top-level blendEnable
    if (pPipe->graphicsPipelineCI.pColorBlendState) {
        for (size_t i = 0; i < pPipe->attachments.size(); ++i) {
            if (VK_TRUE == pPipe->attachments[i].blendEnable) {
                if (((pPipe->attachments[i].dstAlphaBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                     (pPipe->attachments[i].dstAlphaBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                    ((pPipe->attachments[i].dstColorBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                     (pPipe->attachments[i].dstColorBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                    ((pPipe->attachments[i].srcAlphaBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                     (pPipe->attachments[i].srcAlphaBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                    ((pPipe->attachments[i].srcColorBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                     (pPipe->attachments[i].srcColorBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA))) {
                    pPipe->blendConstantsEnabled = true;
                }
            }
        }
    }
}

bool CoreChecks::ValidatePipelineVertexDivisors(std::vector<std::unique_ptr<PIPELINE_STATE>> const &pipe_state_vec,
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

bool ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                    void *cgpl_state_data) {
    // Set up the state that CoreChecks, gpu_validation and later StateTracker Record will use.
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    cgpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    cgpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        cgpl_state->pipe_state.push_back(std::unique_ptr<PIPELINE_STATE>(new PIPELINE_STATE));
        (cgpl_state->pipe_state)[i]->initGraphicsPipeline(this, &pCreateInfos[i],
                                                          GetRenderPassStateSharedPtr(pCreateInfos[i].renderPass));
        (cgpl_state->pipe_state)[i]->pipeline_layout = *GetPipelineLayout(pCreateInfos[i].layout);
    }
    return false;
}

bool CoreChecks::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        void *cgpl_state_data) {
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

// GPU validation may replace pCreateInfos for the down-chain call
void CoreChecks::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                      const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      void *cgpl_state_data) {
    // GPU Validation may replace instrumented shaders with non-instrumented ones, so allow it to modify the createinfos.
    if (enabled.gpu_validation) {
        create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
        cgpl_state->gpu_create_infos = GpuPreCallRecordCreateGraphicsPipelines(pipelineCache, count, pCreateInfos, pAllocator,
                                                                               pPipelines, cgpl_state->pipe_state);
        cgpl_state->pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(cgpl_state->gpu_create_infos.data());
    }
}

void ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   VkResult result, void *cgpl_state_data) {
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (cgpl_state->pipe_state)[i]->pipeline = pPipelines[i];
            pipelineMap[pPipelines[i]] = std::move((cgpl_state->pipe_state)[i]);
        }
    }
    cgpl_state->pipe_state.clear();
}

void CoreChecks::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       VkResult result, void *cgpl_state_data) {
    StateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, result,
                                                        cgpl_state_data);
    // GPU val needs clean up regardless of result
    if (enabled.gpu_validation) {
        create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
        GpuPostCallRecordCreateGraphicsPipelines(count, pCreateInfos, pAllocator, pPipelines);
        cgpl_state->gpu_create_infos.clear();
    }
}

bool ValidationStateTracker::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   void *ccpl_state_data) {
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    ccpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    ccpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        ccpl_state->pipe_state.push_back(unique_ptr<PIPELINE_STATE>(new PIPELINE_STATE));
        ccpl_state->pipe_state.back()->initComputePipeline(this, &pCreateInfos[i]);
        ccpl_state->pipe_state.back()->pipeline_layout = *GetPipelineLayout(pCreateInfos[i].layout);
    }
    return false;
}

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       void *ccpl_state_data) {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                    pPipelines, ccpl_state_data);

    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        // TODO: Add Compute Pipeline Verification
        skip |= ValidateComputePipeline(ccpl_state->pipe_state.back().get());
    }
    return skip;
}

void CoreChecks::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkComputePipelineCreateInfo *pCreateInfos,
                                                     const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                     void *ccpl_state_data) {
    // GPU Validation may replace instrumented shaders with non-instrumented ones, so allow it to modify the createinfos.
    if (enabled.gpu_validation) {
        auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
        ccpl_state->gpu_create_infos = GpuPreCallRecordCreateComputePipelines(pipelineCache, count, pCreateInfos, pAllocator,
                                                                              pPipelines, ccpl_state->pipe_state);
        ccpl_state->pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(ccpl_state->gpu_create_infos.data());
    }
}

void ValidationStateTracker::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkComputePipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  VkResult result, void *ccpl_state_data) {
    create_compute_pipeline_api_state *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);

    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (ccpl_state->pipe_state)[i]->pipeline = pPipelines[i];
            pipelineMap[pPipelines[i]] = std::move((ccpl_state->pipe_state)[i]);
        }
    }
    ccpl_state->pipe_state.clear();
}

void CoreChecks::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      VkResult result, void *ccpl_state_data) {
    StateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, result,
                                                       ccpl_state_data);

    // GPU val needs clean up regardless of result
    if (enabled.gpu_validation) {
        create_compute_pipeline_api_state *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
        GpuPostCallRecordCreateComputePipelines(count, pCreateInfos, pAllocator, pPipelines);
        ccpl_state->gpu_create_infos.clear();
    }
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t count,
                                                                        const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(unique_ptr<PIPELINE_STATE>(new PIPELINE_STATE));
        crtpl_state->pipe_state.back()->initRayTracingPipelineNV(this, &pCreateInfos[i]);
        crtpl_state->pipe_state.back()->pipeline_layout = *GetPipelineLayout(pCreateInfos[i].layout);
    }
    return false;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            void *crtpl_state_data) {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                         pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        skip |= ValidateRayTracingPipelineNV(crtpl_state->pipe_state[i].get());
    }
    return skip;
}

void CoreChecks::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                          const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                          void *crtpl_state_data) {
    // GPU Validation may replace instrumented shaders with non-instrumented ones, so allow it to modify the createinfos.
    if (enabled.gpu_validation) {
        auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
        crtpl_state->gpu_create_infos = GpuPreCallRecordCreateRayTracingPipelinesNV(pipelineCache, count, pCreateInfos, pAllocator,
                                                                                    pPipelines, crtpl_state->pipe_state);
        crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(crtpl_state->gpu_create_infos.data());
    }
}

void ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, VkResult result, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (crtpl_state->pipe_state)[i]->pipeline = pPipelines[i];
            pipelineMap[pPipelines[i]] = std::move((crtpl_state->pipe_state)[i]);
        }
    }
    crtpl_state->pipe_state.clear();
}

void CoreChecks::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           VkResult result, void *crtpl_state_data) {
    StateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                            result, crtpl_state_data);
    // GPU val needs clean up regardless of result
    if (enabled.gpu_validation) {
        auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
        GpuPostCallRecordCreateRayTracingPipelinesNV(count, pCreateInfos, pAllocator, pPipelines);
        crtpl_state->gpu_create_infos.clear();
    }
}

void CoreChecks::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler, VkResult result) {
    samplerMap[*pSampler] = unique_ptr<SAMPLER_STATE>(new SAMPLER_STATE(pSampler, pCreateInfo));
}

bool CoreChecks::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorSetLayout *pSetLayout) {
    return cvdescriptorset::ValidateDescriptorSetLayoutCreateInfo(
        report_data, pCreateInfo, device_extensions.vk_khr_push_descriptor, phys_dev_ext_props.max_push_descriptors,
        device_extensions.vk_ext_descriptor_indexing, &enabled_features.descriptor_indexing, &enabled_features.inline_uniform_block,
        &phys_dev_ext_props.inline_uniform_block_props);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(VkDevice device,
                                                                     const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorSetLayout *pSetLayout, VkResult result) {
    if (VK_SUCCESS != result) return;
    descriptorSetLayoutMap[*pSetLayout] = std::make_shared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo, *pSetLayout);
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
                                                     const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout) {
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
            set_layouts[i] = GetDescriptorSetLayout(this, pCreateInfo->pSetLayouts[i]);
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

// For repeatable sorting, not very useful for "memory in range" search
struct PushConstantRangeCompare {
    bool operator()(const VkPushConstantRange *lhs, const VkPushConstantRange *rhs) const {
        if (lhs->offset == rhs->offset) {
            if (lhs->size == rhs->size) {
                // The comparison is arbitrary, but avoids false aliasing by comparing all fields.
                return lhs->stageFlags < rhs->stageFlags;
            }
            // If the offsets are the same then sorting by the end of range is useful for validation
            return lhs->size < rhs->size;
        }
        return lhs->offset < rhs->offset;
    }
};

static PushConstantRangesDict push_constant_ranges_dict;

PushConstantRangesId GetCanonicalId(const VkPipelineLayoutCreateInfo *info) {
    if (!info->pPushConstantRanges) {
        // Hand back the empty entry (creating as needed)...
        return push_constant_ranges_dict.look_up(PushConstantRanges());
    }

    // Sort the input ranges to ensure equivalent ranges map to the same id
    std::set<const VkPushConstantRange *, PushConstantRangeCompare> sorted;
    for (uint32_t i = 0; i < info->pushConstantRangeCount; i++) {
        sorted.insert(info->pPushConstantRanges + i);
    }

    PushConstantRanges ranges(sorted.size());
    for (const auto range : sorted) {
        ranges.emplace_back(*range);
    }
    return push_constant_ranges_dict.look_up(std::move(ranges));
}

// Dictionary of canoncial form of the pipeline set layout of descriptor set layouts
static PipelineLayoutSetLayoutsDict pipeline_layout_set_layouts_dict;

// Dictionary of canonical form of the "compatible for set" records
static PipelineLayoutCompatDict pipeline_layout_compat_dict;

static PipelineLayoutCompatId GetCanonicalId(const uint32_t set_index, const PushConstantRangesId pcr_id,
                                             const PipelineLayoutSetLayoutsId set_layouts_id) {
    return pipeline_layout_compat_dict.look_up(PipelineLayoutCompatDef(set_index, pcr_id, set_layouts_id));
}

void CoreChecks::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                   void *cpl_state_data) {
    create_pipeline_layout_api_state *cpl_state = reinterpret_cast<create_pipeline_layout_api_state *>(cpl_state_data);
    if (enabled.gpu_validation) {
        GpuPreCallCreatePipelineLayout(pCreateInfo, pAllocator, pPipelineLayout, &cpl_state->new_layouts,
                                       &cpl_state->modified_create_info);
    }
}

void CoreChecks::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                    VkResult result) {
    StateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);

    // Clean up GPU validation
    if (enabled.gpu_validation) {
        GpuPostCallCreatePipelineLayout(result);
    }
}

void ValidationStateTracker::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkPipelineLayout *pPipelineLayout, VkResult result) {
    if (VK_SUCCESS != result) return;

    std::unique_ptr<PIPELINE_LAYOUT_STATE> pipeline_layout_state(new PIPELINE_LAYOUT_STATE{});
    pipeline_layout_state->layout = *pPipelineLayout;
    pipeline_layout_state->set_layouts.resize(pCreateInfo->setLayoutCount);
    PipelineLayoutSetLayoutsDef set_layouts(pCreateInfo->setLayoutCount);
    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        pipeline_layout_state->set_layouts[i] = GetDescriptorSetLayout(this, pCreateInfo->pSetLayouts[i]);
        set_layouts[i] = pipeline_layout_state->set_layouts[i]->GetLayoutId();
    }

    // Get canonical form IDs for the "compatible for set" contents
    pipeline_layout_state->push_constant_ranges = GetCanonicalId(pCreateInfo);
    auto set_layouts_id = pipeline_layout_set_layouts_dict.look_up(set_layouts);
    pipeline_layout_state->compat_for_set.reserve(pCreateInfo->setLayoutCount);

    // Create table of "compatible for set N" cannonical forms for trivial accept validation
    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        pipeline_layout_state->compat_for_set.emplace_back(
            GetCanonicalId(i, pipeline_layout_state->push_constant_ranges, set_layouts_id));
    }
    pipelineLayoutMap[*pPipelineLayout] = std::move(pipeline_layout_state);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDescriptorPool *pDescriptorPool, VkResult result) {
    if (VK_SUCCESS != result) return;
    descriptorPoolMap[*pDescriptorPool] =
        std::unique_ptr<DESCRIPTOR_POOL_STATE>(new DESCRIPTOR_POOL_STATE(*pDescriptorPool, pCreateInfo));
}

bool CoreChecks::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags) {
    // Make sure sets being destroyed are not currently in-use
    if (disabled.idle_descriptor_set) return false;
    bool skip = false;
    DESCRIPTOR_POOL_STATE *pPool = GetDescriptorPoolState(descriptorPool);
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

void ValidationStateTracker::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                               VkDescriptorPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    DESCRIPTOR_POOL_STATE *pPool = GetDescriptorPoolState(descriptorPool);
    // TODO: validate flags
    // For every set off of this pool, clear it, remove from setMap, and free cvdescriptorset::DescriptorSet
    for (auto ds : pPool->sets) {
        FreeDescriptorSet(ds);
    }
    pPool->sets.clear();
    // Reset available count for each type and available sets for this pool
    for (auto it = pPool->availableDescriptorTypeCount.begin(); it != pPool->availableDescriptorTypeCount.end(); ++it) {
        pPool->availableDescriptorTypeCount[it->first] = pPool->maxDescriptorTypeCount[it->first];
    }
    pPool->availableSets = pPool->maxSets;
}

// Ensure the pool contains enough descriptors and descriptor sets to satisfy
// an allocation request. Fills common_data with the total number of descriptors of each type required,
// as well as DescriptorSetLayout ptrs used for later update.
bool CoreChecks::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets, void *ads_state_data) {
    // Always update common data
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    UpdateAllocateDescriptorSetsData(pAllocateInfo, ads_state);
    // All state checks for AllocateDescriptorSets is done in single function
    return ValidateAllocateDescriptorSets(pAllocateInfo, ads_state);
}

// Allocation state was good and call down chain was made so update state based on allocating descriptor sets
void ValidationStateTracker::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                  VkDescriptorSet *pDescriptorSets, VkResult result,
                                                                  void *ads_state_data) {
    if (VK_SUCCESS != result) return;
    // All the updates are contained in a single cvdescriptorset function
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    PerformAllocateDescriptorSets(pAllocateInfo, pDescriptorSets, ads_state);
}

bool CoreChecks::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                   const VkDescriptorSet *pDescriptorSets) {
    // Make sure that no sets being destroyed are in-flight
    bool skip = false;
    // First make sure sets being destroyed are not currently in-use
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateIdleDescriptorSet(pDescriptorSets[i], "vkFreeDescriptorSets");
        }
    }
    DESCRIPTOR_POOL_STATE *pool_state = GetDescriptorPoolState(descriptorPool);
    if (pool_state && !(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT & pool_state->createInfo.flags)) {
        // Can't Free from a NON_FREE pool
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                        HandleToUint64(descriptorPool), "VUID-vkFreeDescriptorSets-descriptorPool-00312",
                        "It is invalid to call vkFreeDescriptorSets() with a pool created without setting "
                        "VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT.");
    }
    return skip;
}

void ValidationStateTracker::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                             const VkDescriptorSet *pDescriptorSets) {
    DESCRIPTOR_POOL_STATE *pool_state = GetDescriptorPoolState(descriptorPool);
    // Update available descriptor sets in pool
    pool_state->availableSets += count;

    // For each freed descriptor add its resources back into the pool as available and remove from pool and setMap
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            auto descriptor_set = setMap[pDescriptorSets[i]].get();
            uint32_t type_index = 0, descriptor_count = 0;
            for (uint32_t j = 0; j < descriptor_set->GetBindingCount(); ++j) {
                type_index = static_cast<uint32_t>(descriptor_set->GetTypeFromIndex(j));
                descriptor_count = descriptor_set->GetDescriptorCountFromIndex(j);
                pool_state->availableDescriptorTypeCount[type_index] += descriptor_count;
            }
            FreeDescriptorSet(descriptor_set);
            pool_state->sets.erase(descriptor_set);
        }
    }
}

bool CoreChecks::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet *pDescriptorCopies) {
    // First thing to do is perform map look-ups.
    // NOTE : UpdateDescriptorSets is somewhat unique in that it's operating on a number of DescriptorSets
    //  so we can't just do a single map look-up up-front, but do them individually in functions below

    // Now make call(s) that validate state, but don't perform state updates in this function
    // Note, here DescriptorSets is unique in that we don't yet have an instance. Using a helper function in the
    //  namespace which will parse params and make calls into specific class instances
    return ValidateUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies,
                                        "vkUpdateDescriptorSets()");
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                               const VkWriteDescriptorSet *pDescriptorWrites,
                                                               uint32_t descriptorCopyCount,
                                                               const VkCopyDescriptorSet *pDescriptorCopies) {
    cvdescriptorset::PerformUpdateDescriptorSets(this, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                 pDescriptorCopies);
}

void CoreChecks::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                      VkCommandBuffer *pCommandBuffer, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto pPool = GetCommandPoolState(pCreateInfo->commandPool);
    if (pPool) {
        for (uint32_t i = 0; i < pCreateInfo->commandBufferCount; i++) {
            // Add command buffer to its commandPool map
            pPool->commandBuffers.insert(pCommandBuffer[i]);
            std::unique_ptr<CMD_BUFFER_STATE> pCB(new CMD_BUFFER_STATE{});
            pCB->createInfo = *pCreateInfo;
            pCB->device = device;
            // Add command buffer to map
            commandBufferMap[pCommandBuffer[i]] = std::move(pCB);
            ResetCommandBufferState(pCommandBuffer[i]);
        }
    }
}

// Add bindings between the given cmd buffer & framebuffer and the framebuffer's children
void ValidationStateTracker::AddFramebufferBinding(CMD_BUFFER_STATE *cb_state, FRAMEBUFFER_STATE *fb_state) {
    AddCommandBufferBinding(&fb_state->cb_bindings, VulkanTypedHandle(fb_state->framebuffer, kVulkanObjectTypeFramebuffer),
                            cb_state);

    const uint32_t attachmentCount = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachmentCount; ++attachment) {
        auto view_state = GetAttachmentImageViewState(fb_state, attachment);
        if (view_state) {
            AddCommandBufferBindingImageView(cb_state, view_state);
        }
    }
}

bool CoreChecks::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo) {
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
        const auto *pPool = GetCommandPoolState(cmdPool);
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

void ValidationStateTracker::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                             const VkCommandBufferBeginInfo *pBeginInfo) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!cb_state) return;
    // This implicitly resets the Cmd Buffer so make sure any fence is done and then clear memory references
    ClearCmdBufAndMemReferences(cb_state);
    if (cb_state->createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        // Secondary Command Buffer
        const VkCommandBufferInheritanceInfo *pInfo = pBeginInfo->pInheritanceInfo;
        if (pInfo) {
            if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                assert(pInfo->renderPass);
                auto framebuffer = GetFramebufferState(pInfo->framebuffer);
                if (framebuffer) {
                    // Connect this framebuffer and its children to this cmdBuffer
                    AddFramebufferBinding(cb_state, framebuffer);
                }
            }
        }
    }
    if (CB_RECORDED == cb_state->state || CB_INVALID_COMPLETE == cb_state->state) {
        ResetCommandBufferState(commandBuffer);
    }
    // Set updated state here in case implicit reset occurs above
    cb_state->state = CB_RECORDING;
    cb_state->beginInfo = *pBeginInfo;
    if (cb_state->beginInfo.pInheritanceInfo) {
        cb_state->inheritanceInfo = *(cb_state->beginInfo.pInheritanceInfo);
        cb_state->beginInfo.pInheritanceInfo = &cb_state->inheritanceInfo;
        // If we are a secondary command-buffer and inheriting.  Update the items we should inherit.
        if ((cb_state->createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) &&
            (cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
            cb_state->activeRenderPass = GetRenderPassState(cb_state->beginInfo.pInheritanceInfo->renderPass);
            cb_state->activeSubpass = cb_state->beginInfo.pInheritanceInfo->subpass;
            cb_state->activeFramebuffer = cb_state->beginInfo.pInheritanceInfo->framebuffer;
            cb_state->framebuffers.insert(cb_state->beginInfo.pInheritanceInfo->framebuffer);
        }
    }

    auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        cb_state->initial_device_mask = chained_device_group_struct->deviceMask;
    } else {
        cb_state->initial_device_mask = (1 << physical_device_count) - 1;
    }
}

bool CoreChecks::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) {
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

void ValidationStateTracker::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!cb_state) return;
    // Cached validation is specific to a specific recording of a specific command buffer.
    for (auto descriptor_set : cb_state->validated_descriptor_sets) {
        descriptor_set->ClearCachedValidation(cb_state);
    }
    cb_state->validated_descriptor_sets.clear();
    if (VK_SUCCESS == result) {
        cb_state->state = CB_RECORDED;
    }
}

bool CoreChecks::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    bool skip = false;
    const CMD_BUFFER_STATE *pCB = GetCBState(commandBuffer);
    if (!pCB) return false;
    VkCommandPool cmdPool = pCB->createInfo.commandPool;
    const auto *pPool = GetCommandPoolState(cmdPool);

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

void ValidationStateTracker::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                              VkResult result) {
    if (VK_SUCCESS == result) {
        ResetCommandBufferState(commandBuffer);
    }
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
                                                VkPipeline pipeline) {
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

void ValidationStateTracker::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                          VkPipeline pipeline) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    auto pipe_state = GetPipelineState(pipeline);
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == pipelineBindPoint) {
        cb_state->status &= ~cb_state->static_status;
        cb_state->static_status = MakeStaticStateMask(pipe_state->graphicsPipelineCI.ptr()->pDynamicState);
        cb_state->status |= cb_state->static_status;
    }
    cb_state->lastBound[pipelineBindPoint].pipeline_state = pipe_state;
    SetPipelineState(pipe_state);
    AddCommandBufferBinding(&pipe_state->cb_bindings, VulkanTypedHandle(pipeline, kVulkanObjectTypePipeline), cb_state);
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport *pViewports) {
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

void ValidationStateTracker::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewport *pViewports) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->viewportMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->status |= CBSTATUS_VIEWPORT_SET;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D *pScissors) {
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
                                                         uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) {
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

void ValidationStateTracker::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                   uint32_t exclusiveScissorCount,
                                                                   const VkRect2D *pExclusiveScissors) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;
    cb_state->status |= CBSTATUS_EXCLUSIVE_SCISSOR_SET;
}

bool CoreChecks::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
        auto view_state = GetImageViewState(imageView);
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
            auto image_state = GetImageState(view_state->create_info.image);
            bool hit_error = false;

            // XXX TODO: While the VUID says "each subresource", only the base mip level is
            // actually used. Since we don't have an existing convenience function to iterate
            // over all mip levels, just don't bother with non-base levels.
            VkImageSubresourceRange &range = view_state->create_info.subresourceRange;
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

void CoreChecks::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                        VkImageLayout imageLayout) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = GetImageViewState(imageView);
        AddCommandBufferBindingImageView(cb_state, view_state);
    }
}

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV *pShadingRatePalettes) {
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

void ValidationStateTracker::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                             uint32_t viewportCount,
                                                                             const VkShadingRatePaletteNV *pShadingRatePalettes) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->status |= CBSTATUS_SHADING_RATE_PALETTE_SET;
}

void CoreChecks::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkAccelerationStructureNV *pAccelerationStructure, VkResult result) {
    if (VK_SUCCESS != result) return;
    std::unique_ptr<ACCELERATION_STRUCTURE_STATE> as_state(new ACCELERATION_STRUCTURE_STATE(*pAccelerationStructure, pCreateInfo));
    accelerationStructureMap[*pAccelerationStructure] = std::move(as_state);
}

void CoreChecks::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2KHR *pMemoryRequirements) {
    ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(pInfo->accelerationStructure);
    if (as_state != nullptr) {
        if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV) {
            as_state->memory_requirements = *pMemoryRequirements;
            as_state->memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV) {
            as_state->build_scratch_memory_requirements = *pMemoryRequirements;
            as_state->build_scratch_memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV) {
            as_state->update_scratch_memory_requirements = *pMemoryRequirements;
            as_state->update_scratch_memory_requirements_checked = true;
        }
    }
}

bool CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV *pBindInfos) {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(info.accelerationStructure);
        if (!as_state) {
            continue;
        }
        uint64_t as_handle = HandleToUint64(info.accelerationStructure);
        if (!as_state->GetBoundMemory().empty()) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT, as_handle,
                "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-02450",
                "vkBindAccelerationStructureMemoryNV(): accelerationStructure must not already be backed by a memory object.");
        }

        if (!as_state->memory_requirements_checked) {
            VkAccelerationStructureMemoryRequirementsInfoNV as_memory_requirements_info = {};
            as_memory_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
            as_memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
            as_memory_requirements_info.accelerationStructure = as_state->acceleration_structure;
            DispatchGetAccelerationStructureMemoryRequirementsNV(device, &as_memory_requirements_info,
                                                                 &as_state->memory_requirements);
            as_state->memory_requirements_checked = true;
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
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, as_handle,
                    "VUID-VkBindAccelerationStructureMemoryInfoNV-size-02595",
                    "vkBindAccelerationStructureMemoryNV(): memory size minus memoryOffset is 0x%" PRIxLEAST64
                    " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                    ", returned from a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure"
                    "and type of VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV.",
                    mem_info->alloc_info.allocationSize - info.memoryOffset, as_state->memory_requirements.memoryRequirements.size);
            }
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                 const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(info.accelerationStructure);
        if (as_state) {
            // Track bound memory range information
            auto mem_info = GetDevMemState(info.memory);
            if (mem_info) {
                if (!as_state->memory_requirements_checked) {
                    VkAccelerationStructureMemoryRequirementsInfoNV as_memory_requirements_info = {};
                    as_memory_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
                    as_memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
                    as_memory_requirements_info.accelerationStructure = as_state->acceleration_structure;
                    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &as_memory_requirements_info,
                                                                         &as_state->memory_requirements);
                    as_state->memory_requirements_checked = true;
                }

                InsertAccelerationStructureMemoryRange(info.accelerationStructure, mem_info, info.memoryOffset,
                                                       as_state->requirements);
            }
            // Track objects tied to memory
            SetMemBinding(info.memory, as_state, info.memoryOffset,
                          VulkanTypedHandle(info.accelerationStructure, kVulkanObjectTypeAccelerationStructureNV));
        }
    }
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBuildAccelerationStructureNV()", VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_BUILDACCELERATIONSTRUCTURENV, "vkCmdBuildAccelerationStructureNV()");

    if (pInfo != nullptr && pInfo->geometryCount > phys_dev_ext_props.ray_tracing_props.maxGeometryCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241",
                        "vkCmdBuildAccelerationStructureNV(): geometryCount [%d] must be less than or equal to "
                        "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.",
                        pInfo->geometryCount);
    }

    ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
    ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);
    BUFFER_STATE *scratch_buffer_state = GetBufferState(scratch);

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
            VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
            memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
            memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
            memoryRequirementsInfo.accelerationStructure = dst;
            DispatchGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo,
                                                                 &dst_as_state->update_scratch_memory_requirements);
            dst_as_state->update_scratch_memory_requirements_checked = true;
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr &&
            dst_as_state->update_scratch_memory_requirements_checked &&
            dst_as_state->update_scratch_memory_requirements.memoryRequirements.size >
                (scratch_buffer_state->binding.size - (scratch_buffer_state->binding.offset + scratchOffset))) {
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
            VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
            memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
            memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
            memoryRequirementsInfo.accelerationStructure = dst;
            DispatchGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo,
                                                                 &dst_as_state->build_scratch_memory_requirements);
            dst_as_state->build_scratch_memory_requirements_checked = true;
        }
        if (scratch_buffer_state != nullptr && dst_as_state != nullptr && dst_as_state->build_scratch_memory_requirements_checked &&
            dst_as_state->build_scratch_memory_requirements.memoryRequirements.size >
                (scratch_buffer_state->binding.size - (scratch_buffer_state->binding.offset + scratchOffset))) {
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

void CoreChecks::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (cb_state) {
        ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
        ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);
        if (dst_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info.initialize(pInfo);
            AddCommandBufferBindingAccelerationStructure(cb_state, dst_as_state);
        }
        if (src_as_state != nullptr) {
            AddCommandBufferBindingAccelerationStructure(cb_state, src_as_state);
        }
    }
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeNV mode) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdCopyAccelerationStructureNV()", VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-cmdpool");

    skip |= ValidateCmd(cb_state, CMD_COPYACCELERATIONSTRUCTURENV, "vkCmdCopyAccelerationStructureNV()");

    ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
    ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);

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

void CoreChecks::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                              VkAccelerationStructureNV src,
                                                              VkCopyAccelerationStructureModeNV mode) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (cb_state) {
        ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureState(src);
        ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureState(dst);
        if (dst_as_state != nullptr && src_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info = src_as_state->build_info;
            AddCommandBufferBindingAccelerationStructure(cb_state, dst_as_state);
            AddCommandBufferBindingAccelerationStructure(cb_state, src_as_state);
        }
    }
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator) {
    ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(accelerationStructure);
    const VulkanTypedHandle obj_struct(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV);
    bool skip = false;
    if (as_state) {
        skip |= ValidateObjectNotInUse(as_state, obj_struct, "vkDestroyAccelerationStructureNV",
                                       "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-02442");
    }
    return skip;
}

void CoreChecks::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!accelerationStructure) return;
    auto *as_state = GetAccelerationStructureState(accelerationStructure);
    if (as_state) {
        const VulkanTypedHandle obj_struct(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV);
        InvalidateCommandBuffers(as_state->cb_bindings, obj_struct);
        for (auto mem_binding : as_state->GetBoundMemory()) {
            auto mem_info = GetDevMemState(mem_binding);
            if (mem_info) {
                RemoveAccelerationStructureMemoryRange(HandleToUint64(accelerationStructure), mem_info);
            }
        }
        ClearMemoryObjectBindings(obj_struct);
        accelerationStructureMap.erase(accelerationStructure);
    }
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
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

void ValidationStateTracker::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_LINE_WIDTH_SET;
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor) {
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

void ValidationStateTracker::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                          float depthBiasClamp, float depthBiasSlopeFactor) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_DEPTH_BIAS_SET;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
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

void ValidationStateTracker::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_BLEND_CONSTANTS_SET;
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
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

void ValidationStateTracker::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                            float maxDepthBounds) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_DEPTH_BOUNDS_SET;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask) {
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

void ValidationStateTracker::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                   uint32_t compareMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_READ_MASK_SET;
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask) {
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

void ValidationStateTracker::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t writeMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_WRITE_MASK_SET;
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference) {
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

void ValidationStateTracker::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t reference) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_REFERENCE_SET;
}

// Update pipeline_layout bind points applying the "Pipeline Layout Compatibility" rules
void ValidationStateTracker::UpdateLastBoundDescriptorSets(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint pipeline_bind_point,
                                                           const PIPELINE_LAYOUT_STATE *pipeline_layout, uint32_t first_set,
                                                           uint32_t set_count,
                                                           const std::vector<cvdescriptorset::DescriptorSet *> descriptor_sets,
                                                           uint32_t dynamic_offset_count, const uint32_t *p_dynamic_offsets) {
    // Defensive
    assert(set_count);
    if (0 == set_count) return;
    assert(pipeline_layout);
    if (!pipeline_layout) return;

    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    assert(last_binding_index < pipeline_layout->compat_for_set.size());

    // Some useful shorthand
    auto &last_bound = cb_state->lastBound[pipeline_bind_point];

    auto &bound_sets = last_bound.boundDescriptorSets;
    auto &dynamic_offsets = last_bound.dynamicOffsets;
    auto &bound_compat_ids = last_bound.compat_id_for_set;
    auto &pipe_compat_ids = pipeline_layout->compat_for_set;

    const uint32_t current_size = static_cast<uint32_t>(bound_sets.size());
    assert(current_size == dynamic_offsets.size());
    assert(current_size == bound_compat_ids.size());

    // We need this three times in this function, but nowhere else
    auto push_descriptor_cleanup = [&last_bound](const cvdescriptorset::DescriptorSet *ds) -> bool {
        if (ds && ds->IsPushDescriptor()) {
            assert(ds == last_bound.push_descriptor_set.get());
            last_bound.push_descriptor_set = nullptr;
            return true;
        }
        return false;
    };

    // Clean up the "disturbed" before and after the range to be set
    if (required_size < current_size) {
        if (bound_compat_ids[last_binding_index] != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (push_descriptor_cleanup(bound_sets[set_idx])) break;
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        // TODO: put these size tied things in a struct (touches many lines)
        bound_sets.resize(required_size);
        dynamic_offsets.resize(required_size);
        bound_compat_ids.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        if (bound_compat_ids[set_idx] != pipe_compat_ids[set_idx]) {
            push_descriptor_cleanup(bound_sets[set_idx]);
            bound_sets[set_idx] = nullptr;
            dynamic_offsets[set_idx].clear();
            bound_compat_ids[set_idx] = pipe_compat_ids[set_idx];
        }
    }

    // Now update the bound sets with the input sets
    const uint32_t *input_dynamic_offsets = p_dynamic_offsets;  // "read" pointer for dynamic offset data
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        cvdescriptorset::DescriptorSet *descriptor_set = descriptor_sets[input_idx];

        // Record binding (or push)
        if (descriptor_set != last_bound.push_descriptor_set.get()) {
            // Only cleanup the push descriptors if they aren't the currently used set.
            push_descriptor_cleanup(bound_sets[set_idx]);
        }
        bound_sets[set_idx] = descriptor_set;
        bound_compat_ids[set_idx] = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index

        if (descriptor_set) {
            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            // TODO: Add logic for tracking push_descriptor offsets (here or in caller)
            if (set_dynamic_descriptor_count && input_dynamic_offsets) {
                const uint32_t *end_offset = input_dynamic_offsets + set_dynamic_descriptor_count;
                dynamic_offsets[set_idx] = std::vector<uint32_t>(input_dynamic_offsets, end_offset);
                input_dynamic_offsets = end_offset;
                assert(input_dynamic_offsets <= (p_dynamic_offsets + dynamic_offset_count));
            } else {
                dynamic_offsets[set_idx].clear();
            }
            if (!descriptor_set->IsPushDescriptor()) {
                // Can't cache validation of push_descriptors
                cb_state->validated_descriptor_sets.insert(descriptor_set);
            }
        }
    }
}

// Update the bound state for the bind point, including the effects of incompatible pipeline layouts
void ValidationStateTracker::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t firstSet, uint32_t setCount,
                                                                const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                                const uint32_t *pDynamicOffsets) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto pipeline_layout = GetPipelineLayout(layout);
    std::vector<cvdescriptorset::DescriptorSet *> descriptor_sets;
    descriptor_sets.reserve(setCount);

    // Resize binding arrays
    uint32_t last_set_index = firstSet + setCount - 1;
    if (last_set_index >= cb_state->lastBound[pipelineBindPoint].boundDescriptorSets.size()) {
        cb_state->lastBound[pipelineBindPoint].boundDescriptorSets.resize(last_set_index + 1);
        cb_state->lastBound[pipelineBindPoint].dynamicOffsets.resize(last_set_index + 1);
        cb_state->lastBound[pipelineBindPoint].compat_id_for_set.resize(last_set_index + 1);
    }

    // Construct a list of the descriptors
    bool found_non_null = false;
    for (uint32_t i = 0; i < setCount; i++) {
        cvdescriptorset::DescriptorSet *descriptor_set = GetSetNode(pDescriptorSets[i]);
        descriptor_sets.emplace_back(descriptor_set);
        found_non_null |= descriptor_set != nullptr;
    }
    if (found_non_null) {  // which implies setCount > 0
        UpdateLastBoundDescriptorSets(cb_state, pipelineBindPoint, pipeline_layout, firstSet, setCount, descriptor_sets,
                                      dynamicOffsetCount, pDynamicOffsets);
        cb_state->lastBound[pipelineBindPoint].pipeline_layout = layout;
    }
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
                                                      const uint32_t *pDynamicOffsets) {
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
            if (!VerifySetLayoutCompatibility(descriptor_set, pipeline_layout, set_idx + firstSet, error_string)) {
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
    auto pool = GetCommandPoolState(cb_state->createInfo.commandPool);
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
                                                        const VkWriteDescriptorSet *pDescriptorWrites) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
    auto layout_data = GetPipelineLayout(layout);

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
                    cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, VK_NULL_HANDLE, dsl, 0, this);
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

void CoreChecks::RecordCmdPushDescriptorSetState(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                 const VkWriteDescriptorSet *pDescriptorWrites) {
    const auto &pipeline_layout = GetPipelineLayout(layout);
    // Short circuit invalid updates
    if (!pipeline_layout || (set >= pipeline_layout->set_layouts.size()) || !pipeline_layout->set_layouts[set] ||
        !pipeline_layout->set_layouts[set]->IsPushDescriptor())
        return;

    // We need a descriptor set to update the bindings with, compatible with the passed layout
    const auto dsl = pipeline_layout->set_layouts[set];
    auto &last_bound = cb_state->lastBound[pipelineBindPoint];
    auto &push_descriptor_set = last_bound.push_descriptor_set;
    // If we are disturbing the current push_desriptor_set clear it
    if (!push_descriptor_set || !CompatForSet(set, last_bound.compat_id_for_set, pipeline_layout->compat_for_set)) {
        last_bound.UnbindAndResetPushDescriptorSet(new cvdescriptorset::DescriptorSet(0, 0, dsl, 0, this));
    }

    std::vector<cvdescriptorset::DescriptorSet *> descriptor_sets = {push_descriptor_set.get()};
    UpdateLastBoundDescriptorSets(cb_state, pipelineBindPoint, pipeline_layout, set, 1, descriptor_sets, 0, nullptr);
    last_bound.pipeline_layout = layout;

    // Now that we have either the new or extant push_descriptor set ... do the write updates against it
    push_descriptor_set->PerformPushDescriptorsUpdate(descriptorWriteCount, pDescriptorWrites);
}

void CoreChecks::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                      const VkWriteDescriptorSet *pDescriptorWrites) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdPushDescriptorSetState(cb_state, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

static VkDeviceSize GetIndexAlignment(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        default:
            // Not a real index type. Express no alignment requirement here; we expect upper layer
            // to have already picked up on the enum being nonsense.
            return 1;
    }
}

bool CoreChecks::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType) {
    auto buffer_state = GetBufferState(buffer);
    auto cb_node = GetCBState(commandBuffer);
    assert(buffer_state);
    assert(cb_node);

    bool skip =
        ValidateBufferUsageFlags(buffer_state, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true, "VUID-vkCmdBindIndexBuffer-buffer-00433",
                                 "vkCmdBindIndexBuffer()", "VK_BUFFER_USAGE_INDEX_BUFFER_BIT");
    skip |= ValidateCmdQueueFlags(cb_node, "vkCmdBindIndexBuffer()", VK_QUEUE_GRAPHICS_BIT,
                                  "VUID-vkCmdBindIndexBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_node, CMD_BINDINDEXBUFFER, "vkCmdBindIndexBuffer()");
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state, "vkCmdBindIndexBuffer()", "VUID-vkCmdBindIndexBuffer-buffer-00434");
    auto offset_align = GetIndexAlignment(indexType);
    if (offset % offset_align) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), "VUID-vkCmdBindIndexBuffer-offset-00432",
                        "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") does not fall on alignment (%s) boundary.", offset,
                        string_VkIndexType(indexType));
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkIndexType indexType) {
    auto buffer_state = GetBufferState(buffer);
    auto cb_state = GetCBState(commandBuffer);

    cb_state->status |= CBSTATUS_INDEX_BUFFER_BOUND;
    cb_state->index_buffer_binding.buffer = buffer;
    cb_state->index_buffer_binding.size = buffer_state->createInfo.size;
    cb_state->index_buffer_binding.offset = offset;
    cb_state->index_buffer_binding.index_type = indexType;
    // Add binding for this index buffer to this commandbuffer
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) {
    auto cb_state = GetCBState(commandBuffer);
    assert(cb_state);

    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdBindVertexBuffers()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdBindVertexBuffers-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_BINDVERTEXBUFFERS, "vkCmdBindVertexBuffers()");
    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = GetBufferState(pBuffers[i]);
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

void CoreChecks::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                   const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) {
    auto cb_state = GetCBState(commandBuffer);

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding.buffer = pBuffers[i];
        vertex_buffer_binding.offset = pOffsets[i];
        // Add binding for this vertex buffer to this commandbuffer
        AddCommandBufferBindingBuffer(cb_state, GetBufferState(pBuffers[i]));
    }
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
                                                VkDeviceSize dataSize, const void *pData) {
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

void ValidationStateTracker::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    auto cb_state = GetCBState(commandBuffer);
    auto dst_buffer_state = GetBufferState(dstBuffer);

    // Update bindings between buffer and cmd buffer
    AddCommandBufferBindingBuffer(cb_state, dst_buffer_state);
}

bool CoreChecks::SetEventStageMask(VkQueue queue, VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *pCB = GetCBState(commandBuffer);
    if (pCB) {
        pCB->eventToStageMap[event] = stageMask;
    }
    auto queue_data = queueMap.find(queue);
    if (queue_data != queueMap.end()) {
        queue_data->second.eventToStageMap[event] = stageMask;
    }
    return false;
}

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

void CoreChecks::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto event_state = GetEventState(event);
    if (event_state) {
        AddCommandBufferBinding(&event_state->cb_bindings, VulkanTypedHandle(event, kVulkanObjectTypeEvent), cb_state);
        event_state->cb_bindings.insert(cb_state);
    }
    cb_state->events.push_back(event);
    if (!cb_state->waitedEvents.count(event)) {
        cb_state->writeEventsBeforeWait.push_back(event);
    }
    cb_state->eventUpdates.emplace_back([=](VkQueue q) { return SetEventStageMask(q, commandBuffer, event, stageMask); });
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

void CoreChecks::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto event_state = GetEventState(event);
    if (event_state) {
        AddCommandBufferBinding(&event_state->cb_bindings, VulkanTypedHandle(event, kVulkanObjectTypeEvent), cb_state);
        event_state->cb_bindings.insert(cb_state);
    }
    cb_state->events.push_back(event);
    if (!cb_state->waitedEvents.count(event)) {
        cb_state->writeEventsBeforeWait.push_back(event);
    }
    // TODO : Add check for "VUID-vkResetEvent-event-01148"
    cb_state->eventUpdates.emplace_back(
        [=](VkQueue q) { return SetEventStageMask(q, commandBuffer, event, VkPipelineStageFlags(0)); });
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
bool CoreChecks::ValidateImageBarrierImage(const char *funcName, CMD_BUFFER_STATE const *cb_state, VkFramebuffer framebuffer,
                                           uint32_t active_subpass, const safe_VkSubpassDescription2KHR &sub_desc,
                                           const VulkanTypedHandle &rp_handle, uint32_t img_index,
                                           const VkImageMemoryBarrier &img_barrier) {
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
        } else if (device_extensions.vk_khr_depth_stencil_resolve) {
            const auto *resolve = lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolveKHR>(sub_desc.pNext);
            if (resolve && resolve->pDepthStencilResolveAttachment &&
                resolve->pDepthStencilResolveAttachment->attachment == attach_index) {
                sub_image_layout = resolve->pDepthStencilResolveAttachment->layout;
                sub_image_found = true;
            }
        } else {
            for (uint32_t j = 0; j < sub_desc.colorAttachmentCount; ++j) {
                if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pColorAttachments[j].layout;
                    sub_image_found = true;
                    break;
                } else if (sub_desc.pResolveAttachments && sub_desc.pResolveAttachments[j].attachment == attach_index) {
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
bool CoreChecks::ValidateRenderPassImageBarriers(const char *funcName, CMD_BUFFER_STATE *cb_state, uint32_t active_subpass,
                                                 const safe_VkSubpassDescription2KHR &sub_desc, const VulkanTypedHandle &rp_handle,
                                                 const safe_VkSubpassDependency2KHR *dependencies,
                                                 const std::vector<uint32_t> &self_dependencies, uint32_t image_mem_barrier_count,
                                                 const VkImageMemoryBarrier *image_barriers) {
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
        // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE == cb_state->activeFramebuffer) {
            assert(VK_COMMAND_BUFFER_LEVEL_SECONDARY == cb_state->createInfo.level);
            // Secondary CB case w/o FB specified delay validation
            cb_state->cmd_execute_commands_functions.emplace_back([=](CMD_BUFFER_STATE *primary_cb, VkFramebuffer fb) {
                return ValidateImageBarrierImage(funcName, cb_state, fb, active_subpass, sub_desc, rp_handle, i, img_barrier);
            });
        } else {
            skip |= ValidateImageBarrierImage(funcName, cb_state, cb_state->activeFramebuffer, active_subpass, sub_desc, rp_handle,
                                              i, img_barrier);
        }
    }
    return skip;
}

// Validate VUs for Pipeline Barriers that are within a renderPass
// Pre: cb_state->activeRenderPass must be a pointer to valid renderPass state
bool CoreChecks::ValidateRenderPassPipelineBarriers(const char *funcName, CMD_BUFFER_STATE *cb_state,
                                                    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                                    VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                                    const VkMemoryBarrier *mem_barriers, uint32_t buffer_mem_barrier_count,
                                                    const VkBufferMemoryBarrier *buffer_mem_barriers,
                                                    uint32_t image_mem_barrier_count, const VkImageMemoryBarrier *image_barriers) {
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
    ValidatorState(const CoreChecks *device_data, const char *func_name, const CMD_BUFFER_STATE *cb_state,
                   const VulkanTypedHandle &barrier_handle, const VkSharingMode sharing_mode, const std::string *val_codes)
        : report_data_(device_data->report_data),
          func_name_(func_name),
          cb_handle64_(HandleToUint64(cb_state->commandBuffer)),
          barrier_handle_(barrier_handle),
          sharing_mode_(sharing_mode),
          val_codes_(val_codes),
          limit_(static_cast<uint32_t>(device_data->physical_device_state->queue_family_properties.size())),
          mem_ext_(device_data->device_extensions.vk_khr_external_memory) {}

    // Create a validator state from an image state... reducing the image specific to the generic version.
    ValidatorState(const CoreChecks *device_data, const char *func_name, const CMD_BUFFER_STATE *cb_state,
                   const VkImageMemoryBarrier *barrier, const IMAGE_STATE *state)
        : ValidatorState(device_data, func_name, cb_state, VulkanTypedHandle(barrier->image, kVulkanObjectTypeImage),
                         state->createInfo.sharingMode, image_error_codes) {}

    // Create a validator state from an buffer state... reducing the buffer specific to the generic version.
    ValidatorState(const CoreChecks *device_data, const char *func_name, const CMD_BUFFER_STATE *cb_state,
                   const VkBufferMemoryBarrier *barrier, const BUFFER_STATE *state)
        : ValidatorState(device_data, func_name, cb_state, VulkanTypedHandle(barrier->buffer, kVulkanObjectTypeBuffer),
                         state->createInfo.sharingMode, buffer_error_codes) {}

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
    static bool ValidateAtQueueSubmit(const VkQueue queue, const CoreChecks *device_data, uint32_t src_family, uint32_t dst_family,
                                      const ValidatorState &val) {
        auto queue_data_it = device_data->queueMap.find(queue);
        if (queue_data_it == device_data->queueMap.end()) return false;

        uint32_t queue_family = queue_data_it->second.queueFamilyIndex;
        if ((src_family != queue_family) && (dst_family != queue_family)) {
            const std::string &val_code = val.val_codes_[kSubmitQueueMustMatchSrcOrDst];
            const char *src_annotation = val.GetFamilyAnnotation(src_family);
            const char *dst_annotation = val.GetFamilyAnnotation(dst_family);
            return log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                           HandleToUint64(queue), val_code,
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
        return IsValid(queue_family) || (mem_ext_ && IsSpecial(queue_family));
    }
    inline bool IsIgnored(uint32_t queue_family) const { return queue_family == VK_QUEUE_FAMILY_IGNORED; }

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

bool Validate(const CoreChecks *device_data, const char *func_name, CMD_BUFFER_STATE *cb_state, const ValidatorState &val,
              const uint32_t src_queue_family, const uint32_t dst_queue_family) {
    bool skip = false;

    const bool mode_concurrent = val.GetSharingMode() == VK_SHARING_MODE_CONCURRENT;
    const bool src_ignored = val.IsIgnored(src_queue_family);
    const bool dst_ignored = val.IsIgnored(dst_queue_family);
    if (val.KhrExternalMem()) {
        if (mode_concurrent) {
            if (!(src_ignored || dst_ignored)) {
                skip |= val.LogMsg(kSrcOrDstMustBeIgnore, src_queue_family, dst_queue_family);
            }
            if ((src_ignored && !(dst_ignored || IsSpecial(dst_queue_family))) ||
                (dst_ignored && !(src_ignored || IsSpecial(src_queue_family)))) {
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
    if (!mode_concurrent && !src_ignored && !dst_ignored) {
        // Only enqueue submit time check if it is needed. If more submit time checks are added, change the criteria
        // TODO create a better named list, or rename the submit time lists to something that matches the broader usage...
        // Note: if we want to create a semantic that separates state lookup, validation, and state update this should go
        // to a local queue of update_state_actions or something.
        cb_state->eventUpdates.emplace_back([device_data, src_queue_family, dst_queue_family, val](VkQueue queue) {
            return ValidatorState::ValidateAtQueueSubmit(queue, device_data, src_queue_family, dst_queue_family, val);
        });
    }
    return skip;
}
}  // namespace barrier_queue_families

// Type specific wrapper for image barriers
bool CoreChecks::ValidateBarrierQueueFamilies(const char *func_name, CMD_BUFFER_STATE *cb_state,
                                              const VkImageMemoryBarrier *barrier, const IMAGE_STATE *state_data) {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the image state
    barrier_queue_families::ValidatorState val(this, func_name, cb_state, barrier, state_data);
    const uint32_t src_queue_family = barrier->srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier->dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, func_name, cb_state, val, src_queue_family, dst_queue_family);
}

// Type specific wrapper for buffer barriers
bool CoreChecks::ValidateBarrierQueueFamilies(const char *func_name, CMD_BUFFER_STATE *cb_state,
                                              const VkBufferMemoryBarrier *barrier, const BUFFER_STATE *state_data) {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the buffer state
    barrier_queue_families::ValidatorState val(this, func_name, cb_state, barrier, state_data);
    const uint32_t src_queue_family = barrier->srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier->dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, func_name, cb_state, val, src_queue_family, dst_queue_family);
}

bool CoreChecks::ValidateBarriers(const char *funcName, CMD_BUFFER_STATE *cb_state, VkPipelineStageFlags src_stage_mask,
                                  VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount,
                                  const VkMemoryBarrier *pMemBarriers, uint32_t bufferBarrierCount,
                                  const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                  const VkImageMemoryBarrier *pImageMemBarriers) {
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
        auto mem_barrier = &pImageMemBarriers[i];
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier->srcAccessMask, src_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184",
                            "%s: pImageMemBarriers[%d].srcAccessMask (0x%X) is not supported by srcStageMask (0x%X).", funcName, i,
                            mem_barrier->srcAccessMask, src_stage_mask);
        }
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier->dstAccessMask, dst_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185",
                            "%s: pImageMemBarriers[%d].dstAccessMask (0x%X) is not supported by dstStageMask (0x%X).", funcName, i,
                            mem_barrier->dstAccessMask, dst_stage_mask);
        }

        auto image_data = GetImageState(mem_barrier->image);
        skip |= ValidateBarrierQueueFamilies(funcName, cb_state, mem_barrier, image_data);

        if (mem_barrier->newLayout == VK_IMAGE_LAYOUT_UNDEFINED || mem_barrier->newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
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

            auto aspect_mask = mem_barrier->subresourceRange.aspectMask;
            skip |= ValidateImageAspectMask(image_data->image, image_data->createInfo.format, aspect_mask, funcName);

            std::string param_name = "pImageMemoryBarriers[" + std::to_string(i) + "].subresourceRange";
            skip |= ValidateImageBarrierSubresourceRange(image_data, mem_barrier->subresourceRange, funcName, param_name.c_str());
        }
    }

    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        auto mem_barrier = &pBufferMemBarriers[i];
        if (!mem_barrier) continue;

        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier->srcAccessMask, src_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184",
                            "%s: pBufferMemBarriers[%d].srcAccessMask (0x%X) is not supported by srcStageMask (0x%X).", funcName, i,
                            mem_barrier->srcAccessMask, src_stage_mask);
        }
        if (!ValidateAccessMaskPipelineStage(device_extensions, mem_barrier->dstAccessMask, dst_stage_mask)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185",
                            "%s: pBufferMemBarriers[%d].dstAccessMask (0x%X) is not supported by dstStageMask (0x%X).", funcName, i,
                            mem_barrier->dstAccessMask, dst_stage_mask);
        }
        // Validate buffer barrier queue family indices
        auto buffer_state = GetBufferState(mem_barrier->buffer);
        skip |= ValidateBarrierQueueFamilies(funcName, cb_state, mem_barrier, buffer_state);

        if (buffer_state) {
            // There is no VUID for this, but there is blanket text:
            //     "Non-sparse resources must be bound completely and contiguously to a single VkDeviceMemory object before
            //     recording commands in a command buffer"
            // TODO: Update this when VUID is defined
            skip |= ValidateMemoryIsBoundToBuffer(buffer_state, funcName, kVUIDUndefined);

            auto buffer_size = buffer_state->createInfo.size;
            if (mem_barrier->offset >= buffer_size) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-VkBufferMemoryBarrier-offset-01187",
                                "%s: Buffer Barrier %s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                                funcName, report_data->FormatHandle(mem_barrier->buffer).c_str(),
                                HandleToUint64(mem_barrier->offset), HandleToUint64(buffer_size));
            } else if (mem_barrier->size != VK_WHOLE_SIZE && (mem_barrier->offset + mem_barrier->size > buffer_size)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), "VUID-VkBufferMemoryBarrier-size-01189",
                            "%s: Buffer Barrier %s has offset 0x%" PRIx64 " and size 0x%" PRIx64
                            " whose sum is greater than total size 0x%" PRIx64 ".",
                            funcName, report_data->FormatHandle(mem_barrier->buffer).c_str(), HandleToUint64(mem_barrier->offset),
                            HandleToUint64(mem_barrier->size), HandleToUint64(buffer_size));
            }
        }
    }

    skip |= ValidateBarriersQFOTransferUniqueness(funcName, cb_state, bufferBarrierCount, pBufferMemBarriers, imageMemBarrierCount,
                                                  pImageMemBarriers);

    return skip;
}

bool CoreChecks::ValidateEventStageMask(VkQueue queue, CMD_BUFFER_STATE *pCB, uint32_t eventCount, size_t firstEventIndex,
                                        VkPipelineStageFlags sourceStageMask) {
    bool skip = false;
    VkPipelineStageFlags stageMask = 0;
    for (uint32_t i = 0; i < eventCount; ++i) {
        auto event = pCB->events[firstEventIndex + i];
        auto queue_data = queueMap.find(queue);
        if (queue_data == queueMap.end()) return false;
        auto event_data = queue_data->second.eventToStageMap.find(event);
        if (event_data != queue_data->second.eventToStageMap.end()) {
            stageMask |= event_data->second;
        } else {
            auto global_event_data = GetEventState(event);
            if (!global_event_data) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,
                                HandleToUint64(event), kVUID_Core_DrawState_InvalidEvent,
                                "%s cannot be waited on if it has never been set.", report_data->FormatHandle(event).c_str());
            } else {
                stageMask |= global_event_data->stageMask;
            }
        }
    }
    // TODO: Need to validate that host_bit is only set if set event is called
    // but set event can be called at any time.
    if (sourceStageMask != stageMask && sourceStageMask != (stageMask | VK_PIPELINE_STAGE_HOST_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
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
                                                  const char *error_code) {
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
BarrierOperationsType CoreChecks::ComputeBarrierOperationsType(CMD_BUFFER_STATE *cb_state, uint32_t buffer_barrier_count,
                                                               const VkBufferMemoryBarrier *buffer_barriers,
                                                               uint32_t image_barrier_count,
                                                               const VkImageMemoryBarrier *image_barriers) {
    auto pool = GetCommandPoolState(cb_state->createInfo.commandPool);
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

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(CMD_BUFFER_STATE const *cb_state,
                                                            VkPipelineStageFlags source_stage_mask,
                                                            VkPipelineStageFlags dest_stage_mask,
                                                            BarrierOperationsType barrier_op_type, const char *function,
                                                            const char *error_code) {
    bool skip = false;
    uint32_t queue_family_index = commandPoolMap[cb_state->createInfo.commandPool].get()->queueFamilyIndex;
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
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
    auto first_event_index = cb_state->events.size();
    for (uint32_t i = 0; i < eventCount; ++i) {
        auto event_state = GetEventState(pEvents[i]);
        if (event_state) {
            AddCommandBufferBinding(&event_state->cb_bindings, VulkanTypedHandle(pEvents[i], kVulkanObjectTypeEvent), cb_state);
            event_state->cb_bindings.insert(cb_state);
        }
        cb_state->waitedEvents.insert(pEvents[i]);
        cb_state->events.push_back(pEvents[i]);
    }
    cb_state->eventUpdates.emplace_back(
        [=](VkQueue q) { return ValidateEventStageMask(q, cb_state, eventCount, first_event_index, sourceStageMask); });
    TransitionImageLayouts(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers);
    if (enabled.gpu_validation) {
        GpuPreCallValidateCmdWaitEvents(sourceStageMask);
    }
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordBarriersQFOTransfers(cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                               pImageMemoryBarriers);
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                   uint32_t bufferMemoryBarrierCount,
                                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                   uint32_t imageMemoryBarrierCount,
                                                   const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

void CoreChecks::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    RecordBarriersQFOTransfers(cb_state, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                               pImageMemoryBarriers);
    TransitionImageLayouts(cb_state, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool CoreChecks::SetQueryState(VkQueue queue, VkCommandBuffer commandBuffer, QueryObject object, QueryState value) {
    CMD_BUFFER_STATE *pCB = GetCBState(commandBuffer);
    if (pCB) {
        pCB->queryToStateMap[object] = value;
    }
    auto queue_data = queueMap.find(queue);
    if (queue_data != queueMap.end()) {
        queue_data->second.queryToStateMap[object] = value;
    }
    return false;
}

bool CoreChecks::ValidateBeginQuery(const CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj, VkFlags flags, CMD_TYPE cmd,
                                    const char *cmd_name, const char *vuid_queue_flags, const char *vuid_queue_feedback,
                                    const char *vuid_queue_occlusion, const char *vuid_precise, const char *vuid_query_count) {
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

void CoreChecks::RecordBeginQuery(CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj) {
    cb_state->activeQueries.insert(query_obj);
    cb_state->startedQueries.insert(query_obj);
    cb_state->queryUpdates.emplace_back([this, cb_state, query_obj](VkQueue q) {
        bool skip = false;
        skip |= VerifyQueryIsReset(q, cb_state->commandBuffer, query_obj);
        skip |= SetQueryState(q, cb_state->commandBuffer, query_obj, QUERYSTATE_RUNNING);
        return skip;
    });
    AddCommandBufferBinding(&GetQueryPoolState(query_obj.pool)->cb_bindings,
                            VulkanTypedHandle(query_obj.pool, kVulkanObjectTypeQueryPool), cb_state);
}

bool CoreChecks::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) {
    if (disabled.query_validation) return false;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    QueryObject query_obj(queryPool, slot);
    return ValidateBeginQuery(cb_state, query_obj, flags, CMD_BEGINQUERY, "vkCmdBeginQuery()",
                              "VUID-vkCmdBeginQuery-commandBuffer-cmdpool", "VUID-vkCmdBeginQuery-queryType-02327",
                              "VUID-vkCmdBeginQuery-queryType-00803", "VUID-vkCmdBeginQuery-queryType-00800",
                              "VUID-vkCmdBeginQuery-query-00802");
}

bool CoreChecks::VerifyQueryIsReset(VkQueue queue, VkCommandBuffer commandBuffer, QueryObject query_obj) {
    bool skip = false;

    auto queue_data = GetQueueState(queue);
    if (!queue_data) return false;

    QueryState state = GetQueryState(queue_data, query_obj.pool, query_obj.query);
    if (state != QUERYSTATE_RESET) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), kVUID_Core_DrawState_QueryNotReset,
                        "vkCmdBeginQuery(): %s and query %" PRIu32
                        ": query not reset. "
                        "After query pool creation, each query must be reset before it is used. "
                        "Queries must also be reset between uses.",
                        report_data->FormatHandle(query_obj.pool).c_str(), query_obj.query);
    }

    return skip;
}

void CoreChecks::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) {
    QueryObject query = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordBeginQuery(cb_state, query);
}

bool CoreChecks::ValidateCmdEndQuery(const CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj, CMD_TYPE cmd,
                                     const char *cmd_name, const char *vuid_queue_flags, const char *vuid_active_queries) {
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

bool CoreChecks::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled.query_validation) return false;
    QueryObject query_obj = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmdEndQuery(cb_state, query_obj, CMD_ENDQUERY, "vkCmdEndQuery()", "VUID-vkCmdEndQuery-commandBuffer-cmdpool",
                               "VUID-vkCmdEndQuery-None-01923");
}

void CoreChecks::RecordCmdEndQuery(CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj) {
    cb_state->activeQueries.erase(query_obj);
    cb_state->queryUpdates.emplace_back([this, cb_state, query_obj](VkQueue q) {
        return SetQueryState(q, cb_state->commandBuffer, query_obj, QUERYSTATE_AVAILABLE);
    });
    AddCommandBufferBinding(&GetQueryPoolState(query_obj.pool)->cb_bindings,
                            VulkanTypedHandle(query_obj.pool, kVulkanObjectTypeQueryPool), cb_state);
}

void CoreChecks::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    QueryObject query_obj = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdEndQuery(cb_state, query_obj);
}

bool CoreChecks::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    if (disabled.query_validation) return false;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    bool skip = InsideRenderPass(cb_state, "vkCmdResetQueryPool()", "VUID-vkCmdResetQueryPool-renderpass");
    skip |= ValidateCmd(cb_state, CMD_RESETQUERYPOOL, "VkCmdResetQueryPool()");
    skip |= ValidateCmdQueueFlags(cb_state, "VkCmdResetQueryPool()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdResetQueryPool-commandBuffer-cmdpool");
    return skip;
}

void CoreChecks::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                 uint32_t queryCount) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    for (uint32_t i = 0; i < queryCount; i++) {
        QueryObject query = {queryPool, firstQuery + i};
        cb_state->queryUpdates.emplace_back(
            [this, commandBuffer, query](VkQueue q) { return SetQueryState(q, commandBuffer, query, QUERYSTATE_RESET); });
    }
    AddCommandBufferBinding(&GetQueryPoolState(queryPool)->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool),
                            cb_state);
}

QueryState CoreChecks::GetQueryState(QUEUE_STATE *queue_data, VkQueryPool queryPool, uint32_t queryIndex) {
    QueryObject query = {queryPool, queryIndex};

    const std::array<decltype(queryToStateMap) *, 2> map_list = {&queue_data->queryToStateMap, &queryToStateMap};

    for (const auto map : map_list) {
        auto query_data = map->find(query);
        if (query_data != map->end()) {
            return query_data->second;
        }
    }
    return QUERYSTATE_UNKNOWN;
}

QueryResultType CoreChecks::GetQueryResultType(QueryState state, VkQueryResultFlags flags) {
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
        case QUERYSTATE_AVAILABLE:
            if ((flags & VK_QUERY_RESULT_WAIT_BIT) || (flags & VK_QUERY_RESULT_PARTIAL_BIT) ||
                (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
                return QUERYRESULT_SOME_DATA;
            } else {
                return QUERYRESULT_MAYBE_NO_DATA;
            }
    }
    assert(false);
    return QUERYRESULT_UNKNOWN;
}

bool CoreChecks::ValidateQuery(VkQueue queue, CMD_BUFFER_STATE *pCB, VkQueryPool queryPool, uint32_t firstQuery,
                               uint32_t queryCount, VkQueryResultFlags flags) {
    bool skip = false;
    auto queue_data = GetQueueState(queue);
    if (!queue_data) return false;
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryState state = GetQueryState(queue_data, queryPool, firstQuery + i);
        QueryResultType result_type = GetQueryResultType(state, flags);
        if (result_type != QUERYRESULT_SOME_DATA) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidQuery,
                            "Requesting a copy from query to buffer on %s query %" PRIu32 ": %s",
                            report_data->FormatHandle(queryPool).c_str(), firstQuery + i, string_QueryResultType(result_type));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags) {
    if (disabled.query_validation) return false;
    auto cb_state = GetCBState(commandBuffer);
    auto dst_buff_state = GetBufferState(dstBuffer);
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

void CoreChecks::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                       VkDeviceSize stride, VkQueryResultFlags flags) {
    auto cb_state = GetCBState(commandBuffer);
    auto dst_buff_state = GetBufferState(dstBuffer);
    AddCommandBufferBindingBuffer(cb_state, dst_buff_state);
    cb_state->queryUpdates.emplace_back([this, cb_state, queryPool, firstQuery, queryCount, flags](VkQueue q) {
        return ValidateQuery(q, cb_state, queryPool, firstQuery, queryCount, flags);
    });
    AddCommandBufferBinding(&GetQueryPoolState(queryPool)->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool),
                            cb_state);
}

bool CoreChecks::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                 const void *pValues) {
    bool skip = false;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
                                                  VkQueryPool queryPool, uint32_t slot) {
    if (disabled.query_validation) return false;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = ValidateCmdQueueFlags(cb_state, "vkCmdWriteTimestamp()",
                                      VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
                                      "VUID-vkCmdWriteTimestamp-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_WRITETIMESTAMP, "vkCmdWriteTimestamp()");
    return skip;
}

void CoreChecks::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                 VkQueryPool queryPool, uint32_t slot) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    QueryObject query = {queryPool, slot};
    cb_state->queryUpdates.emplace_back([this, commandBuffer, query](VkQueue q) {
        bool skip = false;
        skip |= VerifyQueryIsReset(q, commandBuffer, query);
        skip |= SetQueryState(q, commandBuffer, query, QUERYSTATE_AVAILABLE);
        return skip;
    });
    AddCommandBufferBinding(&GetQueryPoolState(queryPool)->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool),
                            cb_state);
}

bool CoreChecks::MatchUsage(uint32_t count, const VkAttachmentReference2KHR *attachments, const VkFramebufferCreateInfo *fbci,
                            VkImageUsageFlagBits usage_flag, const char *error_code) {
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
bool CoreChecks::ValidateFramebufferCreateInfo(const VkFramebufferCreateInfo *pCreateInfo) {
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
                                                                      : "VUID-VkFramebufferCreateInfo-renderPass-03200";
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
                                                  const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) {
    // TODO : Verify that renderPass FB is created with is compatible with FB
    bool skip = false;
    skip |= ValidateFramebufferCreateInfo(pCreateInfo);
    return skip;
}

void CoreChecks::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    // Shadow create info and store in map
    std::unique_ptr<FRAMEBUFFER_STATE> fb_state(
        new FRAMEBUFFER_STATE(*pFramebuffer, pCreateInfo, GetRenderPassStateSharedPtr(pCreateInfo->renderPass)));

    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            VkImageView view = pCreateInfo->pAttachments[i];
            auto view_state = GetImageViewState(view);
            if (!view_state) {
                continue;
            }
        }
    }
    frameBufferMap[*pFramebuffer] = std::move(fb_state);
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

bool CoreChecks::CheckDependencyExists(const uint32_t subpass, const std::vector<uint32_t> &dependent_subpasses,
                                       const std::vector<DAGNode> &subpass_to_node, bool &skip) {
    bool result = true;
    // Loop through all subpasses that share the same attachment and make sure a dependency exists
    for (uint32_t k = 0; k < dependent_subpasses.size(); ++k) {
        if (static_cast<uint32_t>(subpass) == dependent_subpasses[k]) continue;
        const DAGNode &node = subpass_to_node[subpass];
        // Check for a specified dependency between the two nodes. If one exists we are done.
        auto prev_elem = std::find(node.prev.begin(), node.prev.end(), dependent_subpasses[k]);
        auto next_elem = std::find(node.next.begin(), node.next.end(), dependent_subpasses[k]);
        if (prev_elem == node.prev.end() && next_elem == node.next.end()) {
            // If no dependency exits an implicit dependency still might. If not, throw an error.
            std::unordered_set<uint32_t> processed_nodes;
            if (!(FindDependency(subpass, dependent_subpasses[k], subpass_to_node, processed_nodes) ||
                  FindDependency(dependent_subpasses[k], subpass, subpass_to_node, processed_nodes))) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_InvalidRenderpass,
                                "A dependency between subpasses %d and %d must exist but one is not specified.", subpass,
                                dependent_subpasses[k]);
                result = false;
            }
        }
    }
    return result;
}

bool CoreChecks::CheckPreserved(const VkRenderPassCreateInfo2KHR *pCreateInfo, const int index, const uint32_t attachment,
                                const std::vector<DAGNode> &subpass_to_node, int depth, bool &skip) {
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

bool CoreChecks::ValidateDependencies(FRAMEBUFFER_STATE const *framebuffer, RENDER_PASS_STATE const *renderPass) {
    bool skip = false;
    auto const pFramebufferInfo = framebuffer->createInfo.ptr();
    auto const pCreateInfo = renderPass->createInfo.ptr();
    auto const &subpass_to_node = renderPass->subpassToNode;
    std::vector<std::vector<uint32_t>> output_attachment_to_subpass(pCreateInfo->attachmentCount);
    std::vector<std::vector<uint32_t>> input_attachment_to_subpass(pCreateInfo->attachmentCount);
    std::vector<std::vector<uint32_t>> overlapping_attachments(pCreateInfo->attachmentCount);
    // Find overlapping attachments
    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        for (uint32_t j = i + 1; j < pCreateInfo->attachmentCount; ++j) {
            VkImageView viewi = pFramebufferInfo->pAttachments[i];
            VkImageView viewj = pFramebufferInfo->pAttachments[j];
            if (viewi == viewj) {
                overlapping_attachments[i].push_back(j);
                overlapping_attachments[j].push_back(i);
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
                overlapping_attachments[i].push_back(j);
                overlapping_attachments[j].push_back(i);
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
                overlapping_attachments[i].push_back(j);
                overlapping_attachments[j].push_back(i);
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
            input_attachment_to_subpass[attachment].push_back(i);
            for (auto overlapping_attachment : overlapping_attachments[attachment]) {
                input_attachment_to_subpass[overlapping_attachment].push_back(i);
            }
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            output_attachment_to_subpass[attachment].push_back(i);
            for (auto overlapping_attachment : overlapping_attachments[attachment]) {
                output_attachment_to_subpass[overlapping_attachment].push_back(i);
            }
            attachmentIndices.insert(attachment);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            uint32_t attachment = subpass.pDepthStencilAttachment->attachment;
            output_attachment_to_subpass[attachment].push_back(i);
            for (auto overlapping_attachment : overlapping_attachments[attachment]) {
                output_attachment_to_subpass[overlapping_attachment].push_back(i);
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
            CheckDependencyExists(i, output_attachment_to_subpass[attachment], subpass_to_node, skip);
        }
        // If the attachment is an output then all subpasses that use the attachment must have a dependency relationship
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            CheckDependencyExists(i, output_attachment_to_subpass[attachment], subpass_to_node, skip);
            CheckDependencyExists(i, input_attachment_to_subpass[attachment], subpass_to_node, skip);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            const uint32_t &attachment = subpass.pDepthStencilAttachment->attachment;
            CheckDependencyExists(i, output_attachment_to_subpass[attachment], subpass_to_node, skip);
            CheckDependencyExists(i, input_attachment_to_subpass[attachment], subpass_to_node, skip);
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

void CoreChecks::RecordRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                     RENDER_PASS_STATE *render_pass) {
    auto &subpass_to_node = render_pass->subpassToNode;
    subpass_to_node.resize(pCreateInfo->subpassCount);
    auto &self_dependencies = render_pass->self_dependencies;
    self_dependencies.resize(pCreateInfo->subpassCount);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        subpass_to_node[i].pass = i;
        self_dependencies[i].clear();
    }
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const VkSubpassDependency2KHR &dependency = pCreateInfo->pDependencies[i];
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass != VK_SUBPASS_EXTERNAL)) {
            if (dependency.srcSubpass == dependency.dstSubpass) {
                self_dependencies[dependency.srcSubpass].push_back(i);
            } else {
                subpass_to_node[dependency.dstSubpass].prev.push_back(dependency.srcSubpass);
                subpass_to_node[dependency.srcSubpass].next.push_back(dependency.dstSubpass);
            }
        }
    }
}

bool CoreChecks::ValidateRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                       RENDER_PASS_STATE *render_pass) {
    // Shorthand...
    auto &subpass_to_node = render_pass->subpassToNode;
    subpass_to_node.resize(pCreateInfo->subpassCount);
    auto &self_dependencies = render_pass->self_dependencies;
    self_dependencies.resize(pCreateInfo->subpassCount);

    bool skip = false;
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        subpass_to_node[i].pass = i;
        self_dependencies[i].clear();
    }
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const VkSubpassDependency2KHR &dependency = pCreateInfo->pDependencies[i];
        VkPipelineStageFlags exclude_graphics_pipeline_stages =
            ~(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | ExpandPipelineStageFlags(device_extensions, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT));
        VkPipelineStageFlagBits latest_src_stage = GetLogicallyLatestGraphicsPipelineStage(dependency.srcStageMask);
        VkPipelineStageFlagBits earliest_dst_stage = GetLogicallyEarliestGraphicsPipelineStage(dependency.dstStageMask);

        // This VU is actually generalised  to *any* pipeline - not just graphics - but only graphics render passes are
        // currently supported by the spec - so only that pipeline is checked here.
        // If that is ever relaxed, this check should be extended to cover those pipelines.
        if (dependency.srcSubpass == dependency.dstSubpass && (dependency.srcStageMask & exclude_graphics_pipeline_stages) != 0u &&
            (dependency.dstStageMask & exclude_graphics_pipeline_stages) != 0u) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcSubpass-02244" : "VUID-VkSubpassDependency-srcSubpass-01989";
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                "Dependency %u is a self-dependency, but specifies stage masks that contain stages not in the GRAPHICS pipeline.",
                i);
        } else if (dependency.srcSubpass != VK_SUBPASS_EXTERNAL && (dependency.srcStageMask & VK_PIPELINE_STAGE_HOST_BIT)) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-srcSubpass-03078" : "VUID-VkSubpassDependency-srcSubpass-00858";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "Dependency %u specifies a dependency from subpass %u, but includes HOST_BIT in the source stage mask.",
                            i, dependency.srcSubpass);
        } else if (dependency.dstSubpass != VK_SUBPASS_EXTERNAL && (dependency.dstStageMask & VK_PIPELINE_STAGE_HOST_BIT)) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2KHR-dstSubpass-03079" : "VUID-VkSubpassDependency-dstSubpass-00859";
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "Dependency %u specifies a dependency to subpass %u, but includes HOST_BIT in the destination stage mask.",
                        i, dependency.dstSubpass);
        }
        // These next two VUs are actually generalised  to *any* pipeline - not just graphics - but only graphics render passes are
        // currently supported by the spec - so only that pipeline is checked here.
        // If that is ever relaxed, these next two checks should be extended to cover those pipelines.
        else if (dependency.srcSubpass != VK_SUBPASS_EXTERNAL &&
                 pCreateInfo->pSubpasses[dependency.srcSubpass].pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS &&
                 (dependency.srcStageMask & exclude_graphics_pipeline_stages) != 0u) {
            vuid =
                use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054" : "VUID-VkRenderPassCreateInfo-pDependencies-00837";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "Dependency %u specifies a source stage mask that contains stages not in the GRAPHICS pipeline as used "
                            "by the source subpass %u.",
                            i, dependency.srcSubpass);
        } else if (dependency.dstSubpass != VK_SUBPASS_EXTERNAL &&
                   pCreateInfo->pSubpasses[dependency.dstSubpass].pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS &&
                   (dependency.dstStageMask & exclude_graphics_pipeline_stages) != 0u) {
            vuid =
                use_rp2 ? "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03055" : "VUID-VkRenderPassCreateInfo-pDependencies-00838";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                            "Dependency %u specifies a destination stage mask that contains stages not in the GRAPHICS pipeline as "
                            "used by the destination subpass %u.",
                            i, dependency.dstSubpass);
        }
        // The first subpass here serves as a good proxy for "is multiview enabled" - since all view masks need to be non-zero if
        // any are, which enables multiview.
        else if (use_rp2 && (dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) &&
                 (pCreateInfo->pSubpasses[0].viewMask == 0)) {
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
            } else {
                self_dependencies[dependency.srcSubpass].push_back(i);
            }
        } else {
            subpass_to_node[dependency.dstSubpass].prev.push_back(dependency.srcSubpass);
            subpass_to_node[dependency.srcSubpass].next.push_back(dependency.dstSubpass);
        }
    }
    return skip;
}

bool CoreChecks::ValidateAttachmentIndex(RenderPassCreateVersion rp_version, uint32_t attachment, uint32_t attachment_count,
                                         const char *type) {
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
                                  VkImageLayout new_layout) {
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
                                                   const VkRenderPassCreateInfo2KHR *pCreateInfo) {
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
            }

            if (rp_version == RENDER_PASS_VERSION_2) {
                // These are validated automatically as part of parameter validation for create renderpass 1
                // as they are in a struct that only applies to input attachments - not so for v2.

                // Check for 0
                if (attachment_ref.aspectMask == 0) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkSubpassDescription2KHR-aspectMask-03176",
                                    "%s:  Input attachment (%d) aspect mask must not be 0.", function_name, j);
                } else {
                    const VkImageAspectFlags valid_bits =
                        (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT |
                         VK_IMAGE_ASPECT_METADATA_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT |
                         VK_IMAGE_ASPECT_PLANE_2_BIT);

                    // Check for valid aspect mask bits
                    if (attachment_ref.aspectMask & ~valid_bits) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkSubpassDescription2KHR-aspectMask-03175",
                                        "%s:  Input attachment (%d) aspect mask (0x%" PRIx32 ")is invalid.", function_name, j,
                                        attachment_ref.aspectMask);
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

static void MarkAttachmentFirstUse(RENDER_PASS_STATE *render_pass, uint32_t index, bool is_read) {
    if (index == VK_ATTACHMENT_UNUSED) return;

    if (!render_pass->attachment_first_read.count(index)) render_pass->attachment_first_read[index] = is_read;
}

bool CoreChecks::ValidateCreateRenderPass(VkDevice device, RenderPassCreateVersion rp_version,
                                          const VkRenderPassCreateInfo2KHR *pCreateInfo, RENDER_PASS_STATE *render_pass) {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    // TODO: As part of wrapping up the mem_tracker/core_validation merge the following routine should be consolidated with
    //       ValidateLayouts.
    skip |= ValidateRenderpassAttachmentUsage(rp_version, pCreateInfo);

    render_pass->renderPass = VK_NULL_HANDLE;
    skip |= ValidateRenderPassDAG(rp_version, pCreateInfo, render_pass);

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
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
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

    if (!skip) {
        auto render_pass = std::unique_ptr<RENDER_PASS_STATE>(new RENDER_PASS_STATE(pCreateInfo));
        skip |= ValidateCreateRenderPass(device, RENDER_PASS_VERSION_1, render_pass->createInfo.ptr(), render_pass.get());
    }

    return skip;
}

void CoreChecks::RecordCreateRenderPassState(RenderPassCreateVersion rp_version, std::shared_ptr<RENDER_PASS_STATE> &render_pass,
                                             VkRenderPass *pRenderPass) {
    render_pass->renderPass = *pRenderPass;
    auto create_info = render_pass->createInfo.ptr();

    RecordRenderPassDAG(RENDER_PASS_VERSION_1, create_info, render_pass.get());

    for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = create_info->pSubpasses[i];
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            MarkAttachmentFirstUse(render_pass.get(), subpass.pColorAttachments[j].attachment, false);

            // resolve attachments are considered to be written
            if (subpass.pResolveAttachments) {
                MarkAttachmentFirstUse(render_pass.get(), subpass.pResolveAttachments[j].attachment, false);
            }
        }
        if (subpass.pDepthStencilAttachment) {
            MarkAttachmentFirstUse(render_pass.get(), subpass.pDepthStencilAttachment->attachment, false);
        }
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            MarkAttachmentFirstUse(render_pass.get(), subpass.pInputAttachments[j].attachment, true);
        }
    }

    // Even though render_pass is an rvalue-ref parameter, still must move s.t. move assignment is invoked.
    renderPassMap[*pRenderPass] = std::move(render_pass);
}

// Style note:
// Use of rvalue reference exceeds reccommended usage of rvalue refs in google style guide, but intentionally forces caller to move
// or copy.  This is clearer than passing a pointer to shared_ptr and avoids the atomic increment/decrement of shared_ptr copy
// construction or assignment.
void CoreChecks::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    auto render_pass_state = std::make_shared<RENDER_PASS_STATE>(pCreateInfo);
    RecordCreateRenderPassState(RENDER_PASS_VERSION_1, render_pass_state, pRenderPass);
}

void CoreChecks::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                    VkResult result) {
    if (VK_SUCCESS != result) return;
    auto render_pass_state = std::make_shared<RENDER_PASS_STATE>(pCreateInfo);
    RecordCreateRenderPassState(RENDER_PASS_VERSION_2, render_pass_state, pRenderPass);
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
                                                     const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    bool skip = false;

    if (device_extensions.vk_khr_depth_stencil_resolve) {
        skip |= ValidateDepthStencilResolve(report_data, phys_dev_ext_props.depth_stencil_resolve_props, pCreateInfo);
    }

    auto render_pass = std::make_shared<RENDER_PASS_STATE>(pCreateInfo);
    skip |= ValidateCreateRenderPass(device, RENDER_PASS_VERSION_2, render_pass->createInfo.ptr(), render_pass.get());

    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBuffer(const CMD_BUFFER_STATE *pCB, char const *cmd_name, const char *error_code) {
    bool skip = false;
    if (pCB->createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), error_code, "Cannot execute command %s on a secondary command buffer.",
                        cmd_name);
    }
    return skip;
}

bool CoreChecks::VerifyRenderAreaBounds(const VkRenderPassBeginInfo *pRenderPassBegin) {
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

bool CoreChecks::VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo *pRenderPassBeginInfo) {
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
                                            const VkRenderPassBeginInfo *pRenderPassBegin) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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
                                                   VkSubpassContents contents) {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfoKHR *pSubpassBeginInfo) {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

void CoreChecks::RecordCmdBeginRenderPassState(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto render_pass_state = pRenderPassBegin ? GetRenderPassState(pRenderPassBegin->renderPass) : nullptr;
    auto framebuffer = pRenderPassBegin ? GetFramebufferState(pRenderPassBegin->framebuffer) : nullptr;

    if (render_pass_state) {
        cb_state->activeFramebuffer = pRenderPassBegin->framebuffer;
        cb_state->activeRenderPass = render_pass_state;
        // This is a shallow copy as that is all that is needed for now
        cb_state->activeRenderPassBeginInfo = *pRenderPassBegin;
        cb_state->activeSubpass = 0;
        cb_state->activeSubpassContents = contents;
        cb_state->framebuffers.insert(pRenderPassBegin->framebuffer);
        // Connect this framebuffer and its children to this cmdBuffer
        AddFramebufferBinding(cb_state, framebuffer);
        // Connect this RP to cmdBuffer
        AddCommandBufferBinding(&render_pass_state->cb_bindings,
                                VulkanTypedHandle(render_pass_state->renderPass, kVulkanObjectTypeRenderPass), cb_state);
        // transition attachments to the correct layouts for beginning of renderPass and first subpass
        TransitionBeginRenderPassLayouts(cb_state, render_pass_state, framebuffer);

        auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
        if (chained_device_group_struct) {
            cb_state->active_render_pass_device_mask = chained_device_group_struct->deviceMask;
        } else {
            cb_state->active_render_pass_device_mask = cb_state->initial_device_mask;
        }
    }
}

void CoreChecks::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 VkSubpassContents contents) {
    RecordCmdBeginRenderPassState(commandBuffer, pRenderPassBegin, contents);
}

void CoreChecks::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     const VkSubpassBeginInfoKHR *pSubpassBeginInfo) {
    RecordCmdBeginRenderPassState(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

bool CoreChecks::ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

bool CoreChecks::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_1, commandBuffer);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                   const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_2, commandBuffer);
}

void CoreChecks::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->activeSubpass++;
    cb_state->activeSubpassContents = contents;
    TransitionSubpassLayouts(cb_state, cb_state->activeRenderPass, cb_state->activeSubpass,
                             GetFramebufferState(cb_state->activeRenderPassBeginInfo.framebuffer));
}

void CoreChecks::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    RecordCmdNextSubpass(commandBuffer, contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                  const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo->contents);
}

bool CoreChecks::ValidateCmdEndRenderPass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

bool CoreChecks::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    bool skip = ValidateCmdEndRenderPass(RENDER_PASS_VERSION_1, commandBuffer);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    bool skip = ValidateCmdEndRenderPass(RENDER_PASS_VERSION_2, commandBuffer);
    return skip;
}

void CoreChecks::RecordCmdEndRenderPassState(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    FRAMEBUFFER_STATE *framebuffer = GetFramebufferState(cb_state->activeFramebuffer);
    TransitionFinalSubpassLayouts(cb_state, &cb_state->activeRenderPassBeginInfo, framebuffer);
    cb_state->activeRenderPass = nullptr;
    cb_state->activeSubpass = 0;
    cb_state->activeFramebuffer = VK_NULL_HANDLE;
}

void CoreChecks::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) { RecordCmdEndRenderPassState(commandBuffer); }

void CoreChecks::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    RecordCmdEndRenderPassState(commandBuffer);
}

bool CoreChecks::ValidateFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE *pCB, VkCommandBuffer secondaryBuffer,
                                     const CMD_BUFFER_STATE *pSubCB, const char *caller) {
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

bool CoreChecks::ValidateSecondaryCommandBufferState(CMD_BUFFER_STATE *pCB, CMD_BUFFER_STATE *pSubCB) {
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
    auto primary_pool = GetCommandPoolState(pCB->createInfo.commandPool);
    auto secondary_pool = GetCommandPoolState(pSubCB->createInfo.commandPool);
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
                                                   const VkCommandBuffer *pCommandBuffers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    bool skip = false;
    CMD_BUFFER_STATE *sub_cb_state = NULL;
    std::unordered_set<CMD_BUFFER_STATE *> linked_command_buffers = cb_state->linkedCommandBuffers;

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
                auto secondary_rp_state = GetRenderPassState(sub_cb_state->beginInfo.pInheritanceInfo->renderPass);
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
            if (sub_cb_state->in_use.load() || linked_command_buffers.count(sub_cb_state)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), "VUID-vkCmdExecuteCommands-pCommandBuffers-00090",
                                "Attempt to simultaneously execute %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set!",
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

        linked_command_buffers.insert(sub_cb_state);
    }
    skip |= ValidatePrimaryCommandBuffer(cb_state, "vkCmdExecuteCommands()", "VUID-vkCmdExecuteCommands-bufferlevel");
    skip |= ValidateCmdQueueFlags(cb_state, "vkCmdExecuteCommands()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdExecuteCommands-commandBuffer-cmdpool");
    skip |= ValidateCmd(cb_state, CMD_EXECUTECOMMANDS, "vkCmdExecuteCommands()");
    return skip;
}

void CoreChecks::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                 const VkCommandBuffer *pCommandBuffers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    CMD_BUFFER_STATE *sub_cb_state = NULL;
    for (uint32_t i = 0; i < commandBuffersCount; i++) {
        sub_cb_state = GetCBState(pCommandBuffers[i]);
        assert(sub_cb_state);
        if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // TODO: Because this is a state change, clearing the VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT needs to be moved
                // from the validation step to the recording step
                cb_state->beginInfo.flags &= ~VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            }
        }

        // Propagate inital layout and current layout state to the primary cmd buffer
        for (const auto &sub_layout_map_entry : sub_cb_state->image_layout_map) {
            const auto image = sub_layout_map_entry.first;
            const auto *image_state = GetImageState(image);
            if (!image_state) continue;  // Can't set layouts of a dead image

            auto *cb_subres_map = GetImageSubresourceLayoutMap(cb_state, *image_state);
            const auto *sub_cb_subres_map = sub_layout_map_entry.second.get();
            assert(cb_subres_map && sub_cb_subres_map);  // Non const get and map traversal should never be null
            cb_subres_map->UpdateFrom(*sub_cb_subres_map);
        }

        sub_cb_state->primaryCommandBuffer = cb_state->commandBuffer;
        cb_state->linkedCommandBuffers.insert(sub_cb_state);
        sub_cb_state->linkedCommandBuffers.insert(cb_state);
        for (auto &function : sub_cb_state->queryUpdates) {
            cb_state->queryUpdates.push_back(function);
        }
        for (auto &function : sub_cb_state->queue_submit_functions) {
            cb_state->queue_submit_functions.push_back(function);
        }
    }
}

bool CoreChecks::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                          VkFlags flags, void **ppData) {
    bool skip = false;
    DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    if (mem_info) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            skip = log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                           HandleToUint64(mem), "VUID-vkMapMemory-memory-00682",
                           "Mapping Memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set: %s.",
                           report_data->FormatHandle(mem).c_str());
        }
    }
    skip |= ValidateMapMemRange(mem, offset, size);
    return skip;
}

void CoreChecks::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                         void **ppData, VkResult result) {
    if (VK_SUCCESS != result) return;
    // TODO : What's the point of this range? See comment on creating new "bound_range" above, which may replace this
    StoreMemRanges(mem, offset, size);
    InitializeAndTrackMemory(mem, offset, size, ppData);
}

bool CoreChecks::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    bool skip = false;
    auto mem_info = GetDevMemState(mem);
    if (mem_info && !mem_info->mem_range.size) {
        // Valid Usage: memory must currently be mapped
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                        HandleToUint64(mem), "VUID-vkUnmapMemory-memory-00689", "Unmapping Memory without memory being mapped: %s.",
                        report_data->FormatHandle(mem).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    auto mem_info = GetDevMemState(mem);
    mem_info->mem_range.size = 0;
    if (mem_info->shadow_copy) {
        free(mem_info->shadow_copy_base);
        mem_info->shadow_copy_base = 0;
        mem_info->shadow_copy = 0;
    }
}

bool CoreChecks::ValidateMemoryIsMapped(const char *funcName, uint32_t memRangeCount, const VkMappedMemoryRange *pMemRanges) {
    bool skip = false;
    for (uint32_t i = 0; i < memRangeCount; ++i) {
        auto mem_info = GetDevMemState(pMemRanges[i].memory);
        if (mem_info) {
            if (pMemRanges[i].size == VK_WHOLE_SIZE) {
                if (mem_info->mem_range.offset > pMemRanges[i].offset) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                    HandleToUint64(pMemRanges[i].memory), "VUID-VkMappedMemoryRange-size-00686",
                                    "%s: Flush/Invalidate offset (" PRINTF_SIZE_T_SPECIFIER
                                    ") is less than Memory Object's offset (" PRINTF_SIZE_T_SPECIFIER ").",
                                    funcName, static_cast<size_t>(pMemRanges[i].offset),
                                    static_cast<size_t>(mem_info->mem_range.offset));
                }
            } else {
                const uint64_t data_end = (mem_info->mem_range.size == VK_WHOLE_SIZE)
                                              ? mem_info->alloc_info.allocationSize
                                              : (mem_info->mem_range.offset + mem_info->mem_range.size);
                if ((mem_info->mem_range.offset > pMemRanges[i].offset) ||
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

bool CoreChecks::ValidateAndCopyNoncoherentMemoryToDriver(uint32_t mem_range_count, const VkMappedMemoryRange *mem_ranges) {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        auto mem_info = GetDevMemState(mem_ranges[i].memory);
        if (mem_info) {
            if (mem_info->shadow_copy) {
                VkDeviceSize size = (mem_info->mem_range.size != VK_WHOLE_SIZE)
                                        ? mem_info->mem_range.size
                                        : (mem_info->alloc_info.allocationSize - mem_info->mem_range.offset);
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
            VkDeviceSize size = (mem_info->mem_range.size != VK_WHOLE_SIZE)
                                    ? mem_info->mem_range.size
                                    : (mem_info->alloc_info.allocationSize - mem_ranges[i].offset);
            char *data = static_cast<char *>(mem_info->shadow_copy);
            memcpy(data + mem_info->shadow_pad_size, mem_info->p_driver_data, (size_t)(size));
        }
    }
}

bool CoreChecks::ValidateMappedMemoryRangeDeviceLimits(const char *func_name, uint32_t mem_range_count,
                                                       const VkMappedMemoryRange *mem_ranges) {
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
    return skip;
}

bool CoreChecks::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                        const VkMappedMemoryRange *pMemRanges) {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    skip |= ValidateAndCopyNoncoherentMemoryToDriver(memRangeCount, pMemRanges);
    skip |= ValidateMemoryIsMapped("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    return skip;
}

bool CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                             const VkMappedMemoryRange *pMemRanges) {
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

bool CoreChecks::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize *pCommittedMem) {
    bool skip = false;
    auto mem_info = GetDevMemState(mem);

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

bool CoreChecks::ValidateBindImageMemory(const VkBindImageMemoryInfo &bindInfo, const char *api_name) {
    bool skip = false;
    IMAGE_STATE *image_state = GetImageState(bindInfo.image);
    if (image_state) {
        // Track objects tied to memory
        uint64_t image_handle = HandleToUint64(bindInfo.image);
        skip = ValidateSetMemBinding(bindInfo.memory, VulkanTypedHandle(bindInfo.image, kVulkanObjectTypeImage), api_name);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        if (image_state->external_format_android) {
            if (image_state->memory_requirements_checked) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                kVUID_Core_DrawState_InvalidImage,
                                "%s: Must not call vkGetImageMemoryRequirements on %s that will be bound to an external "
                                "Android hardware buffer.",
                                api_name, report_data->FormatHandle(bindInfo.image).c_str());
            }
            return skip;
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
        if (!image_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetImageMemoryRequirements() prior to calling
            // BindImageMemory but it's implied in that memory being bound must conform with VkMemoryRequirements from
            // vkGetImageMemoryRequirements()
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            kVUID_Core_DrawState_InvalidImage,
                            "%s: Binding memory to %s but vkGetImageMemoryRequirements() has not been called on that image.",
                            api_name, report_data->FormatHandle(bindInfo.image).c_str());
            // Make the call for them so we can verify the state
            DispatchGetImageMemoryRequirements(device, bindInfo.image, &image_state->requirements);
        }

        // Validate bound memory range information
        auto mem_info = GetDevMemState(bindInfo.memory);
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
            if (mem_info->is_dedicated && ((mem_info->dedicated_image != bindInfo.image) || (bindInfo.memoryOffset != 0))) {
                // TODO: Add vkBindImageMemory2KHR error message when added to spec.
                auto validation_error = kVUIDUndefined;
                if (strcmp(api_name, "vkBindImageMemory()") == 0) {
                    validation_error = "VUID-vkBindImageMemory-memory-01509";
                }
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                validation_error,
                                "%s: for dedicated memory allocation %s, VkMemoryDedicatedAllocateInfoKHR:: %s must be equal "
                                "to %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                api_name, report_data->FormatHandle(bindInfo.memory).c_str(),
                                report_data->FormatHandle(mem_info->dedicated_image).c_str(),
                                report_data->FormatHandle(bindInfo.image).c_str(), bindInfo.memoryOffset);
            }
        }

        const auto swapchain_info = lvl_find_in_chain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            if (bindInfo.memory != VK_NULL_HANDLE) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                "VUID-VkBindImageMemoryInfo-pNext-01631", "%s: %s is not VK_NULL_HANDLE.", api_name,
                                report_data->FormatHandle(bindInfo.memory).c_str());
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

bool CoreChecks::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    VkBindImageMemoryInfo bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindInfo.image = image;
    bindInfo.memory = mem;
    bindInfo.memoryOffset = memoryOffset;
    return ValidateBindImageMemory(bindInfo, "vkBindImageMemory()");
}

void CoreChecks::UpdateBindImageMemoryState(const VkBindImageMemoryInfo &bindInfo) {
    IMAGE_STATE *image_state = GetImageState(bindInfo.image);
    if (image_state) {
        const auto swapchain_info = lvl_find_in_chain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            image_state->bind_swapchain = swapchain_info->swapchain;
            image_state->bind_swapchain_imageIndex = swapchain_info->imageIndex;
        } else {
            // Track bound memory range information
            auto mem_info = GetDevMemState(bindInfo.memory);
            if (mem_info) {
                InsertImageMemoryRange(bindInfo.image, mem_info, bindInfo.memoryOffset, image_state->requirements,
                                       image_state->createInfo.tiling == VK_IMAGE_TILING_LINEAR);
            }

            // Track objects tied to memory
            SetMemBinding(bindInfo.memory, image_state, bindInfo.memoryOffset,
                          VulkanTypedHandle(bindInfo.image, kVulkanObjectTypeImage));
        }
    }
}

void CoreChecks::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                               VkResult result) {
    if (VK_SUCCESS != result) return;
    VkBindImageMemoryInfo bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindInfo.image = image;
    bindInfo.memory = mem;
    bindInfo.memoryOffset = memoryOffset;
    UpdateBindImageMemoryState(bindInfo);
}

bool CoreChecks::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindImageMemoryInfoKHR *pBindInfos) {
    bool skip = false;
    char api_name[128];
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i], api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfoKHR *pBindInfos) {
    bool skip = false;
    char api_name[128];
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i], api_name);
    }
    return skip;
}

void CoreChecks::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR *pBindInfos,
                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void CoreChecks::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindImageMemoryInfoKHR *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event) {
    bool skip = false;
    auto event_state = GetEventState(event);
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

void CoreChecks::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    auto event_state = GetEventState(event);
    if (event_state) {
        event_state->stageMask = VK_PIPELINE_STAGE_HOST_BIT;
    }
    // Host setting event is visible to all queues immediately so update stageMask for any queue that's seen this event
    // TODO : For correctness this needs separate fix to verify that app doesn't make incorrect assumptions about the
    // ordering of this command in relation to vkCmd[Set|Reset]Events (see GH297)
    for (auto queue_data : queueMap) {
        auto event_entry = queue_data.second.eventToStageMap.find(event);
        if (event_entry != queue_data.second.eventToStageMap.end()) {
            event_entry->second |= VK_PIPELINE_STAGE_HOST_BIT;
        }
    }
}

bool CoreChecks::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                VkFence fence) {
    auto pFence = GetFenceState(fence);
    bool skip = ValidateFenceForSubmit(pFence);
    if (skip) {
        return true;
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
            auto pSemaphore = GetSemaphoreState(semaphore);
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
            auto pSemaphore = GetSemaphoreState(semaphore);
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
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        std::unordered_set<IMAGE_STATE *> sparse_images;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bindInfo.imageBindCount; ++i) {
            const auto &image_bind = bindInfo.pImageBinds[i];
            auto image_state = GetImageState(image_bind.image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    return log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                   HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                   "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                return log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                               HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                               "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                               "vkGetImageMemoryRequirements() to retrieve requirements.",
                               report_data->FormatHandle(image_state->image).c_str());
            }
        }
        for (uint32_t i = 0; i < bindInfo.imageOpaqueBindCount; ++i) {
            const auto &image_opaque_bind = bindInfo.pImageOpaqueBinds[i];
            auto image_state = GetImageState(bindInfo.pImageOpaqueBinds[i].image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    return log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                   HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                   "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                return log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                               HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                               "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                               "vkGetImageMemoryRequirements() to retrieve requirements.",
                               report_data->FormatHandle(image_state->image).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
        for (const auto &sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                return log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                               HandleToUint64(sparse_image_state->image), kVUID_Core_MemTrack_InvalidState,
                               "vkQueueBindSparse(): Binding sparse memory to %s which requires a metadata aspect but no "
                               "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                               report_data->FormatHandle(sparse_image_state->image).c_str());
            }
        }
    }

    return skip;
}
void CoreChecks::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                               VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) return;
    uint64_t early_retire_seq = 0;
    auto pFence = GetFenceState(fence);
    auto pQueue = GetQueueState(queue);

    if (pFence) {
        if (pFence->scope == kSyncScopeInternal) {
            SubmitFence(pQueue, pFence, std::max(1u, bindInfoCount));
            if (!bindInfoCount) {
                // No work to do, just dropping a fence in the queue by itself.
                pQueue->submissions.emplace_back(std::vector<VkCommandBuffer>(), std::vector<SEMAPHORE_WAIT>(),
                                                 std::vector<VkSemaphore>(), std::vector<VkSemaphore>(), fence);
            }
        } else {
            // Retire work up until this fence early, we will not see the wait that corresponds to this signal
            early_retire_seq = pQueue->seq + pQueue->submissions.size();
            if (!external_sync_warning) {
                external_sync_warning = true;
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, HandleToUint64(fence),
                        kVUID_Core_DrawState_QueueForwardProgress,
                        "vkQueueBindSparse(): Signaling external %s on %s will disable validation of preceding command "
                        "buffer lifecycle states and the in-use status of associated objects.",
                        report_data->FormatHandle(fence).c_str(), report_data->FormatHandle(queue).c_str());
            }
        }
    }

    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; ++bindIdx) {
        const VkBindSparseInfo &bindInfo = pBindInfo[bindIdx];
        // Track objects tied to memory
        for (uint32_t j = 0; j < bindInfo.bufferBindCount; j++) {
            for (uint32_t k = 0; k < bindInfo.pBufferBinds[j].bindCount; k++) {
                auto sparse_binding = bindInfo.pBufferBinds[j].pBinds[k];
                SetSparseMemBinding({sparse_binding.memory, sparse_binding.memoryOffset, sparse_binding.size},
                                    VulkanTypedHandle(bindInfo.pBufferBinds[j].buffer, kVulkanObjectTypeBuffer));
            }
        }
        for (uint32_t j = 0; j < bindInfo.imageOpaqueBindCount; j++) {
            for (uint32_t k = 0; k < bindInfo.pImageOpaqueBinds[j].bindCount; k++) {
                auto sparse_binding = bindInfo.pImageOpaqueBinds[j].pBinds[k];
                SetSparseMemBinding({sparse_binding.memory, sparse_binding.memoryOffset, sparse_binding.size},
                                    VulkanTypedHandle(bindInfo.pImageOpaqueBinds[j].image, kVulkanObjectTypeImage));
            }
        }
        for (uint32_t j = 0; j < bindInfo.imageBindCount; j++) {
            for (uint32_t k = 0; k < bindInfo.pImageBinds[j].bindCount; k++) {
                auto sparse_binding = bindInfo.pImageBinds[j].pBinds[k];
                // TODO: This size is broken for non-opaque bindings, need to update to comprehend full sparse binding data
                VkDeviceSize size = sparse_binding.extent.depth * sparse_binding.extent.height * sparse_binding.extent.width * 4;
                SetSparseMemBinding({sparse_binding.memory, sparse_binding.memoryOffset, size},
                                    VulkanTypedHandle(bindInfo.pImageBinds[j].image, kVulkanObjectTypeImage));
            }
        }

        std::vector<SEMAPHORE_WAIT> semaphore_waits;
        std::vector<VkSemaphore> semaphore_signals;
        std::vector<VkSemaphore> semaphore_externals;
        for (uint32_t i = 0; i < bindInfo.waitSemaphoreCount; ++i) {
            VkSemaphore semaphore = bindInfo.pWaitSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                if (pSemaphore->scope == kSyncScopeInternal) {
                    if (pSemaphore->signaler.first != VK_NULL_HANDLE) {
                        semaphore_waits.push_back({semaphore, pSemaphore->signaler.first, pSemaphore->signaler.second});
                        pSemaphore->in_use.fetch_add(1);
                    }
                    pSemaphore->signaler.first = VK_NULL_HANDLE;
                    pSemaphore->signaled = false;
                } else {
                    semaphore_externals.push_back(semaphore);
                    pSemaphore->in_use.fetch_add(1);
                    if (pSemaphore->scope == kSyncScopeExternalTemporary) {
                        pSemaphore->scope = kSyncScopeInternal;
                    }
                }
            }
        }
        for (uint32_t i = 0; i < bindInfo.signalSemaphoreCount; ++i) {
            VkSemaphore semaphore = bindInfo.pSignalSemaphores[i];
            auto pSemaphore = GetSemaphoreState(semaphore);
            if (pSemaphore) {
                if (pSemaphore->scope == kSyncScopeInternal) {
                    pSemaphore->signaler.first = queue;
                    pSemaphore->signaler.second = pQueue->seq + pQueue->submissions.size() + 1;
                    pSemaphore->signaled = true;
                    pSemaphore->in_use.fetch_add(1);
                    semaphore_signals.push_back(semaphore);
                } else {
                    // Retire work up until this submit early, we will not see the wait that corresponds to this signal
                    early_retire_seq = std::max(early_retire_seq, pQueue->seq + pQueue->submissions.size() + 1);
                    if (!external_sync_warning) {
                        external_sync_warning = true;
                        log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
                                HandleToUint64(semaphore), kVUID_Core_DrawState_QueueForwardProgress,
                                "vkQueueBindSparse(): Signaling external %s on %s will disable validation of "
                                "preceding command buffer lifecycle states and the in-use status of associated objects.",
                                report_data->FormatHandle(semaphore).c_str(), report_data->FormatHandle(queue).c_str());
                    }
                }
            }
        }

        pQueue->submissions.emplace_back(std::vector<VkCommandBuffer>(), semaphore_waits, semaphore_signals, semaphore_externals,
                                         bindIdx == bindInfoCount - 1 ? fence : (VkFence)VK_NULL_HANDLE);
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(pQueue, early_retire_seq);
    }
}

void CoreChecks::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore, VkResult result) {
    if (VK_SUCCESS != result) return;
    std::unique_ptr<SEMAPHORE_STATE> semaphore_state(new SEMAPHORE_STATE{});
    semaphore_state->signaler.first = VK_NULL_HANDLE;
    semaphore_state->signaler.second = 0;
    semaphore_state->signaled = false;
    semaphore_state->scope = kSyncScopeInternal;
    semaphoreMap[*pSemaphore] = std::move(semaphore_state);
}

bool CoreChecks::ValidateImportSemaphore(VkSemaphore semaphore, const char *caller_name) {
    bool skip = false;
    SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    if (sema_node) {
        const VulkanTypedHandle obj_struct(semaphore, kVulkanObjectTypeSemaphore);
        skip |= ValidateObjectNotInUse(sema_node, obj_struct, caller_name, kVUIDUndefined);
    }
    return skip;
}

void CoreChecks::RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type,
                                            VkSemaphoreImportFlagsKHR flags) {
    SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    if (sema_node && sema_node->scope != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR || flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR) &&
            sema_node->scope == kSyncScopeInternal) {
            sema_node->scope = kSyncScopeExternalTemporary;
        } else {
            sema_node->scope = kSyncScopeExternalPermanent;
        }
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo) {
    return ValidateImportSemaphore(pImportSemaphoreWin32HandleInfo->semaphore, "vkImportSemaphoreWin32HandleKHR");
}

void CoreChecks::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreWin32HandleInfo->semaphore, pImportSemaphoreWin32HandleInfo->handleType,
                               pImportSemaphoreWin32HandleInfo->flags);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) {
    return ValidateImportSemaphore(pImportSemaphoreFdInfo->semaphore, "vkImportSemaphoreFdKHR");
}

void CoreChecks::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                    VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreFdInfo->semaphore, pImportSemaphoreFdInfo->handleType,
                               pImportSemaphoreFdInfo->flags);
}

void CoreChecks::RecordGetExternalSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type) {
    SEMAPHORE_STATE *semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state && handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        semaphore_state->scope = kSyncScopeExternalPermanent;
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void CoreChecks::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                          HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetWin32HandleInfo->semaphore, pGetWin32HandleInfo->handleType);
}
#endif

void CoreChecks::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetFdInfo->semaphore, pGetFdInfo->handleType);
}

bool CoreChecks::ValidateImportFence(VkFence fence, const char *caller_name) {
    FENCE_STATE *fence_node = GetFenceState(fence);
    bool skip = false;
    if (fence_node && fence_node->scope == kSyncScopeInternal && fence_node->state == FENCE_INFLIGHT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, HandleToUint64(fence),
                        kVUIDUndefined, "Cannot call %s on %s that is currently in use.", caller_name,
                        report_data->FormatHandle(fence).c_str());
    }
    return skip;
}

void CoreChecks::RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type,
                                        VkFenceImportFlagsKHR flags) {
    FENCE_STATE *fence_node = GetFenceState(fence);
    if (fence_node && fence_node->scope != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR || flags & VK_FENCE_IMPORT_TEMPORARY_BIT_KHR) &&
            fence_node->scope == kSyncScopeInternal) {
            fence_node->scope = kSyncScopeExternalTemporary;
        } else {
            fence_node->scope = kSyncScopeExternalPermanent;
        }
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                          const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo) {
    return ValidateImportFence(pImportFenceWin32HandleInfo->fence, "vkImportFenceWin32HandleKHR");
}
void CoreChecks::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo,
                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceWin32HandleInfo->fence, pImportFenceWin32HandleInfo->handleType,
                           pImportFenceWin32HandleInfo->flags);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo) {
    return ValidateImportFence(pImportFenceFdInfo->fence, "vkImportFenceFdKHR");
}
void CoreChecks::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void CoreChecks::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type) {
    FENCE_STATE *fence_state = GetFenceState(fence);
    if (fence_state) {
        if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
            // Export with reference transference becomes external
            fence_state->scope = kSyncScopeExternalPermanent;
        } else if (fence_state->scope == kSyncScopeInternal) {
            // Export with copy transference has a side effect of resetting the fence
            fence_state->state = FENCE_UNSIGNALED;
        }
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void CoreChecks::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetWin32HandleInfo->fence, pGetWin32HandleInfo->handleType);
}
#endif

void CoreChecks::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetFdInfo->fence, pGetFdInfo->handleType);
}

void CoreChecks::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, VkResult result) {
    if (VK_SUCCESS != result) return;
    eventMap[*pEvent].write_in_use = 0;
    eventMap[*pEvent].stageMask = VkPipelineStageFlags(0);
}

bool CoreChecks::ValidateCreateSwapchain(const char *func_name, VkSwapchainCreateInfoKHR const *pCreateInfo,
                                         SURFACE_STATE *surface_state, SWAPCHAIN_NODE *old_swapchain_state) {
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

    if (physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState == UNCALLED) {
        if (log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(physical_device), kVUID_Core_DrawState_SwapchainCreateBeforeQuery,
                    "%s: surface capabilities not retrieved for this physical device", func_name))
            return true;
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
    auto surface_formats_ref = &surface_formats;

    // Validate pCreateInfo values with the results of vkGetPhysicalDeviceSurfaceFormatsKHR():
    if (physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
        if (log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                    kVUID_Core_DrawState_SwapchainCreateBeforeQuery,
                    "%s called before getting format(s) from vkGetPhysicalDeviceSurfaceFormatsKHR().", func_name)) {
            return true;
        }
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

    // Validate pCreateInfo values with the results of vkGetPhysicalDeviceSurfacePresentModesKHR():
    if (physical_device_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS) {
        // FIFO is required to always be supported
        if (pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR) {
            if (log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                        HandleToUint64(device), kVUID_Core_DrawState_SwapchainCreateBeforeQuery,
                        "%s called before getting present mode(s) from vkGetPhysicalDeviceSurfacePresentModesKHR().", func_name))
                return true;
        }
    } else {
        // Validate pCreateInfo->presentMode against vkGetPhysicalDeviceSurfacePresentModesKHR():
        bool foundMatch = std::find(physical_device_state->present_modes.begin(), physical_device_state->present_modes.end(),
                                    pCreateInfo->presentMode) != physical_device_state->present_modes.end();
        if (!foundMatch) {
            if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-VkSwapchainCreateInfoKHR-presentMode-01281", "%s called with a non-supported presentMode (i.e. %s).",
                        func_name, string_VkPresentModeKHR(pCreateInfo->presentMode)))
                return true;
        }
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
                                                   const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) {
    auto surface_state = GetSurfaceState(pCreateInfo->surface);
    auto old_swapchain_state = GetSwapchainState(pCreateInfo->oldSwapchain);
    return ValidateCreateSwapchain("vkCreateSwapchainKHR()", pCreateInfo, surface_state, old_swapchain_state);
}

void CoreChecks::RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                            VkSwapchainKHR *pSwapchain, SURFACE_STATE *surface_state,
                                            SWAPCHAIN_NODE *old_swapchain_state) {
    if (VK_SUCCESS == result) {
        auto swapchain_state = unique_ptr<SWAPCHAIN_NODE>(new SWAPCHAIN_NODE(pCreateInfo, *pSwapchain));
        if (VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR == pCreateInfo->presentMode ||
            VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR == pCreateInfo->presentMode) {
            swapchain_state->shared_presentable = true;
        }
        surface_state->swapchain = swapchain_state.get();
        swapchainMap[*pSwapchain] = std::move(swapchain_state);
    } else {
        surface_state->swapchain = nullptr;
    }
    // Spec requires that even if CreateSwapchainKHR fails, oldSwapchain is retired
    if (old_swapchain_state) {
        old_swapchain_state->retired = true;
    }
    return;
}

void CoreChecks::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                  VkResult result) {
    auto surface_state = GetSurfaceState(pCreateInfo->surface);
    auto old_swapchain_state = GetSwapchainState(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(result, pCreateInfo, pSwapchain, surface_state, old_swapchain_state);
}

void CoreChecks::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  const VkAllocationCallbacks *pAllocator) {
    if (!swapchain) return;
    auto swapchain_data = GetSwapchainState(swapchain);
    if (swapchain_data) {
        if (swapchain_data->images.size() > 0) {
            for (auto swapchain_image : swapchain_data->images) {
                auto image_sub = imageSubresourceMap.find(swapchain_image);
                if (image_sub != imageSubresourceMap.end()) {
                    for (auto imgsubpair : image_sub->second) {
                        auto image_item = imageLayoutMap.find(imgsubpair);
                        if (image_item != imageLayoutMap.end()) {
                            imageLayoutMap.erase(image_item);
                        }
                    }
                    imageSubresourceMap.erase(image_sub);
                }
                ClearMemoryObjectBindings(VulkanTypedHandle(swapchain_image, kVulkanObjectTypeImage));
                EraseQFOImageRelaseBarriers(swapchain_image);
                imageMap.erase(swapchain_image);
            }
        }

        auto surface_state = GetSurfaceState(swapchain_data->createInfo.surface);
        if (surface_state) {
            if (surface_state->swapchain == swapchain_data) surface_state->swapchain = nullptr;
        }

        swapchainMap.erase(swapchain);
    }
}

bool CoreChecks::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                      VkImage *pSwapchainImages) {
    auto swapchain_state = GetSwapchainState(swapchain);
    bool skip = false;
    if (swapchain_state && pSwapchainImages) {
        // Compare the preliminary value of *pSwapchainImageCount with the value this time:
        if (swapchain_state->vkGetSwapchainImagesKHRState == UNCALLED) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), kVUID_Core_Swapchain_PriorCount,
                            "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount; but no prior positive value has "
                            "been seen for pSwapchainImages.");
        } else if (*pSwapchainImageCount > swapchain_state->get_swapchain_image_count) {
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
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    auto swapchain_state = GetSwapchainState(swapchain);

    if (*pSwapchainImageCount > swapchain_state->images.size()) swapchain_state->images.resize(*pSwapchainImageCount);

    if (pSwapchainImages) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_DETAILS) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_DETAILS;
        }
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            if (swapchain_state->images[i] != VK_NULL_HANDLE) continue;  // Already retrieved this.

            IMAGE_LAYOUT_STATE image_layout_node;
            image_layout_node.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_layout_node.format = swapchain_state->createInfo.imageFormat;
            // Add imageMap entries for each swapchain image
            VkImageCreateInfo image_ci = {};
            image_ci.flags = 0;
            image_ci.imageType = VK_IMAGE_TYPE_2D;
            image_ci.format = swapchain_state->createInfo.imageFormat;
            image_ci.extent.width = swapchain_state->createInfo.imageExtent.width;
            image_ci.extent.height = swapchain_state->createInfo.imageExtent.height;
            image_ci.extent.depth = 1;
            image_ci.mipLevels = 1;
            image_ci.arrayLayers = swapchain_state->createInfo.imageArrayLayers;
            image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
            image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_ci.usage = swapchain_state->createInfo.imageUsage;
            image_ci.sharingMode = swapchain_state->createInfo.imageSharingMode;
            imageMap[pSwapchainImages[i]] = unique_ptr<IMAGE_STATE>(new IMAGE_STATE(pSwapchainImages[i], &image_ci));
            auto &image_state = imageMap[pSwapchainImages[i]];
            image_state->valid = false;
            image_state->binding.mem = MEMTRACKER_SWAP_CHAIN_IMAGE_KEY;
            swapchain_state->images[i] = pSwapchainImages[i];
            ImageSubresourcePair subpair = {pSwapchainImages[i], false, VkImageSubresource()};
            imageSubresourceMap[pSwapchainImages[i]].push_back(subpair);
            imageLayoutMap[subpair] = image_layout_node;
        }
    }

    if (*pSwapchainImageCount) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_COUNT) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_COUNT;
        }
        swapchain_state->get_swapchain_image_count = *pSwapchainImageCount;
    }
}

bool CoreChecks::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    bool skip = false;
    auto queue_state = GetQueueState(queue);

    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        auto pSemaphore = GetSemaphoreState(pPresentInfo->pWaitSemaphores[i]);
        if (pSemaphore && !pSemaphore->signaled) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                            kVUID_Core_DrawState_QueueForwardProgress, "%s is waiting on %s that has no way to be signaled.",
                            report_data->FormatHandle(queue).c_str(),
                            report_data->FormatHandle(pPresentInfo->pWaitSemaphores[i]).c_str());
        }
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
        if (swapchain_data) {
            if (pPresentInfo->pImageIndices[i] >= swapchain_data->images.size()) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(pPresentInfo->pSwapchains[i]), kVUID_Core_DrawState_SwapchainInvalidImage,
                            "vkQueuePresentKHR: Swapchain image index too large (%u). There are only %u images in this swapchain.",
                            pPresentInfo->pImageIndices[i], (uint32_t)swapchain_data->images.size());
            } else {
                auto image = swapchain_data->images[pPresentInfo->pImageIndices[i]];
                auto image_state = GetImageState(image);

                if (image_state->shared_presentable) {
                    image_state->layout_locked = true;
                }

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
                auto surface_state = GetSurfaceState(swapchain_data->createInfo.surface);
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
    if (pPresentInfo && pPresentInfo->pNext) {
        // Verify ext struct
        const auto *present_regions = lvl_find_in_chain<VkPresentRegionsKHR>(pPresentInfo->pNext);
        if (present_regions) {
            for (uint32_t i = 0; i < present_regions->swapchainCount; ++i) {
                auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
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

void CoreChecks::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) {
    // Semaphore waits occur before error generation, if the call reached the ICD. (Confirm?)
    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        auto pSemaphore = GetSemaphoreState(pPresentInfo->pWaitSemaphores[i]);
        if (pSemaphore) {
            pSemaphore->signaler.first = VK_NULL_HANDLE;
            pSemaphore->signaled = false;
        }
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        // Note: this is imperfect, in that we can get confused about what did or didn't succeed-- but if the app does that, it's
        // confused itself just as much.
        auto local_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (local_result != VK_SUCCESS && local_result != VK_SUBOPTIMAL_KHR) continue;  // this present didn't actually happen.
        // Mark the image as having been released to the WSI
        auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
        if (swapchain_data && (swapchain_data->images.size() > pPresentInfo->pImageIndices[i])) {
            auto image = swapchain_data->images[pPresentInfo->pImageIndices[i]];
            auto image_state = GetImageState(image);
            if (image_state) {
                image_state->acquired = false;
            }
        }
    }
    // Note: even though presentation is directed to a queue, there is no direct ordering between QP and subsequent work, so QP (and
    // its semaphore waits) /never/ participate in any completion proof.
}

bool CoreChecks::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains) {
    bool skip = false;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = GetSurfaceState(pCreateInfos[i].surface);
            auto old_swapchain_state = GetSwapchainState(pCreateInfos[i].oldSwapchain);
            std::stringstream func_name;
            func_name << "vkCreateSharedSwapchainsKHR[" << swapchainCount << "]()";
            skip |= ValidateCreateSwapchain(func_name.str().c_str(), &pCreateInfos[i], surface_state, old_swapchain_state);
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                         VkResult result) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = GetSurfaceState(pCreateInfos[i].surface);
            auto old_swapchain_state = GetSwapchainState(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(result, &pCreateInfos[i], &pSwapchains[i], surface_state, old_swapchain_state);
        }
    }
}

bool CoreChecks::ValidateAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                          VkFence fence, uint32_t *pImageIndex, const char *func_name) {
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

    auto swapchain_data = GetSwapchainState(swapchain);
    if (swapchain_data && swapchain_data->retired) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                        HandleToUint64(swapchain), "VUID-vkAcquireNextImageKHR-swapchain-01285",
                        "%s: This swapchain has been retired. The application can still present any images it "
                        "has acquired, but cannot acquire any more.",
                        func_name);
    }

    auto physical_device_state = GetPhysicalDeviceState();
    if (physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState != UNCALLED) {
        uint64_t acquired_images = std::count_if(swapchain_data->images.begin(), swapchain_data->images.end(),
                                                 [=](VkImage image) { return GetImageState(image)->acquired; });
        if (acquired_images > swapchain_data->images.size() - physical_device_state->surfaceCapabilities.minImageCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                            HandleToUint64(swapchain), kVUID_Core_DrawState_SwapchainTooManyImages,
                            "%s: Application has already acquired the maximum number of images (0x%" PRIxLEAST64 ")", func_name,
                            acquired_images);
        }
    }

    if (swapchain_data && swapchain_data->images.size() == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
                        HandleToUint64(swapchain), kVUID_Core_DrawState_SwapchainImagesNotFound,
                        "%s: No images found to acquire from. Application probably did not call "
                        "vkGetSwapchainImagesKHR after swapchain creation.",
                        func_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
    return ValidateAcquireNextImage(device, swapchain, timeout, semaphore, fence, pImageIndex, "vkAcquireNextImageKHR");
}

bool CoreChecks::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                     uint32_t *pImageIndex) {
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

void CoreChecks::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                             VkFence fence, uint32_t *pImageIndex) {
    auto pFence = GetFenceState(fence);
    if (pFence && pFence->scope == kSyncScopeInternal) {
        // Treat as inflight since it is valid to wait on this fence, even in cases where it is technically a temporary
        // import
        pFence->state = FENCE_INFLIGHT;
        pFence->signaler.first = VK_NULL_HANDLE;  // ANI isn't on a queue, so this can't participate in a completion proof.
    }

    auto pSemaphore = GetSemaphoreState(semaphore);
    if (pSemaphore && pSemaphore->scope == kSyncScopeInternal) {
        // Treat as signaled since it is valid to wait on this semaphore, even in cases where it is technically a
        // temporary import
        pSemaphore->signaled = true;
        pSemaphore->signaler.first = VK_NULL_HANDLE;
    }

    // Mark the image as acquired.
    auto swapchain_data = GetSwapchainState(swapchain);
    if (swapchain_data && (swapchain_data->images.size() > *pImageIndex)) {
        auto image = swapchain_data->images[*pImageIndex];
        auto image_state = GetImageState(image);
        if (image_state) {
            image_state->acquired = true;
            image_state->shared_presentable = swapchain_data->shared_presentable;
        }
    }
}

void CoreChecks::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

void CoreChecks::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    uint32_t *pImageIndex, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex);
}

void CoreChecks::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                        VkPhysicalDevice *pPhysicalDevices, VkResult result) {
    if ((NULL != pPhysicalDevices) && ((result == VK_SUCCESS || result == VK_INCOMPLETE))) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            auto &phys_device_state = physical_device_map[pPhysicalDevices[i]];
            phys_device_state.phys_device = pPhysicalDevices[i];
            // Init actual features for each physical device
            DispatchGetPhysicalDeviceFeatures(pPhysicalDevices[i], &phys_device_state.features2.features);
        }
    }
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static bool ValidateCommonGetPhysicalDeviceQueueFamilyProperties(debug_report_data *report_data, PHYSICAL_DEVICE_STATE *pd_state,
                                                                 uint32_t requested_queue_family_property_count, bool qfp_null,
                                                                 const char *caller_name) {
    bool skip = false;
    if (!qfp_null) {
        // Verify that for each physical device, this command is called first with NULL pQueueFamilyProperties in order to get count
        if (UNCALLED == pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                HandleToUint64(pd_state->phys_device), kVUID_Core_DevLimit_MissingQueryCount,
                "%s is called with non-NULL pQueueFamilyProperties before obtaining pQueueFamilyPropertyCount. It is recommended "
                "to first call %s with NULL pQueueFamilyProperties in order to obtain the maximal pQueueFamilyPropertyCount.",
                caller_name, caller_name);
            // Then verify that pCount that is passed in on second call matches what was returned
        } else if (pd_state->queue_family_known_count != requested_queue_family_property_count) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                HandleToUint64(pd_state->phys_device), kVUID_Core_DevLimit_CountMismatch,
                "%s is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                ". It is recommended to instead receive all the properties by calling %s with pQueueFamilyPropertyCount that was "
                "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                caller_name, requested_queue_family_property_count, pd_state->queue_family_known_count, caller_name, caller_name);
        }
        pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState = QUERY_DETAILS;
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                       uint32_t *pQueueFamilyPropertyCount,
                                                                       VkQueueFamilyProperties *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties()");
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                        uint32_t *pQueueFamilyPropertyCount,
                                                                        VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2()");
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pQueueFamilyPropertyCount,
                                                                           VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2KHR()");
}

// Common function to update state for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static void StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_STATE *pd_state, uint32_t count,
                                                                    VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    pd_state->queue_family_known_count = std::max(pd_state->queue_family_known_count, count);

    if (!pQueueFamilyProperties) {
        if (UNCALLED == pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState)
            pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState = QUERY_COUNT;
    } else {  // Save queue family properties
        pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState = QUERY_DETAILS;

        pd_state->queue_family_properties.resize(std::max(static_cast<uint32_t>(pd_state->queue_family_properties.size()), count));
        for (uint32_t i = 0; i < count; ++i) {
            pd_state->queue_family_properties[i] = pQueueFamilyProperties[i].queueFamilyProperties;
        }
    }
}

void CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                      uint32_t *pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    VkQueueFamilyProperties2KHR *pqfp = nullptr;
    std::vector<VkQueueFamilyProperties2KHR> qfp;
    qfp.resize(*pQueueFamilyPropertyCount);
    if (pQueueFamilyProperties) {
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; ++i) {
            qfp[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR;
            qfp[i].pNext = nullptr;
            qfp[i].queueFamilyProperties = pQueueFamilyProperties[i];
        }
        pqfp = qfp.data();
    }
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount, pqfp);
}

void CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                       uint32_t *pQueueFamilyPropertyCount,
                                                                       VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                            pQueueFamilyProperties);
}

void CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t *pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                            pQueueFamilyProperties);
}

bool CoreChecks::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                  const VkAllocationCallbacks *pAllocator) {
    auto surface_state = GetSurfaceState(surface);
    bool skip = false;
    if ((surface_state) && (surface_state->swapchain)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
                        HandleToUint64(instance), "VUID-vkDestroySurfaceKHR-surface-01266",
                        "vkDestroySurfaceKHR() called before its associated VkSwapchainKHR was destroyed.");
    }
    return skip;
}

void CoreChecks::PreCallRecordValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                        const VkAllocationCallbacks *pAllocator) {
    surface_map.erase(surface);
}

void CoreChecks::RecordVulkanSurface(VkSurfaceKHR *pSurface) {
    surface_map[*pSurface] = std::unique_ptr<SURFACE_STATE>(new SURFACE_STATE{*pSurface});
}

void CoreChecks::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void CoreChecks::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_IOS_MVK
void CoreChecks::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void CoreChecks::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void CoreChecks::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                               uint32_t queueFamilyIndex,
                                                                               struct wl_display *display) {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-queueFamilyIndex-01306",
                                    "vkGetPhysicalDeviceWaylandPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void CoreChecks::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t queueFamilyIndex) {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-queueFamilyIndex-01309",
                                    "vkGetPhysicalDeviceWin32PresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void CoreChecks::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, xcb_connection_t *connection,
                                                                           xcb_visualid_t visual_id) {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-queueFamilyIndex-01312",
                                    "vkGetPhysicalDeviceXcbPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
void CoreChecks::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                    VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex, Display *dpy,
                                                                            VisualID visualID) {
    const auto pd_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(pd_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-queueFamilyIndex-01315",
                                    "vkGetPhysicalDeviceXlibPresentationSupportKHR", "queueFamilyIndex");
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                       VkResult result) {
    if (VK_SUCCESS != result) return;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    physical_device_state->surfaceCapabilities = *pSurfaceCapabilities;
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                        VkResult result) {
    if (VK_SUCCESS != result) return;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    physical_device_state->surfaceCapabilities = pSurfaceCapabilities->surfaceCapabilities;
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilities2EXT *pSurfaceCapabilities,
                                                                        VkResult result) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    physical_device_state->surfaceCapabilities.minImageCount = pSurfaceCapabilities->minImageCount;
    physical_device_state->surfaceCapabilities.maxImageCount = pSurfaceCapabilities->maxImageCount;
    physical_device_state->surfaceCapabilities.currentExtent = pSurfaceCapabilities->currentExtent;
    physical_device_state->surfaceCapabilities.minImageExtent = pSurfaceCapabilities->minImageExtent;
    physical_device_state->surfaceCapabilities.maxImageExtent = pSurfaceCapabilities->maxImageExtent;
    physical_device_state->surfaceCapabilities.maxImageArrayLayers = pSurfaceCapabilities->maxImageArrayLayers;
    physical_device_state->surfaceCapabilities.supportedTransforms = pSurfaceCapabilities->supportedTransforms;
    physical_device_state->surfaceCapabilities.currentTransform = pSurfaceCapabilities->currentTransform;
    physical_device_state->surfaceCapabilities.supportedCompositeAlpha = pSurfaceCapabilities->supportedCompositeAlpha;
    physical_device_state->surfaceCapabilities.supportedUsageFlags = pSurfaceCapabilities->supportedUsageFlags;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   VkSurfaceKHR surface, VkBool32 *pSupported) {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    return ValidateQueueFamilyIndex(physical_device_state, queueFamilyIndex,
                                    "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-queueFamilyIndex-01269",
                                    "vkGetPhysicalDeviceSurfaceSupportKHR", "queueFamilyIndex");
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  VkSurfaceKHR surface, VkBool32 *pSupported, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto surface_state = GetSurfaceState(surface);
    surface_state->gpu_queue_support[{physicalDevice, queueFamilyIndex}] = (*pSupported == VK_TRUE);
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                       VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    // TODO: This isn't quite right -- available modes may differ by surface AND physical device.
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    auto &call_state = physical_device_state->vkGetPhysicalDeviceSurfacePresentModesKHRState;

    if (*pPresentModeCount) {
        if (call_state < QUERY_COUNT) call_state = QUERY_COUNT;
        if (*pPresentModeCount > physical_device_state->present_modes.size())
            physical_device_state->present_modes.resize(*pPresentModeCount);
    }
    if (pPresentModes) {
        if (call_state < QUERY_DETAILS) call_state = QUERY_DETAILS;
        for (uint32_t i = 0; i < *pPresentModeCount; i++) {
            physical_device_state->present_modes[i] = pPresentModes[i];
        }
    }
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormatKHR *pSurfaceFormats) {
    if (!pSurfaceFormats) return false;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    auto &call_state = physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;
    bool skip = false;
    switch (call_state) {
        case UNCALLED:
            // Since we haven't recorded a preliminary value of *pSurfaceFormatCount, that likely means that the application didn't
            // previously call this function with a NULL value of pSurfaceFormats:
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                            HandleToUint64(physicalDevice), kVUID_Core_DevLimit_MustQueryCount,
                            "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount; but no prior "
                            "positive value has been seen for pSurfaceFormats.");
            break;
        default:
            auto prev_format_count = (uint32_t)physical_device_state->surface_formats.size();
            if (prev_format_count != *pSurfaceFormatCount) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                                HandleToUint64(physicalDevice), kVUID_Core_DevLimit_CountMismatch,
                                "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount, and with "
                                "pSurfaceFormats set to a value (%u) that is greater than the value (%u) that was returned "
                                "when pSurfaceFormatCount was NULL.",
                                *pSurfaceFormatCount, prev_format_count);
            }
            break;
    }
    return skip;
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t *pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR *pSurfaceFormats, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    auto &call_state = physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;

    if (*pSurfaceFormatCount) {
        if (call_state < QUERY_COUNT) call_state = QUERY_COUNT;
        if (*pSurfaceFormatCount > physical_device_state->surface_formats.size())
            physical_device_state->surface_formats.resize(*pSurfaceFormatCount);
    }
    if (pSurfaceFormats) {
        if (call_state < QUERY_DETAILS) call_state = QUERY_DETAILS;
        for (uint32_t i = 0; i < *pSurfaceFormatCount; i++) {
            physical_device_state->surface_formats[i] = pSurfaceFormats[i];
        }
    }
}

void CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR *pSurfaceFormats, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    auto physicalDeviceState = GetPhysicalDeviceState(physicalDevice);
    if (*pSurfaceFormatCount) {
        if (physicalDeviceState->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_COUNT) {
            physicalDeviceState->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_COUNT;
        }
        if (*pSurfaceFormatCount > physicalDeviceState->surface_formats.size())
            physicalDeviceState->surface_formats.resize(*pSurfaceFormatCount);
    }
    if (pSurfaceFormats) {
        if (physicalDeviceState->vkGetPhysicalDeviceSurfaceFormatsKHRState < QUERY_DETAILS) {
            physicalDeviceState->vkGetPhysicalDeviceSurfaceFormatsKHRState = QUERY_DETAILS;
        }
        for (uint32_t i = 0; i < *pSurfaceFormatCount; i++) {
            physicalDeviceState->surface_formats[i] = pSurfaceFormats[i].surfaceFormat;
        }
    }
}

void CoreChecks::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    BeginCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);
}

void CoreChecks::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    EndCmdDebugUtilsLabel(report_data, commandBuffer);
}

void CoreChecks::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    InsertCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);

    // Squirrel away an easily accessible copy.
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->debug_label = LoggingLabel(pLabelInfo);
}

void ValidationStateTracker::RecordEnumeratePhysicalDeviceGroupsState(
    uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHR *pPhysicalDeviceGroupProperties) {
    if (NULL != pPhysicalDeviceGroupProperties) {
        for (uint32_t i = 0; i < *pPhysicalDeviceGroupCount; i++) {
            for (uint32_t j = 0; j < pPhysicalDeviceGroupProperties[i].physicalDeviceCount; j++) {
                VkPhysicalDevice cur_phys_dev = pPhysicalDeviceGroupProperties[i].physicalDevices[j];
                auto &phys_device_state = physical_device_map[cur_phys_dev];
                phys_device_state.phys_device = cur_phys_dev;
                // Init actual features for each physical device
                DispatchGetPhysicalDeviceFeatures(cur_phys_dev, &phys_device_state.features2.features);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHR *pPhysicalDeviceGroupProperties,
    VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordEnumeratePhysicalDeviceGroupsState(pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHR *pPhysicalDeviceGroupProperties,
    VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordEnumeratePhysicalDeviceGroupsState(pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

bool CoreChecks::ValidateDescriptorUpdateTemplate(const char *func_name,
                                                  const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo) {
    bool skip = false;
    const auto layout = GetDescriptorSetLayout(this, pCreateInfo->descriptorSetLayout);
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
                                                               VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplate()", pCreateInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplateKHR()", pCreateInfo);
    return skip;
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                          VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                          const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                             VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                             const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                                       VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    safe_VkDescriptorUpdateTemplateCreateInfo *local_create_info = new safe_VkDescriptorUpdateTemplateCreateInfo(pCreateInfo);
    std::unique_ptr<TEMPLATE_STATE> template_state(new TEMPLATE_STATE(*pDescriptorUpdateTemplate, local_create_info));
    desc_template_map[*pDescriptorUpdateTemplate] = std::move(template_state);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateDescriptorUpdateTemplateState(pCreateInfo, pDescriptorUpdateTemplate);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateDescriptorUpdateTemplateState(pCreateInfo, pDescriptorUpdateTemplate);
}

bool CoreChecks::ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                         const void *pData) {
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
                                                                const void *pData) {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                   const void *pData) {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

void ValidationStateTracker::RecordUpdateDescriptorSetWithTemplateState(VkDescriptorSet descriptorSet,
                                                                        VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                        const void *pData) {
    auto const template_map_entry = desc_template_map.find(descriptorUpdateTemplate);
    if ((template_map_entry == desc_template_map.end()) || (template_map_entry->second.get() == nullptr)) {
        assert(0);
    } else {
        const TEMPLATE_STATE *template_state = template_map_entry->second.get();
        // TODO: Record template push descriptor updates
        if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
            PerformUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state, pData);
        }
    }
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                          const void *pData) {
    RecordUpdateDescriptorSetWithTemplateState(descriptorSet, descriptorUpdateTemplate, pData);
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                             VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                             const void *pData) {
    RecordUpdateDescriptorSetWithTemplateState(descriptorSet, descriptorUpdateTemplate, pData);
}

static std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> GetDslFromPipelineLayout(
    PIPELINE_LAYOUT_STATE const *layout_data, uint32_t set) {
    std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> dsl = nullptr;
    if (layout_data && (set < layout_data->set_layouts.size())) {
        dsl = layout_data->set_layouts[set];
    }
    return dsl;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set, const void *pData) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    const char *const func_name = "vkPushDescriptorSetWithTemplateKHR()";
    bool skip = false;
    skip |= ValidateCmd(cb_state, CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR, func_name);

    auto layout_data = GetPipelineLayout(layout);
    auto dsl = GetDslFromPipelineLayout(layout_data, set);
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
        cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, VK_NULL_HANDLE, dsl, 0, this);
        // Decode the template into a set of write updates
        cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state, pData,
                                                                dsl->GetDescriptorSetLayout());
        // Validate the decoded update against the proxy_ds
        skip |= ValidatePushDescriptorsUpdate(&proxy_ds, static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                              decoded_template.desc_writes.data(), func_name);
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                  VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                  VkPipelineLayout layout, uint32_t set, const void *pData) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    const auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    if (template_state) {
        auto layout_data = GetPipelineLayout(layout);
        auto dsl = GetDslFromPipelineLayout(layout_data, set);
        const auto &template_ci = template_state->create_info;
        if (dsl && !dsl->IsDestroyed()) {
            // Decode the template into a set of write updates
            cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state, pData,
                                                                    dsl->GetDescriptorSetLayout());
            RecordCmdPushDescriptorSetState(cb_state, template_ci.pipelineBindPoint, layout, set,
                                            static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                            decoded_template.desc_writes.data());
        }
    }
}

void CoreChecks::RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                    void *pProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    if (*pPropertyCount) {
        if (physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_COUNT) {
            physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_COUNT;
        }
        physical_device_state->display_plane_property_count = *pPropertyCount;
    }
    if (pProperties) {
        if (physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState < QUERY_DETAILS) {
            physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = QUERY_DETAILS;
        }
    }
}

void CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                          VkDisplayPlanePropertiesKHR *pProperties,
                                                                          VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayPlaneProperties2KHR *pProperties,
                                                                           VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

bool CoreChecks::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                         const char *api_name) {
    bool skip = false;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
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
                                                                    uint32_t *pDisplayCount, VkDisplayKHR *pDisplays) {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex,
                                                                    "vkGetDisplayPlaneSupportedDisplaysKHR");
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                               uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities) {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, planeIndex, "vkGetDisplayPlaneCapabilitiesKHR");
    return skip;
}

bool CoreChecks::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                                VkDisplayPlaneCapabilities2KHR *pCapabilities) {
    bool skip = false;
    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, pDisplayPlaneInfo->planeIndex,
                                                                    "vkGetDisplayPlaneCapabilities2KHR");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                                       const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmd(cb_state, CMD_DEBUGMARKERBEGINEXT, "vkCmdDebugMarkerBeginEXT()");
}

bool CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmd(cb_state, CMD_DEBUGMARKERENDEXT, "vkCmdDebugMarkerEndEXT()");
}

bool CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                        VkQueryControlFlags flags, uint32_t index) {
    if (disabled.query_validation) return false;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
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

void CoreChecks::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                       VkQueryControlFlags flags, uint32_t index) {
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordBeginQuery(cb_state, query_obj);
}

bool CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      uint32_t index) {
    if (disabled.query_validation) return false;
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state);
    return ValidateCmdEndQuery(cb_state, query_obj, CMD_ENDQUERYINDEXEDEXT, "vkCmdEndQueryIndexedEXT()",
                               "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-cmdpool", "VUID-vkCmdEndQueryIndexedEXT-None-02342");
}

void CoreChecks::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                     uint32_t index) {
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdEndQuery(cb_state, query_obj);
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles) {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // Minimal validation for command buffer state
    return ValidateCmd(cb_state, CMD_SETDISCARDRECTANGLEEXT, "vkCmdSetDiscardRectangleEXT()");
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT *pSampleLocationsInfo) {
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // Minimal validation for command buffer state
    return ValidateCmd(cb_state, CMD_SETSAMPLELOCATIONSEXT, "vkCmdSetSampleLocationsEXT()");
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversion(const char *func_name,
                                                      const VkSamplerYcbcrConversionCreateInfo *create_info) {
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
                                                             VkSamplerYcbcrConversion *pYcbcrConversion) {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversion()", pCreateInfo);
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion) {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversionKHR()", pCreateInfo);
}

void CoreChecks::RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                         VkSamplerYcbcrConversion ycbcr_conversion) {
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordCreateSamplerYcbcrConversionANDROID(create_info, ycbcr_conversion);
    }
}

void CoreChecks::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void CoreChecks::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void CoreChecks::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordDestroySamplerYcbcrConversionANDROID(ycbcrConversion);
    }
}

void CoreChecks::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordDestroySamplerYcbcrConversionANDROID(ycbcrConversion);
    }
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo) {
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

    auto buffer_state = GetBufferState(pInfo->buffer);
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
                                    uint32_t queryCount, const char *vuid_badfirst, const char *vuid_badrange) {
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
                                                  uint32_t queryCount) {
    if (disabled.query_validation) return false;

    bool skip = false;

    if (!enabled_features.host_query_reset_features.hostQueryReset) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                        "VUID-vkResetQueryPoolEXT-None-02665", "Host query reset not enabled for device");
    }

    auto query_pool_it = queryPoolMap.find(queryPool);
    if (query_pool_it != queryPoolMap.end()) {
        skip |= ValidateQueryRange(device, queryPool, query_pool_it->second->createInfo.queryCount, firstQuery, queryCount,
                                   "VUID-vkResetQueryPoolEXT-firstQuery-02666", "VUID-vkResetQueryPoolEXT-firstQuery-02667");
    }

    return skip;
}

void CoreChecks::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.host_query_reset_features.hostQueryReset) return;

    // Do nothing if the query pool has been destroyed.
    auto query_pool_it = queryPoolMap.find(queryPool);
    if (query_pool_it == queryPoolMap.end()) return;

    // Reset the state of existing entries.
    QueryObject query_obj{queryPool, 0};
    const uint32_t max_query_count = std::min(queryCount, query_pool_it->second->createInfo.queryCount - firstQuery);
    for (uint32_t i = 0; i < max_query_count; ++i) {
        query_obj.query = firstQuery + i;
        auto query_it = queryToStateMap.find(query_obj);
        if (query_it != queryToStateMap.end()) query_it->second = QUERYSTATE_RESET;
    }
}

void CoreChecks::PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceProperties *pPhysicalDeviceProperties) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled.gpu_validation && enabled.gpu_validation_reserve_binding_slot &&
        pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 0) {
        if (pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties->limits.maxBoundDescriptorSets -= 1;
        } else {
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(physicalDevice), "UNASSIGNED-GPU-Assisted Validation Setup Error.",
                    "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
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

bool CoreChecks::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

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

bool CoreChecks::ValidateQueryPoolStride(const std::string &vuid_not_64, const std::string &vuid_64, const VkDeviceSize stride,
                                         const char *parameter_name, const uint64_t parameter_value,
                                         const VkQueryResultFlags flags) {
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
                                                 const char *struct_name, const uint32_t struct_size) {
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
                                                 const VkDeviceSize offset, const BUFFER_STATE *buffer_state) {
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

void PIPELINE_STATE::initGraphicsPipeline(ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                          std::shared_ptr<RENDER_PASS_STATE> &&rpstate) {
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
        }
    }
    if (graphicsPipelineCI.pColorBlendState) {
        const auto pCBCI = graphicsPipelineCI.pColorBlendState;
        if (pCBCI->attachmentCount) {
            this->attachments =
                std::vector<VkPipelineColorBlendAttachmentState>(pCBCI->pAttachments, pCBCI->pAttachments + pCBCI->attachmentCount);
        }
    }
    if (graphicsPipelineCI.pInputAssemblyState) {
        topology_at_rasterizer = graphicsPipelineCI.pInputAssemblyState->topology;
    }
    rp_state = rpstate;
}

void PIPELINE_STATE::initComputePipeline(ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo) {
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

void PIPELINE_STATE::initRayTracingPipelineNV(ValidationStateTracker *state_data,
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
