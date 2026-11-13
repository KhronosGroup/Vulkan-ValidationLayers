/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "shader_helper.h"

class PositiveGpuAVExpectAssume : public GpuAVExpectAssume {};

void GpuAVExpectAssume::InitExpectAssume(bool safe_mode) {
    AddRequiredExtensions(VK_KHR_SHADER_EXPECT_ASSUME_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderExpectAssume);
    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
}

void GpuAVExpectAssume::ZeroComputeTest(const char* shader, int source_type, uint32_t error_count, const char* expected_error,
                                        const VkSpecializationInfo* spec_info) {
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ =
        VkShaderObj(*m_device, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, (SpvSourceType)source_type, spec_info);
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    memset(buffer.Memory().Map(), 0, 256);

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    if (error_count > 0) {
        m_errorMonitor->SetDesiredError(expected_error, error_count);
    }
    m_default_queue->SubmitAndWait(m_command_buffer);
    if (error_count > 0) {
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(PositiveGpuAVExpectAssume, AssumeTrue) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint result;
        };

        void main() {
            assumeEXT(count == 0);
            result = count;
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 0);
}

TEST_F(PositiveGpuAVExpectAssume, ExpectWrong) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint mode;
            uint result;
        };

        void main() {
            result = expectEXT(mode, 1) == 1 ? 1 : 2;
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 0);
}

TEST_F(PositiveGpuAVExpectAssume, AssumeNotReached) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint result;
        };

        void main() {
            if (count != 0) {
                assumeEXT(count == 0);
                result = 1;
            }
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 0);
}

TEST_F(PositiveGpuAVExpectAssume, SpecConstantTrue) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(constant_id = 0) const bool kCondition = false;
        layout(set=0, binding=0) buffer SSBO {
            uint result;
        };

        void main() {
            assumeEXT(kCondition);
            result = 1;
        }
    )glsl";

    const VkBool32 condition = VK_TRUE;
    const VkSpecializationMapEntry entry = {0, 0, sizeof(VkBool32)};
    const VkSpecializationInfo spec_info = {1, &entry, sizeof(VkBool32), &condition};
    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 0, nullptr, &spec_info);
}
