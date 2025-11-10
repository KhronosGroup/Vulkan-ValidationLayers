/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Copyright (c) 2025 Arm Limited.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
#include "state_tracker/cmd_buffer_state.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_format_utils.h>
#include "error_message/error_location.h"
#include "generated/command_validation.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/last_bound_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/queue_state.h"
#include "utils/assert_utils.h"
#include "utils/image_utils.h"

using RangeGenerator = subresource_adapter::RangeGenerator;

static ShaderObjectStage inline ConvertToShaderObjectStage(VkShaderStageFlagBits stage) {
    if (stage == VK_SHADER_STAGE_VERTEX_BIT) return ShaderObjectStage::VERTEX;
    if (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) return ShaderObjectStage::TESSELLATION_CONTROL;
    if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) return ShaderObjectStage::TESSELLATION_EVALUATION;
    if (stage == VK_SHADER_STAGE_GEOMETRY_BIT) return ShaderObjectStage::GEOMETRY;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderObjectStage::FRAGMENT;
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return ShaderObjectStage::COMPUTE;
    if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) return ShaderObjectStage::TASK;
    if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) return ShaderObjectStage::MESH;

    assert(false);

    return ShaderObjectStage::LAST;
}

// Dynamic Rendering we know it is depth only, but for VkRenderPass, we need to check incase it is a stencil only attachment
bool AttachmentInfo::IsDepth() const {
    return type == Type::Depth ||
           (type == Type::DepthStencil && image_view && vkuFormatHasDepth(image_view->image_state->create_info.format));
}

bool AttachmentInfo::IsStencil() const {
    return type == Type::Stencil ||
           (type == Type::DepthStencil && image_view && vkuFormatHasStencil(image_view->image_state->create_info.format));
}

// For Traditional RenderPasses, the index is simply the index into the VkRenderPassCreateInfo::pAttachments,
// but for dynamic rendering, there is no "standard" way to map the index, instead we have our own custom indexing and it is not
// obvious at all to the user where it came from
std::string AttachmentInfo::Describe(const vvl::CommandBuffer &cb_state, uint32_t rp_index) const {
    std::ostringstream ss;
    if (cb_state.attachment_source == AttachmentSource::DynamicRendering) {
        ss << "VkRenderingInfo::";
        if (type == Type::Color) {
            ss << "pColorAttachments[" << rp_index << "].imageView";
        } else if (type == Type::ColorResolve) {
            // This assumes the caller calculated the correct index with GetDynamicRenderingColorResolveAttachmentIndex
            ss << "pColorAttachments[" << rp_index << "].resolveImageView";
        } else if (type == Type::Depth) {
            ss << "pDepthAttachment->imageView";
        } else if (type == Type::DepthResolve) {
            ss << "pStencilAttachment->resolveImageView";
        } else if (type == Type::Stencil) {
            ss << "pStencilAttachment->imageView";
        } else if (type == Type::StencilResolve) {
            ss << "pStencilAttachment->resolveImageView";
        } else if (type == Type::FragmentDensityMap) {
            ss << "pNext<VkRenderingFragmentDensityMapAttachmentInfoEXT>.imageView";
        } else if (type == Type::FragmentShadingRate) {
            ss << "pNext<VkRenderingFragmentShadingRateAttachmentInfoKHR>.imageView";
        }
    } else {
        // if the user has a [color, depth, color] the last color would have
        //   rp_index == 2
        //   index == 1
        ss << "VkRenderPassCreateInfo::pAttachments[" << rp_index << "] (Subpass " << cb_state.GetActiveSubpass() << ", ";

        if (type == Type::Empty) {
            ss << "VK_ATTACHMENT_UNUSED";
        } else if (type == Type::Input) {
            ss << "VkSubpassDescription::pInputAttachments[" << type_index << "]";
        } else if (type == Type::Color) {
            ss << "VkSubpassDescription::pColorAttachments[" << type_index << "]";
        } else if (type == Type::ColorResolve) {
            ss << "VkSubpassDescription::pResolveAttachments[" << type_index << "]";
        } else if (type == Type::DepthStencil) {
            ss << "VkSubpassDescription::pDepthStencilAttachment";
        } else if (type == Type::FragmentDensityMap) {
            ss << "VkRenderPassFragmentDensityMapCreateInfoEXT::fragmentDensityMapAttachment";
        } else if (type == Type::FragmentShadingRate) {
            ss << "VkFragmentShadingRateAttachmentInfoKHR::pFragmentShadingRateAttachment";
        } else {
            ss << "Unknown Type";
        }

        ss << ")";
    }
    return ss.str();
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkEventCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

namespace vvl {

Event::Event(VkEvent handle, const VkEventCreateInfo *create_info)
    : StateObject(handle, kVulkanObjectTypeEvent),
      flags(create_info->flags)
#ifdef VK_USE_PLATFORM_METAL_EXT
      ,
      metal_event_export(GetMetalExport(create_info))
#endif  // VK_USE_PLATFORM_METAL_EXT
{
}

CommandPool::CommandPool(DeviceState &dev, VkCommandPool handle, const VkCommandPoolCreateInfo *create_info, VkQueueFlags flags)
    : StateObject(handle, kVulkanObjectTypeCommandPool),
      dev_data(dev),
      createFlags(create_info->flags),
      queueFamilyIndex(create_info->queueFamilyIndex),
      queue_flags(flags),
      unprotected((create_info->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) == 0) {}

void CommandPool::Allocate(const VkCommandBufferAllocateInfo *allocate_info, const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < allocate_info->commandBufferCount; i++) {
        auto new_cb = dev_data.CreateCmdBufferState(command_buffers[i], allocate_info, this);
        commandBuffers.emplace(command_buffers[i], new_cb.get());
        dev_data.Add(std::move(new_cb));
    }
}

void CommandPool::Free(uint32_t count, const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < count; i++) {
        auto iter = commandBuffers.find(command_buffers[i]);
        if (iter != commandBuffers.end()) {
            dev_data.Destroy<CommandBuffer>(iter->first);
            commandBuffers.erase(iter);
        }
    }
}

void CommandPool::Reset(const Location &loc) {
    for (auto &entry : commandBuffers) {
        auto guard = entry.second->WriteLock();
        entry.second->Reset(loc);
    }
}

void CommandPool::Destroy() {
    for (auto &entry : commandBuffers) {
        dev_data.Destroy<CommandBuffer>(entry.first);
    }
    commandBuffers.clear();
    StateObject::Destroy();
}

void CommandBuffer::SetActiveSubpass(uint32_t subpass) {
    active_subpass_ = subpass;
    // Always reset stored rasterization samples count
    active_subpass_sample_count_ = std::nullopt;
}

// Put here, instead of vvl::RenderPass for ease of access
const char *CommandBuffer::DescribeActiveColorAttachment() const {
    if (!active_render_pass) {
        return "";
    } else if (active_render_pass->UsesDynamicRendering()) {
        return "Active color attachments are those where VkRenderingInfo::pColorAttachments[i].imageView != VK_NULL_HANDLE";
    } else {
        return "Active color attachments are those where pSubpasses[i].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED";
    }
}

CommandBuffer::CommandBuffer(DeviceState &dev, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *allocate_info,
                             const vvl::CommandPool *pool)
    : RefcountedStateObject(handle, kVulkanObjectTypeCommandBuffer),
      allocate_info(*allocate_info),
      command_pool(pool),
      dev_data(dev),
      unprotected(pool->unprotected),
      lastBound({{{*this, VK_PIPELINE_BIND_POINT_GRAPHICS},
                  {*this, VK_PIPELINE_BIND_POINT_COMPUTE},
                  {*this, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR},
                  {*this, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM}}}) {
    ResetCBState();
}

// Get the image viewstate for a given framebuffer attachment
vvl::ImageView *CommandBuffer::GetActiveAttachmentImageViewState(uint32_t index) {
    assert(!active_attachments.empty() && index != VK_ATTACHMENT_UNUSED && (index < active_attachments.size()));
    return active_attachments[index].image_view;
}

// Get the image viewstate for a given framebuffer attachment
const vvl::ImageView *CommandBuffer::GetActiveAttachmentImageViewState(uint32_t index) const {
    if (active_attachments.empty() || index == VK_ATTACHMENT_UNUSED || (index >= active_attachments.size())) {
        return nullptr;
    }
    return active_attachments[index].image_view;
}

void CommandBuffer::AddChild(std::shared_ptr<StateObject> &child_node) {
    assert(child_node);
    if (child_node->AddParent(this)) {
        object_bindings.insert(child_node);
    }
}

void CommandBuffer::RemoveChild(std::shared_ptr<StateObject> &child_node) {
    assert(child_node);
    child_node->RemoveParent(this);
    object_bindings.erase(child_node);
}

// Reset the command buffer state
// Maintain the createInfo and set state to CB_NEW, but clear all other state
void CommandBuffer::ResetCBState() {
    // Remove object bindings
    for (const auto &obj : object_bindings) {
        obj->RemoveParent(this);
    }
    object_bindings.clear();
    broken_bindings.clear();

    begin_info_flags = 0;
    has_inheritance = false;

    state = CbState::New;
    command_count = 0;
    submit_count = 0;
    image_layout_change_count = 1;  // Start at 1. 0 is insert value for validation cache versions, s.t. new == dirty

    dynamic_state_status.cb.reset();
    dynamic_state_status.pipeline.reset();
    dynamic_state_status.history.reset();
    dynamic_state_status.rtx_stack_size_cb = false;
    dynamic_state_status.rtx_stack_size_pipeline = false;
    CBDynamicFlags all;
    dynamic_state_value.reset(all.set());
    memset(&invalidated_state_pipe, 0, sizeof(VkPipeline) * CB_DYNAMIC_STATE_STATUS_NUM);

    dirty_static_state = false;

    has_render_pass_instance = false;
    resumes_render_pass_instance = false;
    last_suspend_state = SuspendState::Empty;
    first_action_or_sync_command = Func::Empty;
    active_render_pass = nullptr;
    sample_locations_begin_info = {};
    attachment_source = AttachmentSource::Empty;
    active_attachments.clear();
    active_subpasses.clear();
    active_color_attachments_index.clear();
    has_render_pass_striped = false;
    striped_count = 0;
    active_subpass_contents = VK_SUBPASS_CONTENTS_INLINE;
    SetActiveSubpass(0);
    rendering_attachments.Reset();
    waited_events.clear();
    events.clear();
    write_events_before_wait.clear();
    active_queries.clear();
    started_queries.clear();
    render_pass_queries.clear();
    image_layout_registry.clear();
    aliased_image_layout_map.clear();
    current_vertex_buffer_binding_info.clear();
    primary_command_buffer = VK_NULL_HANDLE;
    linked_command_buffers.clear();

    for (auto &item : lastBound) {
        item.Reset();
    }
    active_framebuffer = VK_NULL_HANDLE;
    index_buffer_binding.reset();

    tensor_barriers.clear();

    // Clean up video specific states
    bound_video_session = nullptr;
    bound_video_session_parameters = nullptr;
    bound_video_picture_resources.clear();
    video_encode_quality_level.reset();
    video_session_updates.clear();

    descriptor_buffer.Reset();

    // Clean up the label data
    label_stack_depth_ = 0;
    label_commands_.clear();

    push_constant_ranges_layout.reset();

    transform_feedback_active = false;
    transform_feedback_buffers_bound = 0;

    // Clean up the label data
    dev_data.debug_report->ResetCmdDebugUtilsLabel(VkHandle());
}

