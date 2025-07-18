/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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
#include "sync/sync_commandbuffer.h"
#include "error_message/error_location.h"
#include "sync/sync_op.h"
#include "sync/sync_reporting.h"
#include "sync/sync_validation.h"
#include "sync/sync_image.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/pipeline_state.h"
#include "utils/math_utils.h"
#include "utils/text_utils.h"

constexpr VkImageAspectFlags kColorAspects =
    VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;

struct ShaderStageAccesses {
    SyncAccessIndex sampled_read;
    SyncAccessIndex storage_read;
    SyncAccessIndex storage_write;
    SyncAccessIndex uniform_read;
    SyncAccessIndex acceleration_structure_read;
};

// TODO: generate me
static ShaderStageAccesses GetShaderStageAccesses(VkShaderStageFlagBits shader_stage) {
    static const vvl::unordered_map<VkShaderStageFlagBits, ShaderStageAccesses> map = {
        // clang-format off
        {VK_SHADER_STAGE_VERTEX_BIT, {
            SYNC_VERTEX_SHADER_SHADER_SAMPLED_READ,
            SYNC_VERTEX_SHADER_SHADER_STORAGE_READ,
            SYNC_VERTEX_SHADER_SHADER_STORAGE_WRITE,
            SYNC_VERTEX_SHADER_UNIFORM_READ,
            SYNC_VERTEX_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, {
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_SAMPLED_READ,
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_STORAGE_READ,
            SYNC_TESSELLATION_CONTROL_SHADER_SHADER_STORAGE_WRITE,
            SYNC_TESSELLATION_CONTROL_SHADER_UNIFORM_READ,
            SYNC_TESSELLATION_CONTROL_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, {
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_SAMPLED_READ,
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_STORAGE_READ,
            SYNC_TESSELLATION_EVALUATION_SHADER_SHADER_STORAGE_WRITE,
            SYNC_TESSELLATION_EVALUATION_SHADER_UNIFORM_READ,
            SYNC_TESSELLATION_EVALUATION_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_GEOMETRY_BIT, {
            SYNC_GEOMETRY_SHADER_SHADER_SAMPLED_READ,
            SYNC_GEOMETRY_SHADER_SHADER_STORAGE_READ,
            SYNC_GEOMETRY_SHADER_SHADER_STORAGE_WRITE,
            SYNC_GEOMETRY_SHADER_UNIFORM_READ,
            SYNC_GEOMETRY_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_FRAGMENT_BIT, {
            SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ,
            SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ,
            SYNC_FRAGMENT_SHADER_SHADER_STORAGE_WRITE,
            SYNC_FRAGMENT_SHADER_UNIFORM_READ,
            SYNC_FRAGMENT_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_COMPUTE_BIT, {
            SYNC_COMPUTE_SHADER_SHADER_SAMPLED_READ,
            SYNC_COMPUTE_SHADER_SHADER_STORAGE_READ,
            SYNC_COMPUTE_SHADER_SHADER_STORAGE_WRITE,
            SYNC_COMPUTE_SHADER_UNIFORM_READ,
            SYNC_COMPUTE_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_RAYGEN_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_ANY_HIT_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_MISS_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_INTERSECTION_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_CALLABLE_BIT_KHR, {
            SYNC_RAY_TRACING_SHADER_SHADER_SAMPLED_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_READ,
            SYNC_RAY_TRACING_SHADER_SHADER_STORAGE_WRITE,
            SYNC_RAY_TRACING_SHADER_UNIFORM_READ,
            SYNC_RAY_TRACING_SHADER_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_TASK_BIT_EXT, {
            SYNC_TASK_SHADER_EXT_SHADER_SAMPLED_READ,
            SYNC_TASK_SHADER_EXT_SHADER_STORAGE_READ,
            SYNC_TASK_SHADER_EXT_SHADER_STORAGE_WRITE,
            SYNC_TASK_SHADER_EXT_UNIFORM_READ,
            SYNC_TASK_SHADER_EXT_ACCELERATION_STRUCTURE_READ,
        }},
        {VK_SHADER_STAGE_MESH_BIT_EXT, {
            SYNC_MESH_SHADER_EXT_SHADER_SAMPLED_READ,
            SYNC_MESH_SHADER_EXT_SHADER_STORAGE_READ,
            SYNC_MESH_SHADER_EXT_SHADER_STORAGE_WRITE,
            SYNC_MESH_SHADER_EXT_UNIFORM_READ,
            SYNC_MESH_SHADER_EXT_ACCELERATION_STRUCTURE_READ,
        }},
        // clang-format on
    };
    auto it = map.find(shader_stage);
    assert(it != map.end());
    return it->second;
}

static ResourceAccessRange MakeRange(VkDeviceSize offset, uint32_t first_index, uint32_t count, uint32_t stride) {
    const VkDeviceSize range_start = offset + (first_index * stride);
    const VkDeviceSize range_size = count * stride;
    return MakeRange(range_start, range_size);
}

static ResourceAccessRange MakeRange(const vvl::BufferView &buf_view_state) {
    return MakeRange(*buf_view_state.buffer_state.get(), buf_view_state.create_info.offset, buf_view_state.create_info.range);
}

static SyncAccessIndex GetSyncStageAccessIndexsByDescriptorSet(VkDescriptorType descriptor_type,
                                                               const spirv::ResourceInterfaceVariable &variable,
                                                               VkShaderStageFlagBits stage_flag) {
    if (!variable.IsAccessed()) {
        return SYNC_ACCESS_INDEX_NONE;
    }
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        assert(stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT);
        return SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
    }
    const auto stage_accesses = GetShaderStageAccesses(stage_flag);

    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        return stage_accesses.uniform_read;
    }
    if (descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        return stage_accesses.acceleration_structure_read;
    }

    // If the desriptorSet is writable, we don't need to care SHADER_READ. SHADER_WRITE is enough.
    // Because if write hazard happens, read hazard might or might not happen.
    // But if write hazard doesn't happen, read hazard is impossible to happen.
    if (variable.IsWrittenTo()) {
        return stage_accesses.storage_write;
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
        return stage_accesses.sampled_read;
    } else {
        if (variable.IsImage() && !variable.IsImageReadFrom()) {
            // only image descriptor was accessed, not the image data
            return SYNC_ACCESS_INDEX_NONE;
        }
        return stage_accesses.storage_read;
    }
}

CommandExecutionContext::CommandExecutionContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags)
    : sync_state_(sync_validator), error_messages_(sync_validator.error_messages_), queue_flags_(queue_flags) {}

bool CommandExecutionContext::ValidForSyncOps() const {
    const bool valid = GetCurrentEventsContext() && GetCurrentAccessContext();
    assert(valid);
    return valid;
}

CommandBufferAccessContext::CommandBufferAccessContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags)
    : CommandExecutionContext(sync_validator, queue_flags),
      cb_state_(),
      access_log_(std::make_shared<AccessLog>()),
      cbs_referenced_(std::make_shared<CommandBufferSet>()),
      command_number_(0),
      reset_count_(0),
      cb_access_context_(),
      current_context_(&cb_access_context_),
      events_context_(),
      render_pass_contexts_(),
      current_renderpass_context_(),
      sync_ops_() {}

CommandBufferAccessContext::CommandBufferAccessContext(SyncValidator &sync_validator, vvl::CommandBuffer *cb_state)
    : CommandBufferAccessContext(sync_validator, cb_state->GetQueueFlags()) {
    cb_state_ = cb_state;
    sync_state_.stats.AddCommandBufferContext();
}

// NOTE: Make sure the proxy doesn't outlive from, as the proxy is pointing directly to access contexts owned by from.
CommandBufferAccessContext::CommandBufferAccessContext(const CommandBufferAccessContext &from, AsProxyContext dummy)
    : CommandBufferAccessContext(from.sync_state_, from.cb_state_->GetQueueFlags()) {
    // Copy only the needed fields out of from for a temporary, proxy command buffer context
    cb_state_ = from.cb_state_;
    access_log_ = std::make_shared<AccessLog>(*from.access_log_);  // potentially large, but no choice given tagging lookup.
    command_number_ = from.command_number_;
    reset_count_ = from.reset_count_;

    handles_ = from.handles_;
    sync_state_.stats.AddHandleRecord((uint32_t)from.handles_.size());

    const auto *from_context = from.GetCurrentAccessContext();
    assert(from_context);

    // Construct a fully resolved single access context out of from
    cb_access_context_.ResolveFromContext(*from_context);
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
    sync_state_.stats.AddCommandBufferContext();
}

CommandBufferAccessContext::~CommandBufferAccessContext() {
    sync_state_.stats.RemoveCommandBufferContext();
    sync_state_.stats.RemoveHandleRecord((uint32_t)handles_.size());
}

void CommandBufferAccessContext::Reset() {
    access_log_ = std::make_shared<AccessLog>();
    cbs_referenced_ = std::make_shared<CommandBufferSet>();
    if (cb_state_) {
        cbs_referenced_->push_back(cb_state_->shared_from_this());
    }
    sync_ops_.clear();
    command_number_ = 0;
    reset_count_++;

    sync_state_.stats.RemoveHandleRecord((uint32_t)handles_.size());
    handles_.clear();

    current_command_tag_ = vvl::kNoIndex32;
    cb_access_context_.Reset();
    render_pass_contexts_.clear();
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    events_context_.Clear();
    dynamic_rendering_info_.reset();
}

bool CommandBufferAccessContext::ValidateBeginRendering(const ErrorObject &error_obj,
                                                        syncval_state::BeginRenderingCmdState &cmd_state) const {
    bool skip = false;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();

    // Load operations do not happen when resuming
    if (info.info.flags & VK_RENDERING_RESUMING_BIT) {
        return skip;
    }

    // Need to hazard detect load operations vs. the attachment views
    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = 0; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        const SyncAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) {
            continue;
        }

        const HazardResult hazard =
            GetCurrentAccessContext()->DetectHazard(attachment.view_gen, load_index, attachment.GetOrdering());
        if (hazard.IsHazard()) {
            LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());

            std::stringstream ss;
            ss << vvl::String(vvl::Field::pRenderingInfo) << ".";
            ss << attachment.GetLocation(error_obj.location, i).Fields();
            ss << " (" << sync_state_.FormatHandle(attachment.view->Handle());
            ss << ", loadOp " << string_VkAttachmentLoadOp(attachment.info.loadOp) << ")";
            std::string resource_description = ss.str();

            const std::string error = sync_state_.error_messages_.BeginRenderingError(hazard, *this, error_obj.location.function,
                                                                                      resource_description, attachment.info.loadOp);
            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, error_obj.location.function, error);
            if (skip) {
                break;
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state, const Location &loc) {
    using Attachment = syncval_state::DynamicRenderingInfo::Attachment;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();
    const auto tag = NextCommandTag(loc.function);

    // Only load if not resuming
    if (0 == (info.info.flags & VK_RENDERING_RESUMING_BIT)) {
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        for (uint32_t i = 0; i < attachment_count; i++) {
            const Attachment &attachment = info.attachments[i];
            const SyncAccessIndex load_index = attachment.GetLoadUsage();
            if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

            GetCurrentAccessContext()->UpdateAccessState(attachment.view_gen, load_index, attachment.GetOrdering(),
                                                         ResourceUsageTagEx{tag});
        }
    }

    dynamic_rendering_info_ = std::move(cmd_state.info);
}

bool CommandBufferAccessContext::ValidateEndRendering(const ErrorObject &error_obj) const {
    bool skip = false;

    // Only validate resolve and store if not suspending (as specified by BeginRendering)
    if (!dynamic_rendering_info_ || (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT) != 0) {
        return skip;
    }

    for (uint32_t i = 0; i < (uint32_t)dynamic_rendering_info_->attachments.size(); i++) {
        const auto &attachment = dynamic_rendering_info_->attachments[i];

        auto attachment_description = [this, &error_obj, &attachment, i](const auto &view, std::stringstream &ss) {
            ss << vvl::String(vvl::Field::pRenderingInfo) << ".";
            ss << attachment.GetLocation(error_obj.location, uint32_t(i)).Fields();
            ss << " (" << sync_state_.FormatHandle(view->Handle());
        };

        // The logic about whether to resolve is embedded in the Attachment constructor
        if (attachment.resolve_gen) {
            const bool is_color = attachment.type == syncval_state::AttachmentType::kColor;
            const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;

            HazardResult hazard = current_context_->DetectHazard(attachment.view_gen, kResolveRead, kResolveOrder);
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());

                std::stringstream ss;
                attachment_description(attachment.view, ss);
                ss << ", resolveMode " << string_VkResolveModeFlagBits(attachment.info.resolveMode) << ")";
                const std::string resource_description = ss.str();

                const std::string error = sync_state_.error_messages_.EndRenderingResolveError(
                    hazard, *this, error_obj.location.function, resource_description, attachment.info.resolveMode, false);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, error_obj.location.function, error);
                if (skip) {
                    break;
                }
            }

            hazard = current_context_->DetectHazard(*attachment.resolve_gen, kResolveWrite, kResolveOrder);
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), attachment.resolve_view->Handle());

                std::stringstream ss;
                attachment_description(attachment.resolve_view, ss);
                ss << ", resolveMode " << string_VkResolveModeFlagBits(attachment.info.resolveMode) << ")";
                const std::string resource_description = ss.str();

                const std::string error = sync_state_.error_messages_.EndRenderingResolveError(
                    hazard, *this, error_obj.location.function, resource_description, attachment.info.resolveMode, true);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, error_obj.location.function, error);
                if (skip) {
                    break;
                }
            }
        }

        const SyncAccessIndex store_access = attachment.GetStoreUsage();
        if (store_access != SYNC_ACCESS_INDEX_NONE) {
            HazardResult hazard =
                current_context_->DetectHazard(attachment.view_gen, store_access, kStoreOrder, SyncFlag::kStoreOp);
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());

                std::stringstream ss;
                attachment_description(attachment.view, ss);
                ss << ", storeOp " << string_VkAttachmentStoreOp(attachment.info.storeOp) << ")";
                const std::string resource_description = ss.str();

                const std::string error = sync_state_.error_messages_.EndRenderingStoreError(
                    hazard, *this, error_obj.location.function, resource_description, attachment.info.storeOp);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, error_obj.location.function, error);
                if (skip) {
                    break;
                }
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordEndRendering(const RecordObject &record_obj) {
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        auto store_tag = NextCommandTag(record_obj.location.function, ResourceUsageRecord::SubcommandType::kStoreOp);

        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        AccessContext *access_context = GetCurrentAccessContext();
        for (uint32_t i = 0; i < attachment_count; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                const bool is_color = attachment.type == syncval_state::AttachmentType::kColor;
                const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;
                access_context->UpdateAccessState(attachment.view_gen, kResolveRead, kResolveOrder, ResourceUsageTagEx{store_tag});
                access_context->UpdateAccessState(*attachment.resolve_gen, kResolveWrite, kResolveOrder,
                                                  ResourceUsageTagEx{store_tag});
            }

            const SyncAccessIndex store_index = attachment.GetStoreUsage();
            if (store_index == SYNC_ACCESS_INDEX_NONE) continue;
            access_context->UpdateAccessState(attachment.view_gen, store_index, kStoreOrder, ResourceUsageTagEx{store_tag});
        }
    }

    dynamic_rendering_info_.reset();
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const Location &loc) const {
    bool skip = false;
    if (!sync_state_.syncval_settings.shader_accesses_heuristic) {
        return skip;
    }
    const auto &last_bound_state = cb_state_->lastBound[ConvertToVvlBindPoint(pipelineBindPoint)];
    const vvl::Pipeline *pipe = last_bound_state.pipeline_state;
    const std::vector<LastBound::DescriptorSetSlot> &ds_slots = last_bound_state.ds_slots;
    if (!pipe) {
        return skip;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && pipe->RasterizationDisabled()) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= ds_slots.size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto &ds_slot = ds_slots[variable.decorations.set];
            const auto *descriptor_set = ds_slot.ds_state.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncAccessIndex sync_index = GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Currently, validation of memory accesses based on declared descriptors can produce false-positives.
            // The shader can decide not to do such accesses, it can perform accesses with more narrow scope
            // (e.g. read access, when both reads and writes are allowed) or for an array of descriptors, not all
            // elements are accessed in the general case.
            //
            // This workaround disables validation for the descriptor array case.
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t index = 0; index < binding->count; index++) {
                const auto *descriptor = binding->GetDescriptor(index);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        if (descriptor->Invalid()) {
                            continue;
                        }

                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        const auto *img_view_state = image_descriptor->GetImageViewState();
                        VkImageLayout image_layout = image_descriptor->GetImageLayout();

                        if (img_view_state->is_depth_sliced) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }

                        HazardResult hazard;

                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->render_area.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->render_area.offset);
                            // Input attachments are subject to raster ordering rules
                            hazard =
                                current_context_->DetectHazard(*img_view_state, offset, extent, sync_index, SyncOrdering::kRaster);
                        } else {
                            hazard = current_context_->DetectHazard(*img_view_state, sync_index);
                        }

                        if (hazard.IsHazard() && !sync_state_.SuppressedBoundDescriptorWAW(hazard)) {
                            LogObjectList objlist(cb_state_->Handle(), img_view_state->Handle(), pipe->Handle());
                            const auto error = error_messages_.ImageDescriptorError(
                                hazard, *this, loc.function, sync_state_.FormatHandle(*img_view_state), *pipe,
                                variable.decorations.set, *descriptor_set, descriptor_type, variable.decorations.binding, index,
                                stage_state.GetStage(), image_layout);
                            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_.SuppressedBoundDescriptorWAW(hazard)) {
                            LogObjectList objlist(cb_state_->Handle(), buf_view_state->Handle(), pipe->Handle());
                            const auto error = error_messages_.BufferDescriptorError(
                                hazard, *this, loc.function, sync_state_.FormatHandle(*buf_view_state), *pipe,
                                variable.decorations.set, *descriptor_set, descriptor_type, variable.decorations.binding, index,
                                stage_state.GetStage());
                            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
                        }
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        VkDeviceSize offset = buffer_descriptor->GetOffset();
                        if (vvl::IsDynamicDescriptor(descriptor_type)) {
                            const uint32_t dynamic_offset_index =
                                descriptor_set->GetDynamicOffsetIndexFromBinding(binding->binding);
                            if (dynamic_offset_index >= ds_slot.dynamic_offsets.size()) {
                                continue;  // core validation error
                            }
                            offset += ds_slot.dynamic_offsets[dynamic_offset_index];
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range = MakeRange(*buf_state, offset, buffer_descriptor->GetRange());
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_.SuppressedBoundDescriptorWAW(hazard)) {
                            LogObjectList objlist(cb_state_->Handle(), buf_state->Handle(), pipe->Handle());
                            const auto error = error_messages_.BufferDescriptorError(
                                hazard, *this, loc.function, sync_state_.FormatHandle(*buf_state), *pipe, variable.decorations.set,
                                *descriptor_set, descriptor_type, variable.decorations.binding, index, stage_state.GetStage());
                            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
                        }
                        break;
                    }
                    case DescriptorClass::AccelerationStructure: {
                        const auto *accel_descriptor = static_cast<const vvl::AccelerationStructureDescriptor *>(descriptor);
                        if (accel_descriptor->Invalid()) {
                            continue;
                        }
                        const vvl::AccelerationStructureKHR *accel = accel_descriptor->GetAccelerationStructureStateKHR();
                        if (!accel || !accel->buffer_state) {
                            continue;
                        }
                        const ResourceAccessRange range =
                            MakeRange(*accel->buffer_state, accel->create_info.offset, accel->create_info.size);
                        auto hazard = current_context_->DetectHazard(*accel->buffer_state, sync_index, range);
                        // TODO: figure out what is the purpose of SuppressedBoundDescriptorWAW and do we still need it?
                        if (hazard.IsHazard() && !sync_state_.SuppressedBoundDescriptorWAW(hazard)) {
                            LogObjectList objlist(cb_state_->Handle(), accel->buffer_state->Handle(), pipe->Handle());
                            const std::string resource_description = sync_state_.FormatHandle(accel->Handle());
                            const std::string error = error_messages_.AccelerationStructureDescriptorError(
                                hazard, *this, loc.function, resource_description, *pipe, variable.decorations.set, *descriptor_set,
                                descriptor_type, variable.decorations.binding, index, stage_state.GetStage());
                            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
                        }
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT
                    default:
                        break;
                }
            }
        }
    }
    return skip;
}

