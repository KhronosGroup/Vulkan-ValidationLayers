/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "spirv_logging.h"
#include <regex>
#include <string>

#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include <spirv/unified1/spirv.hpp>

struct SpirvLoggingInfo {
    uint32_t file_string_id = 0;  // OpString with filename
    uint32_t line_number_start = 0;
    uint32_t line_number_end = 0;
    uint32_t column_number = 0;            // most compiler will just give zero here, so just try and get a start column
    bool using_shader_debug_info = false;  // NonSemantic.Shader.DebugInfo.100
    std::string reported_filename;
};

static const spirv::Instruction *FindOpString(const std::vector<spirv::Instruction> &instructions, uint32_t string_id) {
    const spirv::Instruction *string_insn = nullptr;
    for (const auto &insn : instructions) {
        if (insn.Opcode() == spv::OpString && insn.Length() >= 3 && insn.Word(1) == string_id) {
            string_insn = &insn;
            break;
        }
        // OpString can only be in the debug section, so can break early if not found
        if (insn.Opcode() == spv::OpFunction) {
            assert(false);
            break;
        }
    }
    return string_insn;
};

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const std::vector<spirv::Instruction> &instructions, const uint32_t reported_file_id,
                         std::vector<std::string> &out_source_lines) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto &insn = instructions[i];
        if ((insn.Opcode() != spv::OpSource) || (insn.Length() < 5) || (insn.Word(3) != reported_file_id)) {
            continue;
        }

        std::istringstream in_stream;
        std::string current_line;
        in_stream.str(insn.GetAsString(4));
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }

        for (size_t k = i + 1; k < instructions.size(); k++) {
            const auto &continue_insn = instructions[k];
            if (continue_insn.Opcode() != spv::OpSourceContinued) {
                return;
            }
            in_stream.clear();  // without, will fail getline
            in_stream.str(continue_insn.GetAsString(1));
            while (std::getline(in_stream, current_line)) {
                out_source_lines.emplace_back(current_line);
            }
        }
        return;
    }
}

