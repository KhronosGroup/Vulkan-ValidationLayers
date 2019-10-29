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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Shannon McPherson <shannon@lunarg.com>
 */

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include <cmath>
#include <set>
#include <sstream>
#include <string>

#include "vk_enum_string_helper.h"
#include "vk_format_utils.h"
#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "vk_layer_logging.h"
#include "vk_typemap_helper.h"

#include "chassis.h"
#include "state_tracker.h"
#include "shader_validation.h"

using std::max;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

void ValidationStateTracker::RecordCreateImageANDROID(const VkImageCreateInfo *create_info, IMAGE_STATE *is_node) {
    const VkExternalMemoryImageCreateInfo *emici = lvl_find_in_chain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        is_node->imported_ahb = true;
    }
    const VkExternalFormatANDROID *ext_fmt_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android && (0 != ext_fmt_android->externalFormat)) {
        is_node->has_ahb_format = true;
        is_node->ahb_format = ext_fmt_android->externalFormat;
    }
}

void ValidationStateTracker::RecordCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                                       VkSamplerYcbcrConversion ycbcr_conversion) {
    const VkExternalFormatANDROID *ext_format_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_format_android && (0 != ext_format_android->externalFormat)) {
        ycbcr_conversion_ahb_fmt_map.emplace(ycbcr_conversion, ext_format_android->externalFormat);
    }
};

void ValidationStateTracker::RecordDestroySamplerYcbcrConversionANDROID(VkSamplerYcbcrConversion ycbcr_conversion) {
    ycbcr_conversion_ahb_fmt_map.erase(ycbcr_conversion);
};

#else

void ValidationStateTracker::RecordCreateImageANDROID(const VkImageCreateInfo *create_info, IMAGE_STATE *is_node) {}

void ValidationStateTracker::RecordCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                                       VkSamplerYcbcrConversion ycbcr_conversion){};

void ValidationStateTracker::RecordDestroySamplerYcbcrConversionANDROID(VkSamplerYcbcrConversion ycbcr_conversion){};

#endif  // VK_USE_PLATFORM_ANDROID_KHR

void ValidationStateTracker::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto is_node = std::make_shared<IMAGE_STATE>(*pImage, pCreateInfo);
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordCreateImageANDROID(pCreateInfo, is_node.get());
    }
    const auto swapchain_info = lvl_find_in_chain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext);
    if (swapchain_info) {
        is_node->create_from_swapchain = swapchain_info->swapchain;
    }

    bool pre_fetch_memory_reqs = true;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if (is_node->external_format_android) {
        // Do not fetch requirements for external memory images
        pre_fetch_memory_reqs = false;
    }
#endif
    // Record the memory requirements in case they won't be queried
    if (pre_fetch_memory_reqs) {
        DispatchGetImageMemoryRequirements(device, *pImage, &is_node->requirements);
    }
    imageMap.insert(std::make_pair(*pImage, std::move(is_node)));
}

void ValidationStateTracker::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    if (!image) return;
    IMAGE_STATE *image_state = GetImageState(image);
    const VulkanTypedHandle obj_struct(image, kVulkanObjectTypeImage);
    InvalidateCommandBuffers(image_state->cb_bindings, obj_struct);
    // Clean up memory mapping, bindings and range references for image
    for (auto mem_binding : image_state->GetBoundMemory()) {
        auto mem_info = GetDevMemState(mem_binding);
        if (mem_info) {
            RemoveImageMemoryRange(image, mem_info);
        }
    }
    if (image_state->bind_swapchain) {
        auto swapchain = GetSwapchainState(image_state->bind_swapchain);
        if (swapchain) {
            swapchain->images[image_state->bind_swapchain_imageIndex].bound_images.erase(image_state->image);
        }
    }
    RemoveAliasingImage(image_state);
    ClearMemoryObjectBindings(obj_struct);
    image_state->destroyed = true;
    // Remove image from imageMap
    imageMap.erase(image);
}

void ValidationStateTracker::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                             uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    auto cb_node = GetCBState(commandBuffer);
    auto image_state = GetImageState(image);
    if (cb_node && image_state) {
        AddCommandBufferBindingImage(cb_node, image_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                    VkImageLayout imageLayout,
                                                                    const VkClearDepthStencilValue *pDepthStencil,
                                                                    uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    auto cb_node = GetCBState(commandBuffer);
    auto image_state = GetImageState(image);
    if (cb_node && image_state) {
        AddCommandBufferBindingImage(cb_node, image_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageCopy *pRegions) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_image_state = GetImageState(dstImage);

    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(cb_node, src_image_state);
    AddCommandBufferBindingImage(cb_node, dst_image_state);
}

void ValidationStateTracker::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                          VkImageLayout srcImageLayout, VkImage dstImage,
                                                          VkImageLayout dstImageLayout, uint32_t regionCount,
                                                          const VkImageResolve *pRegions) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_image_state = GetImageState(dstImage);

    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(cb_node, src_image_state);
    AddCommandBufferBindingImage(cb_node, dst_image_state);
}

void ValidationStateTracker::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_image_state = GetImageState(dstImage);

    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(cb_node, src_image_state);
    AddCommandBufferBindingImage(cb_node, dst_image_state);
}

void ValidationStateTracker::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                        VkResult result) {
    if (result != VK_SUCCESS) return;
    // TODO : This doesn't create deep copy of pQueueFamilyIndices so need to fix that if/when we want that data to be valid
    auto buffer_state = std::make_shared<BUFFER_STATE>(*pBuffer, pCreateInfo);

    // Get a set of requirements in the case the app does not
    DispatchGetBufferMemoryRequirements(device, *pBuffer, &buffer_state->requirements);

    bufferMap.insert(std::make_pair(*pBuffer, std::move(buffer_state)));
}

void ValidationStateTracker::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                                            VkResult result) {
    if (result != VK_SUCCESS) return;
    auto buffer_state = GetBufferShared(pCreateInfo->buffer);
    bufferViewMap[*pView] = std::make_shared<BUFFER_VIEW_STATE>(buffer_state, *pView, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                           VkResult result) {
    if (result != VK_SUCCESS) return;
    auto image_state = GetImageShared(pCreateInfo->image);
    imageViewMap[*pView] = std::make_shared<IMAGE_VIEW_STATE>(image_state, *pView, pCreateInfo);
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                        uint32_t regionCount, const VkBufferCopy *pRegions) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_buffer_state = GetBufferState(srcBuffer);
    auto dst_buffer_state = GetBufferState(dstBuffer);

    // Update bindings between buffers and cmd buffer
    AddCommandBufferBindingBuffer(cb_node, src_buffer_state);
    AddCommandBufferBindingBuffer(cb_node, dst_buffer_state);
}

void ValidationStateTracker::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView,
                                                           const VkAllocationCallbacks *pAllocator) {
    IMAGE_VIEW_STATE *image_view_state = GetImageViewState(imageView);
    if (!image_view_state) return;
    const VulkanTypedHandle obj_struct(imageView, kVulkanObjectTypeImageView);

    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(image_view_state->cb_bindings, obj_struct);
    image_view_state->destroyed = true;
    imageViewMap.erase(imageView);
}

void ValidationStateTracker::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    if (!buffer) return;
    auto buffer_state = GetBufferState(buffer);
    const VulkanTypedHandle obj_struct(buffer, kVulkanObjectTypeBuffer);

    InvalidateCommandBuffers(buffer_state->cb_bindings, obj_struct);
    for (auto mem_binding : buffer_state->GetBoundMemory()) {
        auto mem_info = GetDevMemState(mem_binding);
        if (mem_info) {
            RemoveBufferMemoryRange(buffer, mem_info);
        }
    }
    ClearMemoryObjectBindings(obj_struct);
    buffer_state->destroyed = true;
    bufferMap.erase(buffer_state->buffer);
}

void ValidationStateTracker::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!bufferView) return;
    auto buffer_view_state = GetBufferViewState(bufferView);
    const VulkanTypedHandle obj_struct(bufferView, kVulkanObjectTypeBufferView);

    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(buffer_view_state->cb_bindings, obj_struct);
    buffer_view_state->destroyed = true;
    bufferViewMap.erase(bufferView);
}

void ValidationStateTracker::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize size, uint32_t data) {
    auto cb_node = GetCBState(commandBuffer);
    auto buffer_state = GetBufferState(dstBuffer);
    // Update bindings between buffer and cmd buffer
    AddCommandBufferBindingBuffer(cb_node, buffer_state);
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                               VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_image_state = GetImageState(srcImage);
    auto dst_buffer_state = GetBufferState(dstBuffer);

    // Update bindings between buffer/image and cmd buffer
    AddCommandBufferBindingImage(cb_node, src_image_state);
    AddCommandBufferBindingBuffer(cb_node, dst_buffer_state);
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                                               const VkBufferImageCopy *pRegions) {
    auto cb_node = GetCBState(commandBuffer);
    auto src_buffer_state = GetBufferState(srcBuffer);
    auto dst_image_state = GetImageState(dstImage);

    AddCommandBufferBindingBuffer(cb_node, src_buffer_state);
    AddCommandBufferBindingImage(cb_node, dst_image_state);
}

// Get the image viewstate for a given framebuffer attachment
IMAGE_VIEW_STATE *ValidationStateTracker::GetAttachmentImageViewState(FRAMEBUFFER_STATE *framebuffer, uint32_t index) {
    assert(framebuffer && (index < framebuffer->createInfo.attachmentCount));
    const VkImageView &image_view = framebuffer->createInfo.pAttachments[index];
    return GetImageViewState(image_view);
}

// Get the image viewstate for a given framebuffer attachment
const IMAGE_VIEW_STATE *ValidationStateTracker::GetAttachmentImageViewState(const FRAMEBUFFER_STATE *framebuffer,
                                                                            uint32_t index) const {
    assert(framebuffer && (index < framebuffer->createInfo.attachmentCount));
    const VkImageView &image_view = framebuffer->createInfo.pAttachments[index];
    return GetImageViewState(image_view);
}

void ValidationStateTracker::AddAliasingImage(IMAGE_STATE *image_state) {
    if (!(image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT)) return;
    std::unordered_set<VkImage> *bound_images = nullptr;

    if (image_state->bind_swapchain) {
        auto swapchain_state = GetSwapchainState(image_state->bind_swapchain);
        if (swapchain_state) {
            bound_images = &swapchain_state->images[image_state->bind_swapchain_imageIndex].bound_images;
        }
    } else {
        auto mem_state = GetDevMemState(image_state->binding.mem);
        if (mem_state) {
            bound_images = &mem_state->bound_images;
        }
    }

    if (bound_images) {
        for (const auto &handle : *bound_images) {
            if (handle != image_state->image) {
                auto is = GetImageState(handle);
                if (is && is->IsCompatibleAliasing(image_state)) {
                    auto inserted = is->aliasing_images.emplace(image_state->image);
                    if (inserted.second) {
                        image_state->aliasing_images.emplace(handle);
                    }
                }
            }
        }
    }
}

void ValidationStateTracker::RemoveAliasingImage(IMAGE_STATE *image_state) {
    for (const auto &image : image_state->aliasing_images) {
        auto is = GetImageState(image);
        if (is) {
            is->aliasing_images.erase(image_state->image);
        }
    }
    image_state->aliasing_images.clear();
}