// TODO: Record structure repeats Validate. Unify this code, it was the source of bugs few times already.
void CommandBufferAccessContext::RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                 const ResourceUsageTag tag) {
    if (!sync_state_.syncval_settings.shader_accesses_heuristic) {
        return;
    }

    const auto &last_bound_state = cb_state_->lastBound[ConvertToVvlBindPoint(pipelineBindPoint)];
    const vvl::Pipeline *pipe = last_bound_state.pipeline_state;
    const std::vector<LastBound::DescriptorSetSlot> &ds_slots = last_bound_state.ds_slots;
    if (!pipe) {
        return;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && pipe->RasterizationDisabled()) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= ds_slots.size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto &ds_slot = ds_slots[variable.decorations.set];
            const auto *descriptor_set = ds_slot.ds_state.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncAccessIndex sync_index = GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Do not update state for descriptor array (the same as in Validate function).
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t i = 0; i < binding->count; i++) {
                const auto *descriptor = binding->GetDescriptor(i);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        if (image_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *img_view_state = image_descriptor->GetImageViewState();
                        if (img_view_state->is_depth_sliced) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, img_view_state->image_state->Handle());
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->render_area.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->render_area.offset);
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kRaster, offset, extent,
                                                                tag_ex);
                        } else {
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kNonAttachment, tag_ex);
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_view_state->Handle());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag_ex);
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        VkDeviceSize offset = buffer_descriptor->GetOffset();
                        if (vvl::IsDynamicDescriptor(descriptor_type)) {
                            const uint32_t dynamic_offset_index =
                                descriptor_set->GetDynamicOffsetIndexFromBinding(binding->binding);
                            if (dynamic_offset_index >= ds_slot.dynamic_offsets.size()) {
                                continue;  // core validation error
                            }
                            offset += ds_slot.dynamic_offsets[dynamic_offset_index];
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range = MakeRange(*buf_state, offset, buffer_descriptor->GetRange());
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_state->Handle());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag_ex);
                        break;
                    }
                    case DescriptorClass::AccelerationStructure: {
                        const auto *accel_descriptor = static_cast<const vvl::AccelerationStructureDescriptor *>(descriptor);
                        if (accel_descriptor->Invalid()) {
                            continue;
                        }
                        const vvl::AccelerationStructureKHR *accel = accel_descriptor->GetAccelerationStructureStateKHR();
                        if (!accel || !accel->buffer_state) {
                            continue;
                        }
                        const ResourceAccessRange range =
                            MakeRange(*accel->buffer_state, accel->create_info.offset, accel->create_info.size);
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, accel->Handle());
                        current_context_->UpdateAccessState(*accel->buffer_state, sync_index, SyncOrdering::kNonAttachment, range,
                                                            tag_ex);
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT
                    default:
                        break;
                }
            }
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex,
                                                    const Location &loc) const {
    bool skip = false;
    const auto *pipe = cb_state_->GetLastBoundGraphics().pipeline_state;
    if (!pipe) {
        return skip;
    }

    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info;
    const auto &vertex_bindings = pipe->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                      ? cb_state_->dynamic_state_value.vertex_bindings
                                      : pipe->vertex_input_state->bindings;

    for (const auto &[_, binding_state] : vertex_bindings) {
        const auto &binding_desc = binding_state.desc;
        if (binding_desc.inputRate != VK_VERTEX_INPUT_RATE_VERTEX) {
            // TODO: add support to determine range of instance level attributes
            continue;
        }
        if (const vvl::VertexBufferBinding *vertex_buffer = vvl::Find(binding_buffers, binding_desc.binding)) {
            const auto buf_state = sync_state_.Get<vvl::Buffer>(vertex_buffer->buffer);
            if (!buf_state) continue;  // also skips if using nullDescriptor

            ResourceAccessRange range;
            if (vertexCount.has_value()) {  // the range is specified
                range = MakeRange(vertex_buffer->offset, firstVertex, *vertexCount, binding_desc.stride);
            } else {  // entire vertex buffer
                range = MakeRange(vertex_buffer->offset, vertex_buffer->effective_size);
            }

            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), buf_state->Handle(), pipe->Handle());
                const std::string resource_description = "vertex " + sync_state_.FormatHandle(*buf_state);
                const auto error = error_messages_.BufferError(hazard, *this, loc.function, resource_description, range);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex,
                                                  const ResourceUsageTag tag) {
    const auto *pipe = cb_state_->GetLastBoundGraphics().pipeline_state;
    if (!pipe) {
        return;
    }
    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info;
    const auto &vertex_bindings = pipe->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                      ? cb_state_->dynamic_state_value.vertex_bindings
                                      : pipe->vertex_input_state->bindings;

    for (const auto &[_, binding_state] : vertex_bindings) {
        const auto &binding_desc = binding_state.desc;
        if (binding_desc.inputRate != VK_VERTEX_INPUT_RATE_VERTEX) {
            // TODO: add support to determine range of instance level attributes
            continue;
        }
        if (const auto *vertex_buffer = vvl::Find(binding_buffers, binding_desc.binding)) {
            const auto buf_state = sync_state_.Get<vvl::Buffer>(vertex_buffer->buffer);
            if (!buf_state) continue;  // also skips if using nullDescriptor

            ResourceAccessRange range;
            if (vertexCount.has_value()) {  // the range is specified
                range = MakeRange(vertex_buffer->offset, firstVertex, *vertexCount, binding_desc.stride);
            } else {  // entire vertex buffer
                range = MakeRange(vertex_buffer->offset, vertex_buffer->effective_size);
            }

            const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_state->Handle());
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ,
                                                SyncOrdering::kNonAttachment, range, tag_ex);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(uint32_t index_count, uint32_t firstIndex, const Location &loc) const {
    bool skip = false;
    const auto &index_binding = cb_state_->index_buffer_binding;
    const auto index_buf_state = sync_state_.Get<vvl::Buffer>(index_binding.buffer);
    if (!index_buf_state) return skip;

    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding.offset, firstIndex, index_count, index_size);

    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, range);
    if (hazard.IsHazard()) {
        LogObjectList objlist(cb_state_->Handle(), index_buf_state->Handle());
        if (const auto *pipe = cb_state_->GetLastBoundGraphics().pipeline_state) {
            objlist.add(pipe->Handle());
        }
        const std::string resource_description = "index " + sync_state_.FormatHandle(*index_buf_state);
        const auto error = error_messages_.BufferError(hazard, *this, loc.function, resource_description, range);
        skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
    }

    // TODO: Shader instrumentation support is needed to read index buffer content and determine more accurate range
    // of accessed versices (new syncval mode). Scanning index buffer for each draw can be impractical though.
    // More practical option can be to leave this as an optional heuristic that always tracks entire vertex buffer.
    skip |= ValidateDrawVertex(std::optional<uint32_t>(), 0, loc);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const ResourceUsageTag tag) {
    const auto &index_binding = cb_state_->index_buffer_binding;
    const auto index_buf_state = sync_state_.Get<vvl::Buffer>(index_binding.buffer);
    if (!index_buf_state) return;

    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding.offset, firstIndex, indexCount, index_size);
    const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, index_buf_state->Handle());
    current_context_->UpdateAccessState(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, SyncOrdering::kNonAttachment, range, tag_ex);

    // TODO: Shader instrumentation support is needed to read index buffer content and determine more accurate range
    // of accessed versices (new syncval mode). Scanning index buffer for each draw can be impractical though.
    // More practical option can be to leave this as an optional heuristic that always tracks entire vertex buffer.
    RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawAttachment(const Location &loc) const {
    bool skip = false;
    if (current_renderpass_context_) {
        skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(*this, loc.function);
    } else if (dynamic_rendering_info_) {
        skip |= ValidateDrawDynamicRenderingAttachment(loc);
    }
    return skip;
}

