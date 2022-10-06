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
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 *
 * The Shader Validation file is in charge of taking the Shader Module data and validating it
 */
#ifndef VULKAN_SHADER_VALIDATION_H
#define VULKAN_SHADER_VALIDATION_H

#include <cstdlib>
#include <array>

#include "vulkan/vulkan.h"
#include <generated/spirv_tools_commit_id.h>
#include "shader_module.h"
#include "vk_layer_utils.h"
#include "range_vector.h"

struct DeviceFeatures;
struct DeviceExtensions;

struct shader_stage_attributes {
    char const *const name;
    bool arrayed_input;
    bool arrayed_output;
    VkShaderStageFlags stage;
};

extern const std::array<shader_stage_attributes, 5> shader_stage_attribs;

struct ShaderAttribute {
    enum class ComponentStatus {
        Uninitialized,
        Seen,
        Unseen,
    };

    ShaderAttribute(const SHADER_MODULE_STATE &shader_module,
                    const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator,
                    const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator_end)
        : is_valid(shader_attributes_iterator != shader_attributes_iterator_end) {
        if (is_valid) {
            start_location = shader_attributes_iterator->first.first;
            start_component = shader_attributes_iterator->first.second;
            current_global_component = start_location * 4 + start_component;

            const auto def = shader_module.get_def(shader_attributes_iterator->second.type_id);
            const auto base_type_components_count = shader_module.GetNumComponentsInBaseType(def);
            const auto total_byte_size = shader_module.GetTypeBytesSize(def);
            const auto array_size = shader_module.GetArraySize(def);  // Will be 1 if not an array
            assert(array_size > 0);
            // a 32 bit or less component count as 1, a 64 bit component as 2.
            if (total_byte_size == 2 * base_type_components_count * 4 * array_size) {
                components_count = base_type_components_count * 2;
            } else {
                components_count = base_type_components_count;
            }
            assert(components_count > 0);
            assert(components_count <= static_cast<uint32_t>(components.size()));
            for (uint32_t component_i = 0; component_i < components_count; ++component_i) {
                components[component_i] = ComponentStatus::Unseen;
            }
            current_components_left = components_count;

            interface = &shader_attributes_iterator->second;
        }
    }

    bool is_valid = false;
    bool is_scan_completed = false;
    uint32_t start_location = 0;
    uint32_t start_component = 0;
    uint32_t current_global_component = 0;
    uint32_t current_components_left = 0;
    uint32_t components_count = 0;
    std::array<ComponentStatus, 16> components = {ComponentStatus::Uninitialized};
    interface_var const *interface = nullptr;

    static void TagMatchingComponentsAsSeen(ShaderAttribute &lhs, ShaderAttribute &rhs) {
        const sparse_container::range<uint32_t> lhs_range(lhs.current_global_component,
                                                          lhs.current_global_component + lhs.current_components_left);
        const sparse_container::range<uint32_t> rhs_range(rhs.current_global_component,
                                                          rhs.current_global_component + rhs.current_components_left);
        const sparse_container::range<uint32_t> range_intersection = lhs_range & rhs_range;
        if (range_intersection.non_empty()) {
            {
                const uint32_t lhs_global_start_component = lhs.start_location * 4 + lhs.start_component;
                auto lhs_component_i = range_intersection.begin - lhs_global_start_component;
                auto lhs_component_end = range_intersection.end - lhs_global_start_component;
                for (; lhs_component_i < lhs_component_end; ++lhs_component_i) {
                    lhs.components[lhs_component_i] = ComponentStatus::Seen;
                }
                assert(lhs.current_global_component < range_intersection.end);
                lhs.current_global_component = range_intersection.end;
                assert((lhs.start_location * 4 + lhs.start_component + lhs.components_count) >= lhs.current_global_component);
                lhs.current_components_left =
                    (lhs.start_location * 4 + lhs.start_component + lhs.components_count) - lhs.current_global_component;
            }
            {
                const uint32_t rhs_global_start_component = rhs.start_location * 4 + rhs.start_component;
                auto rhs_component_i = range_intersection.begin - rhs_global_start_component;
                auto rhs_component_end = range_intersection.end - rhs_global_start_component;
                for (; rhs_component_i < rhs_component_end; ++rhs_component_i) {
                    rhs.components[rhs_component_i] = ComponentStatus::Seen;
                }
                assert(rhs.current_global_component < range_intersection.end);
                rhs.current_global_component = range_intersection.end;
                assert((rhs.start_location * 4 + rhs.start_component + rhs.components_count) >= rhs.current_global_component);
                rhs.current_components_left =
                    (rhs.start_location * 4 + rhs.start_component + rhs.components_count) - rhs.current_global_component;
            }
        }
    }

    static ShaderAttribute *Min(ShaderAttribute *lhs, ShaderAttribute *rhs) {
        assert(lhs);
        assert(rhs);
        if (*lhs < *rhs) {
            return lhs;
        }
        return rhs;
    }

