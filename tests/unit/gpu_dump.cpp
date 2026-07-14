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
#include <spirv-tools/libspirv.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <cstring>
#include "layer_validation_tests.h"
#include "descriptor_helper.h"
#include "generated/vk_function_pointers.h"
#include "pipeline_helper.h"
#include "shader_helper.h"
#include "shader_templates.h"
#include "test_framework.h"
#include "utils/math_utils.h"
#include "descriptor_heap_object.h"
#include "ray_tracing_objects.h"

static const VkLayerSettingEXT kAllDumpSettings[3] = {
    {OBJECT_LAYER_NAME, "gpu_dump_device_generated_commands", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    {OBJECT_LAYER_NAME, "gpu_dump_copy_memory_indirect", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
};
VkLayerSettingsCreateInfoEXT kAllDumpSettingCi = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 3, kAllDumpSettings};

class NegativeGpuDump : public VkLayerTest {
  public:
    void InitDescriptorBuffer(VkLayerSettingsCreateInfoEXT* custom_setting = nullptr);
    void InitDescriptorHeap(VkLayerSettingsCreateInfoEXT* custom_setting = nullptr);
    VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props = vku::InitStructHelper();
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props = vku::InitStructHelper();
};

void NegativeGpuDump::InitDescriptorBuffer(VkLayerSettingsCreateInfoEXT* custom_setting) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFramework(custom_setting ? custom_setting : &kAllDumpSettingCi));
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(descriptor_buffer_props);
}

void NegativeGpuDump::InitDescriptorHeap(VkLayerSettingsCreateInfoEXT* custom_setting) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitFramework(custom_setting ? custom_setting : &kAllDumpSettingCi));
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