bool CommandBufferAccessContext::ValidateDrawDynamicRenderingAttachment(const Location &location) const {
    // TODO: Add tests. This is never called by existing tests.
    // TODO: Check for opportunities to improve error message after this covered by the tests.
    bool skip = false;
    const auto &last_bound_state = cb_state_->GetLastBoundGraphics();
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return skip;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        HazardResult hazard = access_context.DetectHazard(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                          SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(location, output_location);
            const std::string error = error_messages_.Error(
                hazard, *this, location.function, sync_state_.FormatHandle(*attachment.view), "DynamicRenderingAttachmentError");
            skip |= sync_state_.SyncError(hazard.Hazard(), obj_list, loc.dot(vvl::Field::imageView), error);
        }
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            HazardResult hazard =
                access_context.DetectHazard(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                            SyncOrdering::kDepthStencilAttachment);
            // Depth stencil Hazard check
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());
                Location loc = attachment.GetLocation(location);
                const std::string error =
                    error_messages_.Error(hazard, *this, location.function, sync_state_.FormatHandle(*attachment.view),
                                          "DynamicRenderingAttachmentError");
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc.dot(vvl::Field::imageView), error);
            }
        }
    }

    return skip;
}

void CommandBufferAccessContext::RecordDrawAttachment(const ResourceUsageTag tag) {
    if (current_renderpass_context_) {
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_, tag);
    } else if (dynamic_rendering_info_) {
        RecordDrawDynamicRenderingAttachment(tag);
    }
}

