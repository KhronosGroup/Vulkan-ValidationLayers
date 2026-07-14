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
#include "chassis/dispatch_object.h"
#include "containers/container_utils.h"
#include "generated/spirv_grammar_helper.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "instrumentation_status.h"
#include "link.h"
#include "module.h"
#include <cassert>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "pass.h"
#include "type_manager.h"
#include "utils/descriptor_utils.h"
#include "utils/math_utils.h"

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
    {"inst_heap_descriptor_hashing", instrumentation_descriptor_heap_comp_function_9_offset},
};

DescriptorHeapPass::DescriptorHeapPass(Module& module)
    : Pass(module, kOfflineModule), descriptor_heap_props(module.settings_.phys_dev_ext_props->descriptor_heap_props) {
    module.use_bda_ = true;
}

uint32_t DescriptorHeapPass::GetLinkFunctionId(const FunctionNames func_name) {
    return GetLinkFunction(link_function_id_[func_name], kOfflineFunction[func_name]);
}

// Unfortunately this is a duplicate of the util function because there is no spirv::ResourceInterfaceVariable
bool DescriptorHeapPass::ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type, const AccessPath& access_path,
                                                    bool is_sampler) const {
    if (resource_type == VK_SPIRV_RESOURCE_TYPE_ALL_EXT) {
        return true;
    }

    // cant use |descriptor_type| as its only for the image resource here
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) != 0 && is_sampler) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT) != 0 &&
        (access_path.descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)) {
        return true;
    }
    if (access_path.descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
        access_path.descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ||
        access_path.descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        // NonWritable must be on the OpVariable, not the OpTypeStruct
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4789
        const bool nonwritable = GetDecoration(access_path.variable->Id(), spv::DecorationNonWritable) != nullptr;
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT) != 0 && nonwritable) {
            return true;
        }
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT) != 0 && !nonwritable) {
            return true;
        }
    }

    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT) != 0 && access_path.is_combined_image_sampler) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT) != 0 &&
        access_path.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        return true;
    }
    if (access_path.descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        // NonWritable must be on the OpVariable, not the OpTypeStruct
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4789
        const bool nonwritable = GetDecoration(access_path.variable->Id(), spv::DecorationNonWritable) != nullptr;
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT) != 0 && nonwritable) {
            return true;
        }
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT) != 0 && !nonwritable) {
            return true;
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_ACCELERATION_STRUCTURE_BIT_EXT) != 0 &&
        access_path.descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        return true;
    }

    // TODO - VK_SPIRV_RESOURCE_TYPE_TENSOR_BIT_ARM

    return false;
}

// TODO - if we find this slow, we could pre-sort all the mappings
const uint32_t kMappingIndexInvalid = 0xFFFFFFFF;
// TODO - Handle SHADER_RECORD
const uint32_t kMappingIndexShaderRecord = 0xFFFFFFFE;
uint32_t DescriptorHeapPass::GetMapping(const AccessPath& access_path, bool is_sampler) const {
    const Variable& variable = is_sampler ? *access_path.sampler_variable : *access_path.variable;
    const DescriptorInterface& interface = variable.interface_;
    if (interface.IsHeap()) {
        return glsl::kInst_DescriptorHeap_MappingIndexUntyped;
    }

    for (uint32_t i = 0; i < module_.interface_.mapping_info->mappingCount; i++) {
        const VkDescriptorSetAndBindingMappingEXT& mapping = module_.interface_.mapping_info->pMappings[i];
        // bindingCount could be UINT32_MAX
        const uint64_t last_binding = mapping.firstBinding + uint64_t(mapping.bindingCount);
        if (mapping.descriptorSet == interface.set && interface.binding >= mapping.firstBinding &&
            interface.binding < last_binding && ResourceTypeMatchesBinding(mapping.resourceMask, access_path, is_sampler)) {
            if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT ||
                mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT ||
                mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
                return kMappingIndexShaderRecord;
            }

            const uint32_t encode_index = (uint32_t)module_.out_status.device.heap_mappings.size();
            module_.out_status.device.heap_mappings.emplace_back(HeapMappingStatus{i, interface.binding, variable.Id(), mapping});

            if (encode_index >= glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
                module_.InternalWarning(Name(),
                                        "Tried to use an index into VkShaderDescriptorSetAndBindingMappingInfoEXT::pMapping that "
                                        "is over 64k and not able to "
                                        "track. Please report an issue as we made assumption no one would this many mappings!");
            }
            return encode_index;
        }
    }

    assert(false);  // caught by things like VU 11292
    return kMappingIndexInvalid;
}

