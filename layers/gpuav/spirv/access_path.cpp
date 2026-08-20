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

#include "access_path.h"
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>
#include "containers/container_utils.h"
#include "generated/spirv_grammar_helper.h"
#include "module.h"
#include "utils/math_utils.h"

namespace gpuav {
namespace spirv {

bool AccessPath::IsValid(spv::StorageClass storage_class) const {
    return is_valid && variable && variable->StorageClass() == storage_class;
}

// PhysicalStorageBuffer don't have a variable as it just a pointer
bool AccessPath::IsValidBda() const { return is_valid && is_bda; }

bool AccessPath::IsValidDescriptor() const { return is_valid && variable && variable->IsDescriptor(); }

static const Type* FindAccessType(const TypeManager& type_manager, const Function& function, const Instruction& inst) {
    const spv::Op opcode = static_cast<spv::Op>(inst.Opcode());
    const Type* access_type = type_manager.FindTypeById(inst.TypeId());

    if (!access_type) {
        if (opcode == spv::OpStore || opcode == spv::OpCooperativeMatrixStoreKHR) {
            access_type = type_manager.FindTypeGlobal(function, inst.Operand(1));
        } else if (opcode == spv::OpImageWrite) {
            access_type = type_manager.FindTypeGlobal(function, inst.Operand(2));
        } else if (opcode == spv::OpAtomicStore) {
            access_type = type_manager.FindTypeGlobal(function, inst.Operand(3));
        }
    }
    assert(access_type);
    return access_type;
}

static uint32_t GetPhysicalStorageBufferAlignment(const Instruction& inst) {
    const spv::Op opcode = (spv::Op)inst.Opcode();
    if (opcode == spv::OpLoad || opcode == spv::OpStore) {
        // We only care if there is an Aligned Memory Operands
        // VUID-StandaloneSpirv-PhysicalStorageBuffer64-04708 requires there to be an Aligned operand
        const uint32_t memory_operand_index = opcode == spv::OpLoad ? 4 : 3;
        const uint32_t alignment_word_index = opcode == spv::OpLoad ? 5 : 4;  // OpStore is at [4]
        if (inst.Length() < alignment_word_index) {
            return 0;
        }
        const uint32_t memory_operands = inst.Word(memory_operand_index);
        if ((memory_operands & spv::MemoryAccessAlignedMask) == 0) {
            return 0;
        }
        // Even if they are other Memory Operands the spec says it is ordered by smallest bit first,
        // Luckily |Aligned| is the smallest bit that can have an operand so we know it is here
        uint32_t alignment = inst.Word(alignment_word_index);

        // Aligned 0 was not being validated (https://github.com/KhronosGroup/glslang/issues/3893)
        // This is nonsense and we should skip (as it should be validated in spirv-val)
        if (!IsPowerOfTwo(alignment)) {
            return 0;
        }
        return alignment;
    } else if (AtomicOperation(opcode)) {
        // Atomics are naturally aligned and by setting this to 1, it will always pass the alignment check
        return 1;
    }

    // Things like CoopMat are handled with 08986
    return 0;
}

// While unlikely to hit, still possible and need to avoid things like copies
static const Instruction* Unwarp(const Instruction* in_inst, const Function& function) {
    const Instruction* out_inst = in_inst;
    while (out_inst && (out_inst->Opcode() == spv::OpCopyObject || out_inst->Opcode() == spv::OpBitcast)) {
        out_inst = function.FindInstruction(out_inst->Operand(0));
    }
    return out_inst;
}

// This is the "hide all the ugliness here" function
// The goal is a new pass can just call this and have everything it needs to know about the memory access
AccessPath::AccessPath(const Module& module, TypeManager& type_manager, const Function& function, const Instruction& inst) {
    assert(inst.IsMemoryAccess());
    const spv::Op opcode = (spv::Op)inst.Opcode();

    // Find Access Type
    {
        access_type = FindAccessType(type_manager, function, inst);

        const bool image_sampler_access = access_type->spv_type_ == SpvType::kImage ||
                                          access_type->spv_type_ == SpvType::kSampledImage ||
                                          access_type->spv_type_ == SpvType::kSampler;

        // This is just loading the image handle, this alone is the not the access.
        // There will be an access (OpImageWrite, OpImageSampleImplicitLod, etc) later which has the real access information
        if (opcode == spv::OpLoad && image_sampler_access) {
            return;
        }
    }

    // unwrap the optional image access
    // Basically there are 2 flows, images and non-images
    const Instruction* sampler_load_inst = nullptr;
    uint32_t ptr_id = OpcodeImageAccessPosition(opcode);
    const bool is_image_access = ptr_id != 0;

    if (!is_image_access) {
        ptr_id = inst.Operand(0);  // Standard Store/Load
    } else {
        // Image access path
        descriptor.image_load_inst = function.FindInstruction(inst.Word(ptr_id));
        const Instruction* load_inst = descriptor.image_load_inst;
        uint32_t load_operand = 0;

        while (load_inst &&
               IsValueIn(static_cast<spv::Op>(load_inst->Opcode()), {spv::OpSampledImage, spv::OpImage, spv::OpCopyObject})) {
            if (load_inst->Opcode() == spv::OpSampledImage) {
                sampler_load_inst = function.FindInstruction(load_inst->Operand(1));
            }
            load_operand = load_inst->Operand(0);
            load_inst = function.FindInstruction(load_operand);
        }

        // Note - we never "store" an image, we only load its handle and store the "texel" data
        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            // The Undef logic should be able to remove this check, its invalid SPIR-V
            // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7753
            //
            // We currently don't go chasing every function caller
            // Only CTS seems to try and pass OpTypeSampledImage as a parameter, so likely ok
            // (likely only CTS because combined image samplers are a GLSL only thing which doesn't allow passing it by parameter,
            // it is more likley for seperate samplers which Slang can do)
            assert(type_manager.IsUndef(load_operand) || (load_inst && load_inst->Opcode() == spv::OpFunctionParameter));
            return;
        }

        descriptor.is_combined_image_sampler = (sampler_load_inst == nullptr && ImageSampleOperation(opcode));

        // From here the load should look like a non-image access
        ptr_id = load_inst->Operand(0);
    }

