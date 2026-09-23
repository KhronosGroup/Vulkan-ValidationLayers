/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync/sync_common.h"
#include "containers/span.h"
#include "generated/error_location_helper.h"

struct Location;

namespace vvl {
class ImageView;
class Pipeline;
}  // namespace vvl

namespace syncval {
class AccessContext;
struct ErrorReporter;
class HazardResult;
class SyncValidator;
struct SyncEnvironment;
struct AttachmentAccess;
enum class SyncOrdering : uint8_t;

enum class AttachmentType { kColor, kDepth, kStencil };

struct RenderingAttachment {
    AttachmentType type;
    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;
    VkResolveModeFlagBits resolve_mode;
    bool feedback_enabled;
    std::shared_ptr<const vvl::ImageView> view;
    std::shared_ptr<const vvl::ImageView> resolve_view;

    RenderingAttachment(const SyncValidator& validator, const VkRenderingAttachmentInfo& info, const AttachmentType type);

    // Range for writing load/store operations
    ImageRangeGen GetRangeGen(const VkRect2D& render_area, uint32_t view_mask = 0) const;

    bool CanOptimizeDrawAccess() const;
    ImageRangeGen GetRenderAreaRangeGen(const VkRect2D& render_area) const;
    ImageRangeGen GetResolveRangeGen(const VkRect2D& render_area) const;
    SyncAccessIndex GetLoadUsage() const;
    SyncAccessIndex GetStoreUsage() const;
    SyncOrdering GetOrdering() const;
    Location GetLocation(uint32_t attachment_index, vvl::Func function = vvl::Func::Empty) const;
    bool IsWriteable(bool depth_write, bool stencil_write) const;
};

std::vector<RenderingAttachment> CollectAttachments(const SyncValidator& validator, const VkRenderingInfo& rendering_info);

struct RenderingInstance {
    VkRenderingFlags flags;
    VkRect2D render_area;
    uint32_t view_mask;
    uint32_t color_attachment_count;

    // Owned by the command buffer during recording and by CommandData during replay
    vvl::span<const RenderingAttachment> attachments;

    // Range gens for attachment views
    vvl::span<const ImageRangeGen> view_gens{};

    void InitViewGens(std::vector<ImageRangeGen>& view_gen_storage);
    const vvl::ImageView* GetClearAttachmentView(const VkClearAttachment& clear_attachment) const;

    bool ValidateBeginRendering(const SyncEnvironment& env, const AccessContext& access_context, const ErrorReporter& reporter,
                                uint32_t render_pass_instance_id) const;
    void RecordBeginRendering(AccessContext& access_context, uint32_t render_pass_instance_id, ResourceUsageTag tag,
                              QueueId queue_id) const;

    bool ValidateEndRendering(const SyncEnvironment& env, const AccessContext& access_context, const ErrorReporter& reporter,
                              uint32_t render_pass_instance_id) const;
    void RecordEndRendering(AccessContext& access_context, uint32_t render_pass_instance_id, ResourceUsageTag tag,
                            QueueId queue_id) const;

    bool ValidateDrawAttachments(const SyncEnvironment& env, const AccessContext& access_context, const ErrorReporter& reporter,
                                 uint32_t render_pass_instance_id, const vvl::Pipeline* pipeline, bool depth_write,
                                 bool stencil_write) const;
    void RecordDrawAttachments(AccessContext& access_context, uint32_t render_pass_instance_id, const vvl::Pipeline* pipeline,
                               bool depth_write, bool stencil_write, ResourceUsageTag tag, QueueId queue_id) const;

  private:
    ImageRangeGen GetOptimizedDrawRangeGen(const AccessContext& access_context, uint32_t attachment_index, SyncAccessIndex usage,
                                           const AttachmentAccess& attachment_access, QueueId queue_id) const;
    HazardResult DetectDrawHazard(const AccessContext& access_context, uint32_t attachment_index, SyncAccessIndex usage,
                                  const AttachmentAccess& attachment_access, QueueId queue_id) const;
};

}  // namespace syncval
