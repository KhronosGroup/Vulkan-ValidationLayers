/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include <vector>

#include "external/inplace_function.h"
#include "gpuav/instrumentation/descriptor_checks.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

// We pull in most the core state tracking files
// gpuav_state_trackers.h should NOT be included by any other header file
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/push_constant_data.h"

namespace gpuav {

class Validator;
class QueueSubState;

class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    struct LabelLogging {
        const std::vector<std::string> &initial_label_stack;
        const vvl::unordered_map<uint32_t, uint32_t> &action_cmd_i_to_label_cmd_i_map;
    };

    using OnInstrumentionDescSetUpdate =
        stdext::inplace_function<void(CommandBufferSubState &cb, VkPipelineBindPoint bind_point,
                                      VkDescriptorBufferInfo &out_buffer_info, uint32_t &out_dst_binding),
                                 48>;
    using OnCommandBufferSubmission =
        stdext::inplace_function<void(Validator &gpuav, CommandBufferSubState &cb, VkCommandBuffer per_submission_cb)>;
    using OnCommandBufferCompletion =
        stdext::inplace_function<bool(Validator &gpuav, CommandBufferSubState &cb,
                                      const CommandBufferSubState::LabelLogging &label_logging, const Location &loc),
                                 64>;
    using OnPreCommandBufferSubmission =
        stdext::inplace_function<void(Validator &gpuav, CommandBufferSubState &cb, VkCommandBuffer per_pre_submission_cb)>;
    using OnPostCommandBufferSubmission =
        stdext::inplace_function<void(Validator &gpuav, CommandBufferSubState &cb, VkCommandBuffer per_post_submission_cb)>;
    std::vector<OnInstrumentionDescSetUpdate> on_instrumentation_desc_set_update_functions;
    std::vector<OnPreCommandBufferSubmission> on_pre_cb_submission_functions;
    std::vector<OnPostCommandBufferSubmission> on_post_cb_submission_functions;
    std::vector<OnCommandBufferCompletion> on_cb_completion_functions;

    vko::SharedResourcesCache shared_resources_cache;

    // Used to track which spot in the command buffer the error came from
    bool max_actions_cmd_validation_reached_ = false;
    uint32_t draw_index = 0;
    uint32_t compute_index = 0;
    uint32_t trace_rays_index = 0;
    uint32_t action_command_count = 0;

    std::vector<PushConstantData> push_constant_data_chunks;
    std::array<VkPipelineLayout, BindPoint_Count> push_constant_latest_used_layout{};

    CommandBufferSubState(Validator &gpuav, vvl::CommandBuffer &cb);
    ~CommandBufferSubState();

    [[nodiscard]] bool PreSubmit(QueueSubState &queue, const Location &loc);
    [[nodiscard]] bool PostSubmit(QueueSubState &queue, const Location &loc);
    void OnCompletion(VkQueue queue, const std::vector<std::string> &initial_label_stack, const Location &loc);

    const VkDescriptorSetLayout &GetInstrumentationDescriptorSetLayout() const {
        assert(instrumentation_desc_set_layout_ != VK_NULL_HANDLE);
        return instrumentation_desc_set_layout_;
    }

    const vko::BufferRange &GetErrorOutputBufferRange() const {
        assert(error_output_buffer_range_.buffer != VK_NULL_HANDLE);
        return error_output_buffer_range_;
    }

    VkDeviceSize GetCmdErrorsCountsBufferByteSize() const { return 8192 * sizeof(uint32_t); }

    const VkBuffer &GetCmdErrorsCountsBuffer() const {
        assert(cmd_errors_counts_buffer_.VkHandle() != VK_NULL_HANDLE);
        return cmd_errors_counts_buffer_.VkHandle();
    }

    void IncrementCommandCount(Validator &gpuav, VkPipelineBindPoint bind_point, const Location &loc);

    std::string GetDebugLabelRegion(uint32_t label_command_i, const std::vector<std::string> &initial_label_stack) const;

    void Destroy() final;
    void Reset(const Location &loc) final;

    void RecordPushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags, uint32_t offset, uint32_t size,
                             const void *values) final;
    void ClearPushConstants() final;

    vko::GpuResourcesManager gpu_resources_manager;
    // Using stdext::inplace_function over std::function to allocate memory in place
    using ErrorLoggerFunc =
        stdext::inplace_function<bool(const uint32_t *error_record, const LogObjectList &objlist,
                                      const std::vector<std::string> &initial_label_stack),
                                 288 /*lambda storage size (bytes), large enough to store biggest error lambda*/>;
    std::vector<ErrorLoggerFunc> per_command_error_loggers;
    vvl::unordered_map<uint32_t, uint32_t> action_cmd_i_to_label_cmd_i_map;

    // Buffer storing GPU-AV errors
    vko::BufferRange error_output_buffer_range_;
    // Buffer storing an error count per validated commands.
    // Used to limit the number of errors a single command can emit.
    vko::Buffer cmd_errors_counts_buffer_;

  private:
    void AllocateResources(const Location &loc);
    void ResetCBState(bool should_destroy);
    bool NeedsPostProcess();

    Validator &gpuav_;
    VkDescriptorSetLayout instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
};

static inline CommandBufferSubState &SubState(vvl::CommandBuffer &cb) {
    return *static_cast<CommandBufferSubState *>(cb.SubState(LayerObjectTypeGpuAssisted));
}

