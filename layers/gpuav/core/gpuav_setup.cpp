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

#include <array>
#include <string>
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
#include <unistd.h>
#endif
#include "chassis/dispatch_object.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/instrumentation/descriptor_checks.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "utils/dispatch_utils.h"
#include "utils/math_utils.h"

namespace gpuav {

// Location to add per-queue submit debug info if built with -D DEBUG_CAPTURE_KEYBOARD=ON
void Validator::DebugCapture() {}

void Validator::Created(vvl::DescriptorSet &set) {
    set.SetSubState(container_type, std::make_unique<DescriptorSetSubState>(set, *this));
}

void Validator::Created(vvl::CommandBuffer &cb_state) {
    cb_state.SetSubState(container_type, std::make_unique<CommandBufferSubState>(*this, cb_state));
}

void Validator::Created(vvl::Queue &queue) { queue.SetSubState(container_type, std::make_unique<QueueSubState>(*this, queue)); }

void Validator::Created(vvl::Image &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<ImageSubState>(obj, desc_heap));
}
void Validator::Created(vvl::ImageView &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<ImageViewSubState>(obj, desc_heap));
}
void Validator::Created(vvl::Buffer &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<BufferSubState>(obj, desc_heap));
}
void Validator::Created(vvl::BufferView &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<BufferViewSubState>(obj, desc_heap));
}
void Validator::Created(vvl::Sampler &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<SamplerSubState>(obj, desc_heap));
}
void Validator::Created(vvl::AccelerationStructureNV &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<AccelerationStructureNVSubState>(obj, desc_heap));
}
void Validator::Created(vvl::AccelerationStructureKHR &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<AccelerationStructureKHRSubState>(obj, desc_heap));
}
void Validator::Created(vvl::Tensor &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<TensorSubState>(obj, desc_heap));
}
void Validator::Created(vvl::TensorView &obj) {
    DescriptorHeap &desc_heap = shared_resources_manager.Get<DescriptorHeap>();
    obj.SetSubState(container_type, std::make_unique<TensorViewSubState>(obj, desc_heap));
}
void Validator::Created(vvl::ShaderObject &obj) { obj.SetSubState(container_type, std::make_unique<ShaderObjectSubState>(obj)); }

void Validator::Created(vvl::Pipeline &obj) { obj.SetSubState(container_type, std::make_unique<PipelineSubState>(*this, obj)); }

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetInstanceProcAddr(VkInstance inst, const char *name) {
    return DispatchGetInstanceProcAddr(inst, name);
}
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetDeviceProcAddr(VkDevice dev, const char *name) {
    return DispatchGetDeviceProcAddr(dev, name);
}
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

static VkResult UtilInitializeVma(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
                                  VmaAllocator *pAllocator) {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.instance = instance;
    allocator_info.device = device;
    allocator_info.physicalDevice = physical_device;

    allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    functions.vkGetInstanceProcAddr = static_cast<PFN_vkGetInstanceProcAddr>(gpuVkGetInstanceProcAddr);
    functions.vkGetDeviceProcAddr = static_cast<PFN_vkGetDeviceProcAddr>(gpuVkGetDeviceProcAddr);
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

void Instance::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, const RecordObject &record_obj,
                                         vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);

    // GPU-AV requirements not met, exit early or future Vulkan calls may be invalid
    if (api_version < VK_API_VERSION_1_1) {
        return;
    }

    AddFeatures(physicalDevice, modified_create_info, record_obj.location);
}

