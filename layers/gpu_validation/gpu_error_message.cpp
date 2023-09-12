/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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

#include "gpu_validation/gpu_error_message.h"
#include "spirv-tools/instrument.hpp"
#include "state_tracker/shader_module.h"

#include <algorithm>
#include <regex>

// Generate the stage-specific part of the message.
void UtilGenerateStageMessage(const uint32_t *debug_record, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstCommonOutStageIdx]) {
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << debug_record[kInstVertOutVertexIndex]
                 << " Instance Index = " << debug_record[kInstVertOutInstanceIndex] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << debug_record[kInstTessCtlOutInvocationId]
                 << ", Primitive ID = " << debug_record[kInstTessCtlOutPrimitiveId];
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Primitive ID = " << debug_record[kInstTessEvalOutPrimitiveId]
                 << ", TessCoord (u, v) = (" << debug_record[kInstTessEvalOutTessCoordU] << ", "
                 << debug_record[kInstTessEvalOutTessCoordV] << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << debug_record[kInstGeomOutPrimitiveId]
                 << " Invocation ID = " << debug_record[kInstGeomOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordX]) << ", "
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordY]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID (x, y, z) = (" << debug_record[kInstCompOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstCompOutGlobalInvocationIdY] << ", " << debug_record[kInstCompOutGlobalInvocationIdZ] << " )";
        } break;
        case spv::ExecutionModelRayGenerationNV: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelIntersectionNV: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelAnyHitNV: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelClosestHitNV: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelMissNV: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelCallableNV: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelTaskNV: {
            strm << "Stage = Task. Global invocation ID (x, y, z) = (" << debug_record[kInstTaskOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstTaskOutGlobalInvocationIdY] << ", " << debug_record[kInstTaskOutGlobalInvocationIdZ] << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            strm << "Stage = Mesh.Global invocation ID (x, y, z) = (" << debug_record[kInstMeshOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstMeshOutGlobalInvocationIdY] << ", " << debug_record[kInstMeshOutGlobalInvocationIdZ] << " )";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << debug_record[kInstCommonOutStageIdx] << "). ";
            assert(false);
        } break;
    }
    msg = strm.str();
}

