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

#include "buffer_device_address_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_buffer_device_address_comp.h"

namespace gpu {
namespace spirv {

static LinkInfo link_info = {instrumentation_buffer_device_address_comp, instrumentation_buffer_device_address_comp_size,
                             LinkFunctions::inst_buffer_device_address, 0, "inst_buffer_device_address"};

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t BufferDeviceAddressPass::GetLinkFunctionId() {
    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t BufferDeviceAddressPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                     const InjectionData& injection_data) {
    // Convert reference pointer to uint64
    const uint32_t pointer_id = target_instruction_->Operand(0);
    const Type& uint64_type = module_.type_manager_.GetTypeInt(64, 0);
    const uint32_t convert_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpConvertPtrToU, {uint64_type.Id(), convert_id, pointer_id}, inst_it);

    const Constant& length_constant = module_.type_manager_.GetConstantUInt32(type_length_);
    const Constant& access_opcode = module_.type_manager_.GetConstantUInt32(access_opcode_);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, injection_data.inst_position_id,
                             injection_data.stage_info_id, convert_id, length_constant.Id(), access_opcode.Id()},
                            inst_it);

    return function_result;
}

void BufferDeviceAddressPass::Reset() {
    target_instruction_ = nullptr;
    access_opcode_ = 0;
    type_length_ = 0;
}

bool BufferDeviceAddressPass::AnalyzeInstruction(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();
    if (opcode != spv::OpLoad && opcode != spv::OpStore) {
        return false;
    }

    // TODO - Should have loop to walk Load/Store to the Pointer,
    // this case will not cover things such as OpCopyObject or double OpAccessChains
    const Instruction* pointer_inst = function.FindInstruction(inst.Operand(0));
    if (!pointer_inst || pointer_inst->Opcode() != spv::OpAccessChain) {
        return false;
    }

    // Get the OpTypePointer
    const Type* op_type_pointer = module_.type_manager_.FindTypeById(pointer_inst->TypeId());
    if (!op_type_pointer || op_type_pointer->spv_type_ != SpvType::kPointer) {
        return false;
    }

    // The OpTypePointer's type
    uint32_t accessed_type_id = op_type_pointer->inst_.Operand(1);
    const Type* accessed_type = module_.type_manager_.FindTypeById(accessed_type_id);

    // Most common case we will just spot the access directly using the PhysicalStorageBuffer pointer
    if (op_type_pointer->inst_.Operand(0) == spv::StorageClassPhysicalStorageBuffer) {
        // If loading the struct, this is likely just saving it
        // Shown from RADV/Intel NIR compiler, the compiler gets an offset and then dereference just the member, it never "loads the
        // whole struct"
        if (accessed_type->spv_type_ == SpvType::kStruct) {
            // If the struct is only a single element, then everything works and the size will be the same
            if (accessed_type->inst_.Length() > 3) {
                return false;
            }
        }
    } else {
        // TODO https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8089
        return false;
    }

    // Save information to be used to make the Function
    access_opcode_ = opcode;
    target_instruction_ = &inst;
    type_length_ = module_.type_manager_.TypeLength(*accessed_type);
    return true;
}

void BufferDeviceAddressPass::PrintDebugInfo() {
    std::cout << "BufferDeviceAddressPass instrumentation count: " << instrumented_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpu