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

#include "../framework/sync_val_tests.h"
#include "../framework/pipeline_helper.h"
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

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
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

TEST_F(NegativeSyncValRayTracing, TraceAfterBuild) {
    TEST_DESCRIPTION("Trace rays against TLAS without waiting for TLAS build completion");
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);

    // Build TLAS
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);

    // Start tracing ways without synchronization with TLAS build command
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, TraceAfterBuildIndirect) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    vkt::Buffer indirect_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);

    // Build TLAS
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);

    // Start tracing rays while TLAS build is still in progress
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, indirect_buffer.Address());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, TraceAfterBuildIndirect2) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());

    vkt::Buffer indirect_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);

    // Build TLAS
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);

    // Start tracing rays while TLAS build is still in progress
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, indirect_buffer.Address());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteTLASDuringTrace) {
    TEST_DESCRIPTION("Write to TLAS buffer while AS is being used by CmdTraceRays");
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    const vkt::Buffer& tlas_buffer = tlas->GetDstAS()->GetBuffer();

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas->GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, tlas_buffer, 0, sizeof(uint32_t), 0x42);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteTLASDuringTraceIndirect) {
    TEST_DESCRIPTION("Write to TLAS buffer while AS is being used by CmdTraceRays");
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    const vkt::Buffer& tlas_buffer = tlas->GetDstAS()->GetBuffer();

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas->GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    vkt::Buffer indirect_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, indirect_buffer.Address());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, tlas_buffer, 0, sizeof(uint32_t), 0x42);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteTLASDuringTraceIndirect2) {
    TEST_DESCRIPTION("Write to TLAS buffer while AS is being used by CmdTraceRays");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    const vkt::Buffer& tlas_buffer = tlas->GetDstAS()->GetBuffer();

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas->GetDstAS()->handle());

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, indirect_buffer.Address());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, tlas_buffer, 0, sizeof(uint32_t), 0x42);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteToTraceIndirectBuffer) {
    TEST_DESCRIPTION("Run trace indirect command and write to indirect buffer at the same time");
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas->GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    vkt::Buffer indirect_buffer(*m_device, 3 * sizeof(uint32_t),
                                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    // Test indirect buffer validation
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdFillBuffer(m_command_buffer, indirect_buffer, 0, sizeof(uint32_t), 19937);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, indirect_buffer.Address());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test indirect buffer access update
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, indirect_buffer.Address());
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, indirect_buffer, 0, sizeof(uint32_t), 19937);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, WriteToTraceIndirectBuffer2) {
    TEST_DESCRIPTION("Run trace indirect command and write to indirect buffer at the same time");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas->GetDstAS()->handle());

    vkt::Buffer indirect_buffer(*m_device, 1024 * sizeof(uint32_t),
                                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    // Test indirect buffer validation
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdFillBuffer(m_command_buffer, indirect_buffer, 0, sizeof(uint32_t), 19937);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, indirect_buffer.Address());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test indirect buffer access update
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, indirect_buffer.Address());
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, indirect_buffer, 0, sizeof(uint32_t), 19937);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, RayQueryAfterBuild) {
    TEST_DESCRIPTION("Trace rays against TLAS using ray queries without waiting for TLAS build completion");
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();

    // Create TLAS (but not build it yet)
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    CreateComputePipelineHelper pipeline = GetRayQueryComputePipeline(tlas.GetDstAS()->handle());

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);

    // Build TLAS
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);

    // Start tracing rays without synchronization with TLAS build command
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, CopyAfterRayQuery) {
    TEST_DESCRIPTION("Copy to TLAS backing buffer without waiting for ray query operation");
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> tlas = BuildTLAS(*blas->GetDstAS());

    CreateComputePipelineHelper pipeline = GetRayQueryComputePipeline(tlas->GetDstAS()->handle());

    const vkt::Buffer& tlas_buffer = tlas->GetDstAS()->GetBuffer();
    vkt::Buffer src_buffer(*m_device, tlas_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);

    // Trace rays
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // Write to the TLAS backing buffer while it is still being used by the dispatch call
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(src_buffer, tlas_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, ASCopySourceHazard) {
    TEST_DESCRIPTION("Hazard when accessing AS copy source");
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas_src = BuildBLAS();

    vkt::as::BuildGeometryInfoKHR blas_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_dst.SetupBuild(true);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_src->GetDstAS()->handle();
    copy_info.dst = blas_dst.GetDstAS()->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    const vkt::Buffer& blas_src_buffer = blas_src->GetDstAS()->GetBuffer();
    vkt::Buffer buffer(*m_device, blas_src_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Test validation
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer, blas_src_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test update
    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(buffer, blas_src_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, ASCopyDestinationHazard) {
    TEST_DESCRIPTION("Hazard when accessing AS copy destination");
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas_src = BuildBLAS();

    vkt::as::BuildGeometryInfoKHR blas_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_dst.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    blas_dst.SetupBuild(true);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_src->GetDstAS()->handle();
    copy_info.dst = blas_dst.GetDstAS()->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    const vkt::Buffer& blas_dst_buffer = blas_dst.GetDstAS()->GetBuffer();
    vkt::Buffer buffer(*m_device, blas_dst_buffer.CreateInfo().size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Test validation
    m_command_buffer.Begin();
    m_command_buffer.Copy(blas_dst_buffer, buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test update
    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.Copy(buffer, blas_dst_buffer);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, SerializeASHazard) {
    TEST_DESCRIPTION("Test read accesses when acceleration structure is serialized to a buffer");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by TestICD: serialization size query";
    }

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    const vkt::Buffer& blas_buffer = blas->GetDstAS()->GetBuffer();

    vkt::Buffer serialization_buffer = GetSerializationDeserializationBuffer(*blas->GetDstAS());

    VkCopyAccelerationStructureToMemoryInfoKHR copy_to_memory_info = vku::InitStructHelper();
    copy_to_memory_info.src = blas->GetDstAS()->handle();
    copy_to_memory_info.dst = VkDeviceOrHostAddressKHR{serialization_buffer.Address()};
    copy_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    // Test Validation
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, blas_buffer, 0, sizeof(uint32_t), 0x314159);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_to_memory_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test update
    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_to_memory_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdFillBuffer(m_command_buffer, blas_buffer, 0, sizeof(uint32_t), 0x314159);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncValRayTracing, DeserializeASHazard) {
    TEST_DESCRIPTION("Test write accesses when acceleration structure is deserialized from a buffer");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by TestICD: serialization size query";
    }

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();

    vkt::as::BuildGeometryInfoKHR blas2 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas2.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    blas2.SetupBuild(true);
    const vkt::Buffer& blas2_buffer = blas2.GetDstAS()->GetBuffer();

    vkt::Buffer serialization_buffer = GetSerializationDeserializationBuffer(*blas->GetDstAS());

    VkCopyMemoryToAccelerationStructureInfoKHR copy_from_memory_info = vku::InitStructHelper();
    copy_from_memory_info.src = VkDeviceOrHostAddressConstKHR{serialization_buffer.Address()};
    copy_from_memory_info.dst = blas2.GetDstAS()->handle();
    copy_from_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // Test Validation
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, blas2_buffer, 0, sizeof(uint32_t), 0x314159);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_from_memory_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Test update
    m_command_buffer.Begin();
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_from_memory_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdFillBuffer(m_command_buffer, blas2_buffer, 0, sizeof(uint32_t), 0x314159);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
