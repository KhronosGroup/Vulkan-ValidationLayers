/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#include "device_memory_state.h"

class ACCELERATION_STRUCTURE_STATE : public BINDABLE {
  public:
    safe_VkAccelerationStructureCreateInfoNV create_infoNV = {};
    safe_VkAccelerationStructureInfoNV build_info;
    bool memory_requirements_checked = false;
    VkMemoryRequirements2 memory_requirements;
    bool build_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 build_scratch_memory_requirements;
    bool update_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 update_scratch_memory_requirements;
    bool built = false;
    uint64_t opaque_handle = 0;
    const VkAllocationCallbacks *allocator = NULL;
    ACCELERATION_STRUCTURE_STATE(VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci)
        : BINDABLE(as, kVulkanObjectTypeAccelerationStructureNV, false, false, 0),
          create_infoNV(ci),
          memory_requirements{},
          build_scratch_memory_requirements_checked{},
          build_scratch_memory_requirements{},
          update_scratch_memory_requirements_checked{},
          update_scratch_memory_requirements{} {}
    ACCELERATION_STRUCTURE_STATE(const ACCELERATION_STRUCTURE_STATE &rh_obj) = delete;

    VkAccelerationStructureNV acceleration_structure() const { return handle_.Cast<VkAccelerationStructureNV>(); }

    void Build(const VkAccelerationStructureInfoNV *pInfo) {
        built = true;
        build_info.initialize(pInfo);
    };
};

class ACCELERATION_STRUCTURE_STATE_KHR : public BINDABLE {
  public:
    safe_VkAccelerationStructureCreateInfoKHR create_infoKHR = {};
    safe_VkAccelerationStructureBuildGeometryInfoKHR build_info_khr;
    bool memory_requirements_checked = false;
    VkMemoryRequirements2 memory_requirements;
    bool build_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 build_scratch_memory_requirements;
    bool update_scratch_memory_requirements_checked = false;
    VkMemoryRequirements2 update_scratch_memory_requirements;
    bool built = false;
    uint64_t opaque_handle = 0;
    const VkAllocationCallbacks *allocator = NULL;
    ACCELERATION_STRUCTURE_STATE_KHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci)
        : BINDABLE(as, kVulkanObjectTypeAccelerationStructureKHR, false, false, 0),
          create_infoKHR(ci),
          memory_requirements{},
          build_scratch_memory_requirements_checked{},
          build_scratch_memory_requirements{},
          update_scratch_memory_requirements_checked{},
          update_scratch_memory_requirements{} {}
    ACCELERATION_STRUCTURE_STATE_KHR(const ACCELERATION_STRUCTURE_STATE_KHR &rh_obj) = delete;

    VkAccelerationStructureKHR acceleration_structure() const { return handle_.Cast<VkAccelerationStructureKHR>(); }

    void Build(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo) {
        built = true;
        build_info_khr.initialize(pInfo);
    };
};

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