void ValidationStateTracker::RemoveAliasingImages(const std::unordered_set<VkImage> &bound_images) {
    // This is one way clear. Because the bound_images include cross references, the one way clear loop could clear the whole
    // reference. It doesn't need two ways clear.
    for (const auto &handle : bound_images) {
        auto is = GetImageState(handle);
        if (is) {
            is->aliasing_images.clear();
        }
    }
}

const EVENT_STATE *ValidationStateTracker::GetEventState(VkEvent event) const {
    auto it = eventMap.find(event);
    if (it == eventMap.end()) {
        return nullptr;
    }
    return &it->second;
}

EVENT_STATE *ValidationStateTracker::GetEventState(VkEvent event) {
    auto it = eventMap.find(event);
    if (it == eventMap.end()) {
        return nullptr;
    }
    return &it->second;
}

const QUEUE_STATE *ValidationStateTracker::GetQueueState(VkQueue queue) const {
    auto it = queueMap.find(queue);
    if (it == queueMap.cend()) {
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

const PHYSICAL_DEVICE_STATE *ValidationStateTracker::GetPhysicalDeviceState(VkPhysicalDevice phys) const {
    auto *phys_dev_map = ((physical_device_map.size() > 0) ? &physical_device_map : &instance_state->physical_device_map);
    auto it = phys_dev_map->find(phys);
    if (it == phys_dev_map->end()) {
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
template <typename State, typename Result>
static Result GetObjectMemBindingImpl(State state, const VulkanTypedHandle &typed_handle) {
    switch (typed_handle.type) {
        case kVulkanObjectTypeImage:
            return state->GetImageState(typed_handle.Cast<VkImage>());
        case kVulkanObjectTypeBuffer:
            return state->GetBufferState(typed_handle.Cast<VkBuffer>());
        case kVulkanObjectTypeAccelerationStructureNV:
            return state->GetAccelerationStructureState(typed_handle.Cast<VkAccelerationStructureNV>());
        default:
            break;
    }
    return nullptr;
}

const BINDABLE *ValidationStateTracker::GetObjectMemBinding(const VulkanTypedHandle &typed_handle) const {
    return GetObjectMemBindingImpl<const ValidationStateTracker *, const BINDABLE *>(this, typed_handle);
}

BINDABLE *ValidationStateTracker::GetObjectMemBinding(const VulkanTypedHandle &typed_handle) {
    return GetObjectMemBindingImpl<ValidationStateTracker *, BINDABLE *>(this, typed_handle);
}

void ValidationStateTracker::AddMemObjInfo(void *object, const VkDeviceMemory mem, const VkMemoryAllocateInfo *pAllocateInfo) {
    assert(object != NULL);

    memObjMap[mem] = std::make_shared<DEVICE_MEMORY_STATE>(object, mem, pAllocateInfo);
    auto mem_info = memObjMap[mem].get();

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
void ValidationStateTracker::AddCommandBufferBindingSampler(CMD_BUFFER_STATE *cb_node, SAMPLER_STATE *sampler_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    AddCommandBufferBinding(sampler_state->cb_bindings,
                            VulkanTypedHandle(sampler_state->sampler, kVulkanObjectTypeSampler, sampler_state), cb_node);
}

// Create binding link between given image node and command buffer node
void ValidationStateTracker::AddCommandBufferBindingImage(CMD_BUFFER_STATE *cb_node, IMAGE_STATE *image_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    // Skip validation if this image was created through WSI
    if (image_state->create_from_swapchain == VK_NULL_HANDLE) {
        // First update cb binding for image
        if (AddCommandBufferBinding(image_state->cb_bindings,
                                    VulkanTypedHandle(image_state->image, kVulkanObjectTypeImage, image_state), cb_node)) {
            // Now update CB binding in MemObj mini CB list
            for (auto mem_binding : image_state->GetBoundMemory()) {
                DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
                if (pMemInfo) {
                    // Now update CBInfo's Mem reference list
                    AddCommandBufferBinding(pMemInfo->cb_bindings,
                                            VulkanTypedHandle(mem_binding, kVulkanObjectTypeDeviceMemory, pMemInfo), cb_node);
                }
            }
        }
    }
}

// Create binding link between given image view node and its image with command buffer node
void ValidationStateTracker::AddCommandBufferBindingImageView(CMD_BUFFER_STATE *cb_node, IMAGE_VIEW_STATE *view_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    // First add bindings for imageView
    if (AddCommandBufferBinding(view_state->cb_bindings,
                                VulkanTypedHandle(view_state->image_view, kVulkanObjectTypeImageView, view_state), cb_node)) {
        // Only need to continue if this is a new item
        auto image_state = view_state->image_state.get();
        // Add bindings for image within imageView
        if (image_state) {
            AddCommandBufferBindingImage(cb_node, image_state);
        }
    }
}

// Create binding link between given buffer node and command buffer node
void ValidationStateTracker::AddCommandBufferBindingBuffer(CMD_BUFFER_STATE *cb_node, BUFFER_STATE *buffer_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    // First update cb binding for buffer
    if (AddCommandBufferBinding(buffer_state->cb_bindings,
                                VulkanTypedHandle(buffer_state->buffer, kVulkanObjectTypeBuffer, buffer_state), cb_node)) {
        // Now update CB binding in MemObj mini CB list
        for (auto mem_binding : buffer_state->GetBoundMemory()) {
            DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
            if (pMemInfo) {
                // Now update CBInfo's Mem reference list
                AddCommandBufferBinding(pMemInfo->cb_bindings,
                                        VulkanTypedHandle(mem_binding, kVulkanObjectTypeDeviceMemory, pMemInfo), cb_node);
            }
        }
    }
}

// Create binding link between given buffer view node and its buffer with command buffer node
void ValidationStateTracker::AddCommandBufferBindingBufferView(CMD_BUFFER_STATE *cb_node, BUFFER_VIEW_STATE *view_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    // First add bindings for bufferView
    if (AddCommandBufferBinding(view_state->cb_bindings,
                                VulkanTypedHandle(view_state->buffer_view, kVulkanObjectTypeBufferView, view_state), cb_node)) {
        auto buffer_state = view_state->buffer_state.get();
        // Add bindings for buffer within bufferView
        if (buffer_state) {
            AddCommandBufferBindingBuffer(cb_node, buffer_state);
        }
    }
}

// Create binding link between given acceleration structure and command buffer node
void ValidationStateTracker::AddCommandBufferBindingAccelerationStructure(CMD_BUFFER_STATE *cb_node,
                                                                          ACCELERATION_STRUCTURE_STATE *as_state) {
    if (disabled.command_buffer_state) {
        return;
    }
    if (AddCommandBufferBinding(
            as_state->cb_bindings,
            VulkanTypedHandle(as_state->acceleration_structure, kVulkanObjectTypeAccelerationStructureNV, as_state), cb_node)) {
        // Now update CB binding in MemObj mini CB list
        for (auto mem_binding : as_state->GetBoundMemory()) {
            DEVICE_MEMORY_STATE *pMemInfo = GetDevMemState(mem_binding);
            if (pMemInfo) {
                // Now update CBInfo's Mem reference list
                AddCommandBufferBinding(pMemInfo->cb_bindings,
                                        VulkanTypedHandle(mem_binding, kVulkanObjectTypeDeviceMemory, pMemInfo), cb_node);
            }
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

// SetMemBinding is used to establish immutable, non-sparse binding between a single image/buffer object and memory object.
// Corresponding valid usage checks are in ValidateSetMemBinding().
void ValidationStateTracker::SetMemBinding(VkDeviceMemory mem, BINDABLE *mem_binding, VkDeviceSize memory_offset,
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

// For NULL mem case, clear any previous binding Else...
// Make sure given object is in its object map
//  IF a previous binding existed, update binding
//  Add reference from objectInfo to memoryInfo
//  Add reference off of object's binding info
// Return VK_TRUE if addition is successful, VK_FALSE otherwise
bool ValidationStateTracker::SetSparseMemBinding(MEM_BINDING binding, const VulkanTypedHandle &typed_handle) {
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

void ValidationStateTracker::UpdateDrawState(CMD_BUFFER_STATE *cb_state, const VkPipelineBindPoint bind_point) {
    auto &state = cb_state->lastBound[bind_point];
    PIPELINE_STATE *pPipe = state.pipeline_state;
    if (VK_NULL_HANDLE != state.pipeline_layout) {
        for (const auto &set_binding_pair : pPipe->active_slots) {
            uint32_t setIndex = set_binding_pair.first;
            // Pull the set node
            cvdescriptorset::DescriptorSet *descriptor_set = state.per_set[setIndex].bound_descriptor_set;
            if (!descriptor_set->IsPushDescriptor()) {
                // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor binding

                // TODO: If recreating the reduced_map here shows up in profilinging, need to find a way of sharing with the
                // Validate pass.  Though in the case of "many" descriptors, typically the descriptor count >> binding count
                cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second);
                const auto &binding_req_map = reduced_map.FilteredMap(*cb_state, *pPipe);

                if (reduced_map.IsManyDescriptors()) {
                    // Only update validate binding tags if we meet the "many" criteria in the Prefilter class
                    descriptor_set->UpdateValidationCache(*cb_state, *pPipe, binding_req_map);
                }

                // We can skip updating the state if "nothing" has changed since the last validation.
                // See CoreChecks::ValidateCmdBufDrawState for more details.
                bool descriptor_set_changed =
                    !reduced_map.IsManyDescriptors() ||
                    // Update if descriptor set (or contents) has changed
                    state.per_set[setIndex].validated_set != descriptor_set ||
                    state.per_set[setIndex].validated_set_change_count != descriptor_set->GetChangeCount() ||
                    (!disabled.image_layout_validation &&
                     state.per_set[setIndex].validated_set_image_layout_change_count != cb_state->image_layout_change_count);
                bool need_update = descriptor_set_changed ||
                                   // Update if previous bindingReqMap doesn't include new bindingReqMap
                                   !std::includes(state.per_set[setIndex].validated_set_binding_req_map.begin(),
                                                  state.per_set[setIndex].validated_set_binding_req_map.end(),
                                                  binding_req_map.begin(), binding_req_map.end());

                if (need_update) {
                    // Bind this set and its active descriptor resources to the command buffer
                    if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                        // Only record the bindings that haven't already been recorded
                        BindingReqMap delta_reqs;
                        std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                            state.per_set[setIndex].validated_set_binding_req_map.begin(),
                                            state.per_set[setIndex].validated_set_binding_req_map.end(),
                                            std::inserter(delta_reqs, delta_reqs.begin()));
                        descriptor_set->UpdateDrawState(this, cb_state, pPipe, delta_reqs);
                    } else {
                        descriptor_set->UpdateDrawState(this, cb_state, pPipe, binding_req_map);
                    }

                    state.per_set[setIndex].validated_set = descriptor_set;
                    state.per_set[setIndex].validated_set_change_count = descriptor_set->GetChangeCount();
                    state.per_set[setIndex].validated_set_image_layout_change_count = cb_state->image_layout_change_count;
                    if (reduced_map.IsManyDescriptors()) {
                        // Check whether old == new before assigning, the equality check is much cheaper than
                        // freeing and reallocating the map.
                        if (state.per_set[setIndex].validated_set_binding_req_map != set_binding_pair.second) {
                            state.per_set[setIndex].validated_set_binding_req_map = set_binding_pair.second;
                        }
                    } else {
                        state.per_set[setIndex].validated_set_binding_req_map = BindingReqMap();
                    }
                }
            }
        }
    }
    if (!pPipe->vertex_binding_descriptions_.empty()) {
        cb_state->vertex_buffer_used = true;
    }
}

// Remove set from setMap and delete the set
void ValidationStateTracker::FreeDescriptorSet(cvdescriptorset::DescriptorSet *descriptor_set) {
    descriptor_set->destroyed = true;
    const VulkanTypedHandle obj_struct(descriptor_set->GetSet(), kVulkanObjectTypeDescriptorSet);
    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(descriptor_set->cb_bindings, obj_struct);

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

// For given object struct return a ptr of BASE_NODE type for its wrapping struct
BASE_NODE *ValidationStateTracker::GetStateStructPtrFromObject(const VulkanTypedHandle &object_struct) {
    if (object_struct.node) {
#ifdef _DEBUG
        // assert that lookup would find the same object
        VulkanTypedHandle other = object_struct;
        other.node = nullptr;
        assert(object_struct.node == GetStateStructPtrFromObject(other));
#endif
        return object_struct.node;
    }
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
        case kVulkanObjectTypeUnknown:
            // This can happen if an element of the object_bindings vector has been
            // zeroed out, after an object is destroyed.
            break;
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
bool ValidationStateTracker::AddCommandBufferBinding(small_unordered_map<CMD_BUFFER_STATE *, int, 8> &cb_bindings,
                                                     const VulkanTypedHandle &obj, CMD_BUFFER_STATE *cb_node) {
    if (disabled.command_buffer_state) {
        return false;
    }
    // Insert the cb_binding with a default 'index' of -1. Then push the obj into the object_bindings
    // vector, and update cb_bindings[cb_node] with the index of that element of the vector.
    auto inserted = cb_bindings.insert({cb_node, -1});
    if (inserted.second) {
        cb_node->object_bindings.push_back(obj);
        inserted.first->second = (int)cb_node->object_bindings.size() - 1;
        return true;
    }
    return false;
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
        pCB->hasBuildAccelerationStructureCmd = false;
        pCB->hasDispatchCmd = false;
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
        pCB->activeQueries.clear();
        pCB->startedQueries.clear();
        pCB->image_layout_map.clear();
        pCB->current_vertex_buffer_binding_info.vertex_buffer_bindings.clear();
        pCB->vertex_buffer_used = false;
        pCB->primaryCommandBuffer = VK_NULL_HANDLE;
        // If secondary, invalidate any primary command buffer that may call us.
        if (pCB->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
            InvalidateLinkedCommandBuffers(pCB->linkedCommandBuffers, VulkanTypedHandle(cb, kVulkanObjectTypeCommandBuffer));
        }

        // Remove reverse command buffer links.
        for (auto pSubCB : pCB->linkedCommandBuffers) {
            pSubCB->linkedCommandBuffers.erase(pCB);
        }
        pCB->linkedCommandBuffers.clear();
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
        pCB->validate_descriptorsets_in_queuesubmit.clear();
    }
    if (command_buffer_reset_callback) {
        (*command_buffer_reset_callback)(cb);
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
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, this->container_type);
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

    const auto *pipeline_exe_props_features =
        lvl_find_in_chain<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(pCreateInfo->pNext);
    if (pipeline_exe_props_features) {
        state_tracker->enabled_features.pipeline_exe_props_features = *pipeline_exe_props_features;
    }

    const auto *dedicated_allocation_image_aliasing_features =
        lvl_find_in_chain<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>(pCreateInfo->pNext);
    if (dedicated_allocation_image_aliasing_features) {
        state_tracker->enabled_features.dedicated_allocation_image_aliasing_features =
            *dedicated_allocation_image_aliasing_features;
    }

    const auto *subgroup_extended_types_features =
        lvl_find_in_chain<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(pCreateInfo->pNext);
    if (subgroup_extended_types_features) {
        state_tracker->enabled_features.subgroup_extended_types_features = *subgroup_extended_types_features;
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
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_fragment_density_map, &phys_dev_props->fragment_density_map_props);
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
    if (pCreateInfo->pQueueCreateInfos != nullptr) {
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            state_tracker->queue_family_index_map.insert(
                std::make_pair(pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, pCreateInfo->pQueueCreateInfos[i].queueCount));
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    // Reset all command buffers before destroying them, to unlink object_bindings.
    for (auto &commandBuffer : commandBufferMap) {
        ResetCommandBufferState(commandBuffer.first);
    }
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
    bufferViewMap.clear();
    bufferMap.clear();
    // Queues persist until device is destroyed
    queueMap.clear();
}

// Loop through bound objects and increment their in_use counts.
void ValidationStateTracker::IncrementBoundObjects(CMD_BUFFER_STATE const *cb_node) {
    for (auto obj : cb_node->object_bindings) {
        auto base_obj = GetStateStructPtrFromObject(obj);
        if (base_obj) {
            base_obj->in_use.fetch_add(1);
        }
    }
}

// Track which resources are in-flight by atomically incrementing their "in_use" count
void ValidationStateTracker::IncrementResources(CMD_BUFFER_STATE *cb_node) {
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

// Decrement in-use count for objects bound to command buffer
void ValidationStateTracker::DecrementBoundResources(CMD_BUFFER_STATE const *cb_node) {
    BASE_NODE *base_obj = nullptr;
    for (auto obj : cb_node->object_bindings) {
        base_obj = GetStateStructPtrFromObject(obj);
        if (base_obj) {
            base_obj->in_use.fetch_sub(1);
        }
    }
}

void ValidationStateTracker::RetireWorkOnQueue(QUEUE_STATE *pQueue, uint64_t seq) {
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
            QueryMap localQueryToStateMap;
            for (auto &function : cb_node->queryUpdates) {
                function(nullptr, /*do_validate*/ false, &localQueryToStateMap);
            }

            for (auto queryStatePair : localQueryToStateMap) {
                if (queryStatePair.second == QUERYSTATE_ENDED) {
                    queryToStateMap[queryStatePair.first] = QUERYSTATE_AVAILABLE;
                }
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

void ValidationStateTracker::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                                       VkFence fence, VkResult result) {
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
                }
            }
        }
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (cb_node) {
                cbs.push_back(submit->pCommandBuffers[i]);
                for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                    cbs.push_back(secondaryCmdBuffer->commandBuffer);
                    IncrementResources(secondaryCmdBuffer);
                }
                IncrementResources(cb_node);

                QueryMap localQueryToStateMap;
                for (auto &function : cb_node->queryUpdates) {
                    function(nullptr, /*do_validate*/ false, &localQueryToStateMap);
                }

                for (auto queryStatePair : localQueryToStateMap) {
                    queryToStateMap[queryStatePair.first] = queryStatePair.second;
                }

                EventToStageMap localEventToStageMap;
                for (auto &function : cb_node->eventUpdates) {
                    function(nullptr, /*do_validate*/ false, &localEventToStageMap);
                }

                for (auto eventStagePair : localEventToStageMap) {
                    eventMap[eventStagePair.first].stageMask = eventStagePair.second;
                }
            }
        }
        pQueue->submissions.emplace_back(cbs, semaphore_waits, semaphore_signals, semaphore_externals,
                                         submit_idx == submitCount - 1 ? fence : (VkFence)VK_NULL_HANDLE);
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(pQueue, early_retire_seq);
    }
}

void ValidationStateTracker::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                          VkResult result) {
    if (VK_SUCCESS == result) {
        AddMemObjInfo(device, *pMemory, pAllocateInfo);
    }
    return;
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

        if (bindable_state) {
            bindable_state->binding.mem = MEMORY_UNBOUND;
            bindable_state->UpdateBoundMemorySet();
        }
    }
    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(mem_info->cb_bindings, obj_struct);
    RemoveAliasingImages(mem_info->bound_images);
    mem_info->destroyed = true;
    memObjMap.erase(mem);
}

void ValidationStateTracker::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
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

void ValidationStateTracker::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
    auto semaphore_state = std::make_shared<SEMAPHORE_STATE>();
    semaphore_state->signaler.first = VK_NULL_HANDLE;
    semaphore_state->signaler.second = 0;
    semaphore_state->signaled = false;
    semaphore_state->scope = kSyncScopeInternal;
    semaphoreMap[*pSemaphore] = std::move(semaphore_state);
}

void ValidationStateTracker::RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type,
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

void ValidationStateTracker::RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->mapped_range.offset = offset;
        mem_info->mapped_range.size = size;
        mem_info->p_driver_data = *ppData;
    }
}

