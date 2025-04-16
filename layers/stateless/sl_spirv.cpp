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

#include "sl_spirv.h"
#include "generated/spirv_grammar_helper.h"
#include "chassis/dispatch_object.h"
#include "state_tracker/shader_module.h"
#include <inttypes.h>
#include <set>

namespace stateless {

// Temporary data of a OpVariable when validating it.
// If found useful in another location, can move out to the header
struct VariableInstInfo {
    bool has_8bit = false;
    bool has_16bit = false;
};

// easier to use recursion to traverse the OpTypeStruct
static void GetVariableInfo(const spirv::Module &module_state, const spirv::Instruction *insn, VariableInstInfo &info) {
    if (!insn) {
        return;
    } else if (insn->Opcode() == spv::OpTypePointer) {
        return;
    } else if (insn->Opcode() == spv::OpTypeFloat || insn->Opcode() == spv::OpTypeInt) {
        const uint32_t bit_width = insn->Word(2);
        info.has_8bit |= (bit_width == 8);
        info.has_16bit |= (bit_width == 16);
    } else if (insn->Opcode() == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); i++) {
            const spirv::Instruction *member_insn = module_state.FindDef(insn->Word(i));
            if (member_insn->StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
                continue;  // a uint8 pointer is not a 8-bit element
            }
            const uint32_t base_insn_id = module_state.GetBaseType(member_insn);
            const spirv::Instruction *base_insn = module_state.FindDef(base_insn_id);
            GetVariableInfo(module_state, base_insn, info);
        }
    }
}

SpirvValidator::SpirvValidator(DebugReport *debug_report, const vvl::StatelessDeviceData &stateless_device_data)
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
      special_supported(stateless_device_data.special_supported) {}

// stateless spirv == doesn't require pipeline state and/or shader object info
// Originally the goal was to move more validation to vkCreateShaderModule time in case the driver decided to parse an invalid
// SPIR-V here, while that is likely not the case anymore, a bigger reason for checking here is to save on memory. There is a lot of
// state saved in the Module that is only checked once later and could be reduced if not saved.
bool SpirvValidator::Validate(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                              const Location &loc) const {
    bool skip = false;
    if (!module_state.valid_spirv) return skip;

    skip |= ValidateShaderClock(module_state, stateless_data, loc);
    skip |= ValidateAtomicsTypes(module_state, stateless_data, loc);
    skip |= ValidateVariables(module_state, loc);

    if (enabled_features.transformFeedback) {
        skip |= ValidateTransformFeedbackDecorations(module_state, loc);
    }

    // The following tries to limit the number of passes through the shader module.
    // It save a good amount of memory and complex state tracking to just check these in a 2nd pass
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        skip |= ValidateShaderCapabilitiesAndExtensions(module_state, insn, loc);
        skip |= ValidateTexelOffsetLimits(module_state, insn, loc);
        skip |= ValidateMemoryScope(module_state, insn, loc);
        skip |= ValidateSubgroupRotateClustered(module_state, insn, loc);
    }

    for (const auto &entry_point : module_state.static_data_.entry_points) {
        skip |= ValidateShaderStageGroupNonUniform(module_state, stateless_data, entry_point->stage, loc);
        skip |= ValidateShaderStageInputOutputLimits(module_state, *entry_point, stateless_data, loc);
        skip |= ValidateShaderFloatControl(module_state, *entry_point, stateless_data, loc);
        skip |= ValidateExecutionModes(module_state, *entry_point, stateless_data, loc);
        skip |= ValidateConservativeRasterization(module_state, *entry_point, stateless_data, loc);
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackEmitStreams(module_state, *entry_point, stateless_data, loc);
        }
    }
    return skip;
}

bool SpirvValidator::ValidateShaderClock(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                         const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction *clock_inst : stateless_data.read_clock_inst) {
        const spirv::Instruction &insn = *clock_inst;
        const spirv::Instruction *scope_id = module_state.FindDef(insn.Word(3));
        auto scope_type = scope_id->Word(3);
        // if scope isn't Subgroup or Device, spirv-val will catch
        if ((scope_type == spv::ScopeSubgroup) && (enabled_features.shaderSubgroupClock == VK_FALSE)) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSubgroupClock-06267", module_state.handle(), loc,
                             "SPIR-V uses OpReadClockKHR with a Subgroup scope but shaderSubgroupClock was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
        } else if ((scope_type == spv::ScopeDevice) && (enabled_features.shaderDeviceClock == VK_FALSE)) {
            skip |= LogError("VUID-RuntimeSpirv-shaderDeviceClock-06268", module_state.handle(), loc,
                             "SPIR-V uses OpReadClockKHR with a Device scope but shaderDeviceClock was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
        }
    }
    return skip;
}

