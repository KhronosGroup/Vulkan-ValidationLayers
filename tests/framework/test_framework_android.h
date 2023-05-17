//  VK tests
//
//  Copyright (c) 2015-2022 The Khronos Group Inc.
//  Copyright (c) 2015-2023 Valve Corporation
//  Copyright (c) 2015-2023 LunarG, Inc.
//  Copyright (c) 2015-2022 Google, Inc.
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
#pragma once

#include "spirv-tools/libspirv.h"
#include "test_common.h"

// Can be used by tests to record additional details / description of test
#define TEST_DESCRIPTION(desc) RecordProperty("description", desc)

#define ICD_SPV_MAGIC 0x07230203

class VkTestFramework : public ::testing::Test {
  public:
    static void InitArgs(int *argc, char *argv[]);
    static void Finish();

    virtual bool GLSLtoSPV(VkPhysicalDeviceLimits const *const device_limits, const VkShaderStageFlagBits shader_type,
                           const char *pshader, std::vector<uint32_t> &spv, bool debug = false,
                           const spv_target_env spv_ev = SPV_ENV_VULKAN_1_0);
    virtual bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *pasm, std::vector<uint32_t> &spv);

    static inline int m_phys_device_index = -1;
    static inline ANativeWindow *window = nullptr;
};

class TestEnvironment : public ::testing::Environment {
  public:
    void SetUp();

    void TearDown();
};
