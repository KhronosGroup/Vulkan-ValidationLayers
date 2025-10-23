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

static VkExternalMemoryHandleTypeFlags GetExternalHandleTypes(const VkTensorCreateInfoARM *create_info) {
    const auto *external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryTensorCreateInfoARM>(create_info->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

namespace vvl {

Tensor::Tensor(DeviceState &dev_data, VkTensorARM handle, const VkTensorCreateInfoARM *pCreateInfo)
    : Bindable(handle, kVulkanObjectTypeTensorARM, false, (pCreateInfo->flags & VK_TENSOR_CREATE_PROTECTED_BIT_ARM) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      safe_description(pCreateInfo->pDescription),
      description(*safe_description.ptr()) {
    tensor_mem_info_ = {VK_STRUCTURE_TYPE_TENSOR_MEMORY_REQUIREMENTS_INFO_ARM, nullptr, handle};
    DispatchGetTensorMemoryRequirementsARM(dev_data.device, &tensor_mem_info_, &mem_reqs_);
    tracker_.emplace<BindableLinearMemoryTracker>(&mem_reqs_.memoryRequirements);
    SetMemoryTracker(&std::get<BindableLinearMemoryTracker>(tracker_));
}

bool Tensor::CompareCreateInfo(const Tensor &other) const {
    bool valid_queue_family = true;
    if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (create_info.queueFamilyIndexCount != other.create_info.queueFamilyIndexCount) {
            valid_queue_family = false;
        } else {
            for (uint32_t i = 0; i < create_info.queueFamilyIndexCount; i++) {
                if (create_info.pQueueFamilyIndices[i] != other.create_info.pQueueFamilyIndices[i]) {
                    valid_queue_family = false;
                    break;
                }
            }
        }
    }

    // There are limitations what actually needs to be compared, so for simplicity (until found otherwise needed), we only need to
    // check the ExternalHandleType and not other pNext chains
    const bool valid_external = GetExternalHandleTypes(&create_info) == GetExternalHandleTypes(&other.create_info);
    bool valid_dimensions = true;
    if (create_info.pDescription->dimensionCount != other.create_info.pDescription->dimensionCount) {
        valid_dimensions = false;
    } else {
        for (uint32_t i = 0; i < create_info.pDescription->dimensionCount; i++) {
            if (create_info.pDescription->pDimensions[i] != other.create_info.pDescription->pDimensions[i]) {
                valid_dimensions = false;
                break;
            }
        }
    }

    bool valid_strides = true;
    for (uint32_t i = 0; i < create_info.pDescription->dimensionCount; i++) {
        if (create_info.pDescription->pStrides[i] != other.create_info.pDescription->pStrides[i]) {
            valid_strides = false;
            break;
        }
    }

    return (create_info.pDescription->tiling == other.create_info.pDescription->tiling) &&
           (create_info.pDescription->format == other.create_info.pDescription->format) &&
           (create_info.flags == other.create_info.flags) && (create_info.sharingMode == other.create_info.sharingMode) &&
           valid_queue_family && valid_external && valid_dimensions && valid_strides;
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
