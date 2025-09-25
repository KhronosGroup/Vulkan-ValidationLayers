/* Copyright (c) 2025 The Khronos Group Inc.
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
 *
 */

#include "core_validation.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/data_graph_pipeline_session_state.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"

bool CoreChecks::ValidateDataGraphPipelineCreateInfo(VkDevice device, const VkDataGraphPipelineCreateInfoARM& create_info,
                                                     const Location& create_info_loc, const vvl::Pipeline& pipeline) const {
    bool skip = false;
    const auto* dg_pipeline_shader_module_ci =
        vku::FindStructInPNextChain<VkDataGraphPipelineShaderModuleCreateInfoARM>(create_info.pNext);
    if (!dg_pipeline_shader_module_ci) {
        return skip;
    }

    const Location pipeline_shader_module_ci_loc = create_info_loc.pNext(Struct::VkDataGraphPipelineShaderModuleCreateInfoARM);
    auto module_state = Get<vvl::ShaderModule>(dg_pipeline_shader_module_ci->module);
    if (module_state) {
        if (!enabled_features.dataGraphSpecializationConstants) {
            if (dg_pipeline_shader_module_ci->pSpecializationInfo) {
                skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-dataGraphSpecializationConstants-09849", device,
                                 pipeline_shader_module_ci_loc.dot(Field::pSpecializationInfo),
                                 "(%p) is not null but dataGraphSpecializationConstants feature is not enabled",
                                 dg_pipeline_shader_module_ci->pSpecializationInfo);
            }
        }

        for (uint32_t j = 0; j < dg_pipeline_shader_module_ci->constantCount; j++) {
            const auto& constant = dg_pipeline_shader_module_ci->pConstants[j];
            if (const auto* sparsity =
                    vku::FindStructInPNextChain<VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM>(constant.pNext)) {
                const auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(constant.pNext);
                skip |= ValidateTensorSemiStructuredSparsityInfo(device, sparsity, tensor_desc,
                                                                 pipeline_shader_module_ci_loc.dot(Field::pConstants, j), pipeline);
            }
        }
    }

    if (const auto* shader_module_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(create_info.pNext)) {
        if (dg_pipeline_shader_module_ci->module != VK_NULL_HANDLE) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pNext-09873", device, create_info_loc.dot(Field::module),
                "(%p) is not NULL but the pNext chain of VkDataGraphPipelineCreateInfoARM includes a VkShaderModuleCreateInfo",
                reinterpret_cast<const void*>(dg_pipeline_shader_module_ci->module));
        }
        skip |= ValidateShaderModuleCreateInfo(*shader_module_ci, create_info_loc.pNext(Struct::VkShaderModuleCreateInfo));
    } else if (!module_state) {
        // we can't find the state object for dg_pipeline_shader_module_ci->module: it must be invalid: undefined or deleted
        skip |=
            LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pNext-09874", device, create_info_loc.dot(Field::module),
                     "(%p) is not a valid VkShaderModule and there is no VkShaderModuleCreateInfo in the pNext chain",
                     reinterpret_cast<const void*>(dg_pipeline_shader_module_ci->module));
    }

    if (const auto* pipeline_feedback = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext)) {
        if (pipeline_feedback->pipelineStageCreationFeedbackCount != 0) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineCreateInfoARM-pNext-09804", device,
                create_info_loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pipelineStageCreationFeedbackCount),
                "(%" PRIu32 ") must be zero", pipeline_feedback->pipelineStageCreationFeedbackCount);
        }
    }

    auto pipeline_layout = Get<vvl::PipelineLayout>(create_info.layout);
    ASSERT_AND_RETURN_SKIP(pipeline_layout);
    const Location layout_loc = create_info_loc.dot(Field::layout);

    if (!pipeline_layout->push_constant_ranges_layout->empty()) {
        skip |=
            LogError("VUID-VkDataGraphPipelineCreateInfoARM-layout-09767", device, layout_loc.dot(Field::pushConstantRangeCount),
                     "(%zu) must be zero", pipeline_layout->push_constant_ranges_layout->size());
    }
    for (uint32_t j = 0; j < pipeline_layout->set_layouts.size(); j++) {
        auto dsl = pipeline_layout->set_layouts[j];
        if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0) {
            if (!enabled_features.dataGraphUpdateAfterBind) {
                skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-dataGraphUpdateAfterBind-09768", device,
                                 layout_loc.dot(Field::pSetLayouts, j),
                                 "created with flags (%s) but the dataGraphUpdateAfterBind feature was not enabled",
                                 string_VkDescriptorSetLayoutCreateFlags(dsl->GetCreateFlags()).c_str());
            }
        }
        auto bindings = dsl->GetBindings();
        for (uint32_t k = 0; k < bindings.size(); k++) {
            auto binding = bindings[k];
            auto mutable_bindings = dsl->GetMutableTypes(binding.binding);
            if (!mutable_bindings.empty()) {
                skip |=
                    LogError("VUID-VkDataGraphPipelineCreateInfoARM-pSetLayouts-09770", device,
                             layout_loc.dot(Field::pSetLayouts, j), "includes binding(s) of type VK_DESCRIPTOR_TYPE_MUTABLE_EXT");
            }
        }
    }

    for (uint32_t j = 0; j < create_info.resourceInfoCount; j++) {
        auto resource = create_info.pResourceInfos[j];
        if (resource.arrayElement != 0) {
            skip |= LogError("VUID-VkDataGraphPipelineResourceInfoARM-arrayElement-09779", device,
                             create_info_loc.dot(Field::pResourceInfos, j).dot(Field::arrayElement), "(%" PRIu32 ") is not zero",
                             resource.arrayElement);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDataGraphPipelineCreateInfoFlags(VkPipelineCreateFlags2 flags, Location flags_loc) const {
    bool skip = false;

    constexpr VkPipelineCreateFlags2 valid_flag_mask =
        VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT |
        VK_PIPELINE_CREATE_2_DISABLE_OPTIMIZATION_BIT | VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT |
        VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT | VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT;

    if ((flags & ~(valid_flag_mask)) != 0) {
        skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-flags-09764", device, flags_loc, "(%s) contains invalid values.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if (!enabled_features.pipelineProtectedAccess) {
        if ((flags & VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT) != 0 ||
            (flags & VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT) != 0) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineCreateInfoARM-pipelineProtectedAccess-09772", device, flags_loc,
                "(%s) must not contain VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT or "
                "VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT because the pipelineProtectedAccess feature was not enabled.",
                string_VkPipelineCreateFlags2(flags).c_str());
        }
    } else {
        if ((flags & VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT) != 0 &&
            (flags & VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-flags-09773", device, flags_loc,
                             "(%s) must not contain both VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT and "
                             "VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    skip |= ValidatePipelineCacheControlFlags(flags, flags_loc,
                                              "VUID-VkDataGraphPipelineCreateInfoARM-pipelineCreationCacheControl-09871");

    return skip;
}

bool CoreChecks::ValidateTensorSemiStructuredSparsityInfo(
    VkDevice device, const VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM* sparsity,
    const VkTensorDescriptionARM* tensor_desc, const Location& constant_loc, const vvl::Pipeline& pipeline) const {
    bool skip = false;
    while (sparsity) {
        if (!tensor_desc) {
            skip |= LogError("VUID-VkDataGraphPipelineConstantARM-pNext-09775", device,
                             constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM),
                             "exists but no VkTensorDescriptionARM found in the chain");
        } else if (sparsity->dimension >= tensor_desc->dimensionCount) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineConstantARM-pNext-09776", device,
                constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM, Field::dimension),
                "(%" PRIu32 ") must be less than the tensor rank (dimensionCount = %" PRIu32 ")", sparsity->dimension,
                tensor_desc->dimensionCount);
        } else if (tensor_desc->pDimensions[sparsity->dimension] % sparsity->groupSize != 0) {
            skip |= LogError("VUID-VkDataGraphPipelineConstantARM-pNext-09777", device,
                             constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM,
                                                Field::pDimensions, sparsity->dimension),
                             "(%" PRIu64 ") must be a multiple of groupSize (%" PRIu32 ")",
                             tensor_desc->pDimensions[sparsity->dimension], sparsity->groupSize);
        }
        // We can have multiple Sparsity structures in the pNext chain.
        sparsity = vku::FindStructInPNextChain<VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM>(sparsity->pNext);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateDataGraphPipelinesARM(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkDataGraphPipelineCreateInfoARM* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                            chassis::CreateDataGraphPipelinesARM& chassis_state) const {
    bool skip = ValidateDeviceQueueSupport(error_obj.location);

    if (!enabled_features.dataGraph) {
        skip |= LogError("VUID-vkCreateDataGraphPipelinesARM-dataGraph-09760", device, error_obj.location,
                         "dataGraph feature is not enabled");
    }
    if (VK_NULL_HANDLE != deferredOperation) {
        skip |= LogError("VUID-vkCreateDataGraphPipelinesARM-deferredOperation-09761", device, error_obj.location,
                         "deferredOperation must be VK_NULL_HANDLE");
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const VkDataGraphPipelineCreateInfoARM& create_info = pCreateInfos[i];
        const vvl::Pipeline* pipeline = pipeline_states[i].get();
        ASSERT_AND_RETURN_SKIP(pipeline);
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);

        skip |= ValidateDataGraphPipelineCreateInfo(device, create_info, create_info_loc, *pipeline);
        skip |= ValidateDataGraphPipelineCreateInfoFlags(create_info.flags, create_info_loc.dot(Field::flags));
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDataGraphPipelinePropertiesARM(VkDevice device, const VkDataGraphPipelineInfoARM* pPipelineInfo,
                                                                  uint32_t propertiesCount,
                                                                  VkDataGraphPipelinePropertyQueryResultARM* pProperties,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    const auto pipeline_ptr = Get<vvl::Pipeline>(pPipelineInfo->dataGraphPipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_ptr);

    if (VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CREATE_INFO_ARM != pipeline_ptr->GetCreateInfoSType()) {
        skip |= LogError("VUID-VkDataGraphPipelineInfoARM-dataGraphPipeline-09803", device,
                         error_obj.location.dot(Field::dataGraphPipeline),
                         "was not created with vkCreateDataGraphPipelinesARM. The create info structure type was %s",
                         string_VkStructureType(pipeline_ptr->GetCreateInfoSType()));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDataGraphPipelineSessionARM(VkDevice device,
                                                                  const VkDataGraphPipelineSessionCreateInfoARM* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkDataGraphPipelineSessionARM* pSession,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    const auto pipeline_ptr = Get<vvl::Pipeline>(pCreateInfo->dataGraphPipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_ptr);

    if (VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CREATE_INFO_ARM != pipeline_ptr->GetCreateInfoSType()) {
        skip |= LogError("VUID-VkDataGraphPipelineSessionCreateInfoARM-dataGraphPipeline-09781", device,
                         error_obj.location.dot(Field::dataGraphPipeline),
                         "was not created with vkCreateDataGraphPipelinesARM. The create info structure type was %s",
                         string_VkStructureType(pipeline_ptr->GetCreateInfoSType()));
        return skip;  // no point continuing if the pipeline isn't valid
    }

    if ((pCreateInfo->flags & VK_DATA_GRAPH_PIPELINE_SESSION_CREATE_PROTECTED_BIT_ARM) != 0) {
        if (!enabled_features.protectedMemory) {
            skip |= LogError("VUID-VkDataGraphPipelineSessionCreateInfoARM-protectedMemory-09782", device,
                             error_obj.location.dot(Field::dataGraphPipeline).dot(Field::flags),
                             "(%s) contains VK_DATA_GRAPH_PIPELINE_SESSION_CREATE_PROTECTED_BIT_ARM but the protectedMemory "
                             "feature is not enabled",
                             string_VkDataGraphPipelineSessionCreateFlagsARM(pCreateInfo->flags).c_str());
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordGetDataGraphPipelineSessionBindPointRequirementsARM(
    VkDevice device, const VkDataGraphPipelineSessionBindPointRequirementsInfoARM* pInfo, uint32_t* pBindPointRequirementCount,
    VkDataGraphPipelineSessionBindPointRequirementARM* pBindPointRequirements, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    if (auto session_ptr = Get<vvl::DataGraphPipelineSession>(pInfo->session)){
        if (pBindPointRequirements) {
            session_ptr->InitMemoryRequirements(device, pBindPointRequirements, *pBindPointRequirementCount);
        }
    }
}

bool CoreChecks::PreCallValidateGetDataGraphPipelineSessionMemoryRequirementsARM(
    VkDevice device, const VkDataGraphPipelineSessionMemoryRequirementsInfoARM* pInfo, VkMemoryRequirements2* pMemoryRequirements,
    const ErrorObject& error_obj) const {
    bool skip = false;
    auto session_ptr = Get<vvl::DataGraphPipelineSession>(pInfo->session);
    ASSERT_AND_RETURN_SKIP(session_ptr);
    auto& session = *session_ptr;
    const Location pinfo_loc = error_obj.location.dot(Field::pInfo);
    if (auto bpr = session.FindBindPointRequirement(pInfo->bindPoint)) {
        if (VK_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_TYPE_MEMORY_ARM != bpr->bindPointType) {
            skip |= LogError("VUID-vkGetDataGraphPipelineSessionMemoryRequirementsARM-bindPoint-09784", device,
                             pinfo_loc.dot(Field::bindPoint),
                             "%s has type %s, expected VK_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_TYPE_MEMORY_ARM",
                             string_VkDataGraphPipelineSessionBindPointARM(pInfo->bindPoint),
                             string_VkDataGraphPipelineSessionBindPointTypeARM(bpr->bindPointType));
        }

        if (pInfo->objectIndex >= bpr->numObjects) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineSessionMemoryRequirementsInfoARM-objectIndex-09855", device,
                pinfo_loc.dot(Field::objectIndex),
                "(%" PRIu32
                ") must be less than the number of objects returned by vkGetDataGraphPipelineSessionBindPointRequirementsARM via "
                "VkDataGraphPipelineSessionBindPointRequirementARM::numObjects (%" PRIu32
                ") with VkDataGraphPipelineSessionMemoryRequirementsInfoARM::bindPoint equal to (%s)",
                pInfo->objectIndex, bpr->numObjects, string_VkDataGraphPipelineSessionBindPointARM(pInfo->bindPoint));
        }
    } else {
        skip |= LogError("VUID-vkGetDataGraphPipelineSessionMemoryRequirementsARM-bindPoint-09784", device,
                         pinfo_loc.dot(Field::bindPoint), "(%s) not found in session memory requirements",
                         string_VkDataGraphPipelineSessionBindPointARM(pInfo->bindPoint));
    }

    return skip;
}
bool CoreChecks::PreCallValidateDestroyDataGraphPipelineSessionARM(VkDevice device, VkDataGraphPipelineSessionARM session,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto session_state = Get<vvl::DataGraphPipelineSession>(session)) {
        skip |= ValidateObjectNotInUse(session_state.get(), error_obj.location,
                                       "VUID-vkDestroyDataGraphPipelineSessionARM-session-09793");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchDataGraphARM(VkCommandBuffer commandBuffer, VkDataGraphPipelineSessionARM session,
                                                        const VkDataGraphPipelineDispatchInfoARM *pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    const auto& cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundDataGraph();
    const vvl::DrawDispatchVuid &dispatch_vuid = GetDrawDispatchVuid(error_obj.location.function);
    skip |= ValidateActionState(last_bound_state, dispatch_vuid);

    const auto session_state_ptr = Get<vvl::DataGraphPipelineSession>(session);
    ASSERT_AND_RETURN_SKIP(session_state_ptr);
    const auto& session_state = *session_state_ptr;
    const auto& bound_memory_map = session_state.BoundMemoryMap();
    const LogObjectList& objlist = LogObjectList(commandBuffer, session);
    for (const auto& bpr : session_state.BindPointReqs()) {
        /* bpr: requirement; bound_memory_map: actually bound */
        size_t n_bound = bound_memory_map.find(bpr.bindPoint) == bound_memory_map.end() ? 0 : bound_memory_map.at(bpr.bindPoint).size();
        if (bpr.numObjects != n_bound) {
            skip |= LogError("VUID-vkCmdDispatchDataGraphARM-session-09796", objlist, error_obj.location,
                             "%zu objects bound at bind point %s, required numObjects %" PRIu32 "; session %s.", n_bound,
                             string_VkDataGraphPipelineSessionBindPointARM(bpr.bindPoint), bpr.numObjects,
                             FormatHandle(session_state).c_str());
        }
        if (n_bound > 0) {
            for (const auto& binding : bound_memory_map.at(bpr.bindPoint)) {
                skip |= VerifyBoundMemoryIsValid(binding.memory_state.get(), objlist, session_state.Handle(), error_obj.location,
                                                 "VUID-vkCmdDispatchDataGraphARM-session-09796");
            }
        }
    }

    return skip;
}