    void SetIsScanCompleted(bool _is_scan_completed) { is_scan_completed = _is_scan_completed; }
    bool IsScanCompleted() const { return is_scan_completed; }
    uint32_t GetComponentsCount() const { return components_count; }
    const std::array<ComponentStatus, 16> &GetComponents() const { return components; }
    uint32_t LocationFromComponentsIndex(uint32_t component_i) const {
        assert(component_i < components_count);
        const uint32_t global_component_i = start_location * 4 + start_component + component_i;
        const uint32_t location = global_component_i / 4;
        return location;
    }
    uint32_t ComponentFromComponentsIndex(uint32_t component_i) const {
        assert(component_i < components_count);
        const uint32_t global_component_i = start_location * 4 + start_component + component_i;
        const uint32_t component = global_component_i % 4;
        return component;
    }
    // True for vector types with more than 2 components and 64 bits scalar type (dvec3, dvec4, ...)
    bool DoesOverflowOnNextLocation() const { return components_count > 4; }
    bool operator==(const ShaderAttribute &rhs) const {
        if (this == &rhs) {
            return true;
        }
        return is_valid == rhs.is_valid && current_global_component == rhs.current_global_component &&
               current_components_left == rhs.current_components_left;
    }
    bool operator<(const ShaderAttribute &rhs) const {
        if (!is_valid && !rhs.is_valid) {
            return false;
        }
        if (is_valid && !rhs.is_valid) {
            return true;
        }
        if (!is_valid && rhs.is_valid) {
            return false;
        }
        assert(is_valid && rhs.is_valid);
        return (current_global_component + current_components_left) < (rhs.current_global_component + rhs.current_components_left);
    }
    bool IsOnlyPartiallyConsumed() const {
        assert(is_scan_completed);
        bool one_component_seen = false;
        bool one_component_unseen = false;
        for (uint32_t component_i = 0; component_i < components_count; ++component_i) {
            assert(components[component_i] != ComponentStatus::Uninitialized);
            if (components[component_i] == ComponentStatus::Seen) {
                one_component_seen = true;
            } else if (components[component_i] == ComponentStatus::Unseen) {
                one_component_unseen = true;
            }
        }
        return one_component_seen && one_component_unseen;
    }
    bool IsFullyUnconsumed() const {
        assert(is_scan_completed);
        for (uint32_t component_i = 0; component_i < components_count; ++component_i) {
            assert(components[component_i] != ComponentStatus::Uninitialized);
            if (components[component_i] == ComponentStatus::Seen) {
                return false;
            }
        }
        return true;
    }
    bool DoesConsumeNonExistentOutput() const {
        assert(is_scan_completed);
        for (uint32_t component_i = 0; component_i < components_count; ++component_i) {
            assert(components[component_i] != ComponentStatus::Uninitialized);
            if (components[component_i] == ComponentStatus::Unseen) {
                return true;
            }
        }
        return false;
    }
    bool HasSameLocationAndComponentAs(const ShaderAttribute &rhs) const {
        assert(is_valid);
        assert(rhs.is_valid);
        return start_location == rhs.start_location && start_component == rhs.start_component;
    }
};

class ValidationCache {
  public:
    static VkValidationCacheEXT Create(VkValidationCacheCreateInfoEXT const *pCreateInfo) {
        auto cache = new ValidationCache();
        cache->Load(pCreateInfo);
        return VkValidationCacheEXT(cache);
    }

    void Load(VkValidationCacheCreateInfoEXT const *pCreateInfo) {
        const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;
        auto size = headerSize;
        if (!pCreateInfo->pInitialData || pCreateInfo->initialDataSize < size) return;

        uint32_t const *data = (uint32_t const *)pCreateInfo->pInitialData;
        if (data[0] != size) return;
        if (data[1] != VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT) return;
        uint8_t expected_uuid[VK_UUID_SIZE];
        Sha1ToVkUuid(SPIRV_TOOLS_COMMIT_ID, expected_uuid);
        if (memcmp(&data[2], expected_uuid, VK_UUID_SIZE) != 0) return;  // different version

        data = (uint32_t const *)(reinterpret_cast<uint8_t const *>(data) + headerSize);

        auto guard = WriteLock();
        for (; size < pCreateInfo->initialDataSize; data++, size += sizeof(uint32_t)) {
            good_shader_hashes_.insert(*data);
        }
    }

    void Write(size_t *pDataSize, void *pData) {
        const auto headerSize = 2 * sizeof(uint32_t) + VK_UUID_SIZE;  // 4 bytes for header size + 4 bytes for version number + UUID
        if (!pData) {
            *pDataSize = headerSize + good_shader_hashes_.size() * sizeof(uint32_t);
            return;
        }

        if (*pDataSize < headerSize) {
            *pDataSize = 0;
            return;  // Too small for even the header!
        }

        uint32_t *out = (uint32_t *)pData;
        size_t actualSize = headerSize;

        // Write the header
        *out++ = headerSize;
        *out++ = VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT;
        Sha1ToVkUuid(SPIRV_TOOLS_COMMIT_ID, reinterpret_cast<uint8_t *>(out));
        out = (uint32_t *)(reinterpret_cast<uint8_t *>(out) + VK_UUID_SIZE);

        {
            auto guard = ReadLock();
            for (auto it = good_shader_hashes_.begin(); it != good_shader_hashes_.end() && actualSize < *pDataSize;
                 it++, out++, actualSize += sizeof(uint32_t)) {
                *out = *it;
            }
        }

        *pDataSize = actualSize;
    }

