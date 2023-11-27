/* Copyright (c) 2023 LunarG, Inc.
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

#include "pass.h"
#include "module.h"
#include "type_manager.h"
#include <spirv/unified1/spirv.hpp>

namespace gpuav {
namespace spirv {

const Variable& Pass::GetBuiltinVariable(uint32_t built_in) {
    const Instruction* decoration_insn = nullptr;
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationBuiltIn &&
            annotation->Word(3) == built_in) {
            decoration_insn = annotation.get();
            break;
        }
    }

    uint32_t variable_id = 0;
    if (!decoration_insn) {
        variable_id = module_.TakeNextId();
        auto new_insn = std::make_unique<Instruction>(4, spv::OpDecorate);
        new_insn->Fill({variable_id, spv::DecorationBuiltIn, built_in});
        module_.annotations_.emplace_back(std::move(new_insn));
    }

    // Currently we only ever needed Input variables and the built-ins we are using are not those that can be used by both Input and
    // Output storage classes
    const Variable* built_in_variable = module_.type_manager_.FindVariableById(variable_id);
    if (!built_in_variable) {
        const Type& pointer_type = module_.type_manager_.GetTypePointerBuiltInInput(spv::BuiltIn(built_in));
        auto new_insn = std::make_unique<Instruction>(4, spv::OpVariable);
        new_insn->Fill({pointer_type.Id(), variable_id, spv::StorageClassInput});
        built_in_variable = &module_.type_manager_.AddVariable(std::move(new_insn), pointer_type);

        for (auto& entry_point : module_.entry_points_) {
            entry_point->AppendWord(built_in_variable->Id());
        }
    }

    return *built_in_variable;
}

uint32_t Pass::CreateStageInfo(spv::ExecutionModel execution_model, BasicBlock& block) {
    // Stage info is always passed in as a uvec4
    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    const Type& vec4_type = module_.type_manager_.GetTypeVector(uint32_type, 4);
    const uint32_t uint32_0_id = module_.type_manager_.GetConstantZeroUint32().Id();
    uint32_t stage_info[4] = {uint32_0_id, uint32_0_id, uint32_0_id, uint32_0_id};

    stage_info[0] = module_.type_manager_.CreateConstantUInt32(execution_model).Id();

    // Gets BuiltIn variable and creates a valid OpLoad of it
    auto create_load = [this, &block](spv::BuiltIn built_in) {
        const Variable& variable = GetBuiltinVariable(built_in);
        const Type* pointer_type = variable.PointerType(module_.type_manager_);
        const uint32_t load_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpLoad, {pointer_type->Id(), load_id, variable.Id()});
        return load_id;
    };

    switch (execution_model) {
        case spv::ExecutionModelVertex: {
            stage_info[1] = create_load(spv::BuiltInVertexIndex);
            stage_info[2] = create_load(spv::BuiltInInstanceIndex);
        } break;
        case spv::ExecutionModelFragment: {
            const uint32_t load_id = create_load(spv::BuiltInFragCoord);
            // convert vec4 to uvec4
            const uint32_t bitcast_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpBitcast, {vec4_type.Id(), bitcast_id, load_id});

            for (uint32_t i = 0; i < 2; i++) {
                const uint32_t extract_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i});
                stage_info[i + 1] = extract_id;
            }
        } break;
        case spv::ExecutionModelRayGenerationKHR:
        case spv::ExecutionModelIntersectionKHR:
        case spv::ExecutionModelAnyHitKHR:
        case spv::ExecutionModelClosestHitKHR:
        case spv::ExecutionModelMissKHR:
        case spv::ExecutionModelCallableKHR: {
            const uint32_t load_id = create_load(spv::BuiltInLaunchIdKHR);

            for (uint32_t i = 0; i < 3; i++) {
                const uint32_t extract_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i});
                stage_info[i + 1] = extract_id;
            }
        } break;
        case spv::ExecutionModelGLCompute:
        case spv::ExecutionModelTaskNV:
        case spv::ExecutionModelMeshNV:
        case spv::ExecutionModelTaskEXT:
        case spv::ExecutionModelMeshEXT: {
            const uint32_t load_id = create_load(spv::BuiltInGlobalInvocationId);

            for (uint32_t i = 0; i < 3; i++) {
                const uint32_t extract_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i});
                stage_info[i + 1] = extract_id;
            }
        } break;
        case spv::ExecutionModelGeometry: {
            stage_info[1] = create_load(spv::BuiltInPrimitiveId);
            stage_info[2] = create_load(spv::BuiltInInvocationId);
        } break;
        case spv::ExecutionModelTessellationControl: {
            stage_info[1] = create_load(spv::BuiltInInvocationId);
            stage_info[2] = create_load(spv::BuiltInPrimitiveId);
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            stage_info[1] = create_load(spv::BuiltInPrimitiveId);

            // convert vec3 to uvec3
            const Type& vec3_type = module_.type_manager_.GetTypeVector(uint32_type, 3);
            const uint32_t load_id = create_load(spv::BuiltInTessCoord);
            const uint32_t bitcast_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpBitcast, {vec3_type.Id(), bitcast_id, load_id});

            // TessCoord.uv values from it
            for (uint32_t i = 0; i < 2; i++) {
                const uint32_t extract_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpCompositeExtract, {uint32_type.Id(), extract_id, load_id, i});
                stage_info[i + 2] = extract_id;
            }
        } break;
        default: {
            assert(false && "unsupported stage");
        } break;
    }

    const uint32_t result_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpCompositeConstruct,
                            {vec4_type.Id(), result_id, stage_info[0], stage_info[1], stage_info[2], stage_info[3]});

    return result_id;
}

}  // namespace spirv
}  // namespace gpuav