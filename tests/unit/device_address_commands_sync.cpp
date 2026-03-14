/*
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeDeviceAddressCommandsSync : public DeviceAddressCommands {
  public:
    vkt::Buffer buffer_;

    VkMemoryRangeBarrierKHR memory_range_barrier_;
    VkMemoryRangeBarriersInfoKHR memory_range_barriers_info_;
    VkDependencyInfo dependency_info_;

    void InitSyncDeviceAddressCommands() {
        AddRequiredFeature(vkt::Feature::synchronization2);
        RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

        buffer_ =
            vkt::Buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
        memory_range_barrier_ = vku::InitStructHelper();
        memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        memory_range_barrier_.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        memory_range_barrier_.srcQueueFamilyIndex = m_default_queue->family_index;
        memory_range_barrier_.dstQueueFamilyIndex = m_default_queue->family_index;
        memory_range_barrier_.addressRange = buffer_.AddressRange();
        memory_range_barrier_.addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR;

        memory_range_barriers_info_ = vku::InitStructHelper();
        memory_range_barriers_info_.memoryRangeBarrierCount = 1u;
        memory_range_barriers_info_.pMemoryRangeBarriers = &memory_range_barrier_;

        dependency_info_ = vku::InitStructHelper(&memory_range_barriers_info_);
    }
};

TEST_F(NegativeDeviceAddressCommandsSync, MemoryBarrierSrcExclusive) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());

    m_command_buffer.Begin();

    memory_range_barrier_.srcQueueFamilyIndex = queue_family_count;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-address-13087");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MemoryBarrierDstExclusive) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());

    m_command_buffer.Begin();

    memory_range_barrier_.dstQueueFamilyIndex = queue_family_count;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-address-13088");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MemoryBarrierHost) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    if (queue_family_count < 2) {
        GTEST_SKIP() << "At least two queue families required";
    }

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    memory_range_barrier_.dstQueueFamilyIndex = queue_family_count - 1u;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-13089");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, GeometryStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-03929");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TessellationStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-03930");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ConditionalRenderingStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-03931");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, FragmentDensityProcessStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-03932");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TransformFeedbackStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-03933");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MeshShaderStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-03934");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TaskShaderStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-03935");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, FragmentShadingRateStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-07316");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, RayTracingShaderStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-07946");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, AccelerationStructureBuildStage) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-10751");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, AccelerationStructureCopyStage) {
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstStageMask-10752");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MicromapBuildStage) {
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcStageMask-10753");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, IndirectCommandReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03900");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, IndexReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_INDEX_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03901");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, VertexAttributeReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03902");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, InputAttachmentReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03903");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, UniformReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03904");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderSampledReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03905");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderStorageReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03906");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderStorageWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03907");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-07454");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03909");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ColorAttachmentReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03910");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ColorAttachmentWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03911");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, DepthStencilAttachmentReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03912");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, DepthStencilAttachmentWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03913");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TransferReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03914");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TransferWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03915");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, HostReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03916");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, HostWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03917");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ConditionalRenderingReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03918");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, FragmentDensityMapReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03919");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TransformFeedbackWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03920");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, TransformFeedbackCounterReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-04747");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ColorAttachmentReadNoncoherentAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03926");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, AccelerationStructureReadAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-03927");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, AccelerationStructureWriteAccess) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-03928");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, AccelerationStructureReadAccessRayQuery) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-06256");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderBindingTableReadAccess) {
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-07272");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, VideoDecodeReadAccess) {
    AddRequiredExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-04858");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, VideoDecodeWriteAccess) {
    AddRequiredExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-04859");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, VideoEncodeReadAccess) {
    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-04860");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, VideoEncodeWriteAccess) {
    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-04861");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, OpticalFlowReadNVAccess) {
    AddRequiredExtensions(VK_NV_OPTICAL_FLOW_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_OPTICAL_FLOW_READ_BIT_NV;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-07455");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, OpticalFlowWriteNVAccess) {
    AddRequiredExtensions(VK_NV_OPTICAL_FLOW_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_OPTICAL_FLOW_WRITE_BIT_NV;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-07456");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MicromapWriteAccess) {
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-07457");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MicromapReadAccess) {
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_MICROMAP_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-07458");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, DescriptorBufferReadAccess) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-08118");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderTileAttachmentReadQCOMAccess) {
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_READ_BIT_QCOM;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-10670");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, ShaderTileAttachmentWriteQCOMAccess) {
    AddRequiredExtensions(VK_QCOM_TILE_SHADING_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-10671");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MemoryDecompressionReadAccess) {
    AddRequiredExtensions(VK_EXT_MEMORY_DECOMPRESSION_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_MEMORY_DECOMPRESSION_READ_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcAccessMask-11771");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, MemoryDecompressionWriteAccess) {
    AddRequiredExtensions(VK_EXT_MEMORY_DECOMPRESSION_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_MEMORY_DECOMPRESSION_WRITE_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstAccessMask-11772");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, SrcExternalQueueFamily) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    uint32_t queue_families[] = {m_default_queue->family_index, m_second_queue->family_index};

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 256u;
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
    buffer_ci.queueFamilyIndexCount = 2u;
    buffer_ci.pQueueFamilyIndices = queue_families;

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    buffer_ = vkt::Buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          &allocate_flag_info);

    memory_range_barrier_ = vku::InitStructHelper();
    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    memory_range_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    memory_range_barrier_.dstQueueFamilyIndex = m_default_queue->family_index;
    memory_range_barrier_.addressRange = buffer_.AddressRange();
    memory_range_barrier_.addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR;

    memory_range_barriers_info_ = vku::InitStructHelper();
    memory_range_barriers_info_.memoryRangeBarrierCount = 1u;
    memory_range_barriers_info_.pMemoryRangeBarriers = &memory_range_barrier_;

    dependency_info_ = vku::InitStructHelper(&memory_range_barriers_info_);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-None-09097");
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, DstExternalQueueFamily) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    uint32_t queue_families[] = {m_default_queue->family_index, m_second_queue->family_index};

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 256u;
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
    buffer_ci.queueFamilyIndexCount = 2u;
    buffer_ci.pQueueFamilyIndices = queue_families;

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    buffer_ = vkt::Buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          &allocate_flag_info);

    memory_range_barrier_ = vku::InitStructHelper();
    memory_range_barrier_.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier_.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    memory_range_barrier_.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memory_range_barrier_.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    memory_range_barrier_.srcQueueFamilyIndex = m_default_queue->family_index;
    memory_range_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    memory_range_barrier_.addressRange = buffer_.AddressRange();
    memory_range_barrier_.addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR;

    memory_range_barriers_info_ = vku::InitStructHelper();
    memory_range_barriers_info_.memoryRangeBarrierCount = 1u;
    memory_range_barriers_info_.pMemoryRangeBarriers = &memory_range_barrier_;

    dependency_info_ = vku::InitStructHelper(&memory_range_barriers_info_);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-None-09098");
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, SrcForeignQueueFamily) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-srcQueueFamilyIndex-09099");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommandsSync, DstForeignQueueFamily) {
    RETURN_IF_SKIP(InitSyncDeviceAddressCommands());

    m_command_buffer.Begin();

    memory_range_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-dstQueueFamilyIndex-09100");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
