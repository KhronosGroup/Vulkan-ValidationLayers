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
            if (GetErrorGroup(error_record) != kErrorGroup_InstRayQuery) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;

            const uint32_t error_sub_code = GetSubError(error_record);

            switch (error_sub_code) {
                case kErrorSubCode_RayQuery_NegativeMin: {
                    // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
                    strm << "OpRayQueryInitializeKHR operand Ray Tmin value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
                } break;
                case kErrorSubCode_RayQuery_NegativeMax: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
                } break;
                case kErrorSubCode_RayQuery_MinMax: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax is less than RayTmin. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350";
                } break;
                case kErrorSubCode_RayQuery_MinNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmin is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCode_RayQuery_MaxNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Tmax is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCode_RayQuery_OriginNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Origin contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCode_RayQuery_DirectionNaN: {
                    strm << "OpRayQueryInitializeKHR operand Ray Direction contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
                } break;
                case kErrorSubCode_RayQuery_OriginFinite: {
                    strm << "OpRayQueryInitializeKHR operand Ray Origin contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
                } break;
                case kErrorSubCode_RayQuery_DirectionFinite: {
                    strm << "OpRayQueryInitializeKHR operand Ray Direction contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
                } break;
                case kErrorSubCode_RayQuery_BothSkip: {
                    const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                    strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889";
                } break;
                case kErrorSubCode_RayQuery_SkipCull: {
                    const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                    strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06890";
                } break;
                case kErrorSubCode_RayQuery_Opaque: {
                    const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
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
