/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
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
 * Author: Camden Stocker <camden@lunarg.com>
 */

#include "best_practices_validation.h"
#include "layer_chassis_dispatch.h"
#include "best_practices_error_enums.h"
#include "shader_validation.h"

#include <string>
#include <bitset>
#include <memory>

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

const std::map<BPVendorFlagBits, VendorSpecificInfo> vendor_info = {
    {kBPVendorArm, {vendor_specific_arm, "Arm"}},
};

bool BestPractices::VendorCheckEnabled(BPVendorFlags vendors) const {
    for (const auto& vendor : vendor_info) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* VendorSpecificTag(BPVendorFlags vendors) {
    // Cache built vendor tags in a map
    static std::unordered_map<BPVendorFlags, std::string> tag_map;

    auto res = tag_map.find(vendors);
    if (res == tag_map.end()) {
        // Build the vendor tag string
        std::stringstream vendor_tag;

        vendor_tag << "[";
        bool first_vendor = true;
        for (const auto& vendor : vendor_info) {
            if (vendors & vendor.first) {
                if (!first_vendor) {
                    vendor_tag << ", ";
                }
                vendor_tag << vendor.second.name;
                first_vendor = false;
            }
        }
        vendor_tag << "]";

        tag_map[vendors] = vendor_tag.str();
        res = tag_map.find(vendors);
    }

    return res->second.c_str();
}

const char* DepReasonToString(ExtDeprecationReason reason) {
    switch (reason) {
        case kExtPromoted:
            return "promoted to";
            break;
        case kExtObsoleted:
            return "obsoleted by";
            break;
        case kExtDeprecated:
            return "deprecated by";
            break;
        default:
            return "";
            break;
    }
}

bool BestPractices::ValidateDeprecatedExtensions(const char* api_name, const char* extension_name, uint32_t version,
                                                 const char* vuid) const {
    bool skip = false;
    auto dep_info_it = deprecated_extensions.find(extension_name);
    if (dep_info_it != deprecated_extensions.end()) {
        auto dep_info = dep_info_it->second;
        if (((dep_info.target.compare("VK_VERSION_1_1") == 0) && (version >= VK_API_VERSION_1_1)) ||
            ((dep_info.target.compare("VK_VERSION_1_2") == 0) && (version >= VK_API_VERSION_1_2))) {
            skip |=
                LogWarning(instance, vuid, "%s(): Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                           api_name, extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
        } else if (dep_info.target.find("VK_VERSION") == std::string::npos) {
            if (dep_info.target.length() == 0) {
                skip |= LogWarning(instance, vuid,
                                   "%s(): Attempting to enable deprecated extension %s, but this extension has been deprecated "
                                   "without replacement.",
                                   api_name, extension_name);
            } else {
                skip |= LogWarning(instance, vuid,
                                   "%s(): Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                                   api_name, extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
            }
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            skip |= LogWarning(instance, kVUID_BestPractices_CreateInstance_ExtensionMismatch,
                               "vkCreateInstance(): Attempting to enable Device Extension %s at CreateInstance time.",
                               pCreateInfo->ppEnabledExtensionNames[i]);
        }
        uint32_t specified_version =
            (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
        skip |= ValidateDeprecatedExtensions("CreateInstance", pCreateInfo->ppEnabledExtensionNames[i], specified_version,
                                             kVUID_BestPractices_CreateInstance_DeprecatedExtension);
    }

    return skip;
}

void BestPractices::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance) {
    ValidationStateTracker::PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);

    if (pCreateInfo != nullptr && pCreateInfo->pApplicationInfo != nullptr)
        instance_api_version = pCreateInfo->pApplicationInfo->apiVersion;
    else
        instance_api_version = 0;
}

bool BestPractices::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    auto device_api_version = physical_device_properties.apiVersion;

    // check api versions and warn if instance api Version is higher than version on device.
    if (instance_api_version > device_api_version) {
        std::string inst_api_name = StringAPIVersion(instance_api_version);
        std::string dev_api_name = StringAPIVersion(device_api_version);

        skip |= LogWarning(device, kVUID_BestPractices_CreateDevice_API_Mismatch,
                           "vkCreateDevice(): API Version of current instance, %s is higher than API Version on device, %s",
                           inst_api_name.c_str(), dev_api_name.c_str());
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kInstanceExtensionNames)) {
            skip |= LogWarning(instance, kVUID_BestPractices_CreateDevice_ExtensionMismatch,
                               "vkCreateDevice(): Attempting to enable Instance Extension %s at CreateDevice time.",
                               pCreateInfo->ppEnabledExtensionNames[i]);
        }
        skip |= ValidateDeprecatedExtensions("CreateDevice", pCreateInfo->ppEnabledExtensionNames[i], instance_api_version,
                                             kVUID_BestPractices_CreateDevice_DeprecatedExtension);
    }

    auto pd_state = GetPhysicalDeviceState(physicalDevice);
    if ((pd_state->vkGetPhysicalDeviceFeaturesState == UNCALLED) && (pCreateInfo->pEnabledFeatures != NULL)) {
        skip |= LogWarning(device, kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled,
                           "vkCreateDevice() called before getting physical device features from vkGetPhysicalDeviceFeatures().");
    }

    if ((VendorCheckEnabled(kBPVendorArm)) && (pCreateInfo->pEnabledFeatures != nullptr) &&
        (pCreateInfo->pEnabledFeatures->robustBufferAccess == VK_TRUE)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateDevice_RobustBufferAccess,
            "%s vkCreateDevice() called with enabled robustBufferAccess. Use robustBufferAccess as a debugging tool during "
            "development. Enabling it causes loss in performance for accesses to uniform buffers and shader storage "
            "buffers. Disable robustBufferAccess in release builds. Only leave it enabled if the application use-case "
            "requires the additional level of reliability due to the use of unverified user-supplied draw parameters.",
            VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream bufferHex;
        bufferHex << "0x" << std::hex << HandleToUint64(pBuffer);

        skip |= LogWarning(
            device, kVUID_BestPractices_SharingModeExclusive,
            "Warning: Buffer (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
            "(queueFamilyIndexCount of %" PRIu32 ").",
            bufferHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream imageHex;
        imageHex << "0x" << std::hex << HandleToUint64(pImage);

        skip |=
            LogWarning(device, kVUID_BestPractices_SharingModeExclusive,
                       "Warning: Image (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                       "(queueFamilyIndexCount of %" PRIu32 ").",
                       imageHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        if (pCreateInfo->samples > kMaxEfficientSamplesArm) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateImage_TooLargeSampleCount,
                "%s vkCreateImage(): Trying to create an image with %u samples. "
                "The hardware revision may not have full throughput for framebuffers with more than %u samples.",
                VendorSpecificTag(kBPVendorArm), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesArm);
        }

        if (pCreateInfo->samples > VK_SAMPLE_COUNT_1_BIT && !(pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateImage_NonTransientMSImage,
                "%s vkCreateImage(): Trying to create a multisampled image, but createInfo.usage did not have "
                "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. Multisampled images may be resolved on-chip, "
                "and do not need to be backed by physical storage. "
                "TRANSIENT_ATTACHMENT allows tiled GPUs to not back the multisampled image with physical memory.",
                VendorSpecificTag(kBPVendorArm));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    bool skip = false;

    auto physical_device_state = GetPhysicalDeviceState();

    if (physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState == UNCALLED) {
        skip |= LogWarning(
            device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
            "vkCreateSwapchainKHR() called before getting surface capabilities from vkGetPhysicalDeviceSurfaceCapabilitiesKHR().");
    }

    if (physical_device_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS) {
        skip |= LogWarning(device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                           "vkCreateSwapchainKHR() called before getting surface present mode(s) from "
                           "vkGetPhysicalDeviceSurfacePresentModesKHR().");
    }

    if (physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
        skip |= LogWarning(
            device, kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
            "vkCreateSwapchainKHR() called before getting surface format(s) from vkGetPhysicalDeviceSurfaceFormatsKHR().");
    }

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |=
            LogWarning(device, kVUID_BestPractices_SharingModeExclusive,
                       "Warning: A Swapchain is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while "
                       "specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                       pCreateInfo->queueFamilyIndexCount);
    }

    if (pCreateInfo->minImageCount == 2) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_SuboptimalSwapchainImageCount,
            "Warning: A Swapchain is being created with minImageCount set to %" PRIu32
            ", which means double buffering is going "
            "to be used. Using double buffering and vsync locks rendering to an integer fraction of the vsync rate. In turn, "
            "reducing the performance of the application if rendering is slower than vsync. Consider setting minImageCount to "
            "3 to use triple buffering to maximize performance in such cases.",
            pCreateInfo->minImageCount);
    }

    if (VendorCheckEnabled(kBPVendorArm) && (pCreateInfo->presentMode != VK_PRESENT_MODE_FIFO_KHR)) {
        skip |= LogWarning(device, kVUID_BestPractices_CreateSwapchain_PresentMode,
                           "%s Warning: Swapchain is not being created with presentation mode \"VK_PRESENT_MODE_FIFO_KHR\". "
                           "Prefer using \"VK_PRESENT_MODE_FIFO_KHR\" to avoid unnecessary CPU and GPU load and save power. "
                           "Presentation modes which are not FIFO will present the latest available frame and discard other "
                           "frame(s) if any.",
                           VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSwapchainKHR* pSwapchains) const {
    bool skip = false;

    for (uint32_t i = 0; i < swapchainCount; i++) {
        if ((pCreateInfos[i].queueFamilyIndexCount > 1) && (pCreateInfos[i].imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
            skip |= LogWarning(
                device, kVUID_BestPractices_SharingModeExclusive,
                "Warning: A shared swapchain (index %" PRIu32
                ") is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple "
                "queues (queueFamilyIndexCount of %" PRIu32 ").",
                i, pCreateInfos[i].queueFamilyIndexCount);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((FormatIsColor(format) || FormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(device, kVUID_BestPractices_RenderPass_Attatchment,
                                   "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                   "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
            if (FormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(device, kVUID_BestPractices_RenderPass_Attatchment,
                                   "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD "
                                   "and initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
        }

        const auto& attachment = pCreateInfo->pAttachments[i];
        if (attachment.samples > VK_SAMPLE_COUNT_1_BIT) {
            bool access_requires_memory =
                attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE;

            if (FormatHasStencil(format)) {
                access_requires_memory |= attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
                                          attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE;
            }

            if (access_requires_memory) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory,
                    "Attachment %u in the VkRenderPass is a multisampled image with %u samples, but it uses loadOp/storeOp "
                    "which requires accessing data from memory. Multisampled images should always be loadOp = CLEAR or DONT_CARE, "
                    "storeOp = DONT_CARE. This allows the implementation to use lazily allocated memory effectively.",
                    i, static_cast<uint32_t>(attachment.samples));
            }
        }
    }

    for (uint32_t dependency = 0; dependency < pCreateInfo->dependencyCount; dependency++) {
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].srcStageMask);
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].dstStageMask);
    }

    return skip;
}

bool BestPractices::ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachmentCount,
                                        const VkImageView* image_views) const {
    bool skip = false;

    // Check for non-transient attachments that should be transient and vice versa
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        auto& attachment = rpci->pAttachments[i];
        bool attachment_should_be_transient =
            (attachment.loadOp != VK_ATTACHMENT_LOAD_OP_LOAD && attachment.storeOp != VK_ATTACHMENT_STORE_OP_STORE);

        if (FormatHasStencil(attachment.format)) {
            attachment_should_be_transient &= (attachment.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_LOAD &&
                                               attachment.stencilStoreOp != VK_ATTACHMENT_STORE_OP_STORE);
        }

        auto view_state = GetImageViewState(image_views[i]);
        if (view_state) {
            auto& ivci = view_state->create_info;
            auto& ici = GetImageState(ivci.image)->createInfo;

            bool image_is_transient = (ici.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0;

            // The check for an image that should not be transient applies to all GPUs
            if (!attachment_should_be_transient && image_is_transient) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which need to access physical memory, "
                    "but the image backing the image view has VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "Physical memory will need to be backed lazily to this image, potentially causing stalls.",
                    i);
            }

            bool supports_lazy = false;
            for (uint32_t j = 0; j < phys_dev_mem_props.memoryTypeCount; j++) {
                if (phys_dev_mem_props.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                }
            }

            // The check for an image that should be transient only applies to GPUs supporting
            // lazily allocated memory
            if (supports_lazy && attachment_should_be_transient && !image_is_transient) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which never have to be backed by physical memory, "
                    "but the image backing the image view does not have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "You can save physical memory by using transient attachment backed by lazily allocated memory here.",
                    i);
            }
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
    bool skip = false;

    auto rp_state = GetRenderPassState(pCreateInfo->renderPass);
    if (rp_state && !(pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR)) {
        skip = ValidateAttachments(rp_state->createInfo.ptr(), pCreateInfo->attachmentCount, pCreateInfo->pAttachments);
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                          VkDescriptorSet* pDescriptorSets, void* ads_state_data) const {
    bool skip = false;
    skip |= ValidationStateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, ads_state_data);

    if (!skip) {
        const auto& pool_handle = pAllocateInfo->descriptorPool;
        auto iter = descriptor_pool_freed_count.find(pool_handle);
        // if the number of freed sets > 0, it implies they could be recycled instead if desirable
        // this warning is specific to Arm
        if (VendorCheckEnabled(kBPVendorArm) && iter != descriptor_pool_freed_count.end() && iter->second > 0) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_AllocateDescriptorSets_SuboptimalReuse,
                "%s Descriptor set memory was allocated via vkAllocateDescriptorSets() for sets which were previously freed in the "
                "same logical device. On some drivers or architectures it may be most optimal to re-use existing descriptor sets.",
                VendorSpecificTag(kBPVendorArm));
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                               VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state) {
    if (result == VK_SUCCESS) {
        // find the free count for the pool we allocated into
        auto iter = descriptor_pool_freed_count.find(pAllocateInfo->descriptorPool);
        if (iter != descriptor_pool_freed_count.end()) {
            // we record successful allocations by subtracting the allocation count from the last recorded free count
            const auto alloc_count = pAllocateInfo->descriptorSetCount;
            // clamp the unsigned subtraction to the range [0, last_free_count]
            if (iter->second > alloc_count)
                iter->second -= alloc_count;
            else
                iter->second = 0;
        }
    }
}