void CommandBufferAccessContext::RecordDrawDynamicRenderingAttachment(ResourceUsageTag tag) {
    const auto &last_bound_state = cb_state_->GetLastBoundGraphics();
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        access_context.UpdateAccessState(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                         SyncOrdering::kColorAttachment, ResourceUsageTagEx{tag});
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            access_context.UpdateAccessState(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                             SyncOrdering::kDepthStencilAttachment, ResourceUsageTagEx{tag});
        }
    }
}

static VkImageAspectFlags GetAspectsToClear(VkImageAspectFlags clear_aspect_mask, const vvl::ImageView &attachment_view) {
    // Check if clear request is valid.
    const bool clear_color = (clear_aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0;
    const bool clear_depth = (clear_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
    const bool clear_stencil = (clear_aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
    if (!clear_color && !clear_depth && !clear_stencil) {
        return 0;  // nothing to clear
    }
    if (clear_color && (clear_depth || clear_stencil)) {
        return 0;  // according to spec it's not allowed
    }

    // View's aspect mask is used only for color attachment.
    // For depth/stencil attachment view aspect mask is ignored according to spec.
    const VkImageAspectFlags view_aspect_mask = attachment_view.normalized_subresource_range.aspectMask;

    // Collect aspects that should be cleared.
    VkImageAspectFlags aspects_to_clear = VK_IMAGE_ASPECT_NONE;
    if (clear_color && (view_aspect_mask & kColorAspects) != 0) {
        assert(GetBitSetCount(view_aspect_mask) == 1);
        aspects_to_clear |= view_aspect_mask;
    }
    if (clear_depth && vkuFormatHasDepth(attachment_view.create_info.format)) {
        aspects_to_clear |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clear_stencil && vkuFormatHasStencil(attachment_view.create_info.format)) {
        aspects_to_clear |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    return aspects_to_clear;
}

static std::optional<VkImageSubresourceRange> RestrictSubresourceRange(const VkImageSubresourceRange &normalized_subresource_range,
                                                                       const VkClearRect &clear_rect) {
    assert(normalized_subresource_range.layerCount != VK_REMAINING_ARRAY_LAYERS);  // contract of this function
    assert(clear_rect.layerCount != VK_REMAINING_ARRAY_LAYERS);                    // according to spec
    const uint32_t first = std::max(normalized_subresource_range.baseArrayLayer, clear_rect.baseArrayLayer);
    const uint32_t last_range = normalized_subresource_range.baseArrayLayer + normalized_subresource_range.layerCount;
    const uint32_t last_clear = clear_rect.baseArrayLayer + clear_rect.layerCount;
    const uint32_t last = std::min(last_range, last_clear);

    if (first >= last) {
        return {};
    }

    std::optional<VkImageSubresourceRange> result;
    result = normalized_subresource_range;
    result->baseArrayLayer = first;
    result->layerCount = last - first;
    return result;
}

std::optional<CommandBufferAccessContext::ClearAttachmentInfo> CommandBufferAccessContext::GetClearAttachmentInfo(
    const VkClearAttachment &clear_attachment, const VkClearRect &rect) const {
    const vvl::ImageView *attachment_view = nullptr;
    if (current_renderpass_context_) {
        attachment_view = current_renderpass_context_->GetClearAttachmentView(clear_attachment);
    } else if (dynamic_rendering_info_) {
        attachment_view = dynamic_rendering_info_->GetClearAttachmentView(clear_attachment);
    }
    if (!attachment_view) {
        return {};
    }
    const VkImageAspectFlags aspects = GetAspectsToClear(clear_attachment.aspectMask, *attachment_view);
    if (!aspects) {
        return {};
    }
    const auto subresource_range = RestrictSubresourceRange(attachment_view->normalized_subresource_range, rect);
    if (!subresource_range.has_value()) {
        return {};
    }
    return ClearAttachmentInfo{*attachment_view, aspects, *subresource_range};
}

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment,
                                                         uint32_t clear_rect_index, const VkClearRect &clear_rect) const {
    bool skip = false;

    const auto optional_info = GetClearAttachmentInfo(clear_attachment, clear_rect);
    if (!optional_info) {
        return skip;
    }
    const ClearAttachmentInfo &info = *optional_info;

    const VkOffset3D offset = CastTo3D(clear_rect.rect.offset);
    const VkExtent3D extent = CastTo3D(clear_rect.rect.extent);
    VkImageSubresourceRange subresource_range = info.subresource_range;

    if (info.aspects_to_clear & kColorAspects) {
        // [core validation check]: if COLOR_ASPECT is included then PLANE aspects are not allowed,
        // and if PLANE aspect is included then only one is allowed.
        assert(GetBitSetCount(info.aspects_to_clear) == 1);
        const VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(info.aspects_to_clear);
        subresource_range.aspectMask = aspect;

        HazardResult hazard = current_context_->DetectHazard(
            *info.attachment_view.image_state, subresource_range, offset, extent, info.attachment_view.is_depth_sliced,
            SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            std::stringstream ss;
            ss << string_VkImageAspectFlagBits(aspect);
            ss << " aspect of color attachment " << clear_attachment.colorAttachment;
            ss << " (" << sync_state_.FormatHandle(info.attachment_view) << ")";
            if (current_renderpass_context_) {
                ss << " in subpass " << current_renderpass_context_->GetCurrentSubpass();
            }
            const std::string resource_description = ss.str();
            const LogObjectList objlist(cb_state_->Handle(), info.attachment_view.Handle());
            const auto error = error_messages_.ClearAttachmentError(hazard, *this, loc.function, resource_description, aspect,
                                                                    clear_rect_index, clear_rect);
            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
        }
    }

    constexpr VkImageAspectFlagBits depth_stencil_aspects[2] = {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT};
    for (const VkImageAspectFlagBits aspect : depth_stencil_aspects) {
        if (info.aspects_to_clear & aspect) {
            // Original aspect mask can contain both stencil and depth but here we track each aspect separately
            subresource_range.aspectMask = aspect;

            // vkCmdClearAttachments depth/stencil writes are executed by the EARLY_FRAGMENT_TESTS_BIT and LATE_FRAGMENT_TESTS_BIT
            // stages. The implementation tracks the most recent access, which happens in the LATE_FRAGMENT_TESTS_BIT stage.
            HazardResult hazard = current_context_->DetectHazard(
                *info.attachment_view.image_state, info.subresource_range, offset, extent, info.attachment_view.is_depth_sliced,
                SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kDepthStencilAttachment);

            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << string_VkImageAspectFlagBits(aspect);
                ss << " aspect of depth-stencil attachment (";
                ss << sync_state_.FormatHandle(info.attachment_view) << ")";
                if (current_renderpass_context_) {
                    ss << " in subpass " << current_renderpass_context_->GetCurrentSubpass();
                }
                const std::string resource_description = ss.str();
                const LogObjectList objlist(cb_state_->Handle(), info.attachment_view.Handle());
                const auto error = error_messages_.ClearAttachmentError(hazard, *this, loc.function, resource_description, aspect,
                                                                        clear_rect_index, clear_rect);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment,
                                                       const VkClearRect &rect) {
    const auto optional_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (!optional_info) {
        return;
    }
    const ClearAttachmentInfo &info = *optional_info;

    const VkOffset3D offset = CastTo3D(rect.rect.offset);
    const VkExtent3D extent = CastTo3D(rect.rect.extent);
    auto subresource_range = info.subresource_range;

    // Original subresource range can include aspects that are not cleared, they should not be tracked
    subresource_range.aspectMask = info.aspects_to_clear;

    if (info.aspects_to_clear & kColorAspects) {
        assert((info.aspects_to_clear & kDepthStencilAspects) == 0);
        current_context_->UpdateAccessState(*info.attachment_view.image_state, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                            SyncOrdering::kColorAttachment, subresource_range, offset, extent,
                                            ResourceUsageTagEx{tag});
    } else {
        assert((info.aspects_to_clear & kColorAspects) == 0);
        current_context_->UpdateAccessState(
            *info.attachment_view.image_state, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
            SyncOrdering::kDepthStencilAttachment, subresource_range, offset, extent, ResourceUsageTagEx{tag});
    }
}

QueueId CommandBufferAccessContext::GetQueueId() const { return kQueueIdInvalid; }

ResourceUsageTag CommandBufferAccessContext::RecordBeginRenderPass(vvl::Func command, const vvl::RenderPass &rp_state,
                                                                   const VkRect2D &render_area,
                                                                   const std::vector<const vvl::ImageView *> &attachment_views) {
    // Create an access context the current renderpass.
    const auto barrier_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    AddCommandHandle(barrier_tag, rp_state.Handle());
    const auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);
    render_pass_contexts_.emplace_back(
        std::make_unique<RenderPassAccessContext>(rp_state, render_area, GetQueueFlags(), attachment_views, &cb_access_context_));
    current_renderpass_context_ = render_pass_contexts_.back().get();
    current_renderpass_context_->RecordBeginRenderPass(barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordNextSubpass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kStoreOp);
    AddCommandHandle(store_tag, current_renderpass_context_->GetRenderPassState()->Handle());

    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);

    current_renderpass_context_->RecordNextSubpass(store_tag, barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordEndRenderPass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kStoreOp);
    AddCommandHandle(store_tag, current_renderpass_context_->GetRenderPassState()->Handle());

    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);

    current_renderpass_context_->RecordEndRenderPass(&cb_access_context_, store_tag, barrier_tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    return barrier_tag;
}

void CommandBufferAccessContext::RecordDestroyEvent(vvl::Event *event_state) { GetCurrentEventsContext()->Destroy(event_state); }

void CommandBufferAccessContext::RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_cb_context) {
    const AccessContext *recorded_context = recorded_cb_context.GetCurrentAccessContext();
    assert(recorded_context);

    // Just run through the barriers ignoring the usage from the recorded context, as Resolve will overwrite outdated state
    const ResourceUsageTag base_tag = GetTagCount();
    for (const auto &sync_op : recorded_cb_context.GetSyncOps()) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        sync_op.sync_op->ReplayRecord(*this, base_tag + sync_op.tag);
    }

    ImportRecordedAccessLog(recorded_cb_context);
    ResolveExecutedCommandBuffer(*recorded_context, base_tag);
}

void CommandBufferAccessContext::ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    auto tag_offset = [offset](ResourceAccessState *access) { access->OffsetTag(offset); };
    GetCurrentAccessContext()->ResolveFromContext(tag_offset, recorded_context);
}

