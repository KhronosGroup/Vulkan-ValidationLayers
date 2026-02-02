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
#include "utils/math_utils.h"

bool CoreChecks::ValidateDataGraphPipelineBuiltinModelCreateInfoQCOM(const VkDataGraphPipelineBuiltinModelCreateInfoQCOM& dg_model_ci,
                                                                     const VkDataGraphProcessingEngineCreateInfoARM& engine_ci,
                                                                     const Location& loc) const {
    bool skip = false;
    bool has_identical_op = false;
    bool has_identical_paired_op_engine = false;
    VkPhysicalDeviceDataGraphProcessingEngineARM expected_engine{};

    for (size_t family_index = 0; family_index < physical_device_state->queue_family_properties.size(); ++family_index) {
        if ((physical_device_state->queue_family_properties[family_index].queueFlags & VK_QUEUE_DATA_GRAPH_BIT_ARM) == 0) {
            continue;
        }

        const auto data_graph_properties_arms =
                physical_device_state->GetQueueFamilyDataGraphPropsARM(static_cast<uint32_t>(family_index));

        for (size_t prop_index = 0; prop_index < data_graph_properties_arms.size(); ++prop_index) {
            const auto& retrieved_operation = data_graph_properties_arms[prop_index].operation;
            const auto& retrieved_engine = data_graph_properties_arms[prop_index].engine;

            ASSERT_AND_CONTINUE(dg_model_ci.pOperation);

            // Check whether the operations are identical or not
            if (!CompareDataGraphOperationSupportARM(*dg_model_ci.pOperation, retrieved_operation)) {
                continue;
            }

            has_identical_op = true;
            expected_engine = retrieved_engine;

            // Check whether the engines are identical or not
            for (uint32_t engine_index = 0; engine_index < engine_ci.processingEngineCount; ++engine_index) {
                const auto& processing_engine = engine_ci.pProcessingEngines[engine_index];
                if (processing_engine == retrieved_engine) {
                    has_identical_paired_op_engine = true;
                    break;
                }
            }
        }
    }

    if (!has_identical_paired_op_engine) {
        std::stringstream conditional_ss{};

        if (!has_identical_op) {
            if (!dg_model_ci.pOperation) {
                conditional_ss << "member pOperation is null.";
            } else {
                conditional_ss << "with input members operationType = "
                               << string_VkPhysicalDeviceDataGraphOperationTypeARM(dg_model_ci.pOperation->operationType)
                               << ", name = "
                               << dg_model_ci.pOperation->name
                               << ", version = "
                               << dg_model_ci.pOperation->version
                               << ", failed to find any identical operation in all retrieved "
                                  "VkQueueFamilyDataGraphPropertiesARM::operation results.";
            }
        } else {
            conditional_ss << ", found the same operation among the retrieved VkQueueFamilyDataGraphPropertiesARM::operation "
                              "results, but failed to found the same paired VkQueueFamilyDataGraphPropertiesARM::engine; "
                           << "expected VkPhysicalDeviceDataGraphProcessingEngineARM::type = "
                           << string_VkPhysicalDeviceDataGraphProcessingEngineTypeARM(expected_engine.type)
                           << ", expected VkPhysicalDeviceDataGraphProcessingEngineARM::isForeign = "
                           << expected_engine.isForeign
                           << ", please check VkDataGraphProcessingEngineCreateInfoARM structure in "
                              "VkDataGraphPipelineCreateInfoARM::pNext chain.";
        }

        skip |= LogError("VUID-VkDataGraphPipelineBuiltinModelCreateInfoQCOM-pOperation-11842",
                         device,
                         loc.dot(Field::pOperation),
                         conditional_ss.str().c_str());
    }

    return skip;
}

bool CoreChecks::ValidateDataGraphPipelineShaderModuleCreateInfo(VkDevice device,
                                                                 const VkDataGraphPipelineShaderModuleCreateInfoARM& dg_shader_ci,
                                                                 const Location& dg_shader_ci_loc,
                                                                 const vvl::Pipeline& pipeline) const {
    bool skip = false;

    if (auto module_state = Get<vvl::ShaderModule>(dg_shader_ci.module)) {
        if (!enabled_features.dataGraphSpecializationConstants) {
            if (dg_shader_ci.pSpecializationInfo) {
                skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-dataGraphSpecializationConstants-09849", device,
                                 dg_shader_ci_loc.dot(Field::pSpecializationInfo),
                                 "(%p) is not null but dataGraphSpecializationConstants feature is not enabled",
                                 dg_shader_ci.pSpecializationInfo);
            }
        }
    }

    for (uint32_t j = 0; j < dg_shader_ci.constantCount; j++) {
        skip |= ValidateTensorSemiStructuredSparsityInfo(device, dg_shader_ci.pConstants[j],
                                                         dg_shader_ci_loc.dot(Field::pConstants, j), pipeline);
    }

    return skip;
}

