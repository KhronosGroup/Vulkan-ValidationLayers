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

#include "generated/inst_buffer_device_address_comp.h"

namespace gpuav {
namespace spirv {

static LinkInfo link_info = {inst_buffer_device_address_comp, inst_buffer_device_address_comp_size,
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

uint32_t BufferDeviceAddressPass::CreateFunctionCall(BasicBlock& block) {
    // Add any debug information to pass into the function call
    const uint32_t stage_info_id = GetStageInfo(block.function_);
    const uint32_t inst_position = target_instruction_->position_index_;
    auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);

    // Convert reference pointer to uint64
    const uint32_t pointer_id = target_instruction_->Operand(0);
    const Type& uint64_type = module_.type_manager_.GetTypeInt(64, 0);
    const uint32_t convert_id = module_.TakeNextId();
    block.CreateInstruction(spv::OpConvertPtrToU, {uint64_type.Id(), convert_id, pointer_id});

    const Constant& length_constant = module_.type_manager_.GetConstantUInt32(type_length_);
    const Constant& access_opcode = module_.type_manager_.GetConstantUInt32(access_opcode_);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(spv::OpFunctionCall, {bool_type, function_result, function_def, inst_position_constant.Id(),
                                                  stage_info_id, convert_id, length_constant.Id(), access_opcode.Id()});

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

    if (op_type_pointer->inst_.Operand(0) != spv::StorageClassPhysicalStorageBuffer) {
        return false;
    }

    access_opcode_ = opcode;

    // The OpTypePointer's type
    uint32_t accessed_type_id = op_type_pointer->inst_.Operand(1);
    const Type* accessed_type = module_.type_manager_.FindTypeById(accessed_type_id);

    // Save information to be used to make the Function
    target_instruction_ = &inst;
    type_length_ = module_.type_manager_.TypeLength(*accessed_type);
    return true;
}

}  // namespace spirv
}  // namespace gpuav