/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
#include <stdint.h>
#include <chrono>
#include <vulkan/vulkan.h>

static const uint32_t kMemoryObjectWarningLimit = 250;

// Maximum number of instanced vertex buffers which should be used
static const uint32_t kMaxInstancedVertexBuffers = 1;

// Recommended allocation size for vkAllocateMemory
static const VkDeviceSize kMinDeviceAllocationSize = 256 * 1024;

// If a buffer or image is allocated and it consumes an entire VkDeviceMemory, it should at least be this large.
// This is slightly different from minDeviceAllocationSize since the 256K buffer can still be sensibly
// suballocated from. If we consume an entire allocation with one image or buffer, it should at least be for a
// very large allocation.
static const VkDeviceSize kMinDedicatedAllocationSize = 1024 * 1024;

// AMD best practices
// Note: These are initial ball park numbers for good performance
// We expect to adjust them as we get more data on layer usage
// Avoid small command buffers
static const uint32_t kMinRecommendedCommandBufferSizeAMD = 10;
// Avoid small secondary command buffers
static const uint32_t kMinRecommendedDrawsInSecondaryCommandBufferSizeAMD = 10;
// Idealy, only 1 fence per frame, so 3 for triple buffering
static const uint32_t kMaxRecommendedFenceObjectsSizeAMD = 3;
// Avoid excessive sempahores
static const uint32_t kMaxRecommendedSemaphoreObjectsSizeAMD = 10;
// Avoid excessive barriers
static const uint32_t kMaxRecommendedBarriersSizeAMD = 500;
// Avoid excessive pipelines
static const uint32_t kMaxRecommendedNumberOfPSOAMD = 5000;
// Unlikely that the user needs all the dynamic states enabled at the same time, and they encur a cost
static const uint32_t kDynamicStatesWarningLimitAMD = 7;
// Too many dynamic descriptor sets can cause a large pipeline layout
static const uint32_t kPipelineLayoutSizeWarningLimitAMD = 13;
// Check that the user is submitting excessivly to a queue
static const uint32_t kNumberOfSubmissionWarningLimitAMD = 20;
// Check that there is enough work per vertex stream change
static const float kVertexStreamToDrawRatioWarningLimitAMD = 0.8f;
// Check that there is enough work per pipeline change
static const float kDrawsPerPipelineRatioWarningLimitAMD = 5.f;
// Check that command buffers are used with an appropriatly sized pool
static const float kCmdBufferToCmdPoolRatioWarningLimitAMD = 0.1f;
// Size for fast descriptor reads on modern NVIDIA devices
static const uint32_t kPipelineLayoutFastDescriptorSpaceNVIDIA = 256;
// Time threshold for flagging allocations that could have been reused
static const auto kAllocateMemoryReuseTimeThresholdNVIDIA = std::chrono::seconds{5};
// Number of switches in tessellation, gemetry, and mesh shader state before signalling a message
static const uint32_t kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA = 4;
// Ratio where the Z-cull direction starts being considered balanced
static const int kZcullDirectionBalanceRatioNVIDIA = 20;
// Maximum number of custom clear colors
static const size_t kMaxRecommendedNumberOfClearColorsNVIDIA = 16;

// How many small indexed drawcalls in a command buffer before a warning is thrown
static const uint32_t kMaxSmallIndexedDrawcalls = 10;

// How many indices make a small indexed drawcall
static const int kSmallIndexedDrawcallIndices = 10;

// Minimum number of vertices/indices to take into account when doing depth pre-pass checks for Arm Mali GPUs
static const int kDepthPrePassMinDrawCountArm = 500;

// Minimum, number of draw calls in order to trigger depth pre-pass warnings for Arm Mali GPUs
static const int kDepthPrePassNumDrawCallsArm = 20;

// Maximum sample count for full throughput on Mali GPUs
static const VkSampleCountFlagBits kMaxEfficientSamplesArm = VK_SAMPLE_COUNT_4_BIT;

// On Arm Mali architectures, it's generally best to align work group dimensions to 4.
static const uint32_t kThreadGroupDispatchCountAlignmentArm = 4;

// Maximum number of threads which can efficiently be part of a compute workgroup when using thread group barriers.
static const uint32_t kMaxEfficientWorkGroupThreadCountArm = 64;

// Minimum number of vertices/indices a draw needs to have before considering it in depth prepass warnings on PowerVR
static const int kDepthPrePassMinDrawCountIMG = 300;

// Minimum, number of draw calls matching the above criteria before triggerring a depth prepass warning on PowerVR
static const int kDepthPrePassNumDrawCallsIMG = 10;

// Maximum sample count on PowerVR before showing a warning
static const VkSampleCountFlagBits kMaxEfficientSamplesImg = VK_SAMPLE_COUNT_4_BIT;

struct SpecialUseVUIDs {
    const char* cadsupport;
    const char* d3demulation;
    const char* devtools;
    const char* debugging;
    const char* glemulation;
};

typedef enum {
    kBPVendorArm = 0x00000001,
    kBPVendorAMD = 0x00000002,
    kBPVendorIMG = 0x00000004,
    kBPVendorNVIDIA = 0x00000008,
} BPVendorFlagBits;
typedef VkFlags BPVendorFlags;

enum IMAGE_SUBRESOURCE_USAGE_BP {
    UNDEFINED,  // If it has never been used
    RENDER_PASS_CLEARED,
    RENDER_PASS_READ_TO_TILE,
    CLEARED,
    DESCRIPTOR_ACCESS,
    RENDER_PASS_STORED,
    RENDER_PASS_DISCARDED,
    BLIT_READ,
    BLIT_WRITE,
    RESOLVE_READ,
    RESOLVE_WRITE,
    COPY_READ,
    COPY_WRITE
};

enum class ZcullDirection {
    Unknown,
    Less,
    Greater,
};