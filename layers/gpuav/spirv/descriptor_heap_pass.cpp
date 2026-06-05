/* Copyright (c) 2026 LunarG, Inc.
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

#include "descriptor_heap_pass.h"
#include <vulkan/vulkan_core.h>
#include "containers/container_utils.h"
#include "generated/spirv_grammar_helper.h"
#include "link.h"
#include "module.h"
#include <cassert>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "pass.h"
#include "type_manager.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_heap_comp, instrumentation_descriptor_heap_comp_size,
                                             UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction[] = {
    {"inst_heap_mapping_constant_offset", instrumentation_descriptor_heap_comp_function_0_offset},
    {"inst_heap_mapping_push_index", instrumentation_descriptor_heap_comp_function_1_offset},
    {"inst_heap_mapping_indirect_index", instrumentation_descriptor_heap_comp_function_2_offset},
    {"inst_heap_mapping_indirect_index_array", instrumentation_descriptor_heap_comp_function_3_offset},
    {"inst_heap_mapping_resource_heap_data", instrumentation_descriptor_heap_comp_function_4_offset},
    {"inst_heap_mapping_push_data", instrumentation_descriptor_heap_comp_function_5_offset},
    {"inst_heap_mapping_push_address", instrumentation_descriptor_heap_comp_function_6_offset},
    {"inst_heap_mapping_indirect_address", instrumentation_descriptor_heap_comp_function_7_offset},
    {"inst_heap_untyped", instrumentation_descriptor_heap_comp_function_8_offset},
};

DescriptorHeapPass::DescriptorHeapPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t DescriptorHeapPass::GetLinkFunctionId(const FunctionNames func_name) {
    return GetLinkFunction(link_function_id_[func_name], kOfflineFunction[func_name]);
}

// TODO - if we find this slow, we could pre-sort all the mappings
const VkDescriptorSetAndBindingMappingEXT* DescriptorHeapPass::GetMapping(const DescriptorInterface& interface) const {
    if (interface.IsHeap()) {
        return nullptr;
    }
    for (uint32_t map_i = 0; map_i < module_.interface_.mapping_info->mappingCount; map_i++) {
        const VkDescriptorSetAndBindingMappingEXT& mapping = module_.interface_.mapping_info->pMappings[map_i];
        const uint32_t last_binding = mapping.firstBinding + mapping.bindingCount;
        if (mapping.descriptorSet == interface.set && interface.binding >= mapping.firstBinding &&
            interface.binding < last_binding) {
            return &mapping;
        }
    }
    return nullptr;
}

uint32_t DescriptorHeapPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                                bool is_seperate_sampler) {
    const Variable& descriptor_var = is_seperate_sampler ? *meta.access_path.sampler_variable : *meta.access_path.variable;
    const uint32_t descriptor_index =
        is_seperate_sampler ? meta.access_path.sampler_descriptor_index_id : meta.access_path.descriptor_index_id;
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index, block, inst_it);  // might be int32

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t is_sampler_id = type_manager_.GetConstantBool(is_seperate_sampler).Id();

    uint32_t function_result = module_.TakeNextId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    // TODO - This logic is not obvious and should be either fixed or moved to a common util
    // If dealing with a seperate sampler, only need to do it on the resource
    if (meta.access_path.image_load_inst && !is_seperate_sampler) {
        const uint32_t opcode = meta.target_instruction->Opcode();
        if (opcode != spv::OpImageRead && opcode != spv::OpImageFetch && opcode != spv::OpImageWrite) {
            // if not a direct read/write/fetch, will be a OpSampledImage
            // "All OpSampledImage instructions must be in the same block in which their Result <id> are consumed"
            // the simple way around this is to add a OpCopyObject to be consumed by the target instruction
            uint32_t image_id = meta.target_instruction->Operand(0);
            const Instruction* sampled_image_inst = block.function_->FindInstruction(image_id);
            // TODO - Add tests to understand what else can be here other then OpSampledImage
            if (sampled_image_inst->Opcode() == spv::OpSampledImage) {
                const uint32_t type_id = sampled_image_inst->TypeId();
                const uint32_t copy_id = module_.TakeNextId();
                const_cast<Instruction*>(meta.target_instruction)->ReplaceOperandId(image_id, copy_id);

                // incase the OpSampledImage is shared, copy the previous OpCopyObject
                auto copied = copy_object_map_.find(image_id);
                if (copied != copy_object_map_.end()) {
                    image_id = copied->second;
                    block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, inst_it);
                } else {
                    copy_object_map_.emplace(image_id, copy_id);
                    // slower, but need to guarantee it is placed after a OpSampledImage
                    block.function_->CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id, inst_it);
                }
            }
        }
    }

    assert(descriptor_var.IsDescriptor());
    const VkDescriptorSetAndBindingMappingEXT* mapping = GetMapping(descriptor_var.interface_);
    if (!mapping) {
        assert(descriptor_var.interface_.IsHeap());

        uint32_t heap_offset_id = 0;
        const Type* descriptor_array = nullptr;
        if (meta.access_path.pointer_type->spv_type_ == SpvType::kStruct) {
            const Instruction* offset_decoration = GetMemberDecoration(
                meta.access_path.pointer_type->Id(), meta.access_path.heap_offset_member_index, spv::DecorationOffsetIdEXT);
            assert(offset_decoration);
            heap_offset_id = offset_decoration->Word(4);
            heap_offset_id = CastToUint32(heap_offset_id, block, inst_it);

            descriptor_array =
                type_manager_.FindTypeById(meta.access_path.pointer_type->inst_.Operand(meta.access_path.heap_offset_member_index));
            assert(descriptor_array->IsArray());
        } else {
            assert(meta.access_path.pointer_type->IsArray());
            descriptor_array = meta.access_path.pointer_type;
            heap_offset_id = type_manager_.GetConstantZeroUint32().Id();
        }

        const uint32_t array_stride_dec = GetDecoration(descriptor_array->Id(), spv::DecorationArrayStrideIdEXT)->Word(3);
        const Constant* array_stride = type_manager_.FindConstantById(array_stride_dec);
        const uint32_t array_stride_id = CastToUint32(array_stride->Id(), block, inst_it);  // might be int32

        const uint32_t function_def = GetLinkFunctionId(UNTYPED);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, array_stride_id,
                                 descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping->sourceData.constantOffset;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_CONSTANT_OFFSET);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, heap_array_stride_id,
                                 descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping->sourceData.pushIndex;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        // VU enforces pushOffset to multiple of 4, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_INDEX);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 heap_index_stride_id, heap_array_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping->sourceData.indirectIndex;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        // VU enforces pushOffset to multiple of 8, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.addressOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 address_offset_id, heap_index_stride_id, heap_array_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping->sourceData.indirectIndexArray;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        // VU enforces pushOffset to multiple of 8, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.addressOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX_ARRAY);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 address_offset_id, heap_index_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping->sourceData.heapData;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_RESOURCE_HEAP_DATA);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        // TODO - is there any GPU-AV needed for this mapping?
        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_DATA);
        block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def}, inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        const uint32_t push_address_offset_id = type_manager_.GetConstantUInt32(mapping->sourceData.pushAddressOffset / 4).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_ADDRESS);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, push_address_offset_id}, inst_it);

    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping->sourceData.indirectAddress;

        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.addressOffset / 4).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_RESOURCE_HEAP_DATA);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, push_offset_id, address_offset_id},
                                inst_it);

    } else {
        assert(false);  // TODO - Shader Record
    }

    module_.need_log_error_ = true;

    // If there is a sampler, we have another descriptor at this spot we need to validate
    if (!is_seperate_sampler && meta.access_path.HasSampler()) {
        const uint32_t valid_image = function_result;
        uint32_t valid_sampler = 0;
        if (meta.access_path.is_combined_image_sampler) {
            assert(mapping);  // not allowed with untyped pointers
            valid_sampler = CreateFunctionCallCombinedSampler(block, inst_it, meta, *mapping, descriptor_index_id);
        } else {
            valid_sampler = CreateFunctionCall(block, inst_it, meta, true);
        }

        if (module_.settings_.safe_mode) {
            function_result = module_.TakeNextId();
            block.CreateInstruction(spv::OpLogicalAnd, {bool_type, function_result, valid_image, valid_sampler}, inst_it);
        }
    }

    return function_result;
}

// Sampler are annoying, decided this should just be its own function as its only used for 4 mappings
uint32_t DescriptorHeapPass::CreateFunctionCallCombinedSampler(BasicBlock& block, InstructionIt* inst_it,
                                                               const InstructionMeta& meta,
                                                               const VkDescriptorSetAndBindingMappingEXT& mapping,
                                                               const uint32_t descriptor_index_id) {
    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t is_sampler_id = type_manager_.GetConstantBool(true).Id();

    uint32_t function_result = module_.TakeNextId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping.sourceData.constantOffset;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.samplerHeapOffset).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_CONSTANT_OFFSET);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, heap_array_stride_id,
                                 descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping.sourceData.pushIndex;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_INDEX);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 heap_index_stride_id, heap_array_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping.sourceData.indirectIndex;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.samplerHeapOffset).Id();
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.samplerPushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.samplerAddressOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 address_offset_id, heap_index_stride_id, heap_array_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping.sourceData.indirectIndexArray;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.samplerHeapOffset).Id();
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.samplerPushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.samplerAddressOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapIndexStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX_ARRAY);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 address_offset_id, heap_index_stride_id, descriptor_index_id, is_sampler_id},
                                inst_it);
    }

    return function_result;
}

bool DescriptorHeapPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid() || !meta.access_path.variable->IsDescriptor()) {
        return false;
    }

    meta.target_instruction = &inst;

    return true;
}

bool DescriptorHeapPass::Instrument() {
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
                    CreateFunctionCall(current_block, &inst_it, meta, false);
                    // This is just a dumb hack around iterators, for samplers we call a second function
                    if (meta.access_path.HasSampler()) {
                        inst_it++;
                    }
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta, false);
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

void DescriptorHeapPass::PrintDebugInfo() const {
    std::cout << "DescriptorHeapPass instrumentation count: " << instrumentations_count_ << "\n";
}

}  // namespace spirv
}  // namespace gpuav