void CommandBuffer::Reset(const Location &loc) {
    ResetCBState();
    // Remove reverse command buffer links.
    Invalidate(true);
    for (auto &item : sub_states_) {
        item.second->Reset(loc);
    }
}

void CommandBuffer::Destroy() {
    // Remove the cb debug labels
    dev_data.debug_report->EraseCmdDebugUtilsLabel(VkHandle());
    {
        auto guard = WriteLock();
        ResetCBState();
    }
    for (auto &item : sub_states_) {
        item.second->Destroy();
    }
    sub_states_.clear();
    StateObject::Destroy();
}

void CommandBuffer::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    {
        auto guard = WriteLock();
        assert(!invalid_nodes.empty());
        // Save all of the vulkan handles between the command buffer and the now invalid node
        LogObjectList log_list;
        for (auto &obj : invalid_nodes) {
            log_list.add(obj->Handle());
        }

        bool found_invalid = false;
        for (auto &obj : invalid_nodes) {
            // Only record a broken binding if one of the nodes in the invalid chain is still
            // being tracked by the command buffer. This is to try to avoid race conditions
            // caused by separate CommandBuffer and StateObject::parent_nodes locking.
            if (object_bindings.erase(obj)) {
                obj->RemoveParent(this);
                found_invalid = true;
            }
            switch (obj->Type()) {
                case kVulkanObjectTypeCommandBuffer:
                    if (unlink) {
                        linked_command_buffers.erase(static_cast<CommandBuffer *>(obj.get()));
                    }
                    break;
                case kVulkanObjectTypeImage:
                    if (unlink) {
                        image_layout_registry.erase(obj->Handle().Cast<VkImage>());
                    }
                    break;
                default:
                    break;
            }
        }
        if (found_invalid) {
            if (state == CbState::Recording) {
                state = CbState::InvalidIncomplete;
            } else if (state == CbState::Recorded) {
                state = CbState::InvalidComplete;
            }
            broken_bindings.emplace(invalid_nodes[0]->Handle(), log_list);
        }
    }
    for (auto &item : sub_states_) {
        item.second->NotifyInvalidate(invalid_nodes, unlink);
    }
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
}

// The const variant only need the image as it is the key for the map
std::shared_ptr<const CommandBufferImageLayoutMap> CommandBuffer::GetImageLayoutMap(VkImage image) const {
    auto it = image_layout_registry.find(image);
    if (it == image_layout_registry.cend()) {
        return nullptr;
    }
    return it->second;
}

// The non-const variant only needs the image state, as the factory requires it to construct a new entry
std::shared_ptr<CommandBufferImageLayoutMap> CommandBuffer::GetOrCreateImageLayoutMap(const vvl::Image &image_state) {
    // Make sure we don't create a nullptr keyed entry for a zombie Image
    if (image_state.Destroyed() || !image_state.layout_map) {
        return nullptr;
    }
    auto iter = image_layout_registry.find(image_state.VkHandle());
    if (iter != image_layout_registry.end() && iter->second && image_state.GetId() == iter->second->image_id) {
        return iter->second;
    }
    std::shared_ptr<CommandBufferImageLayoutMap> image_layout_map;
    if (image_state.CanAlias()) {
        // Aliasing images need to share the same local layout map.
        // Since they use the same global layout state, use it as a key
        // for the local state. We don't need a lock on the global range
        // map to do a lookup based on its pointer.
        const auto *p_global_layout_map = image_state.layout_map.get();
        auto alias_iter = aliased_image_layout_map.find(p_global_layout_map);
        if (alias_iter != aliased_image_layout_map.end()) {
            image_layout_map = alias_iter->second;
        } else {
            image_layout_map = std::make_shared<CommandBufferImageLayoutMap>(image_state.subresource_encoder.SubresourceCount(),
                                                                             image_state.GetId());
            // Save the local layout map for the next aliased image.
            // The global layout map pointer is only used as a key into the local lookup
            // table so it doesn't need to be locked.
            aliased_image_layout_map.emplace(p_global_layout_map, image_layout_map);
        }
    } else {
        image_layout_map =
            std::make_shared<CommandBufferImageLayoutMap>(image_state.subresource_encoder.SubresourceCount(), image_state.GetId());
    }
    if (iter != image_layout_registry.end()) {
        // overwrite the stale entry
        iter->second = image_layout_map;
    } else {
        // add a new entry
        image_layout_registry.insert({image_state.VkHandle(), image_layout_map});
    }
    return image_layout_map;
}

void CommandBuffer::RecordCommand(const Location &loc) {
    command_count++;

    if (first_action_or_sync_command == Func::Empty) {
        const CommandValidationInfo &info = GetCommandValidationInfo(loc.function);
        if (info.action || info.synchronization) {
            first_action_or_sync_command = loc.function;
        }
    }
}

void CommandBuffer::RecordBeginQuery(const QueryObject &query_obj, const Location &loc) {
    active_queries.insert(query_obj);
    started_queries.insert(query_obj);

    updated_queries.insert(query_obj);
    if (query_obj.inside_render_pass) {
        render_pass_queries.insert(query_obj);
    }

    for (auto &item : sub_states_) {
        item.second->RecordBeginQuery(query_obj, loc);
    }
}

void CommandBuffer::RecordEndQuery(const QueryObject &query_obj, const Location &loc) {
    active_queries.erase(query_obj);
    updated_queries.insert(query_obj);
    if (query_obj.inside_render_pass) {
        render_pass_queries.erase(query_obj);
    }

    for (auto &item : sub_states_) {
        item.second->RecordEndQuery(query_obj, loc);
    }
}

bool CommandBuffer::UpdatesQuery(const QueryObject &query_obj) const {
    // Clear out the perf_pass from the caller because it isn't known when the command buffer is recorded.
    auto key = query_obj;
    key.perf_pass = 0;
    for (auto *sub_cb : linked_command_buffers) {
        if (sub_cb->updated_queries.find(key) != sub_cb->updated_queries.end()) {
            return true;
        }
    }
    return updated_queries.find(key) != updated_queries.end();
}

void CommandBuffer::RecordEndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    for (uint32_t slot = firstQuery; slot < (firstQuery + queryCount); slot++) {
        QueryObject query_obj = {queryPool, slot};
        active_queries.erase(query_obj);
        updated_queries.insert(query_obj);
    }

    for (auto &item : sub_states_) {
        item.second->RecordEndQueries(queryPool, firstQuery, queryCount);
    }
}

void CommandBuffer::RecordWriteTimestamp(VkQueryPool queryPool, uint32_t slot, const Location &loc) {
    RecordCommand(loc);
    if (dev_data.disabled[query_validation]) {
        return;
    }

    if (!dev_data.disabled[command_buffer_state]) {
        auto pool_state = dev_data.Get<vvl::QueryPool>(queryPool);
        AddChild(pool_state);
    }

    QueryObject query_obj = {queryPool, slot};
    for (auto &item : sub_states_) {
        item.second->RecordWriteTimestamp(query_obj, loc);
    }

    // Acts like an end query
    active_queries.erase(query_obj);
    updated_queries.insert(query_obj);
    if (query_obj.inside_render_pass) {
        render_pass_queries.erase(query_obj);
    }
}

void CommandBuffer::RecordResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const Location &loc) {
    RecordCommand(loc);
    if (dev_data.disabled[query_validation]) {
        return;
    }

    auto pool_state = dev_data.Get<QueryPool>(queryPool);
    ASSERT_AND_RETURN(pool_state);
    if (!dev_data.disabled[command_buffer_state]) {
        AddChild(pool_state);
    }

    for (uint32_t slot = firstQuery; slot < (firstQuery + queryCount); slot++) {
        QueryObject query_obj = {queryPool, slot};
        updated_queries.insert(query_obj);
    }

    const bool is_perf_query = pool_state->create_info.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    for (auto &item : sub_states_) {
        item.second->RecordResetQueryPool(queryPool, firstQuery, queryCount, is_perf_query, loc);
    }
}

void CommandBuffer::RecordCopyQueryPoolResults(VkQueryPool queryPool, VkBuffer dstBuffer, uint32_t firstQuery, uint32_t queryCount,
                                               VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                               const Location &loc) {
    RecordCommand(loc);
    if (dev_data.disabled[query_validation]) {
        return;
    }

    auto buffer_state = dev_data.Get<Buffer>(dstBuffer);
    auto pool_state = dev_data.Get<QueryPool>(queryPool);
    ASSERT_AND_RETURN(buffer_state && pool_state);
    if (!dev_data.disabled[command_buffer_state]) {
        AddChild(buffer_state);
        AddChild(pool_state);
    }

    for (auto &item : sub_states_) {
        item.second->RecordCopyQueryPoolResults(*pool_state, *buffer_state, firstQuery, queryCount, dstOffset, stride, flags, loc);
    }
}

void CommandBuffer::RecordWriteAccelerationStructuresProperties(VkQueryPool queryPool, uint32_t firstQuery,
                                                                uint32_t accelerationStructureCount, const Location &loc) {
    RecordCommand(loc);
    if (dev_data.disabled[query_validation]) {
        return;
    }

    if (!dev_data.disabled[command_buffer_state]) {
        auto pool_state = dev_data.Get<QueryPool>(queryPool);
        AddChild(pool_state);
    }

    for (auto &item : sub_states_) {
        item.second->RecordWriteAccelerationStructuresProperties(queryPool, firstQuery, accelerationStructureCount, loc);
    }

    // Same idea as RecordEndQueries
    for (uint32_t slot = firstQuery; slot < (firstQuery + accelerationStructureCount); slot++) {
        QueryObject query_obj = {queryPool, slot};
        active_queries.erase(query_obj);
        updated_queries.insert(query_obj);
    }
}