void CommandBufferAccessContext::ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context) {
    cbs_referenced_->emplace_back(recorded_context.GetCBStateShared());
    access_log_->insert(access_log_->end(), recorded_context.access_log_->cbegin(), recorded_context.access_log_->cend());

    // Adjust command indices for the log records added from recorded_context.
    const auto &recorded_label_commands = recorded_context.cb_state_->GetLabelCommands();
    const bool use_proxy = !proxy_label_commands_.empty();
    const auto &label_commands = use_proxy ? proxy_label_commands_ : cb_state_->GetLabelCommands();
    if (!label_commands.empty()) {
        assert(label_commands.size() >= recorded_label_commands.size());
        const uint32_t command_offset = static_cast<uint32_t>(label_commands.size() - recorded_label_commands.size());
        for (size_t i = 0; i < recorded_context.access_log_->size(); i++) {
            size_t index = (access_log_->size() - 1) - i;
            assert((*access_log_)[index].label_command_index != vvl::kU32Max);
            (*access_log_)[index].label_command_index += command_offset;
        }
    }
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    command_number_++;
    current_command_tag_ = access_log_->size();

    ResourceUsageRecord &record = access_log_->emplace_back(command, command_number_, subcommand, cb_state_, reset_count_);

    if (!cb_state_->GetLabelCommands().empty()) {
        record.label_command_index = static_cast<uint32_t>(cb_state_->GetLabelCommands().size() - 1);
    }
    CheckCommandTagDebugCheckpoint();
    return current_command_tag_;
}

ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    const ResourceUsageTag tag = access_log_->size();
    ResourceUsageRecord &record = access_log_->emplace_back(command, command_number_, subcommand, cb_state_, reset_count_);

    // By default copy handle range from the main command, but can be overwritten with AddSubcommandHandle.
    const auto &main_command_record = (*access_log_)[current_command_tag_];
    record.first_handle_index = main_command_record.first_handle_index;
    record.handle_count = main_command_record.handle_count;

    if (!cb_state_->GetLabelCommands().empty()) {
        record.label_command_index = static_cast<uint32_t>(cb_state_->GetLabelCommands().size() - 1);
    }
    return tag;
}

