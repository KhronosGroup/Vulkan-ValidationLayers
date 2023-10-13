/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"

struct OneOffDescriptorSet {
    vkt::Device *device_;
    VkDescriptorPool pool_;
    vkt::DescriptorSetLayout layout_;
    VkDescriptorSet set_;
    typedef std::vector<VkDescriptorSetLayoutBinding> Bindings;

    // Only one member of ResourceInfo object contains a value.
    // The pointers to Image/Buffer/BufferView info structures can't be stored in 'descriptor_writes'
    // during WriteDescriptor call, because subsequent calls can reallocate which invalidates stored pointers.
    // When UpdateDescriptorSets is called it's safe to initialize the pointers.
    struct ResourceInfo {
        std::optional<VkDescriptorImageInfo> image_info;
        std::optional<VkDescriptorBufferInfo> buffer_info;
        std::optional<VkBufferView> buffer_view;
    };
    std::vector<ResourceInfo> resource_infos;
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    OneOffDescriptorSet(vkt::Device *device, const Bindings &bindings, VkDescriptorSetLayoutCreateFlags layout_flags = 0,
                        void *layout_pnext = NULL, VkDescriptorPoolCreateFlags poolFlags = 0, void *allocate_pnext = NULL);
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
    void UpdateDescriptorSets();

  private:
    void AddDescriptorWrite(uint32_t binding, uint32_t array_element, VkDescriptorType descriptor_type);
};