// Track descriptor sets bound in a command buffer
class DescriptorSetBindings {
  public:
    struct BindingCommand {
        // A GPU stored LUT where each bound descriptor set is given a spot to store a post processing buffer address.
        // Those spots are updated at command buffer submission time.
        vko::BufferRange desc_set_binding_to_post_process_buffers_lut{};

        vko::BufferRange descritpor_state_ssbo{};  // type BoundDescriptorSetsStateSSBO

        // The index into this vector will start from vkCmdBindDescriptorSets::firstSet
        // The vector size is vkCmdBindDescriptorSets::descriptorSetCount
        std::vector<std::shared_ptr<vvl::DescriptorSet>> bound_descriptor_sets;
    };

    using OnDescriptorSetBindingFunc =
        stdext::inplace_function<void(Validator &gpuav, CommandBufferSubState &cb, BindingCommand &)>;
    std::vector<OnDescriptorSetBindingFunc> on_update_bound_descriptor_sets;
    std::vector<BindingCommand> descriptor_set_binding_commands;
};

class QueueSubState : public vvl::QueueSubState {
  public:
    QueueSubState(Validator &gpuav, vvl::Queue &q);
    virtual ~QueueSubState();

    void PreSubmit(std::vector<vvl::QueueSubmission> &submissions) override;
    void PostSubmit(std::deque<vvl::QueueSubmission> &submissions) override;
    void Retire(vvl::QueueSubmission &) override;

    vko::SharedResourcesCache shared_resources_cache;

  protected:
    void SubmitBarrier(const Location &loc, uint64_t seq);

    Validator &gpuav_;
    VkCommandPool barrier_command_pool_{VK_NULL_HANDLE};
    VkCommandBuffer barrier_command_buffer_{VK_NULL_HANDLE};
    VkSemaphore barrier_sem_{VK_NULL_HANDLE};
    std::deque<std::vector<vvl::CommandBufferSubmission>> retiring_;
    const bool timeline_khr_;
};

class ImageSubState : public vvl::ImageSubState {
  public:
    ImageSubState(vvl::Image &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline ImageSubState &SubState(vvl::Image &obj) {
    return *static_cast<ImageSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ImageSubState &SubState(const vvl::Image &obj) {
    return *static_cast<const ImageSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class ImageViewSubState : public vvl::ImageViewSubState {
  public:
    ImageViewSubState(vvl::ImageView &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline ImageViewSubState &SubState(vvl::ImageView &obj) {
    return *static_cast<ImageViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ImageViewSubState &SubState(const vvl::ImageView &obj) {
    return *static_cast<const ImageViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class BufferSubState : public vvl::BufferSubState {
  public:
    BufferSubState(vvl::Buffer &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline BufferSubState &SubState(vvl::Buffer &obj) {
    return *static_cast<BufferSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const BufferSubState &SubState(const vvl::Buffer &obj) {
    return *static_cast<const BufferSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class BufferViewSubState : public vvl::BufferViewSubState {
  public:
    BufferViewSubState(vvl::BufferView &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline BufferViewSubState &SubState(vvl::BufferView &obj) {
    return *static_cast<BufferViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const BufferViewSubState &SubState(const vvl::BufferView &obj) {
    return *static_cast<const BufferViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class SamplerSubState : public vvl::SamplerSubState {
  public:
    SamplerSubState(vvl::Sampler &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline SamplerSubState &SubState(vvl::Sampler &obj) {
    return *static_cast<SamplerSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const SamplerSubState &SubState(const vvl::Sampler &obj) {
    return *static_cast<const SamplerSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class AccelerationStructureNVSubState : public vvl::AccelerationStructureNVSubState {
  public:
    AccelerationStructureNVSubState(vvl::AccelerationStructureNV &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline AccelerationStructureNVSubState &SubState(vvl::AccelerationStructureNV &obj) {
    return *static_cast<AccelerationStructureNVSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const AccelerationStructureNVSubState &SubState(const vvl::AccelerationStructureNV &obj) {
    return *static_cast<const AccelerationStructureNVSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
class AccelerationStructureKHRSubState : public vvl::AccelerationStructureKHRSubState {
  public:
    AccelerationStructureKHRSubState(vvl::AccelerationStructureKHR &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline AccelerationStructureKHRSubState &SubState(vvl::AccelerationStructureKHR &obj) {
    return *static_cast<AccelerationStructureKHRSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const AccelerationStructureKHRSubState &SubState(const vvl::AccelerationStructureKHR &obj) {
    return *static_cast<const AccelerationStructureKHRSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class ShaderObjectSubState : public vvl::ShaderObjectSubState {
  public:
    explicit ShaderObjectSubState(vvl::ShaderObject &obj);

    bool was_instrumented = false;
    uint32_t unique_shader_id = 0;
    // We need to keep incase the user calls vkGetShaderBinaryDataEXT
    vku::safe_VkShaderCreateInfoEXT original_create_info;
    VkShaderEXT original_handle = VK_NULL_HANDLE;
};

static inline ShaderObjectSubState &SubState(vvl::ShaderObject &obj) {
    return *static_cast<ShaderObjectSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ShaderObjectSubState &SubState(const vvl::ShaderObject &obj) {
    return *static_cast<const ShaderObjectSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

}  // namespace gpuav
