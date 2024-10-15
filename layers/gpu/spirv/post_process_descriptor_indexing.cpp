/* Copyright (c) 2024 LunarG, Inc.
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

#include "post_process_descriptor_indexing.h"
#include "module.h"
#include <iostream>

namespace gpuav {
namespace spirv {

bool PostProcessDescriptorIndexingPass::AnalyzeInstruction(const Instruction& inst) { return false; }

// Used between injections of a function
void PostProcessDescriptorIndexingPass::Reset() { target_instruction_ = nullptr; }

bool PostProcessDescriptorIndexingPass::Run() { return false; }

void PostProcessDescriptorIndexingPass::PrintDebugInfo() {
    std::cout << "PostProcessDescriptorIndexingPass instrumentation count: " << instrumented_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
