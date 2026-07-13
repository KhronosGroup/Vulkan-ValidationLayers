/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#pragma once

#include <vulkan/vulkan_core.h>
#include "binding.h"

class VkLayerTest;

namespace vkt {

class Buffer;
namespace as {
class AccelerationStructureKHR;
}

class DescriptorHeap {
  public:
    static void AddDescriptorHeapRequirements(VkLayerTest& test);
    static void AddUntypedDescriptorHeapRequirements(VkLayerTest &test);

    DescriptorHeap(VkLayerTest& test);
    void CreateResourceHeap(VkDeviceSize app_size, bool reserved_range_in_front = false);
    void CreateSamplerHeap(VkDeviceSize app_size, bool reserved_range_in_front = false, bool use_embedded_samplers = false);

    // Writes a descriptor at the internally maintained heap write offset.
    // Heap write offset is first aligned according to descriptor type,
    // then descriptor is written, finally heap write offset is incremented by descriptor size.
    // Returns write offset.
    VkDeviceSize WriteBufferDescriptor(const vkt::Buffer& buffer, VkDescriptorType desc_type);
    VkDeviceSize WriteBufferDescriptor(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type);
    VkDeviceSize WriteImageDescriptor(const vkt::Image& image, VkDescriptorType desc_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                      VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkDeviceSize WriteAccelerationStructureDescriptor(vkt::as::AccelerationStructureKHR& as);
    VkDeviceSize WriteAccelerationStructureDescriptor(VkDeviceAddress as_addr);
    VkDeviceSize WriteSamplerDescriptor(VkSamplerCreateInfo* sampler_create_info = nullptr);
    // Write at supplied heap offset
    // *Warning* Internally maintained heap write offset will not be touched at all,
    // so this probably should not be used with the above functions.
    VkDeviceSize WriteBufferDescriptorAtOffset(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type,
                                               VkDeviceSize heap_offset);
    VkDeviceSize WriteImageDescriptorAtOffset(const vkt::Image& image, VkDeviceSize heap_offset,
                                              VkDescriptorType desc_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                              VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkDeviceSize WriteNullDescriptorAtOffset(VkDescriptorType desc_type, VkDeviceSize heap_offset);

    VkDeviceSize AlignResource(VkDeviceSize offset);
    VkDeviceSize AlignSampler(VkDeviceSize offset);

    VkDeviceSize GetCurrentHeapWriteOffset() const { return heap_offset_; }
    VkDeviceSize GetResourceHeapReservedRangeOffset() const;
    VkDeviceSize GetSamplerHeapReservedRangeOffset() const;

    void BindResourceHeap(vkt::CommandBuffer &cmd_buffer);
    void BindSamplerHeap(vkt::CommandBuffer &cmd_buffer);

    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceDescriptorHeapTensorPropertiesARM tensor_heap_props = vku::InitStructHelper();

    vkt::Buffer resource_heap_;
    uint8_t* resource_heap_data_ = nullptr;

    vkt::Buffer sampler_heap_;
    uint8_t *sampler_heap_data_ = nullptr;

  private:
    VkDeviceSize WriteDescriptorAtOffset(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type, VkDeviceSize heap_offset);
    VkDeviceSize WriteDescriptorAtOffset(const VkImageDescriptorInfoEXT* image_info, VkDescriptorType desc_type,
                                         VkDeviceSize heap_offset);

    VkLayerTest* test_ = nullptr;
    bool resource_reserved_range_in_front_ = false;
    bool sampler_reserved_range_in_front_ = false;
    bool embedded_samplers = false;
    VkDeviceSize heap_offset_ = 0;
    VkDeviceSize sampler_heap_offset_ = 0;
};

}  // namespace vkt