void ValidationStateTracker::RetireFence(VkFence fence) {
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

void ValidationStateTracker::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                         VkBool32 waitAll, uint64_t timeout, VkResult result) {
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

void ValidationStateTracker::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) {
    if (VK_SUCCESS != result) return;
    RetireFence(fence);
}

void ValidationStateTracker::RecordGetDeviceQueueState(uint32_t queue_family_index, VkQueue queue) {
    // Add queue to tracking set only if it is new
    auto queue_is_new = queues.emplace(queue);
    if (queue_is_new.second == true) {
        QUEUE_STATE *queue_state = &queueMap[queue];
        queue_state->queue = queue;
        queue_state->queueFamilyIndex = queue_family_index;
        queue_state->seq = 0;
    }
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                          VkQueue *pQueue) {
    RecordGetDeviceQueueState(queueFamilyIndex, *pQueue);
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, *pQueue);
}

void ValidationStateTracker::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    if (VK_SUCCESS != result) return;
    QUEUE_STATE *queue_state = GetQueueState(queue);
    RetireWorkOnQueue(queue_state, queue_state->seq + queue_state->submissions.size());
}

void ValidationStateTracker::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (auto &queue : queueMap) {
        RetireWorkOnQueue(&queue.second, queue.second.seq + queue.second.submissions.size());
    }
}

void ValidationStateTracker::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    if (!fence) return;
    auto fence_state = GetFenceState(fence);
    fence_state->destroyed = true;
    fenceMap.erase(fence);
}

void ValidationStateTracker::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                           const VkAllocationCallbacks *pAllocator) {
    if (!semaphore) return;
    auto semaphore_state = GetSemaphoreState(semaphore);
    semaphore_state->destroyed = true;
    semaphoreMap.erase(semaphore);
}

void ValidationStateTracker::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    if (!event) return;
    EVENT_STATE *event_state = GetEventState(event);
    const VulkanTypedHandle obj_struct(event, kVulkanObjectTypeEvent);
    InvalidateCommandBuffers(event_state->cb_bindings, obj_struct);
    eventMap.erase(event);
}

void ValidationStateTracker::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    if (!queryPool) return;
    QUERY_POOL_STATE *qp_state = GetQueryPoolState(queryPool);
    const VulkanTypedHandle obj_struct(queryPool, kVulkanObjectTypeQueryPool);
    InvalidateCommandBuffers(qp_state->cb_bindings, obj_struct);
    qp_state->destroyed = true;
    queryPoolMap.erase(queryPool);
}

