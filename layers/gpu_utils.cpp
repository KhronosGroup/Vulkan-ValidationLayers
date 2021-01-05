/* Copyright (c) 2020-2021 The Khronos Group Inc.
 * Copyright (c) 2020-2021 Valve Corporation
 * Copyright (c) 2020-2021 LunarG, Inc.
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
 * Author: Tony Barbour <tony@lunarg.com>
 */

#include "chassis.h"
#include "layer_chassis_dispatch.h"
#include "state_tracker.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include <spirv/unified1/spirv.hpp>
#include <algorithm>
#include <regex>

#define VMA_IMPLEMENTATION
// This define indicates that we will supply Vulkan function pointers at initialization
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

class UtilDescriptorSetManager {
  public:
    UtilDescriptorSetManager(VkDevice device, uint32_t numBindingsInSet);
    ~UtilDescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t numBindingsInSet;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};

// Implementation for Descriptor Set Manager class
UtilDescriptorSetManager::UtilDescriptorSetManager(VkDevice device, uint32_t numBindingsInSet)
    : device(device), numBindingsInSet(numBindingsInSet) {}

UtilDescriptorSetManager::~UtilDescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(device, pool.first, NULL);
    }
    desc_pool_map_.clear();
}

VkResult UtilDescriptorSetManager::GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSetLayout ds_layout,
                                                    VkDescriptorSet *desc_set) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, desc_pool, ds_layout, &desc_sets);
    if (result == VK_SUCCESS) {
        *desc_set = desc_sets[0];
    }
    return result;
}

