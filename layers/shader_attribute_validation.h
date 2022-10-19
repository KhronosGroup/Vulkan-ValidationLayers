/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
 * Author: Arno Galvez <arno@lunarg.com>
 */

#pragma once

#include <cstdlib>
#include <array>

#include "shader_module.h"
#include "range_vector.h"

class ShaderAttributeValidation {
  public:
    enum class ComponentStatus {
        Uninitialized,
        Seen,
        Unseen,
    };

    ShaderAttributeValidation(const SHADER_MODULE_STATE &shader_module,
                              const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator,
                              const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator_end)
        : is_valid_(shader_attributes_iterator != shader_attributes_iterator_end) {
        if (is_valid_) {
            start_location_ = shader_attributes_iterator->first.first;
            start_component_ = shader_attributes_iterator->first.second;
            current_global_component_ = start_location_ * 4 + start_component_;

            const auto def = shader_module.get_def(shader_attributes_iterator->second.type_id);
            const auto base_type_components_count = shader_module.GetNumComponentsInBaseType(def);
            const auto total_byte_size = shader_module.GetTypeBytesSize(def);
            const auto array_size = shader_module.GetArraySize(def);  // Will be 1 if not an array
            assert(array_size > 0);
            // a 32 bit or less component count as 1, a 64 bit component as 2.
            if (total_byte_size == 2 * base_type_components_count * 4 * array_size) {
                components_count_ = base_type_components_count * 2;
            } else {
                components_count_ = base_type_components_count;
            }
            assert(components_count_ > 0);
            assert(components_count_ <= static_cast<uint32_t>(components_.size()));
            for (uint32_t component_i = 0; component_i < components_count_; ++component_i) {
                components_[component_i] = ComponentStatus::Unseen;
            }
            current_components_left_ = components_count_;

            interface_ = &shader_attributes_iterator->second;
        }
    }

    static void TagMatchingComponentsAsSeen(ShaderAttributeValidation &lhs, ShaderAttributeValidation &rhs) {
        const sparse_container::range<uint32_t> lhs_range(lhs.current_global_component_,
                                                          lhs.current_global_component_ + lhs.current_components_left_);
        const sparse_container::range<uint32_t> rhs_range(rhs.current_global_component_,
                                                          rhs.current_global_component_ + rhs.current_components_left_);
        const sparse_container::range<uint32_t> range_intersection = lhs_range & rhs_range;
        if (range_intersection.non_empty()) {
            {
                const uint32_t lhs_global_start_component = lhs.start_location_ * 4 + lhs.start_component_;
                auto lhs_component_i = range_intersection.begin - lhs_global_start_component;
                auto lhs_component_end = range_intersection.end - lhs_global_start_component;
                for (; lhs_component_i < lhs_component_end; ++lhs_component_i) {
                    lhs.components_[lhs_component_i] = ComponentStatus::Seen;
                }
                assert(lhs.current_global_component_ < range_intersection.end);
                lhs.current_global_component_ = range_intersection.end;
                assert((lhs.start_location_ * 4 + lhs.start_component_ + lhs.components_count_) >= lhs.current_global_component_);
                lhs.current_components_left_ =
                    (lhs.start_location_ * 4 + lhs.start_component_ + lhs.components_count_) - lhs.current_global_component_;
            }
            {
                const uint32_t rhs_global_start_component = rhs.start_location_ * 4 + rhs.start_component_;
                auto rhs_component_i = range_intersection.begin - rhs_global_start_component;
                auto rhs_component_end = range_intersection.end - rhs_global_start_component;
                for (; rhs_component_i < rhs_component_end; ++rhs_component_i) {
                    rhs.components_[rhs_component_i] = ComponentStatus::Seen;
                }
                assert(rhs.current_global_component_ < range_intersection.end);
                rhs.current_global_component_ = range_intersection.end;
                assert((rhs.start_location_ * 4 + rhs.start_component_ + rhs.components_count_) >= rhs.current_global_component_);
                rhs.current_components_left_ =
                    (rhs.start_location_ * 4 + rhs.start_component_ + rhs.components_count_) - rhs.current_global_component_;
            }
        }
    }

    static ShaderAttributeValidation *Min(ShaderAttributeValidation *lhs, ShaderAttributeValidation *rhs) {
        assert(lhs);
        assert(rhs);
        if (*lhs < *rhs) {
            return lhs;
        }
        return rhs;
    }