void BestPractices::PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                     const VkDescriptorSet* pDescriptorSets, VkResult result) {
    ValidationStateTracker::PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
    if (result == VK_SUCCESS) {
        // we want to track frees because we're interested in suggesting re-use
        auto iter = descriptor_pool_freed_count.find(descriptorPool);
        if (iter == descriptor_pool_freed_count.end()) {
            descriptor_pool_freed_count.insert(std::make_pair(descriptorPool, descriptorSetCount));
        } else {
            iter->second += descriptorSetCount;
        }
    }
}

bool BestPractices::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    bool skip = false;

    if (num_mem_objects + 1 > kMemoryObjectWarningLimit) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_AllocateMemory_TooManyObjects,
                                      "Performance Warning: This app has > %" PRIu32 " memory objects.", kMemoryObjectWarningLimit);
    }

    if (pAllocateInfo->allocationSize < kMinDeviceAllocationSize) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_AllocateMemory_SmallAllocation,
            "vkAllocateMemory(): Allocating a VkDeviceMemory of size %llu. This is a very small allocation (current "
            "threshold is %llu bytes). "
            "You should make large allocations and sub-allocate from one large VkDeviceMemory.",
            pAllocateInfo->allocationSize, kMinDeviceAllocationSize);
    }

    // TODO: Insert get check for GetPhysicalDeviceMemoryProperties once the state is tracked in the StateTracker

    return skip;
}

void BestPractices::ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                       VkResult result) {
    if (result != VK_SUCCESS) {
        static std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                                                    VK_ERROR_TOO_MANY_OBJECTS, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                                                    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        static std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkReleaseFullScreenExclusiveModeEXT", result, error_codes, success_codes);
        return;
    }
    num_mem_objects++;
}

