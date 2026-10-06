/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "containers/span.h"
#include "state_tracker/device_memory_state.h"
#include "state_tracker/buffer_state.h"
#include "generated/dispatch_functions.h"

namespace vvl {
class AccelerationStructureKHRSubState;

struct BufferAndOffset {
    vvl::Buffer *const state{};
    VkDeviceSize offset{};
    explicit operator bool() const { return state != nullptr; }
};

class AccelerationStructureKHR : public StateObject, public SubStateManager<AccelerationStructureKHRSubState> {
  public:
    AccelerationStructureKHR(vvl::DeviceState &device_state, const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                             VkAccelerationStructureKHR handle);

    AccelerationStructureKHR(vvl::DeviceState &device_state, const VkAccelerationStructureCreateInfo2KHR *pCreateInfo,
                             VkAccelerationStructureKHR handle);
    AccelerationStructureKHR(const AccelerationStructureKHR &rh_obj) = delete;

    virtual ~AccelerationStructureKHR();

    VkAccelerationStructureKHR VkHandle() const { return handle_.Cast<VkAccelerationStructureKHR>(); }

    void LinkChildNodes() override;

    void Destroy() override;
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    bool UsesCreateInfo1() const;
    bool UsesCreateInfo2() const;
    // returns:
    // - pointer to buffer backing AS (can be null)
    // - offset in that buffer AS is stored at
    // For AS created with create info version 1, always returns the buffer the AS was created with
    // For AS created with create info version 2, returns a valid buffer with a device address range
    // containing the VkDeviceAddressRangeKHR supplied at creation time.
    // Given that in some scenarios address ranges lifespan does not match that of buffers it pertains to,
    // the returned buffer should NOT be stored and assumed to always be backing the AS.
    BufferAndOffset GetFirstValidBuffer(const vvl::DeviceState &device_state) const;
    VkAccelerationStructureCreateFlagsKHR GetCreateFlags() const;
    VkDeviceSize GetSize() const;
    VkAccelerationStructureTypeKHR GetType() const;
    // Returns the device address range effectively occupied by the acceleration structure,
    // as defined by its creation info.
    // It does NOT take into account the acceleration structure address as returned by
    // vkGetAccelerationStructureDeviceAddress, this address may be at an offset
    // of the buffer range backing the acceleration structure
    VkDeviceAddressRangeKHR GetEffectiveDeviceAddressRange() const;
    vvl::range<VkDeviceAddress> GetVvlEffectiveDeviceAddressRange() const;
    uint64_t GetOpaqueHandle() const { return opaque_handle; }
    bool WasDeserialized() const { return was_deserialized_; }
    void MarkAsDeserialized() { was_deserialized_ = true; }
    VkDeviceAddress GetAccelerationStructureAddress() const { return acceleration_structure_address.load(); }
    void SetAccelerationStructureAddress(VkDeviceAddress addr) { acceleration_structure_address = addr; }

    std::string Describe(const Logger& dev_data) const;

  private:
    struct CreateInfo1 {
        CreateInfo1(const VkAccelerationStructureCreateInfoKHR *pCreateInfo, std::shared_ptr<vvl::Buffer> buffer)
            : ci(pCreateInfo), buffer_state(std::move(buffer)) {}
        vku::safe_VkAccelerationStructureCreateInfoKHR ci;
        std::shared_ptr<vvl::Buffer> buffer_state;
    };

    std::variant<CreateInfo1, vku::safe_VkAccelerationStructureCreateInfo2KHR> create_info;
    VkDeviceAddressRangeKHR device_address_range{};
    bool was_deserialized_ = false;
    uint64_t opaque_handle = 0;
    std::atomic<VkDeviceAddress> acceleration_structure_address = 0;
};

class AccelerationStructureKHRSubState {
  public:
    explicit AccelerationStructureKHRSubState(AccelerationStructureKHR &ac) : base(ac) {}
    AccelerationStructureKHRSubState(const AccelerationStructureKHRSubState &) = delete;
    AccelerationStructureKHRSubState &operator=(const AccelerationStructureKHRSubState &) = delete;
    virtual ~AccelerationStructureKHRSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {}

    AccelerationStructureKHR &base;
};

}  // namespace vvl