    // Buffer/Image Descriptor will always have an access chain, but some other can have direct access.
    // TaskPayload can be a scalar that does a direct variable access
    // An non-array AccelerationStructure (which uses UniformConstant storage class)
    variable = type_manager.FindVariableById(ptr_id);

    // Will be null if already found |variable|
    const Instruction* root_ptr_inst = function.FindInstruction(ptr_id);
    const Instruction* next_inst = root_ptr_inst;

    // If not a direct access, walk the access chains
    // We might find the |variable| while walking it
    if (!variable) {
        root_ptr_inst = function.FindInstruction(ptr_id);
        next_inst = root_ptr_inst;

        while (next_inst) {
            if (next_inst->IsAccessChain()) {
                // inserting in front allows us to walk over the loop from the front
                ac_list.insert(ac_list.begin(), next_inst);
                const uint32_t base_operand = next_inst->IsUntypedAccessChain() ? 1 : 0;
                const uint32_t access_chain_base_id = next_inst->Operand(base_operand);

                variable = type_manager.FindVariableById(access_chain_base_id);
                if (variable) {
                    break;
                }
                next_inst = function.FindInstruction(access_chain_base_id);
            } else if (next_inst->Opcode() == spv::OpCopyObject || next_inst->Opcode() == spv::OpBitcast) {
                const uint32_t base_id = next_inst->Operand(0);
                variable = type_manager.FindVariableById(base_id);
                if (variable) {
                    break;
                }
                next_inst = function.FindInstruction(base_id);
            } else {
                break;
            }
        }
    }

