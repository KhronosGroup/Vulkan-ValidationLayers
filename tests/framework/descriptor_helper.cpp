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

#include "descriptor_helper.h"

OneOffDescriptorSet::OneOffDescriptorSet(vkt::Device *device, const Bindings &bindings,
                                         VkDescriptorSetLayoutCreateFlags layout_flags, void *layout_pnext,
                                         VkDescriptorPoolCreateFlags poolFlags, void *allocate_pnext)
    : device_{device}, pool_{}, layout_(*device, bindings, layout_flags, layout_pnext), set_(VK_NULL_HANDLE) {
    VkResult err;
    std::vector<VkDescriptorPoolSize> sizes;
    for (const auto &b : bindings) sizes.push_back({b.descriptorType, std::max(1u, b.descriptorCount)});

    VkDescriptorPoolCreateInfo dspci = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, poolFlags, 1, uint32_t(sizes.size()), sizes.data()};
    err = vk::CreateDescriptorPool(device_->handle(), &dspci, nullptr, &pool_);
    if (err != VK_SUCCESS) return;

    if ((layout_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) == 0) {
        VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, allocate_pnext, pool_, 1,
                                                  &layout_.handle()};
        err = vk::AllocateDescriptorSets(device_->handle(), &alloc_info, &set_);
    }
}

OneOffDescriptorSet::~OneOffDescriptorSet() {
    // No need to destroy set-- it's going away with the pool.
    vk::DestroyDescriptorPool(device_->handle(), pool_, nullptr);
}

bool OneOffDescriptorSet::Initialized() { return pool_ != VK_NULL_HANDLE && layout_.initialized() && set_ != VK_NULL_HANDLE; }

void OneOffDescriptorSet::Clear() {
    resource_infos.clear();
    descriptor_writes.clear();
}

void OneOffDescriptorSet::AddDescriptorWrite(uint32_t binding, uint32_t array_element, VkDescriptorType descriptor_type) {
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = array_element;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_writes.emplace_back(descriptor_write);
}

void OneOffDescriptorSet::WriteDescriptorBufferInfo(int binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                                    VkDescriptorType descriptorType, uint32_t arrayElement) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer;
    buffer_info.offset = offset;
    buffer_info.range = range;

    ResourceInfo resource_info;
    resource_info.buffer_info = buffer_info;
    resource_infos.emplace_back(resource_info);
    AddDescriptorWrite(binding, arrayElement, descriptorType);
}

void OneOffDescriptorSet::WriteDescriptorBufferView(int binding, VkBufferView buffer_view, VkDescriptorType descriptorType,
                                                    uint32_t arrayElement) {
    ResourceInfo resource_info;
    resource_info.buffer_view = buffer_view;
    resource_infos.emplace_back(resource_info);
    AddDescriptorWrite(binding, arrayElement, descriptorType);
}

void OneOffDescriptorSet::WriteDescriptorImageInfo(int binding, VkImageView image_view, VkSampler sampler,
                                                   VkDescriptorType descriptorType, VkImageLayout imageLayout,
                                                   uint32_t arrayElement) {
    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image_view;
    image_info.sampler = sampler;
    image_info.imageLayout = imageLayout;

    ResourceInfo resource_info;
    resource_info.image_info = image_info;
    resource_infos.emplace_back(resource_info);
    AddDescriptorWrite(binding, arrayElement, descriptorType);
}

void OneOffDescriptorSet::UpdateDescriptorSets() {
    assert(resource_infos.size() == descriptor_writes.size());
    for (size_t i = 0; i < resource_infos.size(); i++) {
        const auto &info = resource_infos[i];
        if (info.image_info.has_value()) {
            descriptor_writes[i].pImageInfo = &info.image_info.value();
        } else if (info.buffer_info.has_value()) {
            descriptor_writes[i].pBufferInfo = &info.buffer_info.value();
        } else {
            assert(info.buffer_view.has_value());
            descriptor_writes[i].pTexelBufferView = &info.buffer_view.value();
        }
    }
    vk::UpdateDescriptorSets(device_->handle(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);
}