void BestPractices::ValidateReturnCodes(const char* api_name, VkResult result, const std::vector<VkResult>& error_codes,
                                        const std::vector<VkResult>& success_codes) const {
    auto error = std::find(error_codes.begin(), error_codes.end(), result);
    if (error != error_codes.end()) {
        LogWarning(instance, kVUID_BestPractices_Error_Result, "%s(): Returned error %s.", api_name, string_VkResult(result));
        return;
    }
    auto success = std::find(success_codes.begin(), success_codes.end(), result);
    if (success != success_codes.end()) {
        LogInfo(instance, kVUID_BestPractices_NonSuccess_Result, "%s(): Returned non-success return code %s.", api_name,
                string_VkResult(result));
    }
}

bool BestPractices::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory,
                                              const VkAllocationCallbacks* pAllocator) const {
    if (memory == VK_NULL_HANDLE) return false;
    bool skip = false;

    const DEVICE_MEMORY_STATE* mem_info = ValidationStateTracker::GetDevMemState(memory);

    for (auto& obj : mem_info->obj_bindings) {
        LogObjectList objlist(device);
        objlist.add(obj);
        objlist.add(mem_info->mem);
        skip |= LogWarning(objlist, layer_name.c_str(), "VK Object %s still has a reference to mem obj %s.",
                           report_data->FormatHandle(obj).c_str(), report_data->FormatHandle(mem_info->mem).c_str());
    }

    return skip;
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    ValidationStateTracker::PreCallRecordFreeMemory(device, memory, pAllocator);
    if (memory != VK_NULL_HANDLE) {
        num_mem_objects--;
    }
}

bool BestPractices::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    const BUFFER_STATE* buffer_state = GetBufferState(buffer);

    if (!buffer_state->memory_requirements_checked && !buffer_state->external_memory_handle) {
        skip |= LogWarning(device, kVUID_BestPractices_BufferMemReqNotCalled,
                           "%s: Binding memory to %s but vkGetBufferMemoryRequirements() has not been called on that buffer.",
                           api_name, report_data->FormatHandle(buffer).c_str());
    }

    const DEVICE_MEMORY_STATE* mem_state = GetDevMemState(memory);

    if (mem_state->alloc_info.allocationSize == buffer_state->createInfo.size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_SmallDedicatedAllocation,
            "%s: Trying to bind %s to a memory block which is fully consumed by the buffer. "
            "The required size of the allocation is %llu, but smaller buffers like this should be sub-allocated from "
            "larger memory blocks. (Current threshold is %llu bytes.)",
            api_name, report_data->FormatHandle(buffer).c_str(), mem_state->alloc_info.allocationSize, kMinDedicatedAllocationSize);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                    VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "BindBufferMemory()";

    skip |= ValidateBindBufferMemory(buffer, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                        const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    const IMAGE_STATE* image_state = GetImageState(image);

    if (image_state->disjoint == false) {
        if (!image_state->memory_requirements_checked && !image_state->external_memory_handle) {
            skip |= LogWarning(device, kVUID_BestPractices_ImageMemReqNotCalled,
                               "%s: Binding memory to %s but vkGetImageMemoryRequirements() has not been called on that image.",
                               api_name, report_data->FormatHandle(image).c_str());
        }
    } else {
        // TODO If binding disjoint image then this needs to check that VkImagePlaneMemoryRequirementsInfo was called for each
        // plane.
    }

    const DEVICE_MEMORY_STATE* mem_state = GetDevMemState(memory);

    if (mem_state->alloc_info.allocationSize == image_state->requirements.size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_SmallDedicatedAllocation,
            "%s: Trying to bind %s to a memory block which is fully consumed by the image. "
            "The required size of the allocation is %llu, but smaller images like this should be sub-allocated from "
            "larger memory blocks. (Current threshold is %llu bytes.)",
            api_name, report_data->FormatHandle(image).c_str(), mem_state->alloc_info.allocationSize, kMinDedicatedAllocationSize);
    }

    // If we're binding memory to a image which was created as TRANSIENT and the image supports LAZY allocation,
    // make sure this type is actually used.
    // This warning will only trigger if this layer is run on a platform that supports LAZILY_ALLOCATED_BIT
    // (i.e.most tile - based renderers)
    if (image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        bool supports_lazy = false;
        uint32_t suggested_type = 0;

        for (uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++) {
            if ((1u << i) & image_state->requirements.memoryTypeBits) {
                if (phys_dev_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                    suggested_type = i;
                    break;
                }
            }
        }

        uint32_t allocated_properties = phys_dev_mem_props.memoryTypes[mem_state->alloc_info.memoryTypeIndex].propertyFlags;

        if (supports_lazy && (allocated_properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_NonLazyTransientImage,
                "%s: Attempting to bind memory type % u to VkImage which was created with TRANSIENT_ATTACHMENT_BIT,"
                "but this memory type is not LAZILY_ALLOCATED_BIT. You should use memory type %u here instead to save "
                "%llu bytes of physical memory.",
                api_name, mem_state->alloc_info.memoryTypeIndex, suggested_type, image_state->requirements.size);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "vkBindImageMemory()";

    skip |= ValidateBindImageMemory(image, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2() pBindInfos[%u]", i);
        if (!lvl_find_in_chain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[i].pNext)) {
            skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
    }

    return skip;
}

static inline bool FormatHasFullThroughputBlendingArm(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return false;

        default:
            return true;
    }
}

