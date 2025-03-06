/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (c) 2025 RasterGrid Kft.
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

#include "state_tracker/shader_module.h"
#include "error_message/error_location.h"
#include "generated/device_features.h"
#include "chassis/dispatch_object.h"

namespace spirv {
struct StatelessData;
}

namespace stateless {

class SpirvValidator : public Logger {
  public:
    SpirvValidator(DebugReport* debug_report, const vvl::StatelessDeviceData& stateless_device_data)
        : Logger(debug_report),
          api_version(stateless_device_data.api_version),
          extensions(stateless_device_data.extensions),
          phys_dev_props(stateless_device_data.phys_dev_props),
          phys_dev_props_core11(stateless_device_data.phys_dev_props_core11),
          phys_dev_props_core12(stateless_device_data.phys_dev_props_core12),
          phys_dev_props_core13(stateless_device_data.phys_dev_props_core13),
          phys_dev_props_core14(stateless_device_data.phys_dev_props_core14),
          phys_dev_ext_props(stateless_device_data.phys_dev_ext_props),
          enabled_features(stateless_device_data.enabled_features),
          has_format_feature2(stateless_device_data.has_format_feature2) {}

    bool Validate(const spirv::Module& module_state, const spirv::StatelessData& stateless_data, const Location& loc) const;

    const APIVersion& api_version;
    const DeviceExtensions& extensions;
    const VkPhysicalDeviceProperties& phys_dev_props;
    const VkPhysicalDeviceVulkan11Properties& phys_dev_props_core11;
    const VkPhysicalDeviceVulkan12Properties& phys_dev_props_core12;
    const VkPhysicalDeviceVulkan13Properties& phys_dev_props_core13;
    const VkPhysicalDeviceVulkan14Properties& phys_dev_props_core14;
    const vvl::DeviceExtensionProperties& phys_dev_ext_props;
    const DeviceFeatures& enabled_features;
    const bool has_format_feature2;

  private:
    bool ValidateShaderClock(const spirv::Module& module_state, const spirv::StatelessData& stateless_data,
                             const Location& loc) const;
    bool ValidateAtomicsTypes(const spirv::Module& module_state, const spirv::StatelessData& stateless_data,
                              const Location& loc) const;
    bool ValidateVariables(const spirv::Module& module_state, const Location& loc) const;
    bool Validate8And16BitStorage(const spirv::Module& module_state, const spirv::Instruction& var_refsn,
                                  const Location& loc) const;
    bool ValidateShaderStorageImageFormatsVariables(const spirv::Module& module_state, const spirv::Instruction& insn,
                                                    const Location& loc) const;
    bool ValidateTransformFeedbackDecorations(const spirv::Module& module_state, const Location& loc) const;
    bool ValidateTransformFeedbackEmitStreams(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                              const spirv::StatelessData& stateless_data, const Location& loc) const;
    bool ValidateTexelOffsetLimits(const spirv::Module& module_state, const spirv::Instruction& insn, const Location& loc) const;
    bool ValidateMemoryScope(const spirv::Module& module_state, const spirv::Instruction& insn, const Location& loc) const;
    bool ValidateSubgroupRotateClustered(const spirv::Module& module_state, const spirv::Instruction& insn,
                                         const Location& loc) const;
    bool ValidateShaderStageGroupNonUniform(const spirv::Module& module_state, const spirv::StatelessData& stateless_data,
                                            VkShaderStageFlagBits stage, const Location& loc) const;
    bool ValidateShaderStageInputOutputLimits(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                              const spirv::StatelessData& stateless_data, const Location& loc) const;
    bool ValidateShaderFloatControl(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                    const spirv::StatelessData& stateless_data, const Location& loc) const;
    bool ValidateExecutionModes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                const spirv::StatelessData& stateless_data, const Location& loc) const;
    bool ValidatePhysicalStorageBuffers(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                        const Location& loc) const;
    bool ValidateConservativeRasterization(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                           const spirv::StatelessData& stateless_data, const Location& loc) const;

    // Auto-generated helper functions
    bool ValidateShaderCapabilitiesAndExtensions(const spirv::Module& module_state, const spirv::Instruction& insn,
                                                 const Location& loc) const;
};

}  // namespace stateless
