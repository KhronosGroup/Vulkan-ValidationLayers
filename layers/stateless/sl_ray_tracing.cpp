/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_format_utils.h>
#include "containers/container_utils.h"
#include "containers/span.h"
#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

#include "utils/ray_tracing_utils.h"
#include "utils/vk_api_utils.h"
#include "utils/math_utils.h"

namespace stateless {

bool Device::ValidateCreateAccelerationStructure(const VkAccelerationStructureCreateFlagsKHR create_flags,
                                                 const void* create_info_pNext, const Location& create_info_loc) const {
    bool skip = false;
    const bool is_2 = create_info_loc.function == Func::vkCreateAccelerationStructure2KHR;

    if (create_flags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR &&
        !enabled_features.accelerationStructureCaptureReplay) {
        const char* vuid = is_2 ? "VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-03613"
                                : "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613";
        skip |= LogError(vuid, device, create_info_loc.dot(Field::createFlags),
                         "includes VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR, but "
                         "accelerationStructureCaptureReplay feature is not enabled.");
    }
    if ((create_flags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        const char* vuid = is_2 ? "VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-08108"
                                : "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108";
        skip |= LogError(vuid, device, create_info_loc.dot(Field::createFlags),
                         "includes VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT, but the "
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }
    const auto* opaque_capture_descriptor_buffer =
        vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(create_info_pNext);
    if (opaque_capture_descriptor_buffer &&
        !(create_flags & VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109", device, create_info_loc.dot(Field::createFlags),
                         "includes VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT, but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.\n%s",
                         PrintPNextChain(Struct::VkAccelerationStructureCreateInfoKHR, create_info_pNext).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                                  const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkAccelerationStructureKHR *pAccelerationStructure,
                                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCreateAccelerationStructureKHR-accelerationStructure-03611", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateCreateAccelerationStructure(pCreateInfo->createFlags, pCreateInfo->pNext, create_info_loc);

    if (pCreateInfo->deviceAddress &&
        !(pCreateInfo->createFlags & VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
        skip |= LogError(
            "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612", device, create_info_loc.dot(Field::createFlags),
            "(%s) does not include VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR but the deviceAddress "
            "(%" PRIu64 ") is not zero.",
            string_VkAccelerationStructureCreateFlagsKHR(pCreateInfo->createFlags).c_str(), pCreateInfo->deviceAddress);
    }
    if (pCreateInfo->deviceAddress && !enabled_features.accelerationStructureCaptureReplay) {
        skip |=
            LogError("VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488", device, create_info_loc.dot(Field::deviceAddress),
                     "is %" PRIu64 " but accelerationStructureCaptureReplay feature was not enabled.", pCreateInfo->deviceAddress);
    }
    if (!IsIntegerMultipleOf(pCreateInfo->offset, 256)) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-offset-03734", device, create_info_loc.dot(Field::offset),
                         "(%" PRIu64 ") must be a multiple of 256 bytes", pCreateInfo->offset);
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateAccelerationStructure2KHR(VkDevice device,
                                                                   const VkAccelerationStructureCreateInfo2KHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkAccelerationStructureKHR *pAccelerationStructure,
                                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCreateAccelerationStructure2KHR-accelerationStructure-03611", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateCreateAccelerationStructure(pCreateInfo->createFlags, pCreateInfo->pNext, create_info_loc);

    skip |= context.ValidateDeviceAddressFlags(create_info_loc.dot(Field::addressFlags), pCreateInfo->addressFlags);

    if (pCreateInfo->addressRange.address == 0) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11602", device,
                         create_info_loc.dot(Field::addressRange).dot(Field::address), "is 0.");
    } else if (!IsPointerAligned(pCreateInfo->addressRange.address, 256)) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11605", device,
                         create_info_loc.dot(Field::addressRange).dot(Field::address),
                         "(0x%" PRIx64 ") is not aligned to 256 bytes.", pCreateInfo->addressRange.address);
    }
    if (pCreateInfo->addressRange.size == 0) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11608", device,
                         create_info_loc.dot(Field::addressRange).dot(Field::size), "is 0.");
    }
    if (!enabled_features.deviceAddressCommands) {
        skip |= LogError("VUID-vkCreateAccelerationStructure2KHR-deviceAddressCommands-13086", device, error_obj.location,
                         "deviceAddressCommands feature is not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateDestroyAccelerationStructureKHR(VkDevice device,
                                                                   VkAccelerationStructureKHR accelerationStructure,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-08934", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    return skip;
}

bool Device::ValidateCreateRayTracingPipelinesFlagsKHR(const VkPipelineCreateFlags2 flags, const Location &flags_loc) const {
    bool skip = false;

    if (flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (flags & VK_PIPELINE_CREATE_DISPATCH_BASE) {
        skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-flags-03816", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR &&
        (!enabled_features.rayTracingPipelineShaderGroupHandleCaptureReplay)) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03598", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    if (!enabled_features.rayTraversalPrimitiveCulling) {
        if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03596", device, flags_loc,
                             "is %s.", string_VkPipelineCreateFlags2(flags).c_str());
        }
        if (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-rayTraversalPrimitiveCulling-03597", device, flags_loc,
                             "is %s.", string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    if ((flags & VK_PIPELINE_CREATE_2_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT) &&
        (flags & VK_PIPELINE_CREATE_2_DISALLOW_OPACITY_MICROMAP_BIT_ARM)) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-10392", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    if (flags & (VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT)) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-12341", device, flags_loc, "is %s.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.rayTracingPipeline) {
        skip |= LogError("VUID-vkCreateRayTracingPipelinesKHR-rayTracingPipeline-03586", device, error_obj.location,
                         "the rayTracingPipeline feature was not enabled.");
    }
    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkRayTracingPipelineCreateInfoKHR &create_info = pCreateInfos[i];

        const auto *create_flags_2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
        const VkPipelineCreateFlags2 flags =
            create_flags_2 ? create_flags_2->flags : static_cast<VkPipelineCreateFlags2>(create_info.flags);
        const Location flags_loc = create_flags_2 ? create_info_loc.pNext(Struct::VkPipelineCreateFlags2CreateInfo, Field::flags)
                                                  : create_info_loc.dot(Field::flags);
        if (!create_flags_2) {
            skip |= context.ValidateFlags(flags_loc, vvl::FlagBitmask::VkPipelineCreateFlagBits, AllVkPipelineCreateFlagBits,
                                          create_info.flags, kOptionalFlags, "VUID-VkRayTracingPipelineCreateInfoKHR-None-09497");
        } else {
            skip |= ValidateCreatePipelinesFlags2(create_info.flags, flags, flags_loc);
        }
        skip |= ValidateCreatePipelinesFlagsCommon(flags, flags_loc);

        for (uint32_t stage_index = 0; stage_index < create_info.stageCount; ++stage_index) {
            const Location stage_loc = create_info_loc.dot(Field::pStages, stage_index);
            const VkPipelineShaderStageCreateInfo& stage_ci = create_info.pStages[stage_index];
            skip |= ValidatePipelineShaderStageCreateInfoCommon(context, stage_ci, stage_loc);

            if ((stage_ci.stage & kShaderStageAllRayTracing) == 0) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899", device, stage_loc.dot(Field::stage),
                                 "is %s.", string_VkShaderStageFlagBits(stage_ci.stage));
            }
        }

        if (auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext)) {
            if (feedback_struct->pipelineStageCreationFeedbackCount != 0 &&
                feedback_struct->pipelineStageCreationFeedbackCount != create_info.stageCount) {
                skip |= LogError(
                    "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineStageCreationFeedbackCount-06652", device,
                    create_info_loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pipelineStageCreationFeedbackCount),
                    "(%" PRIu32 ") is not equal to %s (%" PRIu32 ").", feedback_struct->pipelineStageCreationFeedbackCount,
                    create_info_loc.Fields().c_str(), create_info.stageCount);
            }
        }

        for (uint32_t group_index = 0; group_index < create_info.groupCount; ++group_index) {
            const Location group_loc = create_info_loc.dot(Field::pGroups, group_index);
            const VkRayTracingShaderGroupCreateInfoKHR& group_ci = create_info.pGroups[group_index];
            if ((group_ci.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) ||
                (group_ci.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR)) {
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) &&
                    (group_ci.anyHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError(
                        "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470", device, flags_loc,
                        "includes VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR but %s is VK_SHADER_UNUSED_KHR.",
                        group_loc.dot(Field::anyHitShader).Fields().c_str());
                }
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) &&
                    (group_ci.closestHitShader == VK_SHADER_UNUSED_KHR)) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471", device, flags_loc,
                                     "includes VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR but %s is "
                                     "VK_SHADER_UNUSED_KHR.",
                                     group_loc.dot(Field::closestHitShader).Fields().c_str());
                }
            }
            if (enabled_features.rayTracingPipelineShaderGroupHandleCaptureReplay && group_ci.pShaderGroupCaptureReplayHandle) {
                if (!(flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |=
                        LogError("VUID-VkRayTracingPipelineCreateInfoKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03599",
                                 device, flags_loc, "is %s.", string_VkPipelineCreateFlags2(flags).c_str());
                }
            }
        }

        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (create_info.basePipelineIndex != -1) {
                if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-07986", device,
                                     create_info_loc.dot(Field::basePipelineIndex),
                                     "is %" PRId32 " and basePipelineHandle is not VK_NULL_HANDLE.", create_info.basePipelineIndex);
                }
                if (create_info.basePipelineIndex > static_cast<int32_t>(i)) {
                    skip |=
                        LogError("VUID-vkCreateRayTracingPipelinesKHR-flags-03415", device,
                                 create_info_loc.dot(Field::basePipelineIndex), "is %" PRId32 ".", create_info.basePipelineIndex);
                }
            }
            if (create_info.basePipelineHandle == VK_NULL_HANDLE) {
                if (create_info.basePipelineIndex < 0 || static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                    skip |= LogError(
                        "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07985", device, flags_loc,
                        "includes VK_PIPELINE_CREATE_DERIVATIVE_BIT but basePipelineIndex has invalid index value %" PRId32 ".",
                        create_info.basePipelineIndex);
                }
            }
        }

        if (flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            if (create_info.pLibraryInterface == nullptr) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465", device, flags_loc,
                                 "includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR but pLibraryInterface is null.");
            }
        }

        if (!IsExtEnabled(extensions.vk_khr_pipeline_library)) {
            if (create_info.pLibraryInfo) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595", device,
                                 create_info_loc.dot(Field::pLibraryInfo), "is not NULL.");
            }
            if (create_info.pLibraryInterface) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03595", device,
                                 create_info_loc.dot(Field::pLibraryInterface), "is not NULL.");
            }
        }

        if (create_info.pLibraryInfo) {
            if ((create_info.pLibraryInfo->libraryCount > 0) && (create_info.pLibraryInterface == nullptr)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590", device,
                                 create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount),
                                 "is %" PRIu32 ", but pLibraryInterface is NULL.", create_info.pLibraryInfo->libraryCount);
            }
        }

        if (create_info.pLibraryInfo == nullptr) {
            if (create_info.stageCount == 0) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999", device,
                                 create_info_loc.dot(Field::pLibraryInfo), "is NULL and stageCount is zero.");
            }
            if (((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (create_info.groupCount == 0)) {
                skip |=
                    LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700", device, create_info_loc.dot(Field::pLibraryInfo),
                             "is NULL and flags is missing VK_PIPELINE_CREATE_LIBRARY_BIT_KHR (%s).",
                             string_VkPipelineCreateFlags2(flags).c_str());
            }
        } else if (create_info.pLibraryInfo->libraryCount == 0) {
            if (create_info.stageCount == 0) {
                skip |=
                    LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999", device,
                             create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount), "is zero and stageCount is zero.");
            }
            if (((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) && (create_info.groupCount == 0)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700", device,
                                 create_info_loc.dot(Field::pLibraryInfo).dot(Field::libraryCount),
                                 "is zero and flags is missing VK_PIPELINE_CREATE_LIBRARY_BIT_KHR (%s).",
                                 string_VkPipelineCreateFlags2(flags).c_str());
            }
        }

        if (create_info.pLibraryInterface) {
            if (create_info.pLibraryInterface->maxPipelineRayHitAttributeSize >
                phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize) {
                skip |= LogError(
                    "VUID-VkRayTracingPipelineInterfaceCreateInfoKHR-maxPipelineRayHitAttributeSize-03605", device,
                    create_info_loc.dot(Field::pLibraryInterface).dot(Field::maxPipelineRayHitAttributeSize),
                    "(%" PRIu32 ") is larger than VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayHitAttributeSize (%" PRIu32
                    ").",
                    create_info.pLibraryInterface->maxPipelineRayHitAttributeSize,
                    phys_dev_ext_props.ray_tracing_props_khr.maxRayHitAttributeSize);
            }
        }

        if (deferredOperation != VK_NULL_HANDLE) {
            if (flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT) {
                skip |= LogError(
                    "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-03587", device, flags_loc,
                    "includes VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT but deferredOperation is not VK_NULL_HANDLE.");
            }
        }

        if (create_info.pDynamicState) {
            for (uint32_t j = 0; j < create_info.pDynamicState->dynamicStateCount; ++j) {
                if (create_info.pDynamicState->pDynamicStates[j] != VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602", device,
                                     create_info_loc.dot(Field::pDynamicState).dot(Field::pDynamicStates, j), "is %s.",
                                     string_VkDynamicState(create_info.pDynamicState->pDynamicStates[j]));
                }
            }
        }

        skip |=
            ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info.layout, create_info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412", device, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(pInfo->mode));
    }
    if (!enabled_features.accelerationStructureHostCommands) {
        skip |= LogError("VUID-vkCopyAccelerationStructureToMemoryKHR-accelerationStructureHostCommands-03584", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }
    skip |= context.ValidateRequiredPointer(info_loc.dot(Field::dst).dot(Field::hostAddress), pInfo->dst.hostAddress,
                                            "VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03732");
    if (!IsPointerAligned(pInfo->dst.hostAddress, 16)) {
        skip |=
            LogError("VUID-vkCopyAccelerationStructureToMemoryKHR-pInfo-03751", device,
                     info_loc.dot(Field::dst).dot(Field::hostAddress), "(%p) must be aligned to 16 bytes.", pInfo->dst.hostAddress);
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                           const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-accelerationStructure-08926", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR) {
        skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-mode-03412", commandBuffer, info_loc.dot(Field::mode),
                         "is %s (must be VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR).",
                         string_VkCopyAccelerationStructureModeKHR(pInfo->mode));
    }
    if (!IsPointerAligned(pInfo->dst.deviceAddress, 256)) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03740", commandBuffer,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->dst.deviceAddress);
    }
    return skip;
}

