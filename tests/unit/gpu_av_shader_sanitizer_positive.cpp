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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "shader_helper.h"

class PositiveGpuAVShaderSanitizer : public GpuAVGpuAVShaderSanitizer {};

// Set a single SSBO to all zero
void GpuAVGpuAVShaderSanitizer::SimpleZeroComputeTest(const char *shader, int source_type, const char *expected_error,
                                                      uint32_t error_count) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, (SpvSourceType)source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void *in_ptr = in_buffer.Memory().Map();
    memset(in_ptr, 0, 256);

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    if (expected_error) m_errorMonitor->SetDesiredError(expected_error, error_count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    if (expected_error) m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVShaderSanitizer, DivideByOne) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            uint u_index;
            int s_index;
            uvec4 v_index;
            float f_index;

            uint u_result;
            int s_result;
            uvec4 v_result;
            float f_result;
        };

        void main() {
            u_result = 5 / (u_index + 1);
            s_result = 5 / (s_index - 1);
            v_result = uvec4(1) / (uvec4(1) + v_index);

            u_result = u_result / 2;
            f_result = 1 / f_index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL);
}