// Object with given handle is being bound to memory w/ given mem_info struct.
//  Track the newly bound memory range with given memoryOffset
//  Also scan any previous ranges, track aliased ranges with new range, and flag an error if a linear
//  and non-linear range incorrectly overlap.
// Return true if an error is flagged and the user callback returns "true", otherwise false
// is_image indicates an image object, otherwise handle is for a buffer
// is_linear indicates a buffer or linear image
void ValidationStateTracker::InsertMemoryRange(const VulkanTypedHandle &typed_handle, DEVICE_MEMORY_STATE *mem_info,
                                               VkDeviceSize memoryOffset, VkMemoryRequirements memRequirements, bool is_linear) {
    if (typed_handle.type == kVulkanObjectTypeImage) {
        mem_info->bound_images.insert(typed_handle.Cast<VkImage>());
    } else if (typed_handle.type == kVulkanObjectTypeBuffer) {
        mem_info->bound_buffers.insert(typed_handle.handle);
    } else if (typed_handle.type == kVulkanObjectTypeAccelerationStructureNV) {
        mem_info->bound_acceleration_structures.insert(typed_handle.handle);
    } else {
        // Unsupported object type
        assert(false);
    }
}

void ValidationStateTracker::InsertImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                    VkMemoryRequirements mem_reqs, bool is_linear) {
    InsertMemoryRange(VulkanTypedHandle(image, kVulkanObjectTypeImage), mem_info, mem_offset, mem_reqs, is_linear);
}

void ValidationStateTracker::InsertBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                     const VkMemoryRequirements &mem_reqs) {
    InsertMemoryRange(VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer), mem_info, mem_offset, mem_reqs, true);
}

void ValidationStateTracker::InsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE *mem_info,
                                                                    VkDeviceSize mem_offset, const VkMemoryRequirements &mem_reqs) {
    InsertMemoryRange(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV), mem_info, mem_offset, mem_reqs, true);
}

// This function will remove the handle-to-index mapping from the appropriate map.
static void RemoveMemoryRange(const VulkanTypedHandle &typed_handle, DEVICE_MEMORY_STATE *mem_info) {
    if (typed_handle.type == kVulkanObjectTypeImage) {
        mem_info->bound_images.erase(typed_handle.Cast<VkImage>());
    } else if (typed_handle.type == kVulkanObjectTypeBuffer) {
        mem_info->bound_buffers.erase(typed_handle.handle);
    } else if (typed_handle.type == kVulkanObjectTypeAccelerationStructureNV) {
        mem_info->bound_acceleration_structures.erase(typed_handle.handle);
    } else {
        // Unsupported object type
        assert(false);
    }
}

void ValidationStateTracker::RemoveBufferMemoryRange(VkBuffer buffer, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer), mem_info);
}

void ValidationStateTracker::RemoveImageMemoryRange(VkImage image, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(VulkanTypedHandle(image, kVulkanObjectTypeImage), mem_info);
}

void ValidationStateTracker::RemoveAccelerationStructureMemoryRange(VkAccelerationStructureNV as, DEVICE_MEMORY_STATE *mem_info) {
    RemoveMemoryRange(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV), mem_info);
}

void ValidationStateTracker::UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
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

void ValidationStateTracker::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                                            VkDeviceSize memoryOffset, VkResult result) {
    if (VK_SUCCESS != result) return;
    UpdateBindBufferMemoryState(buffer, mem, memoryOffset);
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindBufferMemoryInfoKHR *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                                const VkBindBufferMemoryInfoKHR *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::RecordGetBufferMemoryRequirementsState(VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        buffer_state->requirements = *pMemoryRequirements;
        buffer_state->memory_requirements_checked = true;
    }
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                       VkMemoryRequirements *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(buffer, pMemoryRequirements);
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2(VkDevice device,
                                                                        const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                                        VkMemoryRequirements2KHR *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device,
                                                                           const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                                           VkMemoryRequirements2KHR *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

void ValidationStateTracker::RecordGetImageMemoryRequiementsState(VkImage image, VkMemoryRequirements *pMemoryRequirements) {
    IMAGE_STATE *image_state = GetImageState(image);
    if (image_state) {
        image_state->requirements = *pMemoryRequirements;
        image_state->memory_requirements_checked = true;
    }
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                      VkMemoryRequirements *pMemoryRequirements) {
    RecordGetImageMemoryRequiementsState(image, pMemoryRequirements);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                       VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetImageMemoryRequiementsState(pInfo->image, &pMemoryRequirements->memoryRequirements);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device,
                                                                          const VkImageMemoryRequirementsInfo2 *pInfo,
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

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    auto image_state = GetImageState(image);
    image_state->get_sparse_reqs_called = true;
    if (!pSparseMemoryRequirements) return;
    for (uint32_t i = 0; i < *pSparseMemoryRequirementCount; i++) {
        RecordGetImageSparseMemoryRequirementsState(image_state, &pSparseMemoryRequirements[i]);
    }
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2(
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

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2KHR(
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

void ValidationStateTracker::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                              const VkAllocationCallbacks *pAllocator) {
    if (!shaderModule) return;
    auto shader_module_state = GetShaderModuleState(shaderModule);
    shader_module_state->destroyed = true;
    shaderModuleMap.erase(shaderModule);
}

void ValidationStateTracker::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                          const VkAllocationCallbacks *pAllocator) {
    if (!pipeline) return;
    PIPELINE_STATE *pipeline_state = GetPipelineState(pipeline);
    const VulkanTypedHandle obj_struct(pipeline, kVulkanObjectTypePipeline);
    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(pipeline_state->cb_bindings, obj_struct);
    pipeline_state->destroyed = true;
    pipelineMap.erase(pipeline);
}

void ValidationStateTracker::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!pipelineLayout) return;
    auto pipeline_layout_state = GetPipelineLayout(pipelineLayout);
    pipeline_layout_state->destroyed = true;
    pipelineLayoutMap.erase(pipelineLayout);
}

void ValidationStateTracker::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler,
                                                         const VkAllocationCallbacks *pAllocator) {
    if (!sampler) return;
    SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
    const VulkanTypedHandle obj_struct(sampler, kVulkanObjectTypeSampler);
    // Any bound cmd buffers are now invalid
    if (sampler_state) {
        InvalidateCommandBuffers(sampler_state->cb_bindings, obj_struct);
    }
    sampler_state->destroyed = true;
    samplerMap.erase(sampler);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                     const VkAllocationCallbacks *pAllocator) {
    if (!descriptorSetLayout) return;
    auto layout_it = descriptorSetLayoutMap.find(descriptorSetLayout);
    if (layout_it != descriptorSetLayoutMap.end()) {
        layout_it->second.get()->destroyed = true;
        descriptorSetLayoutMap.erase(layout_it);
    }
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
        desc_pool_state->destroyed = true;
        descriptorPoolMap.erase(descriptorPool);
    }
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
            cb_state->destroyed = true;
            commandBufferMap.erase(cb_state->commandBuffer);
        }
    }
}

void ValidationStateTracker::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                             uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    auto pPool = GetCommandPoolState(commandPool);
    FreeCommandBufferStates(pPool, commandBufferCount, pCommandBuffers);
}

void ValidationStateTracker::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    auto cmd_pool_state = std::make_shared<COMMAND_POOL_STATE>();
    cmd_pool_state->createFlags = pCreateInfo->flags;
    cmd_pool_state->queueFamilyIndex = pCreateInfo->queueFamilyIndex;
    commandPoolMap[*pCommandPool] = std::move(cmd_pool_state);
}

void ValidationStateTracker::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
    auto query_pool_state = std::make_shared<QUERY_POOL_STATE>();
    query_pool_state->createInfo = *pCreateInfo;
    queryPoolMap[*pQueryPool] = std::move(query_pool_state);

    QueryObject query_obj{*pQueryPool, 0u};
    for (uint32_t i = 0; i < pCreateInfo->queryCount; ++i) {
        query_obj.query = i;
        queryToStateMap[query_obj] = QUERYSTATE_UNKNOWN;
    }
}

void ValidationStateTracker::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!commandPool) return;
    COMMAND_POOL_STATE *cp_state = GetCommandPoolState(commandPool);
    // Remove cmdpool from cmdpoolmap, after freeing layer data for the command buffers
    // "When a pool is destroyed, all command buffers allocated from the pool are freed."
    if (cp_state) {
        // Create a vector, as FreeCommandBufferStates deletes from cp_state->commandBuffers during iteration.
        std::vector<VkCommandBuffer> cb_vec{cp_state->commandBuffers.begin(), cp_state->commandBuffers.end()};
        FreeCommandBufferStates(cp_state, static_cast<uint32_t>(cb_vec.size()), cb_vec.data());
        cp_state->destroyed = true;
        commandPoolMap.erase(commandPool);
    }
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

void ValidationStateTracker::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                       VkResult result) {
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

// For given cb_nodes, invalidate them and track object causing invalidation.
// InvalidateCommandBuffers and InvalidateLinkedCommandBuffers are essentially
// the same, except one takes a map and one takes a set, and InvalidateCommandBuffers
// can also unlink objects from command buffers.
void ValidationStateTracker::InvalidateCommandBuffers(small_unordered_map<CMD_BUFFER_STATE *, int, 8> &cb_nodes,
                                                      const VulkanTypedHandle &obj, bool unlink) {
    for (const auto &cb_node_pair : cb_nodes) {
        auto &cb_node = cb_node_pair.first;
        if (cb_node->state == CB_RECORDING) {
            cb_node->state = CB_INVALID_INCOMPLETE;
        } else if (cb_node->state == CB_RECORDED) {
            cb_node->state = CB_INVALID_COMPLETE;
        }
        cb_node->broken_bindings.push_back(obj);

        // if secondary, then propagate the invalidation to the primaries that will call us.
        if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
            InvalidateLinkedCommandBuffers(cb_node->linkedCommandBuffers, obj);
        }
        if (unlink) {
            int index = cb_node_pair.second;
            assert(cb_node->object_bindings[index] == obj);
            cb_node->object_bindings[index] = VulkanTypedHandle();
        }
    }
    if (unlink) {
        cb_nodes.clear();
    }
}

void ValidationStateTracker::InvalidateLinkedCommandBuffers(std::unordered_set<CMD_BUFFER_STATE *> &cb_nodes,
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
            InvalidateLinkedCommandBuffers(cb_node->linkedCommandBuffers, obj);
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!framebuffer) return;
    FRAMEBUFFER_STATE *framebuffer_state = GetFramebufferState(framebuffer);
    const VulkanTypedHandle obj_struct(framebuffer, kVulkanObjectTypeFramebuffer);
    InvalidateCommandBuffers(framebuffer_state->cb_bindings, obj_struct);
    framebuffer_state->destroyed = true;
    frameBufferMap.erase(framebuffer);
}

void ValidationStateTracker::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!renderPass) return;
    RENDER_PASS_STATE *rp_state = GetRenderPassState(renderPass);
    const VulkanTypedHandle obj_struct(renderPass, kVulkanObjectTypeRenderPass);
    InvalidateCommandBuffers(rp_state->cb_bindings, obj_struct);
    rp_state->destroyed = true;
    renderPassMap.erase(renderPass);
}

