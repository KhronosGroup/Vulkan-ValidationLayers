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

#include "sync_barrier.h"
#include "containers/span.h"

struct Location;
struct VulkanTypedHandle;

namespace vvl {
class Buffer;
class Image;
enum class Func;
}  // namespace vvl

namespace syncval {

class AccessContext;
class CommandBufferContext;
struct CommandData;
struct SyncEnvironment;

struct BufferCopyRegion {
    VkDeviceSize src_offset;
    VkDeviceSize dst_offset;
    VkDeviceSize size;
};

struct BufferCopyCommand {
    const vvl::Buffer& src_buffer;
    const vvl::Buffer& dst_buffer;
    vvl::span<const BufferCopyRegion> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        uint32_t src_buffer_index;
        uint32_t dst_buffer_index;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        BufferCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct ImageCopyCommand {
    const vvl::Image& src_image;
    const vvl::Image& dst_image;
    vvl::span<const VkImageCopy> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        uint32_t src_image_index;
        uint32_t dst_image_index;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        ImageCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BarrierCommand {
    const BarrierSet& barrier_set;

    struct Storage {
        uint32_t barrier_set_index;
        BarrierCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

using CommandStorage = std::variant<BufferCopyCommand::Storage, ImageCopyCommand::Storage, BarrierCommand::Storage>;

struct CommandData {
    std::vector<std::shared_ptr<const vvl::Buffer>> buffers;
    std::vector<std::shared_ptr<const vvl::Image>> images;
    std::vector<BufferCopyRegion> buffer_copy_regions;
    std::vector<VkImageCopy> image_copy_regions;
    std::vector<BarrierSet> barrier_sets;

    uint32_t AddBuffer(const vvl::Buffer& buffer);
    uint32_t AddImage(const vvl::Image& image);
};

// TODO: CommandEntry won't be needed after all commands are introduced.
// Tag could be derived from command index. Remove entry type when and
// use array of commands instead.
struct CommandEntry {
    ResourceUsageTag tag;
    CommandStorage storage;
};

bool ReplayCommands(SyncEnvironment& env, AccessContext& access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc);

}  // namespace syncval
