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

#include "descriptor_class_general_buffer_pass.h"
#include "generated/device_features.h"
#include "generated/spirv_grammar_helper.h"
#include "containers/container_utils.h"
#include "module.h"
#include <cassert>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "type_manager.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_class_general_buffer_comp,
                                             instrumentation_descriptor_class_general_buffer_comp_size, UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_descriptor_class_general_buffer",
                                                 instrumentation_descriptor_class_general_buffer_comp_function_0_offset};
const static OfflineFunction kOfflineFunctionCoopMat = {"inst_descriptor_class_general_buffer_coop_mat",
                                                        instrumentation_descriptor_class_general_buffer_comp_function_1_offset};

DescriptorClassGeneralBufferPass::DescriptorClassGeneralBufferPass(Module& module)
    : Pass(module, kOfflineModule),
      has_robustness(module.settings_.enabled_features->robustBufferAccess),
      has_coop_mat_robustness(module.settings_.enabled_features->cooperativeMatrixRobustBufferAccess) {
    module.use_bda_ = true;
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorClassGeneralBufferPass::GetLinkFunctionId(bool is_coop_mat) {
    if (is_coop_mat) {
        return GetLinkFunction(link_coop_mat_function_id_, kOfflineFunctionCoopMat);
    } else {
        return GetLinkFunction(link_function_id_, kOfflineFunction);
    }
}

void DescriptorClassGeneralBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    assert(!meta.access_path.ac_list.empty());
    const DescriptorInterface& interface = meta.access_path.variable->interface_;
    const Constant& desc_set_constant = type_manager_.GetConstantUInt32(interface.set);
    const uint32_t desc_index_id = CastToUint32(meta.access_path.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t descriptor_offset_id = GetLastByte(meta.access_path, block, inst_it);

    const auto& layout_lut = module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut;
    BindingLayout binding_layout = layout_lut[interface.set][interface.binding];
    const Constant& binding_layout_offset = type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    const uint32_t function_result = module_.TakeNextId();
    const bool is_coop_mat = meta.access_path.coop_mat.used;
    const uint32_t function_def = GetLinkFunctionId(is_coop_mat);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, inst_position_id, desc_set_constant.Id(), desc_index_id,
                             descriptor_offset_id, binding_layout_offset.Id()},
                            inst_it);

    module_.need_log_error_ = true;
}

bool DescriptorClassGeneralBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                               InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();

    // Only known way to access a UBO/SSBO
    if (!IsValueIn(spv::Op(opcode),
                   {spv::OpLoad, spv::OpStore, spv::OpCooperativeMatrixLoadKHR, spv::OpCooperativeMatrixStoreKHR}) &&
        !AtomicOperation(opcode)) {
        return false;
    }

    bool is_coop_mat = opcode == spv::OpCooperativeMatrixLoadKHR || opcode == spv::OpCooperativeMatrixStoreKHR;
    if ((is_coop_mat && has_coop_mat_robustness) || (!is_coop_mat && has_robustness)) {
        return false;
    }

    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid()) {
        return false;
    }

    if (meta.access_path.ac_list.empty()) {
        // Apperently thie is possible, but not sure if this means we should consider this offset zero into the buffer, as that
        // technically not allowed with typed pointers, so for now just skip these as seems no actual code generators produce this
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12618
        return false;
    }

    uint32_t storage_class = meta.access_path.variable->StorageClass();
    // The idea is General Buffer will not include any UniformConstant descriptor type
    if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
        return false;
    }

    if (meta.access_path.pointer_type->spv_type_ == SpvType::kRuntimeArray) {
        return false;  // TODO - Currently we mark these as "bindless"
    }

    const bool is_descriptor_array = meta.access_path.pointer_type->IsArray();
    meta.descriptor_block_type_id =
        is_descriptor_array ? meta.access_path.pointer_type->inst_.Operand(0) : meta.access_path.pointer_type->Id();
    assert(type_manager_.FindTypeById(meta.descriptor_block_type_id)->spv_type_ == SpvType::kStruct && "unexpected block type");

    if (meta.access_path.variable->interface_.set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    if (!module_.settings_.safe_mode) {
        meta.access_offset = FindOffsetInStruct(meta.descriptor_block_type_id, &meta.access_path.coop_mat, is_descriptor_array,
                                                meta.access_path.ac_list);
    }

    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    return true;
}

void DescriptorClassGeneralBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorClassGeneralBufferPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorClassGeneralBufferPass::Instrument() {
    if (module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            auto& block_instructions = current_block.instructions_;

            // < Descriptor SSA ID, Highest offset byte that will be accessed >
            vvl::unordered_map<uint32_t, uint32_t> block_highest_offset_map;

            if (!module_.settings_.safe_mode) {
                // Pre-pass loop the Block to get the highest offset accessed (statically known)
                // Do here before we inject instructions into the block list below
                for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                    InstructionMeta meta;
                    // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                    if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                        continue;
                    }

                    if (meta.access_offset != 0) {
                        // set offset for the first loop of the block
                        auto map_it = block_highest_offset_map.find(meta.descriptor_block_type_id);
                        if (map_it == block_highest_offset_map.end()) {
                            block_highest_offset_map[meta.descriptor_block_type_id] = meta.access_offset;
                        } else {
                            map_it->second = std::max(map_it->second, meta.access_offset);
                        }
                    }
                }
            }

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                if (!module_.settings_.safe_mode && meta.access_offset != 0) {
                    const uint32_t block_highest_offset = block_highest_offset_map[meta.descriptor_block_type_id];
                    if (meta.access_offset < block_highest_offset) {
                        continue;  // skipping because other instruction in block will be a higher offset
                    }
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    return instrumentations_count_ != 0;
}

void DescriptorClassGeneralBufferPass::PostProcess() {
    if (instrumentations_count_ > 75) {
        module_.InternalWarning(
            "GPUAV-Compile-time-general-buffer",
            "This shader will be very slow to compile and runtime performance may also be slow. This is due to the number of OOB "
            "checks for storage/uniform "
            "buffers. Turn on the |gpuav_force_on_robustness| setting to skip these checks and improve GPU-AV performance.");
    }
}

}  // namespace spirv
}  // namespace gpuav