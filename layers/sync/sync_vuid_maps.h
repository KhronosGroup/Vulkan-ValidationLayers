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
#include "containers/custom_containers.h"

struct Location;
struct DeviceExtensions;
struct SubresourceRangeErrorCodes;
struct DeviceExtensions;

namespace sync_vuid_maps {

extern const std::map<VkPipelineStageFlags2KHR, std::string> kFeatureNameMap;

const std::string &GetBadFeatureVUID(const Location &loc, VkPipelineStageFlags2 bit, const DeviceExtensions &device_extensions);

const std::string &GetBadAccessFlagsVUID(const Location &loc, VkAccessFlags2KHR bit);

const std::string &GetStageQueueCapVUID(const Location &loc, VkPipelineStageFlags2KHR bit);

enum class QueueError {
    kSrcNoExternalExt = 0,
    kDstNoExternalExt,
    kSrcNoForeignExt,
    kDstNoForeignExt,
    kSync1ConcurrentNoIgnored,
    kSync1ConcurrentSrc,
    kSync1ConcurrentDst,
    kExclusiveSrc,
    kExclusiveDst,
    kHostStage,
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
    kBadSync2OldLayout,
    kBadSync2NewLayout,
    kNotColorAspectSinglePlane,
    kNotColorAspectNonDisjoint,
    kBadMultiplanarAspect,
    kBadPlaneCount,
    kNotDepthOrStencilAspect,
    kNotDepthAndStencilAspect,
    kSeparateDepthWithStencilLayout,
    kSeparateStencilhWithDepthLayout,
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
    kBinaryCannotBeSignalled,
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

enum class ShaderTileImageError { kShaderTileImageFeatureError, kShaderTileImageBarrierError };

const std::string &GetShaderTileImageVUID(const Location &loc, ShaderTileImageError error);

const std::string &GetAccessMaskRayQueryVUIDSelector(const Location &loc, const DeviceExtensions &device_extensions);

}  // namespace sync_vuid_maps
