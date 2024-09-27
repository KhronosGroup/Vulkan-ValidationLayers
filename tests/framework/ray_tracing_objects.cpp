/*
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "ray_tracing_objects.h"

#include "utils/vk_layer_utils.h"

#ifdef VVL_DEBUG_LOG_SBT
#include <iostream>
#endif

namespace vkt {
namespace as {

GeometryKHR::GeometryKHR() : vk_obj_(vku::InitStructHelper()) {}

GeometryKHR &GeometryKHR::SetFlags(VkGeometryFlagsKHR flags) {
    vk_obj_.flags = flags;
    return *this;
}

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
        case Type::Instance:
            vk_obj_.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            vk_obj_.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
            vk_obj_.geometry.instances.pNext = nullptr;
            break;
        case Type::_INTERNAL_UNSPECIFIED:
            [[fallthrough]];
        default:
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
        case Type::Instance:
            [[fallthrough]];
        case Type::_INTERNAL_UNSPECIFIED:
            [[fallthrough]];
        default:
            assert(false);
            break;
    }
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesDeviceVertexBuffer(vkt::Buffer &&vertex_buffer, uint32_t max_vertex,
                                                         VkFormat vertex_format /*= VK_FORMAT_R32G32B32_SFLOAT*/,
                                                         VkDeviceSize stride /*= 3 * sizeof(float)*/) {
    triangles_.device_vertex_buffer = std::move(vertex_buffer);
    vk_obj_.geometry.triangles.vertexFormat = vertex_format;
    vk_obj_.geometry.triangles.vertexData.deviceAddress = triangles_.device_vertex_buffer.address();
    vk_obj_.geometry.triangles.maxVertex = max_vertex;
    vk_obj_.geometry.triangles.vertexStride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesHostVertexBuffer(std::unique_ptr<float[]> &&vertex_buffer, uint32_t max_vertex,
                                                       VkDeviceSize stride /*= 3 * sizeof(float)*/) {
    triangles_.host_vertex_buffer = std::move(vertex_buffer);
    vk_obj_.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    vk_obj_.geometry.triangles.vertexData.hostAddress = triangles_.host_vertex_buffer.get();
    vk_obj_.geometry.triangles.maxVertex = max_vertex;
    vk_obj_.geometry.triangles.vertexStride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesDeviceIndexBuffer(vkt::Buffer &&index_buffer,
                                                        VkIndexType index_type /*= VK_INDEX_TYPE_UINT32*/) {
    triangles_.device_index_buffer = std::move(index_buffer);
    vk_obj_.geometry.triangles.indexType = index_type;
    vk_obj_.geometry.triangles.indexData.deviceAddress = triangles_.device_index_buffer.address();
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesHostIndexBuffer(std::unique_ptr<uint32_t[]> index_buffer) {
    triangles_.host_index_buffer = std::move(index_buffer);
    vk_obj_.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    vk_obj_.geometry.triangles.indexData.hostAddress = triangles_.host_index_buffer.get();
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesIndexType(VkIndexType index_type) {
    vk_obj_.geometry.triangles.indexType = index_type;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesVertexFormat(VkFormat vertex_format) {
    vk_obj_.geometry.triangles.vertexFormat = vertex_format;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesMaxVertex(uint32_t max_vertex) {
    vk_obj_.geometry.triangles.maxVertex = max_vertex;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesTransformBuffer(vkt::Buffer &&transform_buffer) {
    triangles_.device_transform_buffer = std::move(transform_buffer);
    vk_obj_.geometry.triangles.transformData.deviceAddress = triangles_.device_transform_buffer.address();
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesTransformatData(VkDeviceAddress address) {
    vk_obj_.geometry.triangles.transformData.deviceAddress = address;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesVertexBufferDeviceAddress(VkDeviceAddress address) {
    vk_obj_.geometry.triangles.vertexData.deviceAddress = address;
    return *this;
}

GeometryKHR &GeometryKHR::SetTrianglesIndexBufferDeviceAddress(VkDeviceAddress address) {
    vk_obj_.geometry.triangles.indexData.deviceAddress = address;
    return *this;
}

GeometryKHR &GeometryKHR::SetAABBsDeviceBuffer(vkt::Buffer &&buffer, VkDeviceSize stride /*= sizeof(VkAabbPositionsKHR)*/) {
    aabbs_.device_buffer = std::move(buffer);
    vk_obj_.geometry.aabbs.data.deviceAddress = aabbs_.device_buffer.address();
    vk_obj_.geometry.aabbs.stride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetAABBsHostBuffer(std::unique_ptr<VkAabbPositionsKHR[]> buffer,
                                             VkDeviceSize stride /*= sizeof(VkAabbPositionsKHR)*/) {
    aabbs_.host_buffer = std::move(buffer);
    vk_obj_.geometry.aabbs.data.hostAddress = aabbs_.host_buffer.get();
    vk_obj_.geometry.aabbs.stride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetAABBsStride(VkDeviceSize stride) {
    vk_obj_.geometry.aabbs.stride = stride;
    return *this;
}

GeometryKHR &GeometryKHR::SetAABBsDeviceAddress(VkDeviceAddress address) {
    vk_obj_.geometry.aabbs.data.deviceAddress = address;
    return *this;
}

GeometryKHR &GeometryKHR::AddInstanceDeviceAccelStructRef(const vkt::Device &device, VkAccelerationStructureKHR blas) {
    auto vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
        vk::GetDeviceProcAddr(device.handle(), "vkGetAccelerationStructureDeviceAddressKHR"));
    assert(vkGetAccelerationStructureDeviceAddressKHR);
    VkAccelerationStructureDeviceAddressInfoKHR as_address_info = vku::InitStructHelper();
    as_address_info.accelerationStructure = blas;
    const VkDeviceAddress as_address = vkGetAccelerationStructureDeviceAddressKHR(device.handle(), &as_address_info);
    VkAccelerationStructureInstanceKHR vk_instance{};

    // Ray Tracing gems 2, page 235, is a good reference on how to fill this affine transform
    // Noting M that transform, (x, y, z) a vertex, transformation is:
    // M * (x, y, z, 1) =
    //   ( M[0][0] * x + M[0][1] * y + M[0][2] * z + M[0][3],
    //     M[1][0] * x + M[1][1] * y + M[1][2] * z + M[1][3],
    //     M[2][0] * x + M[2][1] * y + M[2][2] * z + M[2][3] )

    // Identity transform:
    vk_instance.transform.matrix[0][0] = 1.0f;
    vk_instance.transform.matrix[1][1] = 1.0f;
    vk_instance.transform.matrix[2][2] = 1.0f;
    vk_instance.instanceCustomIndex = 0;
    // "mask is an 8-bit visibility mask for the geometry. The instance may only be hit if Cull Mask & instance.mask != 0"
    vk_instance.mask = 0xff;
    vk_instance.instanceShaderBindingTableRecordOffset = 0;
    vk_instance.flags = 0;  // Could use VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR
    vk_instance.accelerationStructureReference = static_cast<uint64_t>(as_address);
    instance_.vk_instances.push_back(vk_instance);
    ++primitiveCount_;

    // Create instance buffer
    assert(!instance_.buffer.initialized());  // for now, do not handle already initialized buffer
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    instance_.buffer.init(
        device, instance_.vk_instances.size() * sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    auto instance_buffer_ptr = static_cast<VkAccelerationStructureInstanceKHR *>(instance_.buffer.memory().map());
    for (size_t vk_instance_i = 0; vk_instance_i < instance_.vk_instances.size(); ++vk_instance_i) {
        instance_buffer_ptr[vk_instance_i] = instance_.vk_instances[vk_instance_i];
    }
    instance_.buffer.memory().unmap();

    vk_obj_.geometry.instances.arrayOfPointers = VK_FALSE;
    vk_obj_.geometry.instances.data.deviceAddress = instance_.buffer.address();
    return *this;
}

GeometryKHR &GeometryKHR::AddInstanceHostAccelStructRef(VkAccelerationStructureKHR blas) {
    instance_.vk_instances.emplace_back(VkAccelerationStructureInstanceKHR{});
    ++primitiveCount_;
    instance_.vk_instances.back().accelerationStructureReference = (uint64_t)(blas);
    // leave other instance_ attributes to 0

    vk_obj_.geometry.instances.arrayOfPointers = VK_FALSE;
    vk_obj_.geometry.instances.data.hostAddress = instance_.vk_instances.data();
    return *this;
}

GeometryKHR &GeometryKHR::SetInstancesDeviceAddress(VkDeviceAddress address) {
    vk_obj_.geometry.instances.data.deviceAddress = address;
    return *this;
}

GeometryKHR &GeometryKHR::SetInstanceHostAccelStructRef(VkAccelerationStructureKHR blas, uint32_t instance_i) {
    instance_.vk_instances[instance_i].accelerationStructureReference = (uint64_t)(blas);
    return *this;
}

GeometryKHR &GeometryKHR::SetInstanceHostAddress(void *address) {
    vk_obj_.geometry.instances.data.hostAddress = address;
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

AccelerationStructureKHR::AccelerationStructureKHR(const vkt::Device *device)
    : device_(device), vk_info_(vku::InitStructHelper()), device_buffer_() {}

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

AccelerationStructureKHR &AccelerationStructureKHR::SetDeviceBuffer(vkt::Buffer &&buffer) {
    device_buffer_ = std::move(buffer);
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetDeviceBufferMemoryAllocateFlags(
    VkMemoryAllocateFlags memory_allocate_flags) {
    buffer_memory_allocate_flags_ = memory_allocate_flags;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetDeviceBufferMemoryPropertyFlags(
    VkMemoryAllocateFlags memory_property_flags) {
    buffer_memory_property_flags_ = memory_property_flags;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetDeviceBufferInitNoMem(bool buffer_init_no_mem) {
    buffer_init_no_mem_ = buffer_init_no_mem;
    return *this;
}

AccelerationStructureKHR &AccelerationStructureKHR::SetBufferUsageFlags(VkBufferUsageFlags usage_flags) {
    buffer_usage_flags_ = usage_flags;
    return *this;
}

VkDeviceAddress AccelerationStructureKHR::GetBufferDeviceAddress() const {
    assert(initialized());
    assert(device_buffer_.initialized());
    assert(device_buffer_.CreateInfo().size > 0);
    return device_buffer_.address();
}

VkDeviceAddress AccelerationStructureKHR::GetAccelerationStructureDeviceAddress() const {
    VkAccelerationStructureDeviceAddressInfoKHR as_address_info = vku::InitStructHelper();
    as_address_info.accelerationStructure = handle();
    const VkDeviceAddress as_address = vk::GetAccelerationStructureDeviceAddressKHR(*device_, &as_address_info);
    return as_address;
}

void AccelerationStructureKHR::Build() {
    assert(handle() == VK_NULL_HANDLE);

    // Create a buffer to store acceleration structure
    if (!device_buffer_.initialized() && (buffer_usage_flags_ != 0)) {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = buffer_memory_allocate_flags_;
        VkBufferCreateInfo ci = vku::InitStructHelper();
        ci.size = vk_info_.size;
        ci.usage = buffer_usage_flags_;
        if (buffer_init_no_mem_) {
            device_buffer_.init_no_mem(*device_, ci);
        } else {
            device_buffer_.init(*device_, ci, buffer_memory_property_flags_, &alloc_flags);
        }
    }
    vk_info_.buffer = device_buffer_.handle();

    // Create acceleration structure
    VkAccelerationStructureKHR handle;
    const VkResult result = vk::CreateAccelerationStructureKHR(device_->handle(), &vk_info_, nullptr, &handle);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        init(device_->handle(), handle);
    }
}

void AccelerationStructureKHR::Destroy() {
    if (!initialized()) {
        return;
    }
    assert(device_->handle() != VK_NULL_HANDLE);
    assert(handle() != VK_NULL_HANDLE);
    vk::DestroyAccelerationStructureKHR(device_->handle(), handle(), nullptr);
    handle_ = VK_NULL_HANDLE;
    device_buffer_.destroy();
}

BuildGeometryInfoKHR::BuildGeometryInfoKHR(const vkt::Device *device)
    : device_(device),
      vk_info_(vku::InitStructHelper()),
      geometries_(),
      src_as_(std::make_shared<AccelerationStructureKHR>(device)),
      dst_as_(std::make_shared<AccelerationStructureKHR>(device)),
      device_scratch_(std::make_shared<vkt::Buffer>()) {}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetGeometries(std::vector<GeometryKHR> &&geometries) {
    geometries_ = std::move(geometries);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetBuildRanges(
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos) {
    build_range_infos_ = std::move(build_range_infos);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetType(VkAccelerationStructureTypeKHR type) {
    src_as_->SetType(type);
    dst_as_->SetType(type);
    vk_info_.type = type;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetBuildType(VkAccelerationStructureBuildTypeKHR build_type) {
    build_type_ = build_type;
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

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetScratchBuffer(std::shared_ptr<vkt::Buffer> scratch_buffer) {
    device_scratch_ = std::move(scratch_buffer);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetHostScratchBuffer(std::shared_ptr<std::vector<uint8_t>> host_scratch) {
    host_scratch_ = std::move(host_scratch);
    if (host_scratch_) {
        vk_info_.scratchData.hostAddress = host_scratch_->data();
    }
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetDeviceScratchOffset(VkDeviceAddress offset) {
    device_scratch_offset_ = offset;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetEnableScratchBuild(bool build_scratch) {
    build_scratch_ = build_scratch;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetBottomLevelAS(std::shared_ptr<BuildGeometryInfoKHR> bottom_level_as) {
    blas_ = std::move(bottom_level_as);
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetInfoCount(uint32_t info_count) {
    assert(info_count <= 1);
    vk_info_count_ = info_count;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetNullInfos(bool use_null_infos) {
    use_null_infos_ = use_null_infos;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetNullGeometries(bool use_null_geometries) {
    use_null_geometries_ = use_null_geometries;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetNullBuildRangeInfos(bool use_null_build_range_infos) {
    use_null_build_range_infos_ = use_null_build_range_infos;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetDeferredOp(VkDeferredOperationKHR deferred_op) {
    deferred_op_ = deferred_op;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetUpdateDstAccelStructSizeBeforeBuild(bool update_before_build) {
    update_dst_as_size_before_build_ = update_before_build;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetIndirectStride(uint32_t indirect_stride) {
    indirect_stride_ = indirect_stride;
    return *this;
}

BuildGeometryInfoKHR &BuildGeometryInfoKHR::SetIndirectDeviceAddress(std::optional<VkDeviceAddress> indirect_buffer_address) {
    indirect_buffer_address_ = indirect_buffer_address;
    return *this;
}

void BuildGeometryInfoKHR::BuildCmdBuffer(VkCommandBuffer cmd_buffer, bool use_ppGeometries /*= true*/) {
    if (blas_) {
        // blas_->BuildCmdBuffer(cmd_buffer, use_ppGeometries);
        assert(blas_->GetDstAS()->IsBuilt());
    }
    SetupBuild(true);
    VkCmdBuildAccelerationStructuresKHR(cmd_buffer, true);
}

void BuildGeometryInfoKHR::BuildCmdBufferIndirect(VkCommandBuffer cmd_buffer) {
    if (blas_) {
        blas_->BuildCmdBufferIndirect(cmd_buffer);
    }
    SetupBuild(true);
    VkCmdBuildAccelerationStructuresIndirectKHR(cmd_buffer);
}

void BuildGeometryInfoKHR::BuildHost() {
    if (blas_) {
        blas_->BuildHost();
    }
    SetupBuild(false);
    VkBuildAccelerationStructuresKHR();
}

void BuildGeometryInfoKHR::UpdateDstAccelStructSize() {
    const VkAccelerationStructureBuildSizesInfoKHR size_info = GetSizeInfo();
    dst_as_->SetSize(size_info.accelerationStructureSize);
}

void BuildGeometryInfoKHR::SetupBuild(bool is_on_device_build, bool use_ppGeometries /*= true*/) {
    // Build geometries
    for (auto &geometry : geometries_) {
        geometry.Build();
    }

    if (update_dst_as_size_before_build_ && !dst_as_->IsNull() && !dst_as_->IsBuilt()) {
        UpdateDstAccelStructSize();
    }

    // Build source and destination acceleration structures
    if (!src_as_->IsNull() && !src_as_->IsBuilt()) {
        src_as_->Build();
    }
    vk_info_.srcAccelerationStructure = src_as_->handle();
    if (!dst_as_->IsNull() && !dst_as_->IsBuilt()) {
        dst_as_->Build();
    }
    vk_info_.dstAccelerationStructure = dst_as_->handle();

    if (build_scratch_) {
        const VkAccelerationStructureBuildSizesInfoKHR size_info = GetSizeInfo(use_ppGeometries);
        const VkDeviceSize scratch_size = vk_info_.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR
                                              ? size_info.updateScratchSize
                                              : size_info.buildScratchSize;
        if (is_on_device_build) {
            // Allocate device local scratch buffer

            // Get minAccelerationStructureScratchOffsetAlignment
            VkPhysicalDeviceAccelerationStructurePropertiesKHR as_props = vku::InitStructHelper();
            VkPhysicalDeviceProperties2 phys_dev_props = vku::InitStructHelper(&as_props);
            vk::GetPhysicalDeviceProperties2(device_->phy(), &phys_dev_props);

            assert(device_scratch_);  // So far null pointers are not supported
            if (!device_scratch_->initialized()) {
                VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
                alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

                if (scratch_size > 0) {
                    device_scratch_->init(*device_, scratch_size + as_props.minAccelerationStructureScratchOffsetAlignment,
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
                }
            }
            if (device_scratch_->CreateInfo().size != 0 &&
                device_scratch_->CreateInfo().usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                const VkDeviceAddress scratch_address = device_scratch_->address();
                const auto aligned_scratch_address =
                    Align<VkDeviceAddress>(scratch_address, as_props.minAccelerationStructureScratchOffsetAlignment);
                assert(aligned_scratch_address >= scratch_address);
                assert(aligned_scratch_address < (scratch_address + as_props.minAccelerationStructureScratchOffsetAlignment));
                vk_info_.scratchData.deviceAddress = aligned_scratch_address + device_scratch_offset_;
                assert(vk_info_.scratchData.deviceAddress <
                       (scratch_address +
                        device_scratch_->CreateInfo().size));  // Note: This assert may prove overly conservative in the future
            } else {
                vk_info_.scratchData.deviceAddress = 0;
            }
        } else {
            // Allocate on host scratch buffer
            host_scratch_ = nullptr;
            if (scratch_size > 0) {
                assert(scratch_size < vvl::kU32Max);
                host_scratch_ = std::make_shared<std::vector<uint8_t>>(static_cast<size_t>(scratch_size), uint8_t(0u));
            }
            vk_info_.scratchData.hostAddress = host_scratch_->data();
        }
    }
}

void BuildGeometryInfoKHR::VkCmdBuildAccelerationStructuresKHR(VkCommandBuffer cmd_buffer, bool use_ppGeometries /*= true*/) {
    // fill vk_info_ with geometry data, and get build ranges
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    if (use_ppGeometries) {
        pGeometries.resize(geometries_.size());
    } else {
        geometries.resize(geometries_.size());
    }

    assert(build_range_infos_.size() >= geometries_.size());
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> pRange_infos(geometries_.size());
    for (size_t i = 0; i < geometries_.size(); ++i) {
        const auto &geometry = geometries_[i];
        if (use_ppGeometries) {
            pGeometries[i] = &geometry.GetVkObj();
        } else {
            geometries[i] = geometry.GetVkObj();
        }
        pRange_infos[i] = &build_range_infos_[i];
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    if (use_null_geometries_) {
        vk_info_.pGeometries = nullptr;
        vk_info_.ppGeometries = nullptr;
    } else if (use_ppGeometries) {
        vk_info_.ppGeometries = pGeometries.data();
    } else {
        vk_info_.pGeometries = geometries.data();
    }

    // Build acceleration structure
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos = use_null_infos_ ? nullptr : &vk_info_;
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos =
        use_null_build_range_infos_ ? nullptr : pRange_infos.data();
    vk::CmdBuildAccelerationStructuresKHR(cmd_buffer, vk_info_count_, pInfos, ppBuildRangeInfos);

    // pGeometries and geometries are going to be destroyed
    vk_info_.geometryCount = 0;
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;
}

void BuildGeometryInfoKHR::VkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer cmd_buffer) {
    // If vk_info_count is >1, cannot pIndirectDeviceAddresses, pIndirectStrides and ppMaxPrimitiveCounts like done here
    assert(vk_info_count_ <= 1);

    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());

    indirect_buffer_ = std::make_unique<vkt::Buffer>();
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    indirect_buffer_->init(*device_, 1 * vk_info_.geometryCount * sizeof(VkAccelerationStructureBuildRangeInfoKHR),
                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    auto *ranges_info = static_cast<VkAccelerationStructureBuildRangeInfoKHR *>(indirect_buffer_->memory().map());

    // fill vk_info_ with geometry data, and get build ranges
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries(geometries_.size());

    pGeometries.reserve(geometries_.size());
    for (const auto [i, geometry] : vvl::enumerate(geometries_)) {
        pGeometries[i] = &geometry->GetVkObj();
        ranges_info[i] = geometry->GetFullBuildRange();
    }
    if (use_null_geometries_) {
        vk_info_.pGeometries = nullptr;
        vk_info_.ppGeometries = nullptr;
    } else {
        vk_info_.ppGeometries = pGeometries.data();
    }
    indirect_buffer_->memory().unmap();

    std::vector<uint32_t> p_max_primitive_counts(vk_info_.geometryCount, 1);
    const uint32_t *pp_max_primitive_counts = p_max_primitive_counts.data();

    const VkDeviceAddress indirect_address = indirect_buffer_address_ ? *indirect_buffer_address_ : indirect_buffer_->address();

    vk::CmdBuildAccelerationStructuresIndirectKHR(cmd_buffer, vk_info_count_, &vk_info_, &indirect_address, &indirect_stride_,
                                                  &pp_max_primitive_counts);

    // pGeometries and geometries are going to be destroyed
    vk_info_.geometryCount = 0;
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;
}

void BuildGeometryInfoKHR::VkBuildAccelerationStructuresKHR() {
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
    if (use_null_geometries_) {
        vk_info_.pGeometries = nullptr;
        vk_info_.ppGeometries = nullptr;
    } else {
        vk_info_.ppGeometries = pGeometries.data();
    }
    // Build acceleration structure
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos = use_null_infos_ ? nullptr : &vk_info_;
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos =
        use_null_build_range_infos_ ? nullptr : pRange_infos.data();
    vk::BuildAccelerationStructuresKHR(device_->handle(), deferred_op_, vk_info_count_, pInfos, ppBuildRangeInfos);

    // pGeometries is going to be destroyed
    vk_info_.geometryCount = 0;
    vk_info_.ppGeometries = nullptr;
}

VkAccelerationStructureBuildSizesInfoKHR BuildGeometryInfoKHR::GetSizeInfo(bool use_ppGeometries /*= true*/) {
    // Computer total primitives count, and get pointers to geometries
    std::vector<uint32_t> primitives_count(geometries_.size());
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;

    if (use_ppGeometries) {
        pGeometries.reserve(geometries_.size());
    } else {
        geometries.reserve(geometries_.size());
    }

    for (const auto &[geometry_i, geometry] : vvl::enumerate(geometries_)) {
        primitives_count[geometry_i] = geometry->GetFullBuildRange().primitiveCount;
        if (use_ppGeometries) {
            pGeometries.emplace_back(&geometry->GetVkObj());
        } else {
            geometries.emplace_back(geometry->GetVkObj());
        }
    }
    vk_info_.geometryCount = static_cast<uint32_t>(geometries_.size());
    if (use_null_geometries_) {
        vk_info_.pGeometries = nullptr;
        vk_info_.ppGeometries = nullptr;
    } else if (use_ppGeometries) {
        vk_info_.ppGeometries = pGeometries.data();
    } else {
        vk_info_.pGeometries = geometries.data();
    }

    // Get VkAccelerationStructureBuildSizesInfoKHR using this->vk_info_
    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    vk::GetAccelerationStructureBuildSizesKHR(device_->handle(), build_type_, &vk_info_, primitives_count.data(), &size_info);

    // pGeometries and geometries are going to be destroyed
    vk_info_.geometryCount = 0;
    vk_info_.ppGeometries = nullptr;
    vk_info_.pGeometries = nullptr;

    return size_info;
}

std::vector<VkAccelerationStructureBuildRangeInfoKHR> BuildGeometryInfoKHR::GetBuildRangeInfosFromGeometries() {
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(geometries_.size());
    for (const auto [i, geometry] : vvl::enumerate(geometries_)) {
        range_infos[i] = geometry->GetFullBuildRange();
    }

    return range_infos;
}

void BuildAccelerationStructuresKHR(VkCommandBuffer cmd_buffer, std::vector<BuildGeometryInfoKHR> &infos) {
    size_t total_geomertry_count = 0;

    for (auto &build_info : infos) {
        total_geomertry_count += build_info.geometries_.size();
    }

    // Those vectors will be used to contiguously store the "raw vulkan data" for each element of `infos`
    // To do that, total memory needed needs to be know upfront
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries(total_geomertry_count);
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(total_geomertry_count);
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> pRange_infos(total_geomertry_count);

    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vk_infos;
    vk_infos.reserve(infos.size());

    size_t pGeometries_offset = 0;
    size_t range_infos_offset = 0;
    size_t pRange_infos_offset = 0;

    for (auto &build_info : infos) {
        if (build_info.blas_) {
            build_info.blas_->BuildCmdBuffer(cmd_buffer, true);
        }
        build_info.SetupBuild(true);

        // Fill current vk_info_ with geometry data in ppGeometries, and get build ranges
        for (size_t i = 0; i < build_info.geometries_.size(); ++i) {
            const auto &geometry = build_info.geometries_[i];
            pGeometries[pGeometries_offset + i] = &geometry.GetVkObj();
            range_infos[range_infos_offset + i] = geometry.GetFullBuildRange();
            pRange_infos[pRange_infos_offset + i] = &range_infos[range_infos_offset + i];
        }

        build_info.vk_info_.geometryCount = static_cast<uint32_t>(build_info.geometries_.size());
        build_info.vk_info_.ppGeometries = &pGeometries[pGeometries_offset];

        vk_infos.emplace_back(build_info.vk_info_);

        pGeometries_offset += build_info.geometries_.size();
        range_infos_offset += build_info.geometries_.size();
        pRange_infos_offset += build_info.geometries_.size();
    }

    // Build list of acceleration structures
    vk::CmdBuildAccelerationStructuresKHR(cmd_buffer, static_cast<uint32_t>(vk_infos.size()), vk_infos.data(), pRange_infos.data());

    // Clean
    for (auto &build_info : infos) {
        // pGeometries is going to be destroyed
        build_info.vk_info_.geometryCount = 0;
        build_info.vk_info_.ppGeometries = nullptr;
    }
}

void BuildHostAccelerationStructuresKHR(VkDevice device, std::vector<BuildGeometryInfoKHR> &infos) {
    size_t total_geomertry_count = 0;

    for (auto &build_info : infos) {
        total_geomertry_count += build_info.geometries_.size();
    }

    // Those vectors will be used to contiguously store the "raw vulkan data" for each element of `infos`
    // To do that, total memory needed needs to be know upfront
    std::vector<const VkAccelerationStructureGeometryKHR *> pGeometries(total_geomertry_count);
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos(total_geomertry_count);
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> pRange_infos(total_geomertry_count);

    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vk_infos;
    vk_infos.reserve(infos.size());

    size_t pGeometries_offset = 0;
    size_t range_infos_offset = 0;
    size_t pRange_infos_offset = 0;

    for (auto &build_info : infos) {
        if (build_info.blas_) {
            build_info.blas_->BuildHost();
        }
        build_info.SetupBuild(false);

        // Fill current vk_info_ with geometry data in ppGeometries, and get build ranges
        for (size_t i = 0; i < build_info.geometries_.size(); ++i) {
            const auto &geometry = build_info.geometries_[i];
            pGeometries[pGeometries_offset + i] = &geometry.GetVkObj();
            range_infos[range_infos_offset + i] = geometry.GetFullBuildRange();
            pRange_infos[pRange_infos_offset + i] = &range_infos[range_infos_offset + i];
        }

        build_info.vk_info_.geometryCount = static_cast<uint32_t>(build_info.geometries_.size());
        build_info.vk_info_.ppGeometries = &pGeometries[pGeometries_offset];

        vk_infos.emplace_back(build_info.vk_info_);

        pGeometries_offset += build_info.geometries_.size();
        range_infos_offset += build_info.geometries_.size();
        pRange_infos_offset += build_info.geometries_.size();
    }

    // Build list of acceleration structures
    vk::BuildAccelerationStructuresKHR(device, VK_NULL_HANDLE, static_cast<uint32_t>(vk_infos.size()), vk_infos.data(),
                                       pRange_infos.data());

    // Clean
    for (auto &build_info : infos) {
        // pGeometries is going to be destroyed
        build_info.vk_info_.geometryCount = 0;
        build_info.vk_info_.ppGeometries = nullptr;
    }
}

namespace blueprint {
GeometryKHR GeometrySimpleOnDeviceTriangleInfo(const vkt::Device &device, size_t triangles_count) {
    assert(triangles_count > 0);
    GeometryKHR triangle_geometry;

    triangle_geometry.SetType(GeometryKHR::Type::Triangle);

    // Allocate vertex and index buffers
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vkt::Buffer vertex_buffer(device, 1024, buffer_usage,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);
    vkt::Buffer index_buffer(device, 1024 + 3 * triangles_count * sizeof(uint32_t), buffer_usage,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);
    vkt::Buffer transform_buffer(device, sizeof(VkTransformMatrixKHR), buffer_usage,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    // Fill vertex and index buffers with one triangle
    triangle_geometry.SetPrimitiveCount(triangles_count);
    constexpr std::array vertices = {// Vertex 0
                                     10.0f, 10.0f, 0.0f,
                                     // Vertex 1
                                     -10.0f, 10.0f, 0.0f,
                                     // Vertex 2
                                     0.0f, -10.0f, 0.0f};
    std::vector<uint32_t> indices(triangles_count * 3);
    for (size_t triangle_i = 0; triangle_i < triangles_count; ++triangle_i) {
        indices[3 * triangle_i + 0] = 0;
        indices[3 * triangle_i + 1] = 1;
        indices[3 * triangle_i + 2] = 2;
    }

    auto vertex_buffer_ptr = static_cast<float *>(vertex_buffer.memory().map());
    std::copy(vertices.begin(), vertices.end(), vertex_buffer_ptr);
    vertex_buffer.memory().unmap();

    auto index_buffer_ptr = static_cast<uint32_t *>(index_buffer.memory().map());
    std::copy(indices.begin(), indices.end(), index_buffer_ptr);
    index_buffer.memory().unmap();

    // clang-format off
    VkTransformMatrixKHR transform_matrix = {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
    }};
    // clang-format on

    auto transform_buffer_ptr = static_cast<VkTransformMatrixKHR *>(transform_buffer.memory().map());
    std::memcpy(transform_buffer_ptr, &transform_matrix, sizeof(transform_matrix));
    transform_buffer.memory().unmap();

    // Assign vertex and index buffers to out geometry
    triangle_geometry.SetTrianglesDeviceVertexBuffer(std::move(vertex_buffer), uint32_t(vertices.size() / 3));
    triangle_geometry.SetTrianglesIndexType(VK_INDEX_TYPE_UINT32);
    triangle_geometry.SetTrianglesDeviceIndexBuffer(std::move(index_buffer));
    triangle_geometry.SetTrianglesTransformBuffer(std::move(transform_buffer));
    triangle_geometry.SetFlags(VK_GEOMETRY_OPAQUE_BIT_KHR);

    return triangle_geometry;
}

GeometryKHR GeometrySimpleOnHostTriangleInfo() {
    GeometryKHR triangle_geometry;

    triangle_geometry.SetType(GeometryKHR::Type::Triangle);

    // Fill vertex and index buffers with one triangle
    triangle_geometry.SetPrimitiveCount(1);
    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indices = {{0, 1, 2}};

    auto vertex_buffer = std::make_unique<float[]>(vertices.size());
    std::copy(vertices.data(), vertices.data() + vertices.size(), vertex_buffer.get());

    auto index_buffer = std::make_unique<uint32_t[]>(indices.size());
    std::copy(indices.data(), indices.data() + indices.size(), index_buffer.get());

    // Assign vertex and index buffers to out geometry
    triangle_geometry.SetTrianglesHostVertexBuffer(std::move(vertex_buffer), uint32_t(vertices.size() / 3 - 1));
    triangle_geometry.SetTrianglesHostIndexBuffer(std::move(index_buffer));

    return triangle_geometry;
}

GeometryKHR GeometrySimpleOnDeviceAABBInfo(const vkt::Device &device) {
    GeometryKHR aabb_geometry;

    aabb_geometry.SetType(GeometryKHR::Type::AABB);

    // Allocate buffer
    const std::array<VkAabbPositionsKHR, 1> aabbs = {{{-10.0f, -10.0f, -10.0f, +10.0f, +10.0f, +10.0f}}};

    const VkDeviceSize aabb_buffer_size = sizeof(aabbs[0]) * aabbs.size();
    vkt::Buffer aabb_buffer;
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    aabb_buffer.init(device, aabb_buffer_size, buffer_usage,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    // Fill buffer with one AABB
    aabb_geometry.SetPrimitiveCount(static_cast<uint32_t>(aabbs.size()));
    auto mapped_aabb_buffer_data = static_cast<VkAabbPositionsKHR *>(aabb_buffer.memory().map());
    std::copy(aabbs.begin(), aabbs.end(), mapped_aabb_buffer_data);
    aabb_buffer.memory().unmap();

    aabb_geometry.SetAABBsDeviceBuffer(std::move(aabb_buffer));

    return aabb_geometry;
}

GeometryKHR GeometrySimpleOnHostAABBInfo() {
    GeometryKHR aabb_geometry;

    aabb_geometry.SetType(GeometryKHR::Type::AABB);

    // Fill buffer with one aabb
    const std::array<VkAabbPositionsKHR, 1> aabbs = {{{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}}};

    auto aabb_buffer = std::make_unique<VkAabbPositionsKHR[]>(aabbs.size());
    std::copy(aabbs.data(), aabbs.data() + aabbs.size(), aabb_buffer.get());

    // Assign aabb buffer to out geometry
    aabb_geometry.SetPrimitiveCount(1);
    aabb_geometry.SetAABBsHostBuffer(std::move(aabb_buffer));

    return aabb_geometry;
}

GeometryKHR GeometrySimpleDeviceInstance(const vkt::Device &device, VkAccelerationStructureKHR device_blas) {
    GeometryKHR instance_geometry;

    instance_geometry.SetType(GeometryKHR::Type::Instance);
    instance_geometry.AddInstanceDeviceAccelStructRef(device, device_blas);

    return instance_geometry;
}

GeometryKHR GeometrySimpleHostInstance(VkAccelerationStructureKHR host_instance) {
    GeometryKHR instance_geometry;

    instance_geometry.SetType(GeometryKHR::Type::Instance);
    instance_geometry.AddInstanceHostAccelStructRef(host_instance);

    return instance_geometry;
}

std::shared_ptr<AccelerationStructureKHR> AccelStructNull(const vkt::Device &device) {
    auto as = std::make_shared<AccelerationStructureKHR>(&device);
    as->SetNull(true);
    return as;
}

std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceBottomLevel(const vkt::Device &device, VkDeviceSize size) {
    auto as = std::make_shared<AccelerationStructureKHR>(&device);
    as->SetSize(size);
    as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    as->SetDeviceBufferMemoryAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
    as->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    as->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    as->SetDeviceBufferInitNoMem(false);
    return as;
}

std::shared_ptr<vkt::as::AccelerationStructureKHR> AccelStructSimpleOnHostBottomLevel(const vkt::Device &device,
                                                                                      VkDeviceSize size) {
    auto as = std::make_shared<AccelerationStructureKHR>(&device);
    as->SetSize(size);
    as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    as->SetDeviceBufferMemoryAllocateFlags(0);
    as->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    as->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    as->SetDeviceBufferInitNoMem(false);
    return as;
}

std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceTopLevel(const vkt::Device &device, VkDeviceSize size) {
    auto as = std::make_shared<AccelerationStructureKHR>(&device);
    as->SetSize(size);
    as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    as->SetDeviceBufferMemoryAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
    as->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    as->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    as->SetDeviceBufferInitNoMem(false);
    return as;
}

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceBottomLevel(const vkt::Device &device,
                                                                GeometryKHR::Type geometry_type /*= GeometryKHR::Type::Triangle*/) {
    BuildGeometryInfoKHR out_build_info(&device);

    out_build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    out_build_info.SetBuildType(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR);
    out_build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

    // Set geometry
    std::vector<GeometryKHR> geometries;
    switch (geometry_type) {
        case GeometryKHR::Type::Triangle:
            geometries.emplace_back(GeometrySimpleOnDeviceTriangleInfo(device));
            break;
        case GeometryKHR::Type::AABB:
            geometries.emplace_back(GeometrySimpleOnDeviceAABBInfo(device));
            break;
        case GeometryKHR::Type::Instance:
            [[fallthrough]];
        case GeometryKHR::Type::_INTERNAL_UNSPECIFIED:
            assert(false);
            break;
    }
    out_build_info.SetGeometries(std::move(geometries));
    out_build_info.SetBuildRanges(out_build_info.GetBuildRangeInfosFromGeometries());

    // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
    out_build_info.SetSrcAS(AccelStructNull(device));
    auto dstAsSize = out_build_info.GetSizeInfo().accelerationStructureSize;
    out_build_info.SetDstAS(AccelStructSimpleOnDeviceBottomLevel(device, dstAsSize));
    out_build_info.SetUpdateDstAccelStructSizeBeforeBuild(true);

    out_build_info.SetInfoCount(1);
    out_build_info.SetNullInfos(false);
    out_build_info.SetNullBuildRangeInfos(false);

    return out_build_info;
}

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostBottomLevel(const vkt::Device &device,
                                                              GeometryKHR::Type geometry_type /*= GeometryKHR::Type::Triangle*/) {
    BuildGeometryInfoKHR out_build_info(&device);

    out_build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    out_build_info.SetBuildType(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR);
    out_build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

    // Set geometry
    std::vector<GeometryKHR> geometries;
    switch (geometry_type) {
        case GeometryKHR::Type::Triangle:
            geometries.emplace_back(GeometrySimpleOnHostTriangleInfo());
            break;
        case GeometryKHR::Type::AABB:
            geometries.emplace_back(GeometrySimpleOnHostAABBInfo());
            break;
        case GeometryKHR::Type::Instance:
            [[fallthrough]];
        case GeometryKHR::Type::_INTERNAL_UNSPECIFIED:
            assert(false);
            break;
    }
    out_build_info.SetGeometries(std::move(geometries));
    out_build_info.SetBuildRanges(out_build_info.GetBuildRangeInfosFromGeometries());

    // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
    out_build_info.SetSrcAS(AccelStructNull(device));
    auto dstAsSize = out_build_info.GetSizeInfo().accelerationStructureSize;
    out_build_info.SetDstAS(AccelStructSimpleOnHostBottomLevel(device, dstAsSize));
    out_build_info.SetUpdateDstAccelStructSizeBeforeBuild(true);

    out_build_info.SetInfoCount(1);
    out_build_info.SetNullInfos(false);
    out_build_info.SetNullBuildRangeInfos(false);

    return out_build_info;
}

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceTopLevel(
    const vkt::Device &device, std::shared_ptr<BuildGeometryInfoKHR> on_device_bottom_level_geometry) {
    BuildGeometryInfoKHR out_build_info(&device);

    out_build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    out_build_info.SetBuildType(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR);
    out_build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

    // Set bottom level acceleration structure
    assert(on_device_bottom_level_geometry->GetDstAS()->IsBuilt());
    out_build_info.SetBottomLevelAS(std::move(on_device_bottom_level_geometry));

    // Set geometry to one instance pointing to bottom level acceleration structure
    std::vector<GeometryKHR> geometries;
    geometries.emplace_back(GeometrySimpleDeviceInstance(device, out_build_info.GetBottomLevelAS()->GetDstAS()->handle()));
    out_build_info.SetGeometries(std::move(geometries));
    out_build_info.SetBuildRanges(out_build_info.GetBuildRangeInfosFromGeometries());

    // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
    out_build_info.SetSrcAS(AccelStructNull(device));
    auto dstAsSize = out_build_info.GetSizeInfo().accelerationStructureSize;
    auto dst_as = AccelStructSimpleOnDeviceBottomLevel(device, dstAsSize);
    dst_as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    out_build_info.SetDstAS(std::move(dst_as));
    out_build_info.SetUpdateDstAccelStructSizeBeforeBuild(true);

    out_build_info.SetInfoCount(1);
    out_build_info.SetNullInfos(false);
    out_build_info.SetNullBuildRangeInfos(false);

    return out_build_info;
}

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostTopLevel(const vkt::Device &device,
                                                           std::shared_ptr<BuildGeometryInfoKHR> on_host_bottom_level_geometry) {
    BuildGeometryInfoKHR out_build_info(&device);

    out_build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    out_build_info.SetBuildType(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR);
    out_build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

    // Set bottom level acceleration structure
    assert(on_host_bottom_level_geometry->GetDstAS()->IsBuilt());
    out_build_info.SetBottomLevelAS(std::move(on_host_bottom_level_geometry));

    // Set geometry to one instance pointing to bottom level acceleration structure
    std::vector<GeometryKHR> geometries;
    geometries.emplace_back(GeometrySimpleHostInstance(out_build_info.GetBottomLevelAS()->GetDstAS()->handle()));
    out_build_info.SetGeometries(std::move(geometries));
    out_build_info.SetBuildRanges(out_build_info.GetBuildRangeInfosFromGeometries());

    // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
    out_build_info.SetSrcAS(AccelStructNull(device));
    auto dstAsSize = out_build_info.GetSizeInfo().accelerationStructureSize;
    auto dst_as = AccelStructSimpleOnHostBottomLevel(device, dstAsSize);
    dst_as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    out_build_info.SetDstAS(std::move(dst_as));
    out_build_info.SetUpdateDstAccelStructSizeBeforeBuild(true);

    out_build_info.SetInfoCount(1);
    out_build_info.SetNullInfos(false);
    out_build_info.SetNullBuildRangeInfos(false);

    return out_build_info;
}

BuildGeometryInfoKHR BuildOnDeviceTopLevel(const vkt::Device &device, vkt::Queue &queue, vkt::CommandBuffer &cmd_buffer) {
    // Create acceleration structure
    cmd_buffer.begin();
    // Build Bottom Level Acceleration Structure
    auto bot_level_accel_struct =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(device));
    bot_level_accel_struct->BuildCmdBuffer(cmd_buffer);
    cmd_buffer.end();

    queue.Submit(cmd_buffer);
    device.Wait();

    cmd_buffer.begin();
    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR top_level_accel_struct =
        vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(device, bot_level_accel_struct);
    top_level_accel_struct.BuildCmdBuffer(cmd_buffer);
    cmd_buffer.end();

    queue.Submit(cmd_buffer);
    device.Wait();

    return top_level_accel_struct;
}

}  // namespace blueprint

}  // namespace as

namespace rt {

Pipeline::Pipeline(VkLayerTest &test, vkt::Device *device) : test_(test), device_(device) {}

Pipeline::~Pipeline() {
    if (deferred_op_ != VK_NULL_HANDLE) {
        vk::DestroyDeferredOperationKHR(device_->handle(), deferred_op_, nullptr);
    }
}

void Pipeline::AddCreateInfoFlags(VkPipelineCreateFlags flags) { vk_info_.flags |= flags; }

void Pipeline::InitLibraryInfo() {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    test_.GetPhysicalDeviceProperties2(rt_pipeline_props);
    rt_pipeline_interface_info_ = vku::InitStructHelper();
    rt_pipeline_interface_info_.maxPipelineRayPayloadSize = sizeof(float);  // Set according to payload defined in kRayGenShaderText
    rt_pipeline_interface_info_.maxPipelineRayHitAttributeSize = rt_pipeline_props.maxRayHitAttributeSize;
    AddCreateInfoFlags(VK_PIPELINE_CREATE_LIBRARY_BIT_KHR);
    vk_info_.pLibraryInterface = &rt_pipeline_interface_info_;
}

void Pipeline::AddBinding(VkDescriptorType descriptor_type, uint32_t binding, uint32_t descriptor_count /*= 1*/) {
    VkDescriptorSetLayoutBinding binding_layout = {};
    binding_layout.binding = binding;
    binding_layout.descriptorType = descriptor_type;
    binding_layout.descriptorCount = descriptor_count;
    binding_layout.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
    bindings_.emplace_back(binding_layout);
}

void Pipeline::CreateDescriptorSet() { desc_set_ = std::make_unique<OneOffDescriptorSet>(device_, bindings_); }

void Pipeline::SetPushConstantRangeSize(uint32_t byte_size) { push_constant_range_size_ = byte_size; }

void Pipeline::SetGlslRayGenShader(const char *glsl) {
    ray_gen_ = std::make_unique<VkShaderObj>(&test_, glsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
}

void Pipeline::SetSpirvRayGenShader(const char *spirv, const char *entry_point) {
    ray_gen_ = std::make_unique<VkShaderObj>(&test_, spirv, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM,
                                             nullptr, entry_point);
}

void Pipeline::AddGlslMissShader(const char *glsl) {
    miss_shaders_.emplace_back(std::make_unique<VkShaderObj>(&test_, glsl, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2));
}

void Pipeline::AddSpirvMissShader(const char *spirv, const char *entry_point) {
    miss_shaders_.emplace_back(std::make_unique<VkShaderObj>(&test_, spirv, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2,
                                                             SPV_SOURCE_ASM, nullptr, entry_point));
}

void Pipeline::AddGlslClosestHitShader(const char *glsl) {
    closest_hit_shaders_.emplace_back(
        std::make_unique<VkShaderObj>(&test_, glsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2));
}

void Pipeline::AddSpirvClosestHitShader(const char *spirv, const char *entry_point) {
    closest_hit_shaders_.emplace_back(std::make_unique<VkShaderObj>(&test_, spirv, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                                                                    SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM, nullptr, entry_point));
}

void Pipeline::AddLibrary(const Pipeline &library) {
    libraries_.emplace_back(library.rt_pipeline_);
    pipeline_lib_info_ = vku::InitStructHelper();
    pipeline_lib_info_.libraryCount = size32(libraries_);
    pipeline_lib_info_.pLibraries = libraries_.data();
    vk_info_.pLibraryInfo = &pipeline_lib_info_;
}

void Pipeline::AddDynamicState(VkDynamicState dynamic_state) { dynamic_states.emplace_back(dynamic_state); }

void Pipeline::Build() {
    BuildPipeline();
    BuildSbt();
}

void Pipeline::BuildPipeline() {
    // Create push constant range
    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    push_constant_range.offset = 0;
    push_constant_range.size = push_constant_range_size_;

    // Create pipeline layout
    if (!pipeline_layout_.initialized()) {
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        if (push_constant_range_size_ > 0) {
            pipeline_layout_ci.pushConstantRangeCount = 1;
            pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        }
        if (desc_set_) {
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &desc_set_->layout_.handle();
        }
        pipeline_layout_.init(*device_, pipeline_layout_ci);
    }

    // Assemble shaders information (stages and groups)
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_stage_cis;
    assert(shader_group_cis_.empty());  // For now this list is expected to be empty at this point
    if (ray_gen_) {
        VkPipelineShaderStageCreateInfo raygen_stage_ci = vku::InitStructHelper();
        raygen_stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        raygen_stage_ci.module = *ray_gen_;
        raygen_stage_ci.pName = ray_gen_->GetStageCreateInfo().pName;
        pipeline_stage_cis.emplace_back(raygen_stage_ci);

        VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci = vku::InitStructHelper();
        raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        raygen_group_ci.generalShader = pipeline_stage_cis.size() - 1;
        raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
        raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
        raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group_cis_.emplace_back(raygen_group_ci);
    }
    for (const auto [miss_shader_i, miss_shader] : vvl::enumerate(miss_shaders_)) {
        VkPipelineShaderStageCreateInfo miss_stage_ci = vku::InitStructHelper();
        miss_stage_ci.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        miss_stage_ci.module = *miss_shader->get();
        miss_stage_ci.pName = (*miss_shader)->GetStageCreateInfo().pName;
        pipeline_stage_cis.emplace_back(miss_stage_ci);

        VkRayTracingShaderGroupCreateInfoKHR miss_group_ci = vku::InitStructHelper();
        miss_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        miss_group_ci.generalShader = pipeline_stage_cis.size() - 1;
        miss_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
        miss_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
        miss_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group_cis_.emplace_back(miss_group_ci);
    }
    for (const auto [closest_hit_i, closest_hit] : vvl::enumerate(closest_hit_shaders_)) {
        VkPipelineShaderStageCreateInfo closest_hit_stage_ci = vku::InitStructHelper();
        closest_hit_stage_ci.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        closest_hit_stage_ci.module = *closest_hit->get();
        closest_hit_stage_ci.pName = (*closest_hit)->GetStageCreateInfo().pName;
        pipeline_stage_cis.emplace_back(closest_hit_stage_ci);

        VkRayTracingShaderGroupCreateInfoKHR closest_hit_group_ci = vku::InitStructHelper();
        closest_hit_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        closest_hit_group_ci.generalShader = VK_SHADER_UNUSED_KHR;
        closest_hit_group_ci.closestHitShader = pipeline_stage_cis.size() - 1;
        closest_hit_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
        closest_hit_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group_cis_.emplace_back(closest_hit_group_ci);
    }

    // Dynamic states
    VkPipelineDynamicStateCreateInfo dynamic_state_ci = vku::InitStructHelper();
    dynamic_state_ci.dynamicStateCount = size32(dynamic_states);
    dynamic_state_ci.pDynamicStates = dynamic_states.empty() ? nullptr : dynamic_states.data();

    // Create pipeline
    vk_info_.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    vk_info_.stageCount = size32(pipeline_stage_cis);
    vk_info_.pStages = pipeline_stage_cis.data();
    vk_info_.groupCount = size32(shader_group_cis_);
    vk_info_.pGroups = shader_group_cis_.data();
    vk_info_.maxPipelineRayRecursionDepth = 1;
    vk_info_.pDynamicState = &dynamic_state_ci;
    vk_info_.layout = pipeline_layout_;

    if (deferred_op_ == VK_NULL_HANDLE) {
        rt_pipeline_.init(*device_, vk_info_);
    } else {
        rt_pipeline_.InitDeferred(*device_, vk_info_, deferred_op_);

        VkResult result = vk::DeferredOperationJoinKHR(device_->handle(), deferred_op_);
        if (result != VK_SUCCESS) {
            ADD_FAILURE() << "vk::DeferredOperationJoinKHR returned " << string_VkResult(result);
            return;
        }
        result = vk::GetDeferredOperationResultKHR(device_->handle(), deferred_op_);
        if (result != VK_SUCCESS) {
            ADD_FAILURE() << "vk::GetDeferredOperationResultKHR returned " << string_VkResult(result);
            return;
        }
    }
}

void Pipeline::BuildSbt() {
    std::vector<uint8_t> sbt_host_storage = GetRayTracingShaderGroupHandles();

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    test_.GetPhysicalDeviceProperties2(rt_pipeline_props);

    // Allocate buffer to store SBT, and fill it with sbt_host_storage
    VkBufferCreateInfo sbt_buffer_info = vku::InitStructHelper();
    sbt_buffer_info.size = Align<VkDeviceSize>(sbt_host_storage.size() + 2 * rt_pipeline_props.shaderGroupBaseAlignment, 4096);
    sbt_buffer_info.usage =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags sbt_buffer_mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    sbt_buffer_.init(*device_, sbt_buffer_info, sbt_buffer_mem_props, &alloc_flags);

#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "SBT buffer fill:\n";
#endif

    void *const sbt_buffer_base_ptr = sbt_buffer_.memory().map();
    void *sbt_buffer_ptr = sbt_buffer_base_ptr;
    (void)sbt_buffer_base_ptr;
    size_t sbt_buffer_space_left = static_cast<size_t>(sbt_buffer_info.size);
    uint8_t *sbt_host_storage_ptr = sbt_host_storage.data();

    const uint32_t handle_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);

    // Fill SBT with ray gen shader
    void *ray_gen_sbt = sbt_buffer_ptr;
    std::memcpy(ray_gen_sbt, sbt_host_storage_ptr, handle_size_aligned);
#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "Ray Gen SBT entry: offset = 0 | size = " << handle_size_aligned << '\n';
#endif
    sbt_buffer_ptr = (uint8_t *)sbt_buffer_ptr + handle_size_aligned;
    sbt_buffer_space_left -= handle_size_aligned;
    sbt_host_storage_ptr += rt_pipeline_props.shaderGroupHandleSize;

    // Fill SBT with miss shaders
    if (!std::align(rt_pipeline_props.shaderGroupBaseAlignment, miss_shaders_.size() * handle_size_aligned, sbt_buffer_ptr,
                    sbt_buffer_space_left)) {
        assert(false);
    }
    void *miss_shaders_sbt = sbt_buffer_ptr;
    std::memcpy(miss_shaders_sbt, sbt_host_storage.data() + handle_size_aligned, miss_shaders_.size() * handle_size_aligned);
#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "Miss shaders SBT entry: offset = " << ((uint64_t)miss_shaders_sbt - (uint64_t)sbt_buffer_base_ptr)
              << " | size = " << (miss_shaders_.size() * handle_size_aligned) << '\n';
#endif
    sbt_buffer_ptr = (uint8_t *)sbt_buffer_ptr + miss_shaders_.size() * handle_size_aligned;
    sbt_buffer_space_left -= miss_shaders_.size() * handle_size_aligned;
    sbt_host_storage_ptr += miss_shaders_.size() * rt_pipeline_props.shaderGroupHandleSize;

    // Fill SBT with closest hit shaders
    if (!std::align(rt_pipeline_props.shaderGroupBaseAlignment, closest_hit_shaders_.size() * handle_size_aligned, sbt_buffer_ptr,
                    sbt_buffer_space_left)) {
        assert(false);
    }

    void *closest_hit_shaders_sbt = sbt_buffer_ptr;
    std::memcpy(closest_hit_shaders_sbt, sbt_host_storage_ptr, closest_hit_shaders_.size() * handle_size_aligned);
#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "Closest hit shaders SBT entry: offset = " << ((uint64_t)closest_hit_shaders_sbt - (uint64_t)sbt_buffer_base_ptr)
              << " | size = " << (closest_hit_shaders_.size() * handle_size_aligned) << '\n';
#endif
    sbt_buffer_ptr = (uint8_t *)sbt_buffer_ptr + closest_hit_shaders_.size() * handle_size_aligned;
    sbt_buffer_space_left -= closest_hit_shaders_.size() * handle_size_aligned;
    sbt_host_storage_ptr += closest_hit_shaders_.size() * rt_pipeline_props.shaderGroupHandleSize;

    sbt_buffer_.memory().unmap();
}

void Pipeline::DeferBuild() {
    assert(deferred_op_ == VK_NULL_HANDLE);
    const VkResult result = vk::CreateDeferredOperationKHR(device_->handle(), nullptr, &deferred_op_);
    if (result != VK_SUCCESS) {
        ADD_FAILURE() << "vk::CreateDeferredOperationKHR returned " << string_VkResult(result);
        return;
    }
}

vkt::rt::TraceRaysSbt Pipeline::GetTraceRaysSbt() {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    vk::GetPhysicalDeviceProperties2(device_->phy(), &props2);

    const uint32_t handle_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);

    const VkDeviceAddress sbt_base_address = sbt_buffer_.address();
    VkDeviceAddress sbt_address = sbt_base_address;

    assert(sbt_address == Align<VkDeviceAddress>(sbt_address, rt_pipeline_props.shaderGroupBaseAlignment));

#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "SBT Buffer get:\n";
#endif

    VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
    ray_gen_sbt.deviceAddress = sbt_address;
    ray_gen_sbt.stride = handle_size_aligned;
    ray_gen_sbt.size = handle_size_aligned;
    sbt_address += ray_gen_sbt.size;
#ifdef VVL_DEBUG_LOG_SBT
    std::cout << "Ray Gen SBT entry: @ = " << ray_gen_sbt.deviceAddress
              << " (offset from base = " << ray_gen_sbt.deviceAddress - sbt_base_address << ") | stride = " << ray_gen_sbt.stride
              << " | size = " << ray_gen_sbt.size << '\n';
#endif

    VkStridedDeviceAddressRegionKHR miss_sbt{};
    if (!miss_shaders_.empty()) {
        sbt_address = Align<VkDeviceAddress>(sbt_address, rt_pipeline_props.shaderGroupBaseAlignment);
        miss_sbt.deviceAddress = sbt_address;
        miss_sbt.stride = handle_size_aligned;
        miss_sbt.size = miss_shaders_.size() * handle_size_aligned;
        sbt_address += miss_sbt.size;
#ifdef VVL_DEBUG_LOG_SBT
        std::cout << "Miss SBT entry: @ = " << miss_sbt.deviceAddress
                  << " (offset from base = " << miss_sbt.deviceAddress - sbt_base_address << ") | stride = " << miss_sbt.stride
                  << " | size = " << miss_sbt.size << '\n';
#endif
    }

    VkStridedDeviceAddressRegionKHR closest_hit_sbt{};
    if (!closest_hit_shaders_.empty()) {
        sbt_address = Align<VkDeviceAddress>(sbt_address, rt_pipeline_props.shaderGroupBaseAlignment);
        closest_hit_sbt.deviceAddress = sbt_address;
        closest_hit_sbt.stride = handle_size_aligned;
        closest_hit_sbt.size = closest_hit_shaders_.size() * handle_size_aligned;
        sbt_address += closest_hit_sbt.size;
#ifdef VVL_DEBUG_LOG_SBT
        std::cout << "Closest hit SBT entry: @ = " << closest_hit_sbt.deviceAddress
                  << " (offset from base = " << closest_hit_sbt.deviceAddress - sbt_base_address
                  << ") | stride = " << closest_hit_sbt.stride << " | size = " << closest_hit_sbt.size << '\n';
#endif
    }

    VkStridedDeviceAddressRegionKHR empty_sbt{};

    TraceRaysSbt out{};
    out.ray_gen_sbt = ray_gen_sbt;
    out.miss_sbt = miss_sbt;
    out.hit_sbt = closest_hit_sbt;
    out.callable_sbt = empty_sbt;
    return out;
}

uint32_t Pipeline::GetShaderGroupsCount() {
    uint32_t shader_groups_count = 0;
    if (ray_gen_) ++shader_groups_count;
    shader_groups_count += size32(miss_shaders_);
    shader_groups_count += size32(closest_hit_shaders_);
    return shader_groups_count;
}

std::vector<uint8_t> Pipeline::GetRayTracingShaderGroupHandles() {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    test_.GetPhysicalDeviceProperties2(rt_pipeline_props);

    // Get shader group handles to fill shader binding tables (SBT)
    // Consider that handles are stored aligned to shaderGroupHandleSize
    const uint32_t sbt_size = shader_group_cis_.size() * rt_pipeline_props.shaderGroupHandleSize;
    std::vector<uint8_t> sbt_host_storage(sbt_size);

    // #ARNO_TODO use correct group count
    const uint32_t shader_group_count = GetShaderGroupsCount();
    const VkResult result =
        vk::GetRayTracingShaderGroupHandlesKHR(*device_, Handle(), 0, shader_group_count, sbt_size, sbt_host_storage.data());
    if (IsValueIn(result, {VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY})) {
        assert(false);
    }
    return sbt_host_storage;
}

std::vector<uint8_t> Pipeline::GetRayTracingCaptureReplayShaderGroupHandles() {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    test_.GetPhysicalDeviceProperties2(rt_pipeline_props);

    // Get shader group handles to fill ray gen shader binding table (SBT)
    const uint32_t handle_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);
    const uint32_t sbt_size = shader_group_cis_.size() * handle_size_aligned;
    std::vector<uint8_t> sbt_host_storage(sbt_size);

    const VkResult result =
        vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(*device_, Handle(), 0, 1, sbt_size, sbt_host_storage.data());
    if (IsValueIn(result, {VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY})) {
        assert(false);
    }
    return sbt_host_storage;
}

}  // namespace rt
}  // namespace vkt
