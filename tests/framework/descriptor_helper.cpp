/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "descriptor_helper.h"
#include <vulkan/utility/vk_struct_helper.hpp>
#include "binding.h"

OneOffDescriptorSet::OneOffDescriptorSet(vkt::Device *device, const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                                         VkDescriptorSetLayoutCreateFlags layout_flags, void *layout_pnext,
                                         VkDescriptorPoolCreateFlags pool_flags, void *allocate_pnext, void *create_pool_pnext)
    : device_{device}, layout_(*device, bindings, layout_flags, layout_pnext) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto &b : bindings) {
        pool_sizes.emplace_back(VkDescriptorPoolSize{b.descriptorType, std::max(1u, b.descriptorCount)});
    }

    VkDescriptorPoolCreateInfo pool_ci = vku::InitStructHelper(create_pool_pnext);
    pool_ci.flags = pool_flags;
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = pool_sizes.size();
    pool_ci.pPoolSizes = pool_sizes.data();
    VkResult err = vk::CreateDescriptorPool(device_->handle(), &pool_ci, nullptr, &pool_);
    if (err != VK_SUCCESS) return;

    if ((layout_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) == 0) {
        VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(allocate_pnext);
        ds_alloc_info.descriptorPool = pool_;
        ds_alloc_info.descriptorSetCount = 1;
        ds_alloc_info.pSetLayouts = &layout_.handle();
        err = vk::AllocateDescriptorSets(device_->handle(), &ds_alloc_info, &set_);
    }
}