bool SpirvValidator::ValidateAtomicsTypes(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                          const Location &loc) const {
    bool skip = false;

    // "If sparseImageInt64Atomics is enabled, shaderImageInt64Atomics must be enabled"
    const bool valid_image_64_int = enabled_features.shaderImageInt64Atomics == VK_TRUE;

    const bool valid_storage_buffer_float =
        ((enabled_features.shaderBufferFloat32Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_workgroup_float =
        ((enabled_features.shaderSharedFloat32Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_image_float =
        ((enabled_features.shaderImageFloat32Atomics == VK_TRUE) || (enabled_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderImageFloat32AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_16_float =
        ((enabled_features.shaderBufferFloat16Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16AtomicMinMax == VK_TRUE));

    const bool valid_32_float =
        ((enabled_features.shaderBufferFloat32Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderImageFloat32Atomics == VK_TRUE) || (enabled_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderImageFloat32AtomicMinMax == VK_TRUE));

    const bool valid_64_float =
        ((enabled_features.shaderBufferFloat64Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64AtomicMinMax == VK_TRUE));

    const bool valid_16_float_vector = (enabled_features.shaderFloat16VectorAtomics == VK_TRUE);
    // clang-format on

    for (const spirv::Instruction *atomic_def_ptr : stateless_data.atomic_inst) {
        const spirv::Instruction &atomic_def = *atomic_def_ptr;
        const spirv::AtomicInstructionInfo &atomic = module_state.GetAtomicInfo(atomic_def);
        const uint32_t opcode = atomic_def.Opcode();

        if (atomic.type == spv::OpTypeFloat && (atomic.vector_size == 2 || atomic.vector_size == 4)) {
            if (!valid_16_float_vector) {
                skip |=
                    LogError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581", module_state.handle(), loc,
                             "SPIR-V is using 16-bit float vector atomics operations with %s storage class, but "
                             "shaderFloat16VectorAtomics was not enabled.\n%s\n",
                             string_SpvStorageClass(atomic.storage_class), module_state.DescribeInstruction(atomic_def).c_str());
            }
        } else if ((atomic.bit_width == 64) && (atomic.type == spv::OpTypeInt)) {
            // Validate 64-bit image atomics
            if (((atomic.storage_class == spv::StorageClassStorageBuffer) || (atomic.storage_class == spv::StorageClassUniform)) &&
                (enabled_features.shaderBufferInt64Atomics == VK_FALSE)) {
                skip |=
                    LogError("VUID-RuntimeSpirv-None-06278", module_state.handle(), loc,
                             "SPIR-V is using 64-bit int atomics operations with %s storage class, but "
                             "shaderBufferInt64Atomics was not enabled. \n%s\n",
                             string_SpvStorageClass(atomic.storage_class), module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.storage_class == spv::StorageClassWorkgroup) &&
                       (enabled_features.shaderSharedInt64Atomics == VK_FALSE)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06279", module_state.handle(), loc,
                                 "SPIR-V is using 64-bit int atomics operations with Workgroup storage class, but "
                                 "shaderSharedInt64Atomics was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_64_int == false)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06288", module_state.handle(), loc,
                                 "SPIR-V is using 64-bit int atomics operations with Image storage class, but "
                                 "shaderImageInt64Atomics was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            }
        } else if (atomic.type == spv::OpTypeFloat) {
            // Validate Floats
            if (atomic.storage_class == spv::StorageClassStorageBuffer) {
                if (valid_storage_buffer_float == false) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06284", module_state.handle(), loc,
                                     "SPIR-V is using float atomics operations with StorageBuffer storage class, but none of "
                                     "the required features were enabled.\n%s\n",
                                     module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if (atomic.storage_class == spv::StorageClassWorkgroup) {
                if (valid_workgroup_float == false) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06285", module_state.handle(), loc,
                                     "SPIR-V is using float atomics operations with Workgroup storage class, but none of the "
                                     "required features were enabled.\n%s\n",
                                     module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat16AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat32AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat64AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat16AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat32AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat64AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat16Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat32Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat64Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_float == false)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06286", module_state.handle(), loc,
                                 "SPIR-V is using float atomics operations with Image storage class, but none of the required "
                                 "features were enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 16) && (valid_16_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                    "SPIR-V is using 16-bit float atomics operations but none of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 32) && (valid_32_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                    "SPIR-V is using 32-bit float atomics operations but none of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 64) && (valid_64_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                    "SPIR-V is using 64-bit float atomics operations but snone of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            }
        }
    }
    return skip;
}

bool SpirvValidator::ValidateVariables(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction *insn : module_state.static_data_.variable_inst) {
        const uint32_t storage_class = insn->StorageClass();

        if (storage_class == spv::StorageClassWorkgroup) {
            // If Workgroup variable is initalized, make sure the feature is enabled
            if (insn->Length() > 4 && !enabled_features.shaderZeroInitializeWorkgroupMemory) {
                skip |= LogError("VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372", module_state.handle(), loc,
                                 "SPIR-V contains an OpVariable with Workgroup Storage Class with an Initializer operand, but "
                                 "shaderZeroInitializeWorkgroupMemory was not enabled.\n%s\n.",
                                 insn->Describe().c_str());
            }
        }

        skip |= Validate8And16BitStorage(module_state, *insn, loc);

        // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
        // disabled if VK_KHR_format_feature_flags2 is supported.
        //
        //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
        //
        // The other checks need to take into account the format features and so
        // we apply that in the descriptor set matching validation code (see
        // descriptor_sets.cpp).
        if (!special_supported.vk_khr_format_feature_flags2) {
            skip |= ValidateShaderStorageImageFormatsVariables(module_state, *insn, loc);
        }
    }

    return skip;
}

// This is to validate the VK_KHR_8bit_storage and VK_KHR_16bit_storage extensions
bool SpirvValidator::Validate8And16BitStorage(const spirv::Module &module_state, const spirv::Instruction &var_insn,
                                              const Location &loc) const {
    bool skip = false;

    // type will either be a float, int, or struct and if struct need to traverse it
    const spirv::Instruction *type = module_state.GetVariablePointerType(var_insn);
    VariableInstInfo info;
    GetVariableInfo(module_state, type, info);

    const uint32_t storage_class = var_insn.StorageClass();

    if (info.has_8bit) {
        if (!enabled_features.storageBuffer8BitAccess &&
            (storage_class == spv::StorageClassStorageBuffer || storage_class == spv::StorageClassShaderRecordBufferKHR ||
             storage_class == spv::StorageClassPhysicalStorageBuffer)) {
            skip |= LogError("VUID-RuntimeSpirv-storageBuffer8BitAccess-06328", module_state.handle(), loc,
                             "SPIR-V contains an 8-bit "
                             "OpVariable with %s Storage Class, but storageBuffer8BitAccess was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
        if (!enabled_features.uniformAndStorageBuffer8BitAccess && storage_class == spv::StorageClassUniform) {
            skip |= LogError(
                "VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329", module_state.handle(), loc,
                "SPIR-V contains an "
                "8-bit OpVariable with Uniform Storage Class, but uniformAndStorageBuffer8BitAccess was not enabled.\n%s\n",
                var_insn.Describe().c_str());
        }
        if (!enabled_features.storagePushConstant8 && storage_class == spv::StorageClassPushConstant) {
            skip |= LogError("VUID-RuntimeSpirv-storagePushConstant8-06330", module_state.handle(), loc,
                             "SPIR-V contains an 8-bit "
                             "OpVariable with PushConstant Storage Class, but storagePushConstant8 was not enabled.\n%s\n",
                             var_insn.Describe().c_str());
        }
    }

    if (info.has_16bit) {
        if (!enabled_features.storageBuffer16BitAccess &&
            (storage_class == spv::StorageClassStorageBuffer || storage_class == spv::StorageClassShaderRecordBufferKHR ||
             storage_class == spv::StorageClassPhysicalStorageBuffer)) {
            skip |= LogError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with %s Storage Class, but storageBuffer16BitAccess was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
        if (!enabled_features.uniformAndStorageBuffer16BitAccess && storage_class == spv::StorageClassUniform) {
            skip |= LogError(
                "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332", module_state.handle(), loc,
                "SPIR-V contains an "
                "16-bit OpVariable with Uniform Storage Class, but uniformAndStorageBuffer16BitAccess was not enabled.\n%s\n",
                var_insn.Describe().c_str());
        }
        if (!enabled_features.storagePushConstant16 && storage_class == spv::StorageClassPushConstant) {
            skip |= LogError("VUID-RuntimeSpirv-storagePushConstant16-06333", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with PushConstant Storage Class, but storagePushConstant16 was not enabled.\n%s\n",
                             var_insn.Describe().c_str());
        }
        if (!enabled_features.storageInputOutput16 &&
            (storage_class == spv::StorageClassInput || storage_class == spv::StorageClassOutput)) {
            skip |= LogError("VUID-RuntimeSpirv-storageInputOutput16-06334", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with %s Storage Class, but storageInputOutput16 was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
    }

    return skip;
}

bool SpirvValidator::ValidateShaderStorageImageFormatsVariables(const spirv::Module &module_state, const spirv::Instruction &insn,
                                                                const Location &loc) const {
    bool skip = false;
    // Go through all variables for images and check decorations
    // Note: Tried to move to ResourceInterfaceVariable but the issue is the variables don't need to be accessed in the entrypoint
    // to trigger the error.
    assert(insn.Opcode() == spv::OpVariable);
    // spirv-val validates this is an OpTypePointer
    const spirv::Instruction *pointer_def = module_state.FindDef(insn.TypeId());
    if (pointer_def->Word(2) != spv::StorageClassUniformConstant) {
        return skip;  // Vulkan Spec says storage image must be UniformConstant
    }
    const spirv::Instruction *type_def = module_state.FindDef(pointer_def->Word(3));

    // Unpack an optional level of arraying
    if (type_def && type_def->IsArray()) {
        type_def = module_state.FindDef(type_def->Word(2));
    }

    if (type_def && type_def->Opcode() == spv::OpTypeImage) {
        // Only check if the Image Dim operand is not SubpassData
        const uint32_t dim = type_def->Word(3);
        // Only check storage images
        const uint32_t sampled = type_def->Word(7);
        const uint32_t image_format = type_def->Word(8);
        if ((dim == spv::DimSubpassData) || (sampled != 2) || (image_format != spv::ImageFormatUnknown)) {
            return skip;
        }

        const uint32_t var_id = insn.ResultId();
        const auto decorations = module_state.GetDecorationSet(var_id);

        if (!enabled_features.shaderStorageImageReadWithoutFormat && !decorations.Has(spirv::DecorationSet::nonreadable_bit)) {
            skip |=
                LogError("VUID-RuntimeSpirv-apiVersion-07955", module_state.handle(), loc,
                         "SPIR-V variable\n%s\nhas an Image\n%s\nwith Unknown "
                         "format and is not decorated with NonReadable, but shaderStorageImageReadWithoutFormat is not supported.",
                         module_state.FindDef(var_id)->Describe().c_str(), type_def->Describe().c_str());
        }

        if (!enabled_features.shaderStorageImageWriteWithoutFormat && !decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-apiVersion-07954", module_state.handle(), loc,
                "SPIR-V variable\n%s\nhas an Image\n%s\nwith "
                "Unknown format and is not decorated with NonWritable, but shaderStorageImageWriteWithoutFormat is not supported.",
                module_state.FindDef(var_id)->Describe().c_str(), type_def->Describe().c_str());
        }
    }

    return skip;
}

bool SpirvValidator::ValidateTransformFeedbackDecorations(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;

    std::vector<const spirv::Instruction *> xfb_streams;
    std::vector<const spirv::Instruction *> xfb_buffers;
    std::vector<const spirv::Instruction *> xfb_offsets;

    for (const spirv::Instruction *op_decorate : module_state.static_data_.decoration_inst) {
        uint32_t decoration = op_decorate->Word(2);
        if (decoration == spv::DecorationXfbStride) {
            uint32_t stride = op_decorate->Word(3);
            if (stride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride) {
                skip |= LogError("VUID-RuntimeSpirv-XfbStride-06313", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback with xfb_stride (%" PRIu32
                                 ") greater than maxTransformFeedbackBufferDataStride (%" PRIu32 ").",
                                 stride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
            }
        }
        if (decoration == spv::DecorationStream) {
            xfb_streams.push_back(op_decorate);
            uint32_t stream = op_decorate->Word(3);
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError("VUID-RuntimeSpirv-Stream-06312", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback with stream (%" PRIu32
                                 ") not less than maxTransformFeedbackStreams (%" PRIu32 ").",
                                 stream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
        if (decoration == spv::DecorationXfbBuffer) {
            xfb_buffers.push_back(op_decorate);
        }
        if (decoration == spv::DecorationOffset) {
            xfb_offsets.push_back(op_decorate);
        }
    }

    // XfbBuffer, buffer data size
    std::vector<std::pair<uint32_t, uint32_t>> buffer_data_sizes;
    for (const spirv::Instruction *op_decorate : xfb_offsets) {
        for (const spirv::Instruction *xfb_buffer : xfb_buffers) {
            if (xfb_buffer->Word(1) == op_decorate->Word(1)) {
                const auto offset = op_decorate->Word(3);
                const spirv::Instruction *def = module_state.FindDef(xfb_buffer->Word(1));
                const auto size = module_state.GetTypeBytesSize(def);
                const uint32_t buffer_data_size = offset + size;
                if (buffer_data_size > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize) {
                    skip |= LogError(
                        "VUID-RuntimeSpirv-Offset-06308", module_state.handle(), loc,
                        "SPIR-V uses transform feedback with xfb_offset (%" PRIu32 ") + size of variable (%" PRIu32
                        ") greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                        "(%" PRIu32 ").",
                        offset, size, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
                }

                bool found = false;
                for (auto &bds : buffer_data_sizes) {
                    if (bds.first == xfb_buffer->Word(1)) {
                        bds.second = std::max(bds.second, buffer_data_size);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    buffer_data_sizes.emplace_back(xfb_buffer->Word(1), buffer_data_size);
                }

                break;
            }
        }
    }

    vvl::unordered_map<uint32_t, uint32_t> stream_data_size;
    for (const spirv::Instruction *xfb_stream : xfb_streams) {
        for (const auto &bds : buffer_data_sizes) {
            if (xfb_stream->Word(1) == bds.first) {
                uint32_t stream = xfb_stream->Word(3);
                const auto itr = stream_data_size.find(stream);
                if (itr != stream_data_size.end()) {
                    itr->second += bds.second;
                } else {
                    stream_data_size.insert({stream, bds.second});
                }
            }
        }
    }

    for (const auto &stream : stream_data_size) {
        if (stream.second > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            skip |= LogError(
                "VUID-RuntimeSpirv-XfbBuffer-06309", module_state.handle(), loc,
                "SPIR-V uses transform feedback with stream (%" PRIu32 ") having the sum of buffer data sizes (%" PRIu32
                ") not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                "(%" PRIu32 ").",
                stream.first, stream.second, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
        }
    }

    return skip;
}

bool SpirvValidator::ValidateTransformFeedbackEmitStreams(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                          const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;
    if (entrypoint.stage != VK_SHADER_STAGE_GEOMETRY_BIT) {
        return skip;  // GeometryStreams are only used in Geomtry Shaders
    }

    vvl::unordered_set<uint32_t> emitted_streams;
    for (const spirv::Instruction *insn : stateless_data.transform_feedback_stream_inst) {
        const uint32_t opcode = insn->Opcode();
        if (opcode == spv::OpEmitStreamVertex) {
            emitted_streams.emplace(module_state.GetConstantValueById(insn->Word(1)));
        }
        if (opcode == spv::OpEmitStreamVertex || opcode == spv::OpEndStreamPrimitive) {
            uint32_t stream = module_state.GetConstantValueById(insn->Word(1));
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError("VUID-RuntimeSpirv-OpEmitStreamVertex-06310", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback stream\n%s\nwith index %" PRIu32
                                 ", which is not less than maxTransformFeedbackStreams (%" PRIu32 ").",
                                 insn->Describe().c_str(), stream,
                                 phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
    }

    const bool output_points = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::output_points_bit);
    const uint32_t emitted_streams_size = static_cast<uint32_t>(emitted_streams.size());
    if (emitted_streams_size > 1 && !output_points &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        skip |= LogError("VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311", module_state.handle(), loc,
                         "SPIR-V emits to %" PRIu32
                         " vertex streams and transformFeedbackStreamsLinesTriangles "
                         "is VK_FALSE, but execution mode is not OutputPoints.",
                         emitted_streams_size);
    }

    return skip;
}

// Checks for both TexelOffset and TexelGatherOffset limits
bool SpirvValidator::ValidateTexelOffsetLimits(const spirv::Module &module_state, const spirv::Instruction &insn,
                                               const Location &loc) const {
    bool skip = false;

    const uint32_t opcode = insn.Opcode();
    const bool is_image_gather = ImageGatherOperation(opcode);
    if (!is_image_gather && !ImageSampleOperation(opcode) && !ImageFetchOperation(opcode)) {
        return false;
    }

    const uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
    // Image operands can be optional
    if (image_operand_position == 0 || insn.Length() <= image_operand_position) {
        return false;
    }

    const uint32_t image_operand = insn.Word(image_operand_position);

    // Bits we are validating (sample/fetch only check ConstOffset)
    uint32_t offset_bits =
        is_image_gather ? (spv::ImageOperandsOffsetMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsConstOffsetsMask)
                        : (spv::ImageOperandsConstOffsetMask);
    if ((image_operand & offset_bits) == 0) {
        return false;
    }

    // Operand values follow
    uint32_t index = image_operand_position + 1;
    // Each bit has it's own operand, starts with the smallest set bit and loop to the highest bit among
    // ImageOperandsOffsetMask, ImageOperandsConstOffsetMask and ImageOperandsConstOffsetsMask
    for (uint32_t i = 1; i < spv::ImageOperandsConstOffsetsMask; i <<= 1) {
        if ((image_operand & i) == 0) {
            continue;
        }

        // If the bit is set, consume operand
        if (insn.Length() > index && (i & offset_bits)) {
            uint32_t constant_id = insn.Word(index);
            const spirv::Instruction *constant = module_state.FindDef(constant_id);
            const bool is_dynamic_offset = constant == nullptr;
            if (!is_dynamic_offset && constant->Opcode() == spv::OpConstantComposite) {
                for (uint32_t j = 3; j < constant->Length(); ++j) {
                    uint32_t comp_id = constant->Word(j);
                    const spirv::Instruction *comp = module_state.FindDef(comp_id);
                    const spirv::Instruction *comp_type = module_state.FindDef(comp->Word(1));
                    // Get operand value
                    const uint32_t offset = comp->Word(3);
                    // spec requires minTexelGatherOffset/minTexelOffset to be -8 or less so never can compare if
                    // unsigned spec requires maxTexelGatherOffset/maxTexelOffset to be 7 or greater so never can
                    // compare if signed is less then zero
                    const int32_t signed_offset = static_cast<int32_t>(offset);
                    const bool use_signed = (comp_type->Opcode() == spv::OpTypeInt && comp_type->Word(3) != 0);

                    // There are 2 sets of VU being covered where the only main difference is the opcode
                    if (is_image_gather) {
                        // min/maxTexelGatherOffset
                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelGatherOffset)) {
                            skip |=
                                LogError("VUID-RuntimeSpirv-OpImage-06376", module_state.handle(), loc,
                                         "SPIR-V uses %s with offset (%" PRId32
                                         ") less than VkPhysicalDeviceLimits::minTexelGatherOffset (%" PRId32 ").\n%s\n",
                                         string_SpvOpcode(insn.Opcode()), signed_offset, phys_dev_props.limits.minTexelGatherOffset,
                                         module_state.DescribeInstruction(insn).c_str());
                        } else if ((offset > phys_dev_props.limits.maxTexelGatherOffset) &&
                                   (!use_signed || (use_signed && signed_offset > 0))) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImage-06377", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRIu32
                                             ") greater than VkPhysicalDeviceLimits::maxTexelGatherOffset (%" PRIu32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), offset, phys_dev_props.limits.maxTexelGatherOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        }
                    } else {
                        // min/maxTexelOffset
                        // TODO - maintenance8 added ability to use Offset, but will need validation on GPU side
                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelOffset)) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImageSample-06435", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRId32
                                             ") less than VkPhysicalDeviceLimits::minTexelOffset (%" PRId32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), signed_offset, phys_dev_props.limits.minTexelOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        } else if ((offset > phys_dev_props.limits.maxTexelOffset) &&
                                   (!use_signed || (use_signed && signed_offset > 0))) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImageSample-06436", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRIu32
                                             ") greater than VkPhysicalDeviceLimits::maxTexelOffset (%" PRIu32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), offset, phys_dev_props.limits.maxTexelOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        }
                    }
                }
            }
        }
        index += ImageOperandsParamCount(i);
    }

    return skip;
}

bool SpirvValidator::ValidateMemoryScope(const spirv::Module &module_state, const spirv::Instruction &insn,
                                         const Location &loc) const {
    bool skip = false;

    const auto &entry = OpcodeMemoryScopePosition(insn.Opcode());
    if (entry > 0) {
        const uint32_t scope_id = insn.Word(entry);
        const spirv::Instruction *scope_def = module_state.GetConstantDef(scope_id);
        if (scope_def) {
            const spv::Scope scope_type = spv::Scope(scope_def->GetConstantValue());
            if (enabled_features.vulkanMemoryModel && !enabled_features.vulkanMemoryModelDeviceScope &&
                scope_type == spv::Scope::ScopeDevice) {
                skip |=
                    LogError("VUID-RuntimeSpirv-vulkanMemoryModel-06265", module_state.handle(), loc,
                             "SPIR-V uses Device memory scope, but the vulkanMemoryModelDeviceScope feature was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
            } else if (!enabled_features.vulkanMemoryModel && scope_type == spv::Scope::ScopeQueueFamily) {
                skip |= LogError("VUID-RuntimeSpirv-vulkanMemoryModel-06266", module_state.handle(), loc,
                                 "SPIR-V uses QueueFamily memory scope, but the vulkanMemoryModel feature was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(insn).c_str());
            }
        }
    }

    return skip;
}

bool SpirvValidator::ValidateSubgroupRotateClustered(const spirv::Module &module_state, const spirv::Instruction &insn,
                                                     const Location &loc) const {
    bool skip = false;
    if (!enabled_features.shaderSubgroupRotateClustered && insn.Opcode() == spv::OpGroupNonUniformRotateKHR && insn.Length() == 7) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSubgroupRotateClustered-09566", module_state.handle(), loc,
                         "SPIR-V uses ClusterSize operand, but the shaderSubgroupRotateClustered feature was not enabled.\n%s\n",
                         module_state.DescribeInstruction(insn).c_str());
    }
    return skip;
}

bool SpirvValidator::ValidateShaderStageGroupNonUniform(const spirv::Module &module_state,
                                                        const spirv::StatelessData &stateless_data, VkShaderStageFlagBits stage,
                                                        const Location &loc) const {
    bool skip = false;

    // Check anything using a group operation (which currently is only OpGroupNonUnifrom* operations)
    for (const spirv::Instruction *group_inst : stateless_data.group_inst) {
        const spirv::Instruction &insn = *group_inst;
        // Check the quad operations.
        if ((insn.Opcode() == spv::OpGroupNonUniformQuadBroadcast) || (insn.Opcode() == spv::OpGroupNonUniformQuadSwap)) {
            if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) && (stage != VK_SHADER_STAGE_COMPUTE_BIT)) {
                if (!phys_dev_props_core11.subgroupQuadOperationsInAllStages) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06342", module_state.handle(), loc,
                                     "Can't use for stage %s because VkPhysicalDeviceSubgroupProperties::quadOperationsInAllStages "
                                     "is not supported on this VkDevice",
                                     string_VkShaderStageFlagBits(stage));
                }
            }
        }

        uint32_t scope_type = spv::ScopeMax;
        if (insn.Opcode() == spv::OpGroupNonUniformPartitionNV) {
            // OpGroupNonUniformPartitionNV always assumed subgroup as missing operand
            scope_type = spv::ScopeSubgroup;
        } else {
            // "All <id> used for Scope <id> must be of an OpConstant"
            const spirv::Instruction *scope_id = module_state.FindDef(insn.Word(3));
            scope_type = scope_id->Word(3);
        }

        if (scope_type == spv::ScopeSubgroup) {
            // "Group operations with subgroup scope" must have stage support
            const VkSubgroupFeatureFlags supported_stages = phys_dev_props_core11.subgroupSupportedStages;
            if ((supported_stages & stage) == 0) {
                skip |= LogError("VUID-RuntimeSpirv-None-06343", module_state.handle(), loc,
                                 "%s is not supported in subgroupSupportedStages (%s).", string_VkShaderStageFlagBits(stage),
                                 string_VkShaderStageFlags(supported_stages).c_str());
            }
        }

        if (!enabled_features.shaderSubgroupExtendedTypes) {
            const spirv::Instruction *type = module_state.FindDef(insn.Word(1));

            if (type->Opcode() == spv::OpTypeVector) {
                // Get the element type
                type = module_state.FindDef(type->Word(2));
            }

            if (type->Opcode() != spv::OpTypeBool) {
                // Both OpTypeInt and OpTypeFloat the width is in the 2nd word.
                const uint32_t width = type->Word(2);

                if ((type->Opcode() == spv::OpTypeFloat && width == 16) ||
                    (type->Opcode() == spv::OpTypeInt && (width == 8 || width == 16 || width == 64))) {
                    if (!enabled_features.shaderSubgroupExtendedTypes) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-None-06275", module_state.handle(), loc,
                            "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes was not enabled");
                    }
                }
            }
        }
    }

    return skip;
}

bool SpirvValidator::ValidateShaderStageInputOutputLimits(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                          const spirv::StatelessData &stateless_data, const Location &loc) const {
    const VkShaderStageFlagBits stage = entrypoint.stage;
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT || stage == VK_SHADER_STAGE_ALL_GRAPHICS || stage == VK_SHADER_STAGE_ALL) {
        return false;
    }

    bool skip = false;
    auto const &limits = phys_dev_props.limits;

    const uint32_t num_vertices = entrypoint.execution_mode.output_vertices;
    const uint32_t num_primitives = entrypoint.execution_mode.output_primitives;
    const bool is_iso_lines = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::iso_lines_bit);
    const bool is_point_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit);

    // The max is a combiniation of both the user defined variables largest values
    // and
    // The total components used by built ins
    const auto max_input_slot = (entrypoint.max_input_slot_variable && entrypoint.max_input_slot)
                                    ? *entrypoint.max_input_slot
                                    : spirv::InterfaceSlot(0, 0, 0, 0);
    const auto max_output_slot = (entrypoint.max_output_slot_variable && entrypoint.max_output_slot)
                                     ? *entrypoint.max_output_slot
                                     : spirv::InterfaceSlot(0, 0, 0, 0);

    const uint32_t total_input_components = max_input_slot.slot + entrypoint.builtin_input_components;
    const uint32_t total_output_components = max_output_slot.slot + entrypoint.builtin_output_components;

    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            if (total_output_components >= limits.maxVertexOutputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Vertex stage) output interface variable (%s) along with %" PRIu32
                                 " built-in components,  "
                                 "exceeds component limit maxVertexOutputComponents (%" PRIu32 ").",
                                 max_output_slot.Describe().c_str(), entrypoint.builtin_output_components,
                                 limits.maxVertexOutputComponents);
            }
            break;

        // For tessellation, it is not clear if the built-ins should be part of the total component limits or not
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3448#note_459088
        // But seems that from Zink, that GL allowed it, so likely there is some exceptions for tessellation
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            if (max_input_slot.slot >= limits.maxTessellationControlPerVertexInputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Tessellation control stage) input interface variable (%s) "
                                 "exceeds component limit maxTessellationControlPerVertexInputComponents (%" PRIu32 ").",
                                 max_input_slot.Describe().c_str(), limits.maxTessellationControlPerVertexInputComponents);
            }
            if (entrypoint.max_input_slot_variable) {
                if (entrypoint.max_input_slot_variable->is_patch &&
                    max_output_slot.slot >= limits.maxTessellationControlPerPatchOutputComponents) {
                    skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                     "SPIR-V (Tessellation control stage) output interface variable (%s) "
                                     "exceeds component limit maxTessellationControlPerPatchOutputComponents (%" PRIu32 ").",
                                     max_output_slot.Describe().c_str(), limits.maxTessellationControlPerPatchOutputComponents);
                }
                if (!entrypoint.max_input_slot_variable->is_patch &&
                    max_output_slot.slot >= limits.maxTessellationControlPerVertexOutputComponents) {
                    skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                     "SPIR-V (Tessellation control stage) output interface variable (%s) "
                                     "exceeds component limit maxTessellationControlPerVertexOutputComponents (%" PRIu32 ").",
                                     max_output_slot.Describe().c_str(), limits.maxTessellationControlPerVertexOutputComponents);
                }
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            if (max_input_slot.slot >= limits.maxTessellationEvaluationInputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Tessellation evaluation stage) input interface variable (%s) "
                                 "exceeds component limit maxTessellationEvaluationInputComponents (%" PRIu32 ").",
                                 max_input_slot.Describe().c_str(), limits.maxTessellationEvaluationInputComponents);
            }
            if (max_output_slot.slot >= limits.maxTessellationEvaluationOutputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Tessellation evaluation stage) output interface variable (%s) "
                                 "exceeds component limit maxTessellationEvaluationOutputComponents (%" PRIu32 ").",
                                 max_output_slot.Describe().c_str(), limits.maxTessellationEvaluationOutputComponents);
            }
            // Portability validation
            if (IsExtEnabled(extensions.vk_khr_portability_subset)) {
                if (is_iso_lines && (VK_FALSE == enabled_features.tessellationIsolines)) {
                    skip |= LogError("VUID-RuntimeSpirv-tessellationShader-06326", module_state.handle(), loc,
                                     "(portability error) SPIR-V (Tessellation evaluation stage)"
                                     " is using abstract patch type IsoLines, but this is not supported on this platform.");
                }
                if (is_point_mode && (VK_FALSE == enabled_features.tessellationPointMode)) {
                    skip |= LogError("VUID-RuntimeSpirv-tessellationShader-06327", module_state.handle(), loc,
                                     "(portability error) SPIR-V (Tessellation evaluation stage)"
                                     " is using abstract patch type PointMode, but this is not supported on this platform.");
                }
            }
            break;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            if (total_input_components >= limits.maxGeometryInputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Geometry stage) input interface variable (%s) along with %" PRIu32
                                 " built-in components,  "
                                 "exceeds component limit maxGeometryInputComponents (%" PRIu32 ").",
                                 max_input_slot.Describe().c_str(), entrypoint.builtin_input_components,
                                 limits.maxGeometryInputComponents);
            }
            if (total_output_components >= limits.maxGeometryOutputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Geometry stage) output interface variable (%s) along with %" PRIu32
                                 " built-in components,  "
                                 "exceeds component limit maxGeometryOutputComponents (%" PRIu32 ").",
                                 max_output_slot.Describe().c_str(), entrypoint.builtin_output_components,
                                 limits.maxGeometryOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            if (total_input_components >= limits.maxFragmentInputComponents) {
                skip |= LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                                 "SPIR-V (Fragment stage) input interface variable (%s) along with %" PRIu32
                                 " built-in components,  "
                                 "exceeds component limit maxFragmentInputComponents (%" PRIu32 ").",
                                 max_input_slot.Describe().c_str(), entrypoint.builtin_input_components,
                                 limits.maxFragmentInputComponents);
            }

            // Fragment output doesn't have built ins
            // 1 Location == 1 color attachment
            if (max_output_slot.Location() >= limits.maxFragmentOutputAttachments) {
                skip |=
                    LogError("VUID-RuntimeSpirv-Location-06272", module_state.handle(), loc,
                             "SPIR-V (Fragment stage) output interface variable at Location %" PRIu32
                             " "
                             "exceeds the limit maxFragmentOutputAttachments (%" PRIu32 ") (note: Location are zero index based).",
                             max_output_slot.Location(), limits.maxFragmentOutputAttachments);
            }
            break;

        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            break;

        // Shader stage is an alias, but the ExecutionModel is not
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            if (entrypoint.execution_model == spv::ExecutionModelMeshNV) {
                if (num_vertices > phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputVertices) {
                    skip |= LogError("VUID-RuntimeSpirv-MeshNV-07113", module_state.handle(), loc,
                                     "SPIR-V (Mesh stage) output vertices count exceeds the "
                                     "maxMeshOutputVertices of %" PRIu32 " by %" PRIu32 ".",
                                     phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputVertices,
                                     num_vertices - phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputVertices);
                }
                if (num_primitives > phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputPrimitives) {
                    skip |= LogError("VUID-RuntimeSpirv-MeshNV-07114", module_state.handle(), loc,
                                     "SPIR-V (Mesh stage) output primitives count exceeds the "
                                     "maxMeshOutputPrimitives of %" PRIu32 " by %" PRIu32 ".",
                                     phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputPrimitives,
                                     num_primitives - phys_dev_ext_props.mesh_shader_props_nv.maxMeshOutputPrimitives);
                }
            } else if (entrypoint.execution_model == spv::ExecutionModelMeshEXT) {
                if (num_vertices > phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputVertices) {
                    skip |= LogError("VUID-RuntimeSpirv-MeshEXT-07115", module_state.handle(), loc,
                                     "SPIR-V (Mesh stage) output vertices count exceeds the "
                                     "maxMeshOutputVertices of %" PRIu32 " by %" PRIu32 ".",
                                     phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputVertices,
                                     num_vertices - phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputVertices);
                }
                if (num_primitives > phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputPrimitives) {
                    skip |= LogError("VUID-RuntimeSpirv-MeshEXT-07116", module_state.handle(), loc,
                                     "SPIR-V (Mesh stage) output primitives count exceeds the "
                                     "maxMeshOutputPrimitives of %" PRIu32 " by %" PRIu32 ".",
                                     phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputPrimitives,
                                     num_primitives - phys_dev_ext_props.mesh_shader_props_ext.maxMeshOutputPrimitives);
                }
            }
            break;

        default:
            assert(false);  // This should never happen
    }

    // maxFragmentCombinedOutputResources
    //
    // This limit was created from Vulkan 1.0, with the move to bindless, this limit has slowly become less relevant, if using
    // descriptor indexing, the limit should basically be UINT32_MAX
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT && !IsExtEnabled(extensions.vk_ext_descriptor_indexing)) {
        // Variables can be aliased, so use Location to mark things as unique
        vvl::unordered_set<uint32_t> color_attachments;
        for (const auto *variable : entrypoint.user_defined_interface_variables) {
            if (variable->storage_class == spv::StorageClassOutput && variable->decorations.location != spirv::kInvalidValue) {
                // even if using an array of attachments in the shader, each used variable of the array is represented by a single
                // variable
                color_attachments.insert(variable->decorations.location);
            }
        }

        // unordered_set requires to define hashing, and these should be very small and cheap as is
        std::set<std::pair<uint32_t, uint32_t>> storage_buffers;
        std::set<std::pair<uint32_t, uint32_t>> storage_images;
        for (const auto &variable : entrypoint.resource_interface_variables) {
            if (!variable.IsAccessed()) continue;
            if (variable.is_storage_buffer) {
                storage_buffers.insert(std::make_pair(variable.decorations.set, variable.decorations.binding));
            } else if (variable.is_storage_image || variable.is_storage_texel_buffer) {
                storage_images.insert(std::make_pair(variable.decorations.set, variable.decorations.binding));
            }
        }
        const uint32_t total_output = (uint32_t)(color_attachments.size() + storage_buffers.size() + storage_images.size());
        if (total_output > limits.maxFragmentCombinedOutputResources) {
            skip |= LogError("VUID-RuntimeSpirv-Location-06428", module_state.handle(), loc,
                             "SPIR-V (Fragment stage) output contains %zu storage buffer bindings, %zu storage image bindings, and "
                             "%zu color attachments which together is %" PRIu32
                             " which exceeds the limit maxFragmentCombinedOutputResources (%" PRIu32 ").",
                             storage_buffers.size(), storage_images.size(), color_attachments.size(), total_output,
                             limits.maxFragmentCombinedOutputResources);
        }
    }
    return skip;
}