bool BestPractices::ValidateMultisampledBlendingArm(uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo* pCreateInfos) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto pCreateInfo = &pCreateInfos[i];

        if (!pCreateInfo->pColorBlendState || !pCreateInfo->pMultisampleState ||
            pCreateInfo->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT ||
            pCreateInfo->pMultisampleState->sampleShadingEnable) {
            return skip;
        }

        auto rp_state = GetRenderPassState(pCreateInfo->renderPass);
        auto& subpass = rp_state->createInfo.pSubpasses[pCreateInfo->subpass];

        for (uint32_t j = 0; j < pCreateInfo->pColorBlendState->attachmentCount; j++) {
            auto& blend_att = pCreateInfo->pColorBlendState->pAttachments[j];
            uint32_t att = subpass.pColorAttachments[j].attachment;

            if (att != VK_ATTACHMENT_UNUSED && blend_att.blendEnable && blend_att.colorWriteMask) {
                if (!FormatHasFullThroughputBlendingArm(rp_state->createInfo.pAttachments[att].format)) {
                    skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreatePipelines_MultisampledBlending,
                                                  "%s vkCreateGraphicsPipelines() - createInfo #%u: Pipeline is multisampled and "
                                                  "color attachment #%u makes use "
                                                  "of a format which cannot be blended at full throughput when using MSAA.",
                                                  VendorSpecificTag(kBPVendorArm), i, j);
                }
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           void* cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                     pAllocator, pPipelines, cgpl_state_data);
    create_graphics_pipeline_api_state* cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state*>(cgpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreatePipelines_MultiplePipelines,
            "Performance Warning: This vkCreateGraphicsPipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto& createInfo = pCreateInfos[i];

        if (!(cgpl_state->pipe_state[i]->active_shaders & VK_SHADER_STAGE_MESH_BIT_NV)) {
            auto& vertexInput = *createInfo.pVertexInputState;
            uint32_t count = 0;
            for (uint32_t j = 0; j < vertexInput.vertexBindingDescriptionCount; j++) {
                if (vertexInput.pVertexBindingDescriptions[j].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                    count++;
                }
            }
            if (count > kMaxInstancedVertexBuffers) {
                skip |= LogPerformanceWarning(
                    device, kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers,
                    "The pipeline is using %u instanced vertex buffers (current limit: %u), but this can be inefficient on the "
                    "GPU. If using instanced vertex attributes prefer interleaving them in a single buffer.",
                    count, kMaxInstancedVertexBuffers);
            }
        }

        if ((pCreateInfos[i].pRasterizationState->depthBiasEnable) &&
            (pCreateInfos[i].pRasterizationState->depthBiasConstantFactor == 0.0f) &&
            (pCreateInfos[i].pRasterizationState->depthBiasSlopeFactor == 0.0f)) {
            skip |= VendorCheckEnabled(kBPVendorArm) &&
                    LogPerformanceWarning(
                        device, kVUID_BestPractices_CreatePipelines_DepthBias_Zero,
                        "%s Performance Warning: This vkCreateGraphicsPipelines call is created with depthBiasEnable set to true "
                        "and both depthBiasConstantFactor and depthBiasSlopeFactor are set to 0. This can cause reduced "
                        "efficiency during rasterization. Consider disabling depthBias or increasing either "
                        "depthBiasConstantFactor or depthBiasSlopeFactor.",
                        VendorSpecificTag(kBPVendorArm));
        }

        skip |= VendorCheckEnabled(kBPVendorArm) && ValidateMultisampledBlendingArm(createInfoCount, pCreateInfos);
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                VkResult result, void* cgpl_state_data) {
    for (size_t i = 0; i < count; i++) {
        const auto* cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state*>(cgpl_state_data);
        const VkPipeline pipeline_handle = pPipelines[i];

        // record depth stencil state and color blend states for depth pre-pass tracking purposes
        auto gp_cis = graphicsPipelineCIs.find(pipeline_handle);

        // add the tracking state if it doesn't exist
        if (gp_cis == graphicsPipelineCIs.end()) {
            auto result = graphicsPipelineCIs.emplace(std::make_pair(pipeline_handle, GraphicsPipelineCIs{}));

            if (!result.second) continue;

            gp_cis = result.first;
        }

        gp_cis->second.colorBlendStateCI =
            cgpl_state->pCreateInfos[i].pColorBlendState
                ? new safe_VkPipelineColorBlendStateCreateInfo(cgpl_state->pCreateInfos[i].pColorBlendState)
                : nullptr;
        gp_cis->second.depthStencilStateCI =
            cgpl_state->pCreateInfos[i].pDepthStencilState
                ? new safe_VkPipelineDepthStencilStateCreateInfo(cgpl_state->pCreateInfos[i].pDepthStencilState)
                : nullptr;
    }
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          void* ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                    pAllocator, pPipelines, ccpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreatePipelines_MultiplePipelines,
            "Performance Warning: This vkCreateComputePipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        for (size_t i = 0; i < createInfoCount; i++) {
            skip |= ValidateCreateComputePipelineArm(pCreateInfos[i]);
        }
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& createInfo) const {
    bool skip = false;
    auto* module = GetShaderModuleState(createInfo.stage.module);

    uint32_t x = 1, y = 1, z = 1;
    FindLocalSize(module, x, y, z);

    uint32_t thread_count = x * y * z;

    // Generate a priori warnings about work group sizes.
    if (thread_count > kMaxEfficientWorkGroupThreadCountArm) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize,
            "%s vkCreateComputePipelines(): compute shader with work group dimensions (%u, %u, "
            "%u) (%u threads total), has more threads than advised in a single work group. It is advised to use work "
            "groups with less than %u threads, especially when using barrier() or shared memory.",
            VendorSpecificTag(kBPVendorArm), x, y, z, thread_count, kMaxEfficientWorkGroupThreadCountArm);
    }

    if (thread_count == 1 || ((x > 1) && (x & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((y > 1) && (y & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((z > 1) && (z & (kThreadGroupDispatchCountAlignmentArm - 1)))) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment,
                                      "%s vkCreateComputePipelines(): compute shader with work group dimensions (%u, "
                                      "%u, %u) is not aligned to %u "
                                      "threads. On Arm Mali architectures, not aligning work group sizes to %u may "
                                      "leave threads idle on the shader "
                                      "core.",
                                      VendorSpecificTag(kBPVendorArm), x, y, z, kThreadGroupDispatchCountAlignmentArm,
                                      kThreadGroupDispatchCountAlignmentArm);
    }

    // Generate warnings about work group sizes based on active resources.
    auto entrypoint = FindEntrypoint(module, createInfo.stage.pName, createInfo.stage.stage);
    if (entrypoint == module->end()) return false;

    bool has_writeable_descriptors = false;
    bool has_atomic_descriptors = false;
    auto accessible_ids = MarkAccessibleIds(module, entrypoint);
    auto descriptor_uses =
        CollectInterfaceByDescriptorSlot(module, accessible_ids, &has_writeable_descriptors, &has_atomic_descriptors);

    unsigned dimensions = 0;
    if (x > 1) dimensions++;
    if (y > 1) dimensions++;
    if (z > 1) dimensions++;
    // Here the dimension will really depend on the dispatch grid, but assume it's 1D.
    dimensions = std::max(dimensions, 1u);

    // If we're accessing images, we almost certainly want to have a 2D workgroup for cache reasons.
    // There are some false positives here. We could simply have a shader that does this within a 1D grid,
    // or we may have a linearly tiled image, but these cases are quite unlikely in practice.
    bool accesses_2d = false;
    for (const auto& usage : descriptor_uses) {
        auto dim = GetShaderResourceDimensionality(module, usage.second);
        if (dim < 0) continue;
        auto spvdim = spv::Dim(dim);
        if (spvdim != spv::Dim1D && spvdim != spv::DimBuffer) accesses_2d = true;
    }

    if (accesses_2d && dimensions < 2) {
        LogPerformanceWarning(device, kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality,
                              "%s vkCreateComputePipelines(): compute shader has work group dimensions (%u, %u, %u), which "
                              "suggests a 1D dispatch, but the shader is accessing 2D or 3D images. The shader may be "
                              "exhibiting poor spatial locality with respect to one or more shader resources.",
                              VendorSpecificTag(kBPVendorArm), x, y, z);
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(std::string api_name, const VkPipelineStageFlags flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT when %s is called\n", api_name.c_str());
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) {
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        auto swapchains_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (swapchains_result == VK_SUBOPTIMAL_KHR) {
            LogPerformanceWarning(
                pPresentInfo->pSwapchains[i], kVUID_BestPractices_SuboptimalSwapchain,
                "vkQueuePresentKHR: %s :VK_SUBOPTIMAL_KHR was returned. VK_SUBOPTIMAL_KHR - Presentation will still succeed, "
                "subject to the window resize behavior, but the swapchain is no longer configured optimally for the surface it "
                "targets. Applications should query updated surface information and recreate their swapchain at the next "
                "convenient opportunity.",
                report_data->FormatHandle(pPresentInfo->pSwapchains[i]).c_str());
        }
    }
}

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                               VkFence fence) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit", pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
    bool skip = false;

    if (pCreateInfo->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_CreateCommandPool_CommandBufferReset,
            "vkCreateCommandPool(): VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT is set. Consider resetting entire "
            "pool instead.");
    }

    return skip;
}

bool BestPractices::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                      const VkCommandBufferBeginInfo* pBeginInfo) const {
    bool skip = false;

    if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse,
                                      "vkBeginCommandBuffer(): VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT is set.");
    }

    if (!(pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)) {
        skip |= VendorCheckEnabled(kBPVendorArm) &&
                LogPerformanceWarning(device, kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit,
                                      "%s vkBeginCommandBuffer(): VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT is not set. "
                                      "For best performance on Mali GPUs, consider setting ONE_TIME_SUBMIT by default.",
                                      VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdSetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                 VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                     VkQueryPool queryPool, uint32_t query) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp", pipelineStage);

    return skip;
}

