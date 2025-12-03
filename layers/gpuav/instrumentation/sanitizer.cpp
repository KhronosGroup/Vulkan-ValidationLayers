/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "generated/spirv_grammar_helper.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

namespace gpuav {

static std::string GetSpirvSpecLink(const uint32_t opcode) {
    // Currently the Working Group decided to not provide "real" VUIDs as it would become duplicating the SPIR-V spec
    // So these are not "UNASSIGNED", but instead are "SPIRV" VUs because we can point to the instruction in the SPIR-V spec
    // (https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7853)
    return "\nSee more at https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#" + std::string(string_SpvOpcode(opcode));
}

void RegisterSanitizer(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.sanitizer) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator &gpuav, CommandBufferSubState &cb,
                                                                          const LastBound &last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [](Validator &gpuav, const Location &loc,
                                                                                 const uint32_t *error_record,
                                                                                 std::string &out_error_msg,
                                                                                 std::string &out_vuid_msg) {
            using namespace glsl;
            bool error_found = false;
            if (GetErrorGroup(error_record) != kErrorGroupInstSanitizer) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;

            const uint32_t error_sub_code = GetSubError(error_record);
            switch (error_sub_code) {
                case kErrorSubCodeSanitizerDivideZero: {
                    const uint32_t opcode = error_record[kInstLogErrorParameterOffset_0];
                    const uint32_t vector_size = error_record[kInstLogErrorParameterOffset_1];
                    const bool is_float = opcode == spv::OpFMod || opcode == spv::OpFRem;
                    strm << (is_float ? "Float" : "Integer") << " divide by zero. Operand 2 of " << string_SpvOpcode(opcode)
                         << " is ";
                    if (vector_size == 0) {
                        strm << "zero.";
                    } else {
                        strm << "a " << vector_size << "-wide vector which contains a zero value.";
                    }
                    if (is_float) {
                        strm << " The result value is undefined.";
                    }
                    strm << GetSpirvSpecLink(opcode);
                    out_vuid_msg = "SPIRV-Sanitizer-Divide-By-Zero";
                } break;
                case kErrorSubCodeSanitizerImageGather: {
                    const uint32_t component_value = error_record[kInstLogErrorParameterOffset_0];
                    const int32_t signed_value = (int32_t)component_value;
                    strm << "OpImageGather has a component value of ";
                    if (signed_value > 0) {
                        strm << component_value;
                    } else {
                        strm << signed_value;
                    }
                    strm << ", but it must be 0, 1, 2, or 3" << GetSpirvSpecLink(spv::OpImageGather);
                    out_vuid_msg = "SPIRV-Sanitizer-Image-Gather";
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