bool Device::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR &as_info,
                                                      const VulkanTypedHandle &handle, const Location &info_loc) const {
    bool skip = false;
    if (!(as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR ||
          as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        const LogObjectList objlist(handle);
        skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-mode-03410", objlist, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(as_info.mode));
    }
    return skip;
}

bool Device::manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, error_obj.location.dot(Field::pInfo));
    if (!enabled_features.accelerationStructureHostCommands) {
        skip |= LogError("VUID-vkCopyAccelerationStructureKHR-accelerationStructureHostCommands-03582", device, error_obj.location,
                         "feature was not enabled.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureKHR-accelerationStructure-08925", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, error_obj.location.dot(Field::pInfo));
    return skip;
}

bool Device::ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR &as_info,
                                                              const VulkanTypedHandle &handle, const Location &loc) const {
    bool skip = false;
    if (as_info.mode != VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR) {
        const LogObjectList objlist(handle);
        skip |= LogError("VUID-VkCopyMemoryToAccelerationStructureInfoKHR-mode-03413", objlist, loc.dot(Field::mode), "is %s.",
                         string_VkCopyAccelerationStructureModeKHR(as_info.mode));
    }
    return skip;
}

bool Device::manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    if (!enabled_features.accelerationStructureHostCommands) {
        skip |= LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-accelerationStructureHostCommands-03583", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }

    if (!pInfo->src.hostAddress) {
        skip |= LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03729", device,
                         info_loc.dot(Field::src).dot(Field::hostAddress), "is zero.");
    } else if (!IsPointerAligned(pInfo->src.hostAddress, 16)) {
        skip |=
            LogError("VUID-vkCopyMemoryToAccelerationStructureKHR-pInfo-03750", device,
                     info_loc.dot(Field::src).dot(Field::hostAddress), "(%p) must be aligned to 16 bytes.", pInfo->src.hostAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                           const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-accelerationStructure-08927", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    if (!IsPointerAligned(pInfo->src.deviceAddress, 256)) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03743", commandBuffer,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->src.deviceAddress);
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-accelerationStructure-08924", commandBuffer,
                         error_obj.location, "accelerationStructure feature was not enabled.");
    }

    if (!IsValueIn(queryType,
                   {VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR})) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742", commandBuffer,
                         error_obj.location.dot(Field::queryType), "is %s.", string_VkQueryType(queryType));
    }
    return skip;
}