    // If still |variable| is not found, keep searching
    if (!variable) {
        assert(next_inst);
        if (next_inst->Opcode() == spv::OpFunctionParameter) {
            // TODO - Need to handle walking all function callers
            return;
        }

        // check for PhysicalStorageBuffer (BDA)
        const Type* type_pointer = type_manager.FindTypeById(root_ptr_inst->TypeId());
        if (type_pointer && type_pointer->IsBDA()) {
            // While the Pointer Id might not be an OpAccessChain (can be OpLoad, OpCopyObject, etc), we can just examine its result
            // type to see if it is a PhysicalStorageBuffer pointer or not
            is_bda = true;

            uint32_t accessed_type_id = type_pointer->inst_.Operand(1);
            // The original |access_type| is likely a forward pointer
            access_type = type_manager.FindTypeById(accessed_type_id);
            assert(access_type);

            bda_alignment = GetPhysicalStorageBufferAlignment(inst);
            if (bda_alignment != 0) {
                // These accesses are different as they don't have a |variable|
                is_valid = true;
            }

            return;
        }

        // Restored non-variable descriptor pointer instructions
        // (These are really just special cases)
        if (next_inst->Opcode() == spv::OpBufferPointerEXT) {
            spv::StorageClass buffer_ptr_sc = type_manager.FindTypeById(next_inst->TypeId())->inst_.StorageClass();
            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4858
            // It should be a bug to use 1.0 BufferBlock
            descriptor.type = buffer_ptr_sc == spv::StorageClassStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                                                                              : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            const uint32_t buffer_pointer_id = next_inst->Operand(0);
            // For now assume this is a 1D array into the descriptor array
            // https://gitlab.khronos.org/spirv/SPIR-V/-/issues/942
            next_inst = function.FindInstruction(buffer_pointer_id);
            assert(next_inst->Opcode() == spv::OpUntypedAccessChainKHR);
            ac_list.insert(ac_list.begin(), next_inst);
            const uint32_t untyped_variable_id = next_inst->Operand(1);
            variable = type_manager.FindVariableById(untyped_variable_id);
        } else if (next_inst->Opcode() == spv::OpImageTexelPointer || next_inst->Opcode() == spv::OpUntypedImageTexelPointerEXT) {
            // Atomic Storage Image
            descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

            // Found CTS that uses this for untyped texel buffer, but doesn't seem to be done for typed texel buffers
            // ... Not sure if this is just missing from OpImageTexelPointer
            if (next_inst->Opcode() == spv::OpUntypedImageTexelPointerEXT) {
                descriptor.type = type_manager.FindTypeById(next_inst->Word(3))->inst_.GetImageType();
            }

            const uint32_t image_operand = next_inst->Opcode() == spv::OpUntypedImageTexelPointerEXT ? 1 : 0;
            const Instruction* access_chain_inst = function.FindInstruction(next_inst->Operand(image_operand));
            if (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
                next_inst = access_chain_inst;
                ac_list.insert(ac_list.begin(), next_inst);
                const uint32_t base_operand = next_inst->IsUntypedAccessChain() ? 1 : 0;
                variable = type_manager.FindVariableById(access_chain_inst->Operand(base_operand));
            } else {
                // if no array, will point right to a variable
                variable = type_manager.FindVariableById(next_inst->Operand(0));
            }
        }
    }

    if (!variable) {
        // We currently don't care to validate Function Variables are they are never going to be
        // something that interfaces with the API (like a descriptor/pushConstant do).
        assert((next_inst->Opcode() == spv::OpVariable && next_inst->StorageClass() == spv::StorageClassFunction));
        return;  // not a valid access path
    }

    // Find the Pointer Type
    //
    // Welcome to SPV_KHR_untyped_pointers soldier!
    // Untyped we get the pointer type from the last access chain
    // But typed, the OpVariable had it
    {
        if (next_inst && next_inst->IsUntypedAccessChain()) {
            const uint32_t pointer_type_id = next_inst->Operand(0);
            pointer_type = type_manager.FindTypeById(pointer_type_id);
        } else {
            pointer_type = variable->PointerType(type_manager);
        }
        assert(pointer_type);

        // The way CoopMat works, we need to get the size here as the type is not found in the access chains
        if (inst.Opcode() == spv::OpCooperativeMatrixLoadKHR || inst.Opcode() == spv::OpCooperativeMatrixStoreKHR) {
            coop_mat = type_manager.BuildCooperativeMatrixAccess(function, inst);
        }
    }

    // Everything after is related to descriptors, because we made descriptors complex in Vulkan
    if (!variable->IsDescriptor()) {
        is_valid = true;
        return;
    }

    // Descriptor Indexing
    if (pointer_type->IsArray()) {
        const Instruction* descriptor_ac = ac_list.front();
        assert(descriptor_ac);  // no way to have an array otherwise
        if (descriptor_ac->IsUntypedAccessChain()) {
            if (descriptor_ac->Length() > 5) {
                descriptor.index_id = descriptor_ac->Word(5);
            } else {
                // using implicit zero index
                descriptor.index_id = type_manager.GetConstantZeroUint32().Id();
            }
        } else {
            descriptor.index_id = descriptor_ac->Word(4);
        }
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        descriptor.index_id = type_manager.GetConstantZeroUint32().Id();

        // Hack for Offset in Heaps until get better understanding
        if (variable->interface_.IsHeap() && pointer_type->spv_type_ == SpvType::kStruct) {
            assert(next_inst->IsUntypedAccessChain());
            // https://godbolt.org/z/hWz84zdTW - this is required to be a constant

            // If doesn't have an index, it's implicitly the zero index
            if (next_inst->Length() > 5) {
                const Constant* struct_member_index_constant = type_manager.FindConstantById(next_inst->Word(5));
                assert(struct_member_index_constant);
                descriptor.heap_offset_member_index = struct_member_index_constant->GetValueUint32();
            }
            if (next_inst->Length() > 6) {
                descriptor.index_id = next_inst->Word(6);
            }
        }
    }