VkResult UtilDescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, VkDescriptorSetLayout ds_layout,
                                                     std::vector<VkDescriptorSet> *desc_sets) {
    const uint32_t default_pool_size = kItemsPerChunk;
    VkResult result = VK_SUCCESS;
    VkDescriptorPool pool_to_use = VK_NULL_HANDLE;

    if (0 == count) {
        return result;
    }
    desc_sets->clear();
    desc_sets->resize(count);

    for (auto &pool : desc_pool_map_) {
        if (pool.second.used + count < pool.second.size) {
            pool_to_use = pool.first;
            break;
        }
    }
    if (VK_NULL_HANDLE == pool_to_use) {
        uint32_t pool_count = default_pool_size;
        if (count > default_pool_size) {
            pool_count = count;
        }
        const VkDescriptorPoolSize size_counts = {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            pool_count * numBindingsInSet,
        };
        VkDescriptorPoolCreateInfo desc_pool_info = {};
        desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_info.pNext = NULL;
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = pool_count;
        desc_pool_info.poolSizeCount = 1;
        desc_pool_info.pPoolSizes = &size_counts;
        result = DispatchCreateDescriptorPool(device, &desc_pool_info, NULL, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, ds_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool_to_use, count,
                                              desc_layouts.data()};

    result = DispatchAllocateDescriptorSets(device, &alloc_info, desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }
    *pool = pool_to_use;
    desc_pool_map_[pool_to_use].used += count;
    return result;
}

void UtilDescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto iter = desc_pool_map_.find(desc_pool);
    if (iter != desc_pool_map_.end()) {
        VkResult result = DispatchFreeDescriptorSets(device, desc_pool, 1, &desc_set);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return;
        }
        desc_pool_map_[desc_pool].used--;
        if (0 == desc_pool_map_[desc_pool].used) {
            DispatchDestroyDescriptorPool(device, desc_pool, NULL);
            desc_pool_map_.erase(desc_pool);
        }
    }
    return;
}

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                   VkPhysicalDeviceProperties *pProperties) {
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceMemoryProperties *pMemoryProperties) {
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    return DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator) {
    DispatchFreeMemory(device, memory, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                                     VkMemoryMapFlags flags, void **ppData) {
    return DispatchMapMemory(device, memory, offset, size, flags, ppData);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkUnmapMemory(VkDevice device, VkDeviceMemory memory) { DispatchUnmapMemory(device, memory); }
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                   const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                        const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                            VkDeviceSize memoryOffset) {
    return DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                           VkDeviceSize memoryOffset) {
    return DispatchBindImageMemory(device, image, memory, memoryOffset);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                   VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                  VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    return DispatchCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    return DispatchDestroyBuffer(device, buffer, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    return DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    DispatchDestroyImage(device, image, pAllocator);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                     uint32_t regionCount, const VkBufferCopy *pRegions) {
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VkResult UtilInitializeVma(VkPhysicalDevice physical_device, VkDevice device, VmaAllocator *pAllocator) {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.device = device;
    allocator_info.physicalDevice = physical_device;

    functions.vkGetPhysicalDeviceProperties = static_cast<PFN_vkGetPhysicalDeviceProperties>(gpuVkGetPhysicalDeviceProperties);
    functions.vkGetPhysicalDeviceMemoryProperties =
        static_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(gpuVkGetPhysicalDeviceMemoryProperties);
    functions.vkAllocateMemory = static_cast<PFN_vkAllocateMemory>(gpuVkAllocateMemory);
    functions.vkFreeMemory = static_cast<PFN_vkFreeMemory>(gpuVkFreeMemory);
    functions.vkMapMemory = static_cast<PFN_vkMapMemory>(gpuVkMapMemory);
    functions.vkUnmapMemory = static_cast<PFN_vkUnmapMemory>(gpuVkUnmapMemory);
    functions.vkFlushMappedMemoryRanges = static_cast<PFN_vkFlushMappedMemoryRanges>(gpuVkFlushMappedMemoryRanges);
    functions.vkInvalidateMappedMemoryRanges = static_cast<PFN_vkInvalidateMappedMemoryRanges>(gpuVkInvalidateMappedMemoryRanges);
    functions.vkBindBufferMemory = static_cast<PFN_vkBindBufferMemory>(gpuVkBindBufferMemory);
    functions.vkBindImageMemory = static_cast<PFN_vkBindImageMemory>(gpuVkBindImageMemory);
    functions.vkGetBufferMemoryRequirements = static_cast<PFN_vkGetBufferMemoryRequirements>(gpuVkGetBufferMemoryRequirements);
    functions.vkGetImageMemoryRequirements = static_cast<PFN_vkGetImageMemoryRequirements>(gpuVkGetImageMemoryRequirements);
    functions.vkCreateBuffer = static_cast<PFN_vkCreateBuffer>(gpuVkCreateBuffer);
    functions.vkDestroyBuffer = static_cast<PFN_vkDestroyBuffer>(gpuVkDestroyBuffer);
    functions.vkCreateImage = static_cast<PFN_vkCreateImage>(gpuVkCreateImage);
    functions.vkDestroyImage = static_cast<PFN_vkDestroyImage>(gpuVkDestroyImage);
    functions.vkCmdCopyBuffer = static_cast<PFN_vkCmdCopyBuffer>(gpuVkCmdCopyBuffer);
    allocator_info.pVulkanFunctions = &functions;

    return vmaCreateAllocator(&allocator_info, pAllocator);
}

void UtilPreCallRecordCreateDevice(VkPhysicalDevice gpu, safe_VkDeviceCreateInfo *modified_create_info,
                                   VkPhysicalDeviceFeatures supported_features, VkPhysicalDeviceFeatures desired_features) {
    VkPhysicalDeviceFeatures *features = nullptr;
    if (modified_create_info->pEnabledFeatures) {
        // If pEnabledFeatures, VkPhysicalDeviceFeatures2 in pNext chain is not allowed
        features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
    } else {
        VkPhysicalDeviceFeatures2 *features2 = nullptr;
        features2 = const_cast<VkPhysicalDeviceFeatures2 *>(LvlFindInChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext));
        if (features2) features = &features2->features;
    }
    VkPhysicalDeviceFeatures new_features = {};
    VkBool32 *desired = reinterpret_cast<VkBool32 *>(&desired_features);
    VkBool32 *feature_ptr;
    if (features) {
        feature_ptr = reinterpret_cast<VkBool32 *>(features);
    } else {
        feature_ptr = reinterpret_cast<VkBool32 *>(&new_features);
    }
    VkBool32 *supported = reinterpret_cast<VkBool32 *>(&supported_features);
    for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures); i += (sizeof(VkBool32))) {
        if (*supported && *desired) {
            *feature_ptr = true;
        }
        supported++;
        desired++;
        feature_ptr++;
    }
    if (!features) {
        delete modified_create_info->pEnabledFeatures;
        modified_create_info->pEnabledFeatures = new VkPhysicalDeviceFeatures(new_features);
    }
}

