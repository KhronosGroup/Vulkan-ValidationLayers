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
#include <sstream>

#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include <spirv/unified1/spirv.hpp>

const spirv::Instruction *FindOpString(const std::vector<spirv::Instruction> &instructions, uint32_t string_id) {
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
void ReadOpSource(const std::vector<spirv::Instruction> &instructions, const uint32_t reported_file_id,
                  std::vector<std::string> &out_opsource_lines) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto &insn = instructions[i];
        if ((insn.Opcode() != spv::OpSource) || (insn.Length() < 5) || (insn.Word(3) != reported_file_id)) {
            continue;
        }

        std::istringstream in_stream;
        std::string current_line;
        in_stream.str(insn.GetAsString(4));
        while (std::getline(in_stream, current_line)) {
            out_opsource_lines.emplace_back(current_line);
        }

        for (size_t k = i + 1; k < instructions.size(); k++) {
            const auto &continue_insn = instructions[k];
            if (continue_insn.Opcode() != spv::OpSourceContinued) {
                return;
            }
            in_stream.clear();  // without, will fail getline
            in_stream.str(continue_insn.GetAsString(1));
            while (std::getline(in_stream, current_line)) {
                out_opsource_lines.emplace_back(current_line);
            }
        }
        return;
    }
}

void ReadDebugSource(const std::vector<spirv::Instruction> &instructions, const uint32_t debug_source_id,
                     uint32_t &out_file_string_id, std::vector<std::string> &out_opsource_lines) {
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
            out_opsource_lines.emplace_back(current_line);
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
                out_opsource_lines.emplace_back(current_line);
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
bool GetLineAndFilename(const std::string &string, uint32_t *linenumber, std::string &filename) {
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