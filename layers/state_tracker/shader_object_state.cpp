/* Copyright (c) 2023 Nintendo
 * Copyright (c) 2023 LunarG, Inc.
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

#include "shader_object_state.h"
#include "pipeline_layout_state.h"

static SHADER_OBJECT_STATE::SetLayoutVector GetSetLayouts(ValidationStateTracker *dev_data,
                                                          const VkShaderCreateInfoEXT &pCreateInfo) {
    SHADER_OBJECT_STATE::SetLayoutVector set_layouts(pCreateInfo.setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo.setLayoutCount; ++i) {
        set_layouts[i] = dev_data->Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo.pSetLayouts[i]);
    }
    return set_layouts;
}

SHADER_OBJECT_STATE::SHADER_OBJECT_STATE(ValidationStateTracker *dev_data, const VkShaderCreateInfoEXT &create_info,
                                         VkShaderEXT shader_object, std::shared_ptr<SPIRV_MODULE_STATE> &spirv_module,
                                         uint32_t createInfoCount, VkShaderEXT *pShaders, uint32_t unique_shader_id)
    : BASE_NODE(shader_object, kVulkanObjectTypeShaderEXT),
      create_info(&create_info),
      spirv(spirv_module),
      entrypoint(spirv ? spirv->FindEntrypoint(create_info.pName, create_info.stage) : nullptr),
      gpu_validation_shader_id(unique_shader_id),
      active_slots(GetActiveSlots(entrypoint)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      set_layouts(GetSetLayouts(dev_data, create_info)),
      push_constant_ranges(GetCanonicalId(create_info.pushConstantRangeCount, create_info.pPushConstantRanges)),
      set_compat_ids(GetCompatForSet(set_layouts, push_constant_ranges)) {
    if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            if (pShaders[i] != shader_object) {
                linked_shaders.push_back(pShaders[i]);
            }
        }
    }
}

VkPrimitiveTopology SHADER_OBJECT_STATE::GetTopology() const {
    if (spirv) {
        const auto topology = spirv->GetTopology(*entrypoint);
        if (topology) {
            return *topology;
        }
    }
    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}
