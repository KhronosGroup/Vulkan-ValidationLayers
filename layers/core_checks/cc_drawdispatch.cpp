/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vk_enum_string_helper.h>
#include "core_checks/cc_buffer_address.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "core_validation.h"
#include "error_message/error_strings.h"
#include "error_message/logging.h"
#include "generated/error_location_helper.h"
#include "generated/spirv_grammar_helper.h"
#include "generated/vk_extension_helper.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/last_bound_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/pipeline_state.h"
#include "utils/math_utils.h"
#include "utils/image_utils.h"

using vvl::DrawDispatchVuid;
using vvl::GetDrawDispatchVuid;

bool CoreChecks::ValidateGraphicsIndexedCmd(const vvl::CommandBuffer &cb_state, const vvl::Buffer *index_buffer_state,
                                            const DrawDispatchVuid &vuid) const {
    bool skip = false;
    // maintenance6 allows null buffers to be bound
    if (!index_buffer_state && !cb_state.index_buffer_binding.bound) {
        const char *extra =
            enabled_features.maintenance6
                ? "Even with maintenance6, you need to set the buffer in vkCmdBindIndexBuffer to be VK_NULL_HANDLE, not "
                  "calling vkCmdBindIndexBuffer still has the buffer as undeclared."
                : "With maintenance6, you are allowed to set the buffer in vkCmdBindIndexBuffer to be VK_NULL_HANDLE.";
        skip |= LogError(
            vuid.index_binding_07312, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
            "no vkCmdBindIndexBuffer call has bound an index buffer to this command buffer prior to this indexed draw. %s", extra);
    } else if (index_buffer_state) {
        skip |= ValidateProtectedBuffer(cb_state, *index_buffer_state, vuid.loc(), vuid.unprotected_command_buffer_02707,
                                        " (Buffer is the index buffer)");
    }

    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(const LastBound &last_bound_state, uint32_t instanceCount, uint32_t firstInstance,
                                         const DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const auto *pipeline_state = last_bound_state.pipeline_state;

    // Verify maxMultiviewInstanceIndex
    if (cb_state.active_render_pass && cb_state.active_render_pass->has_multiview_enabled &&
        ((static_cast<uint64_t>(instanceCount) + static_cast<uint64_t>(firstInstance)) >
         static_cast<uint64_t>(phys_dev_props_core11.maxMultiviewInstanceIndex))) {
        skip |=
            LogError(vuid.max_multiview_instance_index_02688, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                     "render pass instance has multiview enabled, and maxMultiviewInstanceIndex: %" PRIu32
                     ", but instanceCount: %" PRIu32 " and firstInstance: %" PRIu32 ".",
                     phys_dev_props_core11.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }

    // supportsNonZeroFirstInstance was added from the EXT to KHR (not 1.4) version of VK_KHR_vertex_attribute_divisor
    // If not using the KHR or 1.4 version, we don't check for these VUs
    if (IsExtEnabled(extensions.vk_khr_vertex_attribute_divisor)) {
        if (pipeline_state && pipeline_state->GraphicsCreateInfo().pVertexInputState) {
            const auto *vertex_input_divisor_state = vku::FindStructInPNextChain<VkPipelineVertexInputDivisorStateCreateInfo>(
                pipeline_state->GraphicsCreateInfo().pVertexInputState->pNext);
            if (vertex_input_divisor_state && phys_dev_props_core14.supportsNonZeroFirstInstance == VK_FALSE &&
                firstInstance != 0u) {
                for (uint32_t i = 0; i < vertex_input_divisor_state->vertexBindingDivisorCount; ++i) {
                    if (vertex_input_divisor_state->pVertexBindingDivisors[i].divisor != 1u) {
                        skip |= LogError(
                            vuid.vertex_input_09461, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                            "VkPipelineVertexInputDivisorStateCreateInfo::pVertexBindingDivisors[%" PRIu32 "].divisor is %" PRIu32
                            " and firstInstance is %" PRIu32 ", but supportsNonZeroFirstInstance is VK_FALSE.",
                            i, vertex_input_divisor_state->pVertexBindingDivisors[i].divisor, firstInstance);
                        break;  // only report first instance of the error
                    }
                }
            }
        }

        if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
                phys_dev_props_core14.supportsNonZeroFirstInstance == VK_FALSE && firstInstance != 0u) {
                for (const auto &binding_state : cb_state.dynamic_state_value.vertex_bindings) {
                    const auto &desc = binding_state.second.desc;
                    if (desc.divisor != 1u) {
                        skip |=
                            LogError(vuid.vertex_input_09462, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                                     "vkCmdSetVertexInputEXT set pVertexBindingDivisors[%" PRIu32 "] (binding %" PRIu32
                                     ") divisor as %" PRIu32 ", but firstInstance is %" PRIu32
                                     " and supportsNonZeroFirstInstance is VK_FALSE.",
                                     binding_state.second.index, desc.binding, desc.divisor, firstInstance);
                        break;
                    }
                }
            }
        }
    }

    return skip;
}

