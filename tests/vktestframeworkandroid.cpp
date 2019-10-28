//  VK tests
//
//  Copyright (c) 2015-2019 The Khronos Group Inc.
//  Copyright (c) 2015-2019 Valve Corporation
//  Copyright (c) 2015-2019 LunarG, Inc.
//  Copyright (c) 2015-2019 Google, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vktestframeworkandroid.h"
#include "shaderc/shaderc.hpp"
#include <android/log.h>

VkTestFramework::VkTestFramework() {}
VkTestFramework::~VkTestFramework() {}

// Define static elements
bool VkTestFramework::m_devsim_layer = false;
ANativeWindow *VkTestFramework::window = nullptr;

VkFormat VkTestFramework::GetFormat(VkInstance instance, vk_testing::Device *device) {
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(device->phy().handle(), VK_FORMAT_B8G8R8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
    vk::GetPhysicalDeviceFormatProperties(device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    printf("Error - device does not support VK_FORMAT_B8G8R8A8_UNORM nor VK_FORMAT_R8G8B8A8_UNORM - exiting\n");
    exit(0);
}

void VkTestFramework::InitArgs(int *argc, char *argv[]) {}
void VkTestFramework::Finish() {}

void TestEnvironment::SetUp() {
    vk_testing::set_error_callback(test_error_callback);

    vk::InitDispatchTable();
}

void TestEnvironment::TearDown() {}

// Android specific helper functions for shaderc.
struct shader_type_mapping {
    VkShaderStageFlagBits vkshader_type;
    shaderc_shader_kind shaderc_type;
};

static const shader_type_mapping shader_map_table[] = {
    {VK_SHADER_STAGE_VERTEX_BIT, shaderc_glsl_vertex_shader},
    {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, shaderc_glsl_tess_control_shader},
    {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, shaderc_glsl_tess_evaluation_shader},
    {VK_SHADER_STAGE_GEOMETRY_BIT, shaderc_glsl_geometry_shader},
    {VK_SHADER_STAGE_FRAGMENT_BIT, shaderc_glsl_fragment_shader},
    {VK_SHADER_STAGE_COMPUTE_BIT, shaderc_glsl_compute_shader},
};

shaderc_shader_kind MapShadercType(VkShaderStageFlagBits vkShader) {
    for (auto shader : shader_map_table) {
        if (shader.vkshader_type == vkShader) {
            return shader.shaderc_type;
        }
    }
    assert(false);
    return shaderc_glsl_infer_from_source;
}

// Compile a given string containing GLSL into SPIR-V
// Return value of false means an error was encountered
bool VkTestFramework::GLSLtoSPV(VkPhysicalDeviceLimits const *const device_limits, const VkShaderStageFlagBits shader_type,
                                const char *pshader, std::vector<unsigned int> &spirv, bool debug, uint32_t spirv_minor_version) {
    // On Android, use shaderc instead.
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    if (debug) {
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
        options.SetGenerateDebugInfo();
    }

    switch (spirv_minor_version) {
        default:
        case 0:
            options.SetTargetSpirv(shaderc_spirv_version_1_0);
            break;
        case 1:
            options.SetTargetSpirv(shaderc_spirv_version_1_1);
            break;
        case 2:
            options.SetTargetSpirv(shaderc_spirv_version_1_2);
            break;
        case 3:
            options.SetTargetSpirv(shaderc_spirv_version_1_3);
            break;
        case 4:
            options.SetTargetSpirv(shaderc_spirv_version_1_4);
            break;
    }

    // Override glslang built-in GL settings with actual hardware values
    options.SetLimit(shaderc_limit_max_clip_distances, device_limits->maxClipDistances);
    options.SetLimit(shaderc_limit_max_compute_work_group_count_x, device_limits->maxComputeWorkGroupCount[0]);
    options.SetLimit(shaderc_limit_max_compute_work_group_count_y, device_limits->maxComputeWorkGroupCount[1]);
    options.SetLimit(shaderc_limit_max_compute_work_group_count_z, device_limits->maxComputeWorkGroupCount[2]);
    options.SetLimit(shaderc_limit_max_compute_work_group_size_x, device_limits->maxComputeWorkGroupSize[0]);
    options.SetLimit(shaderc_limit_max_compute_work_group_size_y, device_limits->maxComputeWorkGroupSize[1]);
    options.SetLimit(shaderc_limit_max_compute_work_group_size_z, device_limits->maxComputeWorkGroupSize[2]);
    options.SetLimit(shaderc_limit_max_cull_distances, device_limits->maxCullDistances);
    options.SetLimit(shaderc_limit_max_fragment_input_components, device_limits->maxFragmentInputComponents);
    options.SetLimit(shaderc_limit_max_geometry_input_components, device_limits->maxGeometryInputComponents);
    options.SetLimit(shaderc_limit_max_geometry_output_components, device_limits->maxGeometryOutputComponents);
    options.SetLimit(shaderc_limit_max_geometry_output_vertices, device_limits->maxGeometryOutputVertices);
    options.SetLimit(shaderc_limit_max_geometry_total_output_components, device_limits->maxGeometryTotalOutputComponents);
    options.SetLimit(shaderc_limit_max_vertex_output_components, device_limits->maxVertexOutputComponents);
    options.SetLimit(shaderc_limit_max_viewports, device_limits->maxViewports);

    shaderc::SpvCompilationResult result =
        compiler.CompileGlslToSpv(pshader, strlen(pshader), MapShadercType(shader_type), "shader", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        __android_log_print(ANDROID_LOG_ERROR, "VulkanLayerValidationTests", "GLSLtoSPV compilation failed: %s",
                            result.GetErrorMessage().c_str());
        return false;
    }

    for (auto iter = result.begin(); iter != result.end(); iter++) {
        spirv.push_back(*iter);
    }

    return true;
}

//
// Compile a given string containing SPIR-V assembly into SPV for use by VK
// Return value of false means an error was encountered.
//
bool VkTestFramework::ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *pasm,
                               std::vector<unsigned int> &spv) {
    spv_binary binary;
    spv_diagnostic diagnostic = nullptr;
    spv_context context = spvContextCreate(target_env);
    spv_result_t error = spvTextToBinaryWithOptions(context, pasm, strlen(pasm), options, &binary, &diagnostic);
    spvContextDestroy(context);
    if (error) {
        __android_log_print(ANDROID_LOG_ERROR, "VkLayerValidationTest", "ASMtoSPV compilation failed");
        spvDiagnosticDestroy(diagnostic);
        return false;
    }
    spv.insert(spv.end(), binary->code, binary->code + binary->wordCount);
    spvBinaryDestroy(binary);

    return true;
}