void ValidationStateTracker::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto fence_state = std::make_shared<FENCE_STATE>();
    fence_state->fence = *pFence;
    fence_state->createInfo = *pCreateInfo;
    fence_state->state = (pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED;
    fenceMap[*pFence] = std::move(fence_state);
}

bool ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                    void *cgpl_state_data) const {
    // Set up the state that CoreChecks, gpu_validation and later StateTracker Record will use.
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    cgpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    cgpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        cgpl_state->pipe_state.push_back(std::make_shared<PIPELINE_STATE>());
        (cgpl_state->pipe_state)[i]->initGraphicsPipeline(this, &pCreateInfos[i], GetRenderPassShared(pCreateInfos[i].renderPass));
        (cgpl_state->pipe_state)[i]->pipeline_layout = GetPipelineLayoutShared(pCreateInfos[i].layout);
    }
    return false;
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

bool ValidationStateTracker::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   void *ccpl_state_data) const {
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    ccpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    ccpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        ccpl_state->pipe_state.push_back(std::make_shared<PIPELINE_STATE>());
        ccpl_state->pipe_state.back()->initComputePipeline(this, &pCreateInfos[i]);
        ccpl_state->pipe_state.back()->pipeline_layout = GetPipelineLayoutShared(pCreateInfos[i].layout);
    }
    return false;
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

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t count,
                                                                        const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines, void *crtpl_state_data) const {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(std::make_shared<PIPELINE_STATE>());
        crtpl_state->pipe_state.back()->initRayTracingPipelineNV(this, &pCreateInfos[i]);
        crtpl_state->pipe_state.back()->pipeline_layout = GetPipelineLayoutShared(pCreateInfos[i].layout);
    }
    return false;
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

void ValidationStateTracker::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                         VkResult result) {
    samplerMap[*pSampler] = std::make_shared<SAMPLER_STATE>(pSampler, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(VkDevice device,
                                                                     const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorSetLayout *pSetLayout, VkResult result) {
    if (VK_SUCCESS != result) return;
    descriptorSetLayoutMap[*pSetLayout] = std::make_shared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo, *pSetLayout);
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

void ValidationStateTracker::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkPipelineLayout *pPipelineLayout, VkResult result) {
    if (VK_SUCCESS != result) return;

    auto pipeline_layout_state = std::make_shared<PIPELINE_LAYOUT_STATE>();
    pipeline_layout_state->layout = *pPipelineLayout;
    pipeline_layout_state->set_layouts.resize(pCreateInfo->setLayoutCount);
    PipelineLayoutSetLayoutsDef set_layouts(pCreateInfo->setLayoutCount);
    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        pipeline_layout_state->set_layouts[i] = GetDescriptorSetLayoutShared(pCreateInfo->pSetLayouts[i]);
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
    descriptorPoolMap[*pDescriptorPool] = std::make_shared<DESCRIPTOR_POOL_STATE>(*pDescriptorPool, pCreateInfo);
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

bool ValidationStateTracker::PreCallValidateAllocateDescriptorSets(VkDevice device,
                                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                   VkDescriptorSet *pDescriptorSets, void *ads_state_data) const {
    // Always update common data
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    UpdateAllocateDescriptorSetsData(pAllocateInfo, ads_state);

    return false;
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

void ValidationStateTracker::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                               const VkWriteDescriptorSet *pDescriptorWrites,
                                                               uint32_t descriptorCopyCount,
                                                               const VkCopyDescriptorSet *pDescriptorCopies) {
    cvdescriptorset::PerformUpdateDescriptorSets(this, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                 pDescriptorCopies);
}

void ValidationStateTracker::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                  VkCommandBuffer *pCommandBuffer, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto pPool = GetCommandPoolShared(pCreateInfo->commandPool);
    if (pPool) {
        for (uint32_t i = 0; i < pCreateInfo->commandBufferCount; i++) {
            // Add command buffer to its commandPool map
            pPool->commandBuffers.insert(pCommandBuffer[i]);
            auto pCB = std::make_shared<CMD_BUFFER_STATE>();
            pCB->createInfo = *pCreateInfo;
            pCB->device = device;
            pCB->command_pool = pPool;
            // Add command buffer to map
            commandBufferMap[pCommandBuffer[i]] = std::move(pCB);
            ResetCommandBufferState(pCommandBuffer[i]);
        }
    }
}

// Add bindings between the given cmd buffer & framebuffer and the framebuffer's children
void ValidationStateTracker::AddFramebufferBinding(CMD_BUFFER_STATE *cb_state, FRAMEBUFFER_STATE *fb_state) {
    AddCommandBufferBinding(fb_state->cb_bindings, VulkanTypedHandle(fb_state->framebuffer, kVulkanObjectTypeFramebuffer, fb_state),
                            cb_state);

    const uint32_t attachmentCount = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachmentCount; ++attachment) {
        auto view_state = GetAttachmentImageViewState(fb_state, attachment);
        if (view_state) {
            AddCommandBufferBindingImageView(cb_state, view_state);
        }
    }
}

void ValidationStateTracker::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                             const VkCommandBufferBeginInfo *pBeginInfo) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!cb_state) return;
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

void ValidationStateTracker::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                              VkResult result) {
    if (VK_SUCCESS == result) {
        ResetCommandBufferState(commandBuffer);
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
                case VK_DYNAMIC_STATE_LINE_STIPPLE_EXT:
                    flags &= ~CBSTATUS_LINE_STIPPLE_SET;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
                    flags &= ~CBSTATUS_VIEWPORT_W_SCALING_SET;
                    break;
                default:
                    break;
            }
        }
    }

    return flags;
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
    ResetCommandBufferPushConstantDataIfIncompatible(cb_state, pipe_state->pipeline_layout->layout);
    cb_state->lastBound[pipelineBindPoint].pipeline_state = pipe_state;
    SetPipelineState(pipe_state);
    AddCommandBufferBinding(pipe_state->cb_bindings, VulkanTypedHandle(pipeline, kVulkanObjectTypePipeline), cb_state);
}

void ValidationStateTracker::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewport *pViewports) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->viewportMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->status |= CBSTATUS_VIEWPORT_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                   uint32_t exclusiveScissorCount,
                                                                   const VkRect2D *pExclusiveScissors) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;
    cb_state->status |= CBSTATUS_EXCLUSIVE_SCISSOR_SET;
}

void ValidationStateTracker::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                    VkImageLayout imageLayout) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = GetImageViewState(imageView);
        AddCommandBufferBindingImageView(cb_state, view_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                             uint32_t viewportCount,
                                                                             const VkShadingRatePaletteNV *pShadingRatePalettes) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->status |= CBSTATUS_SHADING_RATE_PALETTE_SET;
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                         const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkAccelerationStructureNV *pAccelerationStructure,
                                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    auto as_state = std::make_shared<ACCELERATION_STRUCTURE_STATE>(*pAccelerationStructure, pCreateInfo);

    // Query the requirements in case the application doesn't (to avoid bind/validation time query)
    VkAccelerationStructureMemoryRequirementsInfoNV as_memory_requirements_info = {};
    as_memory_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    as_memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    as_memory_requirements_info.accelerationStructure = as_state->acceleration_structure;
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &as_memory_requirements_info, &as_state->memory_requirements);

    VkAccelerationStructureMemoryRequirementsInfoNV scratch_memory_req_info = {};
    scratch_memory_req_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    scratch_memory_req_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    scratch_memory_req_info.accelerationStructure = as_state->acceleration_structure;
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &scratch_memory_req_info,
                                                         &as_state->build_scratch_memory_requirements);

    VkAccelerationStructureMemoryRequirementsInfoNV update_memory_req_info = {};
    update_memory_req_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    update_memory_req_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
    update_memory_req_info.accelerationStructure = as_state->acceleration_structure;
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &update_memory_req_info,
                                                         &as_state->update_scratch_memory_requirements);

    accelerationStructureMap[*pAccelerationStructure] = std::move(as_state);
}

void ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
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

void ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(info.accelerationStructure);
        if (as_state) {
            // Track bound memory range information
            auto mem_info = GetDevMemState(info.memory);
            if (mem_info) {
                InsertAccelerationStructureMemoryRange(info.accelerationStructure, mem_info, info.memoryOffset,
                                                       as_state->requirements);
            }
            // Track objects tied to memory
            SetMemBinding(info.memory, as_state, info.memoryOffset,
                          VulkanTypedHandle(info.accelerationStructure, kVulkanObjectTypeAccelerationStructureNV));

            // GPU validation of top level acceleration structure building needs acceleration structure handles.
            if (enabled.gpu_validation) {
                DispatchGetAccelerationStructureHandleNV(device, info.accelerationStructure, 8, &as_state->opaque_handle);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (cb_state == nullptr) {
        return;
    }

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
    cb_state->hasBuildAccelerationStructureCmd = true;
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                          VkAccelerationStructureNV dst,
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

void ValidationStateTracker::PreCallRecordDestroyAccelerationStructureNV(VkDevice device,
                                                                         VkAccelerationStructureNV accelerationStructure,
                                                                         const VkAllocationCallbacks *pAllocator) {
    if (!accelerationStructure) return;
    auto *as_state = GetAccelerationStructureState(accelerationStructure);
    if (as_state) {
        const VulkanTypedHandle obj_struct(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV);
        InvalidateCommandBuffers(as_state->cb_bindings, obj_struct);
        for (auto mem_binding : as_state->GetBoundMemory()) {
            auto mem_info = GetDevMemState(mem_binding);
            if (mem_info) {
                RemoveAccelerationStructureMemoryRange(accelerationStructure, mem_info);
            }
        }
        ClearMemoryObjectBindings(obj_struct);
        as_state->destroyed = true;
        accelerationStructureMap.erase(accelerationStructure);
    }
}

void ValidationStateTracker::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkViewportWScalingNV *pViewportWScalings) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_VIEWPORT_W_SCALING_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_LINE_WIDTH_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                               uint16_t lineStipplePattern) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_LINE_STIPPLE_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                          float depthBiasClamp, float depthBiasSlopeFactor) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_DEPTH_BIAS_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_BLEND_CONSTANTS_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                            float maxDepthBounds) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_DEPTH_BOUNDS_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                   uint32_t compareMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_READ_MASK_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t writeMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_WRITE_MASK_SET;
}

void ValidationStateTracker::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t reference) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->status |= CBSTATUS_STENCIL_REFERENCE_SET;
}