void BestPractices::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline) {
    StateTracker::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // check for depth/blend state tracking
        auto gp_cis = graphicsPipelineCIs.find(pipeline);
        if (gp_cis != graphicsPipelineCIs.end()) {
            auto prepass_state = cbDepthPrePassStates.find(commandBuffer);
            if (prepass_state == cbDepthPrePassStates.end()) {
                auto result = cbDepthPrePassStates.emplace(std::make_pair(commandBuffer, DepthPrePassState{}));

                if (!result.second) return;

                prepass_state = result.first;
            }

            const auto* blend_state = gp_cis->second.colorBlendStateCI;
            const auto* stencil_state = gp_cis->second.depthStencilStateCI;

            if (blend_state) {
                // assume the pipeline is depth-only unless any of the attachments have color writes enabled
                prepass_state->second.depthOnly = true;
                for (size_t i = 0; i < blend_state->attachmentCount; i++) {
                    if (blend_state->pAttachments[i].colorWriteMask != 0) {
                        prepass_state->second.depthOnly = false;
                    }
                }
            }

            // check for depth value usage
            prepass_state->second.depthEqualComparison = false;

            if (stencil_state && stencil_state->depthTestEnable) {
                switch (stencil_state->depthCompareOp) {
                    case VK_COMPARE_OP_EQUAL:
                    case VK_COMPARE_OP_GREATER_OR_EQUAL:
                    case VK_COMPARE_OP_LESS_OR_EQUAL:
                        prepass_state->second.depthEqualComparison = true;
                        break;
                    default:
                        break;
                }
            }
        } else {
            // reset depth pre-pass tracking
            cbDepthPrePassStates.emplace(std::make_pair(commandBuffer, DepthPrePassState{}));
        }
    }
}

static inline bool RenderPassUsesAttachmentOnTile(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        auto& subpassInfo = createInfo.pSubpasses[subpass];

        // If an attachment is ever used as a color attachment,
        // resolve attachment or depth stencil attachment,
        // it needs to exist on tile at some point.

        for (uint32_t i = 0; i < subpassInfo.colorAttachmentCount; i++)
            if (subpassInfo.pColorAttachments[i].attachment == attachment) return true;

        if (subpassInfo.pResolveAttachments) {
            for (uint32_t i = 0; i < subpassInfo.colorAttachmentCount; i++)
                if (subpassInfo.pResolveAttachments[i].attachment == attachment) return true;
        }

        if (subpassInfo.pDepthStencilAttachment && subpassInfo.pDepthStencilAttachment->attachment == attachment) return true;
    }

    return false;
}

bool BestPractices::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                               const VkRenderPassBeginInfo* pRenderPassBegin) const {
    bool skip = false;

    if (!pRenderPassBegin) {
        return skip;
    }

    auto rp_state = GetRenderPassState(pRenderPassBegin->renderPass);
    if (rp_state) {
        if (rp_state->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) {
            const VkRenderPassAttachmentBeginInfo* rpabi =
                lvl_find_in_chain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
            if (rpabi) {
                skip = ValidateAttachments(rp_state->createInfo.ptr(), rpabi->attachmentCount, rpabi->pAttachments);
            }
        }
        // Check if any attachments have LOAD operation on them
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            auto& attachment = rp_state->createInfo.pAttachments[att];

            bool attachmentHasReadback = false;
            if (!FormatHasStencil(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachmentHasReadback = true;
            }

            if (FormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachmentHasReadback = true;
            }

            bool attachmentNeedsReadback = false;

            // Check if the attachment is actually used in any subpass on-tile
            if (attachmentHasReadback && RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                attachmentNeedsReadback = true;
            }

            // Using LOAD_OP_LOAD is expensive on tiled GPUs, so flag it as a potential improvement
            if (attachmentNeedsReadback) {
                skip |= VendorCheckEnabled(kBPVendorArm) &&
                        LogPerformanceWarning(
                            device, kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback,
                            "%s Attachment #%u in render pass has begun with VK_ATTACHMENT_LOAD_OP_LOAD.\n"
                            "Submitting this renderpass will cause the driver to inject a readback of the attachment "
                            "which will copy in total %u pixels (renderArea = { %d, %d, %u, %u }) to the tile buffer.",
                            VendorSpecificTag(kBPVendorArm), att,
                            pRenderPassBegin->renderArea.extent.width * pRenderPassBegin->renderArea.extent.height,
                            pRenderPassBegin->renderArea.offset.x, pRenderPassBegin->renderArea.offset.y,
                            pRenderPassBegin->renderArea.extent.width, pRenderPassBegin->renderArea.extent.height);
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo* pRenderPassBegin,
                                                          const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
    return skip;
}

void BestPractices::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                             const VkRenderPassBeginInfo* pRenderPassBegin) {
    auto prepass_state = cbDepthPrePassStates.find(commandBuffer);

    // add the tracking state if it doesn't exist
    if (prepass_state == cbDepthPrePassStates.end()) {
        auto result = cbDepthPrePassStates.emplace(std::make_pair(commandBuffer, DepthPrePassState{}));

        if (!result.second) return;

        prepass_state = result.first;
    }

    // reset the renderpass state
    prepass_state->second = {};

    const auto* cb_state = GetCBState(commandBuffer);
    const auto* rp_state = cb_state->activeRenderPass.get();

    // track depth / color attachment usage within the renderpass
    for (size_t i = 0; i < rp_state->createInfo.subpassCount; i++) {
        // record if depth/color attachments are in use for this renderpass
        if (rp_state->createInfo.pSubpasses[i].pDepthStencilAttachment != nullptr) prepass_state->second.depthAttachment = true;

        if (rp_state->createInfo.pSubpasses[i].colorAttachmentCount > 0) prepass_state->second.colorAttachment = true;
    }
}

void BestPractices::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_1, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo* pRenderPassBegin,
                                                         const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, RENDER_PASS_VERSION_2, pRenderPassBegin);
}

// Generic function to handle validation for all CmdDraw* type functions
bool BestPractices::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const char* caller) const {
    bool skip = false;
    const CMD_BUFFER_STATE* cb_state = GetCBState(cmd_buffer);
    if (cb_state) {
        const auto last_bound_it = cb_state->lastBound.find(VK_PIPELINE_BIND_POINT_GRAPHICS);
        const PIPELINE_STATE* pipeline_state = nullptr;
        if (last_bound_it != cb_state->lastBound.cend()) {
            pipeline_state = last_bound_it->second.pipeline_state;
        }
        const auto& current_vtx_bfr_binding_info = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings;
        // Verify vertex binding
        if (pipeline_state->vertex_binding_descriptions_.size() <= 0) {
            if ((!current_vtx_bfr_binding_info.empty()) && (!cb_state->vertex_buffer_used)) {
                skip |= LogPerformanceWarning(cb_state->commandBuffer, kVUID_BestPractices_DrawState_VtxIndexOutOfBounds,
                                              "Vertex buffers are bound to %s but no vertex buffers are attached to %s.",
                                              report_data->FormatHandle(cb_state->commandBuffer).c_str(),
                                              report_data->FormatHandle(pipeline_state->pipeline).c_str());
            }
        }
    }
    return skip;
}

void BestPractices::RecordCmdDrawType(VkCommandBuffer cmd_buffer, uint32_t draw_count, const char* caller) {
    if (VendorCheckEnabled(kBPVendorArm)) {
        RecordCmdDrawTypeArm(cmd_buffer, draw_count, caller);
    }
}

void BestPractices::RecordCmdDrawTypeArm(VkCommandBuffer cmd_buffer, uint32_t draw_count, const char* caller) {
    auto prepass_state = cbDepthPrePassStates.find(cmd_buffer);
    if (prepass_state != cbDepthPrePassStates.end() && draw_count >= kDepthPrePassMinDrawCountArm) {
        if (prepass_state->second.depthOnly) prepass_state->second.numDrawCallsDepthOnly++;

        if (prepass_state->second.depthEqualComparison) prepass_state->second.numDrawCallsDepthEqualCompare++;
    }
}

bool BestPractices::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_InstanceCountZero,
                           "Warning: You are calling vkCmdDraw() with an instanceCount of Zero.");
        skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDraw()");
    }

    return skip;
}

void BestPractices::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                          uint32_t firstVertex, uint32_t firstInstance) {
    StateTracker::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    RecordCmdDrawType(commandBuffer, vertexCount * instanceCount, "vkCmdDraw()");
}

