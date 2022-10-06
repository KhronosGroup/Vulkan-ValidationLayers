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

#include "vulkan/vulkan.h"
#include <generated/spirv_tools_commit_id.h>
#include "shader_module.h"
#include "vk_layer_utils.h"

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
    ShaderAttribute(const SHADER_MODULE_STATE &shader_module,
                    const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator,
                    const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator_end)
        : is_valid(shader_attributes_iterator != shader_attributes_iterator_end),
          is_components_reading_at_beginning(true),
          location(0),
          component(0),
          written_components_count(0),
          interface(nullptr) {
        if (is_valid) {
            location = shader_attributes_iterator->first.first;
            component = shader_attributes_iterator->first.second;
            written_components_count =
                shader_module.GetNumComponentsInBaseType(shader_module.get_def(shader_attributes_iterator->second.type_id));
            interface = &shader_attributes_iterator->second;
        }
    }

    bool is_valid = false;
    bool is_components_reading_at_beginning = true;
    uint32_t location = 0;
    uint32_t component = 0;
    union {
        uint32_t written_components_count = 0;
        uint32_t read_components_count;
        uint32_t components_count;
    };
    interface_var const *interface = nullptr;

    bool IsPartiallyUnconsumedBy(const ShaderAttribute &in) const {
        return is_valid && !is_components_reading_at_beginning && (!HasSameLocationAndComponentAs(in) || !in.is_valid);
    }
    bool IsFullyUnconsumedBy(const ShaderAttribute &in) const {
        return is_valid && is_components_reading_at_beginning && (location < in.location || !in.is_valid);
    }
    bool DoesConsumeNonExistent(const ShaderAttribute &out) const {
        return is_valid && (!out.is_valid || (out.location > location) || (out.location == location && out.component > component));
    }
    bool HasSameLocationAndComponentAs(const ShaderAttribute &rhs) const {
        return is_valid && rhs.is_valid && (location == rhs.location) && (component == rhs.component);
    }
    bool Matches(const ShaderAttribute &rhs) const {
        return HasSameLocationAndComponentAs(rhs) && (components_count == rhs.components_count);
    };
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

template <typename ShaderAttributePairChecker>
bool IterateInterfaceBetweenStages(const ShaderAttributePairChecker &checker, const SHADER_MODULE_STATE &producer,
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
    ShaderAttribute in(consumer, in_iterator, shader_inputs.end());

    // Walk (out, in) attributes pairs
    while (out_iterator != shader_outputs.end() || in_iterator != shader_inputs.end()) {
        skip |= checker(producer, producer_stage, out, consumer, consumer_stage, in);

        const auto increment_out_iterator = [&]() {
            assert(out_iterator != shader_outputs.end());
            ++out_iterator;
            out = ShaderAttribute(producer, out_iterator, shader_outputs.end());
        };

        const auto increment_in_iterator = [&]() {
            assert(in_iterator != shader_inputs.end());
            ++in_iterator;
            in = ShaderAttribute(consumer, in_iterator, shader_inputs.end());
        };

        if (out.is_valid && in.is_valid) {
            if (out.Matches(in)) {
                increment_out_iterator();
                increment_in_iterator();

            } else if (out.location == in.location) {
                const auto untouched_out_component = out.component;

                if (out.component <= in.component) {
                    out.component += 1;
                    out.is_components_reading_at_beginning = false;
                    assert(out.written_components_count >= 1);
                    out.written_components_count -= 1;
                    if (out.written_components_count == 0) {
                        increment_out_iterator();
                    }
                }
                if (in.component <= untouched_out_component) {
                    in.component += 1;
                    in.is_components_reading_at_beginning = false;
                    assert(in.written_components_count >= 1);
                    in.written_components_count -= 1;
                    if (in.read_components_count == 0) {
                        increment_in_iterator();
                    }
                }
            } else if (out.location < in.location) {
                increment_out_iterator();
            } else {  // in_attrib.location < out_attrib.location
                increment_in_iterator();
            }
        } else if (out.is_valid) {
            increment_out_iterator();
        } else if (in.is_valid) {
            increment_in_iterator();
        }
    }

    return skip;
}

#endif  // VULKAN_SHADER_VALIDATION_H