bool SpirvValidator::ValidateShaderFloatControl(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;

    // Need to wrap otherwise phys_dev_props_core12 can be junk
    if (!IsExtEnabled(extensions.vk_khr_shader_float_controls)) {
        return skip;
    }

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_16) &&
        !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-06293", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 16 but it is not enabled on the device.");
    } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_32) &&
               !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 32 but it is not enabled on the device.");
    } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_64) &&
               !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-06295", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 64 but it is not enabled on the device.");
    }

    const bool has_denorm_preserve_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_16);
    const bool has_denorm_preserve_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_32);
    const bool has_denorm_preserve_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_64);
    if (has_denorm_preserve_width_16 && !phys_dev_props_core12.shaderDenormPreserveFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat16-06296", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 16 but it is not enabled on the device.");
    } else if (has_denorm_preserve_width_32 && !phys_dev_props_core12.shaderDenormPreserveFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 32 but it is not enabled on the device.");
    } else if (has_denorm_preserve_width_64 && !phys_dev_props_core12.shaderDenormPreserveFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat64-06298", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 64 but it is not enabled on the device.");
    }

    const bool has_denorm_flush_to_zero_width_16 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_16);
    const bool has_denorm_flush_to_zero_width_32 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_32);
    const bool has_denorm_flush_to_zero_width_64 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_64);
    if (has_denorm_flush_to_zero_width_16 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat16-06299", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 16 but it is not enabled on the device.");
    } else if (has_denorm_flush_to_zero_width_32 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 32 but it is not enabled on the device.");
    } else if (has_denorm_flush_to_zero_width_64 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat64-06301", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 64 but it is not enabled on the device.");
    }

    const bool has_rounding_mode_rte_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_16);
    const bool has_rounding_mode_rte_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_32);
    const bool has_rounding_mode_rte_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_64);
    if (has_rounding_mode_rte_width_16 && !phys_dev_props_core12.shaderRoundingModeRTEFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat16-06302", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 16 but it is not enabled on the device.");
    } else if (has_rounding_mode_rte_width_32 && !phys_dev_props_core12.shaderRoundingModeRTEFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 32 but it is not enabled on the device.");
    } else if (has_rounding_mode_rte_width_64 && !phys_dev_props_core12.shaderRoundingModeRTEFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat64-06304", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 64 but it is not enabled on the device.");
    }

    const bool has_rounding_mode_rtz_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_16);
    const bool has_rounding_mode_rtz_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_32);
    const bool has_rounding_mode_rtz_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_64);
    if (has_rounding_mode_rtz_width_16 && !phys_dev_props_core12.shaderRoundingModeRTZFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat16-06305", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 16 but it is not enabled on the device.");
    } else if (has_rounding_mode_rtz_width_32 && !phys_dev_props_core12.shaderRoundingModeRTZFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 32 but it is not enabled on the device.");
    } else if (has_rounding_mode_rtz_width_64 && !phys_dev_props_core12.shaderRoundingModeRTZFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat64-06307", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 64 but it is not enabled on the device.");
    }

    auto get_float_width = [&module_state](uint32_t id) {
        const auto *insn = module_state.FindDef(id);
        if (!insn || insn->Opcode() != spv::OpTypeFloat) {
            return 0u;
        } else {
            return insn->Word(2);
        }
    };

    const uint32_t mask = spv::FPFastMathModeNotNaNMask | spv::FPFastMathModeNotInfMask | spv::FPFastMathModeNSZMask;
    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::fp_fast_math_default)) {
        for (const spirv::Instruction *insn : stateless_data.execution_mode_id_inst) {
            const uint32_t mode = insn->Word(2);
            if (mode != spv::ExecutionModeFPFastMathDefault) {
                continue;
            }

            // spirv-val will catch if this is not a constant
            const auto *fast_math_mode = module_state.FindDef(insn->Word(4));
            const bool has_mask = (fast_math_mode->GetConstantValue() & mask) == mask;
            if (has_mask) {
                continue;  // nothing to validate
            }

            const uint32_t bit_width = get_float_width(insn->Word(3));

            if (bit_width == 16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-09559", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat16 is false, but FPFastMathDefault is setting 16-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            } else if (bit_width == 32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09561", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat32 is false, but FPFastMathDefault is setting 32-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            } else if (bit_width == 64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-09563", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat64 is false, but FPFastMathDefault is setting 64-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            }
        }
    }

    for (const spirv::Instruction *insn : module_state.static_data_.decoration_inst) {
        uint32_t decoration = insn->Word(2);
        if (decoration != spv::DecorationFPFastMathMode) {
            continue;
        }
        uint32_t modes = insn->Word(3);
        const bool has_mask = (modes & mask) == mask;
        if (has_mask) {
            continue;  // nothing to validate
        }

        // See if target instruction uses any floats of various bit widths
        bool float_16 = false;
        bool float_32 = false;
        bool float_64 = false;
        uint32_t operand_index = 2;  // if using OpDecoration, this instruction must have a ResultId

        const auto *target_insn = module_state.FindDef(insn->Word(1));
        if (target_insn->TypeId() != 0) {
            operand_index++;
            const uint32_t bit_width = get_float_width(target_insn->TypeId());
            float_16 |= bit_width == 16;
            float_32 |= bit_width == 32;
            float_64 |= bit_width == 64;
        }

        for (; operand_index < target_insn->Length(); operand_index++) {
            const uint32_t bit_width = get_float_width(target_insn->Word(operand_index));
            float_16 |= bit_width == 16;
            float_32 |= bit_width == 32;
            float_64 |= bit_width == 64;
        }

        if (float_16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-09560", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat16 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 16-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        } else if (float_32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09562", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat32 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 32-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        } else if (float_64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-09564", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat64 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 64-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        }
    }

    return skip;
}

bool SpirvValidator::ValidateExecutionModes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                            const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;
    const VkShaderStageFlagBits stage = entrypoint.stage;

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::local_size_id_bit)) {
        // Special case to print error by extension and feature bit
        if (!enabled_features.maintenance4) {
            skip |= LogError("VUID-RuntimeSpirv-LocalSizeId-06434", module_state.handle(), loc,
                             "SPIR-V OpExecutionMode LocalSizeId is used but maintenance4 feature was not enabled.");
        }
        if (!IsExtEnabled(extensions.vk_khr_maintenance4)) {
            skip |= LogError("VUID-RuntimeSpirv-LocalSizeId-06434", module_state.handle(), loc,
                             "SPIR-V OpExecutionMode LocalSizeId is used but maintenance4 extension is not enabled and used "
                             "Vulkan api version is 1.2 or less.");
        }
    }

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::subgroup_uniform_control_flow_bit)) {
        if (!enabled_features.shaderSubgroupUniformControlFlow ||
            (phys_dev_ext_props.subgroup_props.supportedStages & stage) == 0 || stateless_data.has_invocation_repack_instruction) {
            std::stringstream msg;
            if (!enabled_features.shaderSubgroupUniformControlFlow) {
                msg << "shaderSubgroupUniformControlFlow feature must be enabled";
            } else if ((phys_dev_ext_props.subgroup_props.supportedStages & stage) == 0) {
                msg << "stage" << string_VkShaderStageFlagBits(stage)
                    << " must be in VkPhysicalDeviceSubgroupProperties::supportedStages("
                    << string_VkShaderStageFlags(phys_dev_ext_props.subgroup_props.supportedStages) << ")";
            } else {
                msg << "the shader must not use any invocation repack instructions";
            }
            skip |= LogError("VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379", module_state.handle(), loc,
                             "SPIR-V uses ExecutionModeSubgroupUniformControlFlowKHR, but %s.", msg.str().c_str());
        }
    }

    if ((stage == VK_SHADER_STAGE_MESH_BIT_EXT || stage == VK_SHADER_STAGE_TASK_BIT_EXT) &&
        !phys_dev_ext_props.compute_shader_derivatives_props.meshAndTaskShaderDerivatives) {
        if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::derivative_group_linear)) {
            skip |=
                LogError("VUID-RuntimeSpirv-meshAndTaskShaderDerivatives-10153", module_state.handle(), loc,
                         "SPIR-V uses DerivativeGroupLinearKHR in a %s shader, but meshAndTaskShaderDerivatives is not supported.",
                         string_VkShaderStageFlagBits(stage));
        } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::derivative_group_quads)) {
            skip |=
                LogError("VUID-RuntimeSpirv-meshAndTaskShaderDerivatives-10153", module_state.handle(), loc,
                         "SPIR-V uses DerivativeGroupQuadsKHR in a %s shader, but meshAndTaskShaderDerivatives is not supported.",
                         string_VkShaderStageFlagBits(stage));
        }
    }

    return skip;
}

bool SpirvValidator::ValidateConservativeRasterization(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                       const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;

    // only new to validate if property is not enabled
    if (phys_dev_ext_props.conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        return skip;
    }

    if (stateless_data.has_builtin_fully_covered &&
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::post_depth_coverage_bit)) {
        skip |= LogError("VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235", module_state.handle(), loc,
                         "SPIR-V (Fragment stage) has a\nOpExecutionMode EarlyFragmentTests\nOpDecorate BuiltIn "
                         "FullyCoveredEXT\nbut conservativeRasterizationPostDepthCoverage was not enabled.");
    }

    return skip;
}

}  // namespace stateless