uint32_t DescriptorHeapPass::GetMinBufferAlignment(const InstructionMeta& meta) const {
    const bool is_uniform = meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    const VkDeviceSize buffer_alignment = is_uniform ? module_.settings_.phys_dev_props->limits.minUniformBufferOffsetAlignment
                                                     : module_.settings_.phys_dev_props->limits.minStorageBufferOffsetAlignment;
    return type_manager_.GetConstantUInt32((uint32_t)buffer_alignment).Id();
}

uint32_t DescriptorHeapPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                                bool is_seperate_sampler) {
    // TODO - This logic is not obvious and should be either fixed or moved to a common util
    // If dealing with a seperate sampler, only need to do it on the resource
    // To add to the fire, this needs to go first otherwise the Function::CreateInstruction will break the inst_it
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

    const Variable& descriptor_variable = is_seperate_sampler ? *meta.access_path.sampler_variable : *meta.access_path.variable;
    const uint32_t descriptor_index =
        is_seperate_sampler ? meta.access_path.sampler_descriptor_index_id : meta.access_path.descriptor_index_id;
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index, block, inst_it);  // might be int32

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t is_sampler_id = type_manager_.GetConstantBool(is_seperate_sampler).Id();

    const VkDescriptorSetAndBindingMappingEXT* mapping = is_seperate_sampler ? meta.mapping_ptr_sampler : meta.mapping_ptr;

    // We try and encode a lot of information in a single uint32_t
    uint32_t desc_encoding_id = 0;
    const VkDescriptorType vk_desc_type = is_seperate_sampler ? VK_DESCRIPTOR_TYPE_SAMPLER : meta.access_path.descriptor_type;
    const uint32_t desc_size_value = (uint32_t)module_.settings_.cached_descriptor_size->GetSize(vk_desc_type);
    {
        uint8_t desc_type_mask = (uint8_t)GetMaskFromDescriptorType(vk_desc_type);

        const uint32_t desc_alignment_value = (uint32_t)GetDescriptorHeapAlignment(descriptor_heap_props, vk_desc_type);
        const uint32_t desc_alignment_shift = GetAlignmentShift(desc_alignment_value);
        assert(desc_alignment_shift <= glsl::kInst_DescriptorHeap_AlignmentShiftMask);

        const uint32_t mapping_index_encoded = is_seperate_sampler ? meta.mapping_index_sampler : meta.mapping_index_resource;

        const uint32_t desc_encoding = (desc_type_mask << glsl::kInst_DescriptorHeap_DescriptorTypeShift) |
                                       (desc_size_value << glsl::kInst_DescriptorHeap_DescriptorSizeShift) |
                                       (mapping_index_encoded << glsl::kInst_DescriptorHeap_MappingIndexShift) |
                                       (desc_alignment_shift & glsl::kInst_DescriptorHeap_AlignmentShiftMask);

        desc_encoding_id = type_manager_.GetConstantUInt32(desc_encoding).Id();
    }

    uint32_t function_result = module_.TakeNextId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();
    const uint32_t uint_type = type_manager_.GetTypeInt(32, 0).Id();
    bool return_uint = false;

    const uint32_t binding_offset = mapping ? descriptor_variable.interface_.binding - mapping->firstBinding : 0;
    const bool combined_index = meta.access_path.is_combined_image_sampler && mapping && HasCombinedImageSamplerIndex(*mapping);

    if (!mapping) {
        assert(descriptor_variable.interface_.IsHeap());  // Untyped
        uint32_t heap_offset_id = type_manager_.GetConstantZeroUint32().Id();
        if (meta.untyped_heap_offset_id != 0) {
            heap_offset_id = CastToUint32(meta.untyped_heap_offset_id, block, inst_it);  // might be int32
        }

        uint32_t array_stride_id = type_manager_.GetConstantZeroUint32().Id();
        if (meta.untyped_array_stride_id != 0) {
            array_stride_id = CastToUint32(meta.untyped_array_stride_id, block, inst_it);  // might be int32
        }

        const uint32_t function_def = GetLinkFunctionId(UNTYPED);

        block.CreateInstruction(spv::OpFunctionCall,
                                {uint_type, function_result, function_def, inst_position_id, heap_offset_id, array_stride_id,
                                 descriptor_index_id, desc_encoding_id, is_sampler_id},
                                inst_it);
        return_uint = true;
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping->sourceData.constantOffset;

        const uint32_t heap_offset = map_data.heapOffset + (binding_offset * map_data.heapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_CONSTANT_OFFSET);

        block.CreateInstruction(spv::OpFunctionCall,
                                {uint_type, function_result, function_def, inst_position_id, heap_offset_id, heap_array_stride_id,
                                 descriptor_index_id, desc_encoding_id, is_sampler_id},
                                inst_it);
        return_uint = true;
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping->sourceData.pushIndex;

        const uint32_t heap_offset = map_data.heapOffset + (binding_offset * map_data.heapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        // VU enforces pushOffset to multiple of 4, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t combined_index_id = type_manager_.GetConstantBool(combined_index).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_INDEX);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, heap_index_stride_id,
             heap_array_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
        return_uint = true;
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping->sourceData.indirectIndex;

        const uint32_t heap_offset = map_data.heapOffset + (binding_offset * map_data.heapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        // VU enforces pushOffset to multiple of 8, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.addressOffset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.heapArrayStride).Id();

        const uint32_t combined_index_id = type_manager_.GetConstantBool(combined_index).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, address_offset_id,
             heap_index_stride_id, heap_array_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
        return_uint = true;
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping->sourceData.indirectIndexArray;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        // VU enforces pushOffset to multiple of 8, and the GLSL is using a uint array
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t frist_index_offset = (map_data.addressOffset / 4) + binding_offset;
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(frist_index_offset).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.heapIndexStride).Id();

        const uint32_t combined_index_id = type_manager_.GetConstantBool(combined_index).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX_ARRAY);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, address_offset_id,
             heap_index_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
        return_uint = true;
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping->sourceData.heapData;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.heapOffset).Id();
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();

        assert(meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        const uint32_t buffer_alignment_id = GetMinBufferAlignment(meta);

        const uint32_t function_def = GetLinkFunctionId(MAPPING_RESOURCE_HEAP_DATA);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id,
                                 buffer_alignment_id, desc_encoding_id},
                                inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        // TODO - is there any GPU-AV needed for this mapping?
        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_DATA);
        block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def}, inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(mapping->sourceData.pushAddressOffset / 4).Id();

        const uint32_t buffer_alignment_id = GetMinBufferAlignment(meta);

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_ADDRESS);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, function_result, function_def, inst_position_id, push_offset_id, buffer_alignment_id, desc_encoding_id},
            inst_it);
    } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping->sourceData.indirectAddress;

        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(map_data.pushOffset / 4).Id();
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(map_data.addressOffset / 4).Id();

        const uint32_t buffer_alignment_id = GetMinBufferAlignment(meta);

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_ADDRESS);

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, push_offset_id, address_offset_id,
                                 buffer_alignment_id, desc_encoding_id},
                                inst_it);

    } else {
        assert(false);  // TODO - Shader Record
    }

    module_.need_log_error_ = true;

    // We return a uint (instead of bool) incase we need to read the offset for |descriptor_hashing|
    if (return_uint) {
        const uint32_t function_offset_result = function_result;
        function_result = module_.TakeNextId();
        if (module_.settings_.descriptor_hashing) {
            const uint32_t function_def = GetLinkFunctionId(DESCRIPTOR_HASHING);
            const uint32_t desc_size_id = type_manager_.GetConstantUInt32(desc_size_value).Id();
            block.CreateInstruction(spv::OpFunctionCall,
                                    {bool_type, function_result, function_def, inst_position_id, function_offset_result,
                                     desc_size_id, descriptor_index_id, desc_encoding_id, is_sampler_id},
                                    inst_it);
        } else {
            const uint32_t one_id = type_manager_.GetConstantOneUint32().Id();  // INVALID_RESULT
            block.CreateInstruction(spv::OpINotEqual, {bool_type, function_result, function_offset_result, one_id}, inst_it);
        }
    }

    return function_result;
}