// Update pipeline_layout bind points applying the "Pipeline Layout Compatibility" rules.
// One of pDescriptorSets or push_descriptor_set should be nullptr, indicating whether this
// is called for CmdBindDescriptorSets or CmdPushDescriptorSet.
void ValidationStateTracker::UpdateLastBoundDescriptorSets(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint pipeline_bind_point,
                                                           const PIPELINE_LAYOUT_STATE *pipeline_layout, uint32_t first_set,
                                                           uint32_t set_count, const VkDescriptorSet *pDescriptorSets,
                                                           cvdescriptorset::DescriptorSet *push_descriptor_set,
                                                           uint32_t dynamic_offset_count, const uint32_t *p_dynamic_offsets) {
    assert((pDescriptorSets == nullptr) ^ (push_descriptor_set == nullptr));
    // Defensive
    assert(pipeline_layout);
    if (!pipeline_layout) return;

    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    assert(last_binding_index < pipeline_layout->compat_for_set.size());

    // Some useful shorthand
    auto &last_bound = cb_state->lastBound[pipeline_bind_point];
    auto &pipe_compat_ids = pipeline_layout->compat_for_set;
    const uint32_t current_size = static_cast<uint32_t>(last_bound.per_set.size());

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
        if (last_bound.per_set[last_binding_index].compat_id_for_set != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (push_descriptor_cleanup(last_bound.per_set[set_idx].bound_descriptor_set)) break;
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        last_bound.per_set.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        if (last_bound.per_set[set_idx].compat_id_for_set != pipe_compat_ids[set_idx]) {
            push_descriptor_cleanup(last_bound.per_set[set_idx].bound_descriptor_set);
            last_bound.per_set[set_idx].bound_descriptor_set = nullptr;
            last_bound.per_set[set_idx].dynamicOffsets.clear();
            last_bound.per_set[set_idx].compat_id_for_set = pipe_compat_ids[set_idx];
        }
    }

    // Now update the bound sets with the input sets
    const uint32_t *input_dynamic_offsets = p_dynamic_offsets;  // "read" pointer for dynamic offset data
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        cvdescriptorset::DescriptorSet *descriptor_set =
            push_descriptor_set ? push_descriptor_set : GetSetNode(pDescriptorSets[input_idx]);

        // Record binding (or push)
        if (descriptor_set != last_bound.push_descriptor_set.get()) {
            // Only cleanup the push descriptors if they aren't the currently used set.
            push_descriptor_cleanup(last_bound.per_set[set_idx].bound_descriptor_set);
        }
        last_bound.per_set[set_idx].bound_descriptor_set = descriptor_set;
        last_bound.per_set[set_idx].compat_id_for_set = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index

        if (descriptor_set) {
            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            // TODO: Add logic for tracking push_descriptor offsets (here or in caller)
            if (set_dynamic_descriptor_count && input_dynamic_offsets) {
                const uint32_t *end_offset = input_dynamic_offsets + set_dynamic_descriptor_count;
                last_bound.per_set[set_idx].dynamicOffsets = std::vector<uint32_t>(input_dynamic_offsets, end_offset);
                input_dynamic_offsets = end_offset;
                assert(input_dynamic_offsets <= (p_dynamic_offsets + dynamic_offset_count));
            } else {
                last_bound.per_set[set_idx].dynamicOffsets.clear();
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

    // Resize binding arrays
    uint32_t last_set_index = firstSet + setCount - 1;
    if (last_set_index >= cb_state->lastBound[pipelineBindPoint].per_set.size()) {
        cb_state->lastBound[pipelineBindPoint].per_set.resize(last_set_index + 1);
    }

    UpdateLastBoundDescriptorSets(cb_state, pipelineBindPoint, pipeline_layout, firstSet, setCount, pDescriptorSets, nullptr,
                                  dynamicOffsetCount, pDynamicOffsets);
    cb_state->lastBound[pipelineBindPoint].pipeline_layout = layout;
    ResetCommandBufferPushConstantDataIfIncompatible(cb_state, layout);
}

void ValidationStateTracker::RecordCmdPushDescriptorSetState(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint pipelineBindPoint,
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
    if (!push_descriptor_set || !CompatForSet(set, last_bound, pipeline_layout->compat_for_set)) {
        last_bound.UnbindAndResetPushDescriptorSet(new cvdescriptorset::DescriptorSet(0, nullptr, dsl, 0, this, report_data));
    }

    UpdateLastBoundDescriptorSets(cb_state, pipelineBindPoint, pipeline_layout, set, 1, nullptr, push_descriptor_set.get(), 0,
                                  nullptr);
    last_bound.pipeline_layout = layout;

    // Now that we have either the new or extant push_descriptor set ... do the write updates against it
    push_descriptor_set->PerformPushDescriptorsUpdate(this, descriptorWriteCount, pDescriptorWrites);
}

void ValidationStateTracker::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                  uint32_t set, uint32_t descriptorWriteCount,
                                                                  const VkWriteDescriptorSet *pDescriptorWrites) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdPushDescriptorSetState(cb_state, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

void ValidationStateTracker::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                            VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                            const void *pValues) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (cb_state != nullptr) {
        ResetCommandBufferPushConstantDataIfIncompatible(cb_state, layout);

        auto &push_constant_data = cb_state->push_constant_data;
        assert((offset + size) <= static_cast<uint32_t>(push_constant_data.size()));
        std::memcpy(push_constant_data.data() + offset, pValues, static_cast<std::size_t>(size));
    }
}

void ValidationStateTracker::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

void ValidationStateTracker::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer *pBuffers,
                                                               const VkDeviceSize *pOffsets) {
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

void ValidationStateTracker::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    auto cb_state = GetCBState(commandBuffer);
    auto dst_buffer_state = GetBufferState(dstBuffer);

    // Update bindings between buffer and cmd buffer
    AddCommandBufferBindingBuffer(cb_state, dst_buffer_state);
}

bool ValidationStateTracker::SetEventStageMask(VkEvent event, VkPipelineStageFlags stageMask,
                                               EventToStageMap *localEventToStageMap) {
    (*localEventToStageMap)[event] = stageMask;
    return false;
}

void ValidationStateTracker::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                      VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto event_state = GetEventState(event);
    if (event_state) {
        AddCommandBufferBinding(event_state->cb_bindings, VulkanTypedHandle(event, kVulkanObjectTypeEvent, event_state), cb_state);
    }
    cb_state->events.push_back(event);
    if (!cb_state->waitedEvents.count(event)) {
        cb_state->writeEventsBeforeWait.push_back(event);
    }
    cb_state->eventUpdates.emplace_back(
        [event, stageMask](const ValidationStateTracker *device_data, bool do_validate, EventToStageMap *localEventToStageMap) {
            return SetEventStageMask(event, stageMask, localEventToStageMap);
        });
}

void ValidationStateTracker::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto event_state = GetEventState(event);
    if (event_state) {
        AddCommandBufferBinding(event_state->cb_bindings, VulkanTypedHandle(event, kVulkanObjectTypeEvent, event_state), cb_state);
    }
    cb_state->events.push_back(event);
    if (!cb_state->waitedEvents.count(event)) {
        cb_state->writeEventsBeforeWait.push_back(event);
    }

    cb_state->eventUpdates.emplace_back(
        [event](const ValidationStateTracker *, bool do_validate, EventToStageMap *localEventToStageMap) {
            return SetEventStageMask(event, VkPipelineStageFlags(0), localEventToStageMap);
        });
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                        VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                        uint32_t bufferMemoryBarrierCount,
                                                        const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                        uint32_t imageMemoryBarrierCount,
                                                        const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    for (uint32_t i = 0; i < eventCount; ++i) {
        auto event_state = GetEventState(pEvents[i]);
        if (event_state) {
            AddCommandBufferBinding(event_state->cb_bindings, VulkanTypedHandle(pEvents[i], kVulkanObjectTypeEvent, event_state),
                                    cb_state);
        }
        cb_state->waitedEvents.insert(pEvents[i]);
        cb_state->events.push_back(pEvents[i]);
    }
}

bool ValidationStateTracker::SetQueryState(QueryObject object, QueryState value, QueryMap *localQueryToStateMap) {
    (*localQueryToStateMap)[object] = value;
    return false;
}

bool ValidationStateTracker::SetQueryStateMulti(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, QueryState value,
                                                QueryMap *localQueryToStateMap) {
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryObject object = {queryPool, firstQuery + i};
        (*localQueryToStateMap)[object] = value;
    }
    return false;
}

QueryState ValidationStateTracker::GetQueryState(const QueryMap *localQueryToStateMap, VkQueryPool queryPool,
                                                 uint32_t queryIndex) const {
    QueryObject query = {queryPool, queryIndex};

    const std::array<const decltype(queryToStateMap) *, 2> map_list = {localQueryToStateMap, &queryToStateMap};

    for (const auto map : map_list) {
        auto query_data = map->find(query);
        if (query_data != map->end()) {
            return query_data->second;
        }
    }
    return QUERYSTATE_UNKNOWN;
}

void ValidationStateTracker::RecordCmdBeginQuery(CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj) {
    if (disabled.query_validation) return;
    cb_state->activeQueries.insert(query_obj);
    cb_state->startedQueries.insert(query_obj);
    cb_state->queryUpdates.emplace_back(
        [query_obj](const ValidationStateTracker *device_data, bool do_validate, QueryMap *localQueryToStateMap) {
            SetQueryState(query_obj, QUERYSTATE_RUNNING, localQueryToStateMap);
            return false;
        });
    auto pool_state = GetQueryPoolState(query_obj.pool);
    AddCommandBufferBinding(pool_state->cb_bindings, VulkanTypedHandle(query_obj.pool, kVulkanObjectTypeQueryPool, pool_state),
                            cb_state);
}

void ValidationStateTracker::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                         VkFlags flags) {
    if (disabled.query_validation) return;
    QueryObject query = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdBeginQuery(cb_state, query);
}

void ValidationStateTracker::RecordCmdEndQuery(CMD_BUFFER_STATE *cb_state, const QueryObject &query_obj) {
    if (disabled.query_validation) return;
    cb_state->activeQueries.erase(query_obj);
    cb_state->queryUpdates.emplace_back(
        [query_obj](const ValidationStateTracker *device_data, bool do_validate, QueryMap *localQueryToStateMap) {
            return SetQueryState(query_obj, QUERYSTATE_ENDED, localQueryToStateMap);
        });
    auto pool_state = GetQueryPoolState(query_obj.pool);
    AddCommandBufferBinding(pool_state->cb_bindings, VulkanTypedHandle(query_obj.pool, kVulkanObjectTypeQueryPool, pool_state),
                            cb_state);
}

void ValidationStateTracker::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled.query_validation) return;
    QueryObject query_obj = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdEndQuery(cb_state, query_obj);
}

void ValidationStateTracker::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount) {
    if (disabled.query_validation) return;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    cb_state->queryUpdates.emplace_back([queryPool, firstQuery, queryCount](const ValidationStateTracker *device_data,
                                                                            bool do_validate, QueryMap *localQueryToStateMap) {
        return SetQueryStateMulti(queryPool, firstQuery, queryCount, QUERYSTATE_RESET, localQueryToStateMap);
    });
    auto pool_state = GetQueryPoolState(queryPool);
    AddCommandBufferBinding(pool_state->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool, pool_state),
                            cb_state);
}

void ValidationStateTracker::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                                   VkDeviceSize dstOffset, VkDeviceSize stride,
                                                                   VkQueryResultFlags flags) {
    if (disabled.query_validation) return;
    auto cb_state = GetCBState(commandBuffer);
    auto dst_buff_state = GetBufferState(dstBuffer);
    AddCommandBufferBindingBuffer(cb_state, dst_buff_state);
    auto pool_state = GetQueryPoolState(queryPool);
    AddCommandBufferBinding(pool_state->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool, pool_state),
                            cb_state);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkQueryPool queryPool, uint32_t slot) {
    if (disabled.query_validation) return;
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto pool_state = GetQueryPoolState(queryPool);
    AddCommandBufferBinding(pool_state->cb_bindings, VulkanTypedHandle(queryPool, kVulkanObjectTypeQueryPool, pool_state),
                            cb_state);
    QueryObject query = {queryPool, slot};
    cb_state->queryUpdates.emplace_back(
        [query](const ValidationStateTracker *device_data, bool do_validate, QueryMap *localQueryToStateMap) {
            return SetQueryState(query, QUERYSTATE_ENDED, localQueryToStateMap);
        });
}

