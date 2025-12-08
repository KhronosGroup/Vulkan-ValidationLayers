/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#pragma once

#include <spirv-tools/libspirv.hpp>
#include <glslang/Public/ShaderLang.h>

#include "render.h"
// This include is light and easier to have here a single place for all tests to grab
#include "shader_templates.h"

class VkRenderFramework;

// What is the incoming source to be turned into VkShaderModuleCreateInfo::pCode
typedef enum {
    SPV_SOURCE_GLSL,
    SPV_SOURCE_ASM,
    SPV_SOURCE_SLANG,
    // TRY == Won't try in contructor as need to be called as function that can return the VkResult
    SPV_SOURCE_GLSL_TRY,
    SPV_SOURCE_ASM_TRY,
} SpvSourceType;

bool GLSLtoSPV(const VkPhysicalDeviceLimits &device_limits, const VkShaderStageFlagBits shader_type, const char *p_shader,
               std::vector<uint32_t> &out_spv, const spv_target_env spv_env = SPV_ENV_VULKAN_1_0);
bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *p_asm, std::vector<uint32_t> &out_spv);
// We don't support installation of Slang on some platforms
void CheckSlangSupport();
bool SlangToSPV(const char *slang_shader, const char *entry_point_name, std::vector<uint8_t> &out_bytes);

// VkShaderObj is really just the Shader Module, but we named before VK_EXT_shader_object
// TODO - move all of VkShaderObj to vkt::ShaderModule
class VkShaderObj : public vkt::ShaderModule {
  public:
    VkShaderObj() = default;
    VkShaderObj(VkShaderObj &&rhs) noexcept = default;
    VkShaderObj &operator=(VkShaderObj &&rhs) noexcept = default;

    // optional arguments listed order of most likely to be changed manually by a test
    VkShaderObj(vkt::Device& device, const char* source, VkShaderStageFlagBits stage, const spv_target_env env = SPV_ENV_VULKAN_1_0,
                SpvSourceType source_type = SPV_SOURCE_GLSL, const VkSpecializationInfo* spec_info = nullptr,
                const char* entry_point = "main", const void* shader_module_ci_pNext = nullptr,
                const void* pipeline_shader_stage_ci_pNext = nullptr);

    VkPipelineShaderStageCreateInfo const &GetStageCreateInfo() const;

    bool InitFromGLSL(const void* shader_module_ci_pNext = nullptr);
    VkResult InitFromGLSLTry(const vkt::Device *custom_device = nullptr);
    bool InitFromASM();
    VkResult InitFromASMTry();
    bool InitFromSlang();

    // These functions return a pointer to a newly created _and initialized_ VkShaderObj if initialization was successful.
    // Otherwise, {} is returned.
    static VkShaderObj CreateFromGLSL(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                      const spv_target_env = SPV_ENV_VULKAN_1_0, const VkSpecializationInfo *spec_info = nullptr,
                                      const char *entry_point = "main");
    static VkShaderObj CreateFromASM(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                     const spv_target_env spv_env = SPV_ENV_VULKAN_1_0,
                                     const VkSpecializationInfo *spec_info = nullptr, const char *entry_point = "main");

  private:
    vkt::Device *m_device = nullptr;
    VkPipelineShaderStageCreateInfo m_stage_info;
    const char *m_source;
    spv_target_env m_spv_env;
};
