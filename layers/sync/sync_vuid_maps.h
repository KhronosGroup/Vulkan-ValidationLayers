/* Copyright (c) 2021, 2023 The Khronos Group Inc.
 * Copyright (c) 2021, 2023 Valve Corporation
 * Copyright (c) 2022, 2023 LunarG, Inc.
 * Copyright (C) 2021, 2023 Google Inc.
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
#include <string>
#include <vulkan/vulkan_core.h>
#include "vk_layer_data.h"

namespace core_error {
struct Location;
}

struct SubresourceRangeErrorCodes;

namespace sync_vuid_maps {
using core_error::Location;

extern const std::map<VkPipelineStageFlags2KHR, std::string> kFeatureNameMap;

const std::string &GetBadFeatureVUID(const Location &loc, VkPipelineStageFlags2KHR bit);

const std::string &GetBadAccessFlagsVUID(const Location &loc, VkAccessFlags2KHR bit);

const std::string &GetStageQueueCapVUID(const Location &loc, VkPipelineStageFlags2KHR bit);

enum class QueueError {
    kSrcOrDstMustBeIgnore = 0,
    kSpecialOrIgnoreOnly,
    kSrcAndDstValidOrSpecial,
    kSrcAndDestMustBeIgnore,
    kSrcAndDstBothValid,
    kSubmitQueueMustMatchSrcOrDst,
};

extern const std::map<QueueError, std::string> kQueueErrorSummary;

const std::string &GetBarrierQueueVUID(const Location &loc, QueueError error);

const std::string &GetBadImageLayoutVUID(const Location &loc, VkImageLayout layout);

enum class BufferError {
    kNoMemory = 0,
    kOffsetTooBig,
    kSizeOutOfRange,
    kSizeZero,
    kQueueFamilyExternal,
};

const std::string &GetBufferBarrierVUID(const Location &loc, BufferError error);

enum class ImageError {
    kNoMemory = 0,
    kConflictingLayout,
    kBadLayout,
    kBadAttFeedbackLoopLayout,
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

const std::string &GetImageBarrierVUID(const Location &loc, ImageError error);

struct GetImageBarrierVUIDFunctor {
    ImageError error;
    GetImageBarrierVUIDFunctor(ImageError error_) : error(error_) {}
    const std::string &operator()(const Location &loc) const { return GetImageBarrierVUID(loc, error); }
};

const SubresourceRangeErrorCodes &GetSubResourceVUIDs(const Location &loc);

enum class SubmitError {
    kTimelineSemSmallValue,
    kSemAlreadySignalled,
    kOldBinaryCannotBeSignalled,  // timeline semaphores not supported
    kBinaryCannotBeSignalled,     // timeline semaphores supported
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
    kOtherQueueWaiting,
};

const std::string &GetQueueSubmitVUID(const Location &loc, SubmitError error);

};  // namespace sync_vuid_maps