void CommandBuffer::UpdateSubpassAttachments() {
    ASSERT_AND_RETURN(active_render_pass);
    const auto &subpass = active_render_pass->create_info.pSubpasses[GetActiveSubpass()];
    assert(active_subpasses.size() == active_attachments.size());

    for (size_t i = 0; i < active_attachments.size(); ++i) {
        active_attachments[i].type = AttachmentInfo::Type::Empty;
    }

    for (uint32_t index = 0; index < subpass.inputAttachmentCount; ++index) {
        const uint32_t attachment_index = subpass.pInputAttachments[index].attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::Input;
            active_attachments[attachment_index].layout = subpass.pInputAttachments[index].layout;
            active_attachments[attachment_index].type_index = index;
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            active_subpasses[attachment_index].aspectMask = subpass.pInputAttachments[index].aspectMask;
        }
    }

    for (uint32_t index = 0; index < subpass.colorAttachmentCount; ++index) {
        const uint32_t attachment_index = subpass.pColorAttachments[index].attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::Color;
            active_attachments[attachment_index].layout = subpass.pColorAttachments[index].layout;
            active_attachments[attachment_index].type_index = index;
            active_color_attachments_index.insert(index);
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            active_subpasses[attachment_index].aspectMask = subpass.pColorAttachments[index].aspectMask;
        }
        if (subpass.pResolveAttachments) {
            const uint32_t attachment_index2 = subpass.pResolveAttachments[index].attachment;
            if (attachment_index2 != VK_ATTACHMENT_UNUSED) {
                active_attachments[attachment_index2].type = AttachmentInfo::Type::ColorResolve;
                active_attachments[attachment_index2].layout = subpass.pResolveAttachments[index].layout;
                active_attachments[attachment_index2].type_index = index;
                active_subpasses[attachment_index2].used = true;
                active_subpasses[attachment_index2].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                active_subpasses[attachment_index2].aspectMask = subpass.pResolveAttachments[index].aspectMask;
            }
        }
    }

    if (subpass.pDepthStencilAttachment) {
        const uint32_t attachment_index = subpass.pDepthStencilAttachment->attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::DepthStencil;
            active_attachments[attachment_index].layout = subpass.pDepthStencilAttachment->layout;
            // Look for potential dedicated stencil layout
            if (const auto *stencil_layout =
                    vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(subpass.pDepthStencilAttachment->pNext)) {
                active_attachments[attachment_index].separate_stencil_layout = stencil_layout->stencilLayout;
            }
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            active_subpasses[attachment_index].aspectMask = subpass.pDepthStencilAttachment->aspectMask;
        }
    }

    if (auto rdm_ci =
            vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(active_render_pass->create_info.pNext)) {
        const uint32_t attachment_index = rdm_ci->fragmentDensityMapAttachment.attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::FragmentDensityMap;
            active_attachments[attachment_index].layout = rdm_ci->fragmentDensityMapAttachment.layout;
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
            active_subpasses[attachment_index].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    if (auto rdr_attachment_ci = vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass.pNext)) {
        if (rdr_attachment_ci->pFragmentShadingRateAttachment) {
            const uint32_t attachment_index = rdr_attachment_ci->pFragmentShadingRateAttachment->attachment;
            if (attachment_index != VK_ATTACHMENT_UNUSED) {
                active_attachments[attachment_index].type = AttachmentInfo::Type::FragmentShadingRate;
                active_attachments[attachment_index].layout = rdr_attachment_ci->pFragmentShadingRateAttachment->layout;
                active_subpasses[attachment_index].used = true;
                active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
                active_subpasses[attachment_index].aspectMask = rdr_attachment_ci->pFragmentShadingRateAttachment->aspectMask;
            }
        }
    }
}

// For non Dynamic Renderpass we update the attachments
void CommandBuffer::UpdateAttachmentsView(const VkRenderPassBeginInfo *pRenderPassBegin) {
    const bool imageless = (active_framebuffer->create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0;
    const VkRenderPassAttachmentBeginInfo *attachment_info_struct = nullptr;
    if (pRenderPassBegin) attachment_info_struct = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);

    for (uint32_t i = 0; i < active_attachments.size(); ++i) {
        if (imageless) {
            if (attachment_info_struct && i < attachment_info_struct->attachmentCount) {
                active_attachments[i].image_view = dev_data.Get<vvl::ImageView>(attachment_info_struct->pAttachments[i]).get();
            }
        } else {
            active_attachments[i].image_view = active_framebuffer->attachments_view_state[i].get();
        }
    }

    // While updating the subpass we will set the active_attachments type
    UpdateSubpassAttachments();
}

void CommandBuffer::RecordBeginRenderPass(const VkRenderPassBeginInfo &render_pass_begin,
                                          const VkSubpassBeginInfo &subpass_begin_info, const Location &loc) {
    RecordCommand(loc);
    active_framebuffer = dev_data.Get<vvl::Framebuffer>(render_pass_begin.framebuffer);
    active_render_pass = dev_data.Get<vvl::RenderPass>(render_pass_begin.renderPass);
    render_area = render_pass_begin.renderArea;
    SetActiveSubpass(0);
    active_subpass_contents = subpass_begin_info.contents;
    render_pass_queries.clear();

    // Connect this RP to cmdBuffer
    if (!dev_data.disabled[command_buffer_state]) {
        AddChild(active_render_pass);
    }

    if (auto sample_locations_begin =
            vku::FindStructInPNextChain<VkRenderPassSampleLocationsBeginInfoEXT>(render_pass_begin.pNext)) {
        sample_locations_begin_info = sample_locations_begin;
    }

    if (auto rp_striped_begin = vku::FindStructInPNextChain<VkRenderPassStripeBeginInfoARM>(render_pass_begin.pNext)) {
        has_render_pass_striped = true;
        striped_count += rp_striped_begin->stripeInfoCount;
    }

    // Spec states that after BeginRenderPass all resources should be rebound
    if (active_render_pass->has_multiview_enabled) {
        UnbindResources();
    }

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(render_pass_begin.pNext);
    render_pass_device_mask = chained_device_group_struct ? chained_device_group_struct->deviceMask : initial_device_mask;

    attachment_source = AttachmentSource::RenderPass;
    active_subpasses.clear();
    active_attachments.clear();

    if (active_framebuffer) {
        active_subpasses.resize(active_framebuffer->create_info.attachmentCount);
        active_attachments.resize(active_framebuffer->create_info.attachmentCount);
        UpdateAttachmentsView(&render_pass_begin);

        // Connect this framebuffer and its children to this cmdBuffer
        AddChild(active_framebuffer);
    }

    for (auto &item : sub_states_) {
        item.second->RecordBeginRenderPass(render_pass_begin, subpass_begin_info, loc);
    }
}

void CommandBuffer::RecordNextSubpass(const VkSubpassBeginInfo &subpass_begin_info, const VkSubpassEndInfo *subpass_end_info,
                                      const Location &loc) {
    RecordCommand(loc);
    SetActiveSubpass(GetActiveSubpass() + 1);
    active_subpass_contents = subpass_begin_info.contents;
    ASSERT_AND_RETURN(active_render_pass);

    if (active_framebuffer) {
        active_subpasses.clear();
        active_subpasses.resize(active_framebuffer->create_info.attachmentCount);

        if (GetActiveSubpass() < active_render_pass->create_info.subpassCount) {
            UpdateSubpassAttachments();
        }
    }

    // Spec states that after NextSubpass all resources should be rebound
    if (active_render_pass->has_multiview_enabled) {
        UnbindResources();
    }

    for (auto &item : sub_states_) {
        item.second->RecordNextSubpass(subpass_begin_info, subpass_end_info, loc);
    }
}

void CommandBuffer::RecordEndRenderPass(const VkSubpassEndInfo *subpass_end_info, const Location &loc) {
    // Call first so SubState can use render pass object before we destroy it
    for (auto &item : sub_states_) {
        item.second->RecordEndRenderPass(subpass_end_info, loc);
    }

    RecordCommand(loc);
    active_render_pass = nullptr;
    attachment_source = AttachmentSource::Empty;
    active_attachments.clear();
    active_subpasses.clear();
    active_color_attachments_index.clear();
    SetActiveSubpass(0);
    active_framebuffer = VK_NULL_HANDLE;
    sample_locations_begin_info = {};
}

static void InitDefaultRenderingAttachments(CommandBuffer::RenderingAttachment &attachments, uint32_t count) {
    attachments.color_locations.resize(count);
    attachments.color_indexes.resize(count);
    attachments.depth_index = nullptr;
    attachments.stencil_index = nullptr;
    attachments.set_color_locations = false;
    attachments.set_color_indexes = false;
    for (uint32_t i = 0; i < count; i++) {
        // Default from spec
        attachments.color_locations[i] = i;
        attachments.color_indexes[i] = i;
    }
}

