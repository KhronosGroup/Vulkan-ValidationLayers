/* Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 Valve Corporation
 * Copyright (c) 2025-2026 LunarG, Inc.
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

#include "descriptor_heap_validator.h"
#include "state_tracker/pipeline_state.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "state_tracker/shader_module.h"
#include "utils/math_utils.h"
#include "utils/shader_utils.h"
#include "../layers/containers/container_utils.h"
#include "../layers/core_checks/cc_buffer_address.h"
#include <xxhash.h>

namespace vvl {

DescriptorHeapValidator::DescriptorHeapValidator(vvl::DeviceProxy& dev, gpuav::CommandBufferSubState& cb_state, uint32_t set_index,
                                                 const LogObjectList* objlist, const Location& loc)
    : Logger(dev.debug_report),
      cb_state(cb_state),
      loc(loc),
      vuids(&GetDrawDispatchVuid(loc.function)),
      objlist(objlist) {}

bool DescriptorHeapValidator::ValidateBinding(gpuav::Validator &gpuav, const spirv::ResourceInterfaceVariable &resource_variable,
                                              const VkDescriptorSetAndBindingMappingEXT &mapping,
                                              const gpuav::vko::IndirectAccessMap &indirect_access, uint32_t byte_offset,
                                              bool pipeline, bool robustness) {
    bool skip = false;

    if (IsValueIn(mapping.source,
                  {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT,
                   VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT})) {
        VkDeviceAddress address = 0;
        if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
            address = cb_state.GetPushData<VkDeviceAddress>(mapping.sourceData.pushAddressOffset);
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
            gpuav::vko::IndirectKey key = {false, mapping.sourceData.indirectAddress.pushOffset,
                                           mapping.sourceData.indirectAddress.addressOffset};
            if (auto buffer = indirect_access->find(key); buffer != indirect_access->end()) {
                VkDeviceAddress *indirect_address = static_cast<VkDeviceAddress *>(buffer->second.GetHostBufferPtr());
                address = *indirect_address;
            }
        } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
            gpuav::vko::IndirectKey key = {true, 0, mapping.sourceData.shaderRecordAddressOffset};
            if (auto buffer = indirect_access->find(key); buffer != indirect_access->end()) {
                address = *static_cast<VkDeviceAddress *>(buffer->second.GetHostBufferPtr());
            }
        }
        std::string usage_vuid = {};
        VkBufferUsageFlagBits2 required_usage_bit = 0;

        if (resource_variable.is_uniform_buffer) {
            usage_vuid = CreateActionVuid(vuids->function, ActionVUID::HEAP_USAGE_11438);
        }

        if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT ||
            mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
            if (!usage_vuid.empty()) {
                std::string msg =
                    "The following buffers are missing " + std::string(string_VkBufferUsageFlagBits2(required_usage_bit));
                BufferAddressValidation<1> buffer_address_validator = {{{{usage_vuid,
                                                                          [required_usage_bit](const vvl::Buffer &buffer_state) {
                                                                              return (buffer_state.usage & required_usage_bit) == 0;
                                                                          },
                                                                          [msg]() { return msg; }, kUsageErrorMsgBuffer}}}};

                skip |=
                    buffer_address_validator.ValidateDeviceAddress(gpuav, loc.Get(), *objlist, address, 0u);
            }
        }
    }

    return skip;
}

bool DescriptorHeapValidator::ValidateBinding(gpuav::Validator &gpuav, const spirv::ResourceInterfaceVariable &resource_variable,
                                              const VkShaderDescriptorSetAndBindingMappingInfoEXT &mappings,
                                              const gpuav::vko::IndirectAccessMap &indirect_access, uint32_t byte_offset,
                                              bool pipeline, bool robustness) {
    bool skip = false;

    for (uint32_t i = 0; i < mappings.mappingCount; ++i) {
        const auto &mapping = mappings.pMappings[i];
        if (mapping.descriptorSet == resource_variable.decorations.set &&
            mapping.firstBinding == resource_variable.decorations.binding &&
            ResourceTypeMatchesBinding(mapping.resourceMask, resource_variable)) {
            skip |= ValidateBinding(gpuav, resource_variable, mapping, indirect_access, byte_offset, pipeline, robustness);
            break;
        }
    }

    return skip;
}

}  // namespace vvl