// If there is a sampler, we have another descriptor at this spot we need to validate
uint32_t DescriptorHeapPass::CreateFunctionCallSampler(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta,
                                                       uint32_t non_sampler_result) {
    uint32_t valid_sampler = 0;
    if (meta.access_path.is_combined_image_sampler) {
        assert(meta.mapping_ptr);  // not allowed with untyped pointers
        valid_sampler =
            CreateFunctionCallCombinedSampler(block, inst_it, meta, *meta.mapping_ptr, meta.access_path.descriptor_index_id);
    } else {
        valid_sampler = CreateFunctionCall(block, inst_it, meta, true);
    }

    // This is just a dumb hack around iterators, for samplers we call a second function
    if (inst_it && meta.access_path.HasSampler()) {
        inst_it++;
    }

    uint32_t function_result = 0;
    if (module_.settings_.safe_mode) {
        const uint32_t bool_type = type_manager_.GetTypeBool().Id();
        function_result = module_.TakeNextId();
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type, function_result, non_sampler_result, valid_sampler}, inst_it);
    }
    return function_result;
}

// Sampler are annoying, decided this should just be its own function as its only used for 4 mappings
uint32_t DescriptorHeapPass::CreateFunctionCallCombinedSampler(BasicBlock& block, InstructionIt* inst_it,
                                                               const InstructionMeta& meta,
                                                               const VkDescriptorSetAndBindingMappingEXT& mapping,
                                                               uint32_t descriptor_index_id) {
    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();
    const uint32_t is_sampler_id = type_manager_.GetConstantBool(true).Id();

    descriptor_index_id = CastToUint32(descriptor_index_id, block, inst_it);  // might be int32

    uint32_t desc_encoding_id = 0;
    const uint32_t desc_size_value = (uint32_t)descriptor_heap_props.samplerDescriptorSize;
    {
        uint8_t desc_type_mask = (uint8_t)vvlDescriptorType::CombinedSampler;
        const uint32_t mapping_index_encoded = meta.mapping_index_resource;
        const uint32_t desc_alignment_shift = GetAlignmentShift((uint32_t)descriptor_heap_props.samplerDescriptorAlignment);

        const uint32_t desc_encoding = (desc_type_mask << glsl::kInst_DescriptorHeap_DescriptorTypeShift) |
                                       (desc_size_value << glsl::kInst_DescriptorHeap_DescriptorSizeShift) |
                                       (mapping_index_encoded << glsl::kInst_DescriptorHeap_MappingIndexShift) |
                                       (desc_alignment_shift & glsl::kInst_DescriptorHeap_AlignmentShiftMask);
        desc_encoding_id = type_manager_.GetConstantUInt32(desc_encoding).Id();
    }

    const bool combined_index = HasCombinedImageSamplerIndex(mapping);
    const uint32_t combined_index_id = type_manager_.GetConstantBool(combined_index).Id();

    const uint32_t binding_offset = meta.access_path.variable->interface_.binding - mapping.firstBinding;

    uint32_t function_result = module_.TakeNextId();
    const uint32_t uint_type = type_manager_.GetTypeInt(32, 0).Id();

    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping.sourceData.constantOffset;

        const uint32_t heap_offset = map_data.samplerHeapOffset + (binding_offset * map_data.samplerHeapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_CONSTANT_OFFSET);

        block.CreateInstruction(spv::OpFunctionCall,
                                {uint_type, function_result, function_def, inst_position_id, heap_offset_id, heap_array_stride_id,
                                 descriptor_index_id, desc_encoding_id, is_sampler_id},
                                inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping.sourceData.pushIndex;

        const uint32_t heap_offset = map_data.samplerHeapOffset + (binding_offset * map_data.samplerHeapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        const uint32_t push_offset = combined_index ? map_data.pushOffset : map_data.samplerPushOffset;
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(push_offset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_PUSH_INDEX);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, heap_index_stride_id,
             heap_array_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping.sourceData.indirectIndex;

        const uint32_t heap_offset = map_data.samplerHeapOffset + (binding_offset * map_data.samplerHeapArrayStride);
        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(heap_offset).Id();
        const uint32_t push_offset = combined_index ? map_data.pushOffset : map_data.samplerPushOffset;
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(push_offset / 4).Id();
        const uint32_t address_offset = combined_index ? map_data.addressOffset : map_data.samplerAddressOffset;
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(address_offset / 4).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapIndexStride).Id();
        const uint32_t heap_array_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapArrayStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, address_offset_id,
             heap_index_stride_id, heap_array_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping.sourceData.indirectIndexArray;

        const uint32_t heap_offset_id = type_manager_.GetConstantUInt32(map_data.samplerHeapOffset).Id();
        const uint32_t push_offset = combined_index ? map_data.pushOffset : map_data.samplerPushOffset;
        const uint32_t push_offset_id = type_manager_.GetConstantUInt32(push_offset / 4).Id();
        const uint32_t address_offset = combined_index ? map_data.addressOffset : map_data.samplerAddressOffset;
        const uint32_t frist_index_offset = (address_offset / 4) + binding_offset;
        const uint32_t address_offset_id = type_manager_.GetConstantUInt32(frist_index_offset).Id();
        const uint32_t heap_index_stride_id = type_manager_.GetConstantUInt32(map_data.samplerHeapIndexStride).Id();

        const uint32_t function_def = GetLinkFunctionId(MAPPING_INDIRECT_INDEX_ARRAY);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {uint_type, function_result, function_def, inst_position_id, heap_offset_id, push_offset_id, address_offset_id,
             heap_index_stride_id, descriptor_index_id, desc_encoding_id, is_sampler_id, combined_index_id},
            inst_it);
    }

    // For samplers, mappings can never be inlined mapping, so always will return a uint
    const uint32_t function_offset_result = function_result;
    function_result = module_.TakeNextId();
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();
    if (module_.settings_.descriptor_hashing) {
        const uint32_t function_def = GetLinkFunctionId(DESCRIPTOR_HASHING);
        const uint32_t desc_size_id = type_manager_.GetConstantUInt32(desc_size_value).Id();
        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, function_result, function_def, inst_position_id, function_offset_result, desc_size_id,
                                 descriptor_index_id, desc_encoding_id, is_sampler_id},
                                inst_it);
    } else {
        const uint32_t one_id = type_manager_.GetConstantOneUint32().Id();  // INVALID_RESULT
        block.CreateInstruction(spv::OpINotEqual, {bool_type, function_result, function_offset_result, one_id}, inst_it);
    }

    return function_result;
}

bool DescriptorHeapPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid() || !meta.access_path.variable->IsDescriptor()) {
        return false;
    }
    if (meta.access_path.descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        return false;  // not supported yet
    }

    // We look for mappings here incase we can't find it, we can skip safely
    meta.mapping_index_resource = GetMapping(meta.access_path, false);
    if (meta.access_path.sampler_variable) {
        meta.mapping_index_sampler = GetMapping(meta.access_path, true);
    }
    if (meta.mapping_index_resource == kMappingIndexInvalid || meta.mapping_index_sampler == kMappingIndexInvalid) {
        return false;
    } else if (meta.mapping_index_resource == kMappingIndexShaderRecord ||
               meta.mapping_index_sampler == kMappingIndexShaderRecord) {
        return false;
    }

    meta.target_instruction = &inst;

    if (meta.mapping_index_resource != glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
        meta.mapping_ptr = &module_.out_status.device.heap_mappings[meta.mapping_index_resource].mapping_data;
    }
    // Get to do it again... because samplers
    if (meta.access_path.sampler_variable) {
        if (meta.mapping_index_sampler != glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
            meta.mapping_ptr_sampler = &module_.out_status.device.heap_mappings[meta.mapping_index_sampler].mapping_data;
        }
    };

    bool has_embedded_sampler = false;
    if (meta.mapping_ptr_sampler) {
        has_embedded_sampler = GetEmbeddedSampler(*meta.mapping_ptr_sampler) != nullptr;
    } else if (meta.mapping_ptr) {
        has_embedded_sampler = GetEmbeddedSampler(*meta.mapping_ptr) != nullptr;
    }
    meta.instrument_seperate_sampler = meta.access_path.HasSampler() && !has_embedded_sampler;

    if (meta.mapping_index_resource == glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
        const Type* descriptor_array = nullptr;
        if (meta.access_path.pointer_type->spv_type_ == SpvType::kStruct) {
            const Instruction* offset_decoration = GetMemberDecoration(
                meta.access_path.pointer_type->Id(), meta.access_path.heap_offset_member_index, spv::DecorationOffsetIdEXT);
            if (offset_decoration) {
                meta.untyped_heap_offset_id = offset_decoration->Word(4);
            } else {
                // user has hardcoded offsets, if still can't find, then we can assume an implicit zero value
                offset_decoration = GetMemberDecoration(meta.access_path.pointer_type->Id(),
                                                        meta.access_path.heap_offset_member_index, spv::DecorationOffset);
                if (offset_decoration) {
                    meta.untyped_heap_offset_id = type_manager_.CreateConstantUInt32(offset_decoration->Word(4)).Id();
                }
            }

            descriptor_array =
                type_manager_.FindTypeById(meta.access_path.pointer_type->inst_.Operand(meta.access_path.heap_offset_member_index));
        } else {
            descriptor_array = meta.access_path.pointer_type;
        }

        if (descriptor_array->IsArray()) {
            const Type* element_type = type_manager_.FindTypeById(descriptor_array->inst_.Word(2));
            if (element_type->IsArray()) {
                // TODO - Handle MultidimensionalArray (GPU Dump has support for reference)
                return false;
            }
            const uint32_t array_stride_dec = GetDecoration(descriptor_array->Id(), spv::DecorationArrayStrideIdEXT)->Word(3);
            const Constant* array_stride = type_manager_.FindConstantById(array_stride_dec);
            meta.untyped_array_stride_id = array_stride->Id();
        }
    }

    return true;
}