bool Device::manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void *pData, size_t stride, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.accelerationStructureHostCommands) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructureHostCommands-03585", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }
    if (dataSize < accelerationStructureCount * stride) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-dataSize-03452", device,
                         error_obj.location.dot(Field::dataSize),
                         "(%zu) is less than "
                         "accelerationStructureCount (%" PRIu32 ") x stride (%zu).",
                         dataSize, accelerationStructureCount, stride);
    }

    const Location query_type_loc = error_obj.location.dot(Field::queryType);

    if (!IsValueIn(queryType,
                   {VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR})) {
        skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742", device, query_type_loc, "is %s.",
                         string_VkQueryType(queryType));
    }

    if (dataSize < sizeof(VkDeviceSize)) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03449", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, but dataSize is %zu.", dataSize);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03451", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, but dataSize is %zu.", dataSize);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
            skip |= LogError(
                "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06734", device, query_type_loc,
                "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but dataSize is %zu.", dataSize);
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06732", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but dataSize is %zu.", dataSize);
        }
    }

    if (!IsIntegerMultipleOf(stride, sizeof(VkDeviceSize))) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, but then stride (%zu) must be a multiple "
                             "of the size of VkDeviceSize (%zu).",
                             stride, sizeof(VkDeviceSize));
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, but then stride (%zu) must be a "
                             "multiple of the size of VkDeviceSize (%zu).",
                             stride, sizeof(VkDeviceSize));
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but then stride (%zu) must be a multiple of the "
                             "size of VkDeviceSize (%zu).",
                             stride, sizeof(VkDeviceSize));
        } else if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733", device, query_type_loc,
                             "is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but then stride "
                             "(%zu) must be a multiple of the size of VkDeviceSize (%zu).",
                             stride, sizeof(VkDeviceSize));
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                   uint32_t firstGroup, uint32_t groupCount,
                                                                                   size_t dataSize, void *pData,
                                                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.rayTracingPipelineShaderGroupHandleCaptureReplay) {
        skip |= LogError(
            "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-rayTracingPipelineShaderGroupHandleCaptureReplay-03606", device,
            error_obj.location, "rayTracingPipelineShaderGroupHandleCaptureReplay feature was not enabled.");
    }
    return skip;
}

bool Device::manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
    VkAccelerationStructureCompatibilityKHR *pCompatibility, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-accelerationStructure-08928", device,
                         error_obj.location, "accelerationStructure feature was not enabled.");
    }
    return skip;
}

bool Device::ValidateTotalPrimitivesCount(uint64_t total_triangles_count, uint64_t total_aabbs_count,
                                          const VulkanTypedHandle &handle, const Location &loc) const {
    bool skip = false;

    if (total_triangles_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795", handle, loc,
                         "total number of triangles in all geometries (%" PRIu64
                         ") is larger than maxPrimitiveCount "
                         "(%" PRIu64 ")",
                         total_triangles_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
    }

    if (total_aabbs_count > phys_dev_ext_props.acc_structure_props.maxPrimitiveCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794", handle, loc,
                         "total number of AABBs in all geometries (%" PRIu64
                         ") is larger than maxPrimitiveCount "
                         "(%" PRIu64 ")",
                         total_aabbs_count, phys_dev_ext_props.acc_structure_props.maxPrimitiveCount);
    }

    return skip;
}

bool Device::ValidateAccelerationStructureBuildGeometryInfo(const VulkanTypedHandle& handle,
                                                            const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                            const Location& info_loc) const {
    bool skip = false;

    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654", handle, info_loc.dot(Field::type),
                         "must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.");
    }
    if (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR &&
        info.flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-03796", handle, info_loc.dot(Field::flags),
                         "has the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR bit set,"
                         "then it must not have the VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR bit set.");
    }
    if (info.pGeometries && info.ppGeometries) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788", handle, info_loc,
                         "both pGeometries and ppGeometries are not NULL.");
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && info.geometryCount != 1) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03790", handle, info_loc.dot(Field::type),
                         "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but geometryCount is %" PRIu32 ".", info.geometryCount);
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
        info.geometryCount > phys_dev_ext_props.acc_structure_props.maxGeometryCount) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03793", handle, info_loc.dot(Field::type),
                         "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR but geometryCount (%" PRIu32
                         ") is greater than maxGeometryCount (%" PRIu64 ").",
                         info.geometryCount, phys_dev_ext_props.acc_structure_props.maxGeometryCount);
    }

    if (info.geometryCount > 0 && !info.pGeometries && !info.ppGeometries) {
        skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788", handle,
                         info_loc.dot(Field::geometryCount),
                         "is (%" PRIu32 ") but both pGeometries and ppGeometries are both NULL.", info.geometryCount);
    }

    return skip;
}

