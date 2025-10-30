/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 LunarG, Inc.
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

#include "state_tracker/descriptor_set_layouts.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_module.h"

namespace vvl {

DescriptorSetLayoutList::DescriptorSetLayoutList(size_t size) : list(size){}

const vvl::DescriptorSetLayout *DescriptorSetLayoutList::FindFromVariable(const spirv::ResourceInterfaceVariable &variable) const {
    const uint32_t set = variable.decorations.set;
    if (set < list.size()) {
        const std::shared_ptr<vvl::DescriptorSetLayout const> set_layout = list[set];
        if (set_layout) {
            return set_layout.get();
        }
    }
    return nullptr;
}

bool DescriptorSetLayoutList::HasYcbcrSamplers() const {
    for (const auto &layout : list) {
        if (layout && layout->HasYcbcrSamplers()) {
            return true;
        }
    }
    return false;
}

}  // namespace vvl