// VTG = Vertex Tessellation Geometry
bool CoreChecks::ValidateVTGShaderStages(const LastBound &last_bound_state, const DrawDispatchVuid &vuid) const {
    bool skip = false;

    if (last_bound_state.pipeline_state &&
        last_bound_state.pipeline_state->active_shaders & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(
            vuid.invalid_mesh_shader_stages_06481, last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            vuid.loc(),
            "The bound graphics pipeline must not have been created with "
            "VK_SHADER_STAGE_TASK_BIT_EXT or VK_SHADER_STAGE_MESH_BIT_EXT. Active shader stages on the bound pipeline are %s.",
            string_VkShaderStageFlags(last_bound_state.pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMeshShaderStage(const LastBound &last_bound_state, const DrawDispatchVuid &vuid, bool is_NV) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const auto *pipeline_state = last_bound_state.pipeline_state;

    if (pipeline_state && !(pipeline_state->active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(vuid.missing_mesh_shader_stages_07080, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The current pipeline bound to VK_PIPELINE_BIND_POINT_GRAPHICS must contain a shader stage using the "
                         "%s Execution Model. Active shader stages on the bound pipeline are %s.",
                         is_NV ? "MeshNV" : "MeshEXT", string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    if (pipeline_state &&
        (pipeline_state->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))) {
        skip |= LogError(vuid.mesh_shader_stages_06480, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The bound graphics pipeline must not have been created with "
                         "VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT or VK_SHADER_STAGE_GEOMETRY_BIT. Active shader stages on the "
                         "bound pipeline are %s.",
                         string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    for (const auto &query : cb_state.active_queries) {
        const auto query_pool_state = Get<vvl::QueryPool>(query.pool);
        if (!query_pool_state) continue;
        if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
            skip |= LogError(vuid.xfb_queries_07074, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                             "Query with type %s is active.", string_VkQueryType(query_pool_state->create_info.queryType));
        }
        if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
            skip |= LogError(vuid.pg_queries_07075, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                             "Query with type %s is active.", string_VkQueryType(query_pool_state->create_info.queryType));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdDrawInstance(last_bound_state, instanceCount, firstInstance, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdDrawInstance(last_bound_state, instanceCount, firstInstance, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-None-04933", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "The multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |=
            LogError("VUID-vkCmdDrawMultiEXT-drawCount-04934", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::drawCount), "(%" PRIu32 ") must be less than maxMultiDrawCount (%" PRIu32 ").",
                     drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMultiEXT-drawCount-09628", stride,
                                                Struct::VkMultiDrawInfoEXT, sizeof(VkMultiDrawInfoEXT), error_obj.location);
    }
    if (drawCount != 0 && !pVertexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-drawCount-04935", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pVertexInfo is NULL.", drawCount);
    }

    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(const vvl::CommandBuffer &cb_state, const vvl::Buffer &index_buffer_state,
                                                  uint32_t indexCount, uint32_t firstIndex, const Location &loc,
                                                  const char *first_index_vuid) const {
    bool skip = false;
    if (enabled_features.robustBufferAccess2) {
        return skip;
    }

    const uint32_t index_size = GetIndexAlignment(cb_state.index_buffer_binding.index_type);
    // This doesn't exactly match the pseudocode of the VUID, but the binding size is the *bound* size, such that the offset
    // has already been accounted for (subtracted from the buffer size), and is consistent with the use of
    // BufferBinding::size for vertex buffer bindings (which record the *bound* size, not the size of the bound buffer)
    VkDeviceSize end_offset = static_cast<VkDeviceSize>(index_size * (firstIndex + indexCount));
    if (end_offset > cb_state.index_buffer_binding.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(index_buffer_state.Handle());
        skip |= LogError(first_index_vuid, objlist, loc,
                         "index size (%" PRIu32 ") * (firstIndex (%" PRIu32 ") + indexCount (%" PRIu32
                         ")) "
                         "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                         " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                         index_size, firstIndex, indexCount, cb_state.index_buffer_binding.offset,
                         end_offset + cb_state.index_buffer_binding.offset,
                         cb_state.index_buffer_binding.size + cb_state.index_buffer_binding.offset);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdDrawInstance(last_bound_state, instanceCount, firstInstance, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    {
        const auto index_buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
        skip |= ValidateGraphicsIndexedCmd(cb_state, index_buffer_state.get(), vuid);
        if (index_buffer_state) {
            skip |= ValidateCmdDrawIndexedBufferSize(cb_state, *index_buffer_state, indexCount, firstIndex, error_obj.location,
                                                     "VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdDrawInstance(last_bound_state, instanceCount, firstInstance, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-None-04937", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be less than VkPhysicalDeviceMultiDrawPropertiesEXT::maxMultiDrawCount (%" PRIu32 ").",
                         drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }

    bool invalid_stride = false;
    if (drawCount > 1) {
        invalid_stride = ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-09629", stride,
                                                         Struct::VkMultiDrawIndexedInfoEXT, sizeof(VkMultiDrawIndexedInfoEXT),
                                                         error_obj.location);
    }
    skip |= invalid_stride;

    const auto index_buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
    skip |= ValidateGraphicsIndexedCmd(cb_state, index_buffer_state.get(), vuid);

    // only index into pIndexInfo if we know parameters are sane
    if (drawCount != 0 && !pIndexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pIndexInfo is NULL.", drawCount);
    } else if (index_buffer_state) {
        // Continuing on from this point invokes undefined behavior due to invalid stride size.
        if (invalid_stride) {
            return skip;
        }

        const auto info_bytes = reinterpret_cast<const char *>(pIndexInfo);
        for (uint32_t i = 0; i < drawCount; i++) {
            const auto info_ptr = reinterpret_cast<const VkMultiDrawIndexedInfoEXT *>(info_bytes + i * stride);
            skip |= ValidateCmdDrawIndexedBufferSize(cb_state, *index_buffer_state, info_ptr->indexCount, info_ptr->firstIndex,
                                                     error_obj.location.dot(Field::pIndexInfo, i),
                                                     "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);

        if (drawCount > 1) {
            skip |=
                ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
            skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                                    Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), drawCount, offset,
                                                    *indirect_buffer_state, error_obj.location);
        } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndirectCommand)) > indirect_buffer_state->create_info.size) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(buffer);
            skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-00487", objlist, error_obj.location.dot(Field::drawCount),
                             "is 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64
                             ") is not less than "
                             "or equal to the size of buffer (%" PRIu64 ").",
                             (offset + sizeof(VkDrawIndirectCommand)), indirect_buffer_state->create_info.size);
        }
    }

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    {
        const auto index_buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
        skip |= ValidateGraphicsIndexedCmd(cb_state, index_buffer_state.get(), vuid);
    }

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);

        if (drawCount > 1) {
            skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                    Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                    error_obj.location);
            skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                    Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                    drawCount, offset, *indirect_buffer_state, error_obj.location);
        } else if (offset & 3) {
            skip |= LogError("VUID-vkCmdDrawIndexedIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                             error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
        } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndexedIndirectCommand)) > indirect_buffer_state->create_info.size) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(buffer);
            skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-00539", objlist, error_obj.location.dot(Field::drawCount),
                             "is 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                             ") is not less than "
                             "or equal to the size of buffer (%" PRIu64 ").",
                             (offset + sizeof(VkDrawIndexedIndirectCommand)), indirect_buffer_state->create_info.size);
        }
    }

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundCompute();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);

    if (groupCountX > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountX-00386", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountX),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", groupCountX,
                         phys_dev_props.limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountY-00387", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountY),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", groupCountY,
                         phys_dev_props.limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountZ-00388", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountZ),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", groupCountZ,
                         phys_dev_props.limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundCompute();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00421", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountX-00424", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") + groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                     baseGroupX, groupCountX, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00422", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountY-00425", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") + groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                     baseGroupY, groupCountY, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupZ-00423", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountZ-00426", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") + groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                     baseGroupZ, groupCountZ, limit);
    }

    if (baseGroupX || baseGroupY || baseGroupZ) {
        if (last_bound_state.pipeline_state) {
            if (!(last_bound_state.pipeline_state->create_flags & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
                skip |= LogError("VUID-vkCmdDispatchBase-baseGroupX-00427", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                                 error_obj.location,
                                 "If any of baseGroupX (%" PRIu32 "), baseGroupY (%" PRIu32 "), or baseGroupZ (%" PRIu32
                                 ") are not zero, then the bound compute pipeline "
                                 "must have been created with the VK_PIPELINE_CREATE_DISPATCH_BASE flag",
                                 baseGroupX, baseGroupY, baseGroupZ);
            }
        } else {
            const auto *shader_object = last_bound_state.GetShaderState(ShaderObjectStage::COMPUTE);
            if (shader_object && ((shader_object->create_info.flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) == 0)) {
                skip |= LogError("VUID-vkCmdDispatchBase-baseGroupX-00427", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                                 error_obj.location,
                                 "If any of baseGroupX (%" PRIu32 "), baseGroupY (%" PRIu32 "), or baseGroupZ (%" PRIu32
                                 ") are not zero, then the bound compute shader object "
                                 "must have been created with the VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT flag",
                                 baseGroupX, baseGroupY, baseGroupZ);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                          error_obj);
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundCompute();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);
        if (offset & 3) {
            skip |= LogError("VUID-vkCmdDispatchIndirect-offset-02710", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                             error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
        }
        if ((offset + sizeof(VkDispatchIndirectCommand)) > indirect_buffer_state->create_info.size) {
            skip |= LogError("VUID-vkCmdDispatchIndirect-offset-00407", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                             error_obj.location,
                             "The (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                             ")  is greater than the "
                             "size of the buffer (%" PRIu64 ").",
                             offset + sizeof(VkDispatchIndirectCommand), indirect_buffer_state->create_info.size);
        }
    }

    return skip;
}
bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }

    if (countBufferOffset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawIndirectCount-countBufferOffset-02716", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::countBufferOffset), "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }

    {
        auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
        ASSERT_AND_RETURN_SKIP(count_buffer_state);
        skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, vuid);
    }

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirectCount-stride-03110", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
        if (maxDrawCount > 1) {
            skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride,
                                                    Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), maxDrawCount,
                                                    offset, *indirect_buffer_state, error_obj.location);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }
    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }

    {
        auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
        ASSERT_AND_RETURN_SKIP(count_buffer_state);
        skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, vuid);
    }

    {
        const auto index_buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
        skip |= ValidateGraphicsIndexedCmd(cb_state, index_buffer_state.get(), vuid);
    }

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        if (maxDrawCount > 1) {
            skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                    Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                    maxDrawCount, offset, *indirect_buffer_state, error_obj.location);
        }
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdDrawInstance(last_bound_state, instanceCount, firstInstance, vuid);
    skip |= ValidateVTGShaderStages(last_bound_state, vuid);

    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "transformFeedback feature is not enabled.");
    }
    if (IsExtEnabled(extensions.vk_ext_transform_feedback) && !phys_dev_ext_props.transform_feedback_props.transformFeedbackDraw) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackDraw is not supported");
    }
    if ((vertexStride <= 0) || (vertexStride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride)) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::vertexStride),
                         "(%" PRIu32 ") must be between 0 and maxTransformFeedbackBufferDataStride (%" PRIu32 ").", vertexStride,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    if (SafeModulo(counterBufferOffset, 4) != 0) {
        skip |= LogError(
            "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::counterBufferOffset), "(%" PRIu64 ") must be a multiple of 4.", counterBufferOffset);
    }
    // VUs being added in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6310
    if (SafeModulo(counterOffset, 4) != 0) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-counterOffset-09474",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::counterOffset),
                         "(%" PRIu32 ") must be a multiple of 4.", counterOffset);
    }
    if (SafeModulo(vertexStride, 4) != 0) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-09475",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::vertexStride),
                         "(%" PRIu32 ") must be a multiple of 4.", vertexStride);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundRayTracing();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);

    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (width > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-width-02469", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::width),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0].");
    }
    if (height > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-height-02470", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::height),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1].");
    }
    if (depth > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-depth-02471", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::depth),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2].");
    }

    auto callable_shader_buffer_state = Get<vvl::Buffer>(callableShaderBindingTableBuffer);
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(callableShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461", objlist,
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->create_info.size);
    }
    auto hit_shader_buffer_state = Get<vvl::Buffer>(hitShaderBindingTableBuffer);
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(hitShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459", objlist,
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->create_info.size);
    }
    auto miss_shader_buffer_state = Get<vvl::Buffer>(missShaderBindingTableBuffer);
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(missShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457", objlist,
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->create_info.size);
    }
    auto raygen_shader_buffer_state = Get<vvl::Buffer>(raygenShaderBindingTableBuffer);
    if (raygenShaderBindingOffset >= raygen_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(raygenShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02455", objlist,
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of raygenShaderBindingTableBuffer %" PRIu64 " .",
                         raygenShaderBindingOffset, raygen_shader_buffer_state->create_info.size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysKHR(const Location &loc, const LastBound &last_bound_state,
                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable) const {
    bool skip = false;
    const vvl::Pipeline *pipeline_state = last_bound_state.pipeline_state;
    if (!pipeline_state) return skip;  // possible wasn't bound correctly, check caught elsewhere
    const bool is_indirect = loc.function == Func::vkCmdTraceRaysIndirectKHR;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    if (pHitShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pHitShaderBindingTable);
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03514" : "VUID-vkCmdTraceRaysKHR-flags-03514";
                skip |= LogError(vuid, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03513" : "VUID-vkCmdTraceRaysKHR-flags-03513";
                skip |= LogError(vuid, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            // No vuid to check for pHitShaderBindingTable->deviceAddress == 0 with this flag

            if (pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03512" : "VUID-vkCmdTraceRaysKHR-flags-03512";
                skip |= LogError(vuid, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }

        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03688"
                                                          : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688";
        skip |= ValidateRaytracingShaderBindingTable(cb_state, table_loc, vuid_binding_table_flag, *pHitShaderBindingTable);
    }

    if (pRaygenShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pRaygenShaderBindingTable);
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03681"
                                                          : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681";
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9368
        // TODO - waiting for https://gitlab.khronos.org/vulkan/vulkan/-/issues/4173
        if (const auto buffers = GetBuffersByAddress(pRaygenShaderBindingTable->deviceAddress); buffers.empty()) {
            skip |= LogError("UNASSIGNED-TraceRays-InvalidRayGenSBTAddress",
                             cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), table_loc.dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") does not belong to a valid VkBuffer.", pRaygenShaderBindingTable->deviceAddress);
        }
        skip |= ValidateRaytracingShaderBindingTable(cb_state, table_loc, vuid_binding_table_flag, *pRaygenShaderBindingTable);
    }

    if (pMissShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pMissShaderBindingTable);
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03684"
                                                          : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684";
        skip |= ValidateRaytracingShaderBindingTable(cb_state, table_loc, vuid_binding_table_flag, *pMissShaderBindingTable);
    }

    if (pCallableShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pCallableShaderBindingTable);
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03692"
                                                          : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692";
        skip |= ValidateRaytracingShaderBindingTable(cb_state, table_loc, vuid_binding_table_flag, *pCallableShaderBindingTable);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundRayTracing();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, last_bound_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysIndirect(const Location &loc, const LastBound &last_bound_state,
                                              VkDeviceAddress indirect_device_address) const {
    bool skip = false;
    const bool is_2khr = loc.function == Func::vkCmdTraceRaysIndirect2KHR;

    const char *usage_vuid = is_2khr ? " VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03633"
                                     : "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03633";
    BufferAddressValidation<1> buffer_address_validator = {
        {{{usage_vuid,
           [](const vvl::Buffer &buffer_state) { return (buffer_state.usage & VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT) == 0; },
           []() { return "The following buffers are missing VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT"; }, kUsageErrorMsgBuffer}}}};

    skip |= buffer_address_validator.ValidateDeviceAddress(
        *this, loc.dot(Field::indirectDeviceAddress), LogObjectList(last_bound_state.cb_state.Handle()), indirect_device_address);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundRayTracing();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, last_bound_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    skip |= ValidateCmdTraceRaysIndirect(error_obj.location, last_bound_state, indirectDeviceAddress);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundRayTracing();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateCmdTraceRaysIndirect(error_obj.location, last_bound_state, indirectDeviceAddress);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, true);

    if (taskCount > phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksNV-taskCount-02119", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::taskCount),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, true);

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);

        if (drawCount > 1) {
            skip |= ValidateCmdDrawStrideWithBuffer(
                cb_state, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride, Struct::VkDrawMeshTasksIndirectCommandNV,
                sizeof(VkDrawMeshTasksIndirectCommandNV), drawCount, offset, *indirect_buffer_state, error_obj.location);
            if (!enabled_features.multiDrawIndirect) {
                skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
                                 cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                                 "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
            }
            if ((stride & 3) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV)) {
                skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146",
                                 cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::stride),
                                 "(0x%" PRIxLEAST32
                                 "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).",
                                 stride);
            }
        } else if (drawCount == 1 &&
                   ((offset + sizeof(VkDrawMeshTasksIndirectCommandNV)) > indirect_buffer_state.get()->create_info.size)) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(buffer);
            skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156", objlist, error_obj.location,
                             "(offset + sizeof(VkDrawMeshTasksIndirectNV)) (%" PRIu64
                             ") is greater than the size of buffer (%" PRIu64 ").",
                             offset + sizeof(VkDrawMeshTasksIndirectCommandNV), indirect_buffer_state->create_info.size);
        }
    }

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, true);

    if (offset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    {
        auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
        ASSERT_AND_RETURN_SKIP(count_buffer_state);
        skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, vuid);
    }

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                error_obj.location);

        if (maxDrawCount > 1) {
            skip |=
                ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, *indirect_buffer_state, error_obj.location);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, false);

    if (groupCountX > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountX),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[0] (0x%" PRIxLEAST32
            ").",
            groupCountX, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]);
    }
    if (groupCountY > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountY),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[1] (0x%" PRIxLEAST32
            ").",
            groupCountY, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]);
    }
    if (groupCountZ > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountZ),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[2] (0x%" PRIxLEAST32
            ").",
            groupCountZ, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]);
    }

    uint32_t maxTaskWorkGroupTotalCount = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    uint64_t invocations = static_cast<uint64_t>(groupCountX) * static_cast<uint64_t>(groupCountY);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::kU32Max || invocations > maxTaskWorkGroupTotalCount) {
        fail = true;
    }
    if (!fail) {
        invocations *= static_cast<uint64_t>(groupCountZ);
        if (invocations > vvl::kU32Max || invocations > maxTaskWorkGroupTotalCount) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
            "The product of groupCountX (0x%" PRIxLEAST32 "), groupCountY (0x%" PRIxLEAST32 ") and groupCountZ (0x%" PRIxLEAST32
            ") must be less than or equal to "
            "VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (0x%" PRIxLEAST32 ").",
            groupCountX, groupCountY, groupCountZ, maxTaskWorkGroupTotalCount);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, false);

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);
        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);

        if (drawCount > 1) {
            skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088", stride,
                                                    Struct::VkDrawMeshTasksIndirectCommandEXT,
                                                    sizeof(VkDrawMeshTasksIndirectCommandEXT), error_obj.location);
            skip |= ValidateCmdDrawStrideWithBuffer(
                cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090", stride, Struct::VkDrawMeshTasksIndirectCommandEXT,
                sizeof(VkDrawMeshTasksIndirectCommandEXT), drawCount, offset, *indirect_buffer_state, error_obj.location);
        }
        if ((drawCount == 1) && (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)) > indirect_buffer_state->create_info.size) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(buffer);
            skip |=
                LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(vkCmdDrawMeshTasksIndirectEXT)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)), indirect_buffer_state->create_info.size);
        }
    }

    // TODO: vkMapMemory() and check the contents of buffer at offset
    // issue #4547 (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4547)
    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    const auto &last_bound_state = cb_state.GetLastBoundGraphics();
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    skip |= ValidateActionState(last_bound_state, vuid);
    skip |= ValidateMeshShaderStage(last_bound_state, vuid, false);

    auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
    ASSERT_AND_RETURN_SKIP(count_buffer_state);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *count_buffer_state, error_obj.location.dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, countBuffer), *count_buffer_state,
                                     VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT, true, vuid.indirect_count_buffer_bit_02715,
                                     error_obj.location.dot(Field::countBuffer));

    {
        auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
        ASSERT_AND_RETURN_SKIP(indirect_buffer_state);

        skip |= ValidateIndirectCmd(cb_state, *indirect_buffer_state, vuid);
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandEXT,
                                                sizeof(VkDrawMeshTasksIndirectCommandEXT), error_obj.location);
        if (maxDrawCount > 1) {
            skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097", stride,
                                                    Struct::VkDrawMeshTasksIndirectCommandEXT,
                                                    sizeof(VkDrawMeshTasksIndirectCommandEXT), maxDrawCount, offset,
                                                    *indirect_buffer_state, error_obj.location);
        }
    }

    return skip;
}