bool Device::ValidateAccelerationStructureGeometry(const Context& context, const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                   const VkAccelerationStructureGeometryKHR& geom,
                                                   const Location& geometry_ptr_loc) const {
    bool skip = false;

    const Location geometry_loc = geometry_ptr_loc.dot(Field::geometry);

    skip |= context.ValidateRangedEnum(geometry_ptr_loc.dot(Field::geometryType), vvl::Enum::VkGeometryTypeKHR, geom.geometryType,
                                       "VUID-VkAccelerationStructureGeometryKHR-geometryType-parameter");
    if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        const Location triangles_loc = geometry_loc.dot(Field::triangles);
        const VkAccelerationStructureGeometryTrianglesDataKHR& triangles = geom.geometry.triangles;
        skip |= ValidateAccelerationStructureGeometryTrianglesDataKHR(context, triangles, triangles_loc);

        if (triangles.vertexStride > vvl::kU32Max) {
            skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819", context.error_obj.handle,
                             triangles_loc.dot(Field::vertexStride), "(%" PRIu64 ") must be less than or equal to 2^32-1.",
                             triangles.vertexStride);
        }
        if (!IsValueIn(triangles.indexType, {VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, VK_INDEX_TYPE_NONE_KHR})) {
            skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798", context.error_obj.handle,
                             triangles_loc.dot(Field::indexType), "is %s.", string_VkIndexType(triangles.indexType));
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        const Location instances_loc = geometry_loc.dot(Field::instances);

        skip |= ValidateAccelerationStructureGeometryInstancesDataKHR(context, geom.geometry.instances, instances_loc);
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        const Location aabbs_loc = geometry_loc.dot(Field::aabbs);
        const VkAccelerationStructureGeometryAabbsDataKHR& aabbs = geom.geometry.aabbs;

        skip |= ValidateAccelerationStructureGeometryAabbsDataKHR(context, aabbs, aabbs_loc);

        if (!IsIntegerMultipleOf(aabbs.stride, 8)) {
            skip |= LogError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545", context.error_obj.handle,
                             aabbs_loc.dot(Field::stride), "(%" PRIu64 ") is not a multiple of 8.", aabbs.stride);
        }
        if (aabbs.stride > vvl::kU32Max) {
            skip |= LogError("VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820", context.error_obj.handle,
                             aabbs_loc.dot(Field::stride), "(%" PRIu64 ") must be less than or equal to 2^32-1.", aabbs.stride);
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_SPHERES_NV) {
        auto sphere_struct = reinterpret_cast<VkAccelerationStructureGeometrySpheresDataNV const*>(geom.pNext);
        if (!enabled_features.spheres) {
            skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-None-10429", device,
                             geometry_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV),
                             "The spheres feature must be enabled");
        }
        if (sphere_struct->vertexStride > vvl::kU32Max) {
            skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432", context.error_obj.handle,
                             geometry_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV).dot(Field::vertexStride),
                             "(%" PRIu64 ") must be less than or equal to 2^32-1.", sphere_struct->vertexStride);
        }
        if (sphere_struct->radiusStride > vvl::kU32Max) {
            skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432", context.error_obj.handle,
                             geometry_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV).dot(Field::radiusStride),
                             "(%" PRIu64 ") must be less than or equal to 2^32-1.", sphere_struct->radiusStride);
        }
        if (!IsValueIn(sphere_struct->indexType, {VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, VK_INDEX_TYPE_NONE_KHR})) {
            skip |= LogError("VUID-VkAccelerationStructureGeometrySpheresDataNV-indexData-10437", context.error_obj.handle,
                             geometry_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV).dot(Field::indexType),
                             "is %s.", string_VkIndexType(sphere_struct->indexType));
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_LINEAR_SWEPT_SPHERES_NV) {
        auto sphere_linear_struct = reinterpret_cast<VkAccelerationStructureGeometryLinearSweptSpheresDataNV const*>(geom.pNext);
        if (!enabled_features.linearSweptSpheres) {
            skip |= LogError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-None-10419", device,
                             geometry_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV),
                             "The linearSweptSpheres feature must be enabled");
        }
        if (sphere_linear_struct->vertexStride > vvl::kU32Max) {
            skip |= LogError(
                "VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422", context.error_obj.handle,
                geometry_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV).dot(Field::vertexStride),
                "(%" PRIu64 ") must be less than or equal to 2^32-1.", sphere_linear_struct->vertexStride);
        }
        if (sphere_linear_struct->radiusStride > vvl::kU32Max) {
            skip |= LogError(
                "VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422", context.error_obj.handle,
                geometry_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV).dot(Field::radiusStride),
                "(%" PRIu64 ") must be less than or equal to 2^32-1.", sphere_linear_struct->radiusStride);
        }
        if (!IsValueIn(sphere_linear_struct->indexType, {VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, VK_INDEX_TYPE_NONE_KHR})) {
            skip |=
                LogError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-indexData-10428", context.error_obj.handle,
                         geometry_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV).dot(Field::indexType),
                         "is %s.", string_VkIndexType(sphere_linear_struct->indexType));
        }
    }

    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR && geom.geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        skip |=
            LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03789", context.error_obj.handle,
                     geometry_ptr_loc.dot(Field::geometryType),
                     "is %s but VkAccelerationStructureBuildGeometryInfoKHR::type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR.",
                     string_VkGeometryTypeKHR(geom.geometryType));
    }
    if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
        if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
            skip |= LogError(
                "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03791", context.error_obj.handle,
                geometry_ptr_loc.dot(Field::geometryType),
                "is %s but VkAccelerationStructureBuildGeometryInfoKHR::type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR.",
                string_VkGeometryTypeKHR(geom.geometryType));
        }
        if (geom.geometryType != rt::GetGeometry(info, 0).geometryType) {
            skip |= LogError("VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03792", context.error_obj.handle,
                             geometry_ptr_loc.dot(Field::geometryType), "(%s) is different than pGeometries[0].geometryType (%s)",
                             string_VkGeometryTypeKHR(geom.geometryType),
                             string_VkGeometryTypeKHR(rt::GetGeometry(info, 0).geometryType));
        }
    }

    return skip;
}

bool Device::ValidateAccelerationStructureGeometryHost(const Context& context,
                                                       const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                                       const VkAccelerationStructureGeometryKHR& geom,
                                                       const Location& geometry_ptr_loc) const {
    bool skip = false;
    const Location geometry_loc = geometry_ptr_loc.dot(Field::geometry);

    if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        const VkAccelerationStructureGeometryTrianglesDataKHR& triangles = geom.geometry.triangles;
        if (!triangles.vertexData.hostAddress) {
            skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03771", device,
                             geometry_loc.dot(Field::triangles).dot(Field::vertexData).dot(Field::hostAddress), "is NULL.");
        }
        if (triangles.indexType != VK_INDEX_TYPE_NONE_KHR && !triangles.indexData.hostAddress) {
            skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03772", device,
                             geometry_loc.dot(Field::triangles).dot(Field::indexData).dot(Field::hostAddress), "is NULL.");
        }

        if (const auto* micromap =
                vku::FindStructInPNextChain<VkAccelerationStructureTrianglesOpacityMicromapEXT>(triangles.pNext)) {
            if (micromap->indexType != VK_INDEX_TYPE_NONE_KHR && !micromap->indexBuffer.hostAddress) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-10892", device,
                                 geometry_loc.dot(Field::triangles)
                                     .pNext(Struct::VkAccelerationStructureTrianglesOpacityMicromapEXT, Field::indexBuffer)
                                     .dot(Field::hostAddress),
                                 "is NULL.");
            }
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        if (!geom.geometry.aabbs.data.hostAddress) {
            skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03774", device,
                             geometry_loc.dot(Field::aabbs).dot(Field::data).dot(Field::hostAddress), "is NULL.");
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (!geom.geometry.instances.data.hostAddress) {
            skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03778", device,
                             geometry_loc.dot(Field::instances).dot(Field::data).dot(Field::hostAddress), "is NULL.");
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_SPHERES_NV) {
        auto sphere_struct = reinterpret_cast<VkAccelerationStructureGeometrySpheresDataNV const*>(geom.pNext);
        if (sphere_struct) {
            if (sphere_struct->indexType == VK_INDEX_TYPE_NONE_KHR) {
                if (sphere_struct->indexData.hostAddress != 0) {
                    skip |=
                        LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11822", device,
                                 geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV, Field::indexData)
                                     .dot(Field::hostAddress),
                                 "(%p) is not 0 when indexType is VK_INDEX_TYPE_NONE_KHR.", sphere_struct->indexData.hostAddress);
                }
            } else {
                if (sphere_struct->indexData.hostAddress == 0) {
                    skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11823", device,
                                     geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV, Field::indexData)
                                         .dot(Field::hostAddress),
                                     "is 0");
                }
            }
            if (sphere_struct->vertexData.hostAddress == 0) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11824", device,
                                 geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV, Field::vertexData)
                                     .dot(Field::hostAddress),
                                 "is 0");
            }
            if (sphere_struct->radiusData.hostAddress == 0) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-11825", device,
                                 geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometrySpheresDataNV, Field::radiusData)
                                     .dot(Field::hostAddress),
                                 "is 0");
            }
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_LINEAR_SWEPT_SPHERES_NV) {
        auto sphere_linear_struct = reinterpret_cast<VkAccelerationStructureGeometryLinearSweptSpheresDataNV const*>(geom.pNext);
        if (sphere_linear_struct) {
            if (sphere_linear_struct->indexType == VK_INDEX_TYPE_NONE_KHR) {
                if (sphere_linear_struct->indexData.hostAddress != 0) {
                    skip |= LogError(
                        "VUID-vkBuildAccelerationStructuresKHR-pInfos-11826", device,
                        geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV, Field::indexData)
                            .dot(Field::hostAddress),
                        "(%p) is not 0 when indexType is VK_INDEX_TYPE_NONE_KHR.", sphere_linear_struct->indexData.hostAddress);
                }
            } else {
                if (sphere_linear_struct->indexData.hostAddress == 0) {
                    skip |= LogError(
                        "VUID-vkBuildAccelerationStructuresKHR-pInfos-11827", device,
                        geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV, Field::indexData)
                            .dot(Field::hostAddress),
                        "is 0");
                }
            }
            if (sphere_linear_struct->vertexData.hostAddress == 0) {
                skip |= LogError(
                    "VUID-vkBuildAccelerationStructuresKHR-pInfos-11828", device,
                    geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV, Field::vertexData)
                        .dot(Field::hostAddress),
                    "is 0");
            }
            if (sphere_linear_struct->radiusData.hostAddress == 0) {
                skip |= LogError(
                    "VUID-vkBuildAccelerationStructuresKHR-pInfos-11829", device,
                    geometry_ptr_loc.pNext(Struct::VkAccelerationStructureGeometryLinearSweptSpheresDataNV, Field::radiusData)
                        .dot(Field::hostAddress),
                    "is 0");
            }
        }
    }

    return skip;
}