void CommandBuffer::RecordBeginRendering(const VkRenderingInfo &rendering_info, const Location &loc) {
    RecordCommand(loc);
    active_render_pass = std::make_shared<vvl::RenderPass>(&rendering_info, true);
    render_area = rendering_info.renderArea;
    render_pass_queries.clear();

    InitDefaultRenderingAttachments(rendering_attachments, rendering_info.colorAttachmentCount);

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(rendering_info.pNext);
    render_pass_device_mask = chained_device_group_struct ? chained_device_group_struct->deviceMask : initial_device_mask;

    auto rp_striped_begin = vku::FindStructInPNextChain<VkRenderPassStripeBeginInfoARM>(rendering_info.pNext);
    if (rp_striped_begin) {
        has_render_pass_striped = true;
        striped_count += rp_striped_begin->stripeInfoCount;
    }

    active_subpass_contents = ((rendering_info.flags & VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT)
                                   ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                   : VK_SUBPASS_CONTENTS_INLINE);

    // Track if the first render pass instance does resume
    if (!has_render_pass_instance && (rendering_info.flags & VK_RENDERING_RESUMING_BIT)) {
        resumes_render_pass_instance = true;
    }
    // Track the last suspension state. Notice that both RESUMING/SUSPENDING flags can be specified.
    // The ordering is that suspension action goes after resuming action.
    if (rendering_info.flags & VK_RENDERING_RESUMING_BIT) {
        last_suspend_state = SuspendState::Resumed;
    }
    if (rendering_info.flags & VK_RENDERING_SUSPENDING_BIT) {
        last_suspend_state = SuspendState::Suspended;
    }

    has_render_pass_instance = true;

    attachment_source = AttachmentSource::DynamicRendering;
    active_attachments.clear();
    // add 2 for the Depth and Stencil
    // multiple by 2 because every attachment might have a resolve
    // add 1 for FragmentDensityMap (doesn't need a resolve)
    uint32_t attachment_count = ((rendering_info.colorAttachmentCount + 2) * 2) + 1;

    // Currently reserve the maximum possible size for |active_attachments| so when looping, we NEED to check for null
    active_attachments.resize(attachment_count);

    for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
        const auto &rendering_attachment = rendering_info.pColorAttachments[i];
        if (rendering_attachment.imageView != VK_NULL_HANDLE) {
            auto &color_attachment = active_attachments[GetDynamicRenderingColorAttachmentIndex(i)];
            color_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_attachment.imageView).get();
            color_attachment.type = AttachmentInfo::Type::Color;
            color_attachment.layout = rendering_attachment.imageLayout;
            color_attachment.type_index = i;
            active_color_attachments_index.insert(i);
            if (color_attachment.image_view) {
                TrackImageViewFirstLayout(*color_attachment.image_view, rendering_attachment.imageLayout,
                                          "VUID-vkCmdBeginRendering-pRenderingInfo-09592");
            }
            if (rendering_attachment.resolveMode != VK_RESOLVE_MODE_NONE &&
                rendering_attachment.resolveImageView != VK_NULL_HANDLE) {
                auto &resolve_attachment = active_attachments[GetDynamicRenderingColorResolveAttachmentIndex(i)];
                resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_attachment.resolveImageView).get();
                resolve_attachment.type = AttachmentInfo::Type::ColorResolve;
                resolve_attachment.layout = rendering_attachment.resolveImageLayout;
                resolve_attachment.type_index = i;
            }
        }
    }

    if (rendering_info.pDepthAttachment && rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
        auto &depth_attachment = active_attachments[GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::Depth)];
        depth_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView).get();
        depth_attachment.type = AttachmentInfo::Type::Depth;
        depth_attachment.layout = rendering_info.pDepthAttachment->imageLayout;
        if (depth_attachment.image_view) {
            TrackImageViewFirstLayout(*depth_attachment.image_view, rendering_info.pDepthAttachment->imageLayout,
                                      "VUID-vkCmdBeginRendering-pRenderingInfo-09588");
        }
        if (rendering_info.pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE &&
            rendering_info.pDepthAttachment->resolveImageView != VK_NULL_HANDLE) {
            auto &resolve_attachment = active_attachments[GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::DepthResolve)];
            resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_info.pDepthAttachment->resolveImageView).get();
            resolve_attachment.type = AttachmentInfo::Type::DepthResolve;
            resolve_attachment.layout = rendering_info.pDepthAttachment->resolveImageLayout;
        }
    }

    if (rendering_info.pStencilAttachment && rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
        auto &stencil_attachment = active_attachments[GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::Stencil)];
        stencil_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView).get();
        stencil_attachment.type = AttachmentInfo::Type::Stencil;
        stencil_attachment.layout = rendering_info.pStencilAttachment->imageLayout;
        if (stencil_attachment.image_view) {
            TrackImageViewFirstLayout(*stencil_attachment.image_view, rendering_info.pStencilAttachment->imageLayout,
                                      "VUID-vkCmdBeginRendering-pRenderingInfo-09590");
        }
        if (rendering_info.pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE &&
            rendering_info.pStencilAttachment->resolveImageView != VK_NULL_HANDLE) {
            auto &resolve_attachment = active_attachments[GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::StencilResolve)];
            resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(rendering_info.pStencilAttachment->resolveImageView).get();
            resolve_attachment.type = AttachmentInfo::Type::StencilResolve;
            resolve_attachment.layout = rendering_info.pStencilAttachment->resolveImageLayout;
        }
    }

    if (auto fragment_density_map_info =
            vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(rendering_info.pNext)) {
        auto &fdm_attachment = active_attachments[GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::FragmentDensityMap)];
        fdm_attachment.image_view = dev_data.Get<vvl::ImageView>(fragment_density_map_info->imageView).get();
        fdm_attachment.type = AttachmentInfo::Type::FragmentDensityMap;
        fdm_attachment.layout = fragment_density_map_info->imageLayout;
    }

    for (auto &item : sub_states_) {
        item.second->RecordBeginRendering(rendering_info, loc);
    }
}

void CommandBuffer::RecordEndRendering(const VkRenderingEndInfoEXT *pRenderingEndInfo, const Location &loc) {
    // Call first so SubState can use render pass object before we destroy it
    for (auto &item : sub_states_) {
        item.second->RecordEndRendering(pRenderingEndInfo);
    }

    RecordCommand(loc);
    active_render_pass = nullptr;
    active_color_attachments_index.clear();
}

void CommandBuffer::RecordBeginVideoCoding(const VkVideoBeginCodingInfoKHR &begin_info, const Location &loc) {
    RecordCommand(loc);
    bound_video_session = dev_data.Get<vvl::VideoSession>(begin_info.videoSession);
    ASSERT_AND_RETURN(bound_video_session);
    bound_video_session_parameters = dev_data.Get<vvl::VideoSessionParameters>(begin_info.videoSessionParameters);

    // Connect this video session to cmdBuffer
    if (!dev_data.disabled[command_buffer_state]) {
        AddChild(bound_video_session);
    }

    if (bound_video_session_parameters) {
        // Connect this video session parameters object to cmdBuffer
        if (!dev_data.disabled[command_buffer_state]) {
            AddChild(bound_video_session_parameters);
        }
    }

    // Need to record substate first
    for (auto &item : sub_states_) {
        item.second->RecordBeginVideoCoding(*bound_video_session, begin_info, loc);
    }

    if (bound_video_session->IsEncode()) {
        video_encode_rate_control_state = VideoEncodeRateControlState(bound_video_session->GetCodecOp(), &begin_info);
        video_encode_quality_level.reset();
    }

    if (begin_info.referenceSlotCount > 0) {
        size_t deactivated_slot_count = 0;

        for (uint32_t i = 0; i < begin_info.referenceSlotCount; ++i) {
            // Initialize the set of bound video picture resources
            if (begin_info.pReferenceSlots[i].pPictureResource != nullptr) {
                int32_t slot_index = begin_info.pReferenceSlots[i].slotIndex;
                vvl::VideoPictureResource res(dev_data, *begin_info.pReferenceSlots[i].pPictureResource);
                bound_video_picture_resources.emplace(std::make_pair(res, slot_index));
            }

            if (begin_info.pReferenceSlots[i].slotIndex >= 0 && begin_info.pReferenceSlots[i].pPictureResource == nullptr) {
                deactivated_slot_count++;
            }
        }

        if (deactivated_slot_count > 0) {
            std::vector<int32_t> deactivated_slots{};
            deactivated_slots.reserve(deactivated_slot_count);
            for (uint32_t i = 0; i < begin_info.referenceSlotCount; ++i) {
                if (begin_info.pReferenceSlots[i].slotIndex >= 0 && begin_info.pReferenceSlots[i].pPictureResource == nullptr) {
                    deactivated_slots.emplace_back(begin_info.pReferenceSlots[i].slotIndex);
                }
            }

            // Enqueue submission time DPB slot deactivation
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [deactivated_slots](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    for (const auto &slot_index : deactivated_slots) {
                        dev_state.Deactivate(slot_index);
                    }
                    return false;
                });
        }
    }
}

void CommandBuffer::RecordEndVideoCoding(const Location &loc) {
    RecordCommand(loc);
    bound_video_session = nullptr;
    bound_video_session_parameters = nullptr;
    bound_video_picture_resources.clear();
    video_encode_quality_level.reset();
}

void CommandBuffer::RecordControlVideoCoding(const VkVideoCodingControlInfoKHR &control_info, const Location &loc) {
    RecordCommand(loc);
    if (!bound_video_session) {
        return;
    }

    // Need to record substate first
    for (auto &item : sub_states_) {
        item.second->RecordControlVideoCoding(*bound_video_session, control_info, loc);
    }

    if (control_info.flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) {
        // Remove DPB slot index association for bound video picture resources
        for (auto &binding : bound_video_picture_resources) {
            binding.second = -1;
        }

        // Enqueue submission time video session state reset/initialization
        video_session_updates[bound_video_session->VkHandle()].emplace_back(
            [](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                dev_state.Reset();
                return false;
            });
    }

    if (bound_video_session->IsEncode() && control_info.flags & VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR) {
        auto state = VideoEncodeRateControlState(bound_video_session->GetCodecOp(), &control_info);
        if (state) {
            video_encode_rate_control_state = state;

            // Enqueue rate control specific device state changes
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [state](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    dev_state.SetRateControlState(state);
                    return false;
                });
        }
    }

    if (bound_video_session->IsEncode() && control_info.flags & VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR) {
        auto quality_level_info = vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(control_info.pNext);
        if (quality_level_info != nullptr) {
            uint32_t quality_level = quality_level_info->qualityLevel;
            video_encode_quality_level = quality_level;

            // Enqueue encode quality level device state change
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [quality_level](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    dev_state.SetEncodeQualityLevel(quality_level);
                    return false;
                });
        }
    }
}

void vvl::CommandBuffer::RecordVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info) {
    for (auto &item : sub_states_) {
        item.second->RecordVideoInlineQueries(query_info);
    }

    for (uint32_t i = 0; i < query_info.queryCount; i++) {
        updated_queries.insert(QueryObject(query_info.queryPool, query_info.firstQuery + i));
    }
}

void CommandBuffer::RecordDecodeVideo(const VkVideoDecodeInfoKHR &decode_info, const Location &loc) {
    RecordCommand(loc);
    if (!bound_video_session) {
        return;
    }

    // Need to record substate first
    for (auto &item : sub_states_) {
        item.second->RecordDecodeVideo(*bound_video_session, decode_info, loc);
    }

    if (decode_info.pSetupReferenceSlot && decode_info.pSetupReferenceSlot->pPictureResource) {
        vvl::VideoReferenceSlot setup_slot(dev_data, *bound_video_session->profile, *decode_info.pSetupReferenceSlot);

        // Update bound video picture resource DPB slot index association
        bound_video_picture_resources[setup_slot.resource] = setup_slot.index;

        // Enqueue submission time reference slot setup or invalidation
        bool reference_setup_requested = bound_video_session->ReferenceSetupRequested(decode_info);
        video_session_updates[bound_video_session->VkHandle()].emplace_back(
            [setup_slot, reference_setup_requested](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                                                    bool do_validate) {
                if (reference_setup_requested) {
                    dev_state.Activate(setup_slot.index, setup_slot.picture_id, setup_slot.resource);
                } else {
                    dev_state.Invalidate(setup_slot.index, setup_slot.picture_id);
                }
                return false;
            });
    }

    // Update active query indices
    for (auto &query : active_queries) {
        uint32_t op_count = bound_video_session->GetVideoDecodeOperationCount(&decode_info);
        query.active_query_index += op_count;
    }

    // Update inline queries
    if (bound_video_session->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(decode_info.pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            RecordVideoInlineQueries(*inline_query_info);
        }
    }
}