// Generate the stage-specific part of the message.
void UtilGenerateStageMessage(const uint32_t *debug_record, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstCommonOutStageIdx]) {
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << debug_record[kInstVertOutVertexIndex]
                 << " Instance Index = " << debug_record[kInstVertOutInstanceIndex] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << debug_record[kInstTessCtlOutInvocationId]
                 << ", Primitive ID = " << debug_record[kInstTessCtlOutPrimitiveId];
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Primitive ID = " << debug_record[kInstTessEvalOutPrimitiveId]
                 << ", TessCoord (u, v) = (" << debug_record[kInstTessEvalOutTessCoordU] << ", "
                 << debug_record[kInstTessEvalOutTessCoordV] << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << debug_record[kInstGeomOutPrimitiveId]
                 << " Invocation ID = " << debug_record[kInstGeomOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordX]) << ", "
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordY]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID (x, y, z) = (" << debug_record[kInstCompOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstCompOutGlobalInvocationIdY] << ", " << debug_record[kInstCompOutGlobalInvocationIdZ] << " )";
        } break;
        case spv::ExecutionModelRayGenerationNV: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelIntersectionNV: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelAnyHitNV: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelClosestHitNV: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelMissNV: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelCallableNV: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelTaskNV: {
            strm << "Stage = Task. Global invocation ID (x, y, z) = (" << debug_record[kInstTaskOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstTaskOutGlobalInvocationIdY] << ", " << debug_record[kInstTaskOutGlobalInvocationIdZ] << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            strm << "Stage = Mesh.Global invocation ID (x, y, z) = (" << debug_record[kInstMeshOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstMeshOutGlobalInvocationIdY] << ", " << debug_record[kInstMeshOutGlobalInvocationIdZ] << " )";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << debug_record[kInstCommonOutStageIdx] << "). ";
            assert(false);
        } break;
    }
    msg = strm.str();
}