uint32_t CommandBufferAccessContext::AddHandle(const VulkanTypedHandle &typed_handle, uint32_t index) {
    const uint32_t handle_index = static_cast<uint32_t>(handles_.size());
    handles_.emplace_back(HandleRecord(typed_handle, index));
    sync_state_.stats.AddHandleRecord();
    return handle_index;
}

ResourceUsageTagEx CommandBufferAccessContext::AddCommandHandle(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle) {
    return AddCommandHandleIndexed(tag, typed_handle, vvl::kNoIndex32);
}

ResourceUsageTagEx CommandBufferAccessContext::AddCommandHandleIndexed(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle,
                                                                       uint32_t index) {
    assert(tag < access_log_->size());
    const uint32_t handle_index = AddHandle(typed_handle, index);
    // TODO: the following range check is not needed. Test and remove.
    if (tag < access_log_->size()) {
        auto &record = (*access_log_)[tag];
        if (record.first_handle_index == vvl::kNoIndex32) {
            record.first_handle_index = handle_index;
            record.handle_count = 1;
        } else {
            // assert that command handles occupy continuous range
            assert(handle_index - record.first_handle_index == record.handle_count);
            record.handle_count++;
        }
    }
    return {tag, handle_index};
}

void CommandBufferAccessContext::AddSubcommandHandleIndexed(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle,
                                                            uint32_t index) {
    assert(tag < access_log_->size());
    const uint32_t handle_index = AddHandle(typed_handle, index);
    // TODO: the following range check is not needed. Test and remove.
    if (tag < access_log_->size()) {
        auto &record = (*access_log_)[tag];
        const auto &main_command_record = (*access_log_)[current_command_tag_];
        if (record.first_handle_index == main_command_record.first_handle_index) {
            // override default behavior that subcommand references the same handles as the main command
            record.first_handle_index = handle_index;
            record.handle_count = 1;
        } else {
            // assert that command handles occupy continuous range
            assert(handle_index - record.first_handle_index == record.handle_count);
            record.handle_count++;
        }
    }
}

std::string CommandBufferAccessContext::GetDebugRegionName(const ResourceUsageRecord &record) const {
    const bool use_proxy = !proxy_label_commands_.empty();
    const auto &label_commands = use_proxy ? proxy_label_commands_ : cb_state_->GetLabelCommands();
    return vvl::CommandBuffer::GetDebugRegionName(label_commands, record.label_command_index);
}

void CommandBufferAccessContext::RecordSyncOp(SyncOpPointer &&sync_op) {
    auto tag = sync_op->Record(this);
    // As renderpass operations can have side effects on the command buffer access context,
    // update the sync operation to record these if any.
    sync_ops_.emplace_back(tag, std::move(sync_op));
}