static std::string LookupDebugUtilsName(const debug_report_data *report_data, const uint64_t object) {
    auto object_label = report_data->DebugReportGetUtilsObjectName(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
void UtilGenerateCommonMessage(const debug_report_data *report_data, const VkCommandBuffer commandBuffer,
                               const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                               const VkPipeline pipeline_handle, const VkShaderEXT shader_object_handle,
                               const VkPipelineBindPoint pipeline_bind_point, const uint32_t operation_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE && shader_object_handle == VK_NULL_HANDLE) {
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer) << "). ";
        assert(true);
    } else {
        strm << std::hex << std::showbase << "Command buffer " << LookupDebugUtilsName(report_data, HandleToUint64(commandBuffer))
             << "(" << HandleToUint64(commandBuffer) << "). ";
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            strm << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            strm << "Compute Dispatch ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            strm << "Ray Trace ";
        } else {
            assert(false);
            strm << "Unknown Pipeline Operation ";
        }
        if (shader_module_handle) {
            strm << "Index " << operation_index << ". "
                 << "Pipeline " << LookupDebugUtilsName(report_data, HandleToUint64(pipeline_handle)) << "("
                 << HandleToUint64(pipeline_handle) << "). "
                 << "Shader Module " << LookupDebugUtilsName(report_data, HandleToUint64(shader_module_handle)) << "("
                 << HandleToUint64(shader_module_handle) << "). ";
        } else {
            strm << "Index " << operation_index << ". "
                 << "Shader Object " << LookupDebugUtilsName(report_data, HandleToUint64(shader_object_handle)) << "("
                 << HandleToUint64(shader_object_handle) << "). ";
        }
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[kInstCommonOutInstructionIdx] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const SPIRV_MODULE_STATE &module_state, const uint32_t reported_file_id,
                         std::vector<std::string> &opsource_lines) {
    const std::vector<Instruction> &instructions = module_state.GetInstructions();
    for (size_t i = 0; i < instructions.size(); i++) {
        const Instruction &insn = instructions[i];
        if ((insn.Opcode() == spv::OpSource) && (insn.Length() >= 5) && (insn.Word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str(insn.GetAsString(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }

            for (size_t k = i + 1; k < instructions.size(); k++) {
                const Instruction &continue_insn = instructions[k];
                if (continue_insn.Opcode() != spv::OpSourceContinued) {
                    break;
                }
                in_stream.str(continue_insn.GetAsString(1));
                while (std::getline(in_stream, cur_line)) {
                    opsource_lines.push_back(cur_line);
                }
            }
            break;
        }
    }
}

// The task here is to search the OpSource content to find the #line directive with the
// line number that is closest to, but still prior to the reported error line number and
// still within the reported filename.
// From this known position in the OpSource content we can add the difference between
// the #line line number and the reported error line number to determine the location
// in the OpSource content of the reported error line.
//
// Considerations:
// - Look only at #line directives that specify the reported_filename since
//   the reported error line number refers to its location in the reported filename.
// - If a #line directive does not have a filename, the file is the reported filename, or
//   the filename found in a prior #line directive.  (This is C-preprocessor behavior)
// - It is possible (e.g., inlining) for blocks of code to get shuffled out of their
//   original order and the #line directives are used to keep the numbering correct.  This
//   is why we need to examine the entire contents of the source, instead of leaving early
//   when finding a #line line number larger than the reported error line number.
//
static bool GetLineAndFilename(const std::string &string, uint32_t *linenumber, std::string &filename) {
    static const std::regex line_regex(  // matches #line directives
        "^"                              // beginning of line
        "\\s*"                           // optional whitespace
        "#"                              // required text
        "\\s*"                           // optional whitespace
        "line"                           // required text
        "\\s+"                           // required whitespace
        "([0-9]+)"                       // required first capture - line number
        "(\\s+)?"                        // optional second capture - whitespace
        "(\".+\")?"                      // optional third capture - quoted filename with at least one char inside
        ".*");                           // rest of line (needed when using std::regex_match since the entire line is tested)

    std::smatch captures;

    const bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = (uint32_t)std::stoul(captures[1]);
    return true;
}

// Extract the filename, line number, and column number from the correct OpLine and build a message string from it.
// Scan the source (from OpSource) to find the line of source at the reported line number and place it in another message string.
void UtilGenerateSourceMessages(vvl::span<const uint32_t> pgm, const uint32_t *debug_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg) {
    using namespace spvtools;
    std::ostringstream filename_stream;
    std::ostringstream source_stream;
    SPIRV_MODULE_STATE module_state(pgm);
    if (module_state.words_.empty()) {
        return;
    }
    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t instruction_index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    for (const Instruction &insn : module_state.GetInstructions()) {
        if (insn.Opcode() == spv::OpLine) {
            reported_file_id = insn.Word(1);
            reported_line_number = insn.Word(2);
            reported_column_number = insn.Word(3);
        }
        if (instruction_index == debug_record[kInstCommonOutInstructionIdx]) {
            break;
        }
        instruction_index++;
    }
    // Create message with file information obtained from the OpString pointed to by the discovered OpLine.
    std::string reported_filename;
    if (reported_file_id == 0) {
        filename_stream
            << "Unable to find SPIR-V OpLine for source information.  Build shader with debug info to get source information.";
    } else {
        bool found_opstring = false;
        std::string prefix;
        if (from_printf) {
            prefix = "Debug shader printf message generated ";
        } else {
            prefix = "Shader validation error occurred ";
        }

        for (const Instruction *insn : module_state.static_data_.debug_string_inst) {
            if (insn->Length() >= 3 && insn->Word(1) == reported_file_id) {
                found_opstring = true;
                reported_filename = insn->GetAsString(2);
                if (reported_filename.empty()) {
                    filename_stream << prefix << "at line " << reported_line_number;
                } else {
                    filename_stream << prefix << "in file " << reported_filename << " at line " << reported_line_number;
                }
                if (reported_column_number > 0) {
                    filename_stream << ", column " << reported_column_number;
                }
                filename_stream << ".";
                break;
            }
        }

        if (!found_opstring) {
            filename_stream << "Unable to find SPIR-V OpString for file id " << reported_file_id << " from OpLine instruction."
                            << std::endl;
            filename_stream << "File ID = " << reported_file_id << ", Line Number = " << reported_line_number
                            << ", Column = " << reported_column_number << std::endl;
        }
    }
    filename_msg = filename_stream.str();

    // Create message to display source code line containing error.
    if ((reported_file_id != 0)) {
        // Read the source code and split it up into separate lines.
        std::vector<std::string> opsource_lines;
        ReadOpSource(module_state, reported_file_id, opsource_lines);
        // Find the line in the OpSource content that corresponds to the reported error file and line.
        if (!opsource_lines.empty()) {
            uint32_t saved_line_number = 0;
            std::string current_filename = reported_filename;  // current "preprocessor" filename state.
            std::vector<std::string>::size_type saved_opsource_offset = 0;
            bool found_best_line = false;
            for (auto it = opsource_lines.begin(); it != opsource_lines.end(); ++it) {
                uint32_t parsed_line_number;
                std::string parsed_filename;
                const bool found_line = GetLineAndFilename(*it, &parsed_line_number, parsed_filename);
                if (!found_line) continue;

                const bool found_filename = parsed_filename.size() > 0;
                if (found_filename) {
                    current_filename = parsed_filename;
                }
                if ((!found_filename) || (current_filename == reported_filename)) {
                    // Update the candidate best line directive, if the current one is prior and closer to the reported line
                    if (reported_line_number >= parsed_line_number) {
                        if (!found_best_line ||
                            (reported_line_number - parsed_line_number <= reported_line_number - saved_line_number)) {
                            saved_line_number = parsed_line_number;
                            saved_opsource_offset = std::distance(opsource_lines.begin(), it);
                            found_best_line = true;
                        }
                    }
                }
            }
            if (found_best_line) {
                assert(reported_line_number >= saved_line_number);
                std::vector<std::string>::size_type opsource_index =
                    (reported_line_number - saved_line_number) + 1 + saved_opsource_offset;
                if (opsource_index < opsource_lines.size()) {
                    source_stream << "\n" << reported_line_number << ": " << opsource_lines[opsource_index].c_str();
                } else {
                    source_stream << "Internal error: calculated source line of " << opsource_index << " for source size of "
                                  << opsource_lines.size() << " lines.";
                }
            } else {
                source_stream << "Unable to find suitable #line directive in SPIR-V OpSource.";
            }
        } else {
            source_stream << "Unable to find SPIR-V OpSource.";
        }
    }
    source_msg = source_stream.str();
}
