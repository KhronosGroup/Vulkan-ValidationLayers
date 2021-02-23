/* Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
 * Copyright (C) 2021 Google Inc.
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
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#include <string>
#include <map>
#include <vulkan/vulkan_core.h>

struct CoreErrorLocation;
struct SubresourceRangeErrorCodes;

namespace sync_vuid_maps {

extern const std::map<VkPipelineStageFlags2KHR, std::string> kFeatureNameMap;

const std::string &GetBadFeatureVUID(const CoreErrorLocation &loc, VkPipelineStageFlags2KHR bit);

const std::string &GetBadAccessFlagsVUID(const CoreErrorLocation &loc, VkAccessFlags2KHR bit);

const std::string &GetStageQueueCapVUID(const CoreErrorLocation &loc, VkPipelineStageFlags2KHR bit);

enum class QueueError {
    kSrcOrDstMustBeIgnore = 0,
    kSpecialOrIgnoreOnly,
    kSrcAndDstValidOrSpecial,
    kSrcAndDestMustBeIgnore,
    kSrcAndDstBothValid,
    kSubmitQueueMustMatchSrcOrDst,
};

extern const std::map<QueueError, std::string> kQueueErrorSummary;

const std::string &GetBarrierQueueVUID(const CoreErrorLocation &loc, QueueError error);

const std::string &GetBadImageLayoutVUID(const CoreErrorLocation &loc, VkImageLayout layout);

enum class BufferError {
    kNoMemory = 0,
    kOffsetTooBig,
    kSizeOutOfRange,
    kSizeZero,
};

const std::string &GetBufferBarrierVUID(const CoreErrorLocation &loc, BufferError error);

enum class ImageError {
    kNoMemory = 0,
    kConflictingLayout,
    kBadLayout,
    kNotColorAspect,
    kNotColorAspectYcbcr,
    kBadMultiplanarAspect,
    kBadPlaneCount,
    kNotDepthOrStencilAspect,
    kNotDepthAndStencilAspect,
    kNotSeparateDepthAndStencilAspect,
    kRenderPassMismatch,
    kRenderPassLayoutChange,
};

const std::string &GetImageBarrierVUID(const CoreErrorLocation &loc, ImageError error);

struct GetImageBarrierVUIDFunctor {
    ImageError error;
    GetImageBarrierVUIDFunctor(ImageError error_) : error(error_) {}
    const std::string &operator()(const CoreErrorLocation &loc) const { return GetImageBarrierVUID(loc, error); }
};

const SubresourceRangeErrorCodes& GetSubResourceVUIDs(const CoreErrorLocation &loc);

enum class SubmitError {
    kTimelineSemSmallValue,
    kSemAlreadySignalled,
    kBinaryCannotBeSignalled,
    kTimelineCannotBeSignalled,
    kTimelineSemMaxDiff,
    kProtectedFeatureDisabled,
    kBadUnprotectedSubmit,
    kBadProtectedSubmit,
    kCmdNotSimultaneous,
    kReusedOneTimeCmd,
    kSecondaryCmdNotSimultaneous,
    kCmdWrongQueueFamily,
    kSecondaryCmdInSubmit,
    kHostStageMask,
};

const std::string &GetQueueSubmitVUID(const CoreErrorLocation &loc, SubmitError error);

};  // namespace sync_vuid_maps
