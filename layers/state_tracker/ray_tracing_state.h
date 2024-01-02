/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include "state_tracker/device_memory_state.h"
#include "state_tracker/buffer_state.h"
#include "generated/layer_chassis_dispatch.h"

namespace vvl {

class AccelerationStructureNV : public Bindable {
  public:
    AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci)
        : Bindable(as, kVulkanObjectTypeAccelerationStructureNV, false, false, 0),
          create_infoNV(ci),
          memory_requirements(GetMemReqs(device, as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV)),
          build_scratch_memory_requirements(
              GetMemReqs(device, as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV)),
          update_scratch_memory_requirements(
              GetMemReqs(device, as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV)),
          tracker_(&memory_requirements) {
        Bindable::SetMemoryTracker(&tracker_);
    }
    AccelerationStructureNV(const AccelerationStructureNV &rh_obj) = delete;

    VkAccelerationStructureNV acceleration_structure() const { return handle_.Cast<VkAccelerationStructureNV>(); }

    void Build(const VkAccelerationStructureInfoNV *pInfo) {
        built = true;
        build_info.initialize(pInfo);
    };

    const safe_VkAccelerationStructureCreateInfoNV create_infoNV = {};
    safe_VkAccelerationStructureInfoNV build_info;
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

class AccelerationStructureKHR : public StateObject {
  public:
    AccelerationStructureKHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci,
                             std::shared_ptr<Buffer> &&buf_state, VkDeviceAddress address)
        : StateObject(as, kVulkanObjectTypeAccelerationStructureKHR), create_infoKHR(ci), buffer_state(buf_state), address(address) {}
    AccelerationStructureKHR(const AccelerationStructureKHR &rh_obj) = delete;

    virtual ~AccelerationStructureKHR() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkAccelerationStructureKHR acceleration_structure() const { return handle_.Cast<VkAccelerationStructureKHR>(); }

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        buffer_state->AddParent(this);
    }

    void Destroy() override {
        if (buffer_state) {
            buffer_state->RemoveParent(this);
            buffer_state = nullptr;
        }
        StateObject::Destroy();
    }

    void Build(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo, const bool is_host,
               const VkAccelerationStructureBuildRangeInfoKHR *build_range_info) {
        built = true;
        build_info_khr.initialize(pInfo, is_host, build_range_info);
    };

    void UpdateBuildRangeInfos(const VkAccelerationStructureBuildRangeInfoKHR *p_build_range_infos, uint32_t geometry_count) {
        build_range_infos.resize(geometry_count);
        for (const auto [i, build_range] : vvl::enumerate(p_build_range_infos, geometry_count)) {
            build_range_infos[i] = *build_range;
        }
    }

    const safe_VkAccelerationStructureCreateInfoKHR create_infoKHR = {};
    safe_VkAccelerationStructureBuildGeometryInfoKHR build_info_khr;
    bool built = false;
    uint64_t opaque_handle = 0;
    std::shared_ptr<vvl::Buffer> buffer_state;
    VkDeviceAddress address;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos;
};

}  // namespace vvl

// Safe struct that spans NV and KHR VkRayTracingPipelineCreateInfo structures.
// It is a safe_VkRayTracingPipelineCreateInfoKHR and supports construction from
// a VkRayTracingPipelineCreateInfoNV.
class safe_VkRayTracingPipelineCreateInfoCommon : public safe_VkRayTracingPipelineCreateInfoKHR {
  public:
    safe_VkRayTracingPipelineCreateInfoCommon() : safe_VkRayTracingPipelineCreateInfoKHR() {}
    safe_VkRayTracingPipelineCreateInfoCommon(const VkRayTracingPipelineCreateInfoNV *pCreateInfo)
        : safe_VkRayTracingPipelineCreateInfoKHR() {
        initialize(pCreateInfo);
    }
    safe_VkRayTracingPipelineCreateInfoCommon(const VkRayTracingPipelineCreateInfoKHR *pCreateInfo)
        : safe_VkRayTracingPipelineCreateInfoKHR(pCreateInfo) {}

    void initialize(const VkRayTracingPipelineCreateInfoNV *pCreateInfo) {
        safe_VkRayTracingPipelineCreateInfoNV nvStruct;
        nvStruct.initialize(pCreateInfo);

        sType = nvStruct.sType;

        // Take ownership of the pointer and null it out in nvStruct
        pNext = nvStruct.pNext;
        nvStruct.pNext = nullptr;

        flags = nvStruct.flags;
        stageCount = nvStruct.stageCount;

        pStages = nvStruct.pStages;
        nvStruct.pStages = nullptr;

        groupCount = nvStruct.groupCount;
        maxRecursionDepth = nvStruct.maxRecursionDepth;
        layout = nvStruct.layout;
        basePipelineHandle = nvStruct.basePipelineHandle;
        basePipelineIndex = nvStruct.basePipelineIndex;

        assert(pGroups == nullptr);
        if (nvStruct.groupCount && nvStruct.pGroups) {
            pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
            for (uint32_t i = 0; i < groupCount; ++i) {
                pGroups[i].sType = nvStruct.pGroups[i].sType;
                pGroups[i].pNext = nvStruct.pGroups[i].pNext;
                pGroups[i].type = nvStruct.pGroups[i].type;
                pGroups[i].generalShader = nvStruct.pGroups[i].generalShader;
                pGroups[i].closestHitShader = nvStruct.pGroups[i].closestHitShader;
                pGroups[i].anyHitShader = nvStruct.pGroups[i].anyHitShader;
                pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
                pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
                pGroups[i].pShaderGroupCaptureReplayHandle = nullptr;
            }
        }
    }
    void initialize(const VkRayTracingPipelineCreateInfoKHR *pCreateInfo) {
        safe_VkRayTracingPipelineCreateInfoKHR::initialize(pCreateInfo);
    }
    uint32_t maxRecursionDepth = 0;  // NV specific
};
