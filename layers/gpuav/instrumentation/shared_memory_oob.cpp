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

#include "generated/spirv_grammar_helper.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "error_message/spirv_logging.h"

namespace gpuav {

void RegisterSharedMemoryOobValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.shared_memory_oob) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [](Validator& gpuav, const Location& loc, const uint32_t* error_record, const InstrumentedShader* instrumented_shader,
               std::string& out_error_msg, std::string& out_vuid_msg) {
                using namespace glsl;
                bool error_found = false;
                if (GetErrorGroup(error_record) != kErrorGroup_SharedMemoryOob) {
                    return error_found;
                }
                error_found = true;

                const uint32_t index = error_record[kInst_LogError_ParameterOffset_0];
                const uint32_t encoded_bound = error_record[kInst_LogError_ParameterOffset_1];
                const uint32_t variable_id = error_record[kInst_LogError_ParameterOffset_2];

                const uint32_t bound = encoded_bound & 0x00FFFFFFu;
                const uint32_t access_type = (encoded_bound >> 24) & 0x3u;
                const uint32_t dim_index = (encoded_bound >> 26) & 0x1Fu;

                const uint32_t instruction_position_offset = error_record[kHeader_StageInstructionIdOffset] & kInstructionId_Mask;

                std::ostringstream strm;
                strm << "Shared memory variable \"";
                if (instrumented_shader) {
                    ::spirv::FindGlobalName(strm, instrumented_shader->original_spirv, (uint32_t)spv::OpVariable, variable_id);
                } else {
                    strm << "[error, original SPIR-V not found]";
                }
                strm << "\" ";
                if (access_type == 1) {
                    strm << "vector component";
                } else {
                    strm << "array index";
                    if (dim_index > 0) {
                        strm << " (dimension " << dim_index << ")";
                    }
                }
                strm << " " << index << " is >= " << ((access_type == 1) ? "vector size " : "array size ") << bound << ".";

                uint32_t opcode = (uint32_t)spv::OpMax;
                if (instrumented_shader) {
                    opcode = ::spirv::GetOpcodeAtOffset(instrumented_shader->original_spirv, instruction_position_offset);
                }
                strm << GetSpirvSpecLink(opcode);

                out_vuid_msg = std::string("SPIRV-SharedMemoryOob-") + string_SpvOpcode(opcode);

                out_error_msg += strm.str();
                return error_found;
            };

        return inst_error_logger;
    });
}

}  // namespace gpuav
