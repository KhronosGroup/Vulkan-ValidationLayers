/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "descriptor_helper.h"
#include "pipeline_helper.h"
#include "shader_helper.h"

class UndefinedCore : public VkLayerTest {};

TEST_F(UndefinedCore, BindInvalidPipelineLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6621");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    const char *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=1) uniform foo1 { vec4 y; };
        void main(){
           x = y;
        }
    )glsl";
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    // consume VU so we create a pipeline handle
    // Will crash various drivers
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGraphicsPipelineCreateInfo-layout-07988");
    pipe.CreateGraphicsPipeline();
    if (pipe == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Driver failed to create a invalid pipeline handle";
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