// NOTE: debug location reporting feature works only for reproducible application sessions
// (it uses command number/reset count from the error message from the previous session).
// It's considered experimental and can be replaced with a better way to report syncval debug locations.
//
// Logs informational message when vulkan command stream reaches a specific location.
// The message can be intercepted by the reporting routines. For example, the message handler can trigger a breakpoint.
// The location can be specified through environment variables.
// VK_SYNCVAL_DEBUG_COMMAND_NUMBER: the command number
// VK_SYNCVAL_DEBUG_RESET_COUNT: (optional, default value is 1) command buffer reset count
// VK_SYNCVAL_DEBUG_CMDBUF_PATTERN: (optional, empty string by default) pattern to match command buffer debug name
void CommandBufferAccessContext::CheckCommandTagDebugCheckpoint() {
    auto get_cmdbuf_name = [](const DebugReport &debug_report, uint64_t cmdbuf_handle) {
        std::unique_lock<std::mutex> lock(debug_report.debug_output_mutex);
        std::string object_name = debug_report.GetUtilsObjectNameNoLock(cmdbuf_handle);
        if (object_name.empty()) {
            object_name = debug_report.GetMarkerObjectNameNoLock(cmdbuf_handle);
        }
        text::ToLower(object_name);
        return object_name;
    };
    if (sync_state_.debug_command_number == command_number_ && sync_state_.debug_reset_count == reset_count_) {
        const auto cmdbuf_name = get_cmdbuf_name(*sync_state_.debug_report, cb_state_->Handle().handle);
        const auto &pattern = sync_state_.debug_cmdbuf_pattern;
        const bool cmdbuf_match = pattern.empty() || (cmdbuf_name.find(pattern) != std::string::npos);
        if (cmdbuf_match) {
            sync_state_.LogInfo("SYNCVAL_DEBUG_COMMAND", LogObjectList(), Location(access_log_->back().command),
                                "Command stream has reached command #%" PRIu32 " in command buffer %s with reset count #%" PRIu32,
                                sync_state_.debug_command_number, sync_state_.FormatHandle(cb_state_->Handle()).c_str(),
                                sync_state_.debug_reset_count);
        }
    }
}

namespace syncval_state {

CommandBufferSubState::CommandBufferSubState(SyncValidator &dev, vvl::CommandBuffer &cb)
    : vvl::CommandBufferSubState(cb), access_context(dev, &cb) {
    access_context.SetSelfReference();
}

void CommandBufferSubState::End() {
    // For threads that are dedicated to recording command buffers but do not submit themselves,
    // the end of recording is a logical point to update memory stats
    access_context.GetSyncState().stats.UpdateMemoryStats();
}

void CommandBufferSubState::Destroy() {
    access_context.Destroy();  // must be first to clean up self references correctly.
}

void CommandBufferSubState::Reset(const Location &loc) { access_context.Reset(); }

void CommandBufferSubState::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &obj : invalid_nodes) {
        switch (obj->Type()) {
            case kVulkanObjectTypeEvent:
                access_context.RecordDestroyEvent(static_cast<vvl::Event *>(obj.get()));
                break;
            default:
                break;
        }
    }
}