bool BestPractices::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_InstanceCountZero,
                           "Warning: You are calling vkCmdDrawIndexed() with an instanceCount of Zero.");
    }
    skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexed()");

    // Check if we reached the limit for small indexed draw calls.
    // Note that we cannot update the draw call count here, so we do it in PreCallRecordCmdDrawIndexed.
    const CMD_BUFFER_STATE* cmd_state = GetCBState(commandBuffer);
    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices &&
        (cmd_state->small_indexed_draw_call_count == kMaxSmallIndexedDrawcalls - 1)) {
        skip |= VendorCheckEnabled(kBPVendorArm) &&
                LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls,
                                      "The command buffer contains many small indexed drawcalls "
                                      "(at least %u drawcalls with less than %u indices each). This may cause pipeline bubbles. "
                                      "You can try batching drawcalls or instancing when applicable.",
                                      VendorSpecificTag(kBPVendorArm), kMaxSmallIndexedDrawcalls, kSmallIndexedDrawcallIndices);
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        ValidateIndexBufferArm(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    return skip;
}

bool BestPractices::ValidateIndexBufferArm(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;

    // check for sparse/underutilised index buffer, and post-transform cache thrashing
    const auto* cmd_state = GetCBState(commandBuffer);
    if (cmd_state == nullptr) return skip;

    const auto* ib_state = GetBufferState(cmd_state->index_buffer_binding.buffer);
    if (ib_state == nullptr) return skip;

    const VkIndexType ib_type = cmd_state->index_buffer_binding.index_type;
    const auto& ib_mem_state = *ib_state->binding.mem_state;
    const VkDeviceSize ib_mem_offset = ib_mem_state.mapped_range.offset;
    const void* ib_mem = ib_mem_state.p_driver_data;
    bool primitive_restart_enable = false;

    const auto& pipeline_binding_iter = cmd_state->lastBound.find(VK_PIPELINE_BIND_POINT_GRAPHICS);

    if (pipeline_binding_iter != cmd_state->lastBound.end()) {
        const auto* pipeline_state = pipeline_binding_iter->second.pipeline_state;
        if (pipeline_state != nullptr && pipeline_state->graphicsPipelineCI.pInputAssemblyState != nullptr)
            primitive_restart_enable = pipeline_state->graphicsPipelineCI.pInputAssemblyState->primitiveRestartEnable == VK_TRUE;
    }

    // no point checking index buffer if the memory is nonexistant/unmapped, or if there is no graphics pipeline bound to this CB
    if (ib_mem && pipeline_binding_iter != cmd_state->lastBound.end()) {
        uint32_t scan_stride;
        if (ib_type == VK_INDEX_TYPE_UINT8_EXT) {
            scan_stride = sizeof(uint8_t);
        } else if (ib_type == VK_INDEX_TYPE_UINT16) {
            scan_stride = sizeof(uint16_t);
        } else {
            scan_stride = sizeof(uint32_t);
        }

        const uint8_t* scan_begin = static_cast<const uint8_t*>(ib_mem) + ib_mem_offset + firstIndex * scan_stride;
        const uint8_t* scan_end = scan_begin + indexCount * scan_stride;

        // Min and max are important to track for some Mali architectures. In older Mali devices without IDVS, all
        // vertices corresponding to indices between the minimum and maximum may be loaded, and possibly shaded,
        // irrespective of whether or not they're part of the draw call.

        // start with minimum as 0xFFFFFFFF and adjust to indices in the buffer
        uint32_t min_index = ~0u;
        // start with maximum as 0 and adjust to indices in the buffer
        uint32_t max_index = 0u;

        // first scan-through, we're looking to simulate a model LRU post-transform cache, estimating the number of vertices shaded
        // for the given index buffer
        uint32_t vertex_shade_count = 0;

        PostTransformLRUCacheModel post_transform_cache;

        // The size of the cache being modelled positively correlates with how much behaviour it can capture about
        // arbitrary ground-truth hardware/architecture cache behaviour. I.e. it's a good solution when we don't know the
        // target architecture.
        // However, modelling a post-transform cache with more than 32 elements gives diminishing returns in practice.
        // http://eelpi.gotdns.org/papers/fast_vert_cache_opt.html
        post_transform_cache.resize(32);

        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            uint32_t primitive_restart_value;
            if (ib_type == VK_INDEX_TYPE_UINT8_EXT) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
                primitive_restart_value = 0xFF;
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
                primitive_restart_value = 0xFFFF;
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
                primitive_restart_value = 0xFFFFFFFF;
            }

            max_index = std::max(max_index, scan_index);
            min_index = std::min(min_index, scan_index);

            if (!primitive_restart_enable || scan_index != primitive_restart_value) {
                bool in_cache = post_transform_cache.query_cache(scan_index);
                // if the shaded vertex corresponding to the index is not in the PT-cache, we need to shade again
                if (!in_cache) vertex_shade_count++;
            }
        }

        // if the max and min values were not set, then we either have no indices, or all primitive restarts, exit...
        // if the max and min are the same, then it implies all the indices are the same, then we don't need to do anything
        if (max_index < min_index || max_index == min_index) return skip;

        if (max_index - min_index >= indexCount) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer,
                "%s The indices which were specified for the draw call only utilise approximately %.02f%% of "
                "index buffer value range. Arm Mali architectures before G71 do not have IDVS (Index-Driven "
                "Vertex Shading), meaning all vertices corresponding to indices between the minimum and "
                "maximum would be loaded, and possibly shaded, whether or not they are used.",
                VendorSpecificTag(kBPVendorArm), (static_cast<float>(indexCount) / (max_index - min_index)) * 100.0f);
            return skip;
        }

        // use a dynamic vector of bitsets as a memory-compact representation of which indices are included in the draw call
        // each bit of the n-th bucket contains the inclusion information for indices (n*n_buckets) to ((n+1)*n_buckets)
        const size_t refs_per_bucket = 64;
        std::vector<std::bitset<refs_per_bucket>> vertex_reference_buckets;

        const uint32_t n_indices = max_index - min_index + 1;
        const uint32_t n_buckets = (n_indices / static_cast<uint32_t>(refs_per_bucket)) +
                                   ((n_indices % static_cast<uint32_t>(refs_per_bucket)) != 0 ? 1 : 0);

        // there needs to be at least one bitset to store a set of indices smaller than n_buckets
        vertex_reference_buckets.resize(std::max(1u, n_buckets));

        // To avoid using too much memory, we run over the indices again.
        // Knowing the size from the last scan allows us to record index usage with bitsets
        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            if (ib_type == VK_INDEX_TYPE_UINT8_EXT) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
            }
            // keep track of the set of all indices used to reference vertices in the draw call
            size_t index_offset = scan_index - min_index;
            size_t bitset_bucket_index = index_offset / refs_per_bucket;
            uint64_t used_indices = 1ull << ((index_offset % refs_per_bucket) & 0xFFFFFFFFu);
            vertex_reference_buckets[bitset_bucket_index] |= used_indices;
        }

        uint32_t vertex_reference_count = 0;
        for (const auto& bitset : vertex_reference_buckets) {
            vertex_reference_count += static_cast<uint32_t>(bitset.count());
        }

        // low index buffer utilization implies that: of the vertices available to the draw call, not all are utilized
        float utilization = static_cast<float>(vertex_reference_count) / (max_index - min_index + 1);
        // low hit rate (high miss rate) implies the order of indices in the draw call may be possible to improve
        float cache_hit_rate = static_cast<float>(vertex_reference_count) / vertex_shade_count;

        if (utilization < 0.5f) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer,
                                          "%s The indices which were specified for the draw call only utilise approximately "
                                          "%.02f%% of the bound vertex buffer.",
                                          VendorSpecificTag(kBPVendorArm), utilization);
        }

        if (cache_hit_rate <= 0.5f) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing,
                                      "%s The indices which were specified for the draw call are estimated to cause thrashing of "
                                      "the post-transform vertex cache, with a hit-rate of %.02f%%. "
                                      "I.e. the ordering of the index buffer may not make optimal use of indices associated with "
                                      "recently shaded vertices.",
                                      VendorSpecificTag(kBPVendorArm), cache_hit_rate * 100.0f);
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset,
                                                        firstInstance);

    CMD_BUFFER_STATE* cmd_state = GetCBState(commandBuffer);
    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices) {
        cmd_state->small_indexed_draw_call_count++;
    }
}

