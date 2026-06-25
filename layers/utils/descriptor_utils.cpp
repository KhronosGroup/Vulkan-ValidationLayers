/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "utils/descriptor_utils.h"
#include <vulkan/vulkan_core.h>

#include "containers/container_utils.h"
#include "containers/custom_containers.h"
#include "generated/dispatch_functions.h"
#include "generated/spirv_grammar_helper.h"
#include "generated/vk_extension_helper.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/state_tracker.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

uint32_t CountDescriptorHeapEmbeddedSamplers(const void* pNext) {
    const VkShaderDescriptorSetAndBindingMappingInfoEXT* mapping_info =
        vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(pNext);
    uint32_t count = 0;

    if (mapping_info) {
        for (uint32_t i = 0; i < mapping_info->mappingCount; ++i) {
            const VkDescriptorSetAndBindingMappingEXT& mapping = mapping_info->pMappings[i];
            if (GetEmbeddedSampler(mapping) != nullptr) {
                count++;
            }
        }
    }

    return count;
}

const char* DescribeDescriptorBufferSize(bool robust, VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return "samplerDescriptorSize";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return "combinedImageSamplerDescriptorSize";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return "sampledImageDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return "storageImageDescriptorSize";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return robust ? "robustUniformTexelBufferDescriptorSize" : "uniformTexelBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return robust ? "robustStorageTexelBufferDescriptorSize" : "storageTexelBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return robust ? "robustUniformBufferDescriptorSize" : "uniformBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return robust ? "robustStorageBufferDescriptorSize" : "storageBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return "inputAttachmentDescriptorSize";
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return "accelerationStructureDescriptorSize";
        default:
            break;
    }
    return "[Unknown]";
}

bool IsResourceVaribleInMapping(const VkDescriptorSetAndBindingMappingEXT& mapping,
                                const spirv::ResourceInterfaceVariable& resource_variable) {
    const uint32_t descriptor_set = resource_variable.decorations.set;
    const uint32_t descriptor_binding = resource_variable.decorations.binding;
    // bindingCount could be UINT32_MAX
    const uint64_t last_binding = mapping.firstBinding + uint64_t(mapping.bindingCount);
    return (mapping.descriptorSet == descriptor_set && descriptor_binding >= mapping.firstBinding &&
            descriptor_binding < last_binding && ResourceTypeMatchesBinding(mapping.resourceMask, resource_variable));
}

bool ResourceTypeMatchesBinding(VkSpirvResourceTypeFlagsEXT resource_type,
                                const spirv::ResourceInterfaceVariable& resource_variable) {
    if (resource_type == VK_SPIRV_RESOURCE_TYPE_ALL_EXT) {
        return true;
    }

    const uint32_t opcode = resource_variable.base_type.Opcode();
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) != 0 && opcode == spv::OpTypeSampler) {
        return true;
    }
    if (opcode == spv::OpTypeImage) {
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT) != 0 && resource_variable.base_type.Word(7) == 1) {
            return true;
        }
        // NonWritable must be on the OpVariable, not the OpTypeStruct
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4789
        const bool nonwritable = resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit);
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT) != 0 && resource_variable.base_type.Word(7) == 2 &&
            nonwritable) {
            return true;
        }
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT) != 0 && resource_variable.base_type.Word(7) == 2 &&
            !nonwritable) {
            return true;
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT) != 0 &&
        resource_variable.is_combined_image_sampler) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT) != 0 && resource_variable.is_uniform_buffer) {
        return true;
    }
    if (resource_variable.is_storage_buffer) {
        // NonWritable must be on the OpVariable, not the OpTypeStruct
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4789
        const bool nonwritable = resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit);
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT) != 0 && nonwritable) {
            return true;
        }
        if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT) != 0 && !nonwritable) {
            return true;
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_ACCELERATION_STRUCTURE_BIT_EXT) != 0 &&
        opcode == spv::OpTypeAccelerationStructureKHR) {
        return true;
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_TENSOR_BIT_ARM) != 0 && opcode == spv::OpTypeTensorARM) {
        return true;
    }

    return false;
}

