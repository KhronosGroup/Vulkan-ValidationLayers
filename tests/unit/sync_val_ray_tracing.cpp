/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"

struct NegativeSyncValRayTracing : public VkSyncValTest {};

TEST_F(NegativeSyncValRayTracing, ScratchBufferHazard) {
    TEST_DESCRIPTION("Write to scratch buffer during acceleration structure build");
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD: buffer device address consistent support across different functions";
    }

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetDeviceScratchAdditionalFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    const vkt::Buffer& scratch_buffer = *blas.GetScratchBuffer();

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdFillBuffer(m_command_buffer, scratch_buffer, 0, sizeof(uint32_t), 0x42);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, AccelerationStructureBufferHazard) {
    TEST_DESCRIPTION("Write to accelerationn structure buffer during acceleration structure build");
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    blas.SetupBuild(true);

    const vkt::Buffer& accel_buffer = blas.GetDstAS()->GetBuffer();
    vkt::Buffer copy_buffer(*m_device, accel_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(accel_buffer, copy_buffer);  // READ acceleration structure
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);  // WRITE without proper barrier
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, SourceAccelerationStructureHazard) {
    TEST_DESCRIPTION("Use acceleration structure as a source during update (READ) while it is still being built (WRITE)");
    RETURN_IF_SKIP(InitRayTracing());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.SetupBuild(true);

    auto blas2 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas2.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas2.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas2.SetSrcAS(blas.GetDstAS());
    blas2.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas2.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteVertexDataDuringBuild) {
    TEST_DESCRIPTION("Use vertex buffer as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceIndexedTriangleInfo(*m_device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& vertex_buffer = triangles_geometry.device_vertex_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferCopy region{};
    region.size = 4;

    // Test validation
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, vertex_buffer, 1, &region);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, vertex_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteVertexDataDuringBuild2) {
    TEST_DESCRIPTION("Use vertex buffer for non-indexed geometry as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& vertex_buffer = triangles_geometry.device_vertex_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferCopy region{};
    region.size = 4;

    // Test validation
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, vertex_buffer, 1, &region);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, vertex_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteIndexDataDuringBuild) {
    TEST_DESCRIPTION("Use index buffer as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceIndexedTriangleInfo(*m_device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& index_buffer = triangles_geometry.device_index_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferCopy region{};
    region.size = 4;

    // Test validation
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, index_buffer, 1, &region);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, index_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteTransformDataDuringBuild) {
    TEST_DESCRIPTION("Use transform data as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& transform_data = triangles_geometry.device_transform_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, transform_data.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Test validation
    m_command_buffer.Begin();
    m_command_buffer.Copy(src_buffer, transform_data);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(src_buffer, transform_data);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteAABBDataDuringBuild) {
    TEST_DESCRIPTION("Use AABB data as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceAABBInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& aabbs_geometry = blas.GetGeometries()[0].GetAABBs();
    const vkt::Buffer& aabb_buffer = aabbs_geometry.device_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, aabb_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Test validation
    m_command_buffer.Begin();
    m_command_buffer.Copy(src_buffer, aabb_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(src_buffer, aabb_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteInstanceDataDuringBuild) {
    TEST_DESCRIPTION("Use instance data as a copy destination while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device);
    auto blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
        vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry)));
    blas->SetupBuild(true);
    m_command_buffer.Begin();
    blas->VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    const auto& instance = tlas.GetGeometries()[0].GetInstance();
    tlas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, instance.buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Test validation
    m_command_buffer.Begin();
    m_command_buffer.Copy(src_buffer, instance.buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test access update (that copy can see previous AS access)
    m_command_buffer.Begin();
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(src_buffer, instance.buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
