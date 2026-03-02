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
#include "state_tracker/shader_module.h"
#include "error_message/spirv_logging.h"

namespace gpuav {

void RegisterSharedMemoryDataRaceValidation(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.shared_memory_data_race) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator &gpuav, CommandBufferSubState &cb,
                                                                          const LastBound &last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [&last_bound](Validator &gpuav, const Location &loc,
                                                                                            const uint32_t *error_record,
                                                                                            std::string &out_error_msg,
                                                                                            std::string &out_vuid_msg) {
            using namespace glsl;
            bool error_found = false;
            if (GetErrorGroup(error_record) != kErrorGroup_SharedMemoryDataRace) {
                return error_found;
            }
            error_found = true;

            // TODO - We shouldn't need to grab the core Module object here...
            // The issue is we don't get the raw SPIR-V until LogInstrumentationError;
            auto &module = last_bound.pipeline_state
                               ? last_bound.pipeline_state->GetShaderStageState(VK_SHADER_STAGE_COMPUTE_BIT)->spirv_state
                               : last_bound.GetShaderObjectState(ShaderObjectStage::COMPUTE)->stage.spirv_state;

            const uint32_t thread_id = error_record[kInst_LogError_ParameterOffset_0];
            const uint32_t collide_id = error_record[kInst_LogError_ParameterOffset_1] & 0xFFFF;
            uint32_t variable_id = error_record[kInst_LogError_ParameterOffset_2];

            const uint32_t error_sub_code = GetSubError(error_record);
            std::ostringstream strm;
            strm << "A data race was detected on the shared memory variable \"";
            ::spirv::FindOpVariableName(strm, module->words_, variable_id);
            strm << "\" in local invocation index " << thread_id << " while performing a ";
            switch (error_sub_code) {
                case kErrorSubCode_SharedMemoryDataRace_RaceOnStore: {
                    strm << "store";
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnStore";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnLoad: {
                    strm << "load";
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnLoad";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnLoadStoreVsAtomic: {
                    strm << "load or store";
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnLoadStoreVsAtomic";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnAtomic: {
                    strm << "atomic";
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnAtomic";
                } break;
                default:
                    strm << "UNKNOWN";
                    error_found = false;
                    break;
            }
            strm << " operation. (Likely against local invocation index " << collide_id << ")";

            out_error_msg += strm.str();
            return error_found;
        };

        return inst_error_logger;
    });
}

}  // namespace gpuav
