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

struct PositiveSyncValRayTracing : public VkSyncValTest {};

vkt::Buffer VkSyncValTest::GetSerializationDeserializationBuffer(const vkt::as::AccelerationStructureKHR& as) {
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, 1);
    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer, 1, &as.handle(),
                                                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, query_pool, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    uint32_t serialization_size = 0;
    query_pool.Results(0, 1, sizeof(uint32_t), &serialization_size, 0);
    return vkt::Buffer(*m_device, serialization_size,
                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
}

CreateComputePipelineHelper VkSyncValTest::GetRayQueryComputePipeline(VkAccelerationStructureKHR as) {
    char const* cs_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
            rayQueryProceedEXT(query);
        }
    )glsl";
    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();
    pipeline.descriptor_set_.WriteDescriptorAccelStruct(0, 1, &as);
    pipeline.descriptor_set_.UpdateDescriptorSets();
    return pipeline;
}

std::unique_ptr<vkt::rt::Pipeline> VkSyncValTest::GetTraceRaysPipeline(VkAccelerationStructureKHR as) {
    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 hit;
        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    auto pipeline = std::make_unique<vkt::rt::Pipeline>(*this, m_device);
    pipeline->SetGlslRayGenShader(ray_gen);
    pipeline->AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline->CreateDescriptorSet();
    pipeline->GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &as);
    pipeline->GetDescriptorSet().UpdateDescriptorSets();
    pipeline->Build();
    return pipeline;
}

std::unique_ptr<vkt::as::BuildGeometryInfoKHR> VkSyncValTest::BuildBLAS() {
    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    auto blas =
        std::make_unique<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
    blas->GetDstAS()->SetBufferUsageFlags(usage);
    blas->SetupBuild(true);
    m_command_buffer.Begin();
    blas->VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    return blas;
}

std::unique_ptr<vkt::as::BuildGeometryInfoKHR> VkSyncValTest::BuildTLAS(const vkt::as::AccelerationStructureKHR& blas) {
    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto tlas = std::make_unique<vkt::as::BuildGeometryInfoKHR>(
        vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas));
    tlas->GetDstAS()->SetBufferUsageFlags(usage);
    tlas->SetupBuild(true);
    m_command_buffer.Begin();
    tlas->VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    return tlas;
}