    void SetIsScanCompleted(bool _is_scan_completed) { is_scan_completed_ = _is_scan_completed; }
    bool IsScanCompleted() const { return is_scan_completed_; }
    interface_var const *GetInterface() const { return interface_; }
    uint32_t GetComponentsCount() const { return components_count_; }
    const std::array<ComponentStatus, 16> &GetComponents() const { return components_; }
    uint32_t LocationFromComponentsIndex(uint32_t component_i) const {
        assert(component_i < components_count_);
        const uint32_t global_component_i = start_location_ * 4 + start_component_ + component_i;
        const uint32_t location = global_component_i / 4;
        return location;
    }
    uint32_t ComponentFromComponentsIndex(uint32_t component_i) const {
        assert(component_i < components_count_);
        const uint32_t global_component_i = start_location_ * 4 + start_component_ + component_i;
        const uint32_t component = global_component_i % 4;
        return component;
    }
    // True for vector types with more than 2 components and 64 bits scalar type (dvec3, dvec4, ...)
    bool DoesOverflowOnNextLocation() const { return components_count_ > 4; }
    bool operator==(const ShaderAttributeValidation &rhs) const {
        if (this == &rhs) {
            return true;
        }
        return is_valid_ == rhs.is_valid_ && current_global_component_ == rhs.current_global_component_ &&
               current_components_left_ == rhs.current_components_left_;
    }
    bool operator<(const ShaderAttributeValidation &rhs) const {
        if (!is_valid_ && !rhs.is_valid_) {
            return false;
        }
        if (is_valid_ && !rhs.is_valid_) {
            return true;
        }
        if (!is_valid_ && rhs.is_valid_) {
            return false;
        }
        assert(is_valid_ && rhs.is_valid_);
        return (current_global_component_ + current_components_left_) <
               (rhs.current_global_component_ + rhs.current_components_left_);
    }
    bool IsOnlyPartiallyConsumed() const {
        assert(is_scan_completed_);
        bool one_component_seen = false;
        bool one_component_unseen = false;
        for (uint32_t component_i = 0; component_i < components_count_; ++component_i) {
            assert(components_[component_i] != ComponentStatus::Uninitialized);
            if (components_[component_i] == ComponentStatus::Seen) {
                one_component_seen = true;
            } else if (components_[component_i] == ComponentStatus::Unseen) {
                one_component_unseen = true;
            }
        }
        return one_component_seen && one_component_unseen;
    }
    bool IsFullyUnconsumed() const {
        assert(is_scan_completed_);
        for (uint32_t component_i = 0; component_i < components_count_; ++component_i) {
            assert(components_[component_i] != ComponentStatus::Uninitialized);
            if (components_[component_i] == ComponentStatus::Seen) {
                return false;
            }
        }
        return true;
    }
    bool DoesConsumeNonExistentOutput() const {
        assert(is_scan_completed_);
        for (uint32_t component_i = 0; component_i < components_count_; ++component_i) {
            assert(components_[component_i] != ComponentStatus::Uninitialized);
            if (components_[component_i] == ComponentStatus::Unseen) {
                return true;
            }
        }
        return false;
    }
    bool HasSameLocationAndComponentAs(const ShaderAttributeValidation &rhs) const {
        assert(is_valid_);
        assert(rhs.is_valid_);
        return start_location_ == rhs.start_location_ && start_component_ == rhs.start_component_;
    }

  private:
    bool is_valid_ = false;
    bool is_scan_completed_ = false;
    uint32_t start_location_ = 0;
    uint32_t start_component_ = 0;
    uint32_t current_global_component_ = 0;
    uint32_t current_components_left_ = 0;
    uint32_t components_count_ = 0;
    std::array<ComponentStatus, 16> components_ = {{ComponentStatus::Uninitialized}};
    interface_var const *interface_ = nullptr;
};

