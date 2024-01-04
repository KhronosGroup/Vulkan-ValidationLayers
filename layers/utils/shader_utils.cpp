/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "shader_utils.h"

#include "state_tracker/device_state.h"
#include "generated/state_tracker_helper.h"
#include "generated/vk_extension_helper.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_instruction.h"

spv_target_env PickSpirvEnv(const APIVersion &api_version, bool spirv_1_4) {
    if (api_version >= VK_API_VERSION_1_3) {
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
                            spvtools::ValidatorOptions &options) {
    // VK_KHR_relaxed_block_layout never had a feature bit so just enabling the extension allows relaxed layout
    // Was promotoed in Vulkan 1.1 so anyone using Vulkan 1.1 also gets this for free
    if (IsExtEnabled(device_extensions.vk_khr_relaxed_block_layout)) {
        // --relax-block-layout
        options.SetRelaxBlockLayout(true);
    }

    // The rest of the settings are controlled from a feature bit, which are set correctly in the state tracking. Regardless of
    // Vulkan version used, the feature bit is needed (also described in the spec).

    if (enabled_features.uniformBufferStandardLayout == VK_TRUE) {
        // --uniform-buffer-standard-layout
        options.SetUniformBufferStandardLayout(true);
    }
    if (enabled_features.scalarBlockLayout == VK_TRUE) {
        // --scalar-block-layout
        options.SetScalarBlockLayout(true);
    }
    if (enabled_features.workgroupMemoryExplicitLayoutScalarBlockLayout) {
        // --workgroup-scalar-block-layout
        options.SetWorkgroupScalarBlockLayout(true);
    }
    if (enabled_features.maintenance4) {
        // --allow-localsizeid
        options.SetAllowLocalSizeId(true);
    }

    // Faster validation without friendly names.
    options.SetFriendlyNames(false);
}

void GetActiveSlots(ActiveSlotMap &active_slots, const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    if (!entrypoint) {
        return;
    }
    // Capture descriptor uses for the pipeline
    for (const auto &variable : entrypoint->resource_interface_variables) {
        // While validating shaders capture which slots are used by the pipeline
        DescriptorRequirement entry;
        entry.variable = &variable;
        entry.revalidate_hash = variable.descriptor_hash;
        active_slots[variable.decorations.set].emplace(variable.decorations.binding, entry);
    }
}

// static
ActiveSlotMap GetActiveSlots(const StageStateVec &stage_states) {
    ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        GetActiveSlots(active_slots, stage.entrypoint);
    }
    return active_slots;
}

ActiveSlotMap GetActiveSlots(const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    ActiveSlotMap active_slots;
    GetActiveSlots(active_slots, entrypoint);
    return active_slots;
}

uint32_t GetMaxActiveSlot(const ActiveSlotMap &active_slots) {
    uint32_t max_active_slot = 0;
    for (const auto &entry : active_slots) {
        max_active_slot = std::max(max_active_slot, entry.first);
    }
    return max_active_slot;
}

const char *PipelineStageState::GetPName() const {
    return (pipeline_create_info) ? pipeline_create_info->pName : shader_object_create_info->pName;
}

VkShaderStageFlagBits PipelineStageState::GetStage() const {
    return (pipeline_create_info) ? pipeline_create_info->stage : shader_object_create_info->stage;
}

safe_VkSpecializationInfo *PipelineStageState::GetSpecializationInfo() const {
    return (pipeline_create_info) ? pipeline_create_info->pSpecializationInfo : shader_object_create_info->pSpecializationInfo;
}

const void *PipelineStageState::GetPNext() const {
    return (pipeline_create_info) ? pipeline_create_info->pNext : shader_object_create_info->pNext;
}

bool PipelineStageState::GetInt32ConstantValue(const spirv::Instruction &insn, uint32_t *value) const {
    const spirv::Instruction *type_id = spirv_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeInt || type_id->Word(2) != 32) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstant) {
        *value = insn.Word(3);
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstant) {
        *value = insn.Word(3);  // default value
        const auto *spec_info = GetSpecializationInfo();
        const uint32_t spec_id = spirv_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t *)spec_info->pData + spec_info->pMapEntries[spec_id].offset,
                   spec_info->pMapEntries[spec_id].size);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

PipelineStageState::PipelineStageState(const safe_VkPipelineShaderStageCreateInfo *pipeline_create_info,
                                       const safe_VkShaderCreateInfoEXT *shader_object_create_info,
                                       std::shared_ptr<const vvl::ShaderModule> module_state,
                                       std::shared_ptr<const spirv::Module> spirv_state)
    : module_state(module_state),
      spirv_state(spirv_state),
      pipeline_create_info(pipeline_create_info),
      shader_object_create_info(shader_object_create_info),
      entrypoint(spirv_state ? spirv_state->FindEntrypoint(GetPName(), GetStage()) : nullptr) {}
