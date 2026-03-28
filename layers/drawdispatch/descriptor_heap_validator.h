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

#pragma once
#include <vulkan/vulkan.h>
#include "error_message/error_location.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

namespace spirv {
struct ResourceInterfaceVariable;
}  // namespace spirv

namespace gpuav {
class Validator;
class CommandBufferSubState;
}  // namespace gpuav

namespace vvl {
struct DrawDispatchVuid;
class DeviceProxy;

class DescriptorHeapValidator : public Logger {
  public:
    DescriptorHeapValidator(DeviceProxy& dev, gpuav::CommandBufferSubState& cb_state, uint32_t set_index,
                            const LogObjectList* objlist, const Location& loc);

    bool ValidateBinding(gpuav::Validator& gpuav, const spirv::ResourceInterfaceVariable& resource_variable,
                         const VkShaderDescriptorSetAndBindingMappingInfoEXT& mappings,
                         const gpuav::vko::IndirectAccessMap& indirect_access, uint32_t byte_offset, bool pipeline,
                         bool robustness);

    void SetObjlistForGpuAv(const LogObjectList* objlist) { this->objlist = objlist; }

  private:
    bool ValidateBinding(gpuav::Validator& gpuav, const spirv::ResourceInterfaceVariable& resource_variable,
                         const VkDescriptorSetAndBindingMappingEXT& mapping, const gpuav::vko::IndirectAccessMap& indirect_access,
                         uint32_t byte_offset, bool pipeline, bool robustness);

    gpuav::CommandBufferSubState& cb_state;
    LocationCapture loc;
    const DrawDispatchVuid* vuids;
    const LogObjectList* objlist;  // VkPipeline or VkShaderObject
};
}  // namespace vvl