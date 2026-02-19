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
#include "state_tracker/shader_module.h"

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

            std::ostringstream strm;

            auto &module = last_bound.pipeline_state
                               ? last_bound.pipeline_state->GetShaderStageState(VK_SHADER_STAGE_COMPUTE_BIT)->spirv_state
                               : last_bound.GetShaderObjectState(ShaderObjectStage::COMPUTE)->stage.spirv_state;

            auto &shared_memory_variables = module->static_data_.shared_memory_variables;

            uint32_t variable_idx = error_record[kInst_LogError_ParameterOffset_2];

            assert(variable_idx < shared_memory_variables.size());
            auto &var = shared_memory_variables[variable_idx];
            std::string name = module->GetName(var->Word(2));

            const uint32_t error_sub_code = GetSubError(error_record);

            switch (error_sub_code) {
                case kErrorSubCode_SharedMemoryDataRace_RaceOnStore: {
                    strm << "Shared memory race detected on variable named \"" << name
                         << "\" when performing store in local invocation index " << error_record[kInst_LogError_ParameterOffset_0]
                         << ", likely against local invocation index " << (error_record[kInst_LogError_ParameterOffset_1] & 0xFFFF);
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnStore";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnLoad: {
                    strm << "Shared memory race detected on variable named \"" << name
                         << "\" when performing load in local invocation index " << error_record[kInst_LogError_ParameterOffset_0]
                         << ", likely against local invocation index " << (error_record[kInst_LogError_ParameterOffset_1] & 0xFFFF);
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnLoad";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnLoadStoreVsAtomic: {
                    strm << "Shared memory race detected on variable named \"" << name
                         << "\" when performing load or store in local invocation index "
                         << error_record[kInst_LogError_ParameterOffset_0] << ", against an atomic in another thread";
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnLoadStoreVsAtomic";
                } break;
                case kErrorSubCode_SharedMemoryDataRace_RaceOnAtomic: {
                    strm << "Shared memory race detected on variable named \"" << name
                         << "\" when performing atomic in local invocation index " << error_record[kInst_LogError_ParameterOffset_0]
                         << ", likely against local invocation index " << (error_record[kInst_LogError_ParameterOffset_1] & 0xFFFF);
                    out_vuid_msg = "SharedMemoryDataRace-RaceOnAtomic";
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