// Only currently want to report the first mismatch
std::string DescribeResourceTypeMismatch(VkSpirvResourceTypeFlagsEXT resource_type,
                                         const spirv::ResourceInterfaceVariable& resource_variable) {
    const uint32_t opcode = resource_variable.base_type.Opcode();

    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) != 0 && opcode != spv::OpTypeSampler) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeSampler";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 1) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 1";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 2) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 2";
        } else if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            if (resource_variable.HasInMember(spirv::DecorationSet::nonwritable_bit)) {
                return "is not decorated with NonWritable on the OpVariable, but is in the OpTypeStruct. This likely is hitting a "
                       "known DXC code generation bug (DirectXShaderCompiler/issues/8492). Simple work around is to just use "
                       "VK_SPIRV_RESOURCE_TYPE_ALL_EXT here as that will give the desired behavior.";
            } else {
                return "is not decorated with NonWritable";
            }
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT) != 0) {
        if (opcode != spv::OpTypeImage) {
            return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeImage";
        } else if (resource_variable.base_type.Word(7) != 2) {
            return "OpTypeImage Sampled is " + std::to_string(resource_variable.base_type.Word(7)) + ", not 2";
        } else if (resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is decorated with NonWritable";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT) != 0 &&
        !resource_variable.is_combined_image_sampler) {
        return "is not OpTypeSampledImage";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT) != 0 && !resource_variable.is_uniform_buffer) {
        if (resource_variable.storage_class != spv::StorageClassUniform) {
            return "is not Uniform StorageClass";
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.type_struct_info->decorations.Has(spirv::DecorationSet::block_bit)) {
            return "is not decorated with Block";
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT) != 0) {
        if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            if (resource_variable.HasInMember(spirv::DecorationSet::nonwritable_bit)) {
                return "is not decorated with NonWritable on the OpVariable, but is in the OpTypeStruct. This likely is hitting a "
                       "known DXC code generation bug (DirectXShaderCompiler/issues/8492). Simple work around is to just use "
                       "VK_SPIRV_RESOURCE_TYPE_ALL_EXT here as that will give the desired behavior.";
            } else {
                return "is not decorated with NonWritable";
            }
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.is_storage_buffer) {
            return "is not a storage buffer";  // simplified
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT) != 0) {
        if (!resource_variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            return "is decorated with NonWritable";
        } else if (!resource_variable.type_struct_info) {
            return "is not a OpTypeStruct";
        } else if (!resource_variable.is_storage_buffer) {
            return "is not a storage buffer";  // simplified
        }
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_ACCELERATION_STRUCTURE_BIT_EXT) != 0 &&
        opcode == spv::OpTypeAccelerationStructureKHR) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeAccelerationStructureKHR";
    }
    if ((resource_type & VK_SPIRV_RESOURCE_TYPE_TENSOR_BIT_ARM) != 0 && opcode == spv::OpTypeTensorARM) {
        return "base type is " + std::string(string_SpvOpcode(opcode)) + ", not OpTypeTensorARM";
    }

    return "[UNKNOWN]";
}

VkDeviceSize GetDescriptorHeapAlignment(const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props, VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return props.samplerDescriptorAlignment;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            return props.bufferDescriptorAlignment;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:  // not valid, but handle in case
            return props.imageDescriptorAlignment;
        case VK_DESCRIPTOR_TYPE_TENSOR_ARM:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return 0;  // TODO - vendors handle their own types
        default:
            assert(false);
            break;
    }
    return 0;
}

vvl::Field GetDescriptorHeapAlignmentField(VkDescriptorType type) {
    if (type == VK_DESCRIPTOR_TYPE_TENSOR_ARM) {
        return vvl::Field::tensorDescriptorAlignment;
    } else if (type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        return vvl::Field::samplerDescriptorAlignment;
    } else if (IsValueIn(type,
                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER})) {
        return vvl::Field::imageDescriptorAlignment;
    } else if (IsValueIn(type, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR})) {
        return vvl::Field::bufferDescriptorAlignment;
    }
    assert(false);
    return vvl::Field::Empty;
}

// Assumes you already know this is a combined image sampler
bool HasCombinedImageSamplerIndex(const VkDescriptorSetAndBindingMappingEXT& mapping) {
    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        return !mapping.sourceData.pushIndex.pEmbeddedSampler && mapping.sourceData.pushIndex.useCombinedImageSamplerIndex;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        return !mapping.sourceData.indirectIndex.pEmbeddedSampler && mapping.sourceData.indirectIndex.useCombinedImageSamplerIndex;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        return !mapping.sourceData.indirectIndexArray.pEmbeddedSampler &&
               mapping.sourceData.indirectIndexArray.useCombinedImageSamplerIndex;
    }
    return false;
}