// Perform initializations that can be done at Create Device time.
void Validator::FinishDeviceSetup(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // GPU-AV not supported, exit early to prevent errors inside Validator::PostCallRecordCreateDevice
    if (api_version < VK_API_VERSION_1_1) {
        InternalError(device, loc, "GPU Shader Instrumentation requires Vulkan 1.1 or later.");
        return;
    }

    instrumentation_bindings_ = {
        // DebugPrintf Output buffer
        {glsl::kBindingInstDebugPrintf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Error output buffer
        {glsl::kBindingInstErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding output from GPU to do processing on the CPU
        {glsl::kBindingInstPostProcess, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding input from CPU into the shader for descriptor indexing
        {glsl::kBindingInstDescriptorIndexingOOB, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding buffer device addresses
        {glsl::kBindingInstBufferDeviceAddress, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding action command index in command buffer (a global buffer is used)
        {glsl::kBindingInstActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding a resource index from the per command buffer command resources list
        {glsl::kBindingInstCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Commands errors counts buffer
        {glsl::kBindingInstCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Vertex attribute fetch limits
        {glsl::kBindingInstVertexAttributeFetchLimits, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
    };
    assert(instrumentation_bindings_.size() == glsl::kTotalBindings);

    // TODO - Now that GPU-AV and DebugPrintf are merged, we should just have a single FinishDeviceSetup if possible (or at least
    // better divide what belongs where as it is easy to mess)
    BaseClass::FinishDeviceSetup(pCreateInfo, loc);
    // We might fail in parent class device creation if global requirements are not met
    if (aborted_) {
        return;
    }

    // Need the device to be created before we can query features for settings
    InitSettings(loc);

    VkResult result = UtilInitializeVma(instance, physical_device, device, &vma_allocator_);
    if (result != VK_SUCCESS) {
        InternalVmaError(device, result, "Could not initialize VMA");
        return;
    }

    desc_set_manager_ =
        std::make_unique<vko::DescriptorSetManager>(device, static_cast<uint32_t>(instrumentation_bindings_.size()));

    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    {
        auto chain_info = GetChainInfo(pCreateInfo, VK_LOADER_DATA_CALLBACK);
        assert(chain_info->u.pfnSetDeviceLoaderData);
        vk_set_device_loader_data_ = chain_info->u.pfnSetDeviceLoaderData;
    }

    DescriptorChecksOnFinishDeviceSetup(*this);

    // Create error logging buffer allocation pool
    {
        VkBufferCreateInfo error_buffer_ci = vku::InitStructHelper();
        error_buffer_ci.size = glsl::kErrorBufferByteSize;
        error_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo error_buffer_alloc_ci = {};
        error_buffer_alloc_ci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        error_buffer_alloc_ci.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t mem_type_index;
        result = vmaFindMemoryTypeIndexForBufferInfo(vma_allocator_, &error_buffer_ci, &error_buffer_alloc_ci, &mem_type_index);
        if (result != VK_SUCCESS) {
            InternalVmaError(device, result, "Unable to find memory type index.");
            return;
        }
    }

    // Create command indices buffer
    {
        const uint32_t index_size = sizeof(uint32_t);
        indices_buffer_alignment_ = Align(index_size, (uint32_t)phys_dev_props.limits.minStorageBufferOffsetAlignment);

        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffer_info.size = cst::indices_count * indices_buffer_alignment_;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const bool success = global_indices_buffer_.Create(&buffer_info, &alloc_info);
        if (!success) {
            return;
        }

        uint32_t stride = indices_buffer_alignment_ / sizeof(uint32_t);
        uint32_t *indices_ptr = (uint32_t *)global_indices_buffer_.GetMappedPtr();
        for (uint32_t i = 0; i < cst::indices_count; ++i) {
            const uint32_t offset = i * stride;
            indices_ptr[offset] = i;
        }
    }

    // Create our own Descriptor Buffer we will bind if the user decides to use it
    if (IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = phys_dev_ext_props.descriptor_buffer_props.storageBufferDescriptorSize * cst::total_internal_descriptors;
        buffer_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const bool success = global_resource_descriptor_buffer_.Create(&buffer_info, &alloc_info);
        if (!success) {
            InternalVmaError(device, result, "Failed to create an internal resource Descriptor Buffer.");
            return;
        }
    }
}

namespace setting {

// Each setting in GPU-AV has a common interface to make adding a new setting easier
struct Setting {
    virtual bool IsEnabled(const GpuAVSettings &settings) = 0;
    virtual bool HasRequiredFeatures(const DeviceFeatures &features) = 0;
    virtual void Disable(GpuAVSettings &settings) = 0;
    virtual std::string DisableMessage() = 0;
};

struct BufferDeviceAddress : public Setting {
    bool IsEnabled(const GpuAVSettings &settings) { return settings.shader_instrumentation.buffer_device_address; }
    bool HasRequiredFeatures(const DeviceFeatures &features) { return features.shaderInt64; }
    void Disable(GpuAVSettings &settings) { settings.shader_instrumentation.buffer_device_address = false; }
    std::string DisableMessage() {
        return "Buffer Device Address validation option was enabled, but the shaderInt64 feature was not supported [Disabling "
               "gpuav_buffer_address_oob]";
    }
};

struct RayQuery : public Setting {
    bool IsEnabled(const GpuAVSettings &settings) { return settings.shader_instrumentation.ray_query; }
    bool HasRequiredFeatures(const DeviceFeatures &features) { return features.rayQuery; }
    void Disable(GpuAVSettings &settings) { settings.shader_instrumentation.ray_query = false; }
    std::string DisableMessage() {
        return "Ray Query validation option was enabled, but the rayQuery feature was not supported [Disabling "
               "gpuav_validate_ray_query]";
    }
};
struct BufferCopies : public Setting {
    bool IsEnabled(const GpuAVSettings &settings) { return settings.validate_buffer_copies; }
    // copy_buffer_to_image.comp relies on uint8_t buffers to perform validation
    bool HasRequiredFeatures(const DeviceFeatures &features) { return features.storageBuffer8BitAccess; }
    void Disable(GpuAVSettings &settings) { settings.validate_buffer_copies = false; }
    std::string DisableMessage() {
        return "Buffer copies option was enabled, but the storageBuffer8BitAccess feature was not supported [Disabling "
               "gpuav_buffer_copies]";
    }
};
struct BufferContent : public Setting {
    bool IsEnabled(const GpuAVSettings &settings) { return settings.IsBufferValidationEnabled(); }
    bool HasRequiredFeatures(const DeviceFeatures &features) { return features.shaderInt64; }
    void Disable(GpuAVSettings &settings) { settings.SetBufferValidationEnabled(false); }
    std::string DisableMessage() {
        return "Buffer content validation option was enabled, but the shaderInt64 feature was not supported [Disabling "
               "gpuav_buffers_validation]";
    }
};
}  // namespace setting

// At this point extensions/features may have been turned on by us in PreCallRecord.
// Now that we have all the information, here is where we might disable GPU-AV settings that are missing requirements
void Validator::InitSettings(const Location &loc) {
    setting::BufferDeviceAddress buffer_device_address;
    setting::RayQuery ray_query;
    setting::BufferCopies buffer_copies;
    setting::BufferContent buffer_content;
    std::array<setting::Setting *, 4> all_settings = {&buffer_device_address, &ray_query, &buffer_copies, &buffer_content};

    for (auto &setting_object : all_settings) {
        if (setting_object->IsEnabled(gpuav_settings) && !setting_object->HasRequiredFeatures(modified_features)) {
            setting_object->Disable(gpuav_settings);
            AdjustmentWarning(device, loc, setting_object->DisableMessage().c_str());
        }
    }

    if (IsExtEnabled(extensions.vk_ext_descriptor_buffer) && !gpuav_settings.descriptor_buffer_override) {
        // "can" work, just need more testing now
        AdjustmentWarning(
            device, loc,
            "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
            "[Disabling all shader instrumentation checks] (this does NOT include debug print)"
            "\nThere is a VK_LAYER_GPUAV_DESCRIPTOR_BUFFER_OVERRIDE that can be set to bypass this if you know you are not going "
            "to use descriptor buffers.");
        gpuav_settings.DisableShaderInstrumentationAndOptions();

        if (phys_dev_ext_props.descriptor_buffer_props.maxResourceDescriptorBufferBindings == 1) {
            if (gpuav_settings.debug_printf_enabled) {
                AdjustmentWarning(
                    device, loc,
                    "VK_EXT_descriptor_buffer is enabled with a device that only supports maxResourceDescriptorBufferBindings of "
                    "1\nNeed to disable DebugPrintf as we currently don't have a fallback path. [Disabling debug_printf]"
                    "\nThere is a VK_LAYER_GPUAV_DESCRIPTOR_BUFFER_OVERRIDE that can be set to bypass this if you know you "
                    "are not going to use descriptor buffers.");
                gpuav_settings.debug_printf_enabled = false;
            }
        }
    }

    // If we have turned off all the possible things to instrument, turn off everything fully
    if (!gpuav_settings.IsShaderInstrumentationEnabled()) {
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    }

    gpuav_settings.TracyLogSettings();
}

void Validator::InternalVmaError(LogObjectList objlist, VkResult result, const char *const specific_message) const {
    aborted_ = true;
    std::string error_message = specific_message;

    char *stats_string;
    vmaBuildStatsString(vma_allocator_, &stats_string, false);
    error_message += " VMA statistics = ";
    error_message += stats_string;
    vmaFreeStatsString(vma_allocator_, stats_string);

    const char *layer_name = gpuav_settings.debug_printf_only ? "DebugPrintf" : "GPU-AV";
    const char *vuid = gpuav_settings.debug_printf_only ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, Location(vvl::Func::Empty), "Internal VMA Error (%s), %s is being disabled. Details:\n%s",
             string_VkResult(result), layer_name, error_message.c_str());

    // Once we encounter an internal issue disconnect everything.
    // This prevents need to check "if (aborted)" (which is awful when we easily forget to check somewhere and the user gets spammed
    // with errors making it hard to see the first error with the real source of the problem).
    dispatch_device_->ReleaseValidationObject(LayerObjectTypeGpuAssisted);
}

// On machines where all memory types have both DEVICE_LOCAL and HOST_VISIBLE we need to let VMA know there will be host access,
// otherwise it will assert https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/issues/515
bool Validator::IsAllDeviceLocalMappable() const {
    VkPhysicalDeviceMemoryProperties mem_props;
    DispatchGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
        const VkMemoryPropertyFlags property_flags = mem_props.memoryTypes[i].propertyFlags;
        const bool has_device_local = (property_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
        const bool has_host_visible = (property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        if (has_device_local && !has_host_visible) {
            return false;
        }
    }

    return true;
}

// Things like DescriptorHeap are singleton class that lives in GPU-AV, but are used when state tracking adds/destroy new resources
// we need to track. One issue is on vkDestroyDevice we need to teardown the GPU-AV class, then after we try and destroy leaked
// state objects (ex. user forgot to call vkDestroySampler).
void Validator::DestroySubstate() {
    if (!dispatch_device_ || aborted_) {
        return;
    }

    // While this is not ideal, it is more important to keep normal code fast and do extra cleanup on teardown
    for (auto object_it = dispatch_device_->object_dispatch.begin(); object_it != dispatch_device_->object_dispatch.end();
         object_it++) {
        if ((*object_it)->container_type == LayerObjectTypeStateTracker) {
            auto &state_tracker = dynamic_cast<vvl::DeviceState &>(**object_it);
            state_tracker.RemoveSubState(LayerObjectTypeGpuAssisted);
        }
    }
}

}  // namespace gpuav