TEST_F(NegativeGpuDump, DescriptorBufferNoDescriptor) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorBuffer(&layer_setting_ci));

    vkt::DescriptorSetLayout ds_layout(*m_device, {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
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
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    m_errorMonitor->SetDesiredWarning("NO DESCRIPTOR");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// closet we will get to VU 08053/08054
TEST_F(NegativeGpuDump, DescriptorBufferWrongBufferType) {
    RETURN_IF_SKIP(InitDescriptorBuffer());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},  // unused
    };
    std::vector<VkDescriptorSetLayoutBinding> sampler_binding = {
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::DescriptorSetLayout sampler_ds_layout(*m_device, sampler_binding,
                                               VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout, &sampler_ds_layout});

    vkt::Buffer sampler_descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    vkt::Buffer resource_descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                           vkt::device_address);

    const char* cs_source = R"glsl(
        #version 450
        layout(set = 1, binding = 2) uniform sampler s;
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) buffer SSBO { vec4 result; };

        void main() {
            result = texture(sampler2D(t, s), vec2(0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info[2];
    descriptor_buffer_binding_info[0] = vku::InitStructHelper();
    descriptor_buffer_binding_info[0].address = sampler_descriptor_buffer.Address();
    descriptor_buffer_binding_info[0].usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptor_buffer_binding_info[1] = vku::InitStructHelper();
    descriptor_buffer_binding_info[1].address = resource_descriptor_buffer.Address();
    descriptor_buffer_binding_info[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 2, descriptor_buffer_binding_info);

    uint32_t buffer_index[2] = {0, 1};
    VkDeviceSize buffer_offset[2] = {0, 0};
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 2, buffer_index,
                                         buffer_offset);

    m_errorMonitor->SetDesiredWarning("BUFFER USAGE");
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

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
    m_command_buffer.Begin();

    desc_heap.BindResourceHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    // CONSTANT_DATA Static array, can detect OOB
    vkt::HeapComputePipeline pipe1(*m_device, cs_source_static, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe1);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // CONSTANT_DATA runtime array, can't detect OOB
    vkt::HeapComputePipeline pipe2(*m_device, cs_source_runtime, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::HeapComputePipeline pipe3(*m_device, cs_source_static, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe3);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // PUSH_INDEX runtime array
    vkt::HeapComputePipeline pipe4(*m_device, cs_source_runtime, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe4);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapOffset = 0;
    mapping.sourceData.indirectIndex.pushOffset = 0;
    mapping.sourceData.indirectIndex.addressOffset = 4;  // start at SSBO[2]
    mapping.sourceData.indirectIndex.heapIndexStride = 1;
    mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;
    // INDIRECT_INDEX Static array
    vkt::HeapComputePipeline pipe5(*m_device, cs_source_static, SPV_ENV_VULKAN_1_0, &mapping_info);
    // INDIRECT_INDEX runtime array
    vkt::HeapComputePipeline pipe6(*m_device, cs_source_runtime, SPV_ENV_VULKAN_1_0, &mapping_info);

    if (m_device->Physical().limits_.minUniformBufferOffsetAlignment <= 4) {
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe5);
        m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();

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
    vkt::HeapComputePipeline pipe7(*m_device, cs_source_static, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe7);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    // INDIRECT_INDEX_ARRAY runtime array
    vkt::HeapComputePipeline pipe8(*m_device, cs_source_runtime, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };

        void main() {
            data = 0;
        }
    )glsl";

    m_command_buffer.Begin();

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";

    m_command_buffer.Begin();

    VkDeviceSize max_descriptor_alignement = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";

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
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";

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
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = (uint32_t)max_descriptor_alignement;
    bind_resource_info.reservedRangeSize = (uint32_t)resource_stride * 2;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);
    desc_heap.CreateSamplerHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) uniform sampler s[64];
        void main() {
	        vec4 data = texture(sampler2D(t, s[2]), vec2(0.5f));
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);

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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

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

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[4];

        void main() {
            x[0].data = 0;
        }
    )glsl";

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

    desc_heap.BindResourceHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO_0 {
            uint data;
        };
        void main() {
            uint x = data;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    uint32_t push_data_uint = 1;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data_uint);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mapping.sourceData.heapData.heapOffset = 0;
    mapping.sourceData.pushIndex.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    m_errorMonitor->SetDesiredWarning("MISALIGNED");
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

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
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };

        void main() {
            data = 0;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    vkt::Buffer ssbo_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress read_address = ssbo_buffer.Address() + 1;
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &read_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

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

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
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
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.imageDescriptorSize);
    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler2D tex;
        void main() {
            vec4 data = texture(tex, vec2(0.5f));
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset = {};
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.samplerHeapOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapZeroArrayStride) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.bufferDescriptorSize);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } ssbo[8];

        void main() {
            ssbo[0].data = 0;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("ZERO ARRAY STRIDE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapIndirectIndexBufferType) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO {
            uint data;
        };

        void main() {
            uint a = data;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    // should be a UNIFORM
    vkt::Buffer bad_indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDeviceAddress indirect_ubo_address = bad_indirect_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ubo_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapIndexStride = 0;
    mapping.sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("BUFFER TYPE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapPushAddressBufferType) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform UBO { uint a; };
        layout (set = 0, binding = 1) buffer SSBO { uint b; };

        void main() {
            b = a;
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[1].sourceData.pushAddressOffset = 8;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress indirect_ssbo_address = ssbo_buffer.Address();
    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_ssbo_address);
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &indirect_ubo_address);

    m_errorMonitor->SetDesiredWarning("BUFFER TYPE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapUntypedPointers) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);
    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        #extension GL_EXT_nonuniform_qualifier : require
        layout (set = 0, binding = 0) uniform UBO { uint a; };
        layout (set = 0, binding = 1) buffer SSBO { uint b; };
        layout(descriptor_heap) buffer Heap { uint c; } heap[];
        layout(descriptor_heap) uniform HeapX { uint d; } heapX[];
        layout(descriptor_heap) uniform texture2D heapT[];
        layout(descriptor_heap) uniform sampler heapS[];
        layout(push_constant) uniform PushConstant {
            uint pc;
        };
        void main() {
            b = a;
            heap[0].c = a;
            heap[2].c = pc;
            heap[2].c += 2;
            uint x = heapX[b].d;
            uint y = heapX[a].d;
            uint z = heapX[114].d;

            vec4 data = texture(sampler2D(heapT[0], heapS[0]), vec2(0));
            vec4 data2 = texture(sampler2D(heapT[b], heapS[1]), vec2(0));
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    uint32_t push_data = 0;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapUntypedPointersStorageImage) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    VkDeviceSize heap_size = Align((resource_stride * 4), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    const char* cs_source = R"glsl(
        #version 460 core
        #extension GL_EXT_descriptor_heap: enable
        layout(descriptor_heap, r32i) uniform iimage1D si1[];
        layout(descriptor_heap, R32ui) uniform uimage2D si2[];
        layout(set = 0, binding = 0, r32i) uniform iimage1D si3[];
        layout(set = 0, binding = 1, R32ui) uniform uimage2D si4[];

        void main() {
            imageAtomicAdd(si1[2], 1, 1);
            imageStore(si2[1], ivec2(1), uvec4(1));
            imageAtomicAdd(si3[0], 1, 1);
            imageStore(si4[1], ivec2(1), uvec4(1));
        }
    )glsl";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[0].sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    uint32_t push_data = 0;
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapBindingCount) {
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    VkDeviceSize heap_size = Align((resource_stride * 3), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 5);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; } x[2];
        layout(set = 0, binding = 2) buffer B { uint b; } y[3];
        void main() {
            x[0].a = 2;
            y[0].b = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    char const* cs_source2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer X { uint d; } x;
        layout(set = 0, binding = 1) buffer Y { uint d; } y;
        layout(set = 0, binding = 2) buffer Z { uint d; } z;
        layout(set = 0, binding = 4) buffer W { uint d; } w;
        void main() {
            x.d = y.d + z.d + w.d;
        }
    )glsl";
    vkt::HeapComputePipeline pipe2(*m_device, cs_source2, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    m_errorMonitor->SetDesiredWarning("OUT OF BOUNDS");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapUntypedPointersPushDataIndex) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(8192);

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Alignment not reliable on MockICD";
    }

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        #extension GL_EXT_nonuniform_qualifier : require
        layout(descriptor_heap) buffer Heap { uint x; } heap[];
        layout(push_constant) uniform PushConstant {
            uint pc_0;
            layout(offset = 32) uint pc_1;
            uint pc_2; // offset 36
        };
        void main() {
            heap[0].x = 0;
            heap[pc_0].x = 1;
            heap[pc_1].x = 2;
            heap[pc_0 + pc_2].x = 3;
            heap[pc_0 * pc_2 + pc_1].x = 4;
            heap[pc_0 + 1].x = 5;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    uint8_t unused[40];
    m_command_buffer.PushData(0, 40, unused);  // avoid 11376
    uint32_t pc_0 = 2;
    uint32_t pc_1 = 1;
    uint32_t pc_2 = 3;
    m_command_buffer.PushData(0, sizeof(uint32_t), &pc_0);
    m_command_buffer.PushData(32, sizeof(uint32_t), &pc_1);
    m_command_buffer.PushData(36, sizeof(uint32_t), &pc_2);
    m_errorMonitor->SetDesiredInfo("array index: [1] (from vkCmdPushDataEXT[32:35])");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, UntypedPointersMultiDimensional) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/spirv/SPIR-V/-/issues/942");
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitDescriptorHeap());

    // We want to easily control it for testing
    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    VkDeviceSize heap_size = Align((resource_stride * 8), resource_stride);
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_size);

    // layout(descriptor_heap) buffer Heap { uint data; } heap[3][3];
    // void main() {
    //     heap[1][2].data = 42;
    // }
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %Heap Block
               OpMemberDecorate %Heap 0 Offset 0
               OpDecorateId %out_array ArrayStrideIdEXT %buf_size
               OpDecorateId %in_array ArrayStrideIdEXT %stride_3
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %uint_0 = OpConstant %uint 0
      %uint_1 = OpConstant %uint 1
      %uint_2 = OpConstant %uint 2
      %uint_3 = OpConstant %uint 3
    %uint_42 = OpConstant %uint 42
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %Heap = OpTypeStruct %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
   %buf_type = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %uint %buf_type
   %stride_3 = OpSpecConstantOp %int IMul %buf_size %uint_3
 %in_array = OpTypeArray %buf_type %uint_3