bool Device::ValidateAccelerationStructureGeometryDevice(const Context& context, const VkAccelerationStructureGeometryKHR& geom,
                                                         const Location& geometry_ptr_loc) const {
    bool skip = false;
    const Location geometry_loc = geometry_ptr_loc.dot(Field::geometry);

    const auto pick_vuid = [&geometry_ptr_loc](const char* direct_build_vu, const char* indirect_build_vu) -> const char* {
        return geometry_ptr_loc.function == Func::vkCmdBuildAccelerationStructuresKHR ? direct_build_vu : indirect_build_vu;
    };

    if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        const VkAccelerationStructureGeometryTrianglesDataKHR& triangles = geom.geometry.triangles;
        const uint32_t index_type_size = IndexTypeByteSize(triangles.indexType);
        if (triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
            if (!IsPointerAligned(triangles.indexData.deviceAddress, index_type_size)) {
                skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03712",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03712"),
                                 context.error_obj.handle,
                                 geometry_loc.dot(Field::triangles).dot(Field::indexData).dot(Field::deviceAddress),
                                 "(0x%" PRIx64 ") is not aligned to %" PRIu32 " (the corresponding geometry's VkIndexType of %s).",
                                 triangles.indexData.deviceAddress, index_type_size, string_VkIndexType(triangles.indexType));
            }

            if (!IsPointerAligned(triangles.transformData.deviceAddress, 16)) {
                skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03810",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03810"),
                                 context.error_obj.handle,
                                 geometry_loc.dot(Field::triangles).dot(Field::transformData).dot(Field::deviceAddress),
                                 "(0x%" PRIx64 ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR.",
                                 triangles.transformData.deviceAddress);
            }
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        const VkAccelerationStructureGeometryAabbsDataKHR& aabbs = geom.geometry.aabbs;
        if (!IsPointerAligned(aabbs.data.deviceAddress, 8)) {
            skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03714",
                                       "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03714"),
                             context.error_obj.handle, geometry_loc.dot(Field::aabbs).dot(Field::data).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_AABBS_KHR.",
                             aabbs.data.deviceAddress);
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        const VkAccelerationStructureGeometryInstancesDataKHR& instances = geom.geometry.instances;
        if (instances.arrayOfPointers == VK_TRUE) {
            if (!IsPointerAligned(instances.data.deviceAddress, 8)) {
                skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03716",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03716"),
                                 context.error_obj.handle,
                                 geometry_loc.dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                 "(0x%" PRIx64
                                 ") must be aligned to 8 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                 "geometry.instances.arrayOfPointers is "
                                 "VK_TRUE.",
                                 instances.data.deviceAddress);
            }
        } else {
            if (!IsPointerAligned(instances.data.deviceAddress, 16)) {
                skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03715",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03715"),
                                 context.error_obj.handle,
                                 geometry_loc.dot(Field::instances).dot(Field::data).dot(Field::deviceAddress),
                                 "(0x%" PRIx64
                                 ") must be aligned to 16 bytes when geometryType is VK_GEOMETRY_TYPE_INSTANCES_KHR and "
                                 "geometry.instances.arrayOfPointers is VK_FALSE.",
                                 instances.data.deviceAddress);
            }
        }
    }

    return skip;
}

bool Device::ValidateAccelerationStructureBuildRangeInfo(const Context& context, const VkAccelerationStructureGeometryKHR& geom,
                                                         const VkAccelerationStructureBuildRangeInfoKHR& build_range,
                                                         const Location& geometry_ptr_loc, const Location& build_range_loc) const {
    bool skip = false;

    if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        const VkAccelerationStructureGeometryTrianglesDataKHR& triangles = geom.geometry.triangles;
        const uint32_t index_type_size = IndexTypeByteSize(triangles.indexType);
        if (triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
            if (!IsIntegerMultipleOf(build_range.primitiveOffset, index_type_size)) {
                skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-primitiveOffset-03656", context.error_obj.handle,
                                 build_range_loc.dot(Field::primitiveOffset),
                                 "(%" PRIu32 ") is not a multiple of %" PRIu32 " (the corresponding geometry's VkIndexType of %s).",
                                 build_range.primitiveOffset, index_type_size, string_VkIndexType(triangles.indexType));
            }
        } else {
            if (build_range.primitiveCount > 0) {
                const uint64_t build_range_max_vertex =
                    uint64_t(build_range.firstVertex) + 3 * uint64_t(build_range.primitiveCount) - 1;
                if (uint64_t(triangles.maxVertex) < build_range_max_vertex) {
                    skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-None-10775", context.error_obj.handle,
                                     geometry_ptr_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::maxVertex),
                                     "is %" PRIu32 " but for %s, firstVertex ( %" PRIu32 " ) + primitiveCount ( %" PRIu32
                                     " ) x 3 - 1 = %" PRIu64 ".",
                                     triangles.maxVertex, build_range_loc.Fields().c_str(), build_range.firstVertex,
                                     build_range.primitiveCount, build_range_max_vertex);
                }
            }

            const uint32_t vertex_format_size = vkuFormatTexelBlockSize(triangles.vertexFormat);
            if (!IsIntegerMultipleOf(build_range.primitiveOffset, vertex_format_size)) {
                skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-primitiveOffset-03657", context.error_obj.handle,
                                 build_range_loc.dot(Field::primitiveOffset),
                                 "(%" PRIu32 ") is not a multiple of %" PRIu32
                                 " (the corresponding geometry's triangles vertexFormat %s).",
                                 build_range.primitiveOffset, vertex_format_size, string_VkFormat(triangles.vertexFormat));
            }
        }

        if (!IsIntegerMultipleOf(build_range.transformOffset, 16u)) {
            skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-transformOffset-03658", context.error_obj.handle,
                             build_range_loc.dot(Field::transformOffset), "(%" PRIu32 ") is not a multiple of 16.",
                             build_range.transformOffset);
        }

    } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        if (!IsIntegerMultipleOf(build_range.primitiveOffset, 8u)) {
            skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-primitiveOffset-03659", context.error_obj.handle,
                             build_range_loc.dot(Field::primitiveOffset), "(%" PRIu32 ") is not a multiple of 8.",
                             build_range.primitiveOffset);
        }
    } else if (geom.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (build_range.primitiveCount > phys_dev_ext_props.acc_structure_props.maxInstanceCount) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03801", context.error_obj.handle,
                             build_range_loc.dot(Field::primitiveCount),
                             "(%" PRIu32
                             ") is greater than to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxInstanceCount "
                             "(%" PRIu64 ").",
                             build_range.primitiveCount, phys_dev_ext_props.acc_structure_props.maxInstanceCount);
        }

        if (!IsIntegerMultipleOf(build_range.primitiveOffset, 16u)) {
            skip |= LogError("VUID-VkAccelerationStructureBuildRangeInfoKHR-primitiveOffset-03660", context.error_obj.handle,
                             build_range_loc.dot(Field::primitiveOffset), "(%" PRIu32 ") is not a multiple of 16.",
                             build_range.primitiveOffset);
        }
    }

    return skip;
}

