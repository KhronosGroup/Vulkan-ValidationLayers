/* Copyright (c) 2026 LunarG, Inc.
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

#include <spirv/unified1/spirv.hpp>
#include "generated/spirv_grammar_helper.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

namespace gpuav {

void RegisterPoisonValueValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.poison_value) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, const LastBound& last_bound) {
            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [](Validator& gpuav, const Location& loc, const uint32_t* error_record,
                   const InstrumentedShader* instrumented_shader, std::string& out_error_msg, std::string& out_vuid_msg) {
                    using namespace glsl;
                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroup_InstPoisonValue) {
                        return error_found;
                    }
                    error_found = true;

                    const uint32_t error_sub_code = GetSubError(error_record);
                    std::ostringstream strm;

                    switch (error_sub_code) {
                        case kErrorSubCode_PoisonValue_UninitializedVariable: {
                            const uint32_t trigger_opcode = error_record[kInst_LogError_ParameterOffset_0];

                            if (trigger_opcode == spv::OpBranchConditional || trigger_opcode == spv::OpSwitch) {
                                strm << "Undefined behavior: poison value used in " << string_SpvOpcode(trigger_opcode) << ".";
                                strm << " A poison value was used as a branch condition.";
                                out_vuid_msg = "SPIRV-PoisonValue-BranchOnPoison";
                            } else if (trigger_opcode == spv::OpAccessChain || trigger_opcode == spv::OpInBoundsAccessChain ||
                                       trigger_opcode == spv::OpPtrAccessChain) {
                                strm << "Undefined behavior: poison value used in " << string_SpvOpcode(trigger_opcode) << ".";
                                strm << " A poison value was used as an index in an access chain.";
                                out_vuid_msg = "SPIRV-PoisonValue-AddressFromPoison";
                            } else if (trigger_opcode == spv::OpStore) {
                                strm << "Warning: poison value stored to an externally-visible variable"
                                     << " (not undefined behavior per the SPIR-V spec, but likely an application bug).";
                                out_vuid_msg = "SPIRV-PoisonValue-ExternalStoreOfPoison";
                            } else if (trigger_opcode == spv::OpReturnValue) {
                                strm << "Warning: poison value returned from a function"
                                     << " (not undefined behavior per the SPIR-V spec, but likely an application bug).";
                                out_vuid_msg = "SPIRV-PoisonValue-ReturnOfPoison";
                            } else {
                                assert(false && "unexpected opcode for poison value error");
                                out_vuid_msg = "SPIRV-PoisonValue-UninitializedVariable";
                            }
                        } break;
                        case kErrorSubCode_PoisonValue_StoreToFunctionParam: {
                            strm << "Warning: poison value stored to a function out/inout parameter"
                                 << " (not undefined behavior per the SPIR-V spec, but likely an application bug).";
                            out_vuid_msg = "SPIRV-PoisonValue-StoreToFunctionParam";
                        } break;
                        case kErrorSubCode_PoisonValue_PoisonPointerDereference: {
                            const uint32_t trigger_opcode = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "Undefined behavior: dereference of a poison pointer in " << string_SpvOpcode(trigger_opcode)
                                 << ".";
                            out_vuid_msg = "SPIRV-PoisonValue-PoisonPointerDereference";
                        } break;
                        default:
                            error_found = false;
                            break;
                    }

                    out_error_msg += strm.str();
                    return error_found;
                };

            return inst_error_logger;
        });
}

}  // namespace gpuav
