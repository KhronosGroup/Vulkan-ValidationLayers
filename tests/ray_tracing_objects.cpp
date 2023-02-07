/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "ray_tracing_objects.h"

namespace rt {
namespace as {

GeometryKHR::GeometryKHR(uint32_t vk_api_version) : vk_api_version_(vk_api_version), vk_obj_(LvlInitStruct<decltype(vk_obj_)>()) {}

GeometryKHR &GeometryKHR::SetType(Type type) {
    type_ = type;
    vk_obj_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    vk_obj_.pNext = nullptr;
    switch (type_) {
        case Type::Triangle:
            vk_obj_.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            vk_obj_.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            vk_obj_.geometry.triangles.pNext = nullptr;
            vk_obj_.geometry.triangles.transformData = {0};
            break;
        case Type::AABB:
            vk_obj_.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
            vk_obj_.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
            vk_obj_.geometry.aabbs.pNext = nullptr;
            break;
        case Type::_INTERNAL_UNSPECIFIED:
            assert(false);
            break;
    }
    return *this;
}

GeometryKHR &GeometryKHR::SetPrimitiveCount(uint32_t primitiveCount) {
    primitiveCount_ = primitiveCount;
    return *this;
}

GeometryKHR &GeometryKHR::SetStride(VkDeviceSize stride) {
    switch (type_) {
        case Type::Triangle:
            vk_obj_.geometry.triangles.vertexStride = stride;
            break;
        case Type::AABB:
            vk_obj_.geometry.aabbs.stride = stride;
            break;
        case Type::_INTERNAL_UNSPECIFIED:
            assert(false);
            break;
    }
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesVertexBuffer(vk_testing::Buffer &&vertex_buffer, uint32_t max_vertex,
                                                   VkFormat vertex_format /*= VK_FORMAT_R32G32B32_SFLOAT*/,
                                                   VkDeviceSize stride /*= 3 * sizeof(float)*/) {
    triangles_.vertex_buffer = std::move(vertex_buffer);
    vk_obj_.geometry.triangles.vertexFormat = vertex_format;
    vk_obj_.geometry.triangles.vertexData.deviceAddress = triangles_.vertex_buffer.address(vk_api_version_);
    vk_obj_.geometry.triangles.maxVertex = max_vertex;
    vk_obj_.geometry.triangles.vertexStride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesIndexBuffer(vk_testing::Buffer &&index_buffer,
                                                  VkIndexType index_type /*= VK_INDEX_TYPE_UINT32*/) {
    triangles_.index_buffer = std::move(index_buffer);
    vk_obj_.geometry.triangles.indexType = index_type;
    vk_obj_.geometry.triangles.indexData.deviceAddress = triangles_.index_buffer.address(vk_api_version_);
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesIndexType(VkIndexType index_type) {
    vk_obj_.geometry.triangles.indexType = index_type;
    return *this;
}

GeometryKHR &GeometryKHR::SetAABBsBuffer(vk_testing::Buffer &&buffer, VkDeviceSize stride /*= sizeof(VkAabbPositionsKHR)*/) {
    aabbs_.buffer = std::move(buffer);
    vk_obj_.geometry.aabbs.data.deviceAddress = aabbs_.buffer.address(vk_api_version_);
    vk_obj_.geometry.aabbs.stride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::Build() { return *this; }

VkAccelerationStructureBuildRangeInfoKHR GeometryKHR::GetFullBuildRange() const {
    VkAccelerationStructureBuildRangeInfoKHR range_info{};
    range_info.primitiveCount = primitiveCount_;
    range_info.primitiveOffset = 0;
    range_info.firstVertex = 0;
    range_info.transformOffset = 0;
    assert(range_info.primitiveCount > 0);  // 0 could be a valid value, as of writing it is considered invalid
    return range_info;
}

AccelerationStructureKHR::AccelerationStructureKHR(uint32_t vk_api_version)
    : vk_api_version_(vk_api_version), vk_info_(LvlInitStruct<decltype(vk_info_)>()), buffer_() {}

AccelerationStructureKHR &AccelerationStructureKHR::SetSize(VkDeviceSize size) {
    vk_info_.size = size;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetOffset(VkDeviceSize offset) {
    vk_info_.offset = offset;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetType(VkAccelerationStructureTypeKHR type) {
    vk_info_.type = type;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetFlags(VkAccelerationStructureCreateFlagsKHR flags) {
    vk_info_.createFlags = flags;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetBuffer(vk_testing::Buffer &&buffer) {
    buffer_ = std::move(buffer);
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetBufferMemoryAllocateFlags(VkMemoryAllocateFlags memory_allocate_flags) {
    buffer_memory_allocate_flags_ = memory_allocate_flags;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetBufferMemoryPropertyFlags(VkMemoryAllocateFlags memory_property_flags_) {
    buffer_memory_property_flags_ = memory_property_flags_;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetBufferUsageFlags(VkBufferUsageFlags usage_flags) {
    buffer_usage_flags_ = usage_flags;
    return *this;
}

VkDeviceAddress AccelerationStructureKHR::GetBufferDeviceAddress() const {
    assert(initialized());
    assert(buffer_.initialized());
    assert(buffer_.create_info().size > 0);
    return buffer_.address(vk_api_version_);
}

void AccelerationStructureKHR::Build(const vk_testing::Device &device) {
    assert(handle() == VK_NULL_HANDLE);

    // Create a buffer to store acceleration structure
    if (!buffer_.initialized() && (buffer_usage_flags_ != 0)) {
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = buffer_memory_allocate_flags_;
        buffer_.init(device, vk_info_.size, buffer_memory_property_flags_, buffer_usage_flags_, &alloc_flags);
    }
    vk_info_.buffer = buffer_.handle();

    // Create acceleration structure
    auto vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device.handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR);
    VkAccelerationStructureKHR handle;
    const VkResult result = vkCreateAccelerationStructureKHR(device.handle(), &vk_info_, nullptr, &handle);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        init(device.handle(), handle);
    }
}

void AccelerationStructureKHR::Destroy() {
    if (!initialized()) {
        return;
    }
    assert(device() != VK_NULL_HANDLE);
    assert(handle() != VK_NULL_HANDLE);
    auto vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureKHR"));
    assert(vkDestroyAccelerationStructureKHR);
    vkDestroyAccelerationStructureKHR(device(), handle(), nullptr);
    handle_ = VK_NULL_HANDLE;
    buffer_.destroy();
}

BuildGeometryInfoKHR::BuildGeometryInfoKHR(uint32_t vk_api_version)
    : vk_api_version_(vk_api_version),
      vk_info_(LvlInitStruct<decltype(vk_info_)>()),
      geometries_(),
      src_as_(std::make_shared<AccelerationStructureKHR>(vk_api_version)),
      dst_as_(std::make_shared<AccelerationStructureKHR>(vk_api_version)),
      scratch_() {}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetGeometries(std::vector<GeometryKHR> &&geometries) {
    geometries_ = std::move(geometries);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetType(VkAccelerationStructureTypeKHR type) {
    src_as_->SetType(type);
    dst_as_->SetType(type);
    vk_info_.type = type;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetMode(VkBuildAccelerationStructureModeKHR mode) {
    vk_info_.mode = mode;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetFlags(VkBuildAccelerationStructureFlagsKHR flags) {
    vk_info_.flags = flags;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::AddFlags(VkBuildAccelerationStructureFlagsKHR flags) {
    vk_info_.flags |= flags;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetSrcAS(std::shared_ptr<AccelerationStructureKHR> src_as) {
    assert(src_as);  // nullptr not supported
    src_as_ = std::move(src_as);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetDstAS(std::shared_ptr<AccelerationStructureKHR> dst_as) {
    assert(dst_as);  // nullptr not supported
    dst_as_ = std::move(dst_as);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetScratchBuffer(vk_testing::Buffer &&scratch_buffer) {
    scratch_ = std::move(scratch_buffer);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetInfoCount(uint32_t info_count) {
    assert(info_count <= 1);  // TODO - support array of VkAccelerationStructureBuildGeometryInfoKHR
    vk_info_count_ = info_count;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetNullInfos(bool use_null_infos) {
    use_null_infos_ = use_null_infos;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetNullBuildRangeInfos(bool use_null_build_range_infos) {
    use_null_build_range_infos_ = use_null_build_range_infos;
    return *this;
}

void BuildGeometryInfoKHR::BuildCmdBuffer(const vk_testing::Device &device, VkCommandBuffer cmd_buffer,
                                          bool use_ppGeometries /*= true*/) {
    BuildCommon(device);
    VkCmdBuildAccelerationStructuresKHR(device, cmd_buffer, true);
}

void BuildGeometryInfoKHR::BuildCmdBufferIndirect(const vk_testing::Device &device, VkCommandBuffer cmd_buffer) {
    BuildCommon(device);
    VkCmdBuildAccelerationStructuresIndirectKHR(device, cmd_buffer);
}

void BuildGeometryInfoKHR::BuildHost(VkInstance instance, const vk_testing::Device &device) {
    BuildCommon(device);
    VkBuildAccelerationStructuresKHR(instance, device);
}

void BuildGeometryInfoKHR::VkCmdBuildAccelerationStructuresKHR(const vk_testing::Device &device, VkCommandBuffer cmd_buffer,
                                                               bool use_ppGeometries /*= true*/) {
    // fill vk_info_ with geometry data, and get build ranges
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    if (use_ppGeometries) {
        pGeometries.resize(geometries_.size());
    } else {
        geometries.resize(geometries_.size());
    }
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(geometries_.size());
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> pRange_infos(geometries_.size());
    pGeometries.reserve(geometries_.size());
    for (size_t i = 0; i < geometries_.size(); ++i) {
        const auto &geometry = geometries_[i];
        if (use_ppGeometries) {
            pGeometries[i] = &geometry.GetVkObj();
        } else {
            geometries[i] = geometry.GetVkObj();
        }
        range_infos[i] = geometry.GetFullBuildRange();
        pRange_infos[i] = &range_infos[i];
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    if (use_ppGeometries) {
        vk_info_.ppGeometries = pGeometries.data();
    } else {
        vk_info_.pGeometries = geometries.data();
    }

    // Build acceleration structure
    auto vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vk::GetDeviceProcAddr(device.handle(), "vkCmdBuildAccelerationStructuresKHR"));
    assert(vkCmdBuildAccelerationStructuresKHR);
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos = use_null_infos_ ? nullptr : &vk_info_;
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos =
        use_null_build_range_infos_ ? nullptr : pRange_infos.data();
    vkCmdBuildAccelerationStructuresKHR(cmd_buffer, vk_info_count_, pInfos, ppBuildRangeInfos);

    // pGeometries and geometries are going to be destroyed
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;
}

void BuildGeometryInfoKHR::VkCmdBuildAccelerationStructuresIndirectKHR(const vk_testing::Device &device,
                                                                       VkCommandBuffer cmd_buffer) {
    // fill vk_info_ with geometry data, and get build ranges
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries(geometries_.size());
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(geometries_.size());
    pGeometries.reserve(geometries_.size());
    for (size_t i = 0; i < geometries_.size(); ++i) {
        const auto &geometry = geometries_[i];
        pGeometries[i] = &geometry.GetVkObj();
        range_infos[i] = geometry.GetFullBuildRange();
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    vk_info_.ppGeometries = pGeometries.data();

    auto vkCmdBuildAccelerationStructuresIndirectKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresIndirectKHR>(
        vk::GetDeviceProcAddr(device.handle(), "vkCmdBuildAccelerationStructuresIndirectKHR"));
    assert(vkCmdBuildAccelerationStructuresIndirectKHR);

    VkDeviceAddress indirect_device_addresses{};
    uint32_t indirect_strides = sizeof(VkAccelerationStructureBuildRangeInfoKHR);
    uint32_t max_prim_counts[1] = {1};

    vkCmdBuildAccelerationStructuresIndirectKHR(cmd_buffer, vk_info_count_, &vk_info_, &indirect_device_addresses,
                                                &indirect_strides, reinterpret_cast<uint32_t **>(&max_prim_counts));

    // pGeometries and geometries are going to be destroyed
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;
}

void BuildGeometryInfoKHR::VkBuildAccelerationStructuresKHR(VkInstance instance, const vk_testing::Device &device) {
    // fill vk_info_ with geometry data, and get build ranges
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries(geometries_.size());
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(geometries_.size());
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> pRange_infos(geometries_.size());
    pGeometries.reserve(geometries_.size());
    for (size_t i = 0; i < geometries_.size(); ++i) {
        const auto &geometry = geometries_[i];
        pGeometries[i] = &geometry.GetVkObj();
        range_infos[i] = geometry.GetFullBuildRange();
        pRange_infos[i] = &range_infos[i];
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    vk_info_.ppGeometries = pGeometries.data();

    // Build acceleration structure
    auto vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
        vk::GetInstanceProcAddr(instance, "vkBuildAccelerationStructuresKHR"));
    assert(vkBuildAccelerationStructuresKHR);
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos = use_null_infos_ ? nullptr : &vk_info_;
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos =
        use_null_build_range_infos_ ? nullptr : pRange_infos.data();
    vkBuildAccelerationStructuresKHR(device.handle(), VK_NULL_HANDLE, vk_info_count_, pInfos, ppBuildRangeInfos);

    // pGeometries is going to be destroyed
    vk_info_.ppGeometries = nullptr;
}

VkAccelerationStructureBuildSizesInfoKHR BuildGeometryInfoKHR::GetSizeInfo(VkDevice device, bool use_ppGeometries /*= true*/) {
    // Computer total primitives count, and get pointers to geometries
    uint32_t primitives_count = 0;
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    if (use_ppGeometries) {
        pGeometries.reserve(geometries_.size());
    } else {
        geometries.reserve(geometries_.size());
    }
    for (const auto &geometry : geometries_) {
        primitives_count += geometry.GetFullBuildRange().primitiveCount;
        if (use_ppGeometries) {
            pGeometries.emplace_back(&geometry.GetVkObj());
        } else {
            geometries.emplace_back(geometry.GetVkObj());
        }
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    if (use_ppGeometries) {
        vk_info_.ppGeometries = pGeometries.data();
    } else {
        vk_info_.pGeometries = geometries.data();
    }

    // Get VkAccelerationStructureBuildSizesInfoKHR using this->vk_info_
    auto vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
        vk::GetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
    assert(vkGetAccelerationStructureBuildSizesKHR);
    auto size_info = LvlInitStruct<VkAccelerationStructureBuildSizesInfoKHR>();
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &vk_info_, &primitives_count,
                                            &size_info);

    // pGeometries and geometries are going to be destroyed
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;

    return size_info;
}

void BuildGeometryInfoKHR::BuildCommon(const vk_testing::Device &device, bool use_ppGeometries /*= true*/) {
    assert(vk_info_.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR ||
           vk_info_.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);

    // Build geometries
    for (auto &geometry : geometries_) {
        geometry.Build();
    }

    // Build source and destination acceleration structures
    if (!src_as_->IsNull() && !src_as_->IsBuilt()) {
        src_as_->Build(device);
    }
    vk_info_.srcAccelerationStructure = src_as_->handle();
    if (!dst_as_->IsNull() && !dst_as_->IsBuilt()) {
        dst_as_->Build(device);
    }
    vk_info_.dstAccelerationStructure = dst_as_->handle();

    // Allocate scratch buffer
    if (!scratch_.initialized()) {
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        const VkAccelerationStructureBuildSizesInfoKHR size_info = GetSizeInfo(device.handle(), use_ppGeometries);
        const VkDeviceSize scratch_size = vk_info_.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
                                              ? size_info.buildScratchSize
                                              : size_info.updateScratchSize;
        if (scratch_size != 0) {
            scratch_.init(device, scratch_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &alloc_flags);
        }
    }
    if (scratch_.create_info().size != 0 && scratch_.create_info().usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        vk_info_.scratchData.deviceAddress = scratch_.address(vk_api_version_);
    } else {
        vk_info_.scratchData.deviceAddress = 0;
    }
}

namespace blueprint {
GeometryKHR GeometrySimpleTriangleInfo(uint32_t vk_api_version, const vk_testing::Device &device) {
    GeometryKHR triangle_geometry(vk_api_version);

    triangle_geometry.SetType(GeometryKHR::Type::Triangle);

    // Allocate vertex and index buffers
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vk_testing::Buffer vertex_buffer(device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     buffer_usage, &alloc_flags);
    vk_testing::Buffer index_buffer(device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    buffer_usage, &alloc_flags);

    // Fill vertex and index buffers with one triangle
    triangle_geometry.SetPrimitiveCount(1);
    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {{0, 1, 2}};

    auto mapped_vbo_buffer_data = static_cast<float *>(vertex_buffer.memory().map());
    std::copy(vertices.begin(), vertices.end(), mapped_vbo_buffer_data);
    vertex_buffer.memory().unmap();

    auto mapped_ibo_buffer_data = static_cast<uint32_t *>(index_buffer.memory().map());
    std::copy(indicies.begin(), indicies.end(), mapped_ibo_buffer_data);
    index_buffer.memory().unmap();

    // Assign vertex and index buffers to out geometry
    triangle_geometry.SetTrianglesVertexBuffer(std::move(vertex_buffer), uint32_t(vertices.size() / 3 - 1));
    triangle_geometry.SetTrianglesIndexBuffer(std::move(index_buffer));

    return triangle_geometry;
}

GeometryKHR GeometrySimpleAABBInfo(uint32_t vk_api_version, const vk_testing::Device &device) {
    GeometryKHR aabb_geometry(vk_api_version);

    aabb_geometry.SetType(GeometryKHR::Type::AABB);

    // Allocate buffer
    const std::array<VkAabbPositionsKHR, 1> aabbs = {{{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}}};

    const VkDeviceSize aabb_buffer_size = sizeof(aabbs[0]) * aabbs.size();
    vk_testing::Buffer aabb_buffer;
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    aabb_buffer.init(device, aabb_buffer_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     buffer_usage, &alloc_flags);

    // Fill buffer with one AABB
    aabb_geometry.SetPrimitiveCount(static_cast<uint32_t>(aabbs.size()));
    auto mapped_aabb_buffer_data = static_cast<VkAabbPositionsKHR *>(aabb_buffer.memory().map());
    std::copy(aabbs.begin(), aabbs.end(), mapped_aabb_buffer_data);
    aabb_buffer.memory().unmap();

    aabb_geometry.SetAABBsBuffer(std::move(aabb_buffer));

    return aabb_geometry;
}

std::shared_ptr<AccelerationStructureKHR> AccelStructNull(uint32_t vk_api_version) {
    auto as = std::make_shared<AccelerationStructureKHR>(vk_api_version);
    as->SetNull(true);
    return as;
}

std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceBottomLevel(uint32_t vk_api_version, VkDeviceSize size) {
    auto as = std::make_shared<AccelerationStructureKHR>(vk_api_version);
    as->SetSize(size);
    as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    as->SetBufferMemoryAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
    as->SetBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    as->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    return as;
}

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceBottomLevel(uint32_t vk_api_version, const vk_testing::Device &device,
                                                                GeometryKHR::Type geometry_type /*= GeometryKHR::Type::Triangle*/) {
    BuildGeometryInfoKHR out_build_info(vk_api_version);

    out_build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
    out_build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

    // Set geometry
    std::vector<GeometryKHR> geometries;
    switch (geometry_type) {
        case GeometryKHR::Type::Triangle:
            geometries.emplace_back(GeometrySimpleTriangleInfo(vk_api_version, device));
            break;
        case GeometryKHR::Type::AABB:
            geometries.emplace_back(GeometrySimpleAABBInfo(vk_api_version, device));
            break;
        case GeometryKHR::Type::_INTERNAL_UNSPECIFIED:
            assert(false);
            break;
    }
    out_build_info.SetGeometries(std::move(geometries));

    // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
    out_build_info.SetSrcAS(AccelStructNull(vk_api_version));
    auto dstAsSize = out_build_info.GetSizeInfo(device.handle()).accelerationStructureSize;
    out_build_info.SetDstAS(AccelStructSimpleOnDeviceBottomLevel(vk_api_version, dstAsSize));

    out_build_info.SetInfoCount(1);
    out_build_info.SetNullInfos(false);
    out_build_info.SetNullBuildRangeInfos(false);

    return out_build_info;
}

}  // namespace blueprint

}  // namespace as

}  // namespace rt
