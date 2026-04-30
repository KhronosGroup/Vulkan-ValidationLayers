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
class AccelerationStructureNVSubState;

class AccelerationStructureNV : public Bindable, public SubStateManager<AccelerationStructureNVSubState> {
  public:
    AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV handle,
                            const VkAccelerationStructureCreateInfoNV *pCreateInfo)
        : Bindable(handle, kVulkanObjectTypeAccelerationStructureNV, false, false, 0),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()),
          memory_requirements(GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV)),
          build_scratch_memory_requirements(
              GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV)),
          update_scratch_memory_requirements(
              GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV)),
          tracker_(&memory_requirements) {
        Bindable::SetMemoryTracker(&tracker_);
    }
    AccelerationStructureNV(const AccelerationStructureNV &rh_obj) = delete;

    VkAccelerationStructureNV VkHandle() const { return handle_.Cast<VkAccelerationStructureNV>(); }

    void Destroy() override;
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    void Build(const VkAccelerationStructureInfoNV *pInfo) {
        built = true;
        build_info.initialize(pInfo);
    };

    const vku::safe_VkAccelerationStructureCreateInfoNV safe_create_info;
    const VkAccelerationStructureCreateInfoNV &create_info;

    vku::safe_VkAccelerationStructureInfoNV build_info;
    const VkMemoryRequirements memory_requirements;
    const VkMemoryRequirements build_scratch_memory_requirements;
    const VkMemoryRequirements update_scratch_memory_requirements;
    uint64_t opaque_handle = 0;
    bool memory_requirements_checked = false;
    bool build_scratch_memory_requirements_checked = false;
    bool update_scratch_memory_requirements_checked = false;
    bool built = false;

  private:
    static VkMemoryRequirements GetMemReqs(VkDevice device, VkAccelerationStructureNV as,
                                           VkAccelerationStructureMemoryRequirementsTypeNV mem_type) {
        VkAccelerationStructureMemoryRequirementsInfoNV req_info = vku::InitStructHelper();
        req_info.type = mem_type;
        req_info.accelerationStructure = as;
        VkMemoryRequirements2 requirements = vku::InitStructHelper();
        DispatchGetAccelerationStructureMemoryRequirementsNV(device, &req_info, &requirements);
        return requirements.memoryRequirements;
    }
    BindableLinearMemoryTracker tracker_;
};

class AccelerationStructureNVSubState {
  public:
    explicit AccelerationStructureNVSubState(AccelerationStructureNV &ac) : base(ac) {}
    AccelerationStructureNVSubState(const AccelerationStructureNVSubState &) = delete;
    AccelerationStructureNVSubState &operator=(const AccelerationStructureNVSubState &) = delete;
    virtual ~AccelerationStructureNVSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {}

    AccelerationStructureNV &base;
};

inline void AccelerationStructureNV::Destroy() {
    for (auto &item : sub_states_) {
        item.second->Destroy();
    }
    Bindable::Destroy();
}

inline void AccelerationStructureNV::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    Bindable::NotifyInvalidate(invalid_nodes, unlink);
}

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

    void Build(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo, const bool is_host,
               const VkAccelerationStructureBuildRangeInfoKHR *build_range_info);

    void UpdateBuildRangeInfos(const VkAccelerationStructureBuildRangeInfoKHR *p_build_range_infos, uint32_t geometry_count);

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
    VkDeviceAddress GetAccelerationStructureAddress() const { return acceleration_structure_address.load(); }
    void SetAccelerationStructureAddress(VkDeviceAddress addr) { acceleration_structure_address = addr; }
    const std::optional<vku::safe_VkAccelerationStructureBuildGeometryInfoKHR> &GetBuildInfo() const { return build_geometry_info; }
    void SetBuildInfo(const std::optional<vku::safe_VkAccelerationStructureBuildGeometryInfoKHR> &info) {
        build_geometry_info = info;
    }
    const std::vector<VkAccelerationStructureBuildRangeInfoKHR> &GetBuildRangeInfos() const { return build_range_infos; }

  private:
    struct CreateInfo1 {
        CreateInfo1(const VkAccelerationStructureCreateInfoKHR *pCreateInfo, std::shared_ptr<vvl::Buffer> buffer)
            : ci(pCreateInfo), buffer_state(std::move(buffer)) {}
        vku::safe_VkAccelerationStructureCreateInfoKHR ci;
        std::shared_ptr<vvl::Buffer> buffer_state;
    };

    std::variant<CreateInfo1, vku::safe_VkAccelerationStructureCreateInfo2KHR> create_info;
    VkDeviceAddressRangeKHR device_address_range{};
    uint64_t opaque_handle = 0;
    std::atomic<VkDeviceAddress> acceleration_structure_address = 0;
    std::optional<vku::safe_VkAccelerationStructureBuildGeometryInfoKHR> build_geometry_info;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos{};
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
