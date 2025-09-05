/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
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
#include "state_tracker/tensor_state.h"
#include "generated/dispatch_functions.h"
#include "state_object.h"
#include "state_tracker/state_tracker.h"

namespace vvl {

Tensor::Tensor(DeviceState &dev_data, VkTensorARM handle, const VkTensorCreateInfoARM *pCreateInfo)
    : Bindable(handle, kVulkanObjectTypeTensorARM, false, (pCreateInfo->flags & VK_TENSOR_CREATE_PROTECTED_BIT_ARM) == 0, 0),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      safe_description(pCreateInfo->pDescription),
      description(*safe_description.ptr()) {
    tensor_mem_info_ = {VK_STRUCTURE_TYPE_TENSOR_MEMORY_REQUIREMENTS_INFO_ARM, nullptr, handle};
    DispatchGetTensorMemoryRequirementsARM(dev_data.device, &tensor_mem_info_, &mem_reqs_);
    tracker_.emplace<BindableLinearMemoryTracker>(&mem_reqs_.memoryRequirements);
    SetMemoryTracker(&std::get<BindableLinearMemoryTracker>(tracker_));
}

TensorView::TensorView(const std::shared_ptr<Tensor> &tensor, VkTensorViewARM handle, const VkTensorViewCreateInfoARM *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeTensorViewARM),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      tensor_state(tensor) {}

void TensorView::Destroy() {
    for (auto &item : sub_states_) {
        item.second->Destroy();
    }
    if (tensor_state) {
        tensor_state->RemoveParent(this);
        tensor_state = nullptr;
    }
    StateObject::Destroy();
}

void TensorView::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
}

}  // namespace vvl