uint32_t DescriptorHeapPass::InstructionMeta::Hash(const uint32_t descriptor_index, VkDescriptorType vk_type) const {
    const uint32_t source_id = mapping_ptr ? (uint32_t)mapping_ptr->source : 0;
    const uint32_t desc_type = (uint32_t)vk_type;

    uint32_t hash = 0;
    if (mapping_index_resource == glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
        uint32_t hash_content[4] = {desc_type, descriptor_index, untyped_heap_offset_id, untyped_array_stride_id};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 4);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping_ptr->sourceData.constantOffset;
        uint32_t hash_content[5] = {source_id, desc_type, descriptor_index, map_data.heapOffset, map_data.heapArrayStride};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 5);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping_ptr->sourceData.pushIndex;
        uint32_t hash_content[7] = {
            source_id,           desc_type,          descriptor_index, map_data.heapOffset, map_data.heapArrayStride,
            map_data.heapOffset, map_data.pushOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 7);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping_ptr->sourceData.indirectIndex;
        uint32_t hash_content[8] = {
            source_id,           desc_type,           descriptor_index,      map_data.heapOffset, map_data.heapArrayStride,
            map_data.heapOffset, map_data.pushOffset, map_data.addressOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 8);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping_ptr->sourceData.indirectIndexArray;
        uint32_t hash_content[7] = {source_id,           desc_type,           descriptor_index,      map_data.heapOffset,
                                    map_data.heapOffset, map_data.pushOffset, map_data.addressOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 7);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping_ptr->sourceData.heapData;
        uint32_t hash_content[4] = {source_id, desc_type, map_data.heapOffset, map_data.pushOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 4);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        uint32_t hash_content[3] = {source_id, desc_type, mapping_ptr->sourceData.pushDataOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        uint32_t hash_content[3] = {source_id, desc_type, mapping_ptr->sourceData.pushAddressOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
    } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping_ptr->sourceData.indirectAddress;
        uint32_t hash_content[4] = {source_id, desc_type, map_data.pushOffset, map_data.addressOffset};
        hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 4);
    }

    if (access_path.is_combined_image_sampler) {
        if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
            const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping_ptr->sourceData.constantOffset;
            uint32_t hash_content[3] = {hash, map_data.samplerHeapOffset, map_data.samplerHeapArrayStride};
            hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
        } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
            const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping_ptr->sourceData.pushIndex;
            uint32_t hash_content[5] = {hash, map_data.samplerHeapOffset, map_data.samplerHeapArrayStride,
                                        map_data.samplerHeapOffset, map_data.samplerPushOffset};
            hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 5);
        } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
            const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping_ptr->sourceData.indirectIndex;
            uint32_t hash_content[6] = {hash,
                                        map_data.samplerHeapOffset,
                                        map_data.samplerHeapArrayStride,
                                        map_data.samplerHeapOffset,
                                        map_data.samplerPushOffset,
                                        map_data.samplerAddressOffset};
            hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 6);
        } else if (mapping_ptr->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
            const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping_ptr->sourceData.indirectIndexArray;
            uint32_t hash_content[5] = {hash, map_data.samplerHeapOffset, map_data.samplerHeapOffset, map_data.samplerPushOffset,
                                        map_data.samplerAddressOffset};
            hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 5);
        }
    }

    return hash;
}