static void ReadDebugSource(const std::vector<spirv::Instruction> &instructions, const uint32_t debug_source_id,
                            uint32_t &out_file_string_id, std::vector<std::string> &out_source_lines) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto &insn = instructions[i];
        if (insn.ResultId() != debug_source_id) {
            continue;
        }
        out_file_string_id = insn.Word(5);

        if (insn.Length() < 7) {
            return;  // Optional source Text not provided
        }

        uint32_t string_id = insn.Word(6);
        auto string_inst = FindOpString(instructions, string_id);
        if (!string_inst) {
            return;  // error should be caught in spirv-val, but don't crash here
        }

        std::istringstream in_stream;
        std::string current_line;
        in_stream.str(string_inst->GetAsString(2));
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }

        for (size_t k = i + 1; k < instructions.size(); k++) {
            const auto &continue_insn = instructions[k];
            if (continue_insn.Opcode() != spv::OpExtInst ||
                continue_insn.Word(4) != NonSemanticShaderDebugInfo100DebugSourceContinued) {
                return;
            }
            string_id = continue_insn.Word(5);
            string_inst = FindOpString(instructions, string_id);
            if (!string_inst) {
                return;  // error should be caught in spirv-val, but don't crash here
            }

            in_stream.clear();  // without, will fail getline
            in_stream.str(string_inst->GetAsString(2));
            while (std::getline(in_stream, current_line)) {
                out_source_lines.emplace_back(current_line);
            }
        }
        return;
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
static bool GetLineFromDirective(const std::string &string, uint32_t *linenumber, std::string &filename) {
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

// Return false if any error arise
static bool GetLineAndFilename(std::ostringstream &ss, const std::vector<spirv::Instruction> &instructions,
                               SpirvLoggingInfo &logging_info) {
    const std::string debug_info_type = (logging_info.using_shader_debug_info) ? "DebugSource" : "OpLine";
    if (logging_info.file_string_id == 0) {
        // This error should be caught in spirv-val
        ss << "(Unable to find file string from SPIR-V " << debug_info_type << ")\n";
        return false;
    }

    auto file_string_insn = FindOpString(instructions, logging_info.file_string_id);
    if (!file_string_insn) {
        ss << "(Unable to find SPIR-V OpString from " << debug_info_type << " instruction.\n";
        ss << "File ID = " << logging_info.file_string_id << ", Line Number = " << logging_info.line_number_start
           << ", Column = " << logging_info.column_number << ")\n";
        return false;
    }

    logging_info.reported_filename = file_string_insn->GetAsString(2);
    if (!logging_info.reported_filename.empty()) {
        ss << "in file " << logging_info.reported_filename << " ";
    }

    ss << "at line " << logging_info.line_number_start;
    if (logging_info.line_number_end > logging_info.line_number_start) {
        ss << " to " << logging_info.line_number_end;
    }

    if (logging_info.column_number != 0) {
        ss << ", column " << logging_info.column_number;
    }
    ss << '\n';

    return true;
}

static void GetSourceLines(std::ostringstream &ss, const std::vector<std::string> &source_lines,
                           const SpirvLoggingInfo &logging_info) {
    if (source_lines.empty()) {
        if (logging_info.using_shader_debug_info) {
            ss << "No Text operand found in DebugSource\n";
        } else {
            ss << "Unable to find SPIR-V OpSource\n";
        }
        return;
    }

    // Find the line in the OpSource content that corresponds to the reported error file and line.
    uint32_t saved_line_number = 0;
    std::string current_filename = logging_info.reported_filename;  // current "preprocessor" filename state.
    std::vector<std::string>::size_type saved_opsource_offset = 0;

    // This was designed to fine the best line if using #line in GLSL
    bool found_best_line = false;
    if (!logging_info.using_shader_debug_info) {
        for (auto it = source_lines.begin(); it != source_lines.end(); ++it) {
            uint32_t parsed_line_number;
            std::string parsed_filename;
            const bool found_line = GetLineFromDirective(*it, &parsed_line_number, parsed_filename);
            if (!found_line) continue;

            const bool found_filename = parsed_filename.size() > 0;
            if (found_filename) {
                current_filename = parsed_filename;
            }
            if ((!found_filename) || (current_filename == logging_info.reported_filename)) {
                // Update the candidate best line directive, if the current one is prior and closer to the reported line
                if (logging_info.line_number_start >= parsed_line_number) {
                    if (!found_best_line || (logging_info.line_number_start - parsed_line_number <=
                                             logging_info.line_number_start - saved_line_number)) {
                        saved_line_number = parsed_line_number;
                        saved_opsource_offset = std::distance(source_lines.begin(), it);
                        found_best_line = true;
                    }
                }
            }
        }
    }

    if (logging_info.using_shader_debug_info) {
        // For Shader Debug Info, we should have all the information we need
        ss << '\n';
        for (uint32_t line_index = logging_info.line_number_start; line_index <= logging_info.line_number_end; line_index++) {
            if (line_index > source_lines.size()) {
                ss << line_index << ": [No line found in source]";
                break;
            }
            ss << line_index << ": " << source_lines[line_index - 1] << '\n';
        }
        // Only show column if since line is displayed
        if (logging_info.column_number > 0 && logging_info.line_number_start == logging_info.line_number_end) {
            std::string spaces(logging_info.column_number - 1, ' ');
            ss << spaces << '^';
        }

    } else if (found_best_line) {
        assert(logging_info.line_number_start >= saved_line_number);
        const size_t opsource_index = (logging_info.line_number_start - saved_line_number) + 1 + saved_opsource_offset;
        if (opsource_index < source_lines.size()) {
            ss << '\n' << logging_info.line_number_start << ": " << source_lines[opsource_index] << '\n';
        } else {
            ss << "Internal error: calculated source line of " << opsource_index << " for source size of " << source_lines.size()
               << " lines\n";
        }
    } else if (logging_info.line_number_start < source_lines.size() && logging_info.line_number_start != 0) {
        // file lines normally start at 1 index
        ss << '\n' << source_lines[logging_info.line_number_start - 1] << '\n';
        if (logging_info.column_number > 0) {
            std::string spaces(logging_info.column_number - 1, ' ');
            ss << spaces << '^';
        }
    } else {
        ss << "Unable to find a suitable line in SPIR-V OpSource\n";
    }
}

void GetShaderSourceInfo(std::ostringstream &ss, const std::vector<spirv::Instruction> &instructions,
                         const spirv::Instruction &last_line_insn) {
    // Instead of building up hash map that might not be used, reloop the constants to find the value.
    // Non Semantic instructions are validated to have 32-bit integer constants (not spec constants).
    auto get_constant_value = [&instructions](uint32_t id) {
        for (const auto &insn : instructions) {
            if (insn.Opcode() == spv::OpConstant && insn.ResultId() == id) {
                return insn.Word(3);
            } else if (insn.Opcode() == spv::OpFunction) {
                break;
            }
        }
        assert(false);
        return 0u;
    };

    // Read the source code and split it up into separate lines.
    //
    // 1. OpLine will point to a OpSource/OpSourceContinued which have the string built-in
    // 2. DebugLine will point to a DebugSource/DebugSourceContinued that each point to a OpString
    //
    // For the second one, we need to build the source lines up sooner
    std::vector<std::string> source_lines;

    SpirvLoggingInfo logging_info = {};
    if (last_line_insn.Opcode() == spv::OpLine) {
        logging_info.using_shader_debug_info = false;
        logging_info.file_string_id = last_line_insn.Word(1);
        logging_info.line_number_start = last_line_insn.Word(2);
        logging_info.line_number_end = logging_info.line_number_start;  // OpLine only give a single line granularity
        logging_info.column_number = last_line_insn.Word(3);
    } else {
        // NonSemanticShaderDebugInfo100DebugLine
        logging_info.using_shader_debug_info = true;
        logging_info.line_number_start = get_constant_value(last_line_insn.Word(6));
        logging_info.line_number_end = get_constant_value(last_line_insn.Word(7));
        logging_info.column_number = get_constant_value(last_line_insn.Word(8));
        const uint32_t debug_source_id = last_line_insn.Word(5);
        ReadDebugSource(instructions, debug_source_id, logging_info.file_string_id, source_lines);
    }

    if (!GetLineAndFilename(ss, instructions, logging_info)) {
        return;
    }

    // Defer finding source from OpLine until we know we have a valid file string to tie it too
    if (!logging_info.using_shader_debug_info) {
        ReadOpSource(instructions, logging_info.file_string_id, source_lines);
    }

    GetSourceLines(ss, source_lines, logging_info);
}