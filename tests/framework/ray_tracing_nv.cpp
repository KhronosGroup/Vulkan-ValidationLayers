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

#include "ray_tracing_nv.h"

namespace nv {
namespace rt {

void GetSimpleGeometryForAccelerationStructureTests(const vkt::Device &device, vkt::Buffer *vbo, vkt::Buffer *ibo,
                                                    VkGeometryNV *geometry, VkDeviceSize offset, bool buffer_device_address) {
    VkBufferUsageFlags usage =
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    void *alloc_pnext = nullptr;
    if (buffer_device_address) {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        alloc_pnext = &alloc_flags;
    }
    vbo->init(device, 1024, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, alloc_pnext);
    ibo->init(device, 1024, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, alloc_pnext);

    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {{0, 1, 2}};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo->memory().map();
    std::memcpy(mapped_vbo_buffer_data + offset, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo->memory().unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo->memory().map();
    std::memcpy(mapped_ibo_buffer_data + offset, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo->memory().unmap();

    *geometry = {};
    geometry->sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    geometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry->geometry.triangles.vertexData = vbo->handle();
    geometry->geometry.triangles.vertexOffset = 0;
    geometry->geometry.triangles.vertexCount = 3;
    geometry->geometry.triangles.vertexStride = 12;
    geometry->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry->geometry.triangles.indexData = ibo->handle();
    geometry->geometry.triangles.indexOffset = 0;
    geometry->geometry.triangles.indexCount = 3;
    geometry->geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry->geometry.triangles.transformData = VK_NULL_HANDLE;
    geometry->geometry.triangles.transformOffset = 0;
    geometry->geometry.aabbs = {};
    geometry->geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
}
}  // namespace rt
}  // namespace nv
