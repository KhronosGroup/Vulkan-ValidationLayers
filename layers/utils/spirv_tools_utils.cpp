/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "spirv_tools_utils.h"

#include "generated/device_features.h"
#include "generated/vk_api_version.h"
#include "generated/vk_extension_helper.h"
#include "utils/hash_util.h"

#include <sstream>

spv_target_env PickSpirvEnv(const APIVersion &api_version, bool spirv_1_4) {
    if (api_version >= VK_API_VERSION_1_4) {
        return SPV_ENV_VULKAN_1_4;
    } else if (api_version >= VK_API_VERSION_1_3) {
        return SPV_ENV_VULKAN_1_3;
    } else if (api_version >= VK_API_VERSION_1_2) {
        return SPV_ENV_VULKAN_1_2;
    } else if (api_version >= VK_API_VERSION_1_1) {
        if (spirv_1_4) {
            return SPV_ENV_VULKAN_1_1_SPIRV_1_4;
        } else {
            return SPV_ENV_VULKAN_1_1;
        }
    }
    return SPV_ENV_VULKAN_1_0;
}

// Some Vulkan extensions/features are just all done in spirv-val behind optional settings
void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spv_target_env spirv_environment, spvtools::ValidatorOptions &out_options, uint32_t *out_hash,
                            std::string &out_command) {
    struct Settings {
        bool relax_block_layout;
        bool uniform_buffer_standard_layout;
        bool scalar_block_layout;
        bool workgroup_scalar_block_layout;
        bool allow_local_size_id;
        bool allow_offset_texture_operand;
        bool allow_vulkan_32_bit_bitwise;
    } settings;

    // VK_KHR_relaxed_block_layout never had a feature bit so just enabling the extension allows relaxed layout
    // Was promotoed in Vulkan 1.1 so anyone using Vulkan 1.1 also gets this for free
    settings.relax_block_layout = IsExtEnabled(device_extensions.vk_khr_relaxed_block_layout);
    // The rest of the settings are controlled from a feature bit, which are set correctly in the state tracking. Regardless of
    // Vulkan version used, the feature bit is needed (also described in the spec).
    settings.uniform_buffer_standard_layout = enabled_features.uniformBufferStandardLayout == VK_TRUE;
    settings.scalar_block_layout = enabled_features.scalarBlockLayout == VK_TRUE;
    settings.workgroup_scalar_block_layout = enabled_features.workgroupMemoryExplicitLayoutScalarBlockLayout == VK_TRUE;
    settings.allow_local_size_id = enabled_features.maintenance4 == VK_TRUE;
    settings.allow_offset_texture_operand = enabled_features.maintenance8 == VK_TRUE;
    settings.allow_vulkan_32_bit_bitwise = enabled_features.maintenance9 == VK_TRUE;

    std::stringstream ss;
    ss << "spirv-val <input.spv>";

    if (settings.relax_block_layout) {
        ss << " --relax-block-layout";
        out_options.SetRelaxBlockLayout(true);
    }
    if (settings.uniform_buffer_standard_layout) {
        ss << " --uniform-buffer-standard-layout";
        out_options.SetUniformBufferStandardLayout(true);
    }
    if (settings.scalar_block_layout) {
        ss << " --scalar-block-layout";
        out_options.SetScalarBlockLayout(true);
    }
    if (settings.workgroup_scalar_block_layout) {
        ss << " --workgroup-scalar-block-layout";
        out_options.SetWorkgroupScalarBlockLayout(true);
    }
    if (settings.allow_local_size_id) {
        ss << " --allow-localsizeid";
        out_options.SetAllowLocalSizeId(true);
    }
    if (settings.allow_offset_texture_operand) {
        ss << " --allow-offset-texture-operand";
        out_options.SetAllowOffsetTextureOperand(true);
    }
    if (settings.allow_vulkan_32_bit_bitwise) {
        ss << " --allow-vulkan-32-bit-bitwise";
        out_options.SetAllowVulkan32BitBitwise(true);
    }

    switch (spirv_environment) {
        case SPV_ENV_VULKAN_1_4:
            ss << " --target-env vulkan1.4";
            break;
        case SPV_ENV_VULKAN_1_3:
            ss << " --target-env vulkan1.3";
            break;
        case SPV_ENV_VULKAN_1_2:
            ss << " --target-env vulkan1.2";
            break;
        case SPV_ENV_VULKAN_1_1:
            ss << " --target-env vulkan1.1";
            break;
        case SPV_ENV_VULKAN_1_0:
            ss << " --target-env vulkan1.0";
            break;
        default:
            break;
    }

    // Faster validation without friendly names.
    out_options.SetFriendlyNames(false);

    // The spv_validator_options_t in libspirv.h is hidden so we can't just hash that struct, so instead need to create our own.
    if (out_hash) {
        *out_hash = hash_util::Hash32(&settings, sizeof(Settings));
    }

    ss << "\n";
    out_command = ss.str();
}