OneOffDescriptorIndexingSet::OneOffDescriptorIndexingSet(vkt::Device *device, const OneOffDescriptorIndexingSet::Bindings &bindings,
                                                         void *allocate_pnext, void *create_pool_pnext) {
    device_ = device;
    std::vector<VkDescriptorPoolSize> pool_sizes;
    VkDescriptorSetLayoutCreateFlags layout_flags = 0;
    VkDescriptorPoolCreateFlags pool_flags = 0;
    std::vector<VkDescriptorSetLayoutBinding> ds_layout_bindings;
    std::vector<VkDescriptorBindingFlags> ds_binding_flags;

    for (const auto &b : bindings) {
        pool_sizes.emplace_back(VkDescriptorPoolSize{b.descriptorType, std::max(1u, b.descriptorCount)});
        ds_layout_bindings.emplace_back(
            VkDescriptorSetLayoutBinding{b.binding, b.descriptorType, b.descriptorCount, b.stageFlags, b.pImmutableSamplers});
        ds_binding_flags.emplace_back(b.flag);

        // Automatically set the needed flags if using UAB
        if (b.flag & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
            layout_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            pool_flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        }
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = ds_binding_flags.size();
    flags_create_info.pBindingFlags = ds_binding_flags.data();

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = layout_flags;
    ds_layout_ci.bindingCount = ds_layout_bindings.size();
    ds_layout_ci.pBindings = ds_layout_bindings.data();
    layout_.Init(*device, ds_layout_ci);

    VkDescriptorPoolCreateInfo pool_ci = vku::InitStructHelper(create_pool_pnext);
    pool_ci.flags = pool_flags;
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = pool_sizes.size();
    pool_ci.pPoolSizes = pool_sizes.data();
    VkResult err = vk::CreateDescriptorPool(device_->handle(), &pool_ci, nullptr, &pool_);
    if (err != VK_SUCCESS) return;

    if ((layout_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) == 0) {
        VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(allocate_pnext);
        ds_alloc_info.descriptorPool = pool_;
        ds_alloc_info.descriptorSetCount = 1;
        ds_alloc_info.pSetLayouts = &layout_.handle();
        err = vk::AllocateDescriptorSets(device_->handle(), &ds_alloc_info, &set_);
    }
}

OneOffDescriptorSet::OneOffDescriptorSet(OneOffDescriptorSet &&rhs) noexcept { *this = std::move(rhs); }

OneOffDescriptorSet &OneOffDescriptorSet::operator=(OneOffDescriptorSet &&rhs) noexcept {
    device_ = rhs.device_;
    rhs.device_ = nullptr;

    pool_ = rhs.pool_;
    rhs.pool_ = VK_NULL_HANDLE;

    layout_ = std::move(rhs.layout_);

    set_ = rhs.set_;
    rhs.set_ = VK_NULL_HANDLE;

    resource_infos = std::move(rhs.resource_infos);
    descriptor_writes = std::move(rhs.descriptor_writes);
    return *this;
}

OneOffDescriptorSet::~OneOffDescriptorSet() {
    if (pool_ != VK_NULL_HANDLE) {
        // No need to destroy set-- it's going away with the pool.
        vk::DestroyDescriptorPool(device_->handle(), pool_, nullptr);
    }
}

bool OneOffDescriptorSet::Initialized() { return pool_ != VK_NULL_HANDLE && layout_.initialized() && set_ != VK_NULL_HANDLE; }

void OneOffDescriptorSet::Clear() {
    resource_infos.clear();
    descriptor_writes.clear();
}

void OneOffDescriptorSet::AddDescriptorWrite(uint32_t binding, uint32_t array_element, VkDescriptorType descriptor_type,
                                             uint32_t descriptor_count /*= 1*/) {
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = set_;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = array_element;
    descriptor_write.descriptorCount = descriptor_count;
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

void OneOffDescriptorSet::WriteDescriptorAccelStruct(int binding, uint32_t accelerationStructureCount,
                                                     const VkAccelerationStructureKHR *pAccelerationStructures,
                                                     uint32_t arrayElement /*= 0*/) {
    VkWriteDescriptorSetAccelerationStructureKHR write_desc_set_accel_struct = vku::InitStructHelper();
    write_desc_set_accel_struct.accelerationStructureCount = accelerationStructureCount;
    write_desc_set_accel_struct.pAccelerationStructures = pAccelerationStructures;

    ResourceInfo resource_info;
    resource_info.accel_struct_info = write_desc_set_accel_struct;
    resource_infos.emplace_back(resource_info);
    AddDescriptorWrite(binding, arrayElement, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, accelerationStructureCount);
}

void OneOffDescriptorSet::UpdateDescriptorSets() {
    assert(resource_infos.size() == descriptor_writes.size());
    for (size_t i = 0; i < resource_infos.size(); i++) {
        const auto &info = resource_infos[i];
        if (info.image_info.has_value()) {
            descriptor_writes[i].pImageInfo = &info.image_info.value();
        } else if (info.buffer_info.has_value()) {
            descriptor_writes[i].pBufferInfo = &info.buffer_info.value();
        } else if (info.buffer_view.has_value()) {
            descriptor_writes[i].pTexelBufferView = &info.buffer_view.value();
        } else {
            assert(info.accel_struct_info.has_value());
            descriptor_writes[i].pNext = &info.accel_struct_info.value();
        }
    }
    vk::UpdateDescriptorSets(device_->handle(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);
}

namespace vkt {
DescriptorGetInfo::DescriptorGetInfo(VkSampler *sampler) {
    get_info.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    get_info.data.pSampler = sampler;
}

DescriptorGetInfo::DescriptorGetInfo(VkDeviceAddress address) {
    get_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    get_info.data.accelerationStructure = address;
}

DescriptorGetInfo::DescriptorGetInfo(VkDescriptorType type, VkSampler sampler, VkImageView image_view, VkImageLayout image_layout) {
    image_info = {sampler, image_view, image_layout};
    get_info.type = type;
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        get_info.data.pCombinedImageSampler = &image_info;
    } else if (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        get_info.data.pInputAttachmentImage = &image_info;
    } else if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
        get_info.data.pSampledImage = &image_info;
    } else if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
        get_info.data.pStorageImage = &image_info;
    } else {
        assert(false);
    }
}

DescriptorGetInfo::DescriptorGetInfo(VkDescriptorType type, VkDeviceAddress address, VkDeviceSize range, VkFormat format) {
    address_info = vku::InitStructHelper();
    address_info.address = address;
    address_info.range = range;
    address_info.format = format;
    get_info.type = type;
    if (type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
        get_info.data.pUniformTexelBuffer = &address_info;
    } else if (type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
        get_info.data.pStorageTexelBuffer = &address_info;
    } else if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        get_info.data.pUniformBuffer = &address_info;
    } else if (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        get_info.data.pStorageBuffer = &address_info;
    } else {
        assert(false);
    }
}

DescriptorGetInfo::DescriptorGetInfo(VkDescriptorType type, const vkt::Buffer &buffer, VkDeviceSize range, VkFormat format)
    : DescriptorGetInfo(type, buffer.Address(), range, format) {}
}  // namespace vkt
