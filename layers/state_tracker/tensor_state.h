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
#pragma once
#include <variant>
#include "state_object.h"
#include "state_tracker/device_memory_state.h"

namespace vvl {

class DeviceState;

class Tensor : public Bindable {
  public:
    const vku::safe_VkTensorCreateInfoARM safe_create_info;
    const VkTensorCreateInfoARM &create_info;
    const vku::safe_VkTensorDescriptionARM safe_description;
    const VkTensorDescriptionARM &description;

    explicit Tensor(DeviceState &dev_data, VkTensorARM handle, const VkTensorCreateInfoARM *pCreateInfo);

    // This destructor is needed because Bindable depends on the tracker_ variant defined in this
    // class. So we need to do the Destroy() work before tracker_ is destroyed.
    virtual ~Tensor() {
        if (!Destroyed()) {
            Bindable::Destroy();
        }
    }

    uint32_t DimensionCount() const { return description.dimensionCount; }
    const int64_t *Dimensions() const { return description.pDimensions; }
    const VkMemoryRequirements2 *MemReqs() const { return &mem_reqs_; }
    // VkTensorUsageFlagsARM usage() const { return description.usage; }
    VkFormat Format() const { return description.format; }
    VkTensorTilingARM Tiling() const { return description.tiling; }
    VkSharingMode SharingMode() const { return create_info.sharingMode; }

  private:
    std::variant<std::monostate, BindableLinearMemoryTracker> tracker_;
    VkTensorMemoryRequirementsInfoARM tensor_mem_info_;
    VkMemoryRequirements2 mem_reqs_ = vku::InitStructHelper();
};

class TensorView : public StateObject {
  public:
    const vku::safe_VkTensorViewCreateInfoARM safe_create_info;
    const VkTensorViewCreateInfoARM &create_info;

    std::shared_ptr<Tensor> tensor_state;
    TensorView(const std::shared_ptr<Tensor> &tensor, VkTensorViewARM handle, const VkTensorViewCreateInfoARM *pCreateInfo);
    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        tensor_state->AddParent(this);
    }
    virtual ~TensorView() {
        if (!Destroyed()) {
            Destroy();
        }
    }
    TensorView(const TensorView &rh_obj) = delete;
};
}  // namespace vvl