bool CoreChecks::ValidateDataGraphPipelineCreateInfo(VkDevice device, const VkDataGraphPipelineCreateInfoARM& create_info,
                                                     const Location& create_info_loc, const vvl::Pipeline& pipeline) const {
    bool skip = false;

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
    for (uint32_t j = 0; j < pipeline_layout->set_layouts.list.size(); j++) {
        auto dsl = pipeline_layout->set_layouts.list[j];
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

    return skip;
}

bool CoreChecks::ValidateTensorSemiStructuredSparsityInfo(VkDevice device, const VkDataGraphPipelineConstantARM& constant,
                                                          const Location& constant_loc, const vvl::Pipeline& pipeline) const {
    bool skip = false;

    const auto* sparsity =
        vku::FindStructInPNextChain<VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM>(constant.pNext);

    if (!sparsity) {
        return skip;
    }

    const auto* tensor_desc = vku::FindStructInPNextChain<VkTensorDescriptionARM>(constant.pNext);

    if (!tensor_desc) {
        skip |= LogError("VUID-VkDataGraphPipelineConstantARM-pNext-09775", device,
                         constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM),
                         "exists but the pNext chain doesn't include a VkTensorDescriptionARM.\n%s",
                         PrintPNextChain(Struct::VkDataGraphPipelineConstantARM, constant.pNext).c_str());
        return skip;
    }

    vvl::unordered_set<uint32_t> sparsity_dimensions;
    while (sparsity) {
        if (sparsity->dimension >= tensor_desc->dimensionCount) {
            skip |= LogError(
                "VUID-VkDataGraphPipelineConstantARM-pNext-09776", device,
                constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM, Field::dimension),
                "(%" PRIu32 ") must be less than the tensor rank (dimensionCount = %" PRIu32 ")", sparsity->dimension,
                tensor_desc->dimensionCount);
        } else if (!IsIntegerMultipleOf(tensor_desc->pDimensions[sparsity->dimension], sparsity->groupSize)) {
            skip |= LogError("VUID-VkDataGraphPipelineConstantARM-pNext-09777", device,
                             constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM,
                                                Field::pDimensions, sparsity->dimension),
                             "(%" PRIu64 ") must be a multiple of groupSize (%" PRIu32 ")",
                             tensor_desc->pDimensions[sparsity->dimension], sparsity->groupSize);
        }
        auto insert_ok = sparsity_dimensions.insert(sparsity->dimension).second;
        if (!insert_ok) {
            skip |= LogError("VUID-VkDataGraphPipelineConstantARM-pNext-09870", device,
                             constant_loc.pNext(Struct::VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM, Field::dimension),
                             "(%" PRIu32 ") already has a defined sparsity", sparsity->dimension);
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
    bool use_dg_pipeline_identifier = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const VkDataGraphPipelineCreateInfoARM& create_info = pCreateInfos[i];
        const vvl::Pipeline* pipeline = pipeline_states[i].get();
        ASSERT_AND_RETURN_SKIP(pipeline);
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);

        // three different ways to define a datagraph:
        const auto* dg_shader_ci = vku::FindStructInPNextChain<VkDataGraphPipelineShaderModuleCreateInfoARM>(create_info.pNext);
        const auto* dg_pipeline_identifier_ci =
            vku::FindStructInPNextChain<VkDataGraphPipelineIdentifierCreateInfoARM>(create_info.pNext);
        const auto* qcom_model_ci = vku::FindStructInPNextChain<VkDataGraphPipelineBuiltinModelCreateInfoQCOM>(create_info.pNext);

        // 1 and ONLY 1 of them MUST be present
        uint32_t defined_structs = (qcom_model_ci ? 1 : 0) + (dg_pipeline_identifier_ci ? 1 : 0) + (dg_shader_ci ? 1 : 0);
        if (defined_structs != 1) {
            skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-pNext-09763", device, create_info_loc,
                             "%" PRIu32 " of the possible required structures are included in the pNext chain%s.\n%s",
                             defined_structs, (defined_structs > 1) ? " (only 1 is allowed)" : "",
                             PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
            return skip;
        }

        if (dg_pipeline_identifier_ci || qcom_model_ci) {
            if (!(create_info.flags & VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)) {
                skip |= LogError(
                    "VUID-VkDataGraphPipelineCreateInfoARM-None-11840", device, create_info_loc.dot(Field::flags),
                    "(%s) does not include VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT, but the pNext chain include one "
                    "of VkDataGraphPipelineIdentifierCreateInfoARM or VkDataGraphPipelineBuiltinModelCreateInfoQCOM.\n%s",
                    string_VkPipelineCreateFlags2(create_info.flags).c_str(),
                    PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
            }
            if (create_info.pResourceInfos || create_info.resourceInfoCount) {
                skip |= LogError(
                    "VUID-VkDataGraphPipelineCreateInfoARM-None-11841", device, create_info_loc,
                    "pResourceInfos (%p) is not NULL, or resourceInfoCount (%" PRIu32 ") is not 0, but the pNext chain include one of VkDataGraphPipelineIdentifierCreateInfoARM or VkDataGraphPipelineBuiltinModelCreateInfoQCOM.\n%s",
                    create_info.pResourceInfos, create_info.resourceInfoCount,
                    PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
            }
        }

        if (dg_shader_ci) {
            // checks for datagraph defined via a shader module.

            const Location dg_shader_ci_loc = create_info_loc.pNext(Struct::VkDataGraphPipelineShaderModuleCreateInfoARM);
            if (!enabled_features.dataGraphShaderModule) {
                skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-dataGraphShaderModule-09886", device, dg_shader_ci_loc,
                                 "is in the pNext chain, but the dataGraphShaderModule feature is not enabled.\n%s",
                                 PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
            }

            // 2 possible ways to pass the shader, 1 and ONLY 1 MUST be present:
            // - VkShaderModuleCreateInfo (shader_module_ci), or
            // - VkDataGraphPipelineShaderModuleCreateInfoARM::module (dg_shader_ci->module)
            if (const auto* shader_module_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(create_info.pNext)) {
                if (dg_shader_ci->module) {
                    skip |= LogError("VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pNext-09873", device,
                                     dg_shader_ci_loc.dot(Field::module),
                                     "(%s) is not NULL but the pNext chain includes a VkShaderModuleCreateInfo.\n%s",
                                     FormatHandle(dg_shader_ci->module).c_str(),
                                     PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
                }
                skip |= ValidateShaderModuleCreateInfo(*shader_module_ci, create_info_loc.pNext(Struct::VkShaderModuleCreateInfo));
            } else if (!Get<vvl::ShaderModule>(dg_shader_ci->module)) {
                // there is no shader module: both dg_shader_ci->module and shader_module_ci are NULL
                skip |= LogError(
                    "VUID-VkDataGraphPipelineShaderModuleCreateInfoARM-pNext-09874", device, dg_shader_ci_loc.dot(Field::module),
                    "(%s) is not a valid VkShaderModule and the pNext chain doesn't include a VkShaderModuleCreateInfo.\n%s",
                    FormatHandle(dg_shader_ci->module).c_str(),
                    PrintPNextChain(Struct::VkDataGraphPipelineCreateInfoARM, create_info.pNext).c_str());
            }

            // remaining checks for shader module: create info and spirv
            skip |= ValidateDataGraphPipelineShaderModuleCreateInfo(device, *dg_shader_ci, dg_shader_ci_loc, *pipeline);
            skip |= ValidateDataGraphPipelineShaderModuleSpirv(device, create_info, create_info_loc, *dg_shader_ci, *pipeline);
        } else if (dg_pipeline_identifier_ci) {
            use_dg_pipeline_identifier = true;
        } else if (qcom_model_ci) {
            const Location built_in_model_loc = create_info_loc.pNext(Struct::VkDataGraphPipelineBuiltinModelCreateInfoQCOM);
            const auto* engine_ci = vku::FindStructInPNextChain<VkDataGraphProcessingEngineCreateInfoARM>(create_info.pNext);

            ASSERT_AND_CONTINUE(engine_ci);

            skip |= ValidateDataGraphPipelineBuiltinModelCreateInfoQCOM(*qcom_model_ci, *engine_ci, built_in_model_loc);
        }

        // common checks
        skip |= ValidateDataGraphPipelineCreateInfo(device, create_info, create_info_loc, *pipeline);
    }

    // Only need to validate the pipeline cache once even if multiple pipelines are being created
    if ((use_dg_pipeline_identifier) && (pipelineCache)) {
        skip |= ValidatePipelineCacheHeaderVersionDataGraphQCOM(pipelineCache, error_obj.location.dot(Field::pipelineCache));
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
                         error_obj.location.dot(Field::pPipelineInfo).dot(Field::dataGraphPipeline),
                         "was not created with vkCreateDataGraphPipelinesARM. The create info structure type was %s",
                         string_VkStructureType(pipeline_ptr->GetCreateInfoSType()));
    }
    for (uint32_t i = 0; i < propertiesCount; i++) {
        const VkDataGraphPipelinePropertyQueryResultARM& prop1 = pProperties[i];
        for (uint32_t j = i+1; j < propertiesCount; j++) {
            const VkDataGraphPipelinePropertyQueryResultARM& prop2 = pProperties[j];
            if (prop1.property == prop2.property) {
                skip |= LogError("VUID-vkGetDataGraphPipelinePropertiesARM-pProperties-09889", device, error_obj.location.dot(Field::pProperties, i).dot(Field::property),
                                "and pProperties[%" PRIu32 "].property are the same (%s).", j, string_VkDataGraphPipelinePropertyARM(prop1.property));
            }
        }
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

    // if the pipeline is NULL, skip is already false; otherwise, more checks
    if (!last_bound_state.pipeline_state) {
        return skip;
    }

    const VkPipeline cb_pipeline = last_bound_state.pipeline_state->VkHandle();
    if (session_state.create_info.dataGraphPipeline != cb_pipeline) {
        skip |= LogError("VUID-vkCmdDispatchDataGraphARM-dataGraphPipeline-09951", LogObjectList(), error_obj.location,
            "The pipeline bound to the command buffer (%s) is different from the pipeline bound to the session (%s); session %s.",
            FormatHandle(cb_pipeline).c_str(), FormatHandle(session_state.create_info.dataGraphPipeline).c_str(), FormatHandle(session_state).c_str());
    }

    return skip;
}
