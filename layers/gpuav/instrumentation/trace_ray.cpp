/* Copyright (c) 2024-2026 LunarG, Inc.
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

void RegisterTraceRayValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.trace_ray) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, const LastBound& last_bound) {
            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [](Validator& gpuav, const Location& loc, const uint32_t* error_record, const InstrumentedShader*,
                   std::string& out_error_msg, std::string& out_vuid_msg) {
                    using namespace glsl;
                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroup_TraceRay) {
                        return error_found;
                    }
                    error_found = true;

                    std::ostringstream strm;

                    const uint32_t error_sub_code = GetSubError(error_record);

                    switch (error_sub_code) {
                        case kErrorSubCode_TraceRay_TrianglesFlags: {
                            const uint32_t ray_flags = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "OpTraceRayKHR operand Ray Flags (" << string_SpvRayFlagsMask(ray_flags)
                                 << ") form an invalid combination of mutually exclusive flags.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06892";
                        } break;
                        case kErrorSubCode_TraceRay_OpaqueFlags: {
                            const uint32_t ray_flags = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "OpTraceRayKHR operand Ray Flags (" << string_SpvRayFlagsMask(ray_flags)
                                 << ") form an invalid combination of mutually exclusive flags.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06893";
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
