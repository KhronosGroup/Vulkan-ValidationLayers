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

#include "buffer_device_address_pass.h"
#include "access_path.h"
#include "link.h"
#include "module.h"
#include "access_path.h"
#include <cassert>
#include <spirv/unified1/spirv.hpp>
#include <iostream>
#include "utils/math_utils.h"
#include "utils/assert_utils.h"
#include "gpuav/shaders/gpuav_error_header.h"

#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_buffer_device_address_comp,
                                             instrumentation_buffer_device_address_comp_size,
                                             ZeroInitializeUintPrivateVariables | UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunctionRange = {"inst_buffer_device_address_range",
                                                      instrumentation_buffer_device_address_comp_function_0_offset};
const static OfflineFunction kOfflineFunctionAlign = {"inst_buffer_device_address_align",
                                                      instrumentation_buffer_device_address_comp_function_1_offset};

BufferDeviceAddressPass::BufferDeviceAddressPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t BufferDeviceAddressPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    // The Pointer ID Operand is always the first operand for Load/Store/Atomics
    // We can just take it and cast to a uint64 here to examine the ptr value
    const uint32_t pointer_id = meta.target_instruction->Operand(0);

    // Convert reference pointer to uint64
    const Type& uint64_type = type_manager_.GetTypeInt(64, 0);
    const uint32_t address_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpConvertPtrToU, {uint64_type.Id(), address_id, pointer_id}, inst_it);

    const uint32_t access_size_id = type_manager_.GetConstantUInt32(meta.access_size).Id();
    const uint32_t opcode = meta.target_instruction->Opcode();

    uint32_t access_type_value = 0;
    if (opcode == spv::OpStore) {
        access_type_value |= 1 << glsl::kInst_BuffAddrAccess_PayloadShiftIsWrite;
    }
    if (meta.type_is_struct) {
        access_type_value |= 1 << glsl::kInst_BuffAddrAccess_PayloadShiftIsStruct;
    }
    const Constant& access_type = type_manager_.GetConstantUInt32(access_type_value);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    uint32_t function_range_result = 0;  // only take next ID if needed
    const uint32_t function_range_id = GetLinkFunction(function_range_id_, kOfflineFunctionRange);

    if (module_.settings_.safe_mode || block_skip_list_.find(inst_position) == block_skip_list_.end()) {
        // "normal" check
        function_range_result = module_.TakeNextId();
        block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, function_range_result, function_range_id, inst_position_id, address_id, access_type.Id(), access_size_id},
            inst_it);
    } else {
        // Find if this is the lowest pointer access in the struct
        for (const auto& [struct_id, range] : block_struct_range_map_) {
            // This is only for unsafe mode, so we can ignore all other instructions
            if (range.min_instruction != inst_position) {
                continue;
            }
            ASSERT_AND_CONTINUE(range.max_struct_offsets >= range.min_struct_offsets);

            // If there is only a single access found, range diff is zero and this becomes a "normal" check automatically
            const uint32_t full_access_range = (range.max_struct_offsets - range.min_struct_offsets) + meta.access_size;
            const uint32_t full_range_id = type_manager_.GetConstantUInt32(full_access_range).Id();
            function_range_result = module_.TakeNextId();
            block.CreateInstruction(spv::OpFunctionCall,
                                    {bool_type, function_range_result, function_range_id, inst_position_id, address_id,
                                     access_type.Id(), full_range_id},
                                    inst_it);
            break;
        }
    }

    const Constant& alignment_constant = type_manager_.GetConstantUInt32(meta.access_path->bda_alignment);

    const uint32_t function_align_result = module_.TakeNextId();
    const uint32_t function_align_id = GetLinkFunction(function_align_id_, kOfflineFunctionAlign);
    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_align_result, function_align_id, inst_position_id, address_id, access_type.Id(),
                             alignment_constant.Id()},
                            inst_it);

    module_.need_log_error_ = true;

    // Will return bool that will look like (FuncRange() && FuncAlign()) { }
    if (module_.settings_.safe_mode) {
        const uint32_t logical_and_id = module_.TakeNextId();
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type, logical_and_id, function_range_result, function_align_result},
                                inst_it);
        return logical_and_id;
    }
    return 0;  // unsafe mode, we don't care what this is
}

