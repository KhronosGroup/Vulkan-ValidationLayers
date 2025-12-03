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

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

namespace gpuav {

void RegisterRayQueryValidation(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.ray_query) {
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
            if (GetErrorGroup(error_record) != kErrorGroupInstRayQuery) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;

            const uint32_t error_sub_code = GetSubError(error_record);
            switch (error_sub_code) {
                case kErrorSubCodeRayQueryNegativeMin: {
                    // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
                    strm << "OpRayQueryInitializeKHR operand Ray Tmin value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
                } break;
                case kErrorSubCodeRayQueryNegativeMax: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
                } break;
                case kErrorSubCodeRayQueryMinMax: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax is less than RayTmin. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350";
                } break;
                case kErrorSubCodeRayQueryMinNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmin is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCodeRayQueryMaxNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCodeRayQueryOriginNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Origin contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCodeRayQueryDirectionNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Direction contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCodeRayQueryOriginFinite: {
                    strm << "OpRayQueryInitializeKHR operand Ray Origin contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
                } break;
                case kErrorSubCodeRayQueryDirectionFinite: {
                    strm << "OpRayQueryInitializeKHR operand Ray Direction contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
                } break;
                case kErrorSubCodeRayQueryBothSkip: {
                    const uint32_t value = error_record[kInstRayQueryParamOffset_0];
                    strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889";
                } break;
                case kErrorSubCodeRayQuerySkipCull: {
                    const uint32_t value = error_record[kInstRayQueryParamOffset_0];
                    strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06890";
                } break;
                case kErrorSubCodeRayQueryOpaque: {
                    const uint32_t value = error_record[kInstRayQueryParamOffset_0];
                    strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891";
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