// Action command == vkCmdDraw*, vkCmdDispatch*, vkCmdTraceRays*
// This is the main logic shared by all action commands
bool CoreChecks::ValidateActionState(const LastBound &last_bound_state, const DrawDispatchVuid &vuid) const {
    const Location &loc = vuid.loc();
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;
    const VkPipelineBindPoint bind_point = last_bound_state.bind_point;

    bool skip = false;
    skip |= ValidateCmd(cb_state, loc);

    // Quick verify that if there is no pipeline, the shader object is being used
    if (!pipeline && !enabled_features.shaderObject) {
        return LogError(vuid.pipeline_bound_08606, cb_state.GetObjectList(bind_point), loc,
                        "A valid %s pipeline must be bound with vkCmdBindPipeline before calling this command.",
                        string_VkPipelineBindPoint(bind_point));
    }

    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        skip |= ValidateDrawDynamicState(last_bound_state, vuid);
        skip |= ValidateDrawPrimitivesGeneratedQuery(last_bound_state, vuid);
        skip |= ValidateDrawProtectedMemory(last_bound_state, vuid);
        skip |= ValidateDrawFragmentShadingRate(last_bound_state, vuid);
        skip |= ValidateDrawAttachmentColorBlend(last_bound_state, vuid);
        skip |= ValidateDrawAttachmentSampleLocation(last_bound_state, vuid);
        skip |= ValidateDrawDepthStencilAttachments(last_bound_state, vuid);
        skip |= ValidateDrawTessellation(last_bound_state, vuid);
        skip |= ValidateDrawVertexBinding(last_bound_state, vuid);

        if (cb_state.active_render_pass && cb_state.active_render_pass->UsesDynamicRendering()) {
            skip |= ValidateDrawDynamicRenderingFsOutputs(last_bound_state, cb_state, loc);
            skip |= ValidateDrawDynamicRenderpassExternalFormatResolve(last_bound_state, *cb_state.active_render_pass, vuid);
        }

        if (pipeline) {
            skip |= ValidateDrawPipeline(last_bound_state, *pipeline, vuid);
        } else {
            skip |= ValidateDrawShaderObject(last_bound_state, vuid);
        }

    } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        skip |= InsideRenderPass(cb_state, loc, vuid.compute_inside_rp_10672);

        if (!pipeline && !last_bound_state.IsValidShaderBound(ShaderObjectStage::COMPUTE)) {
            const bool is_null_bound = last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::COMPUTE);
            return LogError(
                vuid.compute_not_bound_10743, cb_state.GetObjectList(bind_point), loc,
                "No compute shader is bound, before this dispatch command, you either need to call vkCmdBindPipeline with a valid "
                "compute pipeline or vkCmdBindShadersEXT with a valid compute shader object.%s",
                is_null_bound ? " (vkCmdBindShadersEXT was called, but it set the compute stage to VK_NULL_HANDLE)" : "");
        }
    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        if (pipeline) {
            skip |= ValidateTraceRaysDynamicStateSetStatus(last_bound_state, *pipeline, vuid);
        }
        if (!cb_state.unprotected) {
            skip |= LogError(vuid.ray_query_protected_cb_03635, cb_state.GetObjectList(bind_point), loc,
                             "called in a protected command buffer.");
        }
    }

    if (pipeline) {
        skip |= ValidateActionStateDescriptorsPipeline(last_bound_state, bind_point, *pipeline, vuid);
    } else if (last_bound_state.cb_state.descriptor_buffer.binding_info.empty()) {
        // TODO - VkPipeline have VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT (descriptor_buffer_mode) to know if using descriptor
        // buffers, but VK_EXT_shader_object has no flag. For now, if the command buffer ever calls vkCmdBindDescriptorBuffersEXT,
        // we just assume things are bound until we add some form of GPU side tracking for descriptor buffers
        skip |= ValidateActionStateDescriptorsShaderObject(last_bound_state, bind_point, vuid);
    }

    skip |= ValidateActionStatePushConstant(last_bound_state, pipeline, vuid);

    if (!cb_state.unprotected) {
        skip |= ValidateActionStateProtectedMemory(last_bound_state, bind_point, pipeline, vuid);
    }

    return skip;
}