bool DescriptorHeapPass::Instrument() {
    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        FunctionDuplicateTracker function_duplicate_tracker;

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

            // We only need to instrument the set/binding/index combo once per block (in unsafe mode)
            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);
            DescriptroIndexPushConstantAccess pc_access;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (!module_.settings_.safe_mode) {
                    pc_access.Update(module_, inst_it);
                }

                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                bool skip_resource = false;
                bool skip_sampler = !meta.instrument_seperate_sampler;
                if (!module_.settings_.safe_mode) {
                    const uint32_t hash_descriptor_index_id = pc_access.next_alias_id == meta.access_path.descriptor_index_id
                                                                  ? pc_access.descriptor_index_id
                                                                  : meta.access_path.descriptor_index_id;
                    const uint32_t resource_hash = meta.Hash(hash_descriptor_index_id, meta.access_path.descriptor_type);
                    if (resource_hash != 0 && function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, resource_hash)) {
                        skip_resource = true;
                    }

                    if (meta.instrument_seperate_sampler) {
                        const uint32_t sampler_hash =
                            meta.Hash(meta.access_path.sampler_descriptor_index_id, VK_DESCRIPTOR_TYPE_SAMPLER);
                        if (sampler_hash != 0 && function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, sampler_hash)) {
                            skip_sampler = true;
                        }
                    }

                    if (skip_resource && skip_sampler) {
                        continue;  // duplicate detected
                    }
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    if (!skip_resource) {
                        CreateFunctionCall(current_block, &inst_it, meta, false);
                    }
                    if (!skip_sampler) {
                        CreateFunctionCallSampler(current_block, &inst_it, meta, 0);
                    }
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta, false);
                    if (meta.instrument_seperate_sampler) {
                        ic_data.function_result_id =
                            CreateFunctionCallSampler(current_block, nullptr, meta, ic_data.function_result_id);
                    }
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