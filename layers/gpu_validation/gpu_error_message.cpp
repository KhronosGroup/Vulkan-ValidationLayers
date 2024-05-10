/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
#include "gpu_validation/gpu_validation.h"
#include "gpu_validation/gpu_subclasses.h"
#include "gpu_validation/gpu_vuids.h"
#include "spirv-tools/instrument.hpp"
#include "state_tracker/shader_instruction.h"
#include "gpu_shaders/gpu_error_header.h"

#include <algorithm>
#include <regex>

// Generate the stage-specific part of the message.
static void GenerateStageMessage(const uint32_t *error_record, std::string &msg) {
    using namespace gpuav;
    using namespace glsl;
    std::ostringstream strm;
    switch (error_record[kHeaderStageIdOffset]) {
        case kHeaderStageIdMultiEntryPoint: {
            strm << "Stage has multiple OpEntryPoint and could not detect stage. ";
        } break;
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << error_record[kHeaderVertexIndexOffset]
                 << " Instance Index = " << error_record[kHeaderVertInstanceIndexOffset] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << error_record[kHeaderTessCltInvocationIdOffset]
                 << ", Primitive ID = " << error_record[kHeaderTessCtlPrimitiveIdOffset];
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Primitive ID = " << error_record[kHeaderTessEvalPrimitiveIdOffset]
                 << ", TessCoord (u, v) = (" << error_record[kHeaderTessEvalCoordUOffset] << ", "
                 << error_record[kHeaderTessEvalCoordVOffset] << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << error_record[kHeaderGeomPrimitiveIdOffset]
                 << " Invocation ID = " << error_record[kHeaderGeomInvocationIdOffset] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&error_record[kHeaderFragCoordXOffset]) << ", "
                 << *reinterpret_cast<const float *>(&error_record[kHeaderFragCoordYOffset]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID (x, y, z) = (" << error_record[kHeaderInvocationIdXOffset] << ", "
                 << error_record[kHeaderInvocationIdYOffset] << ", " << error_record[kHeaderInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelRayGenerationKHR: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset]
                 << ", " << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelIntersectionKHR: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelAnyHitKHR: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelClosestHitKHR: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelMissKHR: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelCallableKHR: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelTaskEXT: {
            strm << "Stage = TaskEXT. Global invocation ID (x, y, z) = (" << error_record[kHeaderTaskGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderTaskGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderTaskGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelMeshEXT: {
            strm << "Stage = MeshEXT. Global invocation ID (x, y, z) = (" << error_record[kHeaderMeshGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderMeshGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderMeshGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelTaskNV: {
            strm << "Stage = TaskNV. Global invocation ID (x, y, z) = (" << error_record[kHeaderTaskGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderTaskGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderTaskGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            strm << "Stage = MeshNV. Global invocation ID (x, y, z) = (" << error_record[kHeaderMeshGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderMeshGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderMeshGlobalInvocationIdZOffset] << " )";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << error_record[kHeaderStageIdOffset] << "). ";
            assert(false);
        } break;
    }
    msg = strm.str();
}

// The lock (debug_output_mutex) is held by the caller (UtilGenerateCommonMessage),
// because the latter has code paths that make multiple calls of this function,
// and all such calls have to access the same debug reporting state to ensure consistency of output information.
static std::string LookupDebugUtilsNameNoLock(const DebugReport *debug_report, const uint64_t object) {
    auto object_label = debug_report->GetUtilsObjectNameNoLock(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
void UtilGenerateCommonMessage(const DebugReport *debug_report, const VkCommandBuffer commandBuffer, const uint32_t *debug_record,
                               const VkShaderModule shader_module_handle, const VkPipeline pipeline_handle,
                               const VkShaderEXT shader_object_handle, const VkPipelineBindPoint pipeline_bind_point,
                               const uint32_t operation_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE && shader_object_handle == VK_NULL_HANDLE) {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer)
             << "). ";
        assert(true);
    } else {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        strm << std::hex << std::showbase << "Command buffer "
             << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer)
             << "). ";
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
                 << "Pipeline " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(pipeline_handle)) << "("
                 << HandleToUint64(pipeline_handle) << "). "
                 << "Shader Module " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(shader_module_handle)) << "("
                 << HandleToUint64(shader_module_handle) << "). ";
        } else {
            strm << "Index " << operation_index << ". "
                 << "Shader Object " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(shader_object_handle)) << "("
                 << HandleToUint64(shader_object_handle) << "). ";
        }
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[gpuav::glsl::kHeaderInstructionIdOffset] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const std::vector<spirv::Instruction> &instructions, const uint32_t reported_file_id,
                         std::vector<std::string> &opsource_lines) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const spirv::Instruction &insn = instructions[i];
        if ((insn.Opcode() == spv::OpSource) && (insn.Length() >= 5) && (insn.Word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str(insn.GetAsString(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }

            for (size_t k = i + 1; k < instructions.size(); k++) {
                const spirv::Instruction &continue_insn = instructions[k];
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
void UtilGenerateSourceMessages(const std::vector<spirv::Instruction> &instructions, const uint32_t *error_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg) {
    using namespace spvtools;
    if (instructions.empty()) {
        // TODO - We currently don't have a good single code path if the shader_map can't find the shader module handle
        return;
    }

    std::ostringstream filename_stream;
    std::ostringstream source_stream;
    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t instruction_index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    for (const spirv::Instruction &insn : instructions) {
        if (insn.Opcode() == spv::OpLine) {
            reported_file_id = insn.Word(1);
            reported_line_number = insn.Word(2);
            reported_column_number = insn.Word(3);
        }
        if (instruction_index == error_record[gpuav::glsl::kHeaderInstructionIdOffset]) {
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

        for (const spirv::Instruction &insn : instructions) {
            if (insn.Opcode() == spv::OpString && insn.Length() >= 3 && insn.Word(1) == reported_file_id) {
                found_opstring = true;
                reported_filename = insn.GetAsString(2);
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
        ReadOpSource(instructions, reported_file_id, opsource_lines);
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

namespace gpuav {
bool Validator::LogMessageInstBindlessDescriptor(const uint32_t *error_record, std::string &out_error_msg,
                                                 std::string &out_vuid_msg, const CommandResources &cmd_resources,
                                                 const std::vector<DescSetState> &descriptor_sets, bool &out_oob_access) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    const GpuVuid vuid = GetGpuVuid(cmd_resources.command);

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBindlessDescriptorBounds: {
            strm << "(set = " << error_record[kInstBindlessBoundsDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessBoundsDescBindingOffset] << ") Index of "
                 << error_record[kInstBindlessBoundsDescIndexOffset] << " used to index descriptor array of length "
                 << error_record[kInstBindlessBoundsDescBoundOffset] << ".";
            out_vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorUninit: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " is uninitialized.";
            out_vuid_msg = vuid.invalid_descriptor;
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorDestroyed: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " references a resource that was destroyed.";
            out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorOOB: {
            const uint32_t set_num = error_record[kInstBindlessBuffOOBDescSetOffset];
            const uint32_t binding_num = error_record[kInstBindlessBuffOOBDescBindingOffset];
            const uint32_t desc_index = error_record[kInstBindlessBuffOOBDescIndexOffset];
            const uint32_t size = error_record[kInstBindlessBuffOOBBuffSizeOffset];
            const uint32_t offset = error_record[kInstBindlessBuffOOBBuffOffOffset];
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            assert(binding_state);
            if (size == 0) {
                strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                     << " is uninitialized.";
                out_vuid_msg = vuid.invalid_descriptor;
                error_found = true;
                break;
            }
            out_oob_access = true;
            auto desc_class = binding_state->descriptor_class;
            if (desc_class == vvl::DescriptorClass::Mutable) {
                desc_class = static_cast<const vvl::MutableBinding *>(binding_state)->descriptors[desc_index].ActiveClass();
            }

            switch (desc_class) {
                case vvl::DescriptorClass::GeneralBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " and highest byte accessed was " << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        out_vuid_msg =
                            cmd_resources.uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg =
                            cmd_resources.uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                case vvl::DescriptorClass::TexelBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " texels and highest texel accessed was "
                         << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                        out_vuid_msg =
                            cmd_resources.uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg =
                            cmd_resources.uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                default:
                    // other OOB checks are not implemented yet
                    assert(false);
            }
        } break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool Validator::LogMessageInstBufferDeviceAddress(const uint32_t *error_record, std::string &out_error_msg,
                                                  std::string &out_vuid_msg, bool &out_oob_access) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBufferDeviceAddressUnallocRef: {
            out_oob_access = true;
            const char *access_type = error_record[kInstBuffAddrAccessInstructionOffset] == spv::OpStore ? "written" : "read";
            uint64_t address = *reinterpret_cast<const uint64_t *>(error_record + kInstBuffAddrUnallocDescPtrLoOffset);
            strm << "Out of bounds access: " << error_record[kInstBuffAddrAccessByteSizeOffset] << " bytes " << access_type
                 << " at buffer device address 0x" << std::hex << address << '.';
            out_vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool Validator::LogMessageInstRayQuery(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
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
    out_error_msg = strm.str();
    return error_found;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
bool Validator::AnalyzeAndGenerateMessage(VkCommandBuffer cmd_buffer, VkQueue queue, CommandResources &cmd_resources,
                                          uint32_t operation_index, uint32_t *const error_record,
                                          const std::vector<DescSetState> &descriptor_sets, const Location &loc) {
    // The second word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor. So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record". This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record. If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.

    std::string error_msg;
    std::string vuid_msg;
    bool oob_access = false;
    bool error_found = false;
    switch (error_record[glsl::kHeaderErrorGroupOffset]) {
        case glsl::kErrorGroupInstBindlessDescriptor:
            error_found =
                LogMessageInstBindlessDescriptor(error_record, error_msg, vuid_msg, cmd_resources, descriptor_sets, oob_access);
            break;
        case glsl::kErrorGroupInstBufferDeviceAddress:
            error_found = LogMessageInstBufferDeviceAddress(error_record, error_msg, vuid_msg, oob_access);
            break;
        case glsl::kErrorGroupInstRayQuery:
            error_found = LogMessageInstRayQuery(error_record, error_msg, vuid_msg);
            break;
        default:
            break;
    }

    if (error_found) {
        VkShaderModule shader_module_handle = VK_NULL_HANDLE;
        VkPipeline pipeline_handle = VK_NULL_HANDLE;
        VkShaderEXT shader_object_handle = VK_NULL_HANDLE;
        vvl::span<const uint32_t> instrumented_spirv;

        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        auto it = shader_map.find(error_record[glsl::kHeaderShaderIdOffset]);
        if (it != shader_map.end()) {
            shader_module_handle = it->second.shader_module;
            pipeline_handle = it->second.pipeline;
            shader_object_handle = it->second.shader_object;
            instrumented_spirv = it->second.instrumented_spirv;
        }

        std::vector<spirv::Instruction> instructions;
        spirv::GenerateInstructions(instrumented_spirv, instructions);

        std::string stage_message;
        std::string common_message;
        std::string filename_message;
        std::string source_message;
        GenerateStageMessage(error_record, stage_message);
        UtilGenerateCommonMessage(debug_report, cmd_buffer, error_record, shader_module_handle, pipeline_handle,
                                  shader_object_handle, cmd_resources.pipeline_bind_point, operation_index, common_message);
        UtilGenerateSourceMessages(instructions, error_record, false, filename_message, source_message);

        if (cmd_resources.uses_robustness && oob_access) {
            if (gpuav_settings.warn_on_robust_oob) {
                LogWarning(vuid_msg.c_str(), queue, loc, "%s %s %s %s%s", error_msg.c_str(), common_message.c_str(),
                           stage_message.c_str(), filename_message.c_str(), source_message.c_str());
            }
        } else {
            LogError(vuid_msg.c_str(), queue, loc, "%s %s %s %s%s", error_msg.c_str(), common_message.c_str(),
                     stage_message.c_str(), filename_message.c_str(), source_message.c_str());
        }
    }

    return error_found;
}

bool CommandResources::LogValidationMessage(Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer,
                                            uint32_t *output_buffer_begin, const uint32_t operation_index,
                                            const LogObjectList &objlist) {
    const DescBindingInfo *di_info = desc_binding_index != vvl::kU32Max ? &(*desc_binding_list)[desc_binding_index] : nullptr;
    const Location loc(command);
    bool error_logged =
        validator.AnalyzeAndGenerateMessage(cmd_buffer, queue, *this, operation_index, output_buffer_begin,
                                            di_info ? di_info->descriptor_set_buffers : std::vector<DescSetState>(), loc);

    if (!error_logged) {
        error_logged = LogCustomValidationMessage(validator, output_buffer_begin, operation_index, objlist);
    }
    return error_logged;
}

bool PreDrawResources::LogCustomValidationMessage(Validator &validator, const uint32_t *error_record,
                                                  const uint32_t operation_index, const LogObjectList &objlist) {
    using namespace glsl;
    bool error_logged = false;
    if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreDraw) {
        return error_logged;
    }

    const GpuVuid &vuids = GetGpuVuid(command);
    const Location loc(command);

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodePreDrawBufferSize: {
            // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand))
            const uint32_t count = error_record[kPreActionParamOffset_0];
            const uint32_t stride = indirect_buffer_stride;
            const uint32_t offset =
                static_cast<uint32_t>(indirect_buffer_offset);  // TODO: why cast to uin32_t? If it is changed, think about
                                                                // also doing it in the error message
            const uint32_t draw_size = (stride * (count - 1) + offset + sizeof(VkDrawIndexedIndirectCommand));

            const char *vuid = nullptr;
            if (count == 1) {
                vuid = vuids.count_exceeds_bufsize_1;
            } else {
                vuid = vuids.count_exceeds_bufsize;
            }
            validator.LogError(vuid, objlist, loc,
                               "Indirect draw count of %" PRIu32 " would exceed buffer size %" PRIu64
                               " of buffer %s "
                               "stride = %" PRIu32 " offset = %" PRIu32
                               " (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand)) = %" PRIu32 ".",
                               count, indirect_buffer_size, validator.FormatHandle(indirect_buffer).c_str(), stride, offset,
                               draw_size);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDrawCountLimit: {
            const uint32_t count = error_record[kPreActionParamOffset_0];
            validator.LogError(vuids.count_exceeds_device_limit, objlist, loc,
                               "Indirect draw count of %" PRIu32 " would exceed maxDrawIndirectCount limit of %" PRIu32 ".", count,
                               validator.phys_dev_props.limits.maxDrawIndirectCount);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDrawFirstInstance: {
            const uint32_t index = error_record[kPreActionParamOffset_0];
            validator.LogError(
                vuids.first_instance_not_zero, objlist, loc,
                "The drawIndirectFirstInstance feature is not enabled, but the firstInstance member of the %s structure at "
                "index %" PRIu32 " is not zero.",
                String(command), index);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDrawGroupCountX:
        case kErrorSubCodePreDrawGroupCountY:
        case kErrorSubCodePreDrawGroupCountZ: {
            const uint32_t group_count = error_record[kPreActionParamOffset_0];
            const uint32_t draw_number = error_record[kPreActionParamOffset_1];
            const char *count_label;
            uint32_t index;
            uint32_t limit;
            const char *vuid;
            if (error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountX) {
                count_label = "groupCountX";
                index = 0;
                vuid = emit_task_error ? vuids.task_group_count_exceeds_max_x : vuids.mesh_group_count_exceeds_max_x;
                limit = validator.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
            } else if (error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountY) {
                count_label = "groupCountY";
                index = 1;
                vuid = emit_task_error ? vuids.task_group_count_exceeds_max_y : vuids.mesh_group_count_exceeds_max_y;
                limit = validator.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
            } else {
                assert(error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountZ);
                count_label = "groupCountZ";
                index = 2;
                vuid = emit_task_error ? vuids.task_group_count_exceeds_max_z : vuids.mesh_group_count_exceeds_max_z;
                limit = validator.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
            }
            validator.LogError(vuid, objlist, loc,
                               "In draw %" PRIu32 ", %s is %" PRIu32
                               " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[%" PRIu32
                               "] (%" PRIu32 ").",
                               draw_number, count_label, group_count, index, limit);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDrawGroupCountTotal: {
            const uint32_t total_count = error_record[kPreActionParamOffset_0];
            const uint32_t draw_number = error_record[kPreActionParamOffset_1];
            auto vuid = emit_task_error ? vuids.task_group_count_exceeds_max_total : vuids.mesh_group_count_exceeds_max_total;
            validator.LogError(
                vuid, objlist, loc,
                "In draw %" PRIu32 ", The product of groupCountX, groupCountY and groupCountZ (%" PRIu32
                ") is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (%" PRIu32 ").",
                draw_number, total_count, validator.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount);
            error_logged = true;
            break;
        }
        default:
            break;
    }
    return error_logged;
}

bool PreDispatchResources::LogCustomValidationMessage(Validator &validator, const uint32_t *error_record,
                                                      const uint32_t operation_index, const LogObjectList &objlist) {
    using namespace glsl;
    bool error_logged = false;
    if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreDispatch) {
        return error_logged;
    }

    const Location loc(command);
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodePreDispatchCountLimitX: {
            uint32_t count = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkDispatchIndirectCommand-x-00417", objlist, loc,
                               "Indirect dispatch VkDispatchIndirectCommand::x of %" PRIu32
                               " would exceed maxComputeWorkGroupCount[0] limit of %" PRIu32 ".",
                               count, validator.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDispatchCountLimitY: {
            uint32_t count = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkDispatchIndirectCommand-y-00418", objlist, loc,
                               "Indirect dispatch VkDispatchIndirectCommand::y of %" PRIu32
                               " would exceed maxComputeWorkGroupCount[1] limit of %" PRIu32 ".",
                               count, validator.phys_dev_props.limits.maxComputeWorkGroupCount[1]);
            error_logged = true;
            break;
        }
        case kErrorSubCodePreDispatchCountLimitZ: {
            uint32_t count = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkDispatchIndirectCommand-z-00419", objlist, loc,
                               "Indirect dispatch VkDispatchIndirectCommand::z of %" PRIu32
                               " would exceed maxComputeWorkGroupCount[2] limit of %" PRIu32 ".",
                               count, validator.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
            error_logged = true;
            break;
        }
        default:
            break;
    }
    return error_logged;
}

bool PreTraceRaysResources::LogCustomValidationMessage(Validator &validator, const uint32_t *error_record,
                                                       const uint32_t operation_index, const LogObjectList &objlist) {
    using namespace glsl;

    const Location loc(command);
    bool error_logged = false;
    if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreTraceRays) {
        return error_logged;
    }

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodePreTraceRaysLimitWidth: {
            const uint32_t width = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkTraceRaysIndirectCommandKHR-width-03638", objlist, loc,
                               "Indirect trace rays of VkTraceRaysIndirectCommandKHR::width of %" PRIu32
                               " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] * "
                               "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] limit of %" PRIu64 ".",
                               width,
                               static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                   static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupSize[0]));
            error_logged = true;
            break;
        }
        case kErrorSubCodePreTraceRaysLimitHeight: {
            uint32_t height = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkTraceRaysIndirectCommandKHR-height-03639", objlist, loc,
                               "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                               " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] * "
                               "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] limit of %" PRIu64 ".",
                               height,
                               static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                   static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupSize[1]));
            error_logged = true;
            break;
        }
        case kErrorSubCodePreTraceRaysLimitDepth: {
            uint32_t depth = error_record[kPreActionParamOffset_0];
            validator.LogError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640", objlist, loc,
                               "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                               " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] * "
                               "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] limit of %" PRIu64 ".",
                               depth,
                               static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                   static_cast<uint64_t>(validator.phys_dev_props.limits.maxComputeWorkGroupSize[2]));
            error_logged = true;
            break;
        }
        default:
            break;
    }
    return error_logged;
}

bool PreCopyBufferToImageResources::LogCustomValidationMessage(Validator &validator, const uint32_t *error_record,
                                                               const uint32_t operation_index, const LogObjectList &objlist) {
    using namespace glsl;
    bool error_logged = false;
    if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuCopyBufferToImage) {
        return error_logged;
    }

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodePreCopyBufferToImageBufferTexel: {
            uint32_t texel_offset = error_record[kPreActionParamOffset_0];
            LogObjectList objlist_and_src_buffer = objlist;
            objlist_and_src_buffer.add(this->src_buffer);
            const char *vuid = this->command == vvl::Func::vkCmdCopyBufferToImage ? "VUID-vkCmdCopyBufferToImage-pRegions-07931"
                                                                                  : "VUID-VkCopyBufferToImageInfo2-pRegions-07931";
            validator.LogError(vuid, objlist_and_src_buffer, command,
                               "Source buffer %s has a float value at offset %" PRIu32 " that is not in the range [0, 1].",
                               validator.FormatHandle(this->src_buffer).c_str(), texel_offset);
            error_logged = true;
            break;
        }
        default:
            break;
    }
    return error_logged;
}
}  // namespace gpuav

void GpuShaderInstrumentor::ReportSetupProblem(LogObjectList objlist, const Location &loc, const char *const specific_message,
                                               bool vma_fail) const {
    std::string error_message = specific_message;
    if (vma_fail) {
        char *stats_string;
        vmaBuildStatsString(vmaAllocator, &stats_string, false);
        error_message += " VMA statistics = ";
        error_message += stats_string;
        vmaFreeStatsString(vmaAllocator, stats_string);
    }

    char const *layer_name = container_type == LayerObjectTypeDebugPrintf ? "Debug PrintF" : "GPU-AV";
    char const *vuid =
        container_type == LayerObjectTypeDebugPrintf ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, loc, "Setup Error, %s is being disabled. Detail: (%s)", layer_name, error_message.c_str());
}
