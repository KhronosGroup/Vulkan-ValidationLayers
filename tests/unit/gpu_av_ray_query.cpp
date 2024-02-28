/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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
#include "../framework/gpu_av_helper.h"
#include "../framework/ray_tracing_objects.h"
#include "../../layers/gpu_shaders/gpu_shaders_constants.h"

TEST_F(NegativeGpuAVRayQuery, NegativeTmin) {
    TEST_DESCRIPTION("Ray query with a negative value for Ray TMin");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float t_min;
          float t_max;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 
              trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.InitState();
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.memory().map());
    uniform_buffer_ptr[0] = -2.0f;  // t_min
    uniform_buffer_ptr[1] = 42.0f;  // t_max
    uniform_buffer.memory().unmap();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349");
    m_commandBuffer->QueueCommandBuffer(false);
    m_device->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, TMaxLessThenTmin) {
    TEST_DESCRIPTION("Ray query with a Ray TMax less than Ray TMin");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float t_min;
          float t_max;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 
              trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.InitState();
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.memory().map());
    uniform_buffer_ptr[0] = 9.9f;  // t_min
    uniform_buffer_ptr[1] = 9.8f;  // t_max
    uniform_buffer.memory().unmap();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350");
    m_commandBuffer->QueueCommandBuffer(false);
    m_device->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayFlagsBothSkip) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.InitState();
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.memory().map());
    uniform_buffer_ptr[0] = 0x100 | 0x200;  // SkipTrianglesKHR and SkipAABBsKHR
    uniform_buffer.memory().unmap();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889");
    m_commandBuffer->QueueCommandBuffer(false);
    m_device->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayFlagsOpaque) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.InitState();
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.memory().map());
    uniform_buffer_ptr[0] = 0x1 | 0x2;  // OpaqueKHR and NoOpaqueKHR
    uniform_buffer.memory().unmap();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891");
    m_commandBuffer->QueueCommandBuffer(false);
    m_device->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeTMinNaN) {
    TEST_DESCRIPTION("Ray query with a Ray TMax less than Ray TMin");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require
        
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float x;
          float y;
        } params;

        void main() {
            rayQueryEXT query;
            // fract(1.0 / 0.0) will produce a NaN
            float bad = fract(params.x / params.y);
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,bad,0), 1.0, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.InitState();
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.memory().map());
    uniform_buffer_ptr[0] = 1.0f;  // t_min
    uniform_buffer_ptr[1] = 0.0f;  // t_max
    uniform_buffer.memory().unmap();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351");
    m_commandBuffer->QueueCommandBuffer(false);
    m_device->wait();
    m_errorMonitor->VerifyFound();
}