std::string LookupDebugUtilsName(const debug_report_data *report_data, const uint64_t object) {
    auto object_label = report_data->DebugReportGetUtilsObjectName(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
void UtilGenerateCommonMessage(const debug_report_data *report_data, const VkCommandBuffer commandBuffer,
                               const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                               const VkPipeline pipeline_handle, const VkPipelineBindPoint pipeline_bind_point,
                               const uint32_t operation_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE) {
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer) << "). ";
        assert(true);
    } else {
        strm << std::hex << std::showbase << "Command buffer " << LookupDebugUtilsName(report_data, HandleToUint64(commandBuffer))
             << "(" << HandleToUint64(commandBuffer) << "). ";
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            strm << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            strm << "Compute ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            strm << "Ray Trace ";
        } else {
            assert(false);
            strm << "Unknown Pipeline Operation ";
        }
        strm << "Index " << operation_index << ". "
             << "Pipeline " << LookupDebugUtilsName(report_data, HandleToUint64(pipeline_handle)) << "("
             << HandleToUint64(pipeline_handle) << "). "
             << "Shader Module " << LookupDebugUtilsName(report_data, HandleToUint64(shader_module_handle)) << "("
             << HandleToUint64(shader_module_handle) << "). ";
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[kInstCommonOutInstructionIdx] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
void ReadOpSource(const SHADER_MODULE_STATE &shader, const uint32_t reported_file_id, std::vector<std::string> &opsource_lines) {
    for (auto insn : shader) {
        if ((insn.opcode() == spv::OpSource) && (insn.len() >= 5) && (insn.word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str((char *)&insn.word(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }
            while ((++insn).opcode() == spv::OpSourceContinued) {
                in_stream.str((char *)&insn.word(1));
                while (std::getline(in_stream, cur_line)) {
                    opsource_lines.push_back(cur_line);
                }
            }
            break;
        }
    }
}

// The task here is to search the OpSource content to find the #line directive with the
// line number that is closest to, but still prior to the reported error line number and
// still within the reported filename.
// From this known position in the OpSource content we can add the difference between
// the #line line number and the reported error line number to determine the location
// in the OpSource content of the reported error line.
//
// Considerations:
// - Look only at #line directives that specify the reported_filename since
//   the reported error line number refers to its location in the reported filename.
// - If a #line directive does not have a filename, the file is the reported filename, or
//   the filename found in a prior #line directive.  (This is C-preprocessor behavior)
// - It is possible (e.g., inlining) for blocks of code to get shuffled out of their
//   original order and the #line directives are used to keep the numbering correct.  This
//   is why we need to examine the entire contents of the source, instead of leaving early
//   when finding a #line line number larger than the reported error line number.
//

// GCC 4.8 has a problem with std::regex that is fixed in GCC 4.9.  Provide fallback code for 4.8
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if defined(__GNUC__) && GCC_VERSION < 40900
bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    // # line <linenumber> "<filename>" or
    // #line <linenumber> "<filename>"
    std::vector<std::string> tokens;
    std::stringstream stream(string);
    std::string temp;
    uint32_t line_index = 0;

    while (stream >> temp) tokens.push_back(temp);
    auto size = tokens.size();
    if (size > 1) {
        if (tokens[0] == "#" && tokens[1] == "line") {
            line_index = 2;
        } else if (tokens[0] == "#line") {
            line_index = 1;
        }
    }
    if (0 == line_index) return false;
    *linenumber = static_cast<uint32_t>(std::stoul(tokens[line_index]));
    uint32_t filename_index = line_index + 1;
    // Remove enclosing double quotes around filename
    if (size > filename_index) filename = tokens[filename_index].substr(1, tokens[filename_index].size() - 2);
    return true;
}
#else
bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    static const std::regex line_regex(  // matches #line directives
        "^"                              // beginning of line
        "\\s*"                           // optional whitespace
        "#"                              // required text
        "\\s*"                           // optional whitespace
        "line"                           // required text
        "\\s+"                           // required whitespace
        "([0-9]+)"                       // required first capture - line number
        "(\\s+)?"                        // optional second capture - whitespace
        "(\".+\")?"                      // optional third capture - quoted filename with at least one char inside
        ".*");                           // rest of line (needed when using std::regex_match since the entire line is tested)

    std::smatch captures;

    bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = std::stoul(captures[1]);
    return true;
}
#endif  // GCC_VERSION

// Extract the filename, line number, and column number from the correct OpLine and build a message string from it.
// Scan the source (from OpSource) to find the line of source at the reported line number and place it in another message string.
void UtilGenerateSourceMessages(const std::vector<unsigned int> &pgm, const uint32_t *debug_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg) {
    using namespace spvtools;
    std::ostringstream filename_stream;
    std::ostringstream source_stream;
    SHADER_MODULE_STATE shader;
    shader.words = pgm;
    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t instruction_index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    if (shader.words.size() > 0) {
        for (auto insn : shader) {
            if (insn.opcode() == spv::OpLine) {
                reported_file_id = insn.word(1);
                reported_line_number = insn.word(2);
                reported_column_number = insn.word(3);
            }
            if (instruction_index == debug_record[kInstCommonOutInstructionIdx]) {
                break;
            }
            instruction_index++;
        }
    }
    // Create message with file information obtained from the OpString pointed to by the discovered OpLine.
    std::string reported_filename;
    if (reported_file_id == 0) {
        filename_stream
            << "Unable to find SPIR-V OpLine for source information.  Build shader with debug info to get source information.";
    } else {
        bool found_opstring = false;
        std::string prefix;
        if (from_printf) {
            prefix = "Debug shader printf message generated ";
        } else {
            prefix = "Shader validation error occurred ";
        }
        for (auto insn : shader) {
            if ((insn.opcode() == spv::OpString) && (insn.len() >= 3) && (insn.word(1) == reported_file_id)) {
                found_opstring = true;
                reported_filename = (char *)&insn.word(2);
                if (reported_filename.empty()) {
                    filename_stream << prefix << "at line " << reported_line_number;
                } else {
                    filename_stream << prefix << "in file " << reported_filename << " at line " << reported_line_number;
                }
                if (reported_column_number > 0) {
                    filename_stream << ", column " << reported_column_number;
                }
                filename_stream << ".";
                break;
            }
        }
        if (!found_opstring) {
            filename_stream << "Unable to find SPIR-V OpString for file id " << reported_file_id << " from OpLine instruction."
                            << std::endl;
            filename_stream << "File ID = " << reported_file_id << ", Line Number = " << reported_line_number
                            << ", Column = " << reported_column_number << std::endl;
        }
    }
    filename_msg = filename_stream.str();

    // Create message to display source code line containing error.
    if ((reported_file_id != 0)) {
        // Read the source code and split it up into separate lines.
        std::vector<std::string> opsource_lines;
        ReadOpSource(shader, reported_file_id, opsource_lines);
        // Find the line in the OpSource content that corresponds to the reported error file and line.
        if (!opsource_lines.empty()) {
            uint32_t saved_line_number = 0;
            std::string current_filename = reported_filename;  // current "preprocessor" filename state.
            std::vector<std::string>::size_type saved_opsource_offset = 0;
            bool found_best_line = false;
            for (auto it = opsource_lines.begin(); it != opsource_lines.end(); ++it) {
                uint32_t parsed_line_number;
                std::string parsed_filename;
                bool found_line = GetLineAndFilename(*it, &parsed_line_number, parsed_filename);
                if (!found_line) continue;

                bool found_filename = parsed_filename.size() > 0;
                if (found_filename) {
                    current_filename = parsed_filename;
                }
                if ((!found_filename) || (current_filename == reported_filename)) {
                    // Update the candidate best line directive, if the current one is prior and closer to the reported line
                    if (reported_line_number >= parsed_line_number) {
                        if (!found_best_line ||
                            (reported_line_number - parsed_line_number <= reported_line_number - saved_line_number)) {
                            saved_line_number = parsed_line_number;
                            saved_opsource_offset = std::distance(opsource_lines.begin(), it);
                            found_best_line = true;
                        }
                    }
                }
            }
            if (found_best_line) {
                assert(reported_line_number >= saved_line_number);
                std::vector<std::string>::size_type opsource_index =
                    (reported_line_number - saved_line_number) + 1 + saved_opsource_offset;
                if (opsource_index < opsource_lines.size()) {
                    source_stream << "\n" << reported_line_number << ": " << opsource_lines[opsource_index].c_str();
                } else {
                    source_stream << "Internal error: calculated source line of " << opsource_index << " for source size of "
                                  << opsource_lines.size() << " lines.";
                }
            } else {
                source_stream << "Unable to find suitable #line directive in SPIR-V OpSource.";
            }
        } else {
            source_stream << "Unable to find SPIR-V OpSource.";
        }
    }
    source_msg = source_stream.str();
}
