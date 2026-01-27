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

void RegisterRayHitObjectValidation(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.ray_hit_object) {
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
            if (GetErrorGroup(error_record) != kErrorGroupInstRayHitObject) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;

            const uint32_t error_sub_code = GetSubError(error_record);
            const uint32_t opcode = error_record[kInstLogErrorParameterOffset_1];

            switch (error_sub_code) {
                case kErrorSubCodeRayHitObjectNegativeMin: {
                    // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
                    strm << string_SpvOpcode(opcode) << " operand Ray Tmin value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879";
                } break;
                case kErrorSubCodeRayHitObjectNegativeMax: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Tmax value is negative. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879";
                } break;
                case kErrorSubCodeRayHitObjectMinMax: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Tmax is less than RayTmin. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11880";
                } break;
                case kErrorSubCodeRayHitObjectMinNaN: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Tmin is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                } break;
                case kErrorSubCodeRayHitObjectMaxNaN: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Tmax is NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                } break;
                case kErrorSubCodeRayHitObjectOriginNaN: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Origin contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                } break;
                case kErrorSubCodeRayHitObjectDirectionNaN: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Direction contains a NaN. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                } break;
                case kErrorSubCodeRayHitObjectOriginFinite: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Origin contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11878";
                } break;
                case kErrorSubCodeRayHitObjectDirectionFinite: {
                    strm << string_SpvOpcode(opcode) << " operand Ray Direction contains a non-finite value. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11878";
                } break;
                case kErrorSubCodeRayHitObjectBothSkip: {
                    const uint32_t value = error_record[kInstLogErrorParameterOffset_0];
                    strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11883";
                } break;
                case kErrorSubCodeRayHitObjectSkipCull: {
                    const uint32_t value = error_record[kInstLogErrorParameterOffset_0];
                    strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11884";
                } break;
                case kErrorSubCodeRayHitObjectOpaque: {
                    const uint32_t value = error_record[kInstLogErrorParameterOffset_0];
                    strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11885";
                } break;
                case kErrorSubCodeRayHitObjectSkipTrianglesWithPipelineSkipAABBs: {
                    const uint32_t value = error_record[kInstLogErrorParameterOffset_0];
                    strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                         << ") contains SkipTrianglesKHR, but pipeline was created with "
                            "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11886";
                } break;
                case kErrorSubCodeRayHitObjectSkipAABBsWithPipelineSkipTriangles: {
                    const uint32_t value = error_record[kInstLogErrorParameterOffset_0];
                    strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                         << ") contains SkipAABBsKHR, but pipeline was created with "
                            "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11887";
                } break;
                case kErrorSubCodeRayHitObjectTimeOutOfRange: {
                    strm << string_SpvOpcode(opcode) << " operand time is not between 0.0 and 1.0. ";
                    out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11882";
                } break;
                case kErrorSubCodeRayHitObjectSBTIndexExceedsLimit: {
                    // For this case, param_0 contains the SBT index and opcode_type slot contains the max SBT index
                    const uint32_t sbt_index = error_record[kInstLogErrorParameterOffset_0];
                    const uint32_t max_sbt_index = error_record[kInstLogErrorParameterOffset_1];
                    strm << "OpHitObjectSetShaderBindingTableRecordIndexEXT SBT index (" << std::dec << sbt_index
                         << ") exceeds VkPhysicalDeviceRayTracingInvocationReorderPropertiesEXT::maxShaderBindingTableRecordIndex (" << max_sbt_index << "). ";
                    out_vuid_msg = "VUID-RuntimeSpirv-maxShaderBindingTableRecordIndex-11888";
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