TEST_F(PositiveSyncValRayTracing, BuildAccelerationStructure) {
    TEST_DESCRIPTION("Just build it");
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, WriteToAccelerationStructureBuffer) {
    TEST_DESCRIPTION("Write to a vacant region of acceleration structure buffer during acceleration structure build");
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    // Reserve initial buffer region for non-acceleration-structure business.
    // Acceleration structure contents start after it.
    blas.GetDstAS()->SetOffset(256);
    blas.SetupBuild(true);
    const vkt::Buffer& accel_buffer = blas.GetDstAS()->GetBuffer();

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    // Write the answer into the first 4 bytes.
    // This should not interfere with acceleration structure build accesses
    vk::CmdFillBuffer(m_command_buffer, accel_buffer, 0, 4, 0x42);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, BuildAccelerationStructureThenBarrier) {
    TEST_DESCRIPTION("Use barrier to wait for acceleration structure accesses");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    blas.SetupBuild(true);

    const vkt::Buffer& accel_buffer = blas.GetDstAS()->GetBuffer();

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.buffer = accel_buffer;
    barrier.offset = 0;
    barrier.size = 4;

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Barrier(barrier);
    vk::CmdFillBuffer(m_command_buffer, accel_buffer, 0, 4, 0x80386);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, UseSourceAccelerationStructureThenBarrier) {
    TEST_DESCRIPTION("Use barrier to wait for the source acceleration structure when building in update mode");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    // Prepare source acceleration structure
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Create another acceleration structure to be built in update mode
    auto blas2 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas2.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas2.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas2.SetSrcAS(blas.GetDstAS());
    blas2.SetupBuild(true);
    const vkt::Buffer& src_accel_buffer = blas2.GetSrcAS()->GetBuffer();

    // Execution dependency is sufficient to prevent WRITE after READ
    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.buffer = src_accel_buffer;
    barrier.offset = 0;
    barrier.size = 4;

    m_command_buffer.Begin();
    // Build in update mode READs source acceleration structure
    blas2.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Barrier(barrier);
    // WRITE to source acceleration structure
    vk::CmdFillBuffer(m_command_buffer, src_accel_buffer, 0, 4, 0x80486);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, UpdateAccelerationStructureInPlace) {
    TEST_DESCRIPTION("In-place update reads and writes the same acceleration structure, ensure this does not trigger error");
    RETURN_IF_SKIP(InitRayTracing());

    // Initial build
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Update acceleration structure in-place
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetSrcAS(blas.GetDstAS());
    blas.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, ReadVertexDataDuringBuild) {
    TEST_DESCRIPTION("Use vertex buffer as a copy source while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& vertex_buffer = triangles_geometry.device_vertex_buffer;
    blas.SetupBuild(true);

    vkt::Buffer dst_buffer(*m_device, vertex_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Copy(vertex_buffer, dst_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, WriteVertexDataDuringBuild) {
    TEST_DESCRIPTION("Write to a vacant region of the vertex data buffer during AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // Build custom vertex data buffer where data region starts at some non-zero offset
    constexpr std::array vertices = {// Vertex 0
                                     10.0f, 10.0f, 0.0f,
                                     // Vertex 1
                                     -10.0f, 10.0f, 0.0f,
                                     // Vertex 2
                                     0.0f, -10.0f, 0.0f};

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer vertex_buffer(*m_device, 1024, buffer_usage, kHostVisibleMemProps, &alloc_flags);
    auto vertex_buffer_ptr = static_cast<float*>(vertex_buffer.Memory().Map());
    std::copy(vertices.begin(), vertices.end(), vertex_buffer_ptr);

    // Specify offset (4 bytes) so vertex data does not start immediately from the beginning of the buffer.
    geometry.SetTrianglesDeviceVertexBuffer(std::move(vertex_buffer), uint32_t(vertices.size() / 3) - 1, VK_FORMAT_R32G32B32_SFLOAT,
                                            3 * sizeof(float), 4);

    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    blas.SetupBuild(true);
    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);

    // Write arbitrary data into the first 4 bytes.
    // This should not interfere with acceleration structure build accesses that read vertex data
    vk::CmdFillBuffer(m_command_buffer, blas.GetGeometries()[0].GetTriangles().device_vertex_buffer, 0, 4, 0x42);

    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, ReadIndexDataDuringBuild) {
    TEST_DESCRIPTION("Use index buffer as a copy source while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceIndexedTriangleInfo(*m_device, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& index_buffer = triangles_geometry.device_index_buffer;
    blas.SetupBuild(true);

    vkt::Buffer dst_buffer(*m_device, index_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Copy(index_buffer, dst_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, ReadTransformDataDuringBuild) {
    TEST_DESCRIPTION("Use transform data as a copy source while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& transform_data = triangles_geometry.device_transform_buffer;
    blas.SetupBuild(true);

    vkt::Buffer dst_buffer(*m_device, transform_data.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Copy(transform_data, dst_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, ReadAABBDataDuringBuild) {
    TEST_DESCRIPTION("Use AABB data as a copy source while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceAABBInfo(*m_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& aabbs_geometry = blas.GetGeometries()[0].GetAABBs();
    const vkt::Buffer& aabb_buffer = aabbs_geometry.device_buffer;
    blas.SetupBuild(true);

    vkt::Buffer dst_buffer(*m_device, aabb_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Copy(aabb_buffer, dst_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, ReadInstanceDataDuringBuild) {
    TEST_DESCRIPTION("Use instance data as a copy source while it is used by the AS build operation");
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    const auto& instance = tlas.GetGeometries()[0].GetInstance();
    tlas.SetupBuild(true);
    vkt::Buffer dst_buffer(*m_device, instance.buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Copy(instance.buffer, dst_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, TraceAfterBuild) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);

    // Build TLAS
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    // Wait for TLAS build completion
    m_command_buffer.Barrier(barrier);
    // Trace against TLAS
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, TraceAfterBuildIndirect) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());
    const vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline->GetTraceRaysSbt();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer indirect_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Barrier(barrier);
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, indirect_buffer.Address());
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, TraceAfterBuildIndirect2) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    std::unique_ptr<vkt::rt::Pipeline> pipeline = GetTraceRaysPipeline(tlas.GetDstAS()->handle());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->GetPipelineLayout(), 0, 1,
                              &pipeline->GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Barrier(barrier);
    vk::CmdTraceRaysIndirect2KHR(m_command_buffer, indirect_buffer.Address());
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, RayQueryAfterBuild) {
    TEST_DESCRIPTION("Build TLAS and trace rays");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitRayTracing());

    // Create TLAS (but not build it yet)
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas->GetDstAS());
    tlas.SetupBuild(true);

    CreateComputePipelineHelper pipeline = GetRayQueryComputePipeline(tlas.GetDstAS()->handle());

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    barrier.buffer = tlas.GetDstAS()->GetBuffer();
    barrier.offset = 0;
    barrier.size = tlas.GetDstAS()->GetBuffer().CreateInfo().size;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);

    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.Barrier(barrier);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, WriteIndexDataThenBuild) {
    TEST_DESCRIPTION("Test that a barrier protects index data writes from subsequent acceleration structure build accesses");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    auto geometry = vkt::as::blueprint::GeometrySimpleOnDeviceIndexedTriangleInfo(*m_device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    const auto& triangles_geometry = blas.GetGeometries()[0].GetTriangles();
    const vkt::Buffer& index_buffer = triangles_geometry.device_index_buffer;
    blas.SetupBuild(true);

    vkt::Buffer src_buffer(*m_device, index_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier.buffer = index_buffer;
    barrier.offset = 0;
    barrier.size = index_buffer.CreateInfo().size;

    m_command_buffer.Begin();
    m_command_buffer.Copy(src_buffer, index_buffer);
    m_command_buffer.Barrier(barrier);
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, InvalidMaxVertexValue) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9810
    TEST_DESCRIPTION("Test invalid maxVertex does not break internal tracking");
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    // SyncVal operates with the assumption that API usage is valid according to core validation.
    // For invalid input synval does basic checks to prevent crashes but usually does not do much above that.
    // For out of bounds buffer access syncval does range clamp to avoid false positives but otherwise
    // relies on the core checks to report such issues.
    //
    // Test that large maxVertex value does not cause invalid internal tracking when the range of AS geometry
    // data overlaps with another resource tracked by syncval (in this case it's a regular buffer).
    // In the case of regression it's possible to have false positive when access to one resource can be tracked
    // as access to another resource (completed unrelated).
    blas.GetGeometries()[0].SetTrianglesMaxVertex(vvl::kU32Max - 10000);

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, buffer, 0, VK_WHOLE_SIZE, 0x314159);
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, CopyAS) {
    TEST_DESCRIPTION("Clone acceleration structure");
    RETURN_IF_SKIP(InitRayTracing());

    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas_src = BuildBLAS();

    vkt::as::BuildGeometryInfoKHR blas_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_dst.SetupBuild(true);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_src->GetDstAS()->handle();
    copy_info.dst = blas_dst.GetDstAS()->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, CompactAS) {
    TEST_DESCRIPTION("Compact acceleration structure");
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas_src = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_src.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    blas_src.SetupBuild(true);
    m_command_buffer.Begin();
    blas_src.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::as::BuildGeometryInfoKHR blas_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_dst.SetupBuild(true);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_src.GetDstAS()->handle();
    copy_info.dst = blas_dst.GetDstAS()->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, CopyASWithBarrier) {
    TEST_DESCRIPTION("Use barrier to protect AS copy accesses");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    vkt::as::BuildGeometryInfoKHR blas_src = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_src.GetDstAS()->SetBufferUsageFlags(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    blas_src.SetupBuild(true);
    m_command_buffer.Begin();
    blas_src.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::as::BuildGeometryInfoKHR blas_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_dst.SetupBuild(true);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_src.GetDstAS()->handle();
    copy_info.dst = blas_dst.GetDstAS()->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    const vkt::Buffer& blas_src_buffer = blas_src.GetDstAS()->GetBuffer();
    vkt::Buffer buffer(*m_device, blas_src_buffer.CreateInfo().size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer, blas_src_buffer);
    m_command_buffer.Barrier(barrier);  // Prevent READ-AFTER-WRITE
    vk::CmdCopyAccelerationStructureKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, SerializeDeserializeAS) {
    TEST_DESCRIPTION("Serialize AS to a buffer. Then deserialize the buffer into another AS object");
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by TestICD: serialization size query";
    }

    // The source blas for serialization
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();

    // Deserialize into blas2
    vkt::as::BuildGeometryInfoKHR blas2 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas2.SetupBuild(true);

    vkt::Buffer serialization_buffer = GetSerializationDeserializationBuffer(*blas->GetDstAS());

    VkCopyAccelerationStructureToMemoryInfoKHR copy_to_memory_info = vku::InitStructHelper();
    copy_to_memory_info.src = blas->GetDstAS()->handle();
    copy_to_memory_info.dst = VkDeviceOrHostAddressKHR{serialization_buffer.Address()};
    copy_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_to_memory_info);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_from_memory_info = vku::InitStructHelper();
    copy_from_memory_info.src = VkDeviceOrHostAddressConstKHR{serialization_buffer.Address()};
    copy_from_memory_info.dst = blas2.GetDstAS()->handle();
    copy_from_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
    m_command_buffer.Begin();
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_from_memory_info);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveSyncValRayTracing, SerializeDeserializeASWithBarrier) {
    TEST_DESCRIPTION("Use barrier to protect accesses to serialize/deserialize buffer");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by TestICD: serialization size query";
    }

    // The source blas for serialization
    std::unique_ptr<vkt::as::BuildGeometryInfoKHR> blas = BuildBLAS();

    // Deserialize into blas2
    vkt::as::BuildGeometryInfoKHR blas2 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas2.SetupBuild(true);

    vkt::Buffer serialization_buffer = GetSerializationDeserializationBuffer(*blas->GetDstAS());

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.buffer = serialization_buffer;
    barrier.size = serialization_buffer.CreateInfo().size;

    VkCopyAccelerationStructureToMemoryInfoKHR copy_to_memory_info = vku::InitStructHelper();
    copy_to_memory_info.src = blas->GetDstAS()->handle();
    copy_to_memory_info.dst = VkDeviceOrHostAddressKHR{serialization_buffer.Address()};
    copy_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    VkCopyMemoryToAccelerationStructureInfoKHR copy_from_memory_info = vku::InitStructHelper();
    copy_from_memory_info.src = VkDeviceOrHostAddressConstKHR{serialization_buffer.Address()};
    copy_from_memory_info.dst = blas2.GetDstAS()->handle();
    copy_from_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer, &copy_to_memory_info);
    m_command_buffer.Barrier(barrier);
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer, &copy_from_memory_info);
    m_command_buffer.End();
}