void BestPractices::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                 uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    StateTracker::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    RecordCmdDrawType(commandBuffer, indexCount * instanceCount, "vkCmdDrawIndexed()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirectCountKHR()");

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_DrawCountZero,
                           "Warning: You are calling vkCmdDrawIndirect() with a drawCount of Zero.");
        skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndirect()");
    }

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t count, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawIndirect()");
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDraw_DrawCountZero,
                           "Warning: You are calling vkCmdDrawIndexedIndirect() with a drawCount of Zero.");
        skip |= ValidateCmdDrawType(commandBuffer, "vkCmdDrawIndexedIndirect()");
    }

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t count, uint32_t stride) {
    StateTracker::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
    RecordCmdDrawType(commandBuffer, count, "vkCmdDrawIndexedIndirect()");
}

bool BestPractices::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) const {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |= LogWarning(device, kVUID_BestPractices_CmdDispatch_GroupCountZero,
                           "Warning: You are calling vkCmdDispatch() while one or more groupCounts are zero (groupCountX = %" PRIu32
                           ", groupCountY = %" PRIu32 ", groupCountZ = %" PRIu32 ").",
                           groupCountX, groupCountY, groupCountZ);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = false;

    skip |= StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);

    auto prepass_state = cbDepthPrePassStates.find(commandBuffer);

    if (prepass_state == cbDepthPrePassStates.end()) return skip;

    bool uses_depth = (prepass_state->second.depthAttachment || prepass_state->second.colorAttachment) &&
                      prepass_state->second.numDrawCallsDepthEqualCompare >= kDepthPrePassNumDrawCallsArm &&
                      prepass_state->second.numDrawCallsDepthOnly >= kDepthPrePassNumDrawCallsArm;
    if (uses_depth) {
        skip |= LogPerformanceWarning(
            device, kVUID_BestPractices_EndRenderPass_DepthPrePassUsage,
            "%s Depth pre-passes may be in use. In general, this is not recommended, as in Arm Mali GPUs since "
            "Mali-T620, Forward Pixel Killing (FPK) can already perform automatic hidden surface removal; in which "
            "case, using depth pre-passes for hidden surface removal may worsen performance.",
            VendorSpecificTag(kBPVendorArm));
    }

    return skip;
}