static void ComputeTotalPrimitiveCountWithBuildRanges(uint32_t info_count,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_infos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR *const *build_ranges,
                                                      uint64_t *out_total_triangles_count, uint64_t *out_total_aabbs_count) {
    *out_total_triangles_count = 0;
    *out_total_aabbs_count = 0;

    for (const auto [info_i, info] : vvl::enumerate(build_geometry_infos, info_count)) {
        if (!info.pGeometries && !info.ppGeometries) {
            *out_total_triangles_count = 0;
            *out_total_aabbs_count = 0;
            return;
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(info, geom_i);
            if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                *out_total_triangles_count += build_ranges[info_i][geom_i].primitiveCount;
            } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                *out_total_aabbs_count += build_ranges[info_i][geom_i].primitiveCount;
            }
        }
    }

    return;
}

static void ComputeTotalPrimitiveCountWithMaxPrimitivesCount(
    uint32_t info_count, const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_infos,
    const uint32_t *const *max_primitives, uint64_t *out_total_triangles_count, uint64_t *out_total_aabbs_count) {
    *out_total_triangles_count = 0;
    *out_total_aabbs_count = 0;

    for (const auto [info_i, info] : vvl::enumerate(build_geometry_infos, info_count)) {
        if (!info.pGeometries && !info.ppGeometries) {
            *out_total_triangles_count = 0;
            *out_total_aabbs_count = 0;
            return;
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(info, geom_i);
            if (geom.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                *out_total_triangles_count += max_primitives[info_i][geom_i];
            } else if (geom.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
                *out_total_aabbs_count += max_primitives[info_i][geom_i];
            }
        }
    }

    return;
}

