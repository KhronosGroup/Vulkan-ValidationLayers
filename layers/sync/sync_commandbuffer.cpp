/*
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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

#include "sync/sync_commandbuffer.h"
#include "sync/sync_op.h"
#include "sync/sync_validation.h"

SyncStageAccessIndex GetSyncStageAccessIndexsByDescriptorSet(VkDescriptorType descriptor_type,
                                                             const spirv::ResourceInterfaceVariable &variable,
                                                             VkShaderStageFlagBits stage_flag) {
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        assert(stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT);
        return SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
    }
    const auto stage_accesses = sync_utils::GetShaderStageAccesses(stage_flag);

    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        return stage_accesses.uniform_read;
    }

    // Detect if variable is nonreadable (writeonly in glsl).
    // NOTE: is_written_to can be used as a more general case instead of adding is_writeonly logic.
    // At first we need to fix is_written_to support for buffers.
    bool is_writeonly = variable.decorations.Has(spirv::DecorationBase::nonreadable_bit);
    if (variable.type_struct_info) {
        is_writeonly |= variable.type_struct_info->decorations.AllMemberHave(spirv::DecorationBase::nonreadable_bit);
    }

    // If the desriptorSet is writable, we don't need to care SHADER_READ. SHADER_WRITE is enough.
    // Because if write hazard happens, read hazard might or might not happen.
    // But if write hazard doesn't happen, read hazard is impossible to happen.
    if (variable.is_written_to || is_writeonly) {
        return stage_accesses.storage_write;
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
        return stage_accesses.sampled_read;
    } else {
        return stage_accesses.storage_read;
    }
}

ResourceUsageRange CommandExecutionContext::ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context) {
    // The execution references ensure lifespan for the referenced child CB's...
    ResourceUsageRange tag_range(GetTagLimit(), 0);
    InsertRecordedAccessLogEntries(recorded_context);
    tag_range.end = GetTagLimit();
    return tag_range;
}

bool CommandExecutionContext::ValidForSyncOps() const {
    const bool valid = GetCurrentEventsContext() && GetCurrentAccessContext();
    assert(valid);
    return valid;
}

CommandBufferAccessContext::CommandBufferAccessContext(const SyncValidator *sync_validator)
    : CommandExecutionContext(sync_validator),
      cb_state_(),
      access_log_(std::make_shared<AccessLog>()),
      cbs_referenced_(std::make_shared<CommandBufferSet>()),
      command_number_(0),
      subcommand_number_(0),
      reset_count_(0),
      cb_access_context_(),
      current_context_(&cb_access_context_),
      events_context_(),
      render_pass_contexts_(),
      current_renderpass_context_(),
      sync_ops_() {}

// NOTE: Make sure the proxy doesn't outlive from, as the proxy is pointing directly to access contexts owned by from.
CommandBufferAccessContext::CommandBufferAccessContext(const CommandBufferAccessContext &from, AsProxyContext dummy)
    : CommandBufferAccessContext(from.sync_state_) {
    // Copy only the needed fields out of from for a temporary, proxy command buffer context
    cb_state_ = from.cb_state_;
    access_log_ = std::make_shared<AccessLog>(*from.access_log_);  // potentially large, but no choice given tagging lookup.
    command_number_ = from.command_number_;
    subcommand_number_ = from.subcommand_number_;
    reset_count_ = from.reset_count_;

    const auto *from_context = from.GetCurrentAccessContext();
    assert(from_context);

    // Construct a fully resolved single access context out of from
    cb_access_context_.ResolveFromContext(*from_context);
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;
    debug_regions_ = from.debug_regions_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
}

void CommandBufferAccessContext::Reset() {
    access_log_ = std::make_shared<AccessLog>();
    cbs_referenced_ = std::make_shared<CommandBufferSet>();
    if (cb_state_) {
        cbs_referenced_->insert(cb_state_->shared_from_this());
    }
    sync_ops_.clear();
    command_number_ = 0;
    subcommand_number_ = 0;
    reset_count_++;
    command_handles_.clear();
    cb_access_context_.Reset();
    render_pass_contexts_.clear();
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    events_context_.Clear();
    dynamic_rendering_info_.reset();
    debug_regions_ = DebugRegions{};
}

std::string CommandBufferAccessContext::FormatUsage(const ResourceUsageTag tag) const {
    if (tag >= access_log_->size()) return std::string();

    std::stringstream out;
    assert(tag < access_log_->size());
    const auto &record = (*access_log_)[tag];
    out << record.Formatter(*sync_state_, cb_state_);
    return out.str();
}

std::string CommandBufferAccessContext::FormatUsage(const char *usage_string, const ResourceFirstAccess &access) const {
    std::stringstream out;
    assert(access.usage_info);
    out << "(" << usage_string << ": " << access.usage_info->name;
    out << ", " << FormatUsage(access.tag) << ")";
    return out.str();
}

bool CommandBufferAccessContext::ValidateBeginRendering(const ErrorObject &error_obj,
                                                        syncval_state::BeginRenderingCmdState &cmd_state) const {
    bool skip = false;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();

    // Load operations do not happen when resuming
    if (info.info.flags & VK_RENDERING_RESUMING_BIT) return skip;

    HazardResult hazard;

    // Need to hazard detect load operations vs. the attachment views
    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = 0; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        const SyncStageAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

        hazard = GetCurrentAccessContext()->DetectHazard(attachment.view_gen, load_index, attachment.GetOrdering());
        if (hazard.IsHazard()) {
            LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(error_obj.location, i);
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                          "(%s), with loadOp %s. Access info %s.",
                                          sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                          string_VkAttachmentLoadOp(attachment.info.loadOp), FormatHazard(hazard).c_str());
            if (skip) break;
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state,
                                                      const RecordObject &record_obj) {
    using Attachment = syncval_state::DynamicRenderingInfo::Attachment;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();
    const auto tag = NextCommandTag(record_obj.location.function);

    // Only load if not resuming
    if (0 == (info.info.flags & VK_RENDERING_RESUMING_BIT)) {
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        for (uint32_t i = 0; i < attachment_count; i++) {
            const Attachment &attachment = info.attachments[i];
            const SyncStageAccessIndex load_index = attachment.GetLoadUsage();
            if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

            GetCurrentAccessContext()->UpdateAccessState(attachment.view_gen, load_index, attachment.GetOrdering(), tag);
        }
    }

    dynamic_rendering_info_ = std::move(cmd_state.info);
}

bool CommandBufferAccessContext::ValidateEndRendering(const ErrorObject &error_obj) const {
    bool skip = false;
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        // Only validate resolve and store if not suspending (as specified by BeginRendering)
        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        const AccessContext *access_context = GetCurrentAccessContext();
        assert(access_context);
        auto report_resolve_hazard = [this](const HazardResult &hazard, const Location &loc, const VulkanTypedHandle image_handle,
                                            const VkResolveModeFlagBits resolve_mode) {
            LogObjectList obj_list(cb_state_->Handle(), image_handle);
            return sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc,
                                         "(%s), during resolve with resolveMode %s. Access info %s.",
                                         sync_state_->FormatHandle(image_handle).c_str(),
                                         string_VkResolveModeFlagBits(resolve_mode), FormatHazard(hazard).c_str());
        };

        for (uint32_t i = 0; i < attachment_count && !skip; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                // The logic about whether to resolve is embedded in the Attachment constructor
                assert(attachment.view);
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, kResolveRead, kResolveOrder);

                if (hazard.IsHazard()) {
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::imageView), attachment.view->Handle(),
                                                  attachment.info.resolveMode);
                }
                if (!skip) {
                    hazard = access_context->DetectHazard(*attachment.resolve_gen, kResolveWrite, kResolveOrder);
                    if (hazard.IsHazard()) {
                        Location loc = attachment.GetLocation(error_obj.location, i);
                        skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::resolveImageView),
                                                      attachment.resolve_view->Handle(), attachment.info.resolveMode);
                    }
                }
            }

            const auto store_usage = attachment.GetStoreUsage();
            if (store_usage != SYNC_ACCESS_INDEX_NONE) {
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, store_usage, kStoreOrder);
                if (hazard.IsHazard()) {
                    const VulkanTypedHandle image_handle = attachment.view->Handle();
                    LogObjectList obj_list(cb_state_->Handle(), image_handle);
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    skip |= sync_state_->LogError(
                        string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                        "(%s), during store with storeOp %s. Access info %s.", sync_state_->FormatHandle(image_handle).c_str(),
                        string_VkAttachmentStoreOp(attachment.info.storeOp), FormatHazard(hazard).c_str());
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
                access_context->UpdateAccessState(attachment.view_gen, kResolveRead, kResolveOrder, store_tag);
                access_context->UpdateAccessState(*attachment.resolve_gen, kResolveWrite, kResolveOrder, store_tag);
            }

            const SyncStageAccessIndex store_index = attachment.GetStoreUsage();
            if (store_index == SYNC_ACCESS_INDEX_NONE) continue;
            access_context->UpdateAccessState(attachment.view_gen, store_index, kStoreOrder, store_tag);
        }
    }

    dynamic_rendering_info_.reset();
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const Location &loc) const {
    bool skip = false;
    const vvl::Pipeline *pipe = nullptr;
    const std::vector<LastBound::PER_SET> *per_sets = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &per_sets);
    if (!pipe || !per_sets) {
        return skip;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= per_sets->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto *descriptor_set = (*per_sets)[variable.decorations.set].bound_descriptor_set.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

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
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        VkImageLayout image_layout = image_descriptor->GetImageLayout();

                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }

                        HazardResult hazard;

                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.offset);
                            // Input attachments are subject to raster ordering rules
                            hazard =
                                current_context_->DetectHazard(*img_view_state, offset, extent, sync_index, SyncOrdering::kRaster);
                        } else {
                            hazard = current_context_->DetectHazard(*img_view_state, sync_index);
                        }

                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), img_view_state->image_view(), loc,
                                "Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32
                                ", index %" PRIu32 ". Access info %s.",
                                string_SyncHazard(hazard.Hazard()), sync_state_->FormatHandle(img_view_state->image_view()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
                                string_VkDescriptorType(descriptor_type), string_VkImageLayout(image_layout),
                                variable.decorations.binding, index, FormatHazard(hazard).c_str());
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
                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), buf_view_state->buffer_view(), loc,
                                "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                string_SyncHazard(hazard.Hazard()),
                                sync_state_->FormatHandle(buf_view_state->buffer_view()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
                                string_VkDescriptorType(descriptor_type), variable.decorations.binding, index,
                                FormatHazard(hazard).c_str());
                        }
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                string_SyncHazard(hazard.Hazard()), sync_state_->FormatHandle(buf_state->buffer()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
                                string_VkDescriptorType(descriptor_type), variable.decorations.binding, index,
                                FormatHazard(hazard).c_str());
                        }
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                 const ResourceUsageTag tag) {
    const vvl::Pipeline *pipe = nullptr;
    const std::vector<LastBound::PER_SET> *per_sets = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &per_sets);
    if (!pipe || !per_sets) {
        return;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= per_sets->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto *descriptor_set = (*per_sets)[variable.decorations.set].bound_descriptor_set.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

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
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.offset);
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kRaster, offset, extent,
                                                                tag);
                        } else {
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kNonAttachment, tag);
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
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag);
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag);
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex,
                                                    const Location &loc) const {
    bool skip = false;
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return skip;
    }

    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pipe->vertex_input_state->binding_descriptions.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pipe->vertex_input_state->binding_descriptions[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (!binding_buffer.bound()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = MakeRange(binding_buffer, firstVertex, vertexCount, binding_description.stride);
            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.IsHazard()) {
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                              "Hazard %s for vertex %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                              sync_state_->FormatHandle(buf_state->buffer()).c_str(),
                                              sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                              FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex,
                                                  const ResourceUsageTag tag) {
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return;
    }
    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pipe->vertex_input_state->binding_descriptions.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pipe->vertex_input_state->binding_descriptions[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (!binding_buffer.bound()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = MakeRange(binding_buffer, firstVertex, vertexCount, binding_description.stride);
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ,
                                                SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(const std::optional<uint32_t> &index_count, uint32_t firstIndex,
                                                         const Location &loc) const {
    bool skip = false;
    if (!cb_state_->index_buffer_binding.bound()) {
        return skip;
    }

    const auto &index_binding = cb_state_->index_buffer_binding;
    auto *index_buf_state = index_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding, firstIndex, index_count, index_size);

    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, range);
    if (hazard.IsHazard()) {
        skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), index_buf_state->buffer(), loc,
                                      "Hazard %s for index %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                      sync_state_->FormatHandle(index_buf_state->buffer()).c_str(),
                                      sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(), FormatHazard(hazard).c_str());
    }

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    skip |= ValidateDrawVertex(std::optional<uint32_t>(), 0, loc);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(const std::optional<uint32_t> &indexCount, uint32_t firstIndex,
                                                       const ResourceUsageTag tag) {
    if (!cb_state_->index_buffer_binding.bound()) return;

    const auto &index_binding = cb_state_->index_buffer_binding;
    auto *index_buf_state = index_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding, firstIndex, indexCount, index_size);
    current_context_->UpdateAccessState(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, SyncOrdering::kNonAttachment, range, tag);

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawAttachment(const Location &loc) const {
    bool skip = false;
    if (current_renderpass_context_) {
        skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(GetExecutionContext(), *cb_state_, loc.function);
    } else if (dynamic_rendering_info_) {
        skip |= ValidateDrawDynamicRenderingAttachment(loc);
    }
    return skip;
}

bool CommandBufferAccessContext::ValidateDrawDynamicRenderingAttachment(const Location &location) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) {
        return skip;
    }

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) {
        return skip;
    }

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
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                          "(%s). Access info %s.", sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                          FormatHazard(hazard).c_str());
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
                LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
                Location loc = attachment.GetLocation(location);
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                              "(%s). Access info %s.", sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                              FormatHazard(hazard).c_str());
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
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) return;

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) return;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        access_context.UpdateAccessState(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                         SyncOrdering::kColorAttachment, tag);
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
                                             SyncOrdering::kDepthStencilAttachment, tag);
        }
    }
}

ClearAttachmentInfo CommandBufferAccessContext::GetClearAttachmentInfo(const VkClearAttachment &clear_attachment,
                                                                       const VkClearRect &rect) const {
    // This is a NOOP if there's no renderpass nor dynamic rendering
    // Caller must used "IsValid" to determine if clear_info contains meaningful information.
    ClearAttachmentInfo clear_info;
    if (current_renderpass_context_) {
        clear_info = current_renderpass_context_->GetClearAttachmentInfo(clear_attachment, rect);
    } else if (dynamic_rendering_info_) {
        clear_info = dynamic_rendering_info_->GetClearAttachmentInfo(clear_attachment, rect);
    }

    return clear_info;
}

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment,
                                                         const VkClearRect &rect) const {
    bool skip = false;

    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        skip = ValidateClearAttachment(loc, clear_info);
    }

    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment,
                                                       const VkClearRect &rect) {
    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        RecordClearAttachment(tag, clear_info);
    }
}

QueueId CommandBufferAccessContext::GetQueueId() const { return kQueueIdInvalid; }

ResourceUsageTag CommandBufferAccessContext::RecordBeginRenderPass(
    vvl::Func command, const vvl::RenderPass &rp_state, const VkRect2D &render_area,
    const std::vector<const syncval_state::ImageViewState *> &attachment_views) {
    // Create an access context the current renderpass.
    const auto barrier_tag = NextCommandTag(command, NamedHandle("renderpass", rp_state.Handle()),
                                            ResourceUsageRecord::SubcommandType::kSubpassTransition);
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

    auto store_tag = NextCommandTag(command, NamedHandle("renderpass", current_renderpass_context_->GetRenderPassState()->Handle()),
                                    ResourceUsageRecord::SubcommandType::kStoreOp);
    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);

    current_renderpass_context_->RecordNextSubpass(store_tag, barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordEndRenderPass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, NamedHandle("renderpass", current_renderpass_context_->GetRenderPassState()->Handle()),
                                    ResourceUsageRecord::SubcommandType::kStoreOp);
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
    const ResourceUsageTag base_tag = GetTagLimit();
    for (const auto &sync_op : recorded_cb_context.GetSyncOps()) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        sync_op.sync_op->ReplayRecord(*this, base_tag + sync_op.tag);
    }

    ResourceUsageRange tag_range = ImportRecordedAccessLog(recorded_cb_context);
    assert(base_tag == tag_range.begin);  // to ensure the to offset calculation agree
    ResolveExecutedCommandBuffer(*recorded_context, tag_range.begin);
}

void CommandBufferAccessContext::ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    auto tag_offset = [offset](ResourceAccessState *access) { access->OffsetTag(offset); };
    GetCurrentAccessContext()->ResolveFromContext(tag_offset, recorded_context);
}

void CommandBufferAccessContext::InsertRecordedAccessLogEntries(const CommandBufferAccessContext &recorded_context) {
    cbs_referenced_->emplace(recorded_context.GetCBStateShared());
    access_log_->insert(access_log_->end(), recorded_context.access_log_->cbegin(), recorded_context.access_log_->cend());
}

ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    return NextSubcommandTag(command, NamedHandle(), subcommand);
}
ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, NamedHandle &&handle,
                                                               ResourceUsageRecord::SubcommandType subcommand) {
    ResourceUsageTag next = access_log_->size();
    access_log_->emplace_back(command, command_number_, subcommand, ++subcommand_number_, cb_state_, reset_count_);
    if (command_handles_.size()) {
        // This is a duplication, but it keeps tags->log information flat (i.e not depending on some "command tag" entry
        access_log_->back().handles = command_handles_;
    }
    if (handle) {
        access_log_->back().AddHandle(std::move(handle));
    }
    if (!debug_regions_.commands.empty()) {
        access_log_->back().debug_region_command_index = static_cast<uint32_t>(debug_regions_.commands.size() - 1);
    }
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    return NextCommandTag(command, NamedHandle(), subcommand);
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, NamedHandle &&handle,
                                                            ResourceUsageRecord::SubcommandType subcommand) {
    command_number_++;
    command_handles_.clear();
    subcommand_number_ = 0;
    ResourceUsageTag next = access_log_->size();
    access_log_->emplace_back(command, command_number_, subcommand, subcommand_number_, cb_state_, reset_count_);
    if (handle) {
        access_log_->back().AddHandle(handle);
        command_handles_.emplace_back(std::move(handle));
    }
    if (!debug_regions_.commands.empty()) {
        access_log_->back().debug_region_command_index = static_cast<uint32_t>(debug_regions_.commands.size() - 1);
    }
    CheckCommandTagDebugCheckpoint();
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextIndexedCommandTag(vvl::Func command, uint32_t index) {
    if (index == 0) {
        return NextCommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
    }
    return NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
}

void CommandBufferAccessContext::PushDebugRegion(const char *region_name) {
    std::string name = (region_name == nullptr || region_name[0] == 0) ? "(unnamed debug region)" : region_name;
    debug_regions_.label_names.push_back(std::move(name));

    DebugRegions::DebugRegionCommand command;
    command.start_region = true;
    command.label_name_index = static_cast<uint32_t>(debug_regions_.label_names.size() - 1);
    debug_regions_.commands.push_back(command);
}

void CommandBufferAccessContext::PopDebugRegion() {
    DebugRegions::DebugRegionCommand command;
    command.start_region = false;
    debug_regions_.commands.push_back(command);
}

std::string CommandBufferAccessContext::GetDebugRegionFullyQualifiedName(uint32_t debug_region_command_index) const {
    return debug_regions_.GetDebugRegionFullyQualifiedName(debug_region_command_index);
}

std::string CommandBufferAccessContext::DebugRegions::GetDebugRegionFullyQualifiedName(uint32_t debug_region_command_index) const {
    // Replay commands up to specified command index. nested_label_indices will be populated
    // with nested debug regions that correspond to the location of the specified command.
    assert(debug_region_command_index < commands.size());
    std::vector<uint32_t> nested_label_indices;
    for (uint32_t i = 0; i <= debug_region_command_index; i++) {
        if (commands[i].start_region) {
            assert(commands[i].label_name_index < label_names.size());
            nested_label_indices.push_back(commands[i].label_name_index);
        } else {
            nested_label_indices.pop_back();
        }
    }
    // Concatenate nested region names
    std::string name;
    for (uint32_t label_index : nested_label_indices) {
        if (!name.empty()) {
            name += "::";
        }
        name += label_names[label_index];
    }
    return name;
}

void CommandBufferAccessContext::RecordSyncOp(SyncOpPointer &&sync_op) {
    auto tag = sync_op->Record(this);
    // As renderpass operations can have side effects on the command buffer access context,
    // update the sync operation to record these if any.
    sync_ops_.emplace_back(tag, std::move(sync_op));
}

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const {
    bool skip = false;
    VkImageSubresourceRange subresource_range = info.subresource_range;
    const AccessContext *access_context = GetCurrentAccessContext();
    assert(access_context);
    if (info.aspects_to_clear & kColorAspects) {
        assert(GetBitSetCount(info.aspects_to_clear) == 1);
        subresource_range.aspectMask = info.aspects_to_clear;

        HazardResult hazard = access_context->DetectHazard(
            *info.view->GetImageState(), subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
            SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                                          "Hazard %s while clearing color attachment%s. Access info %s.",
                                          string_SyncHazard(hazard.Hazard()), info.GetSubpassAttachmentText().c_str(),
                                          FormatHazard(hazard).c_str());
        }
    }

    constexpr VkImageAspectFlagBits depth_stencil_aspects[2] = {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT};
    for (const auto aspect : depth_stencil_aspects) {
        if (info.aspects_to_clear & aspect) {
            // Original aspect mask can contain both stencil and depth but here we track each aspect separately
            subresource_range.aspectMask = aspect;

            // vkCmdClearAttachments depth/stencil writes are executed by the EARLY_FRAGMENT_TESTS_BIT and LATE_FRAGMENT_TESTS_BIT
            // stages. The implementation tracks the most recent access, which happens in the LATE_FRAGMENT_TESTS_BIT stage.
            HazardResult hazard = access_context->DetectHazard(
                *info.view->GetImageState(), info.subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
                SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kDepthStencilAttachment);

            if (hazard.IsHazard()) {
                const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                                              "Hazard %s when clearing %s aspect of depth-stencil attachment%s. Access info %s.",
                                              string_SyncHazard(hazard.Hazard()), string_VkImageAspectFlagBits(aspect),
                                              info.GetSubpassAttachmentText().c_str(), FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info) {
    auto subresource_range = clear_info.subresource_range;

    // Original subresource range can include aspects that are not cleared, they should not be tracked
    subresource_range.aspectMask = clear_info.aspects_to_clear;
    AccessContext *access_context = GetCurrentAccessContext();

    if (clear_info.aspects_to_clear & kColorAspects) {
        assert((clear_info.aspects_to_clear & kDepthStencilAspects) == 0);
        access_context->UpdateAccessState(*clear_info.view->GetImageState(), SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                          SyncOrdering::kColorAttachment, subresource_range, clear_info.offset, clear_info.extent,
                                          tag);
    } else {
        assert((clear_info.aspects_to_clear & kColorAspects) == 0);
        access_context->UpdateAccessState(
            *clear_info.view->GetImageState(), SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
            SyncOrdering::kDepthStencilAttachment, subresource_range, clear_info.offset, clear_info.extent, tag);
    }
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
    auto get_cmdbuf_name = [](const debug_report_data &debug_report, uint64_t cmdbuf_handle) {
        std::unique_lock<std::mutex> lock(debug_report.debug_output_mutex);
        std::string object_name = debug_report.DebugReportGetUtilsObjectNameNoLock(cmdbuf_handle);
        if (object_name.empty()) {
            object_name = debug_report.DebugReportGetMarkerObjectNameNoLock(cmdbuf_handle);
        }
        vvl::ToLower(object_name);
        return object_name;
    };
    if (sync_state_->debug_command_number == command_number_ && sync_state_->debug_reset_count == reset_count_) {
        const auto cmdbuf_name = get_cmdbuf_name(*sync_state_->report_data, cb_state_->Handle().handle);
        const auto &pattern = sync_state_->debug_cmdbuf_pattern;
        const bool cmdbuf_match = pattern.empty() || (cmdbuf_name.find(pattern) != std::string::npos);
        if (cmdbuf_match) {
            sync_state_->LogInfo("SYNCVAL_DEBUG_COMMAND", LogObjectList(), Location(access_log_->back().command),
                                 "Command stream has reached command #%" PRIu32 " in command buffer %s with reset count #%" PRIu32,
                                 sync_state_->debug_command_number, sync_state_->FormatHandle(cb_state_->Handle()).c_str(),
                                 sync_state_->debug_reset_count);
        }
    }
}

static bool IsHazardVsRead(SyncHazard hazard) {
    bool vs_read = false;
    switch (hazard) {
        case SyncHazard::WRITE_AFTER_READ:
            vs_read = true;
            break;
        case SyncHazard::WRITE_RACING_READ:
            vs_read = true;
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            vs_read = true;
            break;
        default:
            break;
    }
    return vs_read;
}

static const SyncStageAccessInfoType *SyncStageAccessInfoFromMask(SyncStageAccessFlags flags) {
    // Return the info for the first bit found
    const SyncStageAccessInfoType *info = nullptr;
    for (size_t i = 0; i < flags.size(); i++) {
        if (flags.test(i)) {
            info = &syncStageAccessInfoByStageAccessIndex()[i];
            break;
        }
    }
    return info;
}

static std::string string_SyncStageAccessFlags(const SyncStageAccessFlags &flags, const char *sep = "|") {
    std::string out_str;
    if (flags.none()) {
        out_str = "0";
    } else {
        for (size_t i = 0; i < syncStageAccessInfoByStageAccessIndex().size(); i++) {
            const auto &info = syncStageAccessInfoByStageAccessIndex()[i];
            if ((flags & info.stage_access_bit).any()) {
                if (!out_str.empty()) {
                    out_str.append(sep);
                }
                out_str.append(info.name);
            }
        }
        if (out_str.length() == 0) {
            out_str.append("Unhandled SyncStageAccess");
        }
    }
    return out_str;
}

std::ostream &operator<<(std::ostream &out, const SyncNodeFormatter &formatter) {
    if (formatter.label) {
        out << formatter.label << ": ";
    }
    if (formatter.node) {
        out << formatter.report_data->FormatHandle(*formatter.node).c_str();
        if (formatter.node->Destroyed()) {
            out << " (destroyed)";
        }
    } else {
        out << "null handle";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const NamedHandle::FormatterState &formatter) {
    const NamedHandle &handle = formatter.that;
    bool labeled = false;
    if (!handle.name.empty()) {
        out << handle.name;
        labeled = true;
    }
    if (handle.IsIndexed()) {
        out << "[" << handle.index << "]";
        labeled = true;
    }
    if (labeled) {
        out << ": ";
    }
    out << formatter.state.FormatHandle(handle.handle);
    return out;
}

std::ostream &operator<<(std::ostream &out, const ResourceUsageRecord::FormatterState &formatter) {
    const ResourceUsageRecord &record = formatter.record;
    if (record.alt_usage) {
        out << record.alt_usage.Formatter(formatter.sync_state);
    } else {
        out << "command: " << vvl::String(record.command);
        // Note: ex_cb_state set to null forces output of record.cb_state
        if (!formatter.ex_cb_state || (formatter.ex_cb_state != record.cb_state)) {
            out << ", " << SyncNodeFormatter(formatter.sync_state, record.cb_state);
        }
        out << ", seq_no: " << record.seq_num;
        if (record.sub_command != 0) {
            out << ", subcmd: " << record.sub_command;
        }
        for (const auto &named_handle : record.handles) {
            out << ", " << named_handle.Formatter(formatter.sync_state);
        }
        out << ", reset_no: " << std::to_string(record.reset_count);

        if (record.debug_region_command_index != vvl::kU32Max) {
            const auto &access_context = static_cast<const syncval_state::CommandBuffer *>(record.cb_state)->access_context;
            const std::string region_name = access_context.GetDebugRegionFullyQualifiedName(record.debug_region_command_index);
            // Empty region name means that we are not inside any debug region.
            // If we are inside debug region with an empty name then it will reported as "(unnamed debug region)".
            if (!region_name.empty()) {
                out << ", debug region: " << region_name;
            }
        }
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const HazardResult::HazardState &hazard) {
    assert(hazard.usage_index < static_cast<SyncStageAccessIndex>(syncStageAccessInfoByStageAccessIndex().size()));
    const auto &usage_info = syncStageAccessInfoByStageAccessIndex()[hazard.usage_index];
    const auto *info = SyncStageAccessInfoFromMask(hazard.prior_access);
    const char *stage_access_name = info ? info->name : "INVALID_STAGE_ACCESS";
    out << "(";
    if (!hazard.recorded_access.get()) {
        // if we have a recorded usage the usage is reported from the recorded contexts point of view
        out << "usage: " << usage_info.name << ", ";
    }
    out << "prior_usage: " << stage_access_name;
    if (IsHazardVsRead(hazard.hazard)) {
        const auto barriers = hazard.access_state->GetReadBarriers(hazard.prior_access);
        out << ", read_barriers: " << string_VkPipelineStageFlags2(barriers);
    } else {
        SyncStageAccessFlags write_barrier = hazard.access_state->GetWriteBarriers();
        out << ", write_barriers: " << string_SyncStageAccessFlags(write_barrier);
    }
    return out;
}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state)
    : report_data(sync_state.report_data), node(cb_state), label("command_buffer") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image)
    : report_data(sync_state.report_data), node(image), label("image") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state)
    : report_data(sync_state.report_data), node(q_state), label("queue") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_)
    : report_data(sync_state.report_data), node(state_object), label(label_) {}

std::string SyncValidationInfo::FormatHazard(const HazardResult &hazard) const {
    std::stringstream out;
    assert(hazard.IsHazard());
    out << hazard.State();
    out << ", " << FormatUsage(hazard.Tag()) << ")";
    return out.str();
}

syncval_state::CommandBuffer::CommandBuffer(SyncValidator *dev, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                                            const vvl::CommandPool *pool)
    : vvl::CommandBuffer(dev, cb, pCreateInfo, pool), access_context(*dev, this) {}

void syncval_state::CommandBuffer::Destroy() {
    access_context.Destroy();  // must be first to clean up self references correctly.
    vvl::CommandBuffer::Destroy();
}

void syncval_state::CommandBuffer::Reset() {
    vvl::CommandBuffer::Reset();
    access_context.Reset();
}

void syncval_state::CommandBuffer::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &obj : invalid_nodes) {
        switch (obj->Type()) {
            case kVulkanObjectTypeEvent:
                access_context.RecordDestroyEvent(static_cast<vvl::Event *>(obj.get()));
                break;
            default:
                break;
        }
        vvl::CommandBuffer::NotifyInvalidate(invalid_nodes, unlink);
    }
}