void vvl::CommandBuffer::RecordEncodeVideo(const VkVideoEncodeInfoKHR &encode_info, const Location &loc) {
    RecordCommand(loc);
    if (!bound_video_session) {
        return;
    }

    // Need to record substate first
    for (auto &item : sub_states_) {
        item.second->RecordEncodeVideo(*bound_video_session, encode_info, loc);
    }

    if (encode_info.pSetupReferenceSlot && encode_info.pSetupReferenceSlot->pPictureResource) {
        vvl::VideoReferenceSlot setup_slot(dev_data, *bound_video_session->profile, *encode_info.pSetupReferenceSlot);

        // Update bound video picture resource DPB slot index association
        bound_video_picture_resources[setup_slot.resource] = setup_slot.index;

        // Enqueue submission time reference slot setup or invalidation
        bool reference_setup_requested = bound_video_session->ReferenceSetupRequested(encode_info);
        video_session_updates[bound_video_session->VkHandle()].emplace_back(
            [setup_slot, reference_setup_requested](const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                                                    bool do_validate) {
                if (reference_setup_requested) {
                    dev_state.Activate(setup_slot.index, setup_slot.picture_id, setup_slot.resource);
                } else {
                    dev_state.Invalidate(setup_slot.index, setup_slot.picture_id);
                }
                return false;
            });
    }

    // Update active query indices
    for (auto &query : active_queries) {
        uint32_t op_count = bound_video_session->GetVideoEncodeOperationCount(&encode_info);
        query.active_query_index += op_count;
    }

    // Update inline queries
    if (bound_video_session->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(encode_info.pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            RecordVideoInlineQueries(*inline_query_info);
        }
    }
}

static void SetRenderingAttachmentLocations(CommandBuffer::RenderingAttachment &attachments, const VkRenderingAttachmentLocationInfo *pLocationInfo) {
    attachments.color_locations.resize(pLocationInfo->colorAttachmentCount);
    const uint32_t *locations = pLocationInfo->pColorAttachmentLocations;
    for (uint32_t i = 0; i < pLocationInfo->colorAttachmentCount; ++i) {
        attachments.color_locations[i] = locations ? locations[i] : i;
    }
}

static void SetRenderingInputAttachmentIndices(CommandBuffer::RenderingAttachment &attachments, const VkRenderingInputAttachmentIndexInfo *pLocationInfo) {
    attachments.color_indexes.resize(pLocationInfo->colorAttachmentCount);
    const uint32_t *indexes = pLocationInfo->pColorAttachmentInputIndices;
    for (uint32_t i = 0; i < pLocationInfo->colorAttachmentCount; ++i) {
        attachments.color_indexes[i] = indexes ? indexes[i] : i;
    }
    if (pLocationInfo->pDepthInputAttachmentIndex) {
        attachments.depth_index_storage = *pLocationInfo->pDepthInputAttachmentIndex;
        attachments.depth_index = &attachments.depth_index_storage;
    } else {
        attachments.depth_index = nullptr;
    }
    if (pLocationInfo->pStencilInputAttachmentIndex) {
        attachments.stencil_index_storage = *pLocationInfo->pStencilInputAttachmentIndex;
        attachments.stencil_index = &attachments.stencil_index_storage;
    } else {
        attachments.stencil_index = nullptr;
    }
}

void CommandBuffer::Begin(const VkCommandBufferBeginInfo *pBeginInfo) {
    if (IsRecorded(state)) {
        Location loc(Func::vkBeginCommandBuffer);
        Reset(loc);
    }

    // Set updated state here in case implicit reset occurs above
    state = CbState::Recording;
    ASSERT_AND_RETURN(pBeginInfo);

    begin_info_flags = pBeginInfo->flags;

    if (pBeginInfo->pInheritanceInfo && IsSecondary()) {
        // pInheritanceInfo could be valid, but ignored, if in a primary command buffer
        has_inheritance = true;
        inheritance_info.initialize(pBeginInfo->pInheritanceInfo);

        // If we are a secondary command-buffer and inheriting.  Update the items we should inherit.
        if (begin_info_flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
            if (inheritance_info.renderPass) {
                active_render_pass = dev_data.Get<vvl::RenderPass>(inheritance_info.renderPass);
                SetActiveSubpass(inheritance_info.subpass);

                if (inheritance_info.framebuffer) {
                    active_framebuffer = dev_data.Get<vvl::Framebuffer>(inheritance_info.framebuffer);
                    attachment_source = AttachmentSource::Inheritance;
                    active_subpasses.clear();
                    active_attachments.clear();

                    if (active_framebuffer) {
                        active_subpasses.resize(active_framebuffer->create_info.attachmentCount);
                        active_attachments.resize(active_framebuffer->create_info.attachmentCount);
                        UpdateAttachmentsView(nullptr);

                        // Connect this framebuffer and its children to this cmdBuffer
                        if (!dev_data.disabled[command_buffer_state]) {
                            AddChild(active_framebuffer);
                        }
                    }
                }
            } else {
                auto inheritance_rendering_info =
                    vku::FindStructInPNextChain<VkCommandBufferInheritanceRenderingInfo>(pBeginInfo->pInheritanceInfo->pNext);
                if (inheritance_rendering_info) {
                    active_render_pass = std::make_shared<vvl::RenderPass>(inheritance_rendering_info);

                    InitDefaultRenderingAttachments(rendering_attachments, inheritance_rendering_info->colorAttachmentCount);
                    if (auto locations = vku::FindStructInPNextChain<VkRenderingAttachmentLocationInfo>(inheritance_rendering_info->pNext)) {
                        SetRenderingAttachmentLocations(rendering_attachments, locations);
                    }
                    if (auto indexes = vku::FindStructInPNextChain<VkRenderingInputAttachmentIndexInfo>(inheritance_rendering_info->pNext)) {
                        SetRenderingInputAttachmentIndices(rendering_attachments, indexes);
                    }
                }
            }
        }
    }

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        initial_device_mask = chained_device_group_struct->deviceMask;
    } else {
        initial_device_mask = (1 << dev_data.physical_device_count) - 1;
    }
    performance_lock_acquired = dev_data.performance_lock_acquired;
    updated_queries.clear();

    for (auto &item : sub_states_) {
        item.second->Begin(*pBeginInfo);
    }
}

void CommandBuffer::End(VkResult result) {
    if (result == VK_SUCCESS) {
        state = CbState::Recorded;
    }
    for (auto &item : sub_states_) {
        item.second->End();
    }
}

void CommandBuffer::RecordExecuteCommands(vvl::span<const VkCommandBuffer> secondary_command_buffers, const Location &loc) {
    RecordCommand(loc);
    uint32_t cmd_index = 0;
    for (const VkCommandBuffer sub_command_buffer : secondary_command_buffers) {
        auto secondary_cb_state = dev_data.GetWrite<CommandBuffer>(sub_command_buffer);
        ASSERT_AND_RETURN(secondary_cb_state);
        if (!(secondary_cb_state->begin_info_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (begin_info_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // TODO: Because this is a state change, clearing the VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT needs to be moved
                // from the validation step to the recording step
                begin_info_flags &= ~VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            }
        }

        // Propagate inital layout and current layout state to the primary cmd buffer
        // NOTE: The update/population of the image_layout_map is done in CoreChecks, but for other classes derived from
        // Device these maps will be empty, so leaving the propagation in the the state tracker should be a no-op
        // for those other classes.
        for (const auto &[image, secondary_cb_layout_map] : secondary_cb_state->image_layout_registry) {
            const auto image_state = dev_data.Get<vvl::Image>(image);
            if (!image_state || image_state->Destroyed() || !secondary_cb_layout_map ||
                image_state->GetId() != secondary_cb_layout_map->image_id) {
                continue;
            }
            if (auto cb_layout_map = GetOrCreateImageLayoutMap(*image_state)) {
                struct Updater {
                    void update(ImageLayoutState &dst, const ImageLayoutState &src) const {
                        if (src.current_layout != kInvalidLayout && src.current_layout != dst.current_layout) {
                            dst.current_layout = src.current_layout;
                        }
                    }
                    std::optional<ImageLayoutState> insert(const ImageLayoutState &src) const {
                        return std::optional<ImageLayoutState>(vvl::in_place, src);
                    }
                };
                sparse_container::splice(*cb_layout_map, *secondary_cb_layout_map, Updater());
            }
        }

        secondary_cb_state->primary_command_buffer = VkHandle();
        linked_command_buffers.insert(secondary_cb_state.get());
        AddChild(secondary_cb_state);

        for (auto &event : secondary_cb_state->events) {
            events.push_back(event);
        }

        if (first_action_or_sync_command == Func::Empty) {
            first_action_or_sync_command = secondary_cb_state->first_action_or_sync_command;
        }

        // Handle secondary command buffer updates for dynamic rendering.
        if (!has_render_pass_instance) {
            resumes_render_pass_instance = secondary_cb_state->resumes_render_pass_instance;
        }
        if (secondary_cb_state->last_suspend_state != SuspendState::Empty) {
            last_suspend_state = secondary_cb_state->last_suspend_state;
        }
        has_render_pass_instance |= secondary_cb_state->has_render_pass_instance;

        // Handle debug labels
        label_stack_depth_ += secondary_cb_state->label_stack_depth_;
        label_commands_.insert(label_commands_.end(), secondary_cb_state->label_commands_.begin(),
                               secondary_cb_state->label_commands_.end());

        for (auto &item : sub_states_) {
            item.second->RecordExecuteCommand(*secondary_cb_state, cmd_index, loc);
        }

        cmd_index++;
    }
}

void CommandBuffer::PushDescriptorSetState(VkPipelineBindPoint pipelineBindPoint,
                                           std::shared_ptr<const vvl::PipelineLayout> pipeline_layout, uint32_t set,
                                           uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                           const Location &loc) {
    // Short circuit invalid updates
    if ((set >= pipeline_layout->set_layouts.list.size()) || !pipeline_layout->set_layouts.list[set] ||
        !pipeline_layout->set_layouts.list[set]->IsPushDescriptor()) {
        return;
    }

    // We need a descriptor set to update the bindings with, compatible with the passed layout
    const auto &dsl = pipeline_layout->set_layouts.list[set];
    auto &last_bound = lastBound[ConvertToVvlBindPoint(pipelineBindPoint)];
    auto &push_descriptor_set = last_bound.push_descriptor_set;
    // If we are disturbing the current push_desriptor_set clear it
    if (!push_descriptor_set || !last_bound.IsBoundSetCompatible(set, *pipeline_layout)) {
        last_bound.UnbindAndResetPushDescriptorSet(dev_data.CreatePushDescriptorSet(dsl));
    }

    UpdateLastBoundDescriptorSets(pipelineBindPoint, pipeline_layout, set, 1, nullptr, push_descriptor_set, 0, nullptr, loc);

    // Now that we have either the new or extant push_descriptor set ... do the write updates against it
    push_descriptor_set->PerformPushDescriptorsUpdate(descriptorWriteCount, pDescriptorWrites);
}

// Generic function to handle state update for all CmdDraw* type functions
void CommandBuffer::RecordDraw(const Location &loc) {
    RecordCommand(loc);
    LastBound &last_bound = lastBound[vvl::BindPointGraphics];
    for (auto &item : sub_states_) {
        item.second->RecordActionCommand(last_bound, loc);
    }
}

// Generic function to handle state update for all CmdDispatch* type functions
void CommandBuffer::RecordDispatch(const Location &loc) {
    RecordCommand(loc);
    LastBound &last_bound = lastBound[vvl::BindPointCompute];
    for (auto &item : sub_states_) {
        item.second->RecordActionCommand(last_bound, loc);
    }
}

// Generic function to handle state update for all CmdTraceRay* type functions
void CommandBuffer::RecordTraceRay(const Location &loc) {
    RecordCommand(loc);
    LastBound &last_bound = lastBound[vvl::BindPointRayTracing];
    for (auto &item : sub_states_) {
        item.second->RecordActionCommand(last_bound, loc);
    }
}

void CommandBuffer::RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline &pipeline) {
    BindLastBoundPipeline(ConvertToVvlBindPoint(bind_point), &pipeline);

    for (auto &item : sub_states_) {
        item.second->RecordBindPipeline(bind_point, pipeline);
    }

    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        dynamic_state_status.pipeline.reset();

        // Make a copy and then xor the new change
        // This gives us which state has been invalidated, allows us to save time for most cases where nothing changes
        CBDynamicFlags invalidated_state = dynamic_state_status.cb;

        // Spec: "[dynamic state] made invalid by another pipeline bind with that state specified as static"
        // So unset the bitmask for the command buffer lifetime tracking (unless ignored, keep set)
        dynamic_state_status.cb &= (pipeline.dynamic_state | pipeline.ignored_dynamic_state);

        invalidated_state ^= dynamic_state_status.cb;
        if (invalidated_state.any()) {
            // Reset dynamic state values
            dynamic_state_value.reset(invalidated_state);

            for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                CBDynamicState status = static_cast<CBDynamicState>(index);
                if (invalidated_state[status]) {
                    invalidated_state_pipe[index] = pipeline.VkHandle();
                }
            }
        }

        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
            !pipeline.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE) && pipeline.vertex_input_state) {
            for (const auto &[binding_index, binding_state] : pipeline.vertex_input_state->bindings) {
                current_vertex_buffer_binding_info[binding_index].stride = binding_state.desc.stride;
            }
        }

        if (!dev_data.enabled_features.variableMultisampleRate) {
            if (const auto *multisample_state = pipeline.MultisampleState(); multisample_state) {
                if (const auto &render_pass = active_render_pass) {
                    const uint32_t subpass = GetActiveSubpass();
                    // if render pass uses no attachment, all bound pipelines in the same subpass must have the same
                    // pMultisampleState->rasterizationSamples. To check that, record pMultisampleState->rasterizationSamples of the
                    // first bound pipeline.
                    if (render_pass->UsesNoAttachment(subpass)) {
                        if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                                GetActiveSubpassRasterizationSampleCount();
                            !subpass_rasterization_samples) {
                            SetActiveSubpassRasterizationSampleCount(multisample_state->rasterizationSamples);
                        }
                    }
                }
            }
        }

    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        dynamic_state_status.rtx_stack_size_pipeline = false;
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
            dynamic_state_status.rtx_stack_size_cb = false;  // invalidated
        }
    }

    dirty_static_state = false;
}