bool Device::manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-accelerationStructure-08923", commandBuffer, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithBuildRanges(infoCount, pInfos, ppBuildRangeInfos, &total_triangles_count, &total_aabbs_count);
    skip |= ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfo(error_obj.handle, info, info_loc);

        if (!IsPointerAligned(info.scratchData.deviceAddress,
                              phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03710", commandBuffer,
                             info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to minAccelerationStructureScratchOffsetAlignment (%" PRIu32 ").",
                             info.scratchData.deviceAddress,
                             phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        }

        skip |= context.ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info.mode,
                                           "VUID-vkCmdBuildAccelerationStructuresKHR-mode-04628");
        skip |= context.ValidateArray(info_loc.dot(Field::geometryCount), error_obj.location.dot(Field::ppBuildRangeInfos, info_i),
                                      info.geometryCount, &ppBuildRangeInfos[info_i], false, true, kVUIDUndefined,
                                      "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-03676");

        if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && info.srcAccelerationStructure == VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630", commandBuffer, info_loc.dot(Field::mode),
                             "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure is VK_NULL_HANDLE.");
        }

        for (const auto [other_info_j, other_info] : vvl::enumerate(pInfos, infoCount)) {
            if (info_i == other_info_j) {
                continue;
            }
            if (info.dstAccelerationStructure == other_info.dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info.dstAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03698", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.dstAccelerationStructure).c_str());
                break;
            }
            if (info.srcAccelerationStructure == other_info.dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03403", objlist,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.srcAccelerationStructure).c_str());
                break;
            }
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geometry = rt::GetGeometry(info, geom_i);
            const Location geometry_ptr_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
            skip |= ValidateAccelerationStructureGeometry(context, info, geometry, geometry_ptr_loc);
            skip |= ValidateAccelerationStructureGeometryDevice(context, geometry, geometry_ptr_loc);

            const VkAccelerationStructureBuildRangeInfoKHR& build_range = ppBuildRangeInfos[info_i][geom_i];
            skip |= ValidateAccelerationStructureBuildRangeInfo(
                context, geometry, build_range, geometry_ptr_loc,
                error_obj.location.dot(Field::ppBuildRangeInfos, info_i).brackets(geom_i));
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts,
    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructureIndirectBuild) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-accelerationStructureIndirectBuild-03650", commandBuffer,
                         error_obj.location, "the accelerationStructureIndirectBuild feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithMaxPrimitivesCount(infoCount, pInfos, ppMaxPrimitiveCounts, &total_triangles_count,
                                                     &total_aabbs_count);
    skip |= ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfo(error_obj.handle, pInfos[info_i], info_loc);

        if (!IsPointerAligned(info.scratchData.deviceAddress,
                              phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03710", commandBuffer,
                             info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to minAccelerationStructureScratchOffsetAlignment (%" PRIu32 ").",
                             info.scratchData.deviceAddress,
                             phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        }

        if (!IsPointerAligned(pIndirectDeviceAddresses[info_i], 4)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectDeviceAddresses-03648", commandBuffer,
                             error_obj.location.dot(Field::pIndirectStrides, info_i), "(0x%" PRIx64 ") is not aligned to 4 bytes.",
                             pIndirectDeviceAddresses[info_i]);
        }

        if (!IsIntegerMultipleOf(pIndirectStrides[info_i], 4)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pIndirectStrides-03787", commandBuffer,
                             error_obj.location.dot(Field::pIndirectStrides, info_i), "(%" PRIu32 ") is not a multiple of 4.",
                             pIndirectStrides[info_i]);
        }

        skip |= context.ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info.mode,
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-mode-04628");

        if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && info.srcAccelerationStructure == VK_NULL_HANDLE) {
            skip |=
                LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-04630", commandBuffer, info_loc.dot(Field::mode),
                         "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but srcAccelerationStructure is VK_NULL_HANDLE.");
        }

        for (const auto [other_info_j, other_info] : vvl::enumerate(pInfos, infoCount)) {
            if (info_i == other_info_j) {
                continue;
            }
            if (info.dstAccelerationStructure == other_info.dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info.dstAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03698", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.dstAccelerationStructure).c_str());
                break;
            }
            if (info.srcAccelerationStructure == other_info.dstAccelerationStructure) {
                const LogObjectList objlist(commandBuffer, info.srcAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03403", objlist,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.srcAccelerationStructure).c_str());
                break;
            }
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geometry = rt::GetGeometry(info, geom_i);
            const Location geometry_ptr_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
            skip |= ValidateAccelerationStructureGeometry(context, info, geometry, geometry_ptr_loc);
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructureHostCommands) {
        skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-accelerationStructureHostCommands-03581", device,
                         error_obj.location, "accelerationStructureHostCommands feature was not enabled.");
    }

    uint64_t total_triangles_count = 0;
    uint64_t total_aabbs_count = 0;
    ComputeTotalPrimitiveCountWithBuildRanges(infoCount, pInfos, ppBuildRangeInfos, &total_triangles_count, &total_aabbs_count);
    skip |= ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        skip |= ValidateAccelerationStructureBuildGeometryInfo(error_obj.handle, info, info_loc);

        skip |= context.ValidateRangedEnum(info_loc.dot(Field::mode), vvl::Enum::VkBuildAccelerationStructureModeKHR, info.mode,
                                           "VUID-vkBuildAccelerationStructuresKHR-mode-04628");

        skip |= context.ValidateArray(info_loc.dot(Field::geometryCount), error_obj.location.dot(Field::ppBuildRangeInfos, info_i),
                                      info.geometryCount, &ppBuildRangeInfos[info_i], false, true, kVUIDUndefined,
                                      "VUID-vkBuildAccelerationStructuresKHR-ppBuildRangeInfos-03676");
        if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR) {
            const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
            if (scratch_size > 0 && !info.scratchData.hostAddress) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-12244", device,
                                 info_loc.dot(Field::scratchData).dot(Field::hostAddress), "is NULL.");
            }
        } else if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
            const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
            if (scratch_size > 0 && !info.scratchData.hostAddress) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-12245", device,
                                 info_loc.dot(Field::scratchData).dot(Field::hostAddress), "is NULL.");
            }

            if (info.srcAccelerationStructure == VK_NULL_HANDLE) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-04630", device, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but %s is VK_NULL_HANDLE.",
                                 info_loc.dot(Field::srcAccelerationStructure).Fields().c_str());
            }
        }

        for (const auto [other_info_j, other_info] : vvl::enumerate(pInfos, infoCount)) {
            if (info_i == other_info_j) {
                continue;
            }
            if (info.dstAccelerationStructure == other_info.dstAccelerationStructure) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03698", device,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.dstAccelerationStructure).c_str());
                break;
            }
            if (info.srcAccelerationStructure == other_info.dstAccelerationStructure) {
                skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03403", device,
                                 info_loc.dot(Field::srcAccelerationStructure),
                                 "and pInfos[%" PRIu32 "].dstAccelerationStructure are both %s.", other_info_j,
                                 FormatHandle(info.srcAccelerationStructure).c_str());
                break;
            }
        }

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR& geometry = rt::GetGeometry(info, geom_i);
            const Location geometry_ptr_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
            skip |= ValidateAccelerationStructureGeometry(context, info, geometry, geometry_ptr_loc);
            skip |= ValidateAccelerationStructureGeometryHost(context, info, geometry, geometry_ptr_loc);
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
    const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo, const Context &context) const {
    bool skip = false;

    const auto &error_obj = context.error_obj;

    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkGetAccelerationStructureBuildSizesKHR-accelerationStructure-08933", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    if (pBuildInfo) {
        const Location build_info_loc = error_obj.location.dot(Field::pBuildInfo);
        const VkAccelerationStructureBuildGeometryInfoKHR& build_info = *pBuildInfo;

        if (build_info.geometryCount != 0 && !pMaxPrimitiveCounts) {
            skip |= LogError("VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03619", device,
                             build_info_loc.dot(Field::geometryCount), "is %" PRIu32 ", but pMaxPrimitiveCounts is NULL.",
                             build_info.geometryCount);
        }

        if (pMaxPrimitiveCounts) {
            uint64_t total_triangles_count = 0;
            uint64_t total_aabbs_count = 0;
            ComputeTotalPrimitiveCountWithMaxPrimitivesCount(1, pBuildInfo, &pMaxPrimitiveCounts, &total_triangles_count,
                                                             &total_aabbs_count);
            skip |= ValidateTotalPrimitivesCount(total_triangles_count, total_aabbs_count, error_obj.handle, error_obj.location);
        }

        skip |=
            ValidateAccelerationStructureBuildGeometryInfo(error_obj.handle, build_info, error_obj.location.dot(Field::pBuildInfo));

        if (pBuildInfo->pGeometries || pBuildInfo->ppGeometries) {
            for (uint32_t geom_i = 0; geom_i < build_info.geometryCount; ++geom_i) {
                const VkAccelerationStructureGeometryKHR& geometry = rt::GetGeometry(build_info, geom_i);
                const Location geometry_ptr_loc =
                    build_info_loc.dot(build_info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
                skip |= ValidateAccelerationStructureGeometry(context, build_info, geometry, geometry_ptr_loc);

                if (pMaxPrimitiveCounts && geometry.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                    if (pMaxPrimitiveCounts[geom_i] > phys_dev_ext_props.acc_structure_props.maxInstanceCount) {
                        skip |= LogError(
                            "VUID-vkGetAccelerationStructureBuildSizesKHR-pBuildInfo-03785", device,
                            geometry_ptr_loc.dot(Field::geometryType),
                            "is %s, but pMaxPrimitiveCount[%" PRIu32 "] (%" PRIu32
                            ") is larger than VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxInstanceCount (%" PRIu64 ").",
                            string_VkGeometryTypeKHR(geometry.geometryType), geom_i, pMaxPrimitiveCounts[geom_i],
                            phys_dev_ext_props.acc_structure_props.maxInstanceCount);
                    }
                }
            }
        }
    }

    return skip;
}

