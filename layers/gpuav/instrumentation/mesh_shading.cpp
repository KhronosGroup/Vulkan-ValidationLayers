/* Copyright (c) 2025 LunarG, Inc.
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

void RegisterMeshShadingValidation(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.mesh_shading) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator &gpuav, CommandBufferSubState &cb, const LastBound &last_bound) {
            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [](Validator &gpuav, const Location &loc, const uint32_t *error_record, std::string &out_error_msg,
                   std::string &out_vuid_msg) {
                    using namespace glsl;
                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroupInstMeshShading) {
                        return error_found;
                    }
                    error_found = true;

                    std::ostringstream strm;

                    const uint32_t error_sub_code = GetSubError(error_record);
                    switch (error_sub_code) {
                        case kErrorSubCode_MeshShading_SetMeshOutputs: {
                            const uint32_t vertex_count = error_record[kInstLogErrorParameterOffset_0];
                            const uint32_t primitive_count = error_record[kInstLogErrorParameterOffset_1];
                            const uint32_t encoded_output = error_record[kInstLogErrorParameterOffset_2];
                            const uint32_t output_vertices = encoded_output >> kMeshShadingOutputVerticesShift;
                            const uint32_t output_primitive = encoded_output & kMeshShadingOutputPrimitivesMask;

                            // We normally don't do "logic" to determine the VU, but in this case, its valuable to print both value,
                            // so we already have all the information and easier to select the VUID here than via the shader
                            // interface
                            out_vuid_msg = (vertex_count > output_vertices) ? "VUID-RuntimeSpirv-MeshEXT-07332"
                                                                            : "VUID-RuntimeSpirv-MeshEXT-07333";

                            strm << "OpSetMeshOutputsEXT set the Vertex Count to " << vertex_count << " and Primitive Count to "
                                 << primitive_count << ", but they must be less than the OutputVertices (" << output_vertices
                                 << ") and OutputPrimitivesEXT (" << output_primitive << ").";
                            // TODO - Use OpSource to detect this for the user
                            strm << "\n  For GLSL these values are set via \"layout(max_vertices = " << output_vertices
                                 << ") out;\" and "
                                    "\"layout(max_primitives = "
                                 << output_primitive
                                 << ") out;\"\n  For HLSL/Slang these values are set via \"void main(out "
                                    "indices uint3 primitives["
                                 << output_primitive << "], out vertices MeshOutput vertices[" << output_vertices << "])\"";
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