// Validate the draw-time state for this descriptor set
// We can skip validating the descriptor set if "nothing" has changed since the last validation.
// Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
// any dynamic descriptors, always revalidate rather than caching the values. We currently only
// apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
// binding_req_map which could potentially be expensive.
static bool NeedDrawStateValidated(const vvl::CommandBuffer &cb_state, const vvl::DescriptorSet *descriptor_set,
                                   const LastBound::DescriptorSetSlot &ds_slot, bool disabled_image_layout_validation) {
    return ds_slot.dynamic_offsets.size() > 0 ||
           // Revalidate if descriptor set (or contents) has changed
           ds_slot.validated_set != descriptor_set || ds_slot.validated_set_change_count != descriptor_set->GetChangeCount() ||
           (!disabled_image_layout_validation &&
            ds_slot.validated_set_image_layout_change_count != cb_state.image_layout_change_count);
}

bool CoreChecks::ValidateActionStateDescriptorsPipeline(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                        const vvl::Pipeline &pipeline, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    // If the pipeline is not using any descriptors, then any descriptor state set can be ignored
    if (pipeline.active_slots.empty()) {
        return skip;
    }
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    for (const auto &ds_slot : last_bound_state.ds_slots) {
        // TODO - This currently implicitly is checking for VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT being set
        if (pipeline.descriptor_buffer_mode) {
            if (ds_slot.ds_state && !ds_slot.ds_state->IsPushDescriptor()) {
                LogObjectList objlist = cb_state.GetObjectList(bind_point);
                objlist.add(ds_slot.ds_state->Handle());
                skip |= LogError(vuid.descriptor_buffer_bit_not_set_08115, objlist, vuid.loc(),
                                 "pipeline bound to %s requires a descriptor buffer (because it was created with "
                                 "VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT), but has a bound VkDescriptorSet (%s)",
                                 string_VkPipelineBindPoint(bind_point), FormatHandle(ds_slot.ds_state->Handle()).c_str());
                break;
            }

        } else if (ds_slot.descriptor_buffer_binding.has_value()) {
            skip |= LogError(vuid.descriptor_buffer_set_offset_missing_08117, cb_state.GetObjectList(bind_point), vuid.loc(),
                             "pipeline bound to %s requires a VkDescriptorSet (because it was not created with "
                             "VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT), but has a bound descriptor buffer"
                             " (index=%" PRIu32 " offset=%" PRIu64 ")",
                             string_VkPipelineBindPoint(bind_point), ds_slot.descriptor_buffer_binding->index,
                             ds_slot.descriptor_buffer_binding->offset);
            break;
        }
    }

    // Check if the current pipeline is compatible for the maximum used set with the bound sets.
    if (pipeline.descriptor_buffer_mode) {
        return skip;
    }

    const auto pipeline_layout = pipeline.PipelineLayoutState();
    if (!last_bound_state.IsBoundSetCompatible(pipeline.max_active_slot, *pipeline_layout)) {
        // If they never bound any descriptors
        if (!last_bound_state.desc_set_pipeline_layout) {
            skip |= LogError(vuid.compatible_pipeline_08600, cb_state.GetObjectList(bind_point), vuid.loc(),
                             "The %s statically uses descriptor set %" PRIu32
                             ", but because a descriptor was never bound, the pipeline layouts are not compatible.\nIf using a "
                             "descriptor, make sure to call one of vkCmdBindDescriptorSets, vkCmdPushDescriptorSet, "
                             "vkCmdSetDescriptorBufferOffset, etc for %s",
                             FormatHandle(pipeline).c_str(), pipeline.max_active_slot, string_VkPipelineBindPoint(bind_point));
            return skip;
        }

        LogObjectList objlist(pipeline.Handle());
        const auto layouts = pipeline.PipelineLayoutStateUnion();
        // GPL can have multiple pipeline layouts used to build up a single valid compatible set
        std::ostringstream pipe_layouts_log;
        if (layouts.size() > 1) {
            pipe_layouts_log << "a union of layouts [ ";
            for (const auto &layout : layouts) {
                objlist.add(layout->Handle());
                pipe_layouts_log << FormatHandle(*layout) << " ";
            }
            pipe_layouts_log << "]";
        } else {
            pipe_layouts_log << FormatHandle(*layouts.front());
        }

        objlist.add(last_bound_state.desc_set_pipeline_layout->Handle());

        std::string range =
            pipeline.max_active_slot == 0 ? "set 0 is" : "all sets 0 to " + std::to_string(pipeline.max_active_slot) + " are";
        skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                         "The %s (created with %s) statically uses descriptor set %" PRIu32
                         ", but %s not compatible with the pipeline layout bound with %s (%s)\n%s",
                         FormatHandle(pipeline).c_str(), pipe_layouts_log.str().c_str(), pipeline.max_active_slot, range.c_str(),
                         String(last_bound_state.desc_set_bound_command),
                         FormatHandle(last_bound_state.desc_set_pipeline_layout->Handle()).c_str(),
                         last_bound_state.DescribeNonCompatibleSet(pipeline.max_active_slot, *pipeline_layout).c_str());
    } else {
        // if the bound set is not compatible, the rest will just be extra redundant errors
        for (const auto &[set_index, binding_req_map] : pipeline.active_slots) {
            std::string error_string;
            const auto ds_slot = last_bound_state.ds_slots[set_index];
            if (!ds_slot.ds_state) {
                skip |= LogError(vuid.compatible_pipeline_08600, cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "%s uses set %" PRIu32
                                 " but that set is not bound. (Need to use a command like vkCmdBindDescriptorSets to bind the set)",
                                 FormatHandle(pipeline).c_str(), set_index);
            } else if (pipeline_layout->set_layouts.list[set_index] &&
                       !VerifyDescriptorSetIsCompatibile(*ds_slot.ds_state, *pipeline_layout->set_layouts.list[set_index],
                                                         error_string)) {
                // Set is bound but not compatible w/ corresponding VkPipelineLayoutCreateInfo::pSetLayouts
                VkDescriptorSet set_handle = ds_slot.ds_state->VkHandle();
                const LogObjectList objlist(cb_state.Handle(), set_handle, pipeline.Handle(), pipeline_layout->Handle());
                skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                 "%s bound as set %" PRIu32 " is not compatible with corresponding %s\n%s",
                                 FormatHandle(set_handle).c_str(), set_index, FormatHandle(*pipeline_layout).c_str(),
                                 error_string.c_str());
            } else {  // Valid set is bound and layout compatible, validate that it's updated
                // Pull the set node
                const auto *descriptor_set = ds_slot.ds_state.get();
                ASSERT_AND_CONTINUE(descriptor_set);

                const bool need_validate =
                    NeedDrawStateValidated(cb_state, descriptor_set, ds_slot, disabled[image_layout_validation]);
                if (need_validate) {
                    skip |= ValidateDrawState(*descriptor_set, set_index, binding_req_map, cb_state, vuid,
                                              LogObjectList(pipeline.Handle()));
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateActionStateDescriptorsShaderObject(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                            const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    // Check if the current shader objects are compatible for the maximum used set with the bound sets.
    for (const auto &shader_state : last_bound_state.shader_object_states) {
        // If the shader is not using any descriptors, then any descriptor state set can be ignored
        if (!shader_state || shader_state->active_slots.empty()) {
            continue;
        }

        if (!last_bound_state.IsBoundSetCompatible(shader_state->max_active_slot, *shader_state)) {
            LogObjectList objlist(cb_state.Handle(), shader_state->Handle());

            if (!last_bound_state.desc_set_pipeline_layout) {
                // If they never bound any descriptors
                skip |= LogError(vuid.compatible_pipeline_08600, cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "The %s statically uses descriptor set %" PRIu32
                                 ", but because a descriptor was never bound, the pipeline layouts are not compatible.\nIf using "
                                 "a descriptor, make sure to call one of vkCmdBindDescriptorSets, vkCmdPushDescriptorSet, "
                                 "vkCmdSetDescriptorBufferOffset, etc for %s",
                                 FormatHandle(shader_state->Handle()).c_str(), shader_state->max_active_slot,
                                 string_VkPipelineBindPoint(bind_point));

            } else {
                objlist.add(last_bound_state.desc_set_pipeline_layout->Handle());
                std::string range = shader_state->max_active_slot == 0
                                        ? "set 0 is"
                                        : "all sets 0 to " + std::to_string(shader_state->max_active_slot) + " are";
                skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                 "The %s statically uses descriptor set %" PRIu32
                                 " but %s not compatible with the pipeline layout bound with %s (%s)\n%s",
                                 FormatHandle(shader_state->Handle()).c_str(), shader_state->max_active_slot, range.c_str(),
                                 String(last_bound_state.desc_set_bound_command),
                                 FormatHandle(last_bound_state.desc_set_pipeline_layout->Handle()).c_str(),
                                 last_bound_state.DescribeNonCompatibleSet(shader_state->max_active_slot, *shader_state).c_str());
            }
        } else {
            // if the bound set is not copmatible, the rest will just be extra redundant errors
            for (const auto &[set_index, binding_req_map] : shader_state->active_slots) {
                std::string error_string;
                const auto ds_slot = last_bound_state.ds_slots[set_index];
                if (!ds_slot.ds_state) {
                    const LogObjectList objlist(cb_state.Handle(), shader_state->Handle());
                    skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                     "%s uses set %" PRIu32 " but that set is not bound.",
                                     FormatHandle(shader_state->Handle()).c_str(), set_index);
                } else if (shader_state->set_layouts.list[set_index] &&
                           !VerifyDescriptorSetIsCompatibile(*ds_slot.ds_state, *shader_state->set_layouts.list[set_index],
                                                             error_string)) {
                    // Set is bound but not compatible w/ corresponding VkShaderCreateInfoEXT::pSetLayouts
                    VkDescriptorSet set_handle = ds_slot.ds_state->VkHandle();
                    const LogObjectList objlist(cb_state.Handle(), set_handle, shader_state->Handle());
                    skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                     "%s bound as set %" PRIu32 " is not compatible with corresponding %s\n%s",
                                     FormatHandle(set_handle).c_str(), set_index, FormatHandle(shader_state->Handle()).c_str(),
                                     error_string.c_str());
                } else {  // Valid set is bound and layout compatible, validate that it's updated
                    // Pull the set node
                    const auto *descriptor_set = ds_slot.ds_state.get();
                    ASSERT_AND_CONTINUE(descriptor_set);

                    const bool need_validate =
                        NeedDrawStateValidated(cb_state, descriptor_set, ds_slot, disabled[image_layout_validation]);
                    if (need_validate) {
                        skip |= ValidateDrawState(*descriptor_set, set_index, binding_req_map, cb_state, vuid,
                                                  LogObjectList(shader_state->Handle()));
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateActionStatePushConstant(const LastBound &last_bound_state, const vvl::Pipeline *pipeline,
                                                 const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    // Push constants validation for DGC will need to be done in GPU-AV
    if (vuid.loc().function == vvl::Func::vkCmdExecuteGeneratedCommandsEXT) {
        return skip;
    }

    // Verify if push constants have been set
    // NOTE: Currently not checking whether active push constants are compatible with the active pipeline, nor whether the
    //       "life times" of push constants are correct.
    //       Discussion on validity of these checks can be found at https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602.
    if (pipeline) {
        auto const &pipeline_layout = pipeline->PipelineLayoutState();
        if (!cb_state.push_constant_ranges_layout ||
            (pipeline_layout->push_constant_ranges_layout == cb_state.push_constant_ranges_layout)) {
            for (const auto &stage : pipeline->stage_states) {
                if (!stage.entrypoint || !stage.entrypoint->push_constant_variable) {
                    continue;  // no static push constant in shader
                }

                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_ranges_layout && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline_layout->Handle(), pipeline->Handle());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, vuid.loc(),
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                     "pipeline layout %s.",
                                     string_VkShaderStageFlags(stage.GetStage()).c_str(),
                                     FormatHandle(pipeline_layout->Handle()).c_str());
                }
            }
        }
    } else {
        if (!cb_state.push_constant_ranges_layout) {
            for (const auto &stage : last_bound_state.shader_object_states) {
                if (!stage || !stage->entrypoint || !stage->entrypoint->push_constant_variable) {
                    continue;
                }
                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_ranges_layout && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.Handle(), stage->Handle());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, vuid.loc(),
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet.",
                                     string_VkShaderStageFlags(stage->create_info.stage).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateActionStateProtectedMemory(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                    const vvl::Pipeline *pipeline, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    if (pipeline) {
        for (const auto &stage : pipeline->stage_states) {
            // Stage may not have SPIR-V data (e.g. due to the use of shader module identifier or in Vulkan SC)
            if (!stage.spirv_state) continue;

            if (stage.spirv_state->HasCapability(spv::CapabilityRayQueryKHR)) {
                skip |= LogError(vuid.ray_query_04617, cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "Shader in %s uses OpCapability RayQueryKHR but the command buffer is protected.",
                                 string_VkShaderStageFlags(stage.GetStage()).c_str());
            }
        }
    } else {
        for (const auto &stage : last_bound_state.shader_object_states) {
            if (stage && stage->spirv->HasCapability(spv::CapabilityRayQueryKHR)) {
                const LogObjectList objlist(cb_state.Handle(), stage->Handle());
                skip |= LogError(vuid.ray_query_04617, objlist, vuid.loc(),
                                 "Shader in %s uses OpCapability RayQueryKHR but the command buffer is protected.",
                                 string_VkShaderStageFlags(stage->create_info.stage).c_str());
            }
        }
    }
    return skip;
}

// Note, these don't include the RTX Indirect commands (vkCmdTraceRaysIndirectKHR) because
// they use a VkDeviceAddress instead of a VkBuffer
bool CoreChecks::ValidateIndirectCmd(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                     const DrawDispatchVuid &vuid) const {
    bool skip = false;
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.VkHandle(), buffer_state, vuid.loc().dot(Field::buffer),
                                          vuid.indirect_contiguous_memory_02708);
    skip |= ValidateBufferUsageFlags(objlist, buffer_state, VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit_02290, vuid.loc().dot(Field::buffer));
    if (cb_state.unprotected == false) {
        skip |= LogError(vuid.indirect_protected_cb_02711, objlist, vuid.loc(),
                         "Indirect commands can't be used in protected command buffers.");
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const vvl::CommandBuffer &cb_state, const vvl::Buffer &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, const DrawDispatchVuid &vuid) const {
    bool skip = false;
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(count_buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.VkHandle(), count_buffer_state, vuid.loc().dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(objlist, count_buffer_state, VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit_02715, vuid.loc().dot(Field::countBuffer));
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.create_info.size) {
        skip |= LogError(vuid.indirect_count_offset_04129, objlist, vuid.loc(),
                         "countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         count_buffer_offset, count_buffer_state.create_info.size);
    }
    return skip;
}

bool CoreChecks::ValidateDrawPrimitivesGeneratedQuery(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    const bool with_rasterizer_discard = enabled_features.primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
    const bool with_non_zero_streams = enabled_features.primitivesGeneratedQueryWithNonZeroStreams == VK_TRUE;

    if (with_rasterizer_discard && with_non_zero_streams) {
        return skip;
    }

    bool primitives_generated_query = false;
    for (const auto &query : cb_state.active_queries) {
        auto query_pool_state = Get<vvl::QueryPool>(query.pool);
        if (query_pool_state && query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
            primitives_generated_query = true;
            break;
        }
    }

    if (primitives_generated_query) {
        if (!with_rasterizer_discard && last_bound_state.IsRasterizationDisabled()) {
            skip |= LogError(vuid.primitives_generated_06708, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                             "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                             "VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable set to VK_TRUE, but "
                             "primitivesGeneratedQueryWithRasterizerDiscard feature is not enabled.");
        }
        const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;
        if (!with_non_zero_streams && pipeline) {
            const auto rasterization_state_stream_ci =
                vku::FindStructInPNextChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(pipeline->RasterizationStatePNext());
            if (rasterization_state_stream_ci && rasterization_state_stream_ci->rasterizationStream != 0) {
                skip |= LogError(vuid.primitives_generated_streams_06709, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                                 vuid.loc(),
                                 "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                 "VkPipelineRasterizationStateStreamCreateInfoEXT::rasterizationStream set to %" PRIu32
                                 ", but primitivesGeneratedQueryWithNonZeroStreams feature is not enabled.",
                                 rasterization_state_stream_ci->rasterizationStream);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawProtectedMemory(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    if (!enabled_features.protectedMemory) {
        return skip;
    }

    // Verify vertex & index buffer for unprotected command buffer.
    // Because vertex & index buffer is read only, it doesn't need to care protected command buffer case.
    for (const auto &vertex_buffer_binding : cb_state.current_vertex_buffer_binding_info) {
        if (const auto buffer_state = Get<vvl::Buffer>(vertex_buffer_binding.second.buffer)) {
            skip |= ValidateProtectedBuffer(cb_state, *buffer_state, vuid.loc(), vuid.unprotected_command_buffer_02707,
                                            " (Buffer is the vertex buffer)");
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawFragmentShadingRate(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const auto *pipeline = last_bound_state.pipeline_state;

    if (!enabled_features.primitiveFragmentShadingRate ||
        phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports) {
        return skip;
    }

    if (pipeline) {
        for (auto &stage_state : pipeline->stage_states) {
            const VkShaderStageFlagBits stage = stage_state.GetStage();
            if (!IsValueIn(stage, {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_MESH_BIT_EXT})) {
                continue;
            }
            if (pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) && cb_state.dynamic_state_value.viewport_count != 1) {
                if (stage_state.entrypoint && stage_state.entrypoint->written_builtin_primitive_shading_rate_khr) {
                    skip |=
                        LogError(vuid.viewport_count_primitive_shading_rate_04552, stage_state.module_state->Handle(), vuid.loc(),
                                 "%s shader of currently bound pipeline statically writes to PrimitiveShadingRateKHR built-in, "
                                 "but multiple viewports (%" PRIu32
                                 ") are set by the last call to vkCmdSetViewportWithCountEXT,"
                                 "and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                                 string_VkShaderStageFlagBits(stage), cb_state.dynamic_state_value.viewport_count);
                }
            }
        }
    } else {
        for (uint32_t stage = 0; stage < kShaderObjectStageCount; ++stage) {
            const auto shader_object = last_bound_state.GetShaderState(static_cast<ShaderObjectStage>(stage));
            if (shader_object && shader_object->entrypoint &&
                shader_object->entrypoint->written_builtin_primitive_shading_rate_khr) {
                if (cb_state.dynamic_state_value.viewport_count != 1) {
                    skip |= LogError(vuid.set_viewport_with_count_08642, cb_state.Handle(), vuid.loc(),
                                     "%s shader of currently bound pipeline statically writes to PrimitiveShadingRateKHR built-in, "
                                     "but multiple viewports (%" PRIu32
                                     ") are set by the last call to vkCmdSetViewportWithCountEXT,"
                                     "and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                                     string_VkShaderStageFlagBits(shader_object->create_info.stage),
                                     cb_state.dynamic_state_value.viewport_count);
                }
                break;
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawAttachmentColorBlend(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;

    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const bool has_pipeline = last_bound_state.pipeline_state != nullptr;
    if (has_pipeline && !last_bound_state.pipeline_state->ColorBlendState()) {
        return skip;
    }

    const spirv::EntryPoint *fragment_entry_point = last_bound_state.GetFragmentEntryPoint();
    if (last_bound_state.IsRasterizationDisabled() || !fragment_entry_point) {
        return skip;
    }

    if (enabled_features.colorWriteEnable && last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT) &&
        cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
        // Found in https://gitlab.khronos.org/vulkan/vulkan/-/issues/4116 that not setting all attachment can invalidate previous
        // calls, so the last call needs to have set them all
        const uint32_t blend_attachment_count = (uint32_t)cb_state.active_color_attachments_index.size();
        const uint32_t dynamic_attachment_count = cb_state.dynamic_state_value.color_write_enable_attachment_count;
        if (dynamic_attachment_count < blend_attachment_count) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            skip |= LogError(vuid.dynamic_color_write_enable_count_07750, objlist, vuid.loc(),
                             "There are currently (%" PRIu32
                             ") active color attachments, but the last call to vkCmdSetColorWriteEnableEXT() only set the color "
                             "write enables for attachmentCount of %" PRIu32
                             " and the color write enable state of the remaining attachments is undefined.%s\n%s",
                             blend_attachment_count, dynamic_attachment_count,
                             cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT).c_str(),
                             cb_state.DescribeActiveColorAttachment());
        }
    }

    const auto get_max_fragment_location = [fragment_entry_point]() {
        uint32_t max_fragment_location = 0;
        for (const auto *variable : fragment_entry_point->user_defined_interface_variables) {
            if (variable->storage_class != spv::StorageClassOutput) {
                continue;
            }
            if (variable->decorations.location != spirv::kInvalidValue) {
                max_fragment_location = std::max(max_fragment_location, variable->decorations.location);
            }
        }
        return max_fragment_location;
    };

    const bool dynamic_equation = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    const bool dynamic_advanced = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    const bool dynamic_blend_enable = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    const bool dynamic_write_mask = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    const bool dynamic_blend_constants = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_BLEND_CONSTANTS);

    for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
        const auto &attachment_info = cb_state.active_attachments[i];
        const auto *attachment = attachment_info.image_view;
        if (!attachment || !attachment_info.IsColor()) {
            continue;
        }
        const uint32_t color_index = attachment_info.type_index;

        if (dynamic_write_mask && cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT) &&
            !cb_state.dynamic_state_value.color_write_mask_attachments[color_index]) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(attachment->Handle());
            skip |= LogError(vuid.dynamic_color_write_mask_07478, objlist, vuid.loc(),
                             "vkCmdSetColorWriteMaskEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.%s\n%s",
                             color_index, cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT).c_str(),
                             cb_state.DescribeActiveColorAttachment());
        }

        if (dynamic_blend_enable && cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
            !cb_state.dynamic_state_value.color_blend_enable_attachments[color_index]) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(attachment->Handle());
            skip |= LogError(vuid.dynamic_color_blend_enable_07476, objlist, vuid.loc(),
                             "vkCmdSetColorBlendEnableEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.%s\n%s",
                             color_index, cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT).c_str(),
                             cb_state.DescribeActiveColorAttachment());
            continue;  // If no value is set, IsColorBlendEnabled will give garbage
        }
        // The following all rely on color blend
        if (!last_bound_state.IsColorBlendEnabled(color_index)) {
            continue;
        }

        const bool has_blend_bit = attachment->format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
        if (!has_blend_bit) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(attachment->Handle());
            skip |= LogError(vuid.blend_enable_04727, objlist, vuid.loc(),
                             "%s was created with %s which doesn't support VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, but %s\n"
                             "(supported features: %s)",
                             attachment_info.Describe(cb_state, i).c_str(), string_VkFormat(attachment->create_info.format),
                             last_bound_state.DescribeColorBlendEnabled(color_index).c_str(),
                             string_VkFormatFeatureFlags2(attachment->format_features).c_str());
        }

        // These checks "could" be done in ValidateGraphicsDynamicStateSetStatus, but are exceptions/complex and better done here
        // where we also need to check for other things related to the color attachments
        if (dynamic_equation && dynamic_advanced) {
            // always the case for shader object, can also be done with pipeline
            if (!cb_state.dynamic_state_value.color_blend_equation_attachments[color_index] &&
                !cb_state.dynamic_state_value.color_blend_advanced_attachments[color_index]) {
                LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                objlist.add(attachment->Handle());
                skip |= LogError(vuid.dynamic_color_blend_equation_10864, objlist, vuid.loc(),
                                 "%s needs to be set for color attachmet index %" PRIu32 " (%s)\n%s\n%s",
                                 IsExtEnabled(extensions.vk_ext_blend_operation_advanced)
                                     ? "Either vkCmdSetColorBlendEquationEXT or vkCmdSetColorBlendAdvancedEXT"
                                     : "vkCmdSetColorBlendEquationEXT",
                                 color_index, attachment_info.Describe(cb_state, i).c_str(),
                                 last_bound_state.DescribeColorBlendEnabled(color_index).c_str(),
                                 cb_state.DescribeActiveColorAttachment());
            }
        } else if (dynamic_equation) {
            // Only possible with pipelines
            if (!cb_state.dynamic_state_value.color_blend_equation_attachments[color_index]) {
                const LogObjectList objlist(cb_state.Handle(), attachment->VkHandle(), last_bound_state.pipeline_state->Handle());
                skip |= LogError(
                    vuid.dynamic_color_blend_equation_10862, objlist, vuid.loc(),
                    "The pipeline was created with VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, but "
                    "vkCmdSetColorBlendEquationEXT was never set for color attachment index %" PRIu32 " (%s).%s\n%s\n%s",
                    color_index, attachment_info.Describe(cb_state, i).c_str(),
                    cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT).c_str(),
                    last_bound_state.DescribeColorBlendEnabled(color_index).c_str(), cb_state.DescribeActiveColorAttachment());
            }
        } else if (dynamic_advanced) {
            // Only possible with pipelines
            if (!cb_state.dynamic_state_value.color_blend_advanced_attachments[color_index]) {
                const LogObjectList objlist(cb_state.Handle(), attachment->VkHandle(), last_bound_state.pipeline_state->Handle());
                skip |= LogError(
                    vuid.dynamic_color_blend_equation_10863, objlist, vuid.loc(),
                    "The pipeline was created with VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT, but "
                    "vkCmdSetColorBlendAdvancedEXT was never set for color attachment index %" PRIu32 " (%s).%s\n%s\n%s",
                    color_index, attachment_info.Describe(cb_state, i).c_str(),
                    cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT).c_str(),
                    last_bound_state.DescribeColorBlendEnabled(color_index).c_str(), cb_state.DescribeActiveColorAttachment());
            }
        }

        if (dynamic_advanced && cb_state.dynamic_state_value.color_blend_advanced_attachments[color_index]) {
            if (cb_state.active_color_attachments_index.size() >
                phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments) {
                LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                objlist.add(attachment->Handle());
                skip |= LogError(
                    vuid.blend_advanced_07480, objlist, vuid.loc(),
                    "vkCmdSetColorBlendAdvancedEXT has set color attachment index %" PRIu32
                    " (%s) to advanced blending, but the total active color attachment count (%zu) is greater than "
                    "advancedBlendMaxColorAttachments (%" PRIu32 ").%s\n%s\n%s",
                    color_index, attachment_info.Describe(cb_state, i).c_str(), cb_state.active_color_attachments_index.size(),
                    phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments,
                    cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT).c_str(),
                    last_bound_state.DescribeColorBlendEnabled(color_index).c_str(), cb_state.DescribeActiveColorAttachment());
            }
        }

        if (dynamic_blend_constants && !cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_BLEND_CONSTANTS) &&
            last_bound_state.IsBlendConstantsEnabled(color_index)) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(attachment->Handle());
            skip |= LogError(vuid.dynamic_blend_constants_07835, objlist, vuid.loc(),
                             "%svkCmdSetBlendConstants was never called, but color attachment index %" PRIu32
                             " (%s) has blending enabled (%s), and the blend factor is constant.\n%s\n%s\n%s",
                             has_pipeline ? "VK_DYNAMIC_STATE_BLEND_CONSTANT state is dynamic, " : "", color_index,
                             attachment_info.Describe(cb_state, i).c_str(),
                             last_bound_state.DescribeColorBlendEnabled(color_index).c_str(),
                             last_bound_state.DescribeBlendFactorEquation(color_index).c_str(),
                             cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_BLEND_CONSTANTS).c_str(),
                             cb_state.DescribeActiveColorAttachment());
        }

        if (last_bound_state.IsDualBlending(color_index)) {
            const uint32_t max_fragment_location = get_max_fragment_location();
            if (max_fragment_location >= phys_dev_props.limits.maxFragmentDualSrcAttachments) {
                skip |= LogError(vuid.blend_dual_source_09239, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                                 "color attachment index %" PRIu32
                                 " (%s) is using Dual-Source Blending, but the largest output fragment Location (%" PRIu32
                                 ") is not less than maxFragmentDualSrcAttachments (%" PRIu32 ").\n%s\n%s",
                                 color_index, attachment_info.Describe(cb_state, i).c_str(), max_fragment_location,
                                 phys_dev_props.limits.maxFragmentDualSrcAttachments,
                                 last_bound_state.DescribeColorBlendEnabled(color_index).c_str(),
                                 last_bound_state.DescribeBlendFactorEquation(color_index).c_str());
                break;
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawAttachmentSampleLocation(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    if (!last_bound_state.IsFragmentBound()) {
        return skip;
    }

    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    if (last_bound_state.IsSampleLocationsEnable()) {
        for (uint32_t i = 0; i < cb_state.active_attachments.size(); i++) {
            const auto &attachment_info = cb_state.active_attachments[i];
            const auto *attachment = attachment_info.image_view;
            if (!attachment || !attachment_info.IsDepth()) {
                continue;
            }
            if ((attachment->image_state->create_info.flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) == 0) {
                const LogObjectList objlist(cb_state.Handle(), attachment->Handle());
                const char *err = last_bound_state.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)
                                      ? vuid.sample_locations_enable_07484
                                      : vuid.sample_location_02689;
                skip |= LogError(err, objlist, vuid.loc(),
                                 "sampleLocationsEnable is true, but %s (%s created with %s) was not created with "
                                 "VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT.",
                                 attachment_info.Describe(cb_state, i).c_str(), FormatHandle(attachment->Handle()).c_str(),
                                 FormatHandle(attachment->image_state->Handle()).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDepthStencilAttachments(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;

    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    for (uint32_t i = 0; i < cb_state.active_attachments.size(); i++) {
        const auto &attachment_info = cb_state.active_attachments[i];
        const auto *attachment = attachment_info.image_view;
        if (!attachment) {
            continue;
        }

        if (attachment_info.IsDepth() && last_bound_state.IsDepthWriteEnable() &&
            IsImageLayoutDepthReadOnly(attachment_info.layout)) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(attachment->Handle());
            skip |= LogError(vuid.depth_read_only_06886, objlist, vuid.loc(),
                             "depthWriteEnable is VK_TRUE, but %s (%s created with %s) has a layout %s which is READ_ONLY.",
                             attachment_info.Describe(cb_state, i).c_str(), FormatHandle(attachment->Handle()).c_str(),
                             FormatHandle(attachment->image_state->Handle()).c_str(), string_VkImageLayout(attachment_info.layout));
        }

        if (attachment_info.IsStencil() && last_bound_state.IsStencilTestEnable()) {
            VkStencilOpState front = last_bound_state.GetStencilOpStateFront();
            VkStencilOpState back = last_bound_state.GetStencilOpStateBack();

            const bool all_keep_op = ((front.failOp == VK_STENCIL_OP_KEEP) && (front.passOp == VK_STENCIL_OP_KEEP) &&
                                      (front.depthFailOp == VK_STENCIL_OP_KEEP) && (back.failOp == VK_STENCIL_OP_KEEP) &&
                                      (back.passOp == VK_STENCIL_OP_KEEP) && (back.depthFailOp == VK_STENCIL_OP_KEEP));
            const bool write_mask_enabled = (front.writeMask != 0) && (back.writeMask != 0);

            if (!all_keep_op && write_mask_enabled) {
                bool is_stencil_layout_read_only = false;
                if (attachment_info.separate_stencil_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
                    is_stencil_layout_read_only = IsImageLayoutStencilReadOnly(attachment_info.separate_stencil_layout);
                } else {
                    is_stencil_layout_read_only = IsImageLayoutStencilReadOnly(attachment_info.layout);
                }

                if (is_stencil_layout_read_only) {
                    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                    objlist.add(attachment->Handle());
                    skip |= LogError(
                        vuid.stencil_read_only_06887, objlist, vuid.loc(),
                        "%s (%s created with %s) has a layout of %s which is READ_ONLY, but stencilTestEnable is VK_TRUE, "
                        "writeMask is non-zero, and all stencil ops are not VK_STENCIL_OP_KEEP.\n"
                        "front:\n%sback:\n%s",
                        attachment_info.Describe(cb_state, i).c_str(), FormatHandle(attachment->Handle()).c_str(),
                        FormatHandle(attachment->image_state->Handle()).c_str(), string_VkImageLayout(attachment_info.layout),
                        string_VkStencilOpState(front).c_str(), string_VkStencilOpState(back).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawTessellation(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    if ((last_bound_state.GetAllActiveBoundStages() & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) == 0) {
        return skip;
    }

    const spirv::ExecutionModeSet *tesc_execution_mode = nullptr;
    const spirv::ExecutionModeSet *tese_execution_mode = nullptr;

    if (last_bound_state.pipeline_state) {
        for (const auto &stage_state : last_bound_state.pipeline_state->stage_states) {
            const VkShaderStageFlagBits stage = stage_state.GetStage();
            if (stage & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                if (stage_state.entrypoint) {
                    tesc_execution_mode = &stage_state.entrypoint->execution_mode;
                }
            } else if (stage & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                if (stage_state.entrypoint) {
                    tese_execution_mode = &stage_state.entrypoint->execution_mode;
                }
            }
        }
    } else {
        const auto tesc_shader = last_bound_state.GetShaderState(ShaderObjectStage::TESSELLATION_CONTROL);
        if (tesc_shader && tesc_shader->entrypoint) {
            tesc_execution_mode = &tesc_shader->entrypoint->execution_mode;
        }
        const auto tese_shader = last_bound_state.GetShaderState(ShaderObjectStage::TESSELLATION_EVALUATION);
        if (tese_shader && tese_shader->entrypoint) {
            tese_execution_mode = &tese_shader->entrypoint->execution_mode;
        }
    }

    if (!tesc_execution_mode || !tese_execution_mode) {
        return skip;  // Occurs if using binary shader object
    }

    // VUID being added in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7694
    const uint32_t tesc_subdivision = tesc_execution_mode->GetTessellationSubdivision();
    const uint32_t tese_subdivision = tese_execution_mode->GetTessellationSubdivision();
    if (tesc_subdivision != spirv::kInvalidValue && tese_subdivision != spirv::kInvalidValue &&
        tesc_subdivision != tese_subdivision) {
        skip |= LogError("UNASSIGNED-vkCmdDraw-tessellation-subdivision",
                         last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The subdivision specified in tessellation control shader (%s) does not match the subdivision in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_subdivision), string_SpvExecutionMode(tese_subdivision));
    }

    const uint32_t tesc_orientation = tesc_execution_mode->GetTessellationOrientation();
    const uint32_t tese_orientation = tese_execution_mode->GetTessellationOrientation();
    if (tesc_orientation != spirv::kInvalidValue && tese_orientation != spirv::kInvalidValue &&
        tesc_orientation != tese_orientation) {
        skip |= LogError("UNASSIGNED-vkCmdDraw-tessellation-orientation",
                         last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The orientation specified in tessellation control shader (%s) does not match the orientation in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_orientation), string_SpvExecutionMode(tese_orientation));
    }

    const uint32_t tesc_spacing = tesc_execution_mode->GetTessellationSpacing();
    const uint32_t tese_spacing = tese_execution_mode->GetTessellationSpacing();
    if (tesc_spacing != spirv::kInvalidValue && tese_spacing != spirv::kInvalidValue && tesc_spacing != tese_spacing) {
        skip |= LogError("UNASSIGNED-vkCmdDraw-tessellation-spacing",
                         last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The spacing specified in tessellation control shader (%s) does not match the spacing in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_spacing), string_SpvExecutionMode(tese_spacing));
    }
    const uint32_t tesc_patch_size = tesc_execution_mode->output_vertices;
    const uint32_t tese_patch_size = tese_execution_mode->output_vertices;
    if (tesc_patch_size != spirv::kInvalidValue && tese_patch_size != spirv::kInvalidValue && tesc_patch_size != tese_patch_size) {
        skip |= LogError("UNASSIGNED-vkCmdDraw-tessellation-patch-size",
                         last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "The OutputVertices (patch size) specified in tessellation control shader (%" PRIu32
                         ") does not match the spacing in "
                         "tessellation evaluation shader (%" PRIu32 ").",
                         tesc_patch_size, tese_patch_size);
    }

    return skip;
}

bool CoreChecks::ValidateDrawVertexBinding(const LastBound &last_bound, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound.cb_state;

    if ((last_bound.GetAllActiveBoundStages() & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        return skip;
    }
    // Since we need to know if the Vertex shader actually declares/uses the Input Location, if the shader validation was disabled,
    // there will no SPIR-V to reflect the information from.
    if (disabled[shader_validation]) {
        return skip;
    }

    // Vertex bindings validation for DGC will need to be done in GPU-AV
    if (vuid.loc().function == vvl::Func::vkCmdExecuteGeneratedCommandsEXT) {
        return skip;
    }

    const bool has_dynamic_descriptions = last_bound.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    const auto &vertex_bindings = has_dynamic_descriptions ? cb_state.dynamic_state_value.vertex_bindings
                                                           : last_bound.pipeline_state->vertex_input_state->bindings;

    const bool robust_pipeline = last_bound.pipeline_state && last_bound.pipeline_state->uses_pipeline_vertex_robustness;

    auto print_binding = [has_dynamic_descriptions](const VertexBindingState binding_description) {
        std::stringstream ss;
        if (has_dynamic_descriptions) {
            ss << "the last call to vkCmdSetVertexInputEXT";
        } else {
            ss << "the last bound pipeline";
        }
        ss << " has pVertexBindingDescriptions[" << binding_description.index << "].binding (" << binding_description.desc.binding
           << ") (pointing to Locations [";
        const char *separator = "";
        for (const auto &location : binding_description.locations) {
            ss << separator << location.first;
            separator = ", ";
        }
        ss << "])";
        return ss.str();
    };

    const spirv::EntryPoint *vertex_entry_point = last_bound.GetVertexEntryPoint();
    // Can be NULL if pipeline binaries are used
    if (!vertex_entry_point) {
        return skip;
    }
    vvl::unordered_set<uint32_t> spirv_input_locations;
    for (const auto &pair : vertex_entry_point->input_interface_slots) {
        spirv_input_locations.emplace(pair.first.Location());
    }

    // It is ok to have binding descriptions not used, them and find if there is matching buffer tied to it or not
    for (const auto &[binding_index, binding_description] : vertex_bindings) {
        // If no attribute points to a binding, it is unused
        if (binding_description.locations.empty()) {
            continue;
        }

        bool shader_has_location = false;
        for (const auto &location : binding_description.locations) {
            if (spirv_input_locations.find(location.first) != spirv_input_locations.end()) {
                shader_has_location = true;
                break;
            }
        }
        if (!shader_has_location) {
            // If the Vertex shader doesn't declare the vertex input, there can be invalid/unbound bindings
            continue;
        }

        const auto *vertex_buffer_binding = vvl::Find(cb_state.current_vertex_buffer_binding_info, binding_index);
        if (!vertex_buffer_binding || !vertex_buffer_binding->bound) {
            // Likely to not get
            skip |=
                LogError(vuid.vertex_binding_04007, last_bound.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                         "%s which didn't have a buffer bound from any vkCmdBindVertexBuffers call in this command buffer.",
                         print_binding(binding_description).c_str());
            continue;
        }

        // This means the app actively set the buffer to null
        // Going to hit VUID-vkCmdBindVertexBuffers-pBuffers-04001 first anyway
        if (vertex_buffer_binding->buffer == VK_NULL_HANDLE) {
            if (!enabled_features.nullDescriptor) {
                skip |=
                    LogError(vuid.vertex_binding_null_04008, last_bound.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                             vuid.loc(), "%s which was bound with a buffer of VK_NULL_HANDLE, but nullDescriptor is not enabled.",
                             print_binding(binding_description).c_str());
            }
            continue;
        }

        const auto vertex_buffer_state = Get<vvl::Buffer>(vertex_buffer_binding->buffer);
        if (!vertex_buffer_state) {
            skip |= LogError(
                vuid.vertex_binding_04007, last_bound.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                "%s which has an invalid/destroyed buffer bound from a vkCmdBindVertexBuffers call in this command buffer.",
                print_binding(binding_description).c_str());
            continue;
        }

        for (const auto &location : binding_description.locations) {
            const auto attr_index = location.second.index;
            const auto &attr_desc = location.second.desc;

            if (last_bound.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
                const VkDeviceSize attribute_binding_extent = attr_desc.offset + GetVertexInputFormatSize(attr_desc.format);
                if (vertex_buffer_binding->stride != 0 && vertex_buffer_binding->stride < attribute_binding_extent) {
                    skip |= LogError("VUID-vkCmdBindVertexBuffers2-pStrides-06209",
                                     last_bound.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                                     "(attribute binding %" PRIu32 ", attribute location %" PRIu32 ") The pStrides value (%" PRIu64
                                     ") parameter in the last call to %s is not 0 "
                                     "and is less than the extent of the binding for the attribute (%" PRIu64 ").",
                                     attr_desc.binding, attr_desc.location, vertex_buffer_binding->stride, String(vuid.function),
                                     attribute_binding_extent);
                }
            }

            if (!enabled_features.robustBufferAccess && !robust_pipeline) {
                const VkDeviceSize vertex_buffer_offset = vertex_buffer_binding->offset;

                // Use 1 as vertex/instance index to use buffer stride as well
                const VkDeviceSize attrib_address = vertex_buffer_offset + vertex_buffer_binding->stride + attr_desc.offset;

                VkDeviceSize vtx_attrib_req_alignment = GetVertexInputFormatSize(attr_desc.format);

                // TODO - There is no real spec language describing these, but also almost no one supports these formats for vertex
                // input and this check should probably just removed and do the safe division always. Will need to run against CTS
                // before-and-after to make sure.
                if (!vkuFormatIsPacked(attr_desc.format) && !vkuFormatIsCompressed(attr_desc.format) &&
                    !vkuFormatIsSinglePlane_422(attr_desc.format) && !vkuFormatIsMultiplane(attr_desc.format)) {
                    vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, vkuFormatComponentCount(attr_desc.format));
                }

                if (SafeModulo(attrib_address, vtx_attrib_req_alignment) != 0) {
                    LogObjectList objlist(last_bound.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS));
                    objlist.add(vertex_buffer_state->Handle());
                    skip |= LogError(vuid.vertex_binding_attribute_02721, objlist, vuid.loc(),
                                     "Format %s has an alignment of %" PRIu64 " but the alignment of attribAddress (%" PRIu64
                                     ") is not aligned in pVertexAttributeDescriptions[%" PRIu32 "] (binding=%" PRIu32
                                     " location=%" PRIu32 ") where attribAddress = vertex buffer offset (%" PRIu64
                                     ") + binding stride (%" PRIu64 ") + attribute offset (%" PRIu32 ").",
                                     string_VkFormat(attr_desc.format), vtx_attrib_req_alignment, attrib_address, attr_index,
                                     attr_desc.binding, attr_desc.location, vertex_buffer_offset, vertex_buffer_binding->stride,
                                     attr_desc.offset);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicRenderpassExternalFormatResolve(const LastBound &last_bound_state,
                                                                    const vvl::RenderPass &rp_state,
                                                                    const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;

    if (!last_bound_state.pipeline_state) {
        // Need to understand if possible to actually use this extension with Shader Object
        // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7510#note_547500
        return skip;
    }
    const uint64_t pipeline_external_format = GetExternalFormat(last_bound_state.pipeline_state->GetCreateInfoPNext());
    if (pipeline_external_format == 0) {
        return skip;
    }

    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const VkRenderingInfo &rendering_info = *(rp_state.dynamic_rendering_begin_rendering_info.ptr());

    if (rendering_info.colorAttachmentCount == 1 &&
        rendering_info.pColorAttachments[0].resolveMode == VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_BIT_ANDROID) {
        if (auto resolve_image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[0].resolveImageView)) {
            if (resolve_image_view_state->image_state->ahb_format != pipeline_external_format) {
                skip |= LogError(vuid.external_format_resolve_09362, objlist, vuid.loc(),
                                 "pipeline externalFormat is %" PRIu64
                                 " but the resolveImageView's image was created with externalFormat %" PRIu64 "",
                                 pipeline_external_format, resolve_image_view_state->image_state->ahb_format);
            }
        }

        if (auto color_image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[0].imageView)) {
            if (color_image_view_state->image_state->ahb_format != pipeline_external_format) {
                skip |= LogError(vuid.external_format_resolve_09363, objlist, vuid.loc(),
                                 "pipeline externalFormat is %" PRIu64
                                 " but the imageView's image was created with externalFormat %" PRIu64 "",
                                 pipeline_external_format, color_image_view_state->image_state->ahb_format);
            }
        }
    }

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
        cb_state.dynamic_state_value.color_blend_enable_attachments.test(0)) {
        skip |= LogError(vuid.external_format_resolve_09364, objlist, vuid.loc(),
                         "pipeline externalFormat is %" PRIu64 ", but dynamic blend enable for attachment zero was set to VK_TRUE.",
                         pipeline_external_format);
    }
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        cb_state.dynamic_state_value.rasterization_samples != VK_SAMPLE_COUNT_1_BIT) {
        skip |=
            LogError(vuid.external_format_resolve_09365, objlist, vuid.loc(),
                     "pipeline externalFormat is %" PRIu64 ", but dynamic rasterization samples set to %s.",
                     pipeline_external_format, string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
    }
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR)) {
        if (cb_state.dynamic_state_value.fragment_size.width != 1) {
            skip |= LogError(vuid.external_format_resolve_09368, objlist, vuid.loc(),
                             "pipeline externalFormat is %" PRIu64 ", but dynamic fragment size width is %" PRIu32 ".",
                             pipeline_external_format, cb_state.dynamic_state_value.fragment_size.width);
        }
        if (cb_state.dynamic_state_value.fragment_size.height != 1) {
            skip |= LogError(vuid.external_format_resolve_09369, objlist, vuid.loc(),
                             "pipeline externalFormat is %" PRIu64 ", but dynamic fragment size height is %" PRIu32 ".",
                             pipeline_external_format, cb_state.dynamic_state_value.fragment_size.height);
        }
    }

    if (auto fragment_entry_point = last_bound_state.GetFragmentEntryPoint()) {
        if (fragment_entry_point->execution_mode.Has(spirv::ExecutionModeSet::depth_replacing_bit)) {
            skip |= LogError(vuid.external_format_resolve_09372, objlist, vuid.loc(),
                             "pipeline externalFormat is %" PRIu64 " but the fragment shader declares DepthReplacing.",
                             pipeline_external_format);
        } else if (fragment_entry_point->execution_mode.Has(spirv::ExecutionModeSet::stencil_ref_replacing_bit)) {
            skip |= LogError(vuid.external_format_resolve_09372, objlist, vuid.loc(),
                             "pipeline externalFormat is %" PRIu64 " but the fragment shader declares StencilRefReplacingEXT.",
                             pipeline_external_format);
        }
    }

    return skip;
}
