/*
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
// stype-check off
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "descriptor_helper.h"
#include "generated/vk_function_pointers.h"
#include "pipeline_helper.h"
#include "shader_templates.h"
#include "utils/math_utils.h"

static const VkLayerSettingEXT kAllDumpSettings[3] = {
    {OBJECT_LAYER_NAME, "gpu_dump_device_generated_commands", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    {OBJECT_LAYER_NAME, "gpu_dump_copy_memory_indirect", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
};
VkLayerSettingsCreateInfoEXT kAllDumpSettingCi = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 3, kAllDumpSettings};

class NegativeGpuDump : public VkLayerTest {
  public:
    void InitDescriptorBuffer();
    void InitDescriptorHeap();
    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props = vku::InitStructHelper();
};

void NegativeGpuDump::InitDescriptorBuffer() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFramework(&kAllDumpSettingCi));
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(descriptor_buffer_props);
}

void NegativeGpuDump::InitDescriptorHeap() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitFramework(&kAllDumpSettingCi));
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(heap_props);
}

TEST_F(NegativeGpuDump, Descriptors) {
    RETURN_IF_SKIP(InitDescriptorBuffer());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::Buffer buffer_data(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);

    void* mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_props.storageBufferDescriptorSize, mapped_descriptor_data);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };
        void main() {
            a = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorBufferWithoutDescriptor) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12250");
    RETURN_IF_SKIP(InitDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer descriptor_buffer(*m_device, 256, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    const char* vs_source = R"glsl(
        #version 450
        layout(push_constant) uniform PushConstants {
            vec4 x;
        } pc;

        void main() {
            gl_Position = pc.x;
        }
    )glsl";
    VkShaderObj vs(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 16};
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.pushConstantRangeCount = 1;
    pipe_layout_ci.pPushConstantRanges = &pc_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    float data[4] = {1.0, 2.0, 3.0, 4.0};
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 16, &data);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    m_errorMonitor->SetDesiredInfo("No VkPipelineLayout found");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorBufferWrongBindPoint) {
    RETURN_IF_SKIP(InitDescriptorBuffer());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer descriptor_buffer(*m_device, 256, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };
        void main() {
            a = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    // from core checks, which the user might not see
    m_errorMonitor->SetAllowedFailureMsg("WARNING-Missing-vkCmdSetDescriptorBufferOffsets");
    m_errorMonitor->SetDesiredWarning("vkCmdSetDescriptorBufferOffsetsEXT was called with VK_PIPELINE_BIND_POINT_GRAPHICS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, CopyMemoryIndirect) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFramework(&kAllDumpSettingCi));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer src_buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    const VkDeviceAddress src_address = src_buffer.Address();
    const VkDeviceAddress dst_address = dst_buffer.Address();
    VkCopyMemoryIndirectCommandKHR cmds[6] = {{src_address + 0, dst_address, 4},        {src_address + 32, dst_address + 32, 32},
                                              {src_address + 0, dst_address + 64, 16},  {src_address + 64, dst_address + 128, 64},
                                              {src_address + 0, dst_address + 256, 16}, {src_address + 128, dst_address + 512, 4}};
    const VkDeviceSize indirect_buffer_size = sizeof(cmds);

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    void* indirect_buffer_data = indirect_buffer.Memory().Map();
    memcpy(indirect_buffer_data, cmds, indirect_buffer_size);

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = indirect_buffer_size;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 6;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}

// TODO - Add and test the feature
TEST_F(NegativeGpuDump, DISABLED_DeviceCopy) {
    const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_copy_memory_indirect", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "gpu_dump_device_copy", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFramework(&layer_setting_ci));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer src_buffer(*m_device, 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 64, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    const VkDeviceAddress src_address = src_buffer.Address();
    const VkDeviceAddress dst_address = dst_buffer.Address();
    VkCopyMemoryIndirectCommandKHR cmds = {src_address, dst_address, 16};
    const VkDeviceSize indirect_buffer_size = sizeof(VkCopyMemoryIndirectCommandKHR);

    vkt::Buffer indirect_buffer_stage(*m_device, 128, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      vkt::device_address);
    void* indirect_buffer_data = indirect_buffer_stage.Memory().Map();
    memcpy(indirect_buffer_data, &cmds, indirect_buffer_size);
    VkMappedMemoryRange flush_ranges = vku::InitStructHelper();
    flush_ranges.memory = indirect_buffer_stage.Memory();
    flush_ranges.offset = 0;
    flush_ranges.size = VK_WHOLE_SIZE;
    vk::FlushMappedMemoryRanges(*m_device, 1, &flush_ranges);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer indirect_buffer_device(
        *m_device, 128,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    m_command_buffer.Begin();
    VkBufferCopy copy_region{0, 0, 128};
    vk::CmdCopyBuffer(m_command_buffer, indirect_buffer_stage, indirect_buffer_device, 1, &copy_region);
    m_command_buffer.FullMemoryBarrier();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer_device.Address();
    address_range.size = indirect_buffer_size;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 1;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    m_command_buffer.FullMemoryBarrier();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapDescriptorIndexing) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Alignment not reliable on MockICD";
    }

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);

    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* ubo_data = (uint32_t*)ubo_buffer.Memory().Map();
    memset((void*)ubo_data, 0, 64);
    ubo_data[0] = (uint32_t)resource_stride;
    ubo_data[1] = (uint32_t)resource_stride * 2;
    ubo_data[3] = (uint32_t)resource_stride * 8;
    ubo_data[5] = (uint32_t)resource_stride * 8;

    const char* cs_source_static = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } ssbo[8];

        void main() {
            ssbo[0].data = 0;
        }
    )glsl";
    const char* cs_source_runtime = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint index;
            uint data;
        } ssbo[];

        void main() {
            ssbo[ssbo[0].index].data = 0;
        }
    )glsl";
    VkShaderObj cs_module_static = VkShaderObj(*m_device, cs_source_static, VK_SHADER_STAGE_COMPUTE_BIT);
    VkShaderObj cs_module_runtime = VkShaderObj(*m_device, cs_source_runtime, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = 0;
    bind_resource_info.reservedRangeSize = 0;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    // CONSTANT_DATA Static array, can detect OOB
    CreateComputePipelineHelper pipe1(*this, &pipeline_create_flags_2_create_info);
    pipe1.cp_ci_.stage = cs_module_static.GetStageCreateInfo(&mapping_info);
    pipe1.cp_ci_.layout = VK_NULL_HANDLE;
    pipe1.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe1);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // CONSTANT_DATA runtime array, can't detect OOB
    CreateComputePipelineHelper pipe2(*this, &pipeline_create_flags_2_create_info);
    pipe2.cp_ci_.stage = cs_module_runtime.GetStageCreateInfo(&mapping_info);
    pipe2.cp_ci_.layout = VK_NULL_HANDLE;
    pipe2.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    uint32_t push_data_uint = 1;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data_uint);

    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex.heapOffset = 0;
    mapping.sourceData.pushIndex.pushOffset = 0;
    mapping.sourceData.pushIndex.heapIndexStride = (uint32_t)resource_stride;  // start at SSBO[1]
    mapping.sourceData.pushIndex.heapArrayStride = (uint32_t)resource_stride;

    // PUSH_INDEX Static array
    CreateComputePipelineHelper pipe3(*this, &pipeline_create_flags_2_create_info);
    pipe3.cp_ci_.stage = cs_module_static.GetStageCreateInfo(&mapping_info);
    pipe3.cp_ci_.layout = VK_NULL_HANDLE;
    pipe3.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe3);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // PUSH_INDEX runtime array
    CreateComputePipelineHelper pipe4(*this, &pipeline_create_flags_2_create_info);
    pipe4.cp_ci_.stage = cs_module_runtime.GetStageCreateInfo(&mapping_info);
    pipe4.cp_ci_.layout = VK_NULL_HANDLE;
    pipe4.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe4);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    if (m_device->Physical().limits_.minUniformBufferOffsetAlignment <= 4) {
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
        mapping.sourceData.indirectIndex.heapOffset = 0;
        mapping.sourceData.indirectIndex.pushOffset = 0;
        mapping.sourceData.indirectIndex.addressOffset = 4;  // start at SSBO[2]
        mapping.sourceData.indirectIndex.heapIndexStride = 1;
        mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;

        // INDIRECT_INDEX Static array
        CreateComputePipelineHelper pipe5(*this, &pipeline_create_flags_2_create_info);
        pipe5.cp_ci_.stage = cs_module_static.GetStageCreateInfo(&mapping_info);
        pipe5.cp_ci_.layout = VK_NULL_HANDLE;
        pipe5.CreateComputePipeline(false);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe5);
        m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();

        // INDIRECT_INDEX runtime array
        CreateComputePipelineHelper pipe6(*this, &pipeline_create_flags_2_create_info);
        pipe6.cp_ci_.stage = cs_module_runtime.GetStageCreateInfo(&mapping_info);
        pipe6.cp_ci_.layout = VK_NULL_HANDLE;
        pipe6.CreateComputePipeline(false);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe6);
        m_errorMonitor->SetDesiredInfo("GPU-DUMP");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();
    }

    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapOffset = 0;
    mapping.sourceData.indirectIndexArray.pushOffset = 0;
    mapping.sourceData.indirectIndexArray.addressOffset = 0;
    mapping.sourceData.indirectIndexArray.heapIndexStride = 1;
    mapping.sourceData.indirectIndexArray.pEmbeddedSampler = nullptr;

    // INDIRECT_INDEX_ARRAY Static array
    CreateComputePipelineHelper pipe7(*this, &pipeline_create_flags_2_create_info);
    pipe7.cp_ci_.stage = cs_module_static.GetStageCreateInfo(&mapping_info);
    pipe7.cp_ci_.layout = VK_NULL_HANDLE;
    pipe7.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe7);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // INDIRECT_INDEX_ARRAY runtime array
    CreateComputePipelineHelper pipe8(*this, &pipeline_create_flags_2_create_info);
    pipe8.cp_ci_.stage = cs_module_runtime.GetStageCreateInfo(&mapping_info);
    pipe8.cp_ci_.layout = VK_NULL_HANDLE;
    pipe8.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe8);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapReservedRangeNonArray) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };

        void main() {
            data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = 0;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("RESERVED RANGE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapReservedRangeArray) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkDeviceSize max_descriptor_alignement = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = (uint32_t)max_descriptor_alignement;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride * 2;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("RESERVED RANGE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapReservedRangeArrayIndexed) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* ubo_data = (uint32_t*)ubo_buffer.Memory().Map();
    memset((void*)ubo_data, 0, 64);
    ubo_data[0] = 0;
    ubo_data[1] = (uint32_t)resource_stride;
    ubo_data[2] = (uint32_t)resource_stride * 2;
    ubo_data[3] = (uint32_t)resource_stride * 3;

    VkDeviceSize max_descriptor_alignement = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = (uint32_t)max_descriptor_alignement;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride * 2;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapIndexStride = 1;
    mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("RESERVED RANGE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapReservedRangeIndirectArray) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* ubo_data = (uint32_t*)ubo_buffer.Memory().Map();
    memset((void*)ubo_data, 0, 64);
    ubo_data[0] = (uint32_t)resource_stride * 2;
    ubo_data[1] = (uint32_t)resource_stride * 3;
    ubo_data[2] = 0;
    ubo_data[3] = (uint32_t)resource_stride;

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkDeviceSize max_descriptor_alignement = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = (uint32_t)max_descriptor_alignement;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride * 2;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    // CONSTANT_DATA Static array, can detect OOB
    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("RESERVED RANGE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapSampler) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.samplerDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 2), resource_stride);
    vkt::Buffer descriptor_heap_r(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    vkt::Buffer descriptor_heap_s(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) uniform sampler s[64];
        void main() {
	        vec4 data = texture(sampler2D(t, s[2]), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap_r.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    bind_resource_info.heapRange = descriptor_heap_s.AddressRange();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 0;
    mappings[1].sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignment) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };
        layout (set = 0, binding = 1) buffer SSBO_1 {
            uint y;
        } x[3];

        void main() {
            data = x[0].y;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    uint32_t push_data_uint = 1;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data_uint);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 2);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex.heapOffset = 0;
    mapping.sourceData.pushIndex.pushOffset = 0;
    mapping.sourceData.pushIndex.heapIndexStride = 1;
    mapping.sourceData.pushIndex.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("MISALIGNED");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignmentIndirectArray) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 5), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* ubo_data = (uint32_t*)ubo_buffer.Memory().Map();
    memset((void*)ubo_data, 0, 64);
    ubo_data[0] = (uint32_t)resource_stride * 2;
    ubo_data[1] = (uint32_t)resource_stride + 1;
    ubo_data[2] = 0;
    ubo_data[3] = (uint32_t)(resource_stride * 3) + 1;

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("MISALIGNED");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignmentHeapData) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO_0 {
            uint data;
        };
        void main() {
            uint x = data;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    uint32_t push_data_uint = 1;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data_uint);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mapping.sourceData.heapData.heapOffset = 0;
    mapping.sourceData.pushIndex.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    //
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapWithoutDescriptor) {
    RETURN_IF_SKIP(InitDescriptorHeap());
    InitRenderTarget();

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vs_source = R"glsl(
        #version 450
        layout(push_constant) uniform PushConstants {
            vec4 x;
        } pc;

        void main() {
            gl_Position = pc.x;
        }
    )glsl";
    VkShaderObj vs(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vs.GetStageCreateInfo();
    stages[1] = fs.GetStageCreateInfo();

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = 2u;
    pipe.gp_ci_.pStages = stages;
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline(false);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    float data[4] = {1.0, 2.0, 3.0, 4.0};
    m_command_buffer.PushData(0, 16, data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = 0;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignmentPushAddress) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (m_device->Physical().limits_.minStorageBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "minStorageBufferOffsetAlignment is 1";
    }

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };

        void main() {
            data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    vkt::Buffer ssbo_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress read_address = ssbo_buffer.Address() + 1;
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &read_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("MISALIGNED");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignmentIndirectAddress) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (m_device->Physical().limits_.minUniformBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "minUniformBufferOffsetAlignment is 1";
    }

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    vkt::Buffer indirect_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    *((VkDeviceAddress*)indirect_buffer.Memory().Map()) = ubo_buffer.Address() + 1;

    VkDeviceAddress indirect_address = indirect_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.indirectAddress.addressOffset = 0;
    mapping.sourceData.indirectAddress.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("MISALIGNED");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapAlignmentIndirectIndex) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (m_device->Physical().limits_.minUniformBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "minUniformBufferOffsetAlignment is 1";
    }

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address() + 1;
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapIndexStride = 0;
    mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("MISALIGNED");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapIndirectIndexNoBuffer) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::Buffer descriptor_heap(*m_device, heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDeviceAddress indirect_ubo_address = 0xBEEE0000;
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapIndexStride = 0;
    mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;
    mapping.sourceData.indirectIndex.addressOffset = 0x10000;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("[WARNING] No VkBuffer found at 0xbeef0000");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapCombinedImageSampler) {
    RETURN_IF_SKIP(InitDescriptorHeap());
    InitRenderTarget();

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange/minSamplerHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    vkt::Buffer resource_heap(*m_device, resource_stride, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    vkt::Buffer sampler_heap(*m_device, sampler_stride, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler2D tex;
        void main() {
            vec4 data = texture(tex, vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset = {};
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.samplerHeapOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = resource_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    bind_resource_info.heapRange = sampler_heap.AddressRange();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapZeroArrayStride) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    vkt::Buffer descriptor_heap(*m_device, resource_stride, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } ssbo[8];

        void main() {
            ssbo[0].data = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap.AddressRange();
    bind_resource_info.reservedRangeOffset = 0;
    bind_resource_info.reservedRangeSize = 0;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.CreateComputePipeline(false);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("ZERO ARRAY STRIDE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}