    void Merge(ValidationCache const *other) {
        // self-merging is invalid, but avoid deadlock below just in case.
        if (other == this) {
            return;
        }
        auto other_guard = other->ReadLock();
        auto guard = WriteLock();
        good_shader_hashes_.reserve(good_shader_hashes_.size() + other->good_shader_hashes_.size());
        for (auto h : other->good_shader_hashes_) good_shader_hashes_.insert(h);
    }

    static uint32_t MakeShaderHash(VkShaderModuleCreateInfo const *smci);

    bool Contains(uint32_t hash) {
        auto guard = ReadLock();
        return good_shader_hashes_.count(hash) != 0;
    }

    void Insert(uint32_t hash) {
        auto guard = WriteLock();
        good_shader_hashes_.insert(hash);
    }

  private:
    ValidationCache() {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    void Sha1ToVkUuid(const char *sha1_str, uint8_t *uuid) {
        // Convert sha1_str from a hex string to binary. We only need VK_UUID_SIZE bytes of
        // output, so pad with zeroes if the input string is shorter than that, and truncate
        // if it's longer.
#if defined(__GNUC__) && (__GNUC__ > 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        char padded_sha1_str[2 * VK_UUID_SIZE + 1] = {};  // 2 hex digits == 1 byte
        std::strncpy(padded_sha1_str, sha1_str, 2 * VK_UUID_SIZE);
#if defined(__GNUC__) && (__GNUC__ > 8)
#pragma GCC diagnostic pop
#endif
        for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
            const char byte_str[] = {padded_sha1_str[2 * i + 0], padded_sha1_str[2 * i + 1], '\0'};
            uuid[i] = static_cast<uint8_t>(std::strtoul(byte_str, nullptr, 16));
        }
    }

    // hashes of shaders that have passed validation before, and can be skipped.
    // we don't store negative results, as we would have to also store what was
    // wrong with them; also, we expect they will get fixed, so we're less
    // likely to see them again.
    layer_data::unordered_set<uint32_t> good_shader_hashes_;
    mutable ReadWriteLock lock_;
};

spv_target_env PickSpirvEnv(uint32_t api_version, bool spirv_1_4);

void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spvtools::ValidatorOptions &options);

template <typename PipeStageState, typename PipeStageStateVec, typename ShaderStageAttributeVec,
          typename ValidateInterfaceBetweenStagesFunc>
bool ValidateInterfaceBetweenAllPipelineStages(const PipeStageStateVec &pipeline_stage_states, const PipeStageState *fragment_stage,
                                               const ShaderStageAttributeVec &shader_stage_attribs,
                                               const ValidateInterfaceBetweenStagesFunc &validation_func) {
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

    ShaderAttribute out(producer, out_iterator, shader_outputs.end());
    if (out.DoesOverflowOnNextLocation()) {
        assert(out_iterator != shader_outputs.end());
        ++out_iterator;
    }
    ShaderAttribute in(consumer, in_iterator, shader_inputs.end());
    if (in.DoesOverflowOnNextLocation()) {
        assert(in_iterator != shader_inputs.end());
        ++in_iterator;
    }

    const auto increment_out_iterator = [&]() {
        if (out_iterator == shader_outputs.end()) return;
        ++out_iterator;
        out = ShaderAttribute(producer, out_iterator, shader_outputs.end());
        if (out.DoesOverflowOnNextLocation()) {
            assert(out_iterator != shader_outputs.end());
            ++out_iterator;
        }
    };

    const auto increment_in_iterator = [&]() {
        if (in_iterator == shader_inputs.end()) return;
        ++in_iterator;
        in = ShaderAttribute(consumer, in_iterator, shader_inputs.end());
        if (in.DoesOverflowOnNextLocation()) {
            assert(in_iterator != shader_inputs.end());
            ++in_iterator;
        }
    };

    // Walk (out, in) attributes pairs
    while (out_iterator != shader_outputs.end() || in_iterator != shader_inputs.end()) {
        ShaderAttribute::TagMatchingComponentsAsSeen(out, in);

        if (out == in) {
            out.SetIsScanCompleted(true);
            in.SetIsScanCompleted(true);
            skip |= validator(producer, producer_stage, out, consumer, consumer_stage, in);
            increment_out_iterator();
            increment_in_iterator();
        } else {
            ShaderAttribute *min_attribute = ShaderAttribute::Min(&out, &in);
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

#endif  // VULKAN_SHADER_VALIDATION_H