bool BestPractices::ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice,
                                                                            const char* api_name) const {
    bool skip = false;
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);

    if (physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState == UNCALLED) {
        skip |= LogWarning(physicalDevice, kVUID_BestPractices_DisplayPlane_PropertiesNotCalled,
                           "Potential problem with calling %s() without first retrieving properties from "
                           "vkGetPhysicalDeviceDisplayPlanePropertiesKHR or vkGetPhysicalDeviceDisplayPlaneProperties2KHR.",
                           api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneSupportedDisplaysKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                  uint32_t planeIndex,
                                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilitiesKHR");

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                   VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    bool skip = false;

    skip |= ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(physicalDevice, "vkGetDisplayPlaneCapabilities2KHR");

    return skip;
}

bool BestPractices::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                         VkImage* pSwapchainImages) const {
    bool skip = false;

    auto swapchain_state = GetSwapchainState(swapchain);

    if (swapchain_state && pSwapchainImages) {
        // Compare the preliminary value of *pSwapchainImageCount with the value this time:
        if (swapchain_state->vkGetSwapchainImagesKHRState == UNCALLED) {
            skip |=
                LogWarning(device, kVUID_Core_Swapchain_PriorCount,
                           "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount; but no prior positive value has "
                           "been seen for pSwapchainImages.");
        }
    }

    return skip;
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
bool BestPractices::ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const PHYSICAL_DEVICE_STATE* pd_state,
                                                                         uint32_t requested_queue_family_property_count,
                                                                         bool qfp_null, const char* caller_name) const {
    bool skip = false;
    if (!qfp_null) {
        // Verify that for each physical device, this command is called first with NULL pQueueFamilyProperties in order to get count
        if (UNCALLED == pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState) {
            skip |= LogWarning(
                pd_state->phys_device, kVUID_Core_DevLimit_MissingQueryCount,
                "%s is called with non-NULL pQueueFamilyProperties before obtaining pQueueFamilyPropertyCount. It is recommended "
                "to first call %s with NULL pQueueFamilyProperties in order to obtain the maximal pQueueFamilyPropertyCount.",
                caller_name, caller_name);
            // Then verify that pCount that is passed in on second call matches what was returned
        } else if (pd_state->queue_family_known_count != requested_queue_family_property_count) {
            skip |= LogWarning(
                pd_state->phys_device, kVUID_Core_DevLimit_CountMismatch,
                "%s is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                ". It is recommended to instead receive all the properties by calling %s with pQueueFamilyPropertyCount that was "
                "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                caller_name, requested_queue_family_property_count, pd_state->queue_family_known_count, caller_name, caller_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const ACCELERATION_STRUCTURE_STATE* as_state = GetAccelerationStructureState(pBindInfos[i].accelerationStructure);
        if (!as_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetImageMemoryRequirements() prior to calling
            // BindAccelerationStructureMemoryNV but it's implied in that memory being bound must conform with
            // VkAccelerationStructureMemoryRequirementsInfoNV from vkGetAccelerationStructureMemoryRequirementsNV
            skip |= LogWarning(
                device, kVUID_BestPractices_BindAccelNV_NoMemReqQuery,
                "vkBindAccelerationStructureMemoryNV(): "
                "Binding memory to %s but vkGetAccelerationStructureMemoryRequirementsNV() has not been called on that structure.",
                report_data->FormatHandle(pBindInfos[i].accelerationStructure).c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties* pQueueFamilyProperties) const {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2KHR()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pSurfaceFormatCount,
                                                                      VkSurfaceFormatKHR* pSurfaceFormats) const {
    if (!pSurfaceFormats) return false;
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    const auto& call_state = physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;
    bool skip = false;
    if (call_state == UNCALLED) {
        // Since we haven't recorded a preliminary value of *pSurfaceFormatCount, that likely means that the application didn't
        // previously call this function with a NULL value of pSurfaceFormats:
        skip |= LogWarning(physicalDevice, kVUID_Core_DevLimit_MustQueryCount,
                           "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount; but no prior "
                           "positive value has been seen for pSurfaceFormats.");
    } else {
        auto prev_format_count = (uint32_t)physical_device_state->surface_formats.size();
        if (*pSurfaceFormatCount > prev_format_count) {
            skip |= LogWarning(physicalDevice, kVUID_Core_DevLimit_CountMismatch,
                               "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount, and with "
                               "pSurfaceFormats set to a value (%u) that is greater than the value (%u) that was returned "
                               "when pSurfaceFormatCount was NULL.",
                               *pSurfaceFormatCount, prev_format_count);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                   VkFence fence) const {
    bool skip = false;

    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; bindIdx++) {
        const VkBindSparseInfo& bindInfo = pBindInfo[bindIdx];
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        std::unordered_set<const IMAGE_STATE*> sparse_images;
        // Track images getting metadata bound by this call in a set, it'll be recorded into the image_state
        // in RecordQueueBindSparse.
        std::unordered_set<const IMAGE_STATE*> sparse_images_with_metadata;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bindInfo.imageBindCount; ++i) {
            const auto& image_bind = bindInfo.pImageBinds[i];
            auto image_state = GetImageState(image_bind.image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(image_state->image, kVUID_Core_MemTrack_InvalidState,
                                       "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(image_state->image, kVUID_Core_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image).c_str());
            }
        }
        for (uint32_t i = 0; i < bindInfo.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bindInfo.pImageOpaqueBinds[i];
            auto image_state = GetImageState(bindInfo.pImageOpaqueBinds[i].image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(image_state->image, kVUID_Core_MemTrack_InvalidState,
                                       "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(image_state->image, kVUID_Core_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   report_data->FormatHandle(image_state->image).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    sparse_images_with_metadata.insert(image_state);
                }
            }
        }
        for (const auto& sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound &&
                sparse_images_with_metadata.find(sparse_image_state) == sparse_images_with_metadata.end()) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                skip |= LogWarning(sparse_image_state->image, kVUID_Core_MemTrack_InvalidState,
                                   "vkQueueBindSparse(): Binding sparse memory to %s which requires a metadata aspect but no "
                                   "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                                   report_data->FormatHandle(sparse_image_state->image).c_str());
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                        VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) {
        return;
    }

    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; bindIdx++) {
        const VkBindSparseInfo& bindInfo = pBindInfo[bindIdx];
        for (uint32_t i = 0; i < bindInfo.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bindInfo.pImageOpaqueBinds[i];
            auto image_state = GetImageState(bindInfo.pImageOpaqueBinds[i].image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
    }
}

bool BestPractices::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                       const VkClearRect* pRects) const {
    bool skip = false;
    const CMD_BUFFER_STATE* cb_node = GetCBState(commandBuffer);
    if (!cb_node) return skip;

    // Warn if this is issued prior to Draw Cmd and clearing the entire attachment
    if (!cb_node->hasDrawCmd && (cb_node->activeRenderPassBeginInfo.renderArea.extent.width == pRects[0].rect.extent.width) &&
        (cb_node->activeRenderPassBeginInfo.renderArea.extent.height == pRects[0].rect.extent.height)) {
        // There are times where app needs to use ClearAttachments (generally when reusing a buffer inside of a render pass)
        // This warning should be made more specific. It'd be best to avoid triggering this test if it's a use that must call
        // CmdClearAttachments.
        skip |= LogPerformanceWarning(commandBuffer, kVUID_BestPractices_DrawState_ClearCmdBeforeDraw,
                                      "vkCmdClearAttachments() issued on %s prior to any Draw Cmds. It is recommended you "
                                      "use RenderPass LOAD_OP_CLEAR on Attachments prior to any Draw.",
                                      report_data->FormatHandle(commandBuffer).c_str());
    }

    // Check for uses of ClearAttachments along with LOAD_OP_LOAD,
    // as it can be more efficient to just use LOAD_OP_CLEAR
    const RENDER_PASS_STATE* rp = cb_node->activeRenderPass.get();
    if (rp) {
        const auto& subpass = rp->createInfo.pSubpasses[cb_node->activeSubpass];

        for (uint32_t i = 0; i < attachmentCount; i++) {
            auto& attachment = pAttachments[i];
            if (attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                uint32_t color_attachment = attachment.colorAttachment;
                uint32_t fb_attachment = subpass.pColorAttachments[color_attachment].attachment;

                if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                    if (rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                        skip |= LogPerformanceWarning(
                            device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
                            "vkCmdClearAttachments() issued on %s for color attachment #%u in this subpass, "
                            "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                            "it is more efficient.",
                            report_data->FormatHandle(commandBuffer).c_str(), color_attachment);
                    }
                }
            }

            if (subpass.pDepthStencilAttachment && attachment.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                uint32_t fb_attachment = subpass.pDepthStencilAttachment->attachment;

                if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                    if (rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                        skip |= LogPerformanceWarning(
                            device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
                            "vkCmdClearAttachments() issued on %s for the depth attachment in this subpass, "
                            "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                            "it is more efficient.",
                            report_data->FormatHandle(commandBuffer).c_str());
                    }
                }
            }

            if (subpass.pDepthStencilAttachment && attachment.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                uint32_t fb_attachment = subpass.pDepthStencilAttachment->attachment;

                if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                    if (rp->createInfo.pAttachments[fb_attachment].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                        skip |= LogPerformanceWarning(
                            device, kVUID_BestPractices_ClearAttachments_ClearAfterLoad,
                            "vkCmdClearAttachments() issued on %s for the stencil attachment in this subpass, "
                            "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                            "it is more efficient.",
                            report_data->FormatHandle(commandBuffer).c_str());
                    }
                }
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve* pRegions) const {
    bool skip = false;

    skip |= VendorCheckEnabled(kBPVendorArm) &&
            LogPerformanceWarning(device, kVUID_BestPractices_CmdResolveImage_ResolvingImage,
                                  "%s Attempting to use vkCmdResolveImage to resolve a multisampled image. "
                                  "This is a very slow and extremely bandwidth intensive path. "
                                  "You should always resolve multisampled images on-tile with pResolveAttachments in VkRenderPass.",
                                  VendorSpecificTag(kBPVendorArm));

    return skip;
}

bool BestPractices::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorArm)) {
        if ((pCreateInfo->addressModeU != pCreateInfo->addressModeV) || (pCreateInfo->addressModeV != pCreateInfo->addressModeW)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_DifferentWrappingModes,
                "%s Creating a sampler object with wrapping modes which do not match (U = %u, V = %u, W = %u). "
                "This may cause reduced performance even if only U (1D image) or U/V wrapping modes (2D "
                "image) are actually used. If you need different wrapping modes, disregard this warning.",
                VendorSpecificTag(kBPVendorArm));
        }

        if ((pCreateInfo->minLod != 0.0f) || (pCreateInfo->maxLod < VK_LOD_CLAMP_NONE)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_LodClamping,
                "%s Creating a sampler object with LOD clamping (minLod = %f, maxLod = %f). This may cause reduced performance. "
                "Instead of clamping LOD in the sampler, consider using an VkImageView which restricts the mip-levels, set minLod "
                "to 0.0, and maxLod to VK_LOD_CLAMP_NONE.",
                VendorSpecificTag(kBPVendorArm), pCreateInfo->minLod, pCreateInfo->maxLod);
        }

        if (pCreateInfo->mipLodBias != 0.0f) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_CreateSampler_LodBias,
                                      "%s Creating a sampler object with LOD bias != 0.0 (%f). This will lead to less efficient "
                                      "descriptors being created and may cause reduced performance.",
                                      VendorSpecificTag(kBPVendorArm), pCreateInfo->mipLodBias);
        }

        if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
             pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
             pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
            (pCreateInfo->borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_BorderClampColor,
                "%s Creating a sampler object with border clamping and borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK. "
                "This will lead to less efficient descriptors being created and may cause reduced performance. "
                "If possible, use VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK as the border color.",
                VendorSpecificTag(kBPVendorArm));
        }

        if (pCreateInfo->unnormalizedCoordinates) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_UnnormalizedCoordinates,
                "%s Creating a sampler object with unnormalized coordinates. This will lead to less efficient "
                "descriptors being created and may cause reduced performance.",
                VendorSpecificTag(kBPVendorArm));
        }

        if (pCreateInfo->anisotropyEnable) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_CreateSampler_Anisotropy,
                "%s Creating a sampler object with anisotropy. This will lead to less efficient descriptors being created "
                "and may cause reduced performance.",
                VendorSpecificTag(kBPVendorArm));
        }
    }

    return skip;
}

void BestPractices::PostTransformLRUCacheModel::resize(size_t size) { _entries.resize(size); }

bool BestPractices::PostTransformLRUCacheModel::query_cache(uint32_t value) {
    // look for a cache hit
    auto hit = std::find_if(_entries.begin(), _entries.end(), [value](const CacheEntry& entry) { return entry.value == value; });
    if (hit != _entries.end()) {
        // mark the cache hit as being most recently used
        hit->age = iteration++;
        return true;
    }

    // if there's no cache hit, we need to model the entry being inserted into the cache
    CacheEntry new_entry = {value, iteration};
    if (iteration < static_cast<uint32_t>(std::distance(_entries.begin(), _entries.end()))) {
        // if there is still space left in the cache, use the next available slot
        *(_entries.begin() + iteration) = new_entry;
    } else {
        // otherwise replace the least recently used cache entry
        auto lru = std::min_element(_entries.begin(), hit, [](const CacheEntry& a, const CacheEntry& b) { return a.age < b.age; });
        *lru = new_entry;
    }
    iteration++;
    return false;
}