template <typename PipeStageState, typename PipeStageStateVec, typename ValidateInterfaceBetweenStagesFunc>
bool ValidateInterfaceBetweenAllPipelineStages(const PipeStageStateVec &pipeline_stage_states, const PipeStageState *fragment_stage,
                                               const ValidateInterfaceBetweenStagesFunc &validation_func) {
    static const std::array<shader_stage_attributes, 5> shader_stage_attribs = {
        {{"vertex shader", false, false, VK_SHADER_STAGE_VERTEX_BIT},
         {"tessellation control shader", true, true, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
         {"tessellation evaluation shader", true, false, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
         {"geometry shader", true, false, VK_SHADER_STAGE_GEOMETRY_BIT},
         {"fragment shader", false, false, VK_SHADER_STAGE_FRAGMENT_BIT}}};

    bool skip = false;
    for (size_t i = 1; i < pipeline_stage_states.size(); i++) {
        const auto &producer = pipeline_stage_states[i - 1];
        const auto &consumer = pipeline_stage_states[i];
        assert(producer.module_state);
        if (&producer == fragment_stage) {
            break;
        }
        if (consumer.module_state) {
            if (consumer.module_state->has_valid_spirv && producer.module_state->has_valid_spirv) {
                auto producer_id = SHADER_MODULE_STATE::GetShaderStageId(producer.stage_flag);
                auto consumer_id = SHADER_MODULE_STATE::GetShaderStageId(consumer.stage_flag);
                skip |= validation_func(*producer.module_state.get(), producer.entrypoint, &shader_stage_attribs[producer_id],
                                        *consumer.module_state.get(), consumer.entrypoint, &shader_stage_attribs[consumer_id]);
            }
        }
    }
    return skip;
}

template <typename ShaderAttributePairValidator>
bool IterateInterfaceBetweenStages(const ShaderAttributePairValidator &validator, const SHADER_MODULE_STATE &producer,
                                   spirv_inst_iter producer_entrypoint, shader_stage_attributes const *producer_stage,
                                   const SHADER_MODULE_STATE &consumer, spirv_inst_iter consumer_entrypoint,
                                   shader_stage_attributes const *consumer_stage) {
    bool skip = false;

    const std::map<location_t, interface_var> shader_outputs =
        producer.CollectInterfaceByLocation(producer_entrypoint, spv::StorageClassOutput, producer_stage->arrayed_output);
    const std::map<location_t, interface_var> shader_inputs =
        consumer.CollectInterfaceByLocation(consumer_entrypoint, spv::StorageClassInput, consumer_stage->arrayed_input);

    auto out_iterator = shader_outputs.begin();
    auto in_iterator = shader_inputs.begin();

    ShaderAttributeValidation out(producer, out_iterator, shader_outputs.end());
    if (out.DoesOverflowOnNextLocation()) {
        assert(out_iterator != shader_outputs.end());
        ++out_iterator;
    }
    ShaderAttributeValidation in(consumer, in_iterator, shader_inputs.end());
    if (in.DoesOverflowOnNextLocation()) {
        assert(in_iterator != shader_inputs.end());
        ++in_iterator;
    }

    const auto increment_out_iterator = [&]() {
        if (out_iterator == shader_outputs.end()) return;
        ++out_iterator;
        out = ShaderAttributeValidation(producer, out_iterator, shader_outputs.end());
        if (out.DoesOverflowOnNextLocation()) {
            assert(out_iterator != shader_outputs.end());
            ++out_iterator;
        }
    };

    const auto increment_in_iterator = [&]() {
        if (in_iterator == shader_inputs.end()) return;
        ++in_iterator;
        in = ShaderAttributeValidation(consumer, in_iterator, shader_inputs.end());
        if (in.DoesOverflowOnNextLocation()) {
            assert(in_iterator != shader_inputs.end());
            ++in_iterator;
        }
    };

    // Walk (out, in) attributes pairs
    while (out_iterator != shader_outputs.end() || in_iterator != shader_inputs.end()) {
        ShaderAttributeValidation::TagMatchingComponentsAsSeen(out, in);

        if (out == in) {
            out.SetIsScanCompleted(true);
            in.SetIsScanCompleted(true);
            skip |= validator(producer, producer_stage, out, consumer, consumer_stage, in);
            increment_out_iterator();
            increment_in_iterator();
        } else {
            ShaderAttributeValidation *min_attribute = ShaderAttributeValidation::Min(&out, &in);
            if (min_attribute == &out) {
                out.SetIsScanCompleted(true);
            } else {
                in.SetIsScanCompleted(true);
            }
            skip |= validator(producer, producer_stage, out, consumer, consumer_stage, in);
            if (min_attribute == &out) {
                increment_out_iterator();
            } else {
                increment_in_iterator();
            }
        }
    }

    return skip;
}