    // When using a SAMPLED_IMAGE and SAMPLER, they are accessed together so we need to check for 2 descriptors
    // We currently don't go chasing every function caller
    if (sampler_load_inst) {
        sampler_load_inst = Unwarp(sampler_load_inst, function);
    }
    if (sampler_load_inst && sampler_load_inst->Opcode() != spv::OpFunctionParameter) {
        assert(sampler_load_inst && sampler_load_inst->Opcode() == spv::OpLoad);

        uint32_t sampler_ptr_id = sampler_load_inst->Operand(0);
        if (const Instruction* sampler_inst = Unwarp(function.FindInstruction(sampler_ptr_id), function)) {
            sampler_ptr_id = sampler_inst->ResultId();
        }
        descriptor.sampler_variable = type_manager.FindVariableById(sampler_ptr_id);

        if (descriptor.sampler_variable) {
            descriptor.sampler_index_id = type_manager.GetConstantZeroUint32().Id();
        } else {
            // descriptor array
            // this is a lazy way with a hard assumption the sampler can only be a 1D array with a single access chain
            next_inst = function.FindInstruction(sampler_ptr_id);
            assert(next_inst->IsNonPtrAccessChain());

            const uint32_t base_operand = next_inst->IsUntypedAccessChain() ? 1 : 0;
            const uint32_t access_chain_base_id = next_inst->Operand(base_operand);
            descriptor.sampler_variable = type_manager.FindVariableById(access_chain_base_id);

            const uint32_t index_0_operand = base_operand + 1;
            descriptor.sampler_index_id = next_inst->Operand(index_0_operand);

            if (next_inst->IsUntypedAccessChain()) {
                const uint32_t pointer_type_id = next_inst->Operand(0);
                descriptor.sampler_pointer_type = type_manager.FindTypeById(pointer_type_id);
            } else {
                descriptor.sampler_pointer_type = descriptor.sampler_variable->PointerType(type_manager);
            }
            assert(descriptor.sampler_pointer_type);
        }

        // Hack for Offset in Heaps until get better understanding
        if (descriptor.sampler_pointer_type && !descriptor.sampler_pointer_type->IsArray()) {
            if (descriptor.sampler_variable->interface_.IsHeap() &&
                descriptor.sampler_pointer_type->spv_type_ == SpvType::kStruct) {
                if (next_inst && next_inst->IsUntypedAccessChain() && next_inst->Length() > 5) {
                    const Constant* struct_member_index_constant = type_manager.FindConstantById(next_inst->Word(5));
                    assert(struct_member_index_constant);
                    descriptor.sampler_heap_offset_member_index = struct_member_index_constant->GetValueUint32();
                }
            }
        }
    }

    // For things like Untyped pointers Buffers and Atomic Images we will find this already
    if (descriptor.type == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        if (is_image_access) {
            const Type* image_type = type_manager.FindTypeById(descriptor.image_load_inst->TypeId());
            assert(image_type && (image_type->spv_type_ == SpvType::kImage || image_type->spv_type_ == SpvType::kSampledImage));

            if (image_type->spv_type_ == SpvType::kSampledImage) {
                descriptor.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            } else {
                descriptor.type = image_type->inst_.GetImageType();
            }
        } else if (access_type->spv_type_ == SpvType::kAccelerationStructureKHR) {
            descriptor.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        } else {
            // Normally buffers will need an access chain, but if a direct access, safe to use the variable
            spv::StorageClass access_sc = spv::StorageClassMax;
            if (ac_list.empty()) {
                access_sc = variable->type_.inst_.StorageClass();
            } else {
                access_sc = type_manager.FindTypeById(ac_list.front()->TypeId())->inst_.StorageClass();
            }

            if (access_sc == spv::StorageClassStorageBuffer) {
                descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            } else if (access_sc == spv::StorageClassUniform) {
                descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                // handles the dumb issue where 1.0 shaders "Uniform" could be really a Storage Buffer
                // https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/extensions/shader_features.adoc#vk_khr_storage_buffer_storage_class
                const uint32_t spirv_version_1_3 = 0x00010300;  // Vulkan 1.1
                if (module.header_.version < spirv_version_1_3) {
                    // Here we have a Vulkan 1.0 shader and need to just make sure there is no BufferBlock on the struct
                    const uint32_t block_type_id = pointer_type->IsArray() ? pointer_type->inst_.Operand(0) : pointer_type->Id();
                    for (const auto& annotation : module.annotations_) {
                        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == block_type_id &&
                            spv::Decoration(annotation->Word(2)) == spv::DecorationBufferBlock) {
                            descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (descriptor.type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        is_valid = true;
    } else {
        // This occurs for direct access to the heap
        // TODO - We don't mark these valid, but in theory they could be OOB or touching reserved range still
        assert(variable->type_.inst_.StorageClass() == spv::StorageClassUniformConstant && variable->interface_.IsHeap());
    }
}

}  // namespace spirv
}  // namespace gpuav