// Helper for descriptor set (and buffer) updates.
static bool PushDescriptorCleanup(LastBound &last_bound, uint32_t set_idx) {
    // All uses are from loops over ds_slots, but just in case..
    assert(set_idx < last_bound.ds_slots.size());

    auto descriptor_set = last_bound.ds_slots[set_idx].ds_state.get();
    if (descriptor_set && descriptor_set->IsPushDescriptor()) {
        assert(descriptor_set == last_bound.push_descriptor_set.get());
        last_bound.push_descriptor_set = nullptr;
        return true;
    }
    return true;
}

// Update pipeline_layout bind points applying the "Pipeline Layout Compatibility" rules.
// One of pDescriptorSets or push_descriptor_set should be nullptr, indicating whether this
// is called for CmdBindDescriptorSets or CmdPushDescriptorSet.
void CommandBuffer::UpdateLastBoundDescriptorSets(VkPipelineBindPoint pipeline_bind_point,
                                                  std::shared_ptr<const vvl::PipelineLayout> pipeline_layout, uint32_t first_set,
                                                  uint32_t set_count, const VkDescriptorSet *pDescriptorSets,
                                                  std::shared_ptr<vvl::DescriptorSet> &push_descriptor_set,
                                                  uint32_t dynamic_offset_count, const uint32_t *p_dynamic_offsets,
                                                  const Location &loc) {
    ASSERT_AND_RETURN((pDescriptorSets == nullptr) ^ (push_descriptor_set == nullptr));

    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    ASSERT_AND_RETURN(last_binding_index < pipeline_layout->set_compat_ids.size());

    auto &last_bound = lastBound[ConvertToVvlBindPoint(pipeline_bind_point)];
    last_bound.desc_set_pipeline_layout = pipeline_layout;
    last_bound.desc_set_bound_command = loc.function;
    last_bound.SetDescriptorMode(DescriptorModeClassic);
    auto &pipe_compat_ids = pipeline_layout->set_compat_ids;
    // Resize binding arrays
    if (last_binding_index >= last_bound.ds_slots.size()) {
        last_bound.ds_slots.resize(required_size);
    }
    const uint32_t current_size = static_cast<uint32_t>(last_bound.ds_slots.size());

    // Clean up the "disturbed" before and after the range to be set
    if (required_size < current_size) {
        if (last_bound.ds_slots[last_binding_index].compat_id_for_set != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (PushDescriptorCleanup(last_bound, set_idx)) {
                    break;
                }
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        last_bound.ds_slots.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        auto &ds_slot = last_bound.ds_slots[set_idx];
        if (ds_slot.compat_id_for_set != pipe_compat_ids[set_idx]) {
            PushDescriptorCleanup(last_bound, set_idx);
            ds_slot.Reset();
            ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];
        }
    }

    // Now update the bound sets with the input sets
    const uint32_t *input_dynamic_offsets = p_dynamic_offsets;  // "read" pointer for dynamic offset data
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        auto &ds_slot = last_bound.ds_slots[set_idx];
        auto descriptor_set =
            push_descriptor_set ? push_descriptor_set : dev_data.Get<vvl::DescriptorSet>(pDescriptorSets[input_idx]);

        ds_slot.Reset();
        // Record binding (or push)
        if (descriptor_set != last_bound.push_descriptor_set) {
            // Only cleanup the push descriptors if they aren't the currently used set.
            PushDescriptorCleanup(last_bound, set_idx);
        }
        ds_slot.ds_state = descriptor_set;
        ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index

        if (descriptor_set) {
            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            // TODO: Add logic for tracking push_descriptor offsets (here or in caller)
            if (set_dynamic_descriptor_count && input_dynamic_offsets) {
                const uint32_t *end_offset = input_dynamic_offsets + set_dynamic_descriptor_count;
                ds_slot.dynamic_offsets = std::vector<uint32_t>(input_dynamic_offsets, end_offset);
                input_dynamic_offsets = end_offset;
                assert(input_dynamic_offsets <= (p_dynamic_offsets + dynamic_offset_count));
            } else {
                ds_slot.dynamic_offsets.clear();
            }
        }
    }

    for (auto &item : sub_states_) {
        item.second->UpdateLastBoundDescriptorSets(pipeline_bind_point, loc);
    }
}

void CommandBuffer::UpdateLastBoundDescriptorBuffers(VkPipelineBindPoint pipeline_bind_point,
                                                     std::shared_ptr<const vvl::PipelineLayout> pipeline_layout, uint32_t first_set,
                                                     uint32_t set_count, const uint32_t *buffer_indicies,
                                                     const VkDeviceSize *buffer_offsets) {
    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    assert(last_binding_index < pipeline_layout->set_compat_ids.size());

    const vvl::BindPoint vvl_bind_point = ConvertToVvlBindPoint(pipeline_bind_point);
    auto &last_bound = lastBound[vvl_bind_point];
    last_bound.desc_set_pipeline_layout = pipeline_layout;
    auto &pipe_compat_ids = pipeline_layout->set_compat_ids;
    // Resize binding arrays
    if (last_binding_index >= last_bound.ds_slots.size()) {
        last_bound.ds_slots.resize(required_size);
    }
    const uint32_t current_size = static_cast<uint32_t>(last_bound.ds_slots.size());

    // Clean up the "disturbed" before and after the range to be set
    if (required_size < current_size) {
        if (last_bound.ds_slots[last_binding_index].compat_id_for_set != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (PushDescriptorCleanup(last_bound, set_idx)) {
                    break;
                }
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        last_bound.ds_slots.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        PushDescriptorCleanup(last_bound, set_idx);
        last_bound.ds_slots[set_idx].Reset();
    }

    // Now update the bound sets with the input sets
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        auto &ds_slot = last_bound.ds_slots[set_idx];
        ds_slot.Reset();

        // Record binding
        ds_slot.descriptor_buffer_binding = {buffer_indicies[input_idx], buffer_offsets[input_idx]};
        ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index
    }
}

// Set image layout for given subresource range
void CommandBuffer::SetImageLayout(const vvl::Image &image_state, const VkImageSubresourceRange &normalized_subresource_range,
                                   VkImageLayout layout, VkImageLayout expected_layout) {
    if (auto image_layout_map = GetOrCreateImageLayoutMap(image_state)) {
        if (image_state.subresource_encoder.InRange(normalized_subresource_range)) {
            RangeGenerator range_gen(image_state.subresource_encoder, normalized_subresource_range);
            if (UpdateCurrentLayout(*image_layout_map, std::move(range_gen), layout, expected_layout,
                                    normalized_subresource_range.aspectMask)) {
                image_layout_change_count++;  // Change the version of this data to force revalidation
            }
        }
    }
}

void CommandBuffer::TrackImageViewFirstLayout(const vvl::ImageView &view_state, VkImageLayout layout,
                                              const char *submit_time_layout_mismatch_vuid) {
    if (auto image_layout_map = GetOrCreateImageLayoutMap(*view_state.image_state.get())) {
        RangeGenerator range_gen(view_state.range_generator);
        TrackFirstLayout(*image_layout_map, std::move(range_gen), layout, view_state.normalized_subresource_range.aspectMask,
                         submit_time_layout_mismatch_vuid);
    }
}