void CommandBufferSubState::RecordCopyBuffer(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                             const VkBufferCopy *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_buffer_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_buffer_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        const ResourceAccessRange src_range = MakeRange(src_buffer_state, copy_region.srcOffset, copy_region.size);
        context->UpdateAccessState(src_buffer_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);

        const ResourceAccessRange dst_range = MakeRange(dst_buffer_state, copy_region.dstOffset, copy_region.size);
        context->UpdateAccessState(dst_buffer_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyBuffer2(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                              const VkBufferCopy2 *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_buffer_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_buffer_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        const ResourceAccessRange src_range = MakeRange(src_buffer_state, copy_region.srcOffset, copy_region.size);
        context->UpdateAccessState(src_buffer_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);

        const ResourceAccessRange dst_range = MakeRange(dst_buffer_state, copy_region.dstOffset, copy_region.size);
        context->UpdateAccessState(dst_buffer_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyImage(vvl::Image &src_image_state, vvl::Image &dst_image_state,
                                            VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                            const VkImageCopy *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent,
                                   src_tag_ex);
        context->UpdateAccessState(dst_image_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state,
                                             VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                             const VkImageCopy2 *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent,
                                   src_tag_ex);
        context->UpdateAccessState(dst_image_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout,
                                                    uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_buffer_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        ResourceAccessRange src_range =
            MakeRange(copy_region.bufferOffset, dst_image_state.GetBufferSizeFromCopyImage(copy_region));
        context->UpdateAccessState(src_buffer_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);

        context->UpdateAccessState(dst_image_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset, copy_region.imageExtent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage2(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout,
                                                     uint32_t region_count, const VkBufferImageCopy2 *regions,
                                                     const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_buffer_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        ResourceAccessRange src_range =
            MakeRange(copy_region.bufferOffset, dst_image_state.GetBufferSizeFromCopyImage(copy_region));
        context->UpdateAccessState(src_buffer_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);

        context->UpdateAccessState(dst_image_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset, copy_region.imageExtent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state,
                                                    VkImageLayout src_image_layout, uint32_t region_count,
                                                    const VkBufferImageCopy *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_buffer_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset, copy_region.imageExtent,
                                   src_tag_ex);

        ResourceAccessRange dst_range =
            MakeRange(copy_region.bufferOffset, src_image_state.GetBufferSizeFromCopyImage(copy_region));
        context->UpdateAccessState(dst_buffer_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer2(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state,
                                                     VkImageLayout src_image_layout, uint32_t region_count,
                                                     const VkBufferImageCopy2 *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_buffer_state.Handle());

    for (const auto &copy_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset, copy_region.imageExtent,
                                   src_tag_ex);

        ResourceAccessRange dst_range =
            MakeRange(copy_region.bufferOffset, src_image_state.GetBufferSizeFromCopyImage(copy_region));
        context->UpdateAccessState(dst_buffer_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordBlitImage(vvl::Image &src_image_state, vvl::Image &dst_image_state,
                                            VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                            const VkImageBlit *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &blit_region : vvl::make_span(regions, region_count)) {
        VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                             std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                             std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
        VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                             static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                             static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
        context->UpdateAccessState(src_image_state, SYNC_BLIT_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(blit_region.srcSubresource), offset, extent, src_tag_ex);

        offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                  std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                  std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
        extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                  static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                  static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
        context->UpdateAccessState(dst_image_state, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(blit_region.dstSubresource), offset, extent, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordBlitImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state,
                                             VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                             const VkImageBlit2 *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &blit_region : vvl::make_span(regions, region_count)) {
        VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                             std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                             std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
        VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                             static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                             static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
        context->UpdateAccessState(src_image_state, SYNC_BLIT_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(blit_region.srcSubresource), offset, extent, src_tag_ex);

        offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                  std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                  std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
        extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                  static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                  static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
        context->UpdateAccessState(dst_image_state, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(blit_region.dstSubresource), offset, extent, dst_tag_ex);
    }
}

void CommandBufferSubState::RecordResolveImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                               const VkImageResolve *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &resolve_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset, resolve_region.extent,
                                   src_tag_ex);
        context->UpdateAccessState(dst_image_state, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset, resolve_region.extent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordResolveImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                                const VkImageResolve2 *regions, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    auto src_tag_ex = access_context.AddCommandHandle(tag, src_image_state.Handle());
    auto dst_tag_ex = access_context.AddCommandHandle(tag, dst_image_state.Handle());

    for (const auto &resolve_region : vvl::make_span(regions, region_count)) {
        context->UpdateAccessState(src_image_state, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset, resolve_region.extent,
                                   src_tag_ex);
        context->UpdateAccessState(dst_image_state, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                   RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset, resolve_region.extent,
                                   dst_tag_ex);
    }
}

void CommandBufferSubState::RecordClearColorImage(vvl::Image &image_state, VkImageLayout, const VkClearColorValue *,
                                                  uint32_t range_count, const VkImageSubresourceRange *ranges,
                                                  const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();
    assert(context);

    access_context.AddCommandHandle(tag, image_state.Handle());

    for (uint32_t index = 0; index < range_count; index++) {
        const auto &range = ranges[index];
        context->UpdateAccessState(image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

void CommandBufferSubState::RecordClearDepthStencilImage(vvl::Image &image_state, VkImageLayout, const VkClearDepthStencilValue *,
                                                         uint32_t range_count, const VkImageSubresourceRange *ranges,
                                                         const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();
    assert(context);

    access_context.AddCommandHandle(tag, image_state.Handle());

    for (uint32_t index = 0; index < range_count; index++) {
        const auto &range = ranges[index];
        context->UpdateAccessState(image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

void CommandBufferSubState::RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment *pAttachments,
                                                   uint32_t rect_count, const VkClearRect *pRects, const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);

    for (const auto &attachment : vvl::make_span(pAttachments, attachment_count)) {
        for (const auto &rect : vvl::make_span(pRects, rect_count)) {
            access_context.RecordClearAttachment(tag, attachment, rect);
        }
    }
}

void CommandBufferSubState::RecordFillBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size,
                                             const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();
    assert(context);

    const ResourceAccessRange range = MakeRange(buffer_state, offset, size);
    const ResourceUsageTagEx tag_ex = access_context.AddCommandHandle(tag, buffer_state.Handle());
    context->UpdateAccessState(buffer_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
}

void CommandBufferSubState::RecordUpdateBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size,
                                               const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();
    assert(context);

    // VK_WHOLE_SIZE not allowed
    const ResourceAccessRange range = MakeRange(offset, size);
    const ResourceUsageTagEx tag_ex = access_context.AddCommandHandle(tag, buffer_state.Handle());
    context->UpdateAccessState(buffer_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
}

void CommandBufferSubState::RecordDecodeVideo(vvl::VideoSession &vs_state, const VkVideoDecodeInfoKHR &decode_info,
                                              const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    if (auto src_buffer = base.dev_data.Get<vvl::Buffer>(decode_info.srcBuffer)) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, decode_info.srcBufferOffset, decode_info.srcBufferRange);
        const ResourceUsageTagEx src_tag_ex = access_context.AddCommandHandle(tag, src_buffer->Handle());
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, SyncOrdering::kNonAttachment, src_range,
                                   src_tag_ex);
    }

    const auto *device_state = access_context.GetSyncState().device_state;
    auto dst_resource = vvl::VideoPictureResource(*device_state, decode_info.dstPictureResource);
    if (dst_resource) {
        context->UpdateAccessState(vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
    }

    if (decode_info.pSetupReferenceSlot != nullptr && decode_info.pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*device_state, *decode_info.pSetupReferenceSlot->pPictureResource);
        if (setup_resource && (setup_resource != dst_resource)) {
            context->UpdateAccessState(vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        if (decode_info.pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*device_state, *decode_info.pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, tag);
            }
        }
    }
}

void CommandBufferSubState::RecordEncodeVideo(vvl::VideoSession &vs_state, const VkVideoEncodeInfoKHR &encode_info,
                                              const Location &loc) {
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    if (auto src_buffer = base.dev_data.Get<vvl::Buffer>(encode_info.dstBuffer)) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, encode_info.dstBufferOffset, encode_info.dstBufferRange);
        const ResourceUsageTagEx src_tag_ex = access_context.AddCommandHandle(tag, src_buffer->Handle());
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, SyncOrdering::kNonAttachment, src_range,
                                   src_tag_ex);
    }

    const auto *device_state = access_context.GetSyncState().device_state;
    auto src_resource = vvl::VideoPictureResource(*device_state, encode_info.srcPictureResource);
    if (src_resource) {
        context->UpdateAccessState(vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
    }

    if (encode_info.pSetupReferenceSlot != nullptr && encode_info.pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*device_state, *encode_info.pSetupReferenceSlot->pPictureResource);
        if (setup_resource) {
            context->UpdateAccessState(vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        if (encode_info.pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*device_state, *encode_info.pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
            }
        }
    }

    if (encode_info.flags & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
        auto quantization_map_info = vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapInfoKHR>(encode_info.pNext);
        if (quantization_map_info) {
            auto image_view_state = base.dev_data.Get<vvl::ImageView>(quantization_map_info->quantizationMap);
            if (image_view_state) {
                VkOffset3D offset = {0, 0, 0};
                VkExtent3D extent = {quantization_map_info->quantizationMapExtent.width,
                                     quantization_map_info->quantizationMapExtent.height, 1};
                context->UpdateAccessState(*image_view_state, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, SyncOrdering::kOrderingNone,
                                           offset, extent, ResourceUsageTagEx{tag});
            }
        }
    }
}

void CommandBufferSubState::RecordCopyQueryPoolResults(vvl::QueryPool &pool_state, vvl::Buffer &dst_buffer_state,
                                                       uint32_t first_query, uint32_t query_count, VkDeviceSize dst_offset,
                                                       VkDeviceSize stride, VkQueryResultFlags flags, const Location &loc) {
    if (query_count == 0) {
        return;
    }
    const auto tag = access_context.NextCommandTag(loc.function);
    auto *context = access_context.GetCurrentAccessContext();

    const uint32_t query_size = (flags & VK_QUERY_RESULT_64_BIT) ? 8 : 4;
    const VkDeviceSize range_size = (query_count - 1) * stride + query_size;
    const ResourceAccessRange range = MakeRange(dst_offset, range_size);
    const ResourceUsageTagEx tag_ex = access_context.AddCommandHandle(tag, dst_buffer_state.Handle());
    context->UpdateAccessState(dst_buffer_state, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);

    // TODO:Track VkQueryPool
}

void CommandBufferSubState::RecordBeginRenderPass(const VkRenderPassBeginInfo &render_pass_begin,
                                                  const VkSubpassBeginInfo &subpass_begin_info, const Location &loc) {
    if (!base.IsPrimary()) {
        return;  // [core validation check]: only primary command buffer can begin render pass
    }
    access_context.RecordSyncOp<SyncOpBeginRenderPass>(loc.function, access_context.GetSyncState(), &render_pass_begin,
                                                       &subpass_begin_info);
}

void CommandBufferSubState::RecordNextSubpass(const VkSubpassBeginInfo &subpass_begin_info,
                                              const VkSubpassEndInfo *subpass_end_info, const Location &loc) {
    if (!base.IsPrimary()) {
        return;  // [core validation check]: only primary command buffer can start next subpass
    }
    access_context.RecordSyncOp<SyncOpNextSubpass>(loc.function, access_context.GetSyncState(), &subpass_begin_info,
                                                   subpass_end_info);
}

void CommandBufferSubState::RecordEndRenderPass(const VkSubpassEndInfo *subpass_end_info, const Location &loc) {
    if (!base.IsPrimary()) {
        return;  // [core validation check]: only primary command buffer can end render pass
    }
    // Resolve the all subpass contexts to the command buffer contexts
    access_context.RecordSyncOp<SyncOpEndRenderPass>(loc.function, access_context.GetSyncState(), subpass_end_info);
}

void CommandBufferSubState::RecordExecuteCommand(vvl::CommandBuffer &secondary_command_buffer, uint32_t cmd_index,
                                                 const Location &loc) {
    const auto subcommand = ResourceUsageRecord::SubcommandType::kIndex;
    if (cmd_index == 0) {
        ResourceUsageTag cb_tag = access_context.NextCommandTag(loc.function, subcommand);
        access_context.AddCommandHandleIndexed(cb_tag, secondary_command_buffer.Handle(), cmd_index);
    } else {
        ResourceUsageTag cb_tag = access_context.NextSubcommandTag(loc.function, subcommand);
        access_context.AddSubcommandHandleIndexed(cb_tag, secondary_command_buffer.Handle(), cmd_index);
    }
    access_context.RecordExecutedCommandBuffer(*syncval_state::AccessContext(secondary_command_buffer));
}

}  // namespace syncval_state