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

#include "generated/spirv_grammar_helper.h"
#include "state_tracker/shader_module.h"

#include <cstring>
#include <string>

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

size_t GetDescriptorBufferSize(const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props, bool robust, VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return props.samplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return props.combinedImageSamplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return props.sampledImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return props.storageImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return robust ? props.robustUniformTexelBufferDescriptorSize : props.uniformTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return robust ? props.robustStorageTexelBufferDescriptorSize : props.storageTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return robust ? props.robustUniformBufferDescriptorSize : props.uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return robust ? props.robustStorageBufferDescriptorSize : props.storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return props.inputAttachmentDescriptorSize;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return props.accelerationStructureDescriptorSize;
        default:
            break;
    }
    return 0;
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
    const uint32_t last_binding = mapping.firstBinding + mapping.bindingCount;
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