void CommandBuffer::TrackImageFirstLayout(const vvl::Image &image_state, const VkImageSubresourceRange &subresource_range,
                                          int32_t depth_offset, uint32_t depth_extent, VkImageLayout layout) {
    if (auto image_layout_map = GetOrCreateImageLayoutMap(image_state)) {
        VkImageSubresourceRange normalized_subresource_range = image_state.NormalizeSubresourceRange(subresource_range);

        if (depth_extent != 0 && CanTransitionDepthSlices(dev_data.extensions, image_state.create_info)) {
            normalized_subresource_range.baseArrayLayer = (uint32_t)depth_offset;
            normalized_subresource_range.layerCount = depth_extent;
        }
        if (image_state.subresource_encoder.InRange(normalized_subresource_range)) {
            RangeGenerator range_gen(image_state.subresource_encoder, normalized_subresource_range);
            TrackFirstLayout(*image_layout_map, std::move(range_gen), layout, normalized_subresource_range.aspectMask, nullptr);
        }
    }
}

// Set image layout for all slices of an image view
void CommandBuffer::SetImageViewLayout(const vvl::ImageView &view_state, VkImageLayout layout, VkImageLayout layoutStencil) {
    const vvl::Image *image_state = view_state.image_state.get();

    VkImageSubresourceRange sub_range = view_state.normalized_subresource_range;

    if (sub_range.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) && layoutStencil != kInvalidLayout) {
        sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        SetImageLayout(*image_state, sub_range, layout);
        sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        SetImageLayout(*image_state, sub_range, layoutStencil);
    } else {
        // If layoutStencil is kInvalidLayout (meaning no separate depth/stencil layout), image view format has both depth and
        // stencil aspects, and subresource has only one of aspect out of depth or stencil, then the missing aspect will also be
        // transitioned and thus must be included explicitly
        if (const VkFormat format = view_state.create_info.format; vkuFormatIsDepthAndStencil(format)) {
            if (layoutStencil == kInvalidLayout &&
                (sub_range.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                sub_range.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        SetImageLayout(*image_state, sub_range, layout);
    }
}

void CommandBuffer::RecordStateCmd(CBDynamicState state) {
    // NOTE: this can be extended to use RecordCommand for state commands if needed (currently not needed)
    command_count++;
    RecordDynamicState(state);

    vvl::Pipeline *pipeline = GetLastBoundGraphics().pipeline_state;
    if (pipeline && !pipeline->IsDynamic(state)) {
        dirty_static_state = true;
    }
}

void CommandBuffer::RecordDynamicState(CBDynamicState state) {
    dynamic_state_status.cb.set(state);
    dynamic_state_status.pipeline.set(state);
    dynamic_state_status.history.set(state);
}

void CommandBuffer::RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count, const VkViewport *viewports) {
    RecordStateCmd(CB_DYNAMIC_STATE_VIEWPORT);
    if (dynamic_state_value.viewports.size() < first_viewport + viewport_count) {
        dynamic_state_value.viewports.resize(first_viewport + viewport_count);
    }
    for (size_t i = 0; i < viewport_count; ++i) {
        dynamic_state_value.viewports[first_viewport + i] = viewports[i];
    }
    for (auto &item : sub_states_) {
        item.second->RecordSetViewport(first_viewport, viewport_count);
    }
}

void CommandBuffer::RecordSetViewportWithCount(uint32_t viewport_count, const VkViewport *viewports) {
    RecordStateCmd(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    dynamic_state_value.viewport_count = viewport_count;
    dynamic_state_value.viewports.resize(viewport_count);
    for (size_t i = 0; i < viewport_count; ++i) {
        dynamic_state_value.viewports[i] = viewports[i];
    }

    for (auto &item : sub_states_) {
        item.second->RecordSetViewportWithCount(viewport_count);
    }
}

void CommandBuffer::RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count) {
    RecordStateCmd(CB_DYNAMIC_STATE_SCISSOR);
    for (auto &item : sub_states_) {
        item.second->RecordSetScissor(first_scissor, scissor_count);
    }
}

void CommandBuffer::RecordSetScissorWithCount(uint32_t scissor_count) {
    RecordStateCmd(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    dynamic_state_value.scissor_count = scissor_count;
    for (auto &item : sub_states_) {
        item.second->RecordSetScissorWithCount(scissor_count);
    }
}

void CommandBuffer::RecordSetDepthCompareOp(VkCompareOp depth_compare_op) {
    RecordStateCmd(CB_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    for (auto &item : sub_states_) {
        item.second->RecordSetDepthCompareOp(depth_compare_op);
    }
}
void CommandBuffer::RecordSetDepthTestEnable(VkBool32 depth_test_enable) {
    RecordStateCmd(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    dynamic_state_value.depth_test_enable = depth_test_enable;
    for (auto &item : sub_states_) {
        item.second->RecordSetDepthTestEnable(depth_test_enable);
    }
}

void CommandBuffer::RecordCopyBuffer(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                     const VkBufferCopy *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyBuffer(src_buffer_state, dst_buffer_state, region_count, regions, loc);
    }
}

void CommandBuffer::RecordCopyBuffer2(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                      const VkBufferCopy2 *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyBuffer2(src_buffer_state, dst_buffer_state, region_count, regions, loc);
    }
}

void CommandBuffer::RecordCopyImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                    VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy *regions,
                                    const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyImage(src_image_state, dst_image_state, src_image_layout, dst_image_layout, region_count, regions,
                                     loc);
    }
}

void CommandBuffer::RecordCopyImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                     VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy2 *regions,
                                     const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyImage2(src_image_state, dst_image_state, src_image_layout, dst_image_layout, region_count, regions,
                                      loc);
    }
}

void CommandBuffer::RecordCopyBufferToImage(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state,
                                            VkImageLayout dst_image_layout, uint32_t region_count, const VkBufferImageCopy *regions,
                                            const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyBufferToImage(src_buffer_state, dst_image_state, dst_image_layout, region_count, regions, loc);
    }
}

void CommandBuffer::RecordCopyBufferToImage2(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state,
                                             VkImageLayout dst_image_layout, uint32_t region_count,
                                             const VkBufferImageCopy2 *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyBufferToImage2(src_buffer_state, dst_image_state, dst_image_layout, region_count, regions, loc);
    }
}

void CommandBuffer::RecordCopyImageToBuffer(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state,
                                            VkImageLayout src_image_layout, uint32_t region_count, const VkBufferImageCopy *regions,
                                            const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyImageToBuffer(src_image_state, dst_buffer_state, src_image_layout, region_count, regions, loc);
    }
}

void CommandBuffer::RecordCopyImageToBuffer2(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state,
                                             VkImageLayout src_image_layout, uint32_t region_count,
                                             const VkBufferImageCopy2 *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordCopyImageToBuffer2(src_image_state, dst_buffer_state, src_image_layout, region_count, regions, loc);
    }
}

void CommandBuffer::RecordBlitImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                    VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit *regions,
                                    const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordBlitImage(src_image_state, dst_image_state, src_image_layout, dst_image_layout, region_count, regions,
                                     loc);
    }
}

void CommandBuffer::RecordBlitImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                     VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit2 *regions,
                                     const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordBlitImage2(src_image_state, dst_image_state, src_image_layout, dst_image_layout, region_count, regions,
                                      loc);
    }
}

void CommandBuffer::RecordResolveImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                       const VkImageResolve *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordResolveImage(src_image_state, dst_image_state, region_count, regions, loc);
    }
}

void CommandBuffer::RecordResolveImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                        const VkImageResolve2 *regions, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordResolveImage2(src_image_state, dst_image_state, region_count, regions, loc);
    }
}

void CommandBuffer::RecordClearColorImage(vvl::Image &image_state, VkImageLayout image_layout,
                                          const VkClearColorValue *color_values, uint32_t range_count,
                                          const VkImageSubresourceRange *ranges, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordClearColorImage(image_state, image_layout, color_values, range_count, ranges, loc);
    }
}

void CommandBuffer::RecordClearDepthStencilImage(vvl::Image &image_state, VkImageLayout image_layout,
                                                 const VkClearDepthStencilValue *depth_stencil_values, uint32_t range_count,
                                                 const VkImageSubresourceRange *ranges, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordClearDepthStencilImage(image_state, image_layout, depth_stencil_values, range_count, ranges, loc);
    }
}

void CommandBuffer::RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment *pAttachments, uint32_t rect_count,
                                           const VkClearRect *pRects, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordClearAttachments(attachment_count, pAttachments, rect_count, pRects, loc);
    }
}

void CommandBuffer::RecordFillBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordFillBuffer(buffer_state, offset, size, loc);
    }
}

void CommandBuffer::RecordUpdateBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordUpdateBuffer(buffer_state, offset, size, loc);
    }
}

void CommandBuffer::RecordSetEvent(VkEvent event, VkPipelineStageFlags2 stage_mask, const VkDependencyInfo *dependency_info,
                                   const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordSetEvent(event, stage_mask, dependency_info);
    }

    if (!dev_data.disabled[command_buffer_state]) {
        if (auto event_state = dev_data.Get<vvl::Event>(event)) {
            AddChild(event_state);
        }
    }
    events.push_back(event);
    if (!waited_events.count(event)) {
        write_events_before_wait.push_back(event);
    }
}

void CommandBuffer::RecordResetEvent(VkEvent event, VkPipelineStageFlags2 stage_mask, const Location &loc) {
    RecordCommand(loc);
    for (auto &item : sub_states_) {
        item.second->RecordResetEvent(event, stage_mask);
    }

    if (!dev_data.disabled[command_buffer_state]) {
        if (auto event_state = dev_data.Get<vvl::Event>(event)) {
            AddChild(event_state);
        }
    }
    events.push_back(event);
    if (!waited_events.count(event)) {
        write_events_before_wait.push_back(event);
    }
}

void CommandBuffer::RecordWaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags2 src_stage_mask,
                                     const VkDependencyInfo *dependency_info, const Location &loc) {
    for (auto &item : sub_states_) {
        item.second->RecordWaitEvents(eventCount, pEvents, src_stage_mask, dependency_info, loc);
    }
    for (uint32_t i = 0; i < eventCount; ++i) {
        const VkEvent event_hanle = pEvents[i];
        if (!dev_data.disabled[command_buffer_state]) {
            if (auto event_state = dev_data.Get<vvl::Event>(event_hanle)) {
                AddChild(event_state);
            }
        }
        waited_events.insert(event_hanle);
        events.push_back(event_hanle);
    }
}

void CommandBuffer::RecordBarrierObjects(uint32_t buffer_barrier_count, const VkBufferMemoryBarrier *buffer_barriers,
                                         uint32_t image_barrier_count, const VkImageMemoryBarrier *image_barriers,
                                         VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                         const Location &loc) {
    if (!dev_data.disabled[command_buffer_state]) {
        for (uint32_t i = 0; i < buffer_barrier_count; i++) {
            if (auto buffer_state = dev_data.Get<vvl::Buffer>(buffer_barriers[i].buffer)) {
                AddChild(buffer_state);
            }
        }
        for (uint32_t i = 0; i < image_barrier_count; i++) {
            if (auto image_state = dev_data.Get<vvl::Image>(image_barriers[i].image)) {
                AddChild(image_state);
            }
        }
    }

    for (auto &item : sub_states_) {
        item.second->RecordBarriers(buffer_barrier_count, buffer_barriers, image_barrier_count, image_barriers, src_stage_mask,
                                    dst_stage_mask, loc);
    }
}