%out_array = OpTypeArray %in_array %uint_3
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpUntypedAccessChainKHR %_ptr_UniformConstant %out_array %resource_heap %uint_1 %uint_2
         %19 = OpBufferPointerEXT %_ptr_StorageBuffer %15
         %20 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %Heap %19 %uint_0
               OpStore %20 %uint_42
               OpReturn
               OpFunctionEnd
    )asm";

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredInfo("array index: [1][2]");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapPushOffsetOOB) {
    RETURN_IF_SKIP(InitDescriptorHeap());
    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.bufferDescriptorSize);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 8;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    uint32_t push_index = 0;
    m_command_buffer.PushData(8, sizeof(uint32_t), &push_index);
    VkDeviceAddress indirect_data = 0;
    m_command_buffer.PushData(16, sizeof(VkDeviceAddress), &indirect_data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("PUSH DATA");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapNoDescriptor) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(1024);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 3;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("NO DESCRIPTOR");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapWrongDescriptor) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(1024);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    vkt::Buffer ssbo_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, R32ui) uniform uimage2D si;
        void main() {
            imageStore(si, ivec2(1), uvec4(0));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("WRONG DESCRIPTOR");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapWrongDescriptorSampler) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(1024);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host{desc_heap.resource_heap_data_, heap_props.samplerDescriptorSize};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, R32ui) uniform uimage2D si;
        void main() {
            imageStore(si, ivec2(1), uvec4(0));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("WRONG DESCRIPTOR");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapWrongDescriptorNotSampler) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    vkt::Buffer descriptor_heap_r(*m_device, 1024, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    vkt::Buffer descriptor_heap_s(*m_device, 1024, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_SINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);
    void* heap_data = descriptor_heap_r.Memory().Map();
    VkHostAddressRangeEXT resource_host{heap_data, heap_props.imageDescriptorSize};
    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info.data.pImage = &image_info;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &resource_host);

    heap_data = descriptor_heap_s.Memory().Map();
    resource_host.address = heap_data;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &resource_host);

    const char* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) uniform sampler s;
        void main() {
	        vec4 data = texture(sampler2D(t, s), vec2(0.5f));
        }
    )glsl";

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
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("WRONG DESCRIPTOR");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapNoDescriptorCombinedSampler) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0 || heap_props.minSamplerHeapReservedRange != 0) {
        GTEST_SKIP() << "heapReservedRange is not zero";
    }

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.imageDescriptorSize);
    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    desc_heap.WriteImageDescriptor(image);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler2D tex;
        void main() {
            vec4 data = texture(tex, vec2(0.5f));
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.samplerHeapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapWrongDescriptorDebugNames) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(1024);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.bufferDescriptorSize;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    VkDebugUtilsObjectNameInfoEXT debug_obj_info = vku::InitStructHelper();
    debug_obj_info.objectHandle = 0;
    debug_obj_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
    debug_obj_info.pObjectName = "My Sampler";

    uint8_t* heap_data = desc_heap.resource_heap_data_;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo(&debug_obj_info);
    VkHostAddressRangeEXT sampler_host{heap_data, heap_props.samplerDescriptorSize};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    debug_obj_info.pObjectName = "My UBO";
    vkt::Buffer ssbo_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkHostAddressRangeEXT descriptor_host{heap_data + heap_props.bufferDescriptorSize, heap_props.bufferDescriptorSize};
    VkDeviceAddressRangeEXT device_range = ssbo_buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper(&debug_obj_info);
    descriptor_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, R32ui) uniform uimage2D si;
        layout(set = 0, binding = 1) uniform UBO { uvec4 data;};
        void main() {
            imageStore(si, ivec2(1), data);
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredWarning("[My Sampler]");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapHashConflict) {
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }

    // storage image and sampled image might be the exact same descriptor
    uint8_t storage_descriptor[256];
    uint8_t sampled_descriptor[256];
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);
    VkHostAddressRangeEXT resource_host{storage_descriptor, heap_props.imageDescriptorSize};
    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info.data.pImage = &image_info;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &resource_host);
    resource_host.address = sampled_descriptor;
    descriptor_info.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &resource_host);

    if (memcmp(storage_descriptor, sampled_descriptor, heap_props.imageDescriptorSize) != 0) {
        GTEST_SKIP() << "UBO and SSBO are different descriptors";
    }

    vkt::Buffer resource_heap(*m_device, 1024, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    void* heap_data = resource_heap.Memory().Map();
    memcpy(heap_data, storage_descriptor, heap_props.imageDescriptorSize);

    vkt::Buffer sampler_heap(*m_device, 1024, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    heap_data = sampler_heap.Memory().Map();
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host{heap_data, heap_props.samplerDescriptorSize};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 0;
    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 2) uniform sampler s;
        layout(set = 0, binding = 0) uniform texture2D sampledImage;
        layout(set = 0, binding = 1, rgba8) uniform image2D storageImage;
        void main(){
            vec4 color = texture(sampler2D(sampledImage, s), vec2(0));
            imageStore(storageImage, ivec2(1), color);
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = resource_heap.AddressRange();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    bind_resource_info.heapRange = sampler_heap.AddressRange();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, AccelStructPushAddressValid) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitDescriptorHeap());

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);

    VkDebugUtilsObjectNameInfoEXT tlas_dbg_name = vku::InitStructHelper();
    tlas_dbg_name.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_dbg_name.pObjectName = "my_tlas";
    tlas_dbg_name.objectHandle = (uint64_t)tlas.GetDstAS()->handle();
    vk::SetDebugUtilsObjectNameEXT(device(), &tlas_dbg_name);

    VkDescriptorSetAndBindingMappingEXT mapping = vku::InitStructHelper();
    mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags2(VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen, nullptr, &mapping_info);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss, nullptr, &mapping_info);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_ARB_gpu_shader_int64 : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit, nullptr, &mapping_info);
    pipeline.Build();

    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;

    m_command_buffer.Begin();
    const VkDeviceAddress as_addr = tlas.GetDstAS()->GetAccelerationStructureDeviceAddress();
    m_command_buffer.PushData(0, sizeof(as_addr), &as_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

    m_errorMonitor->SetDesiredInfo("my_tlas");
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, AccelStructPushAddressInvalid) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitDescriptorHeap());

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);

    VkDebugUtilsObjectNameInfoEXT tlas_dbg_name = vku::InitStructHelper();
    tlas_dbg_name.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_dbg_name.pObjectName = "my_tlas";
    tlas_dbg_name.objectHandle = (uint64_t)tlas.GetDstAS()->handle();
    vk::SetDebugUtilsObjectNameEXT(device(), &tlas_dbg_name);

    VkDescriptorSetAndBindingMappingEXT mapping = vku::InitStructHelper();
    mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags2(VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen, nullptr, &mapping_info);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss, nullptr, &mapping_info);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit, nullptr, &mapping_info);
    pipeline.Build();

    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;

    m_command_buffer.Begin();
    // Invalidate AS address
    const VkDeviceAddress as_addr = tlas.GetDstAS()->GetAccelerationStructureDeviceAddress() + 256;
    m_command_buffer.PushData(0, sizeof(as_addr), &as_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

    m_errorMonitor->SetDesiredWarning("No VkAccelerationStructureKHR found");
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, AccelStructHeapIndirectAddressValid) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitDescriptorHeap());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (TLAS address will end up being garbage)";
    }

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);

    VkDebugUtilsObjectNameInfoEXT tlas_dbg_name = vku::InitStructHelper();
    tlas_dbg_name.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_dbg_name.pObjectName = "my_tlas";
    tlas_dbg_name.objectHandle = (uint64_t)tlas.GetDstAS()->handle();
    vk::SetDebugUtilsObjectNameEXT(device(), &tlas_dbg_name);

    VkDescriptorSetAndBindingMappingEXT mapping = vku::InitStructHelper();
    mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.indirectAddress.pushOffset = 0;
    mapping.sourceData.indirectAddress.addressOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags2(VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen, nullptr, &mapping_info);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss, nullptr, &mapping_info);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_ARB_gpu_shader_int64 : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit, nullptr, &mapping_info);
    pipeline.Build();

    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;

    const VkDeviceAddress as_addr = tlas.GetDstAS()->GetAccelerationStructureDeviceAddress();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDeviceAddress),
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT, vkt::device_address);
    auto indirect_buffer_ptr = (VkDeviceAddress*)indirect_buffer.Memory().Map();
    *indirect_buffer_ptr = as_addr;
    indirect_buffer.Memory().Unmap();
    const VkDeviceAddress indirect_buffer_addr = indirect_buffer.Address();

    m_command_buffer.Begin();

    m_command_buffer.PushData(0, sizeof(indirect_buffer_addr), &indirect_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

    m_errorMonitor->SetDesiredInfo("my_tlas");
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, AccelStructHeapIndirectAddressInvalid) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitDescriptorHeap());

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);

    VkDebugUtilsObjectNameInfoEXT tlas_dbg_name = vku::InitStructHelper();
    tlas_dbg_name.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_dbg_name.pObjectName = "my_tlas";
    tlas_dbg_name.objectHandle = (uint64_t)tlas.GetDstAS()->handle();
    vk::SetDebugUtilsObjectNameEXT(device(), &tlas_dbg_name);

    VkDescriptorSetAndBindingMappingEXT mapping = vku::InitStructHelper();
    mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.indirectAddress.pushOffset = 0;
    mapping.sourceData.indirectAddress.addressOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags2(VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen, nullptr, &mapping_info);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss, nullptr, &mapping_info);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_ARB_gpu_shader_int64 : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit, nullptr, &mapping_info);
    pipeline.Build();

    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;

    const VkDeviceAddress as_addr = tlas.GetDstAS()->GetAccelerationStructureDeviceAddress();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDeviceAddress),
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT, vkt::device_address);
    auto indirect_buffer_ptr = (VkDeviceAddress*)indirect_buffer.Memory().Map();
    // Invalid AS addr
    *indirect_buffer_ptr = as_addr + 256;
    indirect_buffer.Memory().Unmap();
    const VkDeviceAddress indirect_buffer_addr = indirect_buffer.Address();

    m_command_buffer.Begin();

    m_command_buffer.PushData(0, sizeof(indirect_buffer_addr), &indirect_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

    m_errorMonitor->SetDesiredWarning("No VkAccelerationStructureKHR found");
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapNullDescriptor) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    static const VkLayerSettingEXT layer_settings[2] = {
        {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};
    RETURN_IF_SKIP(InitDescriptorHeap(&layer_setting_ci));

    if (heap_props.minResourceHeapReservedRange != 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is not zero";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(1024);
    desc_heap.WriteNullDescriptorAtOffset(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO { uint x; };
        void main() {
            x = 0;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredInfo("Null Descriptor");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DescriptorHeapUntypedPointersImageFunctionParam) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.imageDescriptorSize);

    // TODO - vkt::DescriptorHeap needs to be fixed to make sure the heap memory is allocated to
    // imageDescriptorAlignment
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "issues with alignment";
    }

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 2);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"(
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main" %3 %4 %5
               OpExecutionMode %2 LocalSize 1 1 1
               OpDecorate %3 BuiltIn ResourceHeapEXT
               OpDecorateId %6 ArrayStrideIdEXT %7
               OpDecorate %4 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %8 Block
               OpMemberDecorate %8 0 Offset 0
               OpDecorate %5 Binding 1
               OpDecorate %5 DescriptorSet 0
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 0
         %12 = OpTypeImage %11 Buffer 0 0 0 1 Unknown
         %13 = OpTypePointer UniformConstant %12
         %14 = OpTypeUntypedPointerKHR UniformConstant
         %15 = OpTypeFunction %11 %14
         %16 = OpConstant %11 0
         %17 = OpTypeVector %11 4
         %18 = OpTypeVector %11 2
          %3 = OpUntypedVariableKHR %14 UniformConstant
          %7 = OpConstantSizeOfEXT %11 %12
          %6 = OpTypeRuntimeArray %12
         %19 = OpTypePointer Function %11
          %4 = OpVariable %13 UniformConstant
         %20 = OpConstant %11 1
          %8 = OpTypeStruct %18
         %21 = OpTypePointer StorageBuffer %8
          %5 = OpVariable %21 StorageBuffer
         %22 = OpTypePointer StorageBuffer %18
         %23 = OpTypeVector %11 3
         %24 = OpConstantComposite %23 %20 %20 %20
         %25 = OpFunction %11 None %15
         %26 = OpFunctionParameter %14
         %27 = OpLabel
         %28 = OpLoad %12 %26
         %29 = OpImageFetch %17 %28 %16 ZeroExtend
         %30 = OpCompositeExtract %11 %29 0
               OpReturnValue %30
               OpFunctionEnd
          %2 = OpFunction %9 None %10
         %31 = OpLabel
         %32 = OpUntypedAccessChainKHR %14 %6 %3 %16
         %33 = OpUntypedAccessChainKHR %14 %12 %4
         %34 = OpFunctionCall %11 %25 %32
         %35 = OpFunctionCall %11 %25 %33
         %36 = OpCompositeConstruct %18 %34 %35
         %37 = OpAccessChain %22 %5 %16
               OpStore %37 %36
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredInfo("(is a function argument)");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}