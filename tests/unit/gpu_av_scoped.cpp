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

#include <filesystem>
#include <fstream>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "descriptor_helper.h"
#include "ray_tracing_objects.h"
#include "shader_object_helper.h"

class NegativeGpuAVScoped : public GpuAVTest {};

TEST_F(NegativeGpuAVScoped, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);

    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue}};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
        )glsl";

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                   &features);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedShadersRegex) {
    TEST_DESCRIPTION(
        "Selectively instrument shaders for validation, using regexes: all shaders matching regexes must be instrumented. Here it "
        "means they should emit errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 2> shader_regexes = {{"vertex_foo", "fragment_.*"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
        )glsl";

    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main");
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                   "main");

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;

    name_info.pObjectName = "vertex_foo";
    name_info.objectHandle = uint64_t(vs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    name_info.pObjectName = "fragment_bar";
    name_info.objectHandle = uint64_t(fs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    pipe.gp_ci_.layout = pipeline_layout;
    m_errorMonitor->SetDesiredInfo("vertex_foo");
    m_errorMonitor->SetDesiredInfo("fragment_bar");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedComputePipelineRegex) {
    TEST_DESCRIPTION("Selectively instrument a compute pipeline for validation, using regexes");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 1> shader_regexes = {{"pipeline_foo"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char cs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main");
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.pObjectName = "pipeline_foo";
    name_info.objectHandle = uint64_t(pipe.Handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDispatch-storageBuffers-06936", "pipeline_foo");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedComputePipelineCDLDump) {
    TEST_DESCRIPTION("Selectively instrument a compute pipeline for validation, using a CDL dump file as input");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char* cdl_dump_content = R"(
# ----------------------------------------------------------------
# -                    CRASH DIAGNOSTIC LAYER                    -
# ----------------------------------------------------------------

version: 1.4.350
startTime: 2026-07-06 18:02:38
timeSinceStart: 00:00:00.265
Settings:
  output_path: cdl_test_output\GpuCrash\InfiniteLoopSubmit2
  trace_on: false
  dump_queue_submits: running
  dump_command_buffers: running
  dump_commands: running
  dump_shaders: off
  trigger_watchdog_timeout: true
  watchdog_timeout_ms: 30000
  track_semaphores: true
  trace_all_semaphores: false
  instrument_all_commands: false
  sync_after_commands: false
SystemInfo:
  osName: Windows 10 Home
  osVersion: 26200
  osBitdepth: 64-bit
  osAdditional:
    build: ge_release
  cpuName: x64
  numCpus: 24
  totalRam: 63 GB
  totalDiskSpace: 1 TB
  availDiskSpace: 91 GB
Instance:
  handle: 0x000001C7C89F7E30 []
  applicationInfo:
    application: GpuCrash
    applicationVersion: 1
    engine: InfiniteLoopSubmit2
    engineVersion: 1
    apiVersion: 1.3.0 (0x00403000)
  extensions:
    - VK_EXT_debug_utils
    - VK_EXT_layer_settings
Device:
  handle: 0x000001C7CA5B9A30[]
  deviceName: CDL Mock Device
  apiVersion: 1.4.350 (0x0040415E)
  driverVersion: 0x00000001 (1)
  vendorID: 0xBA5EBA11
  deviceID: 0x0BADC0DE
  extensions:
    []
  Queues:
    -  # Queue
      handle: 0x000001C7CA5B5A40[]
      queueFamilyIndex: 0
      index: 0
      flags:
        - graphics
        - compute
        - transfer
        - sparse
        - protected
      completedSeq: 1
      submittedSeq: 4
      IncompleteSubmits:
        - type: vkQueueSubmit2
          startSeq: 1
          endSeq: 4
          SubmitInfos:
            - startSeq: 1
              endSeq: 4
              state: INCOMPLETE
              CommandBuffers:
                - 0x000001C7CA5C7D80[] INCOMPLETE 3
  CommandBuffers:
    -  # CommandBuffer
      state: INCOMPLETE
      handle: 0x000001C7CA5C7D80[]
      commandPool: 0x000001C7CA5F7E80[]
      queue: 0x000001C7CA5B5A40[]
      fence: 0x0000000000000000[]
      queueSeq: 3
      level: Primary
      simultaneousUse: false
      beginValue: 0x00000001
      endValue: 0x0000FFFF
      topCheckpointValue: 0x00000006
      bottomCheckpointValue: 0x00000003
      lastStartedCommand: 5
      lastCompletedCommand: 2
      Commands:
        -  # Command:
          id: 2
          checkpointValue: 0x00000003
          name: vkCmdBindPipeline
          state: COMPLETED
          parameters:
            pipelineBindPoint: VK_PIPELINE_BIND_POINT_COMPUTE
            pipeline: 0x0000000000000008[my pipeline]
          message: "'>>>>>>>>>>>>>> LAST COMPLETE COMMAND <<<<<<<<<<<<<<'"
        -  # Command:
          id: 3
          checkpointValue: 0x00000004
          name: vkCmdBindDescriptorSets
          state: INCOMPLETE
          parameters:
            pipelineBindPoint: VK_PIPELINE_BIND_POINT_COMPUTE
            layout: 0x0000000000000007[]
            firstSet: 0
            descriptorSetCount: 1
            pDescriptorSets:  # VkDescriptorSet
              - 0x0000000000000006[]
            dynamicOffsetCount: 0
            pDynamicOffsets: nullptr
        -  # Command:
          id: 4
          checkpointValue: 0x00000005
          name: vkCmdBeginDebugUtilsLabelEXT
          state: INCOMPLETE
          labels:
            - hang-expected
          parameters:
            pLabelInfo:
              sType: VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT
              pNext:
                - nullptr
              pLabelName: hang-expected
              color:  # float
                - 0
                - 0
                - 0
                - 0
        -  # Command:
          id: 5
          checkpointValue: 0x00000006
          name: vkCmdDispatch
          state: INCOMPLETE
          labels:
            - hang-expected
          parameters:
            groupCountX: 1
            groupCountY: 1
            groupCountZ: 1
          internalState:
            pipeline:
              handle: 0x0000000000000008[my pipeline]
              bindPoint: compute
              shaderInfos:
                - stage: cs
                  module: 0x0000000000000003[]
                  entry: main
            descriptorSets:
              -  # descriptorSet
                index: 0
                set: 0x0000000000000006[]
          message: "'^^^^^^^^^^^^^^ LAST STARTED COMMAND ^^^^^^^^^^^^^^'"
        )";

    const std::filesystem::path cdl_dump_path = std::filesystem::temp_directory_path() / "vvl_test_cdl_dump.yml";

    struct FileGuard {
        std::filesystem::path path;
        ~FileGuard() { std::filesystem::remove(path); }
    } cdl_dump_file_guard{cdl_dump_path};

    {
        std::ofstream cdl_dump_file(cdl_dump_path, std::ios::out | std::ios::trunc);
        ASSERT_TRUE(cdl_dump_file.is_open());
        cdl_dump_file << cdl_dump_content;
    }

    const std::string cdl_dump_path_str = cdl_dump_path.string();
    const char* cdl_dump_path_cstr = cdl_dump_path_str.c_str();

    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_cdl_dump_path", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &cdl_dump_path_cstr};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char cs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main");
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.pObjectName = "my pipeline";
    name_info.objectHandle = uint64_t(pipe.Handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDispatch-storageBuffers-06936", "my pipeline");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedPipelineLibrariesRegex) {
    TEST_DESCRIPTION(
        "Use one library to link two pipelines. Destroy first one before creating second one. Second one should be instrumented.");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 1> shader_regexes = {{"pipeline_foo"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t* data = (uint32_t*)offset_buffer.Memory().Map();
    *data = 8;

    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { uint index[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
        void main() {
            uint index = index[0];
            data[index] = 0xdeadca71;
        }
    )glsl";

    // Create VkShaderModule to pass in
    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    CreatePipelineHelper pre_raster_lib(*this);
    {
        pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
        pre_raster_lib.vp_state_ci_.pViewports = &viewport;
        pre_raster_lib.vp_state_ci_.pScissors = &scissor;
        pre_raster_lib.gp_ci_.layout = pipeline_layout;
        pre_raster_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
        frag_shader_lib.gp_ci_.layout = pipeline_layout;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    {
        vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    }
    vkt::Pipeline exe_pipe_2(*m_device, exe_pipe_ci);

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.pObjectName = "pipeline_foo";
    name_info.objectHandle = uint64_t(exe_pipe_2.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe_2);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDraw-storageBuffers-06936", "pipeline_foo", 3);

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedGraphicsPipelineRegex) {
    TEST_DESCRIPTION("Selectively instrument a pipeline for validation, using regexes");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 1> shader_regexes = {{"pipeline_foo"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
        )glsl";

    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main");
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                   "main");

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;

    name_info.pObjectName = "pipeline_foo";
    name_info.objectHandle = uint64_t(pipe.Handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDraw-storageBuffers-06936", "pipeline_foo", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedShadersRegexDestroyedShaders) {
    TEST_DESCRIPTION(
        "Selectively instrument shaders for validation, using regexes: all shaders matching regexes must be instrumented. Here it "
        "means they should emit errors. Shaders are destroyed after creating pipeline, it should have no impact.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 2> shader_regexes = {{"vertex_foo", "fragment_.*"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
            Data.data[4] = 0xdeadca71;
        }
    )glsl";

    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main");
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                   "main");

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;

    name_info.pObjectName = "vertex_foo";
    name_info.objectHandle = uint64_t(vs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    name_info.pObjectName = "fragment_bar";
    name_info.objectHandle = uint64_t(fs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    pipe.gp_ci_.layout = pipeline_layout;
    m_errorMonitor->SetDesiredInfo("vertex_foo");
    m_errorMonitor->SetDesiredInfo("fragment_bar");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
    vs.Destroy();
    fs.Destroy();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedShadersShaderObject) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);

    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue}};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_});

    const char vert_src[] = R"glsl(
        #version 460
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
            Data.data[4] = 0xdeadca71;
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_};

    VkShaderCreateInfoEXT vert_create_info = ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 1, descriptor_set_layouts);
    VkShaderCreateInfoEXT frag_create_info = ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 1, descriptor_set_layouts);

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    vert_create_info.pNext = &features;
    frag_create_info.pNext = &features;

    const vkt::Shader vert_shader(*m_device, vert_create_info);
    const vkt::Shader frag_shader(*m_device, frag_create_info);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &vert_descriptor_set.set_,
                              0u, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    // Should get a warning since shader was instrumented
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08613", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, SelectInstrumentedShadersShaderObjectDrawIndexedIndirect2KHR) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue}};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_});

    const char vert_src[] = R"glsl(
        #version 460
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
            Data.data[4] = 0xdeadca71;
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_};

    VkShaderCreateInfoEXT vert_create_info = ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 1, descriptor_set_layouts);
    VkShaderCreateInfoEXT frag_create_info = ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 1, descriptor_set_layouts);

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    vert_create_info.pNext = &features;
    frag_create_info.pNext = &features;

    const vkt::Shader vert_shader(*m_device, vert_create_info);
    const vkt::Shader frag_shader(*m_device, frag_create_info);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    uint32_t* index_data = reinterpret_cast<uint32_t*>(index_buffer.Memory().Map());
    index_data[0] = 0u;
    index_data[1] = 1u;
    index_data[2] = 2u;

    vkt::Buffer indirect_buffer(*m_device, 32, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndexedIndirectCommand* draw_command = reinterpret_cast<VkDrawIndexedIndirectCommand*>(indirect_buffer.Memory().Map());
    draw_command->indexCount = 3u;
    draw_command->instanceCount = 1u;
    draw_command->firstIndex = 0u;
    draw_command->vertexOffset = 0u;
    draw_command->firstInstance = 0u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &vert_descriptor_set.set_,
                              0u, nullptr);

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_info = vku::InitStructHelper();
    draw_indirect_info.addressRange = indirect_buffer.StridedAddressRange();
    draw_indirect_info.addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
    draw_indirect_info.drawCount = 1u;
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_info);

    m_command_buffer.EndRendering();
    m_command_buffer.End();

    // Should get a warning since shader was instrumented
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-None-08613", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, GPLWriteSelectPipeline) {
    TEST_DESCRIPTION("Select shaders within pipeline libraries for instrumentation");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);

    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 2> shader_regexes = {{"bim_bap_boom", ".*_my_pipeline.*"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t* data = (uint32_t*)offset_buffer.Memory().Map();
    *data = 8;

    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { uint index[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
        void main() {
            uint index = index[0];
            data[index] = 0xdeadca71;
        }
    )glsl";

    // Create VkShaderModule to pass in
    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    CreatePipelineHelper pre_raster_lib(*this);
    {
        pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
        pre_raster_lib.vp_state_ci_.pViewports = &viewport;
        pre_raster_lib.vp_state_ci_.pScissors = &scissor;
        pre_raster_lib.gp_ci_.layout = pipeline_layout;
        pre_raster_lib.CreateGraphicsPipeline();

        name_info.pObjectName = "oh_my_pipeline";
        name_info.objectHandle = uint64_t(pre_raster_lib.Handle());
        vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
        frag_shader_lib.gp_ci_.layout = pipeline_layout;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    m_errorMonitor->SetDesiredInfo("oh_my_pipeline");
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, GPLWriteSelectShaders) {
    TEST_DESCRIPTION("Select shaders within pipeline libraries for instrumentation");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);

    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 2> shader_regexes = {{"verte[xyz]_foo", ".*gment_.*"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};

    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t* data = (uint32_t*)offset_buffer.Memory().Map();
    *data = 8;

    const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { uint index[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
        void main() {
            uint index = index[0];
            data[index] = 0xdeadca71;
        }
    )glsl";

    // Create VkShaderModule to pass in
    VkShaderObj vs(*m_device, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;

    name_info.pObjectName = "vertex_foo";
    name_info.objectHandle = uint64_t(vs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    name_info.pObjectName = "fragment_bar";
    name_info.objectHandle = uint64_t(fs.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    CreatePipelineHelper pre_raster_lib(*this);
    {
        pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
        pre_raster_lib.vp_state_ci_.pViewports = &viewport;
        pre_raster_lib.vp_state_ci_.pScissors = &scissor;
        pre_raster_lib.gp_ci_.layout = pipeline_layout;
        pre_raster_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
        frag_shader_lib.gp_ci_.layout = pipeline_layout;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout;
    m_errorMonitor->SetDesiredInfo("vertex_foo");
    m_errorMonitor->SetDesiredInfo("fragment_bar");
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVScoped, RayGenUseQueryUninitSelectShaders) {
    TEST_DESCRIPTION(
        "rayQueryInitializeEXT is never called, make sure we don't hang with an uninit query object. Selectively instrument ray "
        "gen shader");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    std::vector<VkLayerSettingEXT> layer_settings(2);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    std::array<const char*, 3> shader_regexes = {{"vertex_foo", "fragment_.*", ".*ray.*"}};
    layer_settings[1] = {OBJECT_LAYER_NAME, "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes),
                         shader_regexes.data()};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) buffer SSBO {
            float x;
            float y;
        } params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,1,0), params.x, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
            if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
                params.y = rayQueryGetIntersectionTEXT(query, true);
            }
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    VkShaderObj& ray_gen_shader = pipeline.GetRayGenShader(0);
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
    name_info.pObjectName = "my_oh_my_ray_gen";
    name_info.objectHandle = uint64_t(ray_gen_shader.handle());
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer ssbo(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto buffer_ptr = static_cast<float*>(ssbo.Memory().Map());
    buffer_ptr[0] = -16.0f;

    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, ssbo, 0, 4096, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    m_errorMonitor->SetDesiredInfo("my_oh_my_ray_gen");
    pipeline.Build();
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}
