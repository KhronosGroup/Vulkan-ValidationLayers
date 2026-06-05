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
#include "link.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "pass.h"

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
};

DescriptorHeapPass::DescriptorHeapPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

uint32_t DescriptorHeapPass::GetLinkFunctionId(const FunctionNames func_name) {
    return GetLinkFunction(link_function_id_[func_name], kOfflineFunction[func_name]);
}

// TODO - if we find this slow, we could pre-sort all the mappings
const VkDescriptorSetAndBindingMappingEXT* DescriptorHeapPass::GetMapping(uint32_t descriptor_set, uint32_t descriptor_binding) {
    for (uint32_t map_i = 0; map_i < module_.interface_.mapping_info->mappingCount; map_i++) {
        const VkDescriptorSetAndBindingMappingEXT& mapping = module_.interface_.mapping_info->pMappings[map_i];
        const uint32_t last_binding = mapping.firstBinding + mapping.bindingCount;
        if (mapping.descriptorSet == descriptor_set && descriptor_binding >= mapping.firstBinding &&
            descriptor_binding < last_binding) {
            return &mapping;
        }
    }
    return nullptr;
}

uint32_t DescriptorHeapPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                                bool is_sampler) {
    const DescriptorMeta descriptor_meta = is_sampler ? meta.sampler : meta.resource;
    const uint32_t descriptor_index_id = CastToUint32(descriptor_meta.descriptor_index_id, block, inst_it);  // might be int32

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t is_sampler_id = type_manager_.GetConstantBool(is_sampler).Id();

    uint32_t function_result = module_.TakeNextId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    // TODO - This logic is not obvious and should be either fixed or moved to a common util
    // If dealing with a seperate sampler, only need to do it on the resource
    if (meta.image_inst && !is_sampler) {
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
                    block.function_->CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id);
                }
            }
        }
    }

    const VkDescriptorSetAndBindingMappingEXT* mapping =
        GetMapping(descriptor_meta.descriptor_set, descriptor_meta.descriptor_binding);
    if (!mapping) {
        // TODO - handle untyped
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
    if (!is_sampler && meta.HasSampler()) {
        const uint32_t valid_image = function_result;
        uint32_t valid_sampler = 0;
        if (meta.is_combined_image_sampler) {
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
    const spv::Op opcode = (spv::Op)inst.Opcode();

    const Instruction* sampler_load_inst = nullptr;
    if (opcode == spv::OpAtomicLoad || opcode == spv::OpAtomicStore || opcode == spv::OpAtomicExchange) {
        // Image Atomics
        const Instruction* image_texel_ptr_inst = function.FindInstruction(inst.Operand(0));
        if (!image_texel_ptr_inst || image_texel_ptr_inst->Opcode() != spv::OpImageTexelPointer) {
            return false;
        }

        const Variable* variable = nullptr;
        const Instruction* access_chain_inst = function.FindInstruction(image_texel_ptr_inst->Operand(0));
        if (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
            variable = type_manager_.FindVariableById(access_chain_inst->Operand(0));
        } else {
            // if no array, will point right to a variable
            variable = type_manager_.FindVariableById(image_texel_ptr_inst->Operand(0));
        }

        if (!variable) {
            return false;
        }
        meta.resource.var_inst = &variable->inst_;

        const Type* pointer_type = variable->PointerType(type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        const bool non_empty_access_chain = access_chain_inst && access_chain_inst->Length() >= 5;
        if (pointer_type->IsArray() && non_empty_access_chain) {
            meta.resource.descriptor_index_id = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            meta.resource.descriptor_index_id = type_manager_.GetConstantZeroUint32().Id();
        }
    } else if (IsValueIn(opcode, {spv::OpLoad, spv::OpStore, spv::OpCooperativeMatrixLoadKHR, spv::OpCooperativeMatrixStoreKHR}) ||
               AtomicOperation(opcode)) {
        // Buffer and Buffer Atomics and Storage Images

        const AccessPath access_path = type_manager_.BuildAccessPath(function, inst);
        if (!access_path.IsValid()) {
            return false;
        }
        meta.resource.var_inst = &access_path.variable->inst_;

        const uint32_t storage_class = access_path.variable->StorageClass();
        if (storage_class == spv::StorageClassUniformConstant) {
            // TODO - Need to add Storage Image support
            return false;
        }
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;  // Prevents things like Push Constants
        }

        const Type* pointer_type = access_path.variable->PointerType(type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        if (pointer_type->IsArray()) {
            meta.resource.descriptor_index_id = access_path.DescriptorIndexId();
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            meta.resource.descriptor_index_id = type_manager_.GetConstantZeroUint32().Id();
        }
    } else {
        // sampled image (non-atomic)

        // Reference is not load or store, so if it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }

        // Things that have an OpImage (in OpcodeImageAccessPosition) but we don't want to handle
        if (opcode == spv::OpImageRead || opcode == spv::OpImageWrite) {
            return false;  // Storage Images are handled at OpLoad
        } else if (opcode == spv::OpImageTexelPointer) {
            return false;  // atomics are handled separately
        } else if (opcode == spv::OpImage) {
            return false;  // Don't deal with the access directly
        }

        meta.image_inst = function.FindInstruction(inst.Word(image_word));
        const Instruction* load_inst = meta.image_inst;
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            const uint32_t load_operand = load_inst->Operand(0);
            load_inst = function.FindInstruction(load_operand);

            if (!load_inst) {
                assert(type_manager_.IsUndef(load_operand));
                return false;  //
            } else if (load_inst->Opcode() == spv::OpSampledImage) {
                sampler_load_inst = function.FindInstruction(load_inst->Operand(1));
            }
        }

        if (!sampler_load_inst && meta.image_inst->Opcode() == spv::OpSampledImage) {
            sampler_load_inst = function.FindInstruction(meta.image_inst->Operand(1));
        }

        // If we can't find a seperate sampler, and non sampled images are check elsewhere
        // we know this is actually a combined image sampler
        meta.is_combined_image_sampler = sampler_load_inst == nullptr;

        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            return false;  // TODO: Handle additional possibilities?
        }

        meta.resource.var_inst = function.FindInstruction(load_inst->Operand(0));
        if (!meta.resource.var_inst) {
            // can be a global variable
            const Variable* global_var = type_manager_.FindVariableById(load_inst->Operand(0));
            meta.resource.var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!meta.resource.var_inst ||
            (!meta.resource.var_inst->IsNonPtrAccessChain() && meta.resource.var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.resource.var_inst->IsNonPtrAccessChain()) {
            meta.resource.descriptor_index_id = meta.resource.var_inst->Operand(1);

            if (meta.resource.var_inst->Length() > 5) {
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = type_manager_.FindVariableById(meta.resource.var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            meta.resource.var_inst = &variable->inst_;
        } else {
            meta.resource.descriptor_index_id = type_manager_.GetConstantZeroUint32().Id();
        }
    }

    assert(meta.resource.var_inst);
    uint32_t variable_id = meta.resource.var_inst->ResultId();
    GetDescriptorSetAndBinding(variable_id, meta.resource.descriptor_set, meta.resource.descriptor_binding);

    // When using a SAMPLED_IMAGE and SAMPLER, they are accessed together so we need check for 2 descriptors at the same time
    // TODO - This is currently 95% the same logic as above, find a way to combine it
    if (sampler_load_inst && sampler_load_inst->Opcode() == spv::OpLoad) {
        meta.sampler.var_inst = function.FindInstruction(sampler_load_inst->Operand(0));
        if (!meta.sampler.var_inst) {
            // can be a global variable
            const Variable* global_var = type_manager_.FindVariableById(sampler_load_inst->Operand(0));
            meta.sampler.var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!meta.sampler.var_inst ||
            (!meta.sampler.var_inst->IsNonPtrAccessChain() && meta.sampler.var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.sampler.var_inst->IsNonPtrAccessChain()) {
            meta.sampler.descriptor_index_id = meta.sampler.var_inst->Operand(1);

            if (meta.sampler.var_inst->Length() > 5) {
                module_.InternalError(Name(), "Sampler OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = type_manager_.FindVariableById(meta.sampler.var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "Sampler OpAccessChain base is not a variable");
                return false;
            }
            meta.sampler.var_inst = &variable->inst_;
        } else {
            meta.sampler.descriptor_index_id = type_manager_.GetConstantZeroUint32().Id();
        }

        variable_id = meta.sampler.var_inst->ResultId();
        GetDescriptorSetAndBinding(variable_id, meta.sampler.descriptor_set, meta.sampler.descriptor_binding);
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
                    if (meta.HasSampler()) {
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