void ValidationStateTracker::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    // Shadow create info and store in map
    auto fb_state = std::make_shared<FRAMEBUFFER_STATE>(*pFramebuffer, pCreateInfo, GetRenderPassShared(pCreateInfo->renderPass));

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

void ValidationStateTracker::RecordRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR *pCreateInfo,
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

static void MarkAttachmentFirstUse(RENDER_PASS_STATE *render_pass, uint32_t index, bool is_read) {
    if (index == VK_ATTACHMENT_UNUSED) return;

    if (!render_pass->attachment_first_read.count(index)) render_pass->attachment_first_read[index] = is_read;
}

void ValidationStateTracker::RecordCreateRenderPassState(RenderPassCreateVersion rp_version,
                                                         std::shared_ptr<RENDER_PASS_STATE> &render_pass,
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
void ValidationStateTracker::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    auto render_pass_state = std::make_shared<RENDER_PASS_STATE>(pCreateInfo);
    RecordCreateRenderPassState(RENDER_PASS_VERSION_1, render_pass_state, pRenderPass);
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    auto render_pass_state = std::make_shared<RENDER_PASS_STATE>(pCreateInfo);
    RecordCreateRenderPassState(RENDER_PASS_VERSION_2, render_pass_state, pRenderPass);
}

void ValidationStateTracker::RecordCmdBeginRenderPassState(VkCommandBuffer commandBuffer,
                                                           const VkRenderPassBeginInfo *pRenderPassBegin,
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
        AddCommandBufferBinding(render_pass_state->cb_bindings,
                                VulkanTypedHandle(render_pass_state->renderPass, kVulkanObjectTypeRenderPass, render_pass_state),
                                cb_state);

        auto chained_device_group_struct = lvl_find_in_chain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
        if (chained_device_group_struct) {
            cb_state->active_render_pass_device_mask = chained_device_group_struct->deviceMask;
        } else {
            cb_state->active_render_pass_device_mask = cb_state->initial_device_mask;
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                                             VkSubpassContents contents) {
    RecordCmdBeginRenderPassState(commandBuffer, pRenderPassBegin, contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                 const VkSubpassBeginInfoKHR *pSubpassBeginInfo) {
    RecordCmdBeginRenderPassState(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->activeSubpass++;
    cb_state->activeSubpassContents = contents;
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    RecordCmdNextSubpass(commandBuffer, contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
                                                              const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                              const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::RecordCmdEndRenderPassState(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->activeRenderPass = nullptr;
    cb_state->activeSubpass = 0;
    cb_state->activeFramebuffer = VK_NULL_HANDLE;
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderPassState(commandBuffer);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    RecordCmdEndRenderPassState(commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
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
        // NOTE: The update/population of the image_layout_map is done in CoreChecks, but for other classes derived from
        // ValidationStateTracker these maps will be empty, so leaving the propagation in the the state tracker should be a no-op
        // for those other classes.
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

void ValidationStateTracker::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                                     VkFlags flags, void **ppData, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordMappedMemory(mem, offset, size, ppData);
}

void ValidationStateTracker::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void ValidationStateTracker::UpdateBindImageMemoryState(const VkBindImageMemoryInfo &bindInfo) {
    IMAGE_STATE *image_state = GetImageState(bindInfo.image);
    if (image_state) {
        const auto swapchain_info = lvl_find_in_chain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            auto swapchain = GetSwapchainState(swapchain_info->swapchain);
            if (swapchain) {
                swapchain->images[swapchain_info->imageIndex].bound_images.emplace(image_state->image);
                image_state->bind_swapchain = swapchain_info->swapchain;
                image_state->bind_swapchain_imageIndex = swapchain_info->imageIndex;
            }
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
        if (image_state->createInfo.flags & VK_IMAGE_CREATE_ALIAS_BIT) {
            AddAliasingImage(image_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                           VkDeviceSize memoryOffset, VkResult result) {
    if (VK_SUCCESS != result) return;
    VkBindImageMemoryInfo bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindInfo.image = image;
    bindInfo.memory = mem;
    bindInfo.memoryOffset = memoryOffset;
    UpdateBindImageMemoryState(bindInfo);
}

void ValidationStateTracker::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindImageMemoryInfoKHR *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void ValidationStateTracker::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                               const VkBindImageMemoryInfoKHR *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void ValidationStateTracker::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    auto event_state = GetEventState(event);
    if (event_state) {
        event_state->stageMask = VK_PIPELINE_STAGE_HOST_BIT;
    }
}

void ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(VkDevice device,
                                                                const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreFdInfo->semaphore, pImportSemaphoreFdInfo->handleType,
                               pImportSemaphoreFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalSemaphoreState(VkSemaphore semaphore,
                                                             VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type) {
    SEMAPHORE_STATE *semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state && handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        semaphore_state->scope = kSyncScopeExternalPermanent;
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreWin32HandleInfo->semaphore, pImportSemaphoreWin32HandleInfo->handleType,
                               pImportSemaphoreWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                      const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                      HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetWin32HandleInfo->semaphore, pGetWin32HandleInfo->handleType);
}

void ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice device, const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceWin32HandleInfo->fence, pImportFenceWin32HandleInfo->handleType,
                           pImportFenceWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(VkDevice device,
                                                                  const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                  HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetWin32HandleInfo->fence, pGetWin32HandleInfo->handleType);
}
#endif

void ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetFdInfo->semaphore, pGetFdInfo->handleType);
}

void ValidationStateTracker::RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type,
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

void ValidationStateTracker::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type) {
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

void ValidationStateTracker::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetFdInfo->fence, pGetFdInfo->handleType);
}

void ValidationStateTracker::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, VkResult result) {
    if (VK_SUCCESS != result) return;
    eventMap[*pEvent].write_in_use = 0;
    eventMap[*pEvent].stageMask = VkPipelineStageFlags(0);
}

void ValidationStateTracker::RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                        VkSwapchainKHR *pSwapchain, SURFACE_STATE *surface_state,
                                                        SWAPCHAIN_NODE *old_swapchain_state) {
    if (VK_SUCCESS == result) {
        auto swapchain_state = std::make_shared<SWAPCHAIN_NODE>(pCreateInfo, *pSwapchain);
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

void ValidationStateTracker::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                              VkResult result) {
    auto surface_state = GetSurfaceState(pCreateInfo->surface);
    auto old_swapchain_state = GetSwapchainState(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(result, pCreateInfo, pSwapchain, surface_state, old_swapchain_state);
}

void ValidationStateTracker::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              const VkAllocationCallbacks *pAllocator) {
    if (!swapchain) return;
    auto swapchain_data = GetSwapchainState(swapchain);
    if (swapchain_data) {
        for (const auto &swapchain_image : swapchain_data->images) {
            ClearMemoryObjectBindings(VulkanTypedHandle(swapchain_image.image, kVulkanObjectTypeImage));
            imageMap.erase(swapchain_image.image);
            RemoveAliasingImages(swapchain_image.bound_images);
        }

        auto surface_state = GetSurfaceState(swapchain_data->createInfo.surface);
        if (surface_state) {
            if (surface_state->swapchain == swapchain_data) surface_state->swapchain = nullptr;
        }
        swapchain_data->destroyed = true;
        swapchainMap.erase(swapchain);
    }
}

void ValidationStateTracker::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) {
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
            auto image = swapchain_data->images[pPresentInfo->pImageIndices[i]].image;
            auto image_state = GetImageState(image);
            if (image_state) {
                image_state->acquired = false;
                if (image_state->shared_presentable) {
                    image_state->layout_locked = true;
                }
            }
        }
    }
    // Note: even though presentation is directed to a queue, there is no direct ordering between QP and subsequent work, so QP (and
    // its semaphore waits) /never/ participate in any completion proof.
}

void ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                     const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkSwapchainKHR *pSwapchains, VkResult result) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = GetSurfaceState(pCreateInfos[i].surface);
            auto old_swapchain_state = GetSwapchainState(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(result, &pCreateInfos[i], &pSwapchains[i], surface_state, old_swapchain_state);
        }
    }
}

void ValidationStateTracker::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
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
        auto image = swapchain_data->images[*pImageIndex].image;
        auto image_state = GetImageState(image);
        if (image_state) {
            image_state->acquired = true;
            image_state->shared_presentable = swapchain_data->shared_presentable;
        }
    }
}

void ValidationStateTracker::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                               VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                               VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

void ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                uint32_t *pImageIndex, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex);
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                            pQueueFamilyProperties);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                            pQueueFamilyProperties);
}
void ValidationStateTracker::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!surface) return;
    auto surface_state = GetSurfaceState(surface);
    surface_state->destroyed = true;
    surface_map.erase(surface);
}

void ValidationStateTracker::RecordVulkanSurface(VkSurfaceKHR *pSurface) {
    surface_map[*pSurface] = std::make_shared<SURFACE_STATE>(*pSurface);
}

void ValidationStateTracker::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                        const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSurfaceKHR *pSurface, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance,
                                                                   const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_IOS_MVK
void ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance,
                                                                 const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance,
                                                                   const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance,
                                                                 const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
void ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                                     VkPhysicalDeviceFeatures *pFeatures) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    physical_device_state->features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physical_device_state->features2.pNext = nullptr;
    physical_device_state->features2.features = *pFeatures;
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                                      VkPhysicalDeviceFeatures2 *pFeatures) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    physical_device_state->features2.initialize(pFeatures);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceFeatures2 *pFeatures) {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    physical_device_state->features2.initialize(pFeatures);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    physical_device_state->surfaceCapabilities = *pSurfaceCapabilities;
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = QUERY_DETAILS;
    physical_device_state->surfaceCapabilities = pSurfaceCapabilities->surfaceCapabilities;
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                    VkSurfaceKHR surface,
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                              VkBool32 *pSupported, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto surface_state = GetSurfaceState(surface);
    surface_state->gpu_queue_support[{physicalDevice, queueFamilyIndex}] = (*pSupported == VK_TRUE);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pPresentModeCount,
                                                                                   VkPresentModeKHR *pPresentModes,
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              uint32_t *pSurfaceFormatCount,
                                                                              VkSurfaceFormatKHR *pSurfaceFormats,
                                                                              VkResult result) {
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               uint32_t *pSurfaceFormatCount,
                                                                               VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                               VkResult result) {
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

void ValidationStateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                     const VkDebugUtilsLabelEXT *pLabelInfo) {
    BeginCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);
}

