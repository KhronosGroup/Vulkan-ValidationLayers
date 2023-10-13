/*
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

#include "binding.h"

#include <memory>

namespace vkt {
// acceleration structure
namespace as {

// Helper classes to create instances of:
// - VkAccelerationStructureGeometryKHR
// - VkAccelerationStructureCreateInfoKHR
// - VkAccelerationStructureBuildGeometryInfoKHR

// The vkt::as::blueprint namespace (bottom of file) contains functions to readily create a valid instance of those classes.
// Those instances are typically modified using the available public methods.
// When done with modifications, call the Build() method to build the internal Vulkan objects.
// Access them using relevant methods, eg: handle(), GetVkObj()...

// 3 types of geometry handled: Triangle, AABB and VkAccelerationStructureInstanceKHR (used in top level acceleration structures)
// Those objects can be managed on the device, using the ***Device*** methods,
// or on the host using the ***Host*** methods
class GeometryKHR {
  public:
    enum class Type { Triangle, AABB, Instance, _INTERNAL_UNSPECIFIED };
    struct Triangles {
        vkt::Buffer device_vertex_buffer;
        std::unique_ptr<float[]> host_vertex_buffer;
        vkt::Buffer device_index_buffer;
        std::unique_ptr<uint32_t[]> host_index_buffer;
    };
    struct AABBs {
        vkt::Buffer device_buffer;
        std::unique_ptr<VkAabbPositionsKHR[]> host_buffer;
    };
    struct Instance {
        // As of now, only manage one instance
        std::unique_ptr<VkAccelerationStructureInstanceKHR> vk_instance{};
        // Used to (eventually, no need for host instance) store on device instance
        vkt::Buffer buffer;
    };

    ~GeometryKHR() = default;
    GeometryKHR();
    GeometryKHR(const GeometryKHR&) = delete;
    GeometryKHR(GeometryKHR&&) = default;
    GeometryKHR& operator=(GeometryKHR&&) = default;
    GeometryKHR& operator=(const GeometryKHR&) = delete;

    // Common methods for all types
    GeometryKHR& SetType(Type type);
    GeometryKHR& SetPrimitiveCount(uint32_t primitiveCount);
    GeometryKHR& SetStride(VkDeviceSize stride);
    // Triangle
    GeometryKHR& SetTrianglesDeviceVertexBuffer(vkt::Buffer&& vertex_buffer, uint32_t max_vertex,
                                                VkFormat vertex_format = VK_FORMAT_R32G32B32_SFLOAT,
                                                VkDeviceSize stride = 3 * sizeof(float));
    GeometryKHR& SetTrianglesHostVertexBuffer(std::unique_ptr<float[]>&& vertex_buffer, uint32_t max_vertex,
                                              VkDeviceSize stride = 3 * sizeof(float));
    GeometryKHR& SetTrianglesDeviceIndexBuffer(vkt::Buffer&& index_buffer, VkIndexType index_type = VK_INDEX_TYPE_UINT32);
    GeometryKHR& SetTrianglesHostIndexBuffer(std::unique_ptr<uint32_t[]> index_buffer);
    GeometryKHR& SetTrianglesIndexType(VkIndexType index_type);
    // AABB
    GeometryKHR& SetAABBsDeviceBuffer(vkt::Buffer&& buffer, VkDeviceSize stride = sizeof(VkAabbPositionsKHR));
    GeometryKHR& SetAABBsHostBuffer(std::unique_ptr<VkAabbPositionsKHR[]> buffer, VkDeviceSize stride = sizeof(VkAabbPositionsKHR));
    // Instance
    GeometryKHR& SetInstanceDeviceAccelStructRef(const vkt::Device& device, VkAccelerationStructureKHR bottom_level_as);
    GeometryKHR& SetInstanceHostAccelStructRef(VkAccelerationStructureKHR bottom_level_as);

    GeometryKHR& Build();

    const auto& GetVkObj() const { return vk_obj_; }
    VkAccelerationStructureBuildRangeInfoKHR GetFullBuildRange() const;
    const auto& GetTriangles() const { return triangles_; }
    const auto& GetAABBs() const { return aabbs_; }

  private:
    VkAccelerationStructureGeometryKHR vk_obj_;
    Type type_ = Type::_INTERNAL_UNSPECIFIED;
    uint32_t primitiveCount_ = 0;
    Triangles triangles_;
    AABBs aabbs_;
    Instance instance_;
};

class AccelerationStructureKHR : public vkt::internal::NonDispHandle<VkAccelerationStructureKHR> {
  public:
    ~AccelerationStructureKHR() { Destroy(); }
    AccelerationStructureKHR();
    AccelerationStructureKHR(AccelerationStructureKHR&& rhs) = default;
    AccelerationStructureKHR& operator=(AccelerationStructureKHR&&) = default;
    AccelerationStructureKHR& operator=(const AccelerationStructureKHR&) = delete;

    AccelerationStructureKHR& SetSize(VkDeviceSize size);
    AccelerationStructureKHR& SetOffset(VkDeviceSize offset);
    AccelerationStructureKHR& SetType(VkAccelerationStructureTypeKHR type);
    AccelerationStructureKHR& SetFlags(VkAccelerationStructureCreateFlagsKHR flags);
    AccelerationStructureKHR& SetDeviceBuffer(vkt::Buffer&& buffer);
    AccelerationStructureKHR& SetDeviceBufferMemoryAllocateFlags(VkMemoryAllocateFlags memory_allocate_flags);
    AccelerationStructureKHR& SetDeviceBufferMemoryPropertyFlags(VkMemoryAllocateFlags memory_property_flags);
    AccelerationStructureKHR& SetDeviceBufferInitNoMem(bool buffer_init_no_mem);
    // Set it to 0 to skip buffer initialization at Build() step
    AccelerationStructureKHR& SetBufferUsageFlags(VkBufferUsageFlags usage_flags);

    VkDeviceAddress GetBufferDeviceAddress() const;

    // Null check is done in BuildGeometryInfoKHR::Build(). Object is build iff it is not null.
    void SetNull(bool is_null) { is_null_ = is_null; }
    bool IsNull() const { return is_null_; }
    void Build(const vkt::Device& device);
    bool IsBuilt() const { return initialized(); }
    void Destroy();

    const auto& GetBuffer() const { return device_buffer_; }

  private:
    bool is_null_ = false;
    VkAccelerationStructureCreateInfoKHR vk_info_;
    vkt::Buffer device_buffer_;
    VkMemoryAllocateFlags buffer_memory_allocate_flags_{};
    VkMemoryPropertyFlags buffer_memory_property_flags_{};
    VkBufferUsageFlags buffer_usage_flags_{};
    bool buffer_init_no_mem_ = false;
};

class BuildGeometryInfoKHR {
  public:
    ~BuildGeometryInfoKHR() = default;
    BuildGeometryInfoKHR();
    BuildGeometryInfoKHR(BuildGeometryInfoKHR&&) = default;
    BuildGeometryInfoKHR& operator=(BuildGeometryInfoKHR&& rhs) = default;
    BuildGeometryInfoKHR& operator=(const BuildGeometryInfoKHR&) = delete;

    BuildGeometryInfoKHR& SetType(VkAccelerationStructureTypeKHR type);
    BuildGeometryInfoKHR& SetMode(VkBuildAccelerationStructureModeKHR mode);
    BuildGeometryInfoKHR& SetFlags(VkBuildAccelerationStructureFlagsKHR flags);
    BuildGeometryInfoKHR& AddFlags(VkBuildAccelerationStructureFlagsKHR flags);
    BuildGeometryInfoKHR& SetGeometries(std::vector<GeometryKHR>&& geometries);
    // Using the same pointers for src and dst is supported
    BuildGeometryInfoKHR& SetSrcAS(std::shared_ptr<AccelerationStructureKHR> src_as);
    BuildGeometryInfoKHR& SetDstAS(std::shared_ptr<AccelerationStructureKHR> dst_as);
    BuildGeometryInfoKHR& SetScratchBuffer(std::shared_ptr<vkt::Buffer> scratch_buffer);
    BuildGeometryInfoKHR& SetDeviceScratchOffset(VkDeviceAddress offset);
    BuildGeometryInfoKHR& SetBottomLevelAS(std::shared_ptr<BuildGeometryInfoKHR> bottom_level_as);
    // Should be 0 or 1
    BuildGeometryInfoKHR& SetInfoCount(uint32_t info_count);
    BuildGeometryInfoKHR& SetNullInfos(bool use_null_infos);
    BuildGeometryInfoKHR& SetNullBuildRangeInfos(bool use_null_build_range_infos);

    // Those functions call Build() on internal resources (geometries, src and dst acceleration structures, scratch buffer),
    // then one the build acceleration structure function.
    void BuildCmdBuffer(const vkt::Device& device, VkCommandBuffer cmd_buffer, bool use_ppGeometries = true);
    void BuildCmdBufferIndirect(const vkt::Device& device, VkCommandBuffer cmd_buffer);
    void BuildHost(VkInstance instance, const vkt::Device& device);
    void VkCmdBuildAccelerationStructuresKHR(const vkt::Device& device, VkCommandBuffer cmd_buffer, bool use_ppGeometries = true);
    // TODO - indirect build not fully implemented, only cared about having a valid call at time of writing
    void VkCmdBuildAccelerationStructuresIndirectKHR(const vkt::Device& device, VkCommandBuffer cmd_buffer);
    void VkBuildAccelerationStructuresKHR(VkInstance instance, const vkt::Device& device);

    auto& GetInfo() { return vk_info_; }
    auto& GetGeometries() { return geometries_; }
    auto& GetSrcAS() { return src_as_; }
    auto& GetDstAS() { return dst_as_; }
    auto& GetBottomLevelAS() { return blas_; }
    const auto& GetScratchBuffer() const { return device_scratch_; }
    VkAccelerationStructureBuildSizesInfoKHR GetSizeInfo(VkDevice device, bool use_ppGeometries = true);

  private:
    friend void BuildAccelerationStructuresKHR(const vkt::Device& device, VkCommandBuffer cmd_buffer,
                                               std::vector<BuildGeometryInfoKHR>& infos);

    void BuildCommon(const vkt::Device& device, bool is_on_device_build, bool use_ppGeometries = true);

    uint32_t vk_info_count_ = 1;
    bool use_null_infos_ = false;
    bool use_null_build_range_infos_ = false;
    VkAccelerationStructureBuildGeometryInfoKHR vk_info_;
    std::vector<GeometryKHR> geometries_;
    std::shared_ptr<AccelerationStructureKHR> src_as_, dst_as_;
    VkDeviceAddress device_scratch_offset_ = 0;
    std::shared_ptr<vkt::Buffer> device_scratch_;
    std::unique_ptr<uint8_t[]> host_scratch_;
    std::shared_ptr<BuildGeometryInfoKHR> blas_;
};

// Helper functions
void BuildAccelerationStructuresKHR(const vkt::Device& device, VkCommandBuffer cmd_buffer,
                                    std::vector<BuildGeometryInfoKHR>& infos);

// Helper functions providing simple, valid objects.
// Calling Build() on them without further modifications results in a usable and valid Vulkan object.
// Typical usage probably is:
// {
//    vkt::as::BuildGeometryInfoKHR as_build_info = BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
//
//    // for instance:
//    as_build_info.GetDstAS().SetBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
//    as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
//
//    m_commandBuffer->begin();
//    as_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
//    m_commandBuffer->end();
// }
namespace blueprint {
GeometryKHR GeometrySimpleOnDeviceTriangleInfo(const vkt::Device& device);
GeometryKHR GeometrySimpleOnHostTriangleInfo();
GeometryKHR GeometrySimpleOnDeviceAABBInfo(const vkt::Device& device);
GeometryKHR GeometrySimpleOnHostAABBInfo();
GeometryKHR GeometrySimpleDeviceInstance(const vkt::Device& device, VkAccelerationStructureKHR device_instance);
GeometryKHR GeometrySimpleHostInstance(VkAccelerationStructureKHR host_instance);

std::shared_ptr<AccelerationStructureKHR> AccelStructNull();
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceBottomLevel(VkDeviceSize size);
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnHostBottomLevel(VkDeviceSize size);
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceTopLevel(VkDeviceSize size);

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceBottomLevel(const vkt::Device& device,
                                                                GeometryKHR::Type geometry_type = GeometryKHR::Type::Triangle);

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostBottomLevel(const vkt::Device& device,
                                                              GeometryKHR::Type geometry_type = GeometryKHR::Type::Triangle);

// on_device_bottom_level_geometry must have been built previously, and on the device
BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceTopLevel(const vkt::Device& device,
                                                             std::shared_ptr<BuildGeometryInfoKHR> on_device_bottom_level_geometry);
// on_host_bottom_level_geometry must have been built previously, and on the host
BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostTopLevel(const vkt::Device& device,
                                                           std::shared_ptr<BuildGeometryInfoKHR> on_host_bottom_level_geometry);

}  // namespace blueprint

}  // namespace as
}  // namespace vkt
