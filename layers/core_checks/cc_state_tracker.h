/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#pragma once
#include "cc_submit.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/queue_state.h"

class CoreChecks;

namespace core {

// CommandBuffer is over 3 times larger than the next largest state object struct, but the majority of the state is only used in
// CoreChecks. This state object is used by everyone else (best practice, sync val, GPU-AV, etc). For this reason, we have
// CommandBuffer object only for core and keep only the most basic items in the parent class
class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    CommandBufferSubState(vvl::CommandBuffer &cb, CoreChecks &validator);

    void RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                          VkPipelineStageFlags2KHR src_stage_mask) override;

    void Reset(const Location &loc) final;
    void Destroy() final;

    void ExecuteCommands(vvl::CommandBuffer &secondary_command_buffer) final;

    void SubmitTimeValidate();

    CoreChecks &validator;

    uint32_t nesting_level;  // VK_EXT_nested_command_buffer

    QFOTransferBarrierSets<QFOBufferTransferBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<QFOImageTransferBarrier> qfo_transfer_image_barriers;
    const QFOTransferBarrierSets<QFOImageTransferBarrier> &GetQFOBarrierSets(const QFOImageTransferBarrier &type_tag) const {
        return qfo_transfer_image_barriers;
    }
    const QFOTransferBarrierSets<QFOBufferTransferBarrier> &GetQFOBarrierSets(const QFOBufferTransferBarrier &type_tag) const {
        return qfo_transfer_buffer_barriers;
    }

    // used for VK_EXT_fragment_density_map_offset
    // currently need to hold in Command buffer because it can be a suspended renderpassss
    std::vector<VkOffset2D> fragment_density_offsets;

    // The subresources from dynamic rendering barriers that can't be validated during record time.
    vvl::unordered_map<VkImage, std::vector<std::pair<VkImageSubresourceRange, vvl::LocationCapture>>>
        submit_validate_dynamic_rendering_barrier_subresources;

  private:
    void ResetCBState();
};

static inline CommandBufferSubState &SubState(vvl::CommandBuffer &cb) {
    return *static_cast<CommandBufferSubState *>(cb.SubState(LayerObjectTypeCoreValidation));
}
static inline const CommandBufferSubState &SubState(const vvl::CommandBuffer &cb) {
    return *static_cast<const CommandBufferSubState *>(cb.SubState(LayerObjectTypeCoreValidation));
}

class QueueSubState : public vvl::QueueSubState {
  public:
    QueueSubState(Logger& logger, vvl::Queue& q);

    void PreSubmit(std::vector<vvl::QueueSubmission> &submissions) override;

    // Override Retire to validate submissions in the order defined by synchronization
    void Retire(vvl::QueueSubmission&) override;

  private:
    QueueSubmissionValidator queue_submission_validator_;
};

}  // namespace core