void ValidationStateTracker::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    EndCmdDebugUtilsLabel(report_data, commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                      const VkDebugUtilsLabelEXT *pLabelInfo) {
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

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                          VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                          const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    template_state->destroyed = true;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                             VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                                             const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    template_state->destroyed = true;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                                       VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    safe_VkDescriptorUpdateTemplateCreateInfo local_create_info(pCreateInfo);
    auto template_state = std::make_shared<TEMPLATE_STATE>(*pDescriptorUpdateTemplate, &local_create_info);
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

void ValidationStateTracker::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set,
    const void *pData) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);

    const auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    if (template_state) {
        auto layout_data = GetPipelineLayout(layout);
        auto dsl = GetDslFromPipelineLayout(layout_data, set);
        const auto &template_ci = template_state->create_info;
        if (dsl && !dsl->destroyed) {
            // Decode the template into a set of write updates
            cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state, pData,
                                                                    dsl->GetDescriptorSetLayout());
            RecordCmdPushDescriptorSetState(cb_state, template_ci.pipelineBindPoint, layout, set,
                                            static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                            decoded_template.desc_writes.data());
        }
    }
}

void ValidationStateTracker::RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice,
                                                                                uint32_t *pPropertyCount, void *pProperties) {
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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t *pPropertyCount,
                                                                                      VkDisplayPlanePropertiesKHR *pProperties,
                                                                                      VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                       uint32_t *pPropertyCount,
                                                                                       VkDisplayPlaneProperties2KHR *pProperties,
                                                                                       VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdBeginQuery(cb_state, query_obj);
}

void ValidationStateTracker::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                 uint32_t query, uint32_t index) {
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    RecordCmdEndQuery(cb_state, query_obj);
}

void ValidationStateTracker::RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                                     VkSamplerYcbcrConversion ycbcr_conversion) {
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordCreateSamplerYcbcrConversionANDROID(create_info, ycbcr_conversion);
    }
}

void ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                                        const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                        VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                           const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                           const VkAllocationCallbacks *pAllocator,
                                                                           VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                         const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordDestroySamplerYcbcrConversionANDROID(ycbcrConversion);
    }
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                                            VkSamplerYcbcrConversion ycbcrConversion,
                                                                            const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    if (device_extensions.vk_android_external_memory_android_hardware_buffer) {
        RecordDestroySamplerYcbcrConversionANDROID(ycbcrConversion);
    }
}

void ValidationStateTracker::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                             uint32_t queryCount) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.host_query_reset_features.hostQueryReset) return;

    // Do nothing if the query pool has been destroyed.
    auto query_pool_state = GetQueryPoolState(queryPool);
    if (!query_pool_state) return;

    // Reset the state of existing entries.
    QueryObject query_obj{queryPool, 0};
    const uint32_t max_query_count = std::min(queryCount, query_pool_state->createInfo.queryCount - firstQuery);
    for (uint32_t i = 0; i < max_query_count; ++i) {
        query_obj.query = firstQuery + i;
        auto query_it = queryToStateMap.find(query_obj);
        if (query_it != queryToStateMap.end()) query_it->second = QUERYSTATE_RESET;
    }
}

void ValidationStateTracker::PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                                        const TEMPLATE_STATE *template_state, const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    cvdescriptorset::PerformUpdateDescriptorSets(this, static_cast<uint32_t>(decoded_update.desc_writes.size()),
                                                 decoded_update.desc_writes.data(), 0, NULL);
}

// Update the common AllocateDescriptorSetsData
void ValidationStateTracker::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                              cvdescriptorset::AllocateDescriptorSetsData *ds_data) const {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayoutShared(p_alloc_info->pSetLayouts[i]);
        if (layout) {
            ds_data->layout_nodes[i] = layout;
            // Count total descriptors required per type
            for (uint32_t j = 0; j < layout->GetBindingCount(); ++j) {
                const auto &binding_layout = layout->GetDescriptorSetLayoutBindingPtrFromIndex(j);
                uint32_t typeIndex = static_cast<uint32_t>(binding_layout->descriptorType);
                ds_data->required_descriptors_by_type[typeIndex] += binding_layout->descriptorCount;
            }
        }
        // Any unknown layouts will be flagged as errors during ValidateAllocateDescriptorSets() call
    }
}

// Decrement allocated sets from the pool and insert new sets into set_map
void ValidationStateTracker::PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                           const VkDescriptorSet *descriptor_sets,
                                                           const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    auto pool_state = descriptorPoolMap[p_alloc_info->descriptorPool].get();
    // Account for sets and individual descriptors allocated from pool
    pool_state->availableSets -= p_alloc_info->descriptorSetCount;
    for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
        pool_state->availableDescriptorTypeCount[it->first] -= ds_data->required_descriptors_by_type.at(it->first);
    }

    const auto *variable_count_info = lvl_find_in_chain<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>(p_alloc_info->pNext);
    bool variable_count_valid = variable_count_info && variable_count_info->descriptorSetCount == p_alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = std::make_shared<cvdescriptorset::DescriptorSet>(descriptor_sets[i], pool_state, ds_data->layout_nodes[i],
                                                                       variable_count, this, report_data);
        pool_state->sets.insert(new_ds.get());
        new_ds->in_use.store(0);
        setMap[descriptor_sets[i]] = std::move(new_ds);
    }
}

// Generic function to handle state update for all CmdDraw* and CmdDispatch* type functions
void ValidationStateTracker::UpdateStateCmdDrawDispatchType(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateDrawState(cb_state, bind_point);
    cb_state->hasDispatchCmd = true;
}

// Generic function to handle state update for all CmdDraw* type functions
void ValidationStateTracker::UpdateStateCmdDrawType(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point) {
    UpdateStateCmdDrawDispatchType(cb_state, bind_point);
    cb_state->hasDrawCmd = true;
}

void ValidationStateTracker::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

void ValidationStateTracker::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void ValidationStateTracker::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                               VkDeviceSize offset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
    AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                         VkDeviceSize offset, VkBuffer countBuffer,
                                                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                         uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
    AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                             uint32_t firstTask) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                          VkDeviceSize offset, VkBuffer countBuffer,
                                                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                          uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    UpdateStateCmdDrawType(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, buffer_state);
    }
    if (count_buffer_state) {
        AddCommandBufferBindingBuffer(cb_state, count_buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkShaderModule *pShaderModule, VkResult result,
                                                              void *csm_state_data) {
    if (VK_SUCCESS != result) return;
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);

    spv_target_env spirv_environment = ((api_version >= VK_API_VERSION_1_1) ? SPV_ENV_VULKAN_1_1 : SPV_ENV_VULKAN_1_0);
    bool is_spirv = (pCreateInfo->pCode[0] == spv::MagicNumber);
    auto new_shader_module = is_spirv ? std::make_shared<SHADER_MODULE_STATE>(pCreateInfo, *pShaderModule, spirv_environment,
                                                                              csm_state->unique_shader_id)
                                      : std::make_shared<SHADER_MODULE_STATE>();
    shaderModuleMap[*pShaderModule] = std::move(new_shader_module);
}

void ValidationStateTracker::RecordPipelineShaderStage(VkPipelineShaderStageCreateInfo const *pStage, PIPELINE_STATE *pipeline,
                                                       PIPELINE_STATE::StageState *stage_state) const {
    // Validation shouldn't rely on anything in stage state being valid if the spirv isn't
    auto module = GetShaderModuleState(pStage->module);
    if (!module->has_valid_spirv) return;

    // Validation shouldn't rely on anything in stage state being valid if the entrypoint isn't present
    auto entrypoint = FindEntrypoint(module, pStage->pName, pStage->stage);
    if (entrypoint == module->end()) return;

    // Mark accessible ids
    stage_state->accessible_ids = MarkAccessibleIds(module, entrypoint);
    ProcessExecutionModes(module, entrypoint, pipeline);

    stage_state->descriptor_uses =
        CollectInterfaceByDescriptorSlot(report_data, module, stage_state->accessible_ids, &stage_state->has_writable_descriptor);
    // Capture descriptor uses for the pipeline
    for (auto use : stage_state->descriptor_uses) {
        // While validating shaders capture which slots are used by the pipeline
        const uint32_t slot = use.first.first;
        auto &reqs = pipeline->active_slots[slot][use.first.second];
        reqs = descriptor_req(reqs | DescriptorTypeToReqs(module, use.second.type_id));
        pipeline->max_active_slot = std::max(pipeline->max_active_slot, slot);
    }
}

void ValidationStateTracker::ResetCommandBufferPushConstantDataIfIncompatible(CMD_BUFFER_STATE *cb_state, VkPipelineLayout layout) {
    if (cb_state == nullptr) {
        return;
    }

    const PIPELINE_LAYOUT_STATE *pipeline_layout_state = GetPipelineLayout(layout);
    if (pipeline_layout_state == nullptr) {
        return;
    }

    if (cb_state->push_constant_data_ranges != pipeline_layout_state->push_constant_ranges) {
        cb_state->push_constant_data_ranges = pipeline_layout_state->push_constant_ranges;
        cb_state->push_constant_data.clear();
        uint32_t size_needed = 0;
        for (auto push_constant_range : *cb_state->push_constant_data_ranges) {
            size_needed = std::max(size_needed, (push_constant_range.offset + push_constant_range.size));
        }
        cb_state->push_constant_data.resize(size_needed, 0);
    }
}

void ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages,
                                                                 VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    auto swapchain_state = GetSwapchainState(swapchain);

    if (*pSwapchainImageCount > swapchain_state->images.size()) swapchain_state->images.resize(*pSwapchainImageCount);

    if (pSwapchainImages) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_DETAILS) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_DETAILS;
        }
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            if (swapchain_state->images[i].image != VK_NULL_HANDLE) continue;  // Already retrieved this.

            // Add imageMap entries for each swapchain image
            VkImageCreateInfo image_ci;
            image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_ci.pNext = nullptr;                    // to be set later
            image_ci.flags = VK_IMAGE_CREATE_ALIAS_BIT;  // to be updated below
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
            image_ci.queueFamilyIndexCount = swapchain_state->createInfo.queueFamilyIndexCount;
            image_ci.pQueueFamilyIndices = swapchain_state->createInfo.pQueueFamilyIndices;
            image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            image_ci.pNext = lvl_find_in_chain<VkImageFormatListCreateInfoKHR>(swapchain_state->createInfo.pNext);

            if (swapchain_state->createInfo.flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)
                image_ci.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
            if (swapchain_state->createInfo.flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
                image_ci.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
            if (swapchain_state->createInfo.flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)
                image_ci.flags |= (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR);

            imageMap[pSwapchainImages[i]] = std::make_shared<IMAGE_STATE>(pSwapchainImages[i], &image_ci);
            auto &image_state = imageMap[pSwapchainImages[i]];
            image_state->valid = false;
            image_state->create_from_swapchain = swapchain;
            image_state->bind_swapchain = swapchain;
            image_state->bind_swapchain_imageIndex = i;
            swapchain_state->images[i].image = pSwapchainImages[i];
            swapchain_state->images[i].bound_images.emplace(pSwapchainImages[i]);
        }
    }

    if (*pSwapchainImageCount) {
        if (swapchain_state->vkGetSwapchainImagesKHRState < QUERY_COUNT) {
            swapchain_state->vkGetSwapchainImagesKHRState = QUERY_COUNT;
        }
        swapchain_state->get_swapchain_image_count = *pSwapchainImageCount;
    }
}
