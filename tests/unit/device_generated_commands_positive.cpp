/*
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <vulkan/vulkan_core.h>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "shader_object_helper.h"
#include "generated/vk_function_pointers.h"

void DeviceGeneratedCommandsTest::InitBasicDeviceGeneratedCommands() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    } else if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_VERTEX_BIT is not supported.";
    }
}

void DeviceGeneratedCommandsTest::SetPreProcessBuffer(VkGeneratedCommandsInfoEXT& generated_commands_info,
                                                      void* pipeline_or_shader_object) {
    pre_process_buffer_.Init(*m_device, generated_commands_info, pipeline_or_shader_object);
}

class PositiveDeviceGeneratedCommands : public DeviceGeneratedCommandsTest {};

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectExecutionSetPipeline) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe, 1);
}

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectExecutionSetShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
}

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectCommandsLayout) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    vkt::IndirectCommandsToken tokens;
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST_F(PositiveDeviceGeneratedCommands, GetGeneratedCommandsMemoryRequirements) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::IndirectCommandsToken tokens;
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();
    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe;

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper(&pipeline_info);
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = VK_NULL_HANDLE;
    req_info.indirectCommandsLayout = command_layout;

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
}

TEST_F(PositiveDeviceGeneratedCommands, PushConstant) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 4, 12}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);

    vkt::IndirectCommandsToken tokens;
    tokens.PushConstant({VK_SHADER_STAGE_VERTEX_BIT, 8, 8});
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, pipeline_layout);
}

TEST_F(PositiveDeviceGeneratedCommands, PushData) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 4, 12}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);

    vkt::IndirectCommandsToken tokens;
    tokens.PushData({VK_SHADER_STAGE_ALL, 8, 8});
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, pipeline_layout);
}

TEST_F(PositiveDeviceGeneratedCommands, CmdExecuteGeneratedCommandsGraphics) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe, 1);

    vkt::PipelineLayout layout = vkt::PipelineLayout(*m_device);

    vkt::IndirectCommandsToken tokens;
    tokens.ExecutionSetPipeline(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, layout);

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = exe_set;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, PreprocessUsesStateCommandBufferPipeline) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::PipelineLayout layout(*m_device);
    vkt::IndirectCommandsToken tokens;
    tokens.VertexBuffer(0);
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, layout,
                                               VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT);

    CreatePipelineHelper dgc_pipe(*this);
    dgc_pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    dgc_pipe.CreateGraphicsPipeline();

    CreatePipelineHelper other_pipe(*this);
    other_pipe.CreateGraphicsPipeline();

    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = dgc_pipe;

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&pipeline_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info, &pipeline_info);

    vkt::CommandBuffer state_cb(*m_device, m_command_pool);
    state_cb.Begin();
    vk::CmdBindPipeline(state_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, dgc_pipe);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, other_pipe);
    vk::CmdPreprocessGeneratedCommandsEXT(m_command_buffer, &generated_commands_info, state_cb);
    m_command_buffer.End();
    state_cb.End();
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetPipeline) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe, 4);

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets[3];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 1;
    write_exe_sets[0].pipeline = pipe;
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 2;
    write_exe_sets[1].pipeline = pipe;
    write_exe_sets[2] = vku::InitStructHelper();
    write_exe_sets[2].index = 3;
    write_exe_sets[2].pipeline = pipe;
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set, 3, write_exe_sets);
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetPipelineDestory) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12052");
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set;
    {
        VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
        exe_set_pipeline_info.initialPipeline = pipe;
        exe_set_pipeline_info.maxPipelineCount = 2;

        VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
        exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
        exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;
        exe_set.Init(*m_device, exe_set_ci);

        exe_set_ci.info.pPipelineInfo = nullptr;
    }

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 1;
    write_exe_sets.pipeline = pipe;
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set, 1, &write_exe_sets);
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetShader) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);

    VkWriteIndirectExecutionSetShaderEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 0;
    write_exe_set.shader = vertShader;
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set, 1, &write_exe_set);
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetShaderDestroye) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12052");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    vkt::IndirectExecutionSet exe_set;
    {
        VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
        exe_set_layouts.setLayoutCount = 1;
        exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

        VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
        exe_set_shader_info.shaderCount = 1;
        exe_set_shader_info.pInitialShaders = shaders;
        exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
        exe_set_shader_info.maxShaderCount = 1;
        exe_set_shader_info.pushConstantRangeCount = 0;

        VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
        exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
        exe_set_ci.info.pShaderInfo = &exe_set_shader_info;
        exe_set.Init(*m_device, exe_set_ci);

        // Null out everything once we create a IES
        // (unlike most unions, the values here are not required to be alive later)
        exe_set_ci.info.pShaderInfo = nullptr;
        exe_set_layouts.pSetLayouts = nullptr;
        exe_set_shader_info.pInitialShaders = nullptr;
        exe_set_shader_info.pSetLayoutInfos = nullptr;
    }

    VkWriteIndirectExecutionSetShaderEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 0;
    write_exe_set.shader = vertShader;
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set, 1, &write_exe_set);
}

TEST_F(PositiveDeviceGeneratedCommands, CmdExecuteGeneratedCommandsCompute) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    vkt::PipelineLayout layout = vkt::PipelineLayout(*m_device);
    vkt::IndirectCommandsToken tokens;
    tokens.ExecutionSetPipeline(VK_SHADER_STAGE_COMPUTE_BIT);
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_COMPUTE_BIT, layout);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateComputePipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe, 1);

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = exe_set;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, ExecuteShaderObjectVertex) {
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::PipelineLayout layout = vkt::PipelineLayout(*m_device);
    vkt::IndirectCommandsToken tokens;
    tokens.ExecutionSetShaderObject(VK_SHADER_STAGE_VERTEX_BIT);
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, layout);

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = exe_set;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT};
    vk::CmdBindShadersEXT(m_command_buffer, 1u, stages, shaders);
    SetDefaultDynamicStatesAllEXT(m_command_buffer);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, ExecuteShaderObjectMesh) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::IndirectCommandsToken tokens;
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_MESH_BIT_EXT);

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT mesh_create_info = ShaderCreateInfoFlagEXT(
        spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT | VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);
    const vkt::ShaderEXT mesh_shader(*m_device, mesh_create_info);
    const VkShaderEXT shaders[] = {mesh_shader};

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkGeneratedCommandsShaderInfoEXT init_shader_info = vku::InitStructHelper();
    init_shader_info.shaderCount = 1;
    init_shader_info.pShaders = shaders;

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&init_shader_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_MESH_BIT_EXT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info, &init_shader_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_MESH_BIT_EXT};
    vk::CmdBindShadersEXT(m_command_buffer, 1u, stages, shaders);
    const VkShaderEXT null_shader = VK_NULL_HANDLE;
    VkShaderStageFlagBits null_stages = VK_SHADER_STAGE_VERTEX_BIT;
    vk::CmdBindShadersEXT(m_command_buffer, 1u, &null_stages, &null_shader);
    null_stages = VK_SHADER_STAGE_FRAGMENT_BIT;
    vk::CmdBindShadersEXT(m_command_buffer, 1u, &null_stages, &null_shader);
    null_stages = VK_SHADER_STAGE_TASK_BIT_EXT;
    vk::CmdBindShadersEXT(m_command_buffer, 1u, &null_stages, &null_shader);
    SetDefaultDynamicStatesAllEXT(m_command_buffer);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, ExecutePipelineMesh) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::IndirectCommandsToken tokens;
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderObj ms(*m_device, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe;
    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&pipeline_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info, &pipeline_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, IndirectExecutionSetNullLayout) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const char comp_src[] = R"glsl(
        #version 460
        layout (local_size_x=64, local_size_y=1, local_size_z=1) in;
        layout (set=0, binding=0, std430) buffer OutputBlock { uint values[]; } outputBuffer;
        layout (set=0, binding=1, std430) readonly buffer InputBlock { uint values[]; } inputBuffer;
        layout (push_constant, std430) uniform PushConstantBlock { uint bufferOffset; } pc;
        void main(void) {
            const uint idx = gl_LocalInvocationIndex + pc.bufferOffset;
            outputBuffer.values[idx] = inputBuffer.values[idx];
        }
    )glsl";

    std::vector<uint32_t> spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src);

    vkt::Buffer buffer1(*m_device, 512, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer buffer2(*m_device, 512, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    vkt::DescriptorSetLayout descriptor_set_layout(
        *m_device, {
                       {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                       {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                   });

    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});

    VkPushConstantRange push_constant_range{VK_SHADER_STAGE_COMPUTE_BIT, 0, 4};
    VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
    shader_ci.flags = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT;
    shader_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    shader_ci.codeSize = spv.size() * sizeof(uint32_t);
    shader_ci.pCode = spv.data();
    shader_ci.pName = "main";
    shader_ci.setLayoutCount = 1u;
    shader_ci.pSetLayouts = &descriptor_set_layout.handle();
    shader_ci.pushConstantRangeCount = 1u;
    shader_ci.pPushConstantRanges = &push_constant_range;
    vkt::ShaderEXT shader(*m_device, shader_ci);

    VkIndirectExecutionSetShaderInfoEXT shader_info = vku::InitStructHelper();
    shader_info.shaderCount = 1u;
    shader_info.pInitialShaders = &shader.handle();
    shader_info.pSetLayoutInfos = nullptr;
    shader_info.maxShaderCount = 2u;
    shader_info.pushConstantRangeCount = 1u;
    shader_info.pPushConstantRanges = &push_constant_range;

    VkIndirectExecutionSetCreateInfoEXT indirect_execution_set_ci = vku::InitStructHelper();
    indirect_execution_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
    indirect_execution_set_ci.info.pShaderInfo = &shader_info;

    VkIndirectExecutionSetEXT indirect_execution_set;
    vk::CreateIndirectExecutionSetEXT(*m_device, &indirect_execution_set_ci, nullptr, &indirect_execution_set);
    vk::DestroyIndirectExecutionSetEXT(*m_device, indirect_execution_set, nullptr);
}

TEST_F(PositiveDeviceGeneratedCommands, PushConstantMultipleRanges) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    std::vector<VkPushConstantRange> pc_ranges = {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 16},
        {VK_SHADER_STAGE_FRAGMENT_BIT, 16, 16},
    };
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_ranges);

    vkt::IndirectCommandsToken tokens;
    tokens.PushConstant(pc_ranges[0]);
    tokens.PushConstant(pc_ranges[1]);
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT, pipeline_layout);
}

TEST_F(PositiveDeviceGeneratedCommands, IndirectCommandsLayoutDescriptorHeap) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    vkt::IndirectCommandsToken tokens;
    tokens.PushData({VK_SHADER_STAGE_ALL, 4, 8});
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST_F(PositiveDeviceGeneratedCommands, ShaderObjectQuery) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12284.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT vert_shader(*m_device, vert_create_info);

    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT frag_create_info =
        ShaderCreateInfoFlagEXT(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::ShaderEXT frag_shader(*m_device, frag_create_info);

    const VkShaderStageFlagBits stages[7] = {VK_SHADER_STAGE_VERTEX_BIT,
                                             VK_SHADER_STAGE_FRAGMENT_BIT,
                                             VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                             VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                             VK_SHADER_STAGE_GEOMETRY_BIT,
                                             VK_SHADER_STAGE_MESH_BIT_EXT,
                                             VK_SHADER_STAGE_TASK_BIT_EXT};
    const VkShaderEXT shaders[7] = {vert_shader.handle(), frag_shader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE,
                                    VK_NULL_HANDLE,       VK_NULL_HANDLE,       VK_NULL_HANDLE};

    vkt::PipelineLayout layout = vkt::PipelineLayout(*m_device);

    vkt::IndirectCommandsToken tokens;
    tokens.Action(VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT);
    vkt::IndirectCommandsLayout command_layout(*m_device, tokens, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                               layout);

    vkt::Buffer vertex_buffer(*m_device, 4096, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer indirect_buffer(*m_device, 32, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    query_pool_ci.queryCount = 1;
    query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkGeneratedCommandsShaderInfoEXT shader_info = vku::InitStructHelper();
    shader_info.shaderCount = 2;
    shader_info.pShaders = shaders;

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&shader_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout;
    generated_commands_info.indirectAddressSize = 32;
    generated_commands_info.indirectAddress = indirect_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 2;
    generated_commands_info.maxDrawCount = 0;

    SetPreProcessBuffer(generated_commands_info, &shader_info);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer, query_pool, 0, 1);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &offset);

    vk::CmdBindShadersEXT(m_command_buffer, 7u, stages, shaders);

    SetDefaultDynamicStatesAllEXT(m_command_buffer);
    VkVertexInputBindingDescription2EXT binding_desc = vku::InitStructHelper();
    binding_desc.binding = 0;
    binding_desc.stride = 16;
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_desc.divisor = 1;

    VkVertexInputAttributeDescription2EXT attr_desc = vku::InitStructHelper();
    attr_desc.location = 0;
    attr_desc.binding = 0;
    attr_desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attr_desc.offset = 0;

    vk::CmdSetVertexInputEXT(m_command_buffer, 1, &binding_desc, 1, &attr_desc);

    vk::CmdBeginQuery(m_command_buffer, query_pool, 0, 0);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer, false, &generated_commands_info);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, IndirectExecutionSetNullSetLayout) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_11_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::maintenance11);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlagEXT(vert_spv, VK_SHADER_STAGE_VERTEX_BIT,
                                VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT | VK_SHADER_CREATE_INDEPENDENT_SETS_BIT_KHR);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkDescriptorSetLayout shader_set_layouts[2] = {VK_NULL_HANDLE, descriptor_set.layout_.handle()};
    vert_create_info.setLayoutCount = 2;
    vert_create_info.pSetLayouts = shader_set_layouts;
    const vkt::ShaderEXT vert_shader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vert_shader};

    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 2;
    exe_set_layouts.pSetLayouts = shader_set_layouts;

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
}

TEST_F(PositiveDeviceGeneratedCommands, IndirectExecutionSetGPL) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo create_flags = vku::InitStructHelper();
    create_flags.flags = VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo(&create_flags);
    frag_out_lib.gp_ci_.layout = VK_NULL_HANDLE;
    frag_out_lib.CreateGraphicsPipeline(false);

    vkt::IndirectExecutionSet exe_set(*m_device, frag_out_lib, 1);
}