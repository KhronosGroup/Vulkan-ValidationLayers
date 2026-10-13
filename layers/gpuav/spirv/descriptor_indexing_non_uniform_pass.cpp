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

#include "descriptor_indexing_non_uniform_pass.h"
#include "access_path.h"
#include "chassis/dispatch_object.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "utils/hash_util.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_indexing_non_uniform_comp,
                                             instrumentation_descriptor_indexing_non_uniform_comp_size, UseErrorPayloadVariable};

const static OfflineFunction kOfflineFunction = {"inst_descriptor_indexing_non_uniform",
                                                 instrumentation_descriptor_indexing_non_uniform_comp_function_0_offset};

DescriptorIndexingNonUniformPass::DescriptorIndexingNonUniformPass(Module& module) : Pass(module, kOfflineModule) {}

// Without this capability, an index into an array of this descriptor type must be uniform
static spv::Capability NonUniformIndexingCapability(VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return spv::CapabilityUniformBufferArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return spv::CapabilityStorageBufferArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return spv::CapabilitySampledImageArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return spv::CapabilityStorageImageArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return spv::CapabilityInputAttachmentArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return spv::CapabilityUniformTexelBufferArrayNonUniformIndexing;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return spv::CapabilityStorageTexelBufferArrayNonUniformIndexing;
        default:
            return spv::CapabilityMax;
    }
}

// Compilers decorate different IDs (glslang decorates the index, access chain and load, but not the OpSampledImage),
// so a NonUniform anywhere between the access and the descriptor variable counts
bool DescriptorIndexingNonUniformPass::IsDecoratedNonUniform(const Function& function, uint32_t id,
                                                             bool follow_sampled_image_sampler) const {
    std::vector<uint32_t> ids = {id};
    while (!ids.empty()) {
        const uint32_t current_id = ids.back();
        ids.pop_back();
        if (non_uniform_ids_.find(current_id) != non_uniform_ids_.end()) {
            return true;
        }

        const Instruction* inst = function.FindInstruction(current_id);
        if (!inst) {
            continue;  // reached the variable
        }
        const uint32_t opcode = inst->Opcode();
        if (opcode == spv::OpSampledImage) {
            ids.push_back(inst->Operand(follow_sampled_image_sampler ? 1 : 0));
        } else if (opcode == spv::OpCopyObject || opcode == spv::OpLoad || opcode == spv::OpImage ||
                   opcode == spv::OpImageTexelPointer) {
            ids.push_back(inst->Operand(0));
        } else if (inst->IsAccessChain()) {
            // The base and every index
            for (uint32_t i = 0; i < inst->Length() - 3; i++) {
                ids.push_back(inst->Operand(i));
            }
        }
    }
    return false;
}

void DescriptorIndexingNonUniformPass::AddIndexCheck(InstructionMeta& meta, const Variable* variable, const Type* pointer_type,
                                                     uint32_t index_id, VkDescriptorType descriptor_type, bool is_decorated) const {
    // The decoration is enough, glslang never declares SampledImageArrayNonUniformIndexing for sampler arrays
    if (is_decorated || !variable || !pointer_type || !pointer_type->IsArray() || type_manager_.FindConstantById(index_id)) {
        return;
    }

    // AccessPath reports any OpImageTexelPointer as a storage image, the variable knows if it is a texel buffer
    const Type* element_type = type_manager_.FindTypeById(pointer_type->inst_.Operand(0));
    if (element_type && element_type->spv_type_ == SpvType::kImage) {
        descriptor_type = element_type->inst_.GetImageType();
    }

    uint32_t missing_capability = 0;
    const spv::Capability capability = NonUniformIndexingCapability(descriptor_type);
    if (capability != spv::CapabilityMax && !module_.HasCapability(capability)) {
        missing_capability = capability;
    } else if (has_descriptor_heap_capability_) {
        return;  // VUID-RuntimeSpirv-subgroupSize-10149 does not apply
    }

    meta.checks[meta.check_count++] = {variable, index_id, missing_capability};
}

bool DescriptorIndexingNonUniformPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                               InstructionMeta& meta) {
    const AccessPath* access_path = module_.GetAccessPath(function, inst);
    if (!access_path || !access_path->IsValidDescriptor()) {
        return false;
    }

    // Operand 0 is the pointer, image or sampled image being accessed
    const AccessPath::Descriptor& descriptor = access_path->descriptor;
    const bool is_decorated =
        non_uniform_ids_.find(inst.ResultId()) != non_uniform_ids_.end() || IsDecoratedNonUniform(function, inst.Operand(0), false);
    AddIndexCheck(meta, access_path->variable, access_path->pointer_type, descriptor.index_id, descriptor.type, is_decorated);

    if (descriptor.sampler_variable) {
        const bool is_sampler_decorated = IsDecoratedNonUniform(function, inst.Operand(0), true);
        AddIndexCheck(meta, descriptor.sampler_variable, descriptor.sampler_pointer_type, descriptor.sampler_index_id,
                      VK_DESCRIPTOR_TYPE_SAMPLER, is_sampler_decorated);
    }

    meta.target_instruction = &inst;
    return meta.check_count != 0;
}

void DescriptorIndexingNonUniformPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                          const Instruction& target_instruction, const IndexCheck& check) {
    const DescriptorInterface& interface = check.variable->interface_;
    const uint32_t set_id = type_manager_.GetConstantUInt32(interface.set).Id();
    const uint32_t binding_id = type_manager_.GetConstantUInt32(interface.binding).Id();
    const uint32_t missing_capability_id = type_manager_.GetConstantUInt32(check.missing_capability).Id();
    const uint32_t index_id = CastToUint32(check.index_id, block, inst_it);  // might be int32 or 64-bit
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(target_instruction.GetPositionOffset()).Id();

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunction(link_function_id_, kOfflineFunction);
    const uint32_t void_type = type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {void_type, function_result, function_def, inst_position_id, set_id, binding_id, index_id, missing_capability_id}, inst_it);

    module_.need_log_error_ = true;
    module_.need_subgroup_operations_ = true;
}

bool DescriptorIndexingNonUniformPass::Instrument() {
    if (module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    // Only stages with workgroups, elsewhere a subgroup can hold invocations from several draws, and an index that is uniform for
    // each draw is allowed to differ within the subgroup
    const VkShaderStageFlagBits stage = module_.interface_.entry_point_stage;
    if (stage != VK_SHADER_STAGE_COMPUTE_BIT && stage != VK_SHADER_STAGE_TASK_BIT_EXT && stage != VK_SHADER_STAGE_MESH_BIT_EXT) {
        return false;
    }

    const VkPhysicalDeviceSubgroupProperties& subgroup_props = module_.settings_.phys_dev_ext_props->subgroup_props;
    const VkSubgroupFeatureFlags required_operations = VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_VOTE_BIT;
    if ((subgroup_props.supportedOperations & required_operations) != required_operations ||
        (subgroup_props.supportedStages & stage) == 0) {
        return false;
    }

    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(2) == spv::DecorationNonUniform) {
            non_uniform_ids_.insert(annotation->Word(1));
        }
    }
    has_descriptor_heap_capability_ = module_.HasCapability(spv::CapabilityDescriptorHeapEXT);

    // Can safely loop function list as there is no injecting of new Functions until linking time
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

            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);

            auto& block_instructions = current_block.instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                InstructionMeta meta;
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    continue;
                }

                // No control flow is added, the call goes right before the access
                for (uint32_t i = 0; i < meta.check_count; i++) {
                    const IndexCheck& check = meta.checks[i];
                    uint32_t hash_content[4] = {check.variable->interface_.set, check.variable->interface_.binding, check.index_id,
                                                check.variable->Id()};
                    if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker,
                                                                 hash_util::Hash32(hash_content, sizeof(hash_content)))) {
                        continue;  // already checked in a block every invocation here went through
                    }

                    if (MaxInstrumentationsCountReached()) {
                        return instrumentations_count_ != 0;
                    }
                    instrumentations_count_++;

                    CreateFunctionCall(current_block, &inst_it, *meta.target_instruction, check);
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void DescriptorIndexingNonUniformPass::PrintDebugInfo() const {
    std::cout << "DescriptorIndexingNonUniformPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
