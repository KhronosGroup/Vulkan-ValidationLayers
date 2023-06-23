/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include "glslang/SPIRV/GLSL.std.450.h"
#include "spirv-tools/libspirv.h"
#include "glslang/Public/ShaderLang.h"
#include "test_common.h"

#include <fstream>
#include <iostream>
#include <list>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Can be used by tests to record additional details / description of test
#define TEST_DESCRIPTION(desc) RecordProperty("description", desc)

class VkImageObj;

class VkTestFramework : public ::testing::Test {
  public:
    static void InitArgs(int *argc, char *argv[]);
    static void Finish();

    virtual bool GLSLtoSPV(VkPhysicalDeviceLimits const *const device_limits, const VkShaderStageFlagBits shader_type,
                           const char *pshader, std::vector<uint32_t> &spv, bool debug = false,
                           const spv_target_env spv_env = SPV_ENV_VULKAN_1_0);
    virtual bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *pasm, std::vector<uint32_t> &spv);

    char **ReadFileData(const char *fileName);
    void FreeFileData(char **data);

    static inline bool m_canonicalize_spv = false;
    static inline bool m_print_vu = false;
    static inline bool m_strip_spv = false;
    static inline bool m_do_everything_spv = false;
    static inline int m_phys_device_index = -1;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    static inline ANativeWindow *window = nullptr;
#endif

  private:
    int m_compile_options = 0;
    int m_num_shader_strings = 0;
    TBuiltInResource Resources;
    void SetMessageOptions(EShMessages &messages);
    void ProcessConfigFile(VkPhysicalDeviceLimits const *const device_limits);
    EShLanguage FindLanguage(const std::string &name);
    EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);
    std::string ConfigFile;
    bool SetConfigFile(const std::string &name);
    std::string m_testName;

    static inline int m_width = 0;
    static inline int m_height = 0;
};

class TestEnvironment : public ::testing::Environment {
  public:
    void SetUp();

    void TearDown();
};
