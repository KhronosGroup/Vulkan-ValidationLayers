/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "layer_validation_tests.h"

class OneOffDescriptorSet {
  public:
    vkt::Device *device_ = nullptr;
    VkDescriptorPool pool_{VK_NULL_HANDLE};
    vkt::DescriptorSetLayout layout_;
    VkDescriptorSet set_{VK_NULL_HANDLE};

    // Only one member of ResourceInfo object contains a value.
    // The pointers to Image/Buffer/BufferView info structures can't be stored in 'descriptor_writes'
    // during WriteDescriptor call, because subsequent calls can reallocate which invalidates stored pointers.
    // When UpdateDescriptorSets is called it's safe to initialize the pointers.
    struct ResourceInfo {
        std::optional<VkDescriptorImageInfo> image_info;
        std::optional<VkDescriptorBufferInfo> buffer_info;
        std::optional<VkBufferView> buffer_view;
        std::optional<VkWriteDescriptorSetAccelerationStructureKHR> accel_struct_info;
        std::optional<VkWriteDescriptorSetTensorARM> tensor_info;
    };
    std::vector<ResourceInfo> resource_infos;
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    OneOffDescriptorSet() = default;
    OneOffDescriptorSet(vkt::Device *device, const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                        VkDescriptorSetLayoutCreateFlags layout_flags = 0, void *layout_pnext = nullptr,
                        VkDescriptorPoolCreateFlags pool_flags = 0, void *allocate_pnext = nullptr,
                        void *create_pool_pnext = nullptr);

    OneOffDescriptorSet(OneOffDescriptorSet &&rhs) noexcept;
    OneOffDescriptorSet &operator=(OneOffDescriptorSet &&) noexcept;
    ~OneOffDescriptorSet();
    bool Initialized();
    void Clear();
    void WriteDescriptorBufferInfo(int binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t arrayElement = 0);
    void WriteDescriptorBufferView(int binding, VkBufferView buffer_view,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                   uint32_t arrayElement = 0);
    void WriteDescriptorImageInfo(int binding, VkImageView image_view, VkSampler sampler,
                                  VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayElement = 0);
    void WriteDescriptorAccelStruct(int binding, uint32_t accelerationStructureCount,
                                    const VkAccelerationStructureKHR *pAccelerationStructures, uint32_t arrayElement = 0);
    void WriteDescriptorTensorInfo(int binding, const VkTensorViewARM *view, uint32_t arrayElement = 0);
    void UpdateDescriptorSets();

  private:
    void AddDescriptorWrite(uint32_t binding, uint32_t array_element, VkDescriptorType descriptor_type,
                            uint32_t descriptor_count = 1);
};

// Descriptor Indexing focused variation
class OneOffDescriptorIndexingSet : public OneOffDescriptorSet {
  public:
    // Same as VkDescriptorSetLayoutBinding but ties the flags into it
    struct Binding {
        uint32_t binding;
        VkDescriptorType descriptorType;
        uint32_t descriptorCount;
        VkShaderStageFlags stageFlags;
        const VkSampler *pImmutableSamplers;
        VkDescriptorBindingFlags flag;
    };
    typedef std::vector<Binding> Bindings;

    OneOffDescriptorIndexingSet(vkt::Device *device, const Bindings &bindings, void *allocate_pnext = nullptr,
                                void *create_pool_pnext = nullptr);
};

namespace vkt {
class Buffer;

// VK_EXT_descriptor_buffer
struct DescriptorGetInfo {
    explicit DescriptorGetInfo(VkSampler *sampler);       // VK_DESCRIPTOR_TYPE_SAMPLER
    explicit DescriptorGetInfo(VkDeviceAddress address);  // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
    DescriptorGetInfo(VkDescriptorType type, VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
    DescriptorGetInfo(VkDescriptorType type, VkDeviceAddress address, VkDeviceSize range, VkFormat format = VK_FORMAT_UNDEFINED);
    DescriptorGetInfo(VkDescriptorType type, const vkt::Buffer &buffer, VkDeviceSize range, VkFormat format = VK_FORMAT_UNDEFINED);

    VkDescriptorGetInfoEXT get_info = vku::InitStructHelper();
    VkSampler sampler_handle;
    VkDescriptorImageInfo image_info;
    VkDescriptorAddressInfoEXT address_info;

    operator VkDescriptorGetInfoEXT *() { return &get_info; }
};

}  // namespace vkt
