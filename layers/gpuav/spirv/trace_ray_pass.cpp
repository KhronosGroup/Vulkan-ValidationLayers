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

#include "trace_ray_pass.h"

#include "containers/container_utils.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "module.h"

#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_trace_ray_comp, instrumentation_trace_ray_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kTraceRayAccelerationStructureValidationFunction = {"inst_trace_ray_acceleration_structure",
                                                                                 instrumentation_trace_ray_comp_function_0_offset};
const static OfflineFunction kTraceRayValidationFunction = {"inst_trace_ray", instrumentation_trace_ray_comp_function_1_offset};
const static OfflineFunction kRayHitObjectValidationFunction = {"inst_ray_hit_object",
                                                                instrumentation_trace_ray_comp_function_2_offset};
const static OfflineFunction kRayHitObjectSbtIndexValidationFunction = {"inst_ray_hit_object_sbt_index_check",
                                                                        instrumentation_trace_ray_comp_function_3_offset};
const static OfflineFunction kRayQueryInitializeValidationFunction = {"inst_ray_query_comp",
                                                                      instrumentation_trace_ray_comp_function_4_offset};

TraceRayPass::TraceRayPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

std::vector<uint32_t> TraceRayPass::GetTlasValidationFunctionCallInstructions(const Function& function, uint32_t tlas_operand_pos,
                                                                              uint32_t error_sub_code, BasicBlock& block,
                                                                              InstructionIt* trace_ray_inst_it) {
    if (module_.interface_.descriptor_mode != vvl::DescriptorMode::DescriptorModeClassic) {
        return {};
    }

    const uint32_t as_op_load_id = (*trace_ray_inst_it)->get()->Operand(tlas_operand_pos);
    const Instruction* as_op_load_inst = function.FindInstruction(as_op_load_id);
    if (!as_op_load_inst) {
        return {};
    }

    // AS descriptors use UniformConstant storage class (unlike buffers which use Uniform/StorageBuffer),
    // so a non-array AS can be loaded directly from its variable with no access chain in between.
    // Pre-initialize variable for that case; the loop below handles the access-chain case.
    const Variable* variable = type_manager_.FindVariableById(as_op_load_inst->Operand(0));
    std::vector<const Instruction*> access_chain_insts;
    const Instruction* next_access_chain = function.FindInstruction(as_op_load_inst->Operand(0));
    // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
    while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
        access_chain_insts.push_back(next_access_chain);
        const uint32_t access_chain_base_id = next_access_chain->Operand(0);
        variable = type_manager_.FindVariableById(access_chain_base_id);
        if (variable) {
            break;  // found
        }
        next_access_chain = function.FindInstruction(access_chain_base_id);
    }
    if (!variable) {
        return {};
    }

    const Type* descriptor_type = variable->PointerType(type_manager_);
    if (!descriptor_type || descriptor_type->spv_type_ == SpvType::kRuntimeArray) {
        return {};  // TODO - Currently we mark these as "bindless"
    }

    const bool is_descriptor_array = descriptor_type->IsArray();
    if (is_descriptor_array && access_chain_insts.empty()) {
        return {};  // array descriptor without an access chain is invalid SPIR-V
    }

    uint32_t descriptor_index_id = 0;
    if (is_descriptor_array) {
        // Because you can't have 2D array of descriptors, the first index of the last accessChain is the descriptor index
        descriptor_index_id = access_chain_insts.back()->Operand(1);
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        descriptor_index_id = type_manager_.GetConstantZeroUint32().Id();
    }

    uint32_t descriptor_set = 0;
    uint32_t descriptor_binding = 0;
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable->Id()) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                descriptor_set = annotation->Word(3);
            } else if (annotation->Word(2) == spv::DecorationBinding) {
                descriptor_binding = annotation->Word(3);
            }
        }
    }

    if (descriptor_set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return {};
    }

    const Constant& desc_set_constant = type_manager_.GetConstantUInt32(descriptor_set);
    const uint32_t desc_index_id = CastToUint32(descriptor_index_id, block, trace_ray_inst_it);  // might be int32

    const auto& layout_lut = module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut;
    BindingLayout binding_layout = layout_lut[descriptor_set][descriptor_binding];
    const Constant& binding_layout_offset = type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(trace_ray_as_link_function_id_, kTraceRayAccelerationStructureValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t inst_position = (*trace_ray_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t error_sub_code_id = type_manager_.GetConstantUInt32(error_sub_code).Id();

    return {bool_type,
            function_result,
            function_def,
            inst_position_id,
            desc_set_constant.Id(),
            desc_index_id,
            binding_layout_offset.Id(),
            error_sub_code_id};
}

std::vector<uint32_t> TraceRayPass::GetTraceRayValidationFunctionCallInstructions(InstructionIt* trace_ray_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(trace_ray_link_function_id_, kTraceRayValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t ray_flags_id = (*trace_ray_inst_it)->get()->Operand(1);
    const uint32_t cull_mask_id = (*trace_ray_inst_it)->get()->Operand(2);
    const uint32_t sbt_record_offset_id = (*trace_ray_inst_it)->get()->Operand(3);
    const uint32_t sbt_record_stride_id = (*trace_ray_inst_it)->get()->Operand(4);
    const uint32_t miss_index_id = (*trace_ray_inst_it)->get()->Operand(5);
    const uint32_t origin_id = (*trace_ray_inst_it)->get()->Operand(6);
    const uint32_t t_min_id = (*trace_ray_inst_it)->get()->Operand(7);
    const uint32_t direction_id = (*trace_ray_inst_it)->get()->Operand(8);
    const uint32_t t_max_id = (*trace_ray_inst_it)->get()->Operand(9);

    const uint32_t pipeline_flags = (module_.interface_.pipeline_has_skip_aabbs_flag ? 0x1 : 0u) |
                                    (module_.interface_.pipeline_has_skip_triangles_flag ? 0x2 : 0u);
    const uint32_t pipeline_flags_id = type_manager_.CreateConstantUInt32(pipeline_flags).Id();

    const uint32_t inst_position = (*trace_ray_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    return {bool_type,
            function_result,
            function_def,
            inst_position_id,
            ray_flags_id,
            cull_mask_id,
            sbt_record_offset_id,
            sbt_record_stride_id,
            miss_index_id,
            origin_id,
            t_min_id,
            direction_id,
            t_max_id,
            pipeline_flags_id};
}

std::vector<uint32_t> TraceRayPass::GetRayHitObjectValidationFunctionCallInstructions(InstructionIt* ray_hit_object_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(hit_object_link_function_id_, kRayHitObjectValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t opcode = (*ray_hit_object_inst_it)->get()->Opcode();

    // All HitObject opcodes have ray parameters at the same positions
    const uint32_t ray_flags_id = (*ray_hit_object_inst_it)->get()->Operand(2);
    const uint32_t ray_origin_id = (*ray_hit_object_inst_it)->get()->Operand(7);
    const uint32_t ray_tmin_id = (*ray_hit_object_inst_it)->get()->Operand(8);
    const uint32_t ray_direction_id = (*ray_hit_object_inst_it)->get()->Operand(9);
    const uint32_t ray_tmax_id = (*ray_hit_object_inst_it)->get()->Operand(10);

    uint32_t time_id = 0;
    if (opcode == spv::OpHitObjectTraceRayMotionEXT || opcode == spv::OpHitObjectTraceMotionReorderExecuteEXT) {
        time_id = (*ray_hit_object_inst_it)->get()->Operand(11);
    }

    const uint32_t inst_position = (*ray_hit_object_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t opcode_type_id = type_manager_.CreateConstantUInt32(opcode).Id();

    const uint32_t pipeline_flags = (module_.interface_.pipeline_has_skip_aabbs_flag ? 0x1 : 0u) |
                                    (module_.interface_.pipeline_has_skip_triangles_flag ? 0x2 : 0u);
    const uint32_t pipeline_flags_id = type_manager_.CreateConstantUInt32(pipeline_flags).Id();

    // For non-motion opcodes, pass 0.0 as time (valid value, won't trigger error)
    if (time_id == 0) {
        time_id = type_manager_.GetConstantZeroFloat32().Id();
    }

    return {bool_type,     function_result, function_def,     inst_position_id, opcode_type_id,    ray_flags_id,
            ray_origin_id, ray_tmin_id,     ray_direction_id, ray_tmax_id,      pipeline_flags_id, time_id};
}

std::vector<uint32_t> TraceRayPass::GetRayHitObjectSbtIndexValidationFunctionCallInstructions(
    InstructionIt* ray_hit_object_sbt_index_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(hit_object_sbt_index_link_function_id_, kRayHitObjectSbtIndexValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t sbt_index_id = (*ray_hit_object_sbt_index_inst_it)->get()->Operand(1);

    const uint32_t inst_position = (*ray_hit_object_sbt_index_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    // maxShaderBindingTableRecordIndex
    const uint32_t max_sbt_index = module_.interface_.max_shader_binding_table_record_index;
    const uint32_t max_sbt_index_id = type_manager_.CreateConstantUInt32(max_sbt_index).Id();

    return {bool_type, function_result, function_def, inst_position_id, sbt_index_id, max_sbt_index_id};
}

std::vector<uint32_t> TraceRayPass::GetRayQueryInitializeValidationFunctionCallInstructions(InstructionIt* ray_query_init_inst_it) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(ray_query_initialize_function_id_, kRayQueryInitializeValidationFunction);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t ray_flags_id = (*ray_query_init_inst_it)->get()->Operand(2);
    const uint32_t ray_origin_id = (*ray_query_init_inst_it)->get()->Operand(4);
    const uint32_t ray_tmin_id = (*ray_query_init_inst_it)->get()->Operand(5);
    const uint32_t ray_direction_id = (*ray_query_init_inst_it)->get()->Operand(6);
    const uint32_t ray_tmax_id = (*ray_query_init_inst_it)->get()->Operand(7);

    const uint32_t inst_position = (*ray_query_init_inst_it)->get()->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    return {bool_type,     function_result, function_def,     inst_position_id, ray_flags_id,
            ray_origin_id, ray_tmin_id,     ray_direction_id, ray_tmax_id};
}

uint32_t TraceRayPass::AddFunctionCall(BasicBlock& block, std::vector<uint32_t>&& instructions, InstructionIt* inst_it) {
    const uint32_t function_result = instructions[1];
    block.CreateInstruction(spv::OpFunctionCall, std::move(instructions), inst_it);
    module_.need_log_error_ = true;
    return function_result;
}

bool TraceRayPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }
        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            if (current_block.IsLoopHeader()) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }

                const spv::Op opcode = spv::Op(inst_it->get()->Opcode());

                std::vector<std::vector<uint32_t>> func_calls;
                auto add_func_call = [&](std::vector<uint32_t>&& func_call) {
                    if (!func_call.empty()) {
                        func_calls.emplace_back(std::move(func_call));
                    }
                };
                switch (opcode) {
                    case spv::OpRayQueryInitializeKHR: {
                        add_func_call(GetTlasValidationFunctionCallInstructions(
                            function, 1, glsl::kErrorSubCode_RayQuery_TlasNotBuilt, current_block, &inst_it));
                        add_func_call(GetRayQueryInitializeValidationFunctionCallInstructions(&inst_it));
                        break;
                    }
                    case spv::OpTraceRayKHR: {
                        add_func_call(GetTraceRayValidationFunctionCallInstructions(&inst_it));
                        add_func_call(GetTlasValidationFunctionCallInstructions(
                            function, 0, glsl::kErrorSubCode_TraceRay_TlasNotBuilt, current_block, &inst_it));
                        break;
                    }
                    case spv::OpHitObjectSetShaderBindingTableRecordIndexEXT: {
                        add_func_call(GetRayHitObjectSbtIndexValidationFunctionCallInstructions(&inst_it));
                        break;
                    }
                    case spv::OpHitObjectTraceRayEXT:
                    case spv::OpHitObjectTraceReorderExecuteEXT:
                    case spv::OpHitObjectTraceRayMotionEXT:
                    case spv::OpHitObjectTraceMotionReorderExecuteEXT: {
                        add_func_call(GetRayHitObjectValidationFunctionCallInstructions(&inst_it));
                        break;
                    }
                    default:
                        break;
                }

                if (func_calls.empty()) {
                    continue;
                }

                ++instrumentations_count_;

                if (!module_.settings_.safe_mode) {
                    for (auto& func_call : func_calls) {
                        AddFunctionCall(current_block, std::move(func_call), &inst_it);
                    }
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    uint32_t combined_func_results_id = AddFunctionCall(current_block, std::move(func_calls[0]), nullptr);
                    for (size_t i = 1; i < func_calls.size(); ++i) {
                        const uint32_t next_func_result_id = AddFunctionCall(current_block, std::move(func_calls[i]), nullptr);
                        const uint32_t next_combined_func_results_id = module_.TakeNextId();
                        current_block.CreateInstruction(spv::OpLogicalAnd,
                                                        {type_manager_.GetTypeBool().Id(), next_combined_func_results_id,
                                                         combined_func_results_id, next_func_result_id});
                        combined_func_results_id = next_combined_func_results_id;
                    }
                    ic_data.function_result_id = combined_func_results_id;
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    ++block_it;
                    ++block_it;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void TraceRayPass::PrintDebugInfo() const {
    std::cout << "TraceRayPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