void CommandBuffer::RecordBarrierObjects(const VkDependencyInfo &dep_info, const Location &loc) {
    if (!dev_data.disabled[command_buffer_state]) {
        for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
            if (auto buffer_state = dev_data.Get<vvl::Buffer>(dep_info.pBufferMemoryBarriers[i].buffer)) {
                AddChild(buffer_state);
            }
        }
        for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
            if (auto image_state = dev_data.Get<vvl::Image>(dep_info.pImageMemoryBarriers[i].image)) {
                AddChild(image_state);
            }
        }
    }

    // TODO - When moving here, these were not in CoreCheck, need to understand if we want SetEvents or not to validate the same
    // things as WaitEvent/PipelineBarriers
    if (loc.function != vvl::Func::vkCmdSetEvent2 && loc.function != vvl::Func::vkCmdSetEvent2KHR) {
        for (auto &item : sub_states_) {
            item.second->RecordBarriers2(dep_info, loc);
        }
    }
}

void CommandBuffer::RecordPushConstants(const vvl::PipelineLayout &pipeline_layout_state, VkShaderStageFlags stage_flags,
                                        uint32_t offset, uint32_t size, const void *values) {
    // Discussed in details in https://github.com/KhronosGroup/Vulkan-Docs/issues/1081
    // Internal discussion and CTS were written to prove that this is not called after an incompatible vkCmdBindPipeline
    // "Binding a pipeline with a layout that is not compatible with the push constant layout does not disturb the push constant
    // values"
    //
    // vkCmdBindDescriptorSet has nothing to do with push constants and don't need to call this after neither
    //
    // Part of this assumes apps at draw/dispatch/traceRays/etc time will have it properly compatible or else other VU will be
    // triggered
    if (push_constant_ranges_layout != pipeline_layout_state.push_constant_ranges_layout) {
        push_constant_ranges_layout = pipeline_layout_state.push_constant_ranges_layout;
        for (auto &item : sub_states_) {
            item.second->ClearPushConstants();
        }
    }

    for (auto &item : sub_states_) {
        item.second->RecordPushConstants(pipeline_layout_state.VkHandle(), stage_flags, offset, size, values);
    }
}

void CommandBuffer::RecordBeginConditionalRendering(const Location &loc) {
    RecordCommand(loc);
    conditional_rendering_active = true;
    conditional_rendering_inside_render_pass = active_render_pass != nullptr;
    conditional_rendering_subpass = GetActiveSubpass();
}

void CommandBuffer::RecordEndConditionalRendering(const Location &loc) {
    RecordCommand(loc);
    conditional_rendering_active = false;
    conditional_rendering_inside_render_pass = false;
    conditional_rendering_subpass = 0;
}

void CommandBuffer::RecordSetRenderingAttachmentLocations(const VkRenderingAttachmentLocationInfo *pLocationInfo,
                                                          const Location &loc) {
    RecordCommand(loc);
    rendering_attachments.set_color_locations = true;
    SetRenderingAttachmentLocations(rendering_attachments, pLocationInfo);
}

void CommandBuffer::RecordSetRenderingInputAttachmentIndices(const VkRenderingInputAttachmentIndexInfo *pLocationInfo,
                                                             const Location &loc) {
    RecordCommand(loc);
    rendering_attachments.set_color_indexes = true;
    SetRenderingInputAttachmentIndices(rendering_attachments, pLocationInfo);
}

void CommandBuffer::SubmitTimeValidate(Queue &queue_state, uint32_t perf_submit_pass, const Location &loc) {
    for (const auto &it : video_session_updates) {
        auto video_session_state = dev_data.Get<vvl::VideoSession>(it.first);
        auto device_state = video_session_state->DeviceStateWrite();
        for (const auto &function : it.second) {
            function(video_session_state.get(), *device_state, /*do_validate*/ false);
        }
    }

    for (auto &item : sub_states_) {
        item.second->Submit(queue_state, perf_submit_pass, loc);
    }
}

uint32_t CommandBuffer::GetDynamicRenderingColorAttachmentCount() const {
    return active_render_pass ? active_render_pass->dynamic_rendering_color_attachment_count : 0;
}

uint32_t CommandBuffer::GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type type) const {
    // The first indexes are the color attachments, multiply by 2 as each has a resolve attachment index
    const uint32_t color_offset = 2 * GetDynamicRenderingColorAttachmentCount();
    switch (type) {
        case AttachmentInfo::Type::Depth:
            return color_offset;
        case AttachmentInfo::Type::DepthResolve:
            return color_offset + 1;
        case AttachmentInfo::Type::Stencil:
            return color_offset + 2;
        case AttachmentInfo::Type::StencilResolve:
            return color_offset + 3;
        case AttachmentInfo::Type::FragmentDensityMap:
            return color_offset + 4;
        default:
            assert(false);
    }
    return 0;
}

bool CommandBuffer::HasExternalFormatResolveAttachment() const {
    if (active_render_pass && active_render_pass->use_dynamic_rendering &&
        active_render_pass->dynamic_rendering_begin_rendering_info.colorAttachmentCount > 0) {
        return active_render_pass->dynamic_rendering_begin_rendering_info.pColorAttachments->resolveMode ==
               VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_BIT_ANDROID;
    }
    return false;
}

void CommandBuffer::BindShader(VkShaderStageFlagBits shader_stage, vvl::ShaderObject *shader_object_state) {
    auto &last_bound_state = lastBound[ConvertStageToVvlBindPoint(shader_stage)];
    const auto stage_index = static_cast<uint32_t>(ConvertToShaderObjectStage(shader_stage));
    last_bound_state.shader_object_bound[stage_index] = true;
    last_bound_state.shader_object_states[stage_index] = shader_object_state;
}

// Only called for Graphics and during Multiview
// "When multiview is enabled, at the beginning of each subpass all non-render pass state is undefined."
void CommandBuffer::UnbindResources() {
    // Vertex and index buffers
    index_buffer_binding.reset();
    current_vertex_buffer_binding_info.clear();

    // Push constants
    push_constant_ranges_layout.reset();

    // Reset status of graphics cb to force rebinding of all resources
    dynamic_state_status.cb.reset();
    dynamic_state_status.pipeline.reset();
    dynamic_state_status.history.reset();

    // Pipeline and descriptor sets
    lastBound[vvl::BindPointGraphics].Reset();
}

LogObjectList CommandBuffer::GetObjectList(VkShaderStageFlagBits stage) const {
    LogObjectList objlist(handle_);
    const auto &last_bound = lastBound[ConvertStageToVvlBindPoint(stage)];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (pipeline_state) {
        objlist.add(pipeline_state->Handle());
    } else if (VkShaderEXT shader = last_bound.GetShader(ConvertToShaderObjectStage(stage))) {
        objlist.add(shader);
    }
    return objlist;
}

LogObjectList CommandBuffer::GetObjectList(VkPipelineBindPoint pipeline_bind_point) const {
    LogObjectList objlist(handle_);

    const auto &last_bound = lastBound[ConvertToVvlBindPoint(pipeline_bind_point)];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (pipeline_state) {
        objlist.add(pipeline_state->Handle());
    } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::COMPUTE)) {
            objlist.add(shader);
        }
    } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // If using non-compute, need to check all graphics stages
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::VERTEX)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TESSELLATION_CONTROL)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TESSELLATION_EVALUATION)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::GEOMETRY)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::FRAGMENT)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::MESH)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TASK)) {
            objlist.add(shader);
        }
    }

    // If using dynamic rendering, will just not add anything
    if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS && active_render_pass) {
        objlist.add(active_render_pass->Handle());
    }

    return objlist;
}

void CommandBuffer::BeginLabel(const char *label_name) {
    ++label_stack_depth_;
    label_commands_.emplace_back(LabelCommand{true, label_name});
}

void CommandBuffer::EndLabel() {
    --label_stack_depth_;
    label_commands_.emplace_back(LabelCommand{false, std::string()});
}

void CommandBuffer::ReplayLabelCommands(const vvl::span<const LabelCommand> &label_commands,
                                        std::vector<std::string> &label_stack) {
    for (const LabelCommand &command : label_commands) {
        if (command.begin) {
            label_stack.emplace_back(command.label_name.empty() ? "(empty label)" : command.label_name);
        } else if (!label_stack.empty()) {
            // The above condition is needed for several reasons. On the primary command buffer level
            // the labels are not necessary balanced. And if the empty stack is detected in the context
            // where it is an error, then it will be reported by the core validation, but we still need
            // a safety check.
            label_stack.pop_back();
        }
    }
}

std::string CommandBuffer::GetDebugRegionName(const std::vector<LabelCommand> &label_commands, uint32_t label_command_index,
                                              const std::vector<std::string> &initial_label_stack) {
    if (label_command_index >= label_commands.size()) {
        // Can happen due to core validation error when in-use command buffer was re-recorded.
        // It's a bug if this happens in a valid vulkan program.
        return {};
    }
    auto label_commands_to_replay = vvl::make_span(label_commands.data(), label_command_index + 1);
    auto label_stack = initial_label_stack;
    vvl::CommandBuffer::ReplayLabelCommands(label_commands_to_replay, label_stack);

    // Build up complete debug region name from all enclosing regions
    std::string debug_region;
    for (const std::string &label_name : label_stack) {
        if (!debug_region.empty()) {
            debug_region += "::";
        }
        debug_region += label_name;
    }
    return debug_region;
}

std::string CommandBuffer::DescribeInvalidatedState(CBDynamicState dynamic_state) const {
    std::stringstream ss;
    if (dynamic_state_status.history[dynamic_state] && !dynamic_state_status.cb[dynamic_state]) {
        ss << " (There was a call to vkCmdBindPipeline";
        if (auto pipeline = dev_data.Get<vvl::Pipeline>(invalidated_state_pipe[dynamic_state])) {
            ss << " with " << dev_data.FormatHandle(*pipeline);
        }
        ss << " that didn't have " << DynamicStateToString(dynamic_state) << " and invalidated the prior "
           << DescribeDynamicStateCommand(dynamic_state) << " call)";
    }
    if (GetActiveSubpass() != 0 && active_render_pass->has_multiview_enabled) {
        ss << " (When multiview is enabled, vkCmdNextSubpass will invalidate all dynamic state)";
    }
    return ss.str();
}

VulkanTypedHandle CommandBufferSubState::Handle() const { return base.Handle(); }
VkCommandBuffer CommandBufferSubState::VkHandle() const { return base.VkHandle(); }

}  // namespace vvl
