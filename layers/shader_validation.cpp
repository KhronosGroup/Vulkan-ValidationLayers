/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#include <cinttypes>
#include <cassert>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <SPIRV/spirv.hpp>
#include "vk_loader_platform.h"
#include "vk_enum_string_helper.h"
#include "vk_layer_data.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "chassis.h"
#include "core_validation.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "xxhash.h"


void CoreChecks::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                 void *csm_state_data) {
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);
    if (enabled.gpu_validation) {
        GpuPreCallCreateShaderModule(pCreateInfo, pAllocator, pShaderModule, &csm_state->unique_shader_id,
                                     &csm_state->instrumented_create_info, &csm_state->instrumented_pgm);
    }
}

void CoreChecks::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                  VkResult result, void *csm_state_data) {
    if (VK_SUCCESS != result) return;
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);

    spv_target_env spirv_environment = ((api_version >= VK_API_VERSION_1_1) ? SPV_ENV_VULKAN_1_1 : SPV_ENV_VULKAN_1_0);
    bool is_spirv = (pCreateInfo->pCode[0] == spv::MagicNumber);
    std::unique_ptr<SHADER_MODULE_STATE> new_shader_module(
        is_spirv ? new SHADER_MODULE_STATE(pCreateInfo, *pShaderModule, spirv_environment, csm_state->unique_shader_id)
                 : new SHADER_MODULE_STATE());
    shaderModuleMap[*pShaderModule] = std::move(new_shader_module);
}