bool BufferDeviceAddressPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    meta.access_path = module_.GetAccessPath(function, inst);
    if (!meta.access_path || !meta.access_path->IsValidBda()) {
        return false;
    }

    // This might be an OpTypeStruct, even if some compilers are smart enough (know Mesa is) to detect only the first part of a
    // struct is loaded, we have to assume the entire struct is loaded and the entire memory is accessed (see
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8089)
    meta.access_size = type_manager_.GetTypeBytesSize(*meta.access_path->access_type);
    // Will mark this is a struct acess to inform the user
    meta.type_is_struct = meta.access_path->access_type->spv_type_ == SpvType::kStruct;

    meta.target_instruction = &inst;
    return true;
}

bool BufferDeviceAddressPass::Instrument() {
    // TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12629
    // We need to just move BDA over to use AccessPath instead so we can find this information there
    // Then we can remove this function and burn it
    //
    // We need to detect structs with a VERY specific case
    // If there is an array-of-structs where that struct has a runtime array inside of it
    vvl::unordered_set<uint32_t> struct_id_temp_workaround;
    if (!module_.settings_.safe_mode) {
        type_manager_.FindArrayOfPSBStructWithRuntime(struct_id_temp_workaround);
    }

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

            if (!module_.settings_.safe_mode) {
                // Pre-Pass optimization where we detect statically all the offsets inside a BDA Struct that are accessed.
                // From here we can create a range and only do the check once since there is no real way to split a VkBuffer mid
                // struct.
                block_struct_range_map_.clear();
                block_skip_list_.clear();
                for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                    InstructionMeta meta;
                    if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                        continue;
                    }

                    if (meta.access_path->ac_list.empty()) {
                        continue;
                    }
                    const Instruction* last_access = meta.access_path->ac_list.front();
                    const uint32_t access_chain_base_id = last_access->Operand(0);
                    const Instruction* next_inst = function.FindInstruction(access_chain_base_id);

                    const Type* load_type_pointer = type_manager_.FindTypeById(next_inst->TypeId());
                    if (!load_type_pointer || !load_type_pointer->IsBDA()) {
                        assert(false);
                        continue;
                    }

                    const Type* struct_type = type_manager_.FindTypeById(load_type_pointer->inst_.Operand(1));
                    if (struct_type && struct_type->spv_type_ == SpvType::kStruct) {
                        uint32_t root_struct_id = struct_type->Id();
                        if (struct_id_temp_workaround.find(root_struct_id) != struct_id_temp_workaround.end()) {
                            continue;
                        }

                        // GLSL/HLSL will only ever use structs, but for Slang, we might have the first access be the pointer and
                        // we actually need that outer struct, which "looks" an OpTypePointer with an ArrayStride attached to it.
                        // aka. we have a pointer-of-structs and need to use that to get the offset
                        if (!last_access->IsNonPtrAccessChain() && last_access->TypeId() == load_type_pointer->Id()) {
                            root_struct_id = load_type_pointer->Id();
                        }

                        const uint32_t struct_offset =
                            FindOffsetInStruct(root_struct_id, nullptr, false, meta.access_path->ac_list);
                        if (struct_offset == 0) {
                            continue;
                        }
                        uint32_t inst_position = meta.target_instruction->GetPositionOffset();
                        block_skip_list_.insert(inst_position);

                        Range& range = block_struct_range_map_[struct_type->Id()];
                        if (struct_offset < range.min_struct_offsets) {
                            range.min_instruction = inst_position;
                            range.min_struct_offsets = struct_offset;
                        }
                        range.max_struct_offsets = std::max(range.max_struct_offsets, struct_offset);
                    }
                }
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    block_it++;
                    block_it++;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void BufferDeviceAddressPass::PrintDebugInfo() const {
    std::cout << "BufferDeviceAddressPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav