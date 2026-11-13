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

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

namespace gpuav {

void RegisterExpectAssume(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.sanitizer) {
        // currently sharing setting with sanitizer
        // really just as a mechanism to toggle this off if needed
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, const LastBound& last_bound) {
            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [](Validator& gpuav, const Location& loc, const uint32_t* error_record, const InstrumentedShader*,
                   std::string& out_error_msg, std::string& out_vuid_msg) {
                    using namespace glsl;
                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroup_InstExpectAssume) {
                        return error_found;
                    }
                    error_found = true;

                    std::ostringstream ss;

                    const uint32_t error_sub_code = GetSubError(error_record);
                    switch (error_sub_code) {
                        case kErrorSubCode_ExpectAssume_AssumeTrue: {
                            ss << "OpAssumeTrueKHR condition was false, which is undefined behavior as the driver might have "
                                  "compiled the shader relying on the condition being true\nNote: GPU-AV replace the "
                                  "OpAssumeTrueKHR instruciton, so the shader may behave differently with it on.\nSee more at "
                                  "https://github.khronos.org/SPIRV-Registry/extensions/KHR/SPV_KHR_expect_assume.html";
                            out_vuid_msg = "SPIRV-ExpectAssume-AssumeTrue";
                        } break;
                        default:
                            error_found = false;
                            break;
                    }
                    out_error_msg += ss.str();
                    return error_found;
                };

            return inst_error_logger;
        });
}

}  // namespace gpuav
