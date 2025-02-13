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

struct PositiveSyncValRayTracing : public VkSyncValTest {};

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

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
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
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

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

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    // Build in update mode READs source acceleration structure
    blas2.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    // WRITE to source acceleration structure
    vk::CmdFillBuffer(m_command_buffer, src_accel_buffer, 0, 4, 0x80486);
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRayTracing, UpdateAccelerationStructureInPlace) {
    TEST_DESCRIPTION("In-place update reads and writes the same acceleration structure, ensure this does not trigger error");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitRayTracing());

    // Initial build
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
    blas.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Update acceleration structure in-place
    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetSrcAS(blas.GetDstAS());
    blas.SetupBuild(true);

    m_command_buffer.Begin();
    blas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
}