bool Device::ValidateTraceRaysRaygenShaderBindingTable(VkCommandBuffer commandBuffer,
                                                       const VkStridedDeviceAddressRegionKHR &raygen_shader_binding_table,
                                                       const Location &table_loc) const {
    bool skip = false;
    const bool indirect = table_loc.function == vvl::Func::vkCmdTraceRaysIndirectKHR;

    if (raygen_shader_binding_table.size != raygen_shader_binding_table.stride) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-size-04023" : "VUID-vkCmdTraceRaysKHR-size-04023";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::size), "(%" PRIu64 ") is not equal to stride (%" PRIu64 ").",
                         raygen_shader_binding_table.size, raygen_shader_binding_table.stride);
    }

    if (!IsPointerAligned(raygen_shader_binding_table.deviceAddress,
                          phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03682"
                                    : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03682";
        skip |=
            LogError(vuid, commandBuffer, table_loc.dot(Field::deviceAddress),
                     "(0x%" PRIx64
                     ") must be aligned to "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                     raygen_shader_binding_table.deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }

    return skip;
}

bool Device::ValidateTraceRaysMissShaderBindingTable(VkCommandBuffer commandBuffer,
                                                     const VkStridedDeviceAddressRegionKHR &miss_shader_binding_table,
                                                     const Location &table_loc) const {
    bool skip = false;
    const bool indirect = table_loc.function == vvl::Func::vkCmdTraceRaysIndirectKHR;
    auto &props = phys_dev_ext_props.ray_tracing_props_khr;

    if (!IsIntegerMultipleOf(miss_shader_binding_table.stride, props.shaderGroupHandleAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-03686" : "VUID-vkCmdTraceRaysKHR-stride-03686";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         miss_shader_binding_table.stride, props.shaderGroupHandleAlignment);
    }
    if (miss_shader_binding_table.stride > props.maxShaderGroupStride) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-04029" : "VUID-vkCmdTraceRaysKHR-stride-04029";
        skip |=
            LogError(vuid, commandBuffer, table_loc.dot(Field::stride),
                     "(%" PRIu64
                     ") must be "
                     "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                     miss_shader_binding_table.stride, props.maxShaderGroupStride);
    }
    if (!IsPointerAligned(miss_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03685"
                                    : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03685";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::deviceAddress),
                         "(0x%" PRIx64
                         ") must be aligned to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         miss_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment);
    }

    return skip;
}

bool Device::ValidateTraceRaysHitShaderBindingTable(VkCommandBuffer commandBuffer,
                                                    const VkStridedDeviceAddressRegionKHR &hit_shader_binding_table,
                                                    const Location &table_loc) const {
    bool skip = false;
    const bool indirect = table_loc.function == vvl::Func::vkCmdTraceRaysIndirectKHR;
    auto &props = phys_dev_ext_props.ray_tracing_props_khr;

    if (!IsIntegerMultipleOf(hit_shader_binding_table.stride, props.shaderGroupHandleAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-03690" : "VUID-vkCmdTraceRaysKHR-stride-03690";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         hit_shader_binding_table.stride, props.shaderGroupHandleAlignment);
    }
    if (hit_shader_binding_table.stride > props.maxShaderGroupStride) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-04035" : "VUID-vkCmdTraceRaysKHR-stride-04035";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::stride),
                         "(%" PRIu64
                         ") must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                         hit_shader_binding_table.stride, props.maxShaderGroupStride);
    }
    if (!IsPointerAligned(hit_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03689"
                                    : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03689";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::deviceAddress),
                         "(0x%" PRIx64
                         ") must be aligned to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         hit_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment);
    }

    return skip;
}

bool Device::ValidateTraceRaysCallableShaderBindingTable(VkCommandBuffer commandBuffer,
                                                         const VkStridedDeviceAddressRegionKHR &callable_shader_binding_table,
                                                         const Location &table_loc) const {
    bool skip = false;
    const bool indirect = table_loc.function == vvl::Func::vkCmdTraceRaysIndirectKHR;
    auto &props = phys_dev_ext_props.ray_tracing_props_khr;

    if (!IsIntegerMultipleOf(callable_shader_binding_table.stride, props.shaderGroupHandleAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-03694" : "VUID-vkCmdTraceRaysKHR-stride-03694";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         callable_shader_binding_table.stride, props.shaderGroupHandleAlignment);
    }

    if (callable_shader_binding_table.stride > props.maxShaderGroupStride) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-stride-04041" : "VUID-vkCmdTraceRaysKHR-stride-04041";
        skip |= LogError(
            vuid, commandBuffer, table_loc.dot(Field::stride),
            "(%" PRIu64
            ") must be less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
            callable_shader_binding_table.stride, props.maxShaderGroupStride);
    }

    if (!IsPointerAligned(callable_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment)) {
        const char *vuid = indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03693"
                                    : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03693";
        skip |= LogError(vuid, commandBuffer, table_loc.dot(Field::deviceAddress),
                         "(0x%" PRIx64
                         ") must be aligned to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         callable_shader_binding_table.deviceAddress, props.shaderGroupBaseAlignment);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                   uint32_t width, uint32_t height, uint32_t depth, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pRaygenShaderBindingTable) {
        skip |= ValidateTraceRaysRaygenShaderBindingTable(commandBuffer, *pRaygenShaderBindingTable,
                                                          error_obj.location.dot(Field::pRaygenShaderBindingTable));
    }
    if (pMissShaderBindingTable) {
        skip |= ValidateTraceRaysMissShaderBindingTable(commandBuffer, *pMissShaderBindingTable,
                                                        error_obj.location.dot(Field::pMissShaderBindingTable));
    }
    if (pHitShaderBindingTable) {
        skip |= ValidateTraceRaysHitShaderBindingTable(commandBuffer, *pHitShaderBindingTable,
                                                       error_obj.location.dot(Field::pHitShaderBindingTable));
    }
    if (pCallableShaderBindingTable) {
        skip |= ValidateTraceRaysCallableShaderBindingTable(commandBuffer, *pCallableShaderBindingTable,
                                                            error_obj.location.dot(Field::pCallableShaderBindingTable));
    }

    const uint64_t invocations = static_cast<uint64_t>(width) * static_cast<uint64_t>(depth) * static_cast<uint64_t>(height);
    if (invocations > phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-width-03641", commandBuffer, error_obj.location,
                         "width x height x depth (%" PRIu32 " x %" PRIu32 " x %" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount (%" PRIu32 ").",
                         width, depth, height, phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount);
    }

    const uint64_t max_width = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                               static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    if (width > max_width) {
        skip |= LogError(
            "VUID-vkCmdTraceRaysKHR-width-03638", commandBuffer, error_obj.location.dot(Field::width),
            "(%" PRIu32 ") must be less than or equal to maxComputeWorkGroupCount[0] x maxComputeWorkGroupSize[0] (%" PRIu32
            " x %" PRIu32 " = %" PRIu64 ").",
            width, phys_dev_props.limits.maxComputeWorkGroupCount[0], phys_dev_props.limits.maxComputeWorkGroupSize[0], max_width);
    }

    const uint64_t max_height = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    if (height > max_height) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-height-03639", commandBuffer, error_obj.location.dot(Field::height),
                         "(%" PRIu32
                         ") must be less than or equal to maxComputeWorkGroupCount[1] x maxComputeWorkGroupSize[1] (%" PRIu32
                         " x %" PRIu32 " = %" PRIu64 ").",
                         height, phys_dev_props.limits.maxComputeWorkGroupCount[1],
                         phys_dev_props.limits.maxComputeWorkGroupSize[1], max_height);
    }

    const uint64_t max_depth = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                               static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    if (depth > max_depth) {
        skip |= LogError(
            "VUID-vkCmdTraceRaysKHR-depth-03640", commandBuffer, error_obj.location.dot(Field::depth),
            "(%" PRIu32 ") must be less than or equal to maxComputeWorkGroupCount[2] x maxComputeWorkGroupSize[2] (%" PRIu32
            " x %" PRIu32 " = %" PRIu64 ").",
            depth, phys_dev_props.limits.maxComputeWorkGroupCount[2], phys_dev_props.limits.maxComputeWorkGroupSize[2], max_depth);
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                           VkDeviceAddress indirectDeviceAddress, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.rayTracingPipelineTraceRaysIndirect) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-rayTracingPipelineTraceRaysIndirect-03637", commandBuffer,
                         error_obj.location, "rayTracingPipelineTraceRaysIndirect feature must be enabled.");
    }

    if (pRaygenShaderBindingTable) {
        skip |= ValidateTraceRaysRaygenShaderBindingTable(commandBuffer, *pRaygenShaderBindingTable,
                                                          error_obj.location.dot(Field::pRaygenShaderBindingTable));
    }
    if (pMissShaderBindingTable) {
        skip |= ValidateTraceRaysMissShaderBindingTable(commandBuffer, *pMissShaderBindingTable,
                                                        error_obj.location.dot(Field::pMissShaderBindingTable));
    }
    if (pHitShaderBindingTable) {
        skip |= ValidateTraceRaysHitShaderBindingTable(commandBuffer, *pHitShaderBindingTable,
                                                       error_obj.location.dot(Field::pHitShaderBindingTable));
    }
    if (pCallableShaderBindingTable) {
        skip |= ValidateTraceRaysCallableShaderBindingTable(commandBuffer, *pCallableShaderBindingTable,
                                                            error_obj.location.dot(Field::pCallableShaderBindingTable));
    }

    if (!IsPointerAligned(indirectDeviceAddress, 4)) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03634", commandBuffer,
                         error_obj.location.dot(Field::indirectDeviceAddress), "(0x%" PRIx64 ") must be aligned to 4.",
                         indirectDeviceAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.rayTracingPipelineTraceRaysIndirect2) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637", commandBuffer,
                         error_obj.location, "rayTracingPipelineTraceRaysIndirect2 feature was not enabled.");
    }

    if (!IsPointerAligned(indirectDeviceAddress, 4)) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634", commandBuffer,
                         error_obj.location.dot(Field::indirectDeviceAddress), "(0x%" PRIx64 ") must be aligned to 4.",
                         indirectDeviceAddress);
    }
    return skip;
}

}  // namespace stateless