void CachedDescriptorSize::Init(const vvl::DeviceState& device_state) {
    const VkPhysicalDevice gpu = device_state.physical_device;
    if (IsExtEnabled(device_state.extensions.vk_ext_descriptor_heap)) {
        heap_size_[0] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_SAMPLER);
        heap_size_[2] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
        heap_size_[3] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        heap_size_[4] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
        heap_size_[5] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
        heap_size_[6] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        heap_size_[7] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        heap_size_[10] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        if (IsExtEnabled(device_state.extensions.vk_khr_acceleration_structure)) {
            heap_size_[1] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
        } else {
            heap_size_[1] = 0;
        }
        if (IsExtEnabled(device_state.extensions.vk_nv_ray_tracing)) {
            heap_size_[8] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);
        } else {
            heap_size_[8] = 0;
        }
        if (IsExtEnabled(device_state.extensions.vk_nv_partitioned_acceleration_structure)) {
            heap_size_[9] =
                DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV);
        } else {
            heap_size_[9] = 0;
        }
        if (IsExtEnabled(device_state.extensions.vk_arm_tensors)) {
            heap_size_[11] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_TENSOR_ARM);
        } else {
            heap_size_[11] = 0;
        }
        if (IsExtEnabled(device_state.extensions.vk_qcom_image_processing)) {
            heap_size_[12] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM);
            heap_size_[13] = DispatchGetPhysicalDeviceDescriptorSizeEXT(gpu, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM);
        } else {
            heap_size_[12] = 0;
            heap_size_[13] = 0;
        }
    }
    if (IsExtEnabled(device_state.extensions.vk_ext_descriptor_buffer)) {
        const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props = device_state.phys_dev_ext_props.descriptor_buffer_props;
        const bool robust = device_state.enabled_features.robustBufferAccess;
        buffer_size_[0] = props.samplerDescriptorSize;
        buffer_size_[1] = props.accelerationStructureDescriptorSize;
        buffer_size_[2] = props.sampledImageDescriptorSize;
        buffer_size_[3] = props.storageImageDescriptorSize;
        buffer_size_[4] = robust ? props.robustUniformTexelBufferDescriptorSize : props.uniformTexelBufferDescriptorSize;
        buffer_size_[5] = robust ? props.robustStorageTexelBufferDescriptorSize : props.storageTexelBufferDescriptorSize;
        buffer_size_[6] = robust ? props.robustUniformBufferDescriptorSize : props.uniformBufferDescriptorSize;
        buffer_size_[7] = robust ? props.robustStorageBufferDescriptorSize : props.storageBufferDescriptorSize;
        buffer_size_[8] = 0;  // nv as
        buffer_size_[9] = 0;  // nv partitioned as
        buffer_size_[10] = props.inputAttachmentDescriptorSize;
        buffer_size_[11] = 0;  // arm tensor
        buffer_size_[12] = 0;  // qcom sample weight
        buffer_size_[13] = 0;  // qcom block match
    }
}

VkDeviceSize CachedDescriptorSize::GetSize(VkDescriptorType type, bool use_heap) const {
    uint32_t index = (uint32_t)type;
    if (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        // takes VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER value at index 1
        index = 1;
    } else if (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
        // takes VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC value at index 8
        index = 8;
    } else if (type == VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV) {
        // takes VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC value at index 9
        index = 9;
    } else if (type == VK_DESCRIPTOR_TYPE_TENSOR_ARM) {
        index = 11;
    } else if (type == VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM) {
        index = 12;
    } else if (type == VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM) {
        index = 13;
    }
    assert(IsValueIn(
        type, {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
               VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT}));
    return use_heap ? heap_size_[index] : buffer_size_[index];
}

std::vector<VkDeviceSize> CachedDescriptorSize::GetAllSizes(bool use_heap) const {
    std::vector<VkDeviceSize> sizes;
    // Pre-allocate as no known implementations seem to go above 4 different sizes
    sizes.reserve(4);

    for (uint32_t i = 0; i < 14; i++) {
        const VkDeviceSize new_size = use_heap ? heap_size_[i] : buffer_size_[i];
        if (new_size == 0) {
            continue;
        }

        const size_t current_count = sizes.size();
        if (current_count == 0) {
            sizes.emplace_back(new_size);
        } else if (current_count == 1) {
            if (sizes[0] != new_size) {
                sizes.emplace_back(new_size);
            }
        } else {
            // The vector is small enough and the number of unique sizes is small enough that just looping the vector to find it
            // should be fast enough
            bool add = true;
            for (size_t j = 0; j < current_count; j++) {
                if (sizes[j] == new_size) {
                    add = false;
                    break;
                }
            }
            if (add) {
                sizes.emplace_back(new_size);
            }
        }
    }
    return sizes;
}