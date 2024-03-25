/* Copyright (c) 2015-2016, 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2024 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2024 LunarG, Inc.
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

#include "vk_layer_utils.h"

#include <string.h>
#include <sys/stat.h>

#include "containers/range_vector.h"
#include "vulkan/vulkan.h"

// Debug callbacks get created in three ways:
//   o  Application-defined debug callbacks
//   o  Through settings in a vk_layer_settings.txt file
//   o  By default, if neither an app-defined debug callback nor a vk_layer_settings.txt file is present
//
// At layer initialization time, default logging callbacks are created to output layer error messages.
// If a vk_layer_settings.txt file is present its settings will override any default settings.
//
// If a vk_layer_settings.txt file is present and an application defines a debug callback, both callbacks
// will be active.  If no vk_layer_settings.txt file is present, creating an application-defined debug
// callback will cause the default callbacks to be unregisterd and removed.
void LayerDebugMessengerActions(DebugReport *debug_report, const char *layer_identifier) {
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

    std::string report_flags_key = layer_identifier;
    std::string debug_action_key = layer_identifier;
    std::string log_filename_key = layer_identifier;
    report_flags_key.append(".report_flags");
    debug_action_key.append(".debug_action");
    log_filename_key.append(".log_filename");

    const vvl::unordered_map<std::string, VkFlags> debug_actions_option_definitions = {
        {std::string("VK_DBG_LAYER_ACTION_IGNORE"), VK_DBG_LAYER_ACTION_IGNORE},
        {std::string("VK_DBG_LAYER_ACTION_CALLBACK"), VK_DBG_LAYER_ACTION_CALLBACK},
        {std::string("VK_DBG_LAYER_ACTION_LOG_MSG"), VK_DBG_LAYER_ACTION_LOG_MSG},
        {std::string("VK_DBG_LAYER_ACTION_BREAK"), VK_DBG_LAYER_ACTION_BREAK},
        {std::string("VK_DBG_LAYER_ACTION_DEBUG_OUTPUT"), VK_DBG_LAYER_ACTION_DEBUG_OUTPUT},
        {std::string("VK_DBG_LAYER_ACTION_DEFAULT"), VK_DBG_LAYER_ACTION_DEFAULT}};

    const vvl::unordered_map<std::string, VkFlags> log_msg_type_option_definitions = {{std::string("warn"), kWarningBit},
                                                                                      {std::string("info"), kInformationBit},
                                                                                      {std::string("perf"), kPerformanceWarningBit},
                                                                                      {std::string("error"), kErrorBit},
                                                                                      {std::string("verbose"), kVerboseBit}};

    // Initialize layer options
    LogMessageTypeFlags report_flags = GetLayerOptionFlags(report_flags_key, log_msg_type_option_definitions, 0);
    VkLayerDbgActionFlags debug_action = GetLayerOptionFlags(debug_action_key, debug_actions_option_definitions, 0);
    // Flag as default if these settings are not from a vk_layer_settings.txt file
    const bool default_layer_callback = (debug_action & VK_DBG_LAYER_ACTION_DEFAULT) != 0;

    VkDebugUtilsMessengerCreateInfoEXT dbg_create_info = vku::InitStructHelper();
    dbg_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    if (report_flags & kErrorBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    if (report_flags & kWarningBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    }
    if (report_flags & kPerformanceWarningBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        dbg_create_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    }
    if (report_flags & kInformationBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    if (report_flags & kVerboseBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    }

    if (debug_action & VK_DBG_LAYER_ACTION_LOG_MSG) {
        const char *log_filename = getLayerOption(log_filename_key.c_str());
        FILE *log_output = getLayerLogOutput(log_filename, layer_identifier);
        dbg_create_info.pfnUserCallback = MessengerLogCallback;
        dbg_create_info.pUserData = (void *)log_output;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    }

    messenger = VK_NULL_HANDLE;

    if (debug_action & VK_DBG_LAYER_ACTION_DEBUG_OUTPUT) {
        dbg_create_info.pfnUserCallback = MessengerWin32DebugOutputMsg;
        dbg_create_info.pUserData = NULL;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    }

    messenger = VK_NULL_HANDLE;

    if (debug_action & VK_DBG_LAYER_ACTION_BREAK) {
        dbg_create_info.pfnUserCallback = MessengerBreakCallback;
        dbg_create_info.pUserData = NULL;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    }
}

VkLayerInstanceCreateInfo *GetChainInfo(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerInstanceCreateInfo *chain_info = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerInstanceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

VkLayerDeviceCreateInfo *GetChainInfo(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerDeviceCreateInfo *chain_info = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerDeviceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

std::string GetTempFilePath() {
    auto tmp_path = GetEnvironment("XDG_CACHE_HOME");
    if (!tmp_path.size()) {
        auto cachepath = GetEnvironment("HOME") + "/.cache";
        struct stat info;
        if (stat(cachepath.c_str(), &info) == 0) {
            if ((info.st_mode & S_IFMT) == S_IFDIR) {
                tmp_path = cachepath;
            }
        }
    }
    if (!tmp_path.size()) tmp_path = GetEnvironment("TMPDIR");
    if (!tmp_path.size()) tmp_path = GetEnvironment("TMP");
    if (!tmp_path.size()) tmp_path = GetEnvironment("TEMP");
    if (!tmp_path.size()) tmp_path = "/tmp";
    return tmp_path;
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask, const uint32_t mip_level) {
    // Return zero extent if mip level doesn't exist
    if (mip_level >= ci.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    VkExtent3D extent = ci.extent;

    // If multi-plane, adjust per-plane extent
    const VkFormat format = ci.format;
    if (vkuFormatIsMultiplane(format)) {
        VkExtent2D divisors = vkuFindMultiplaneExtentDivisors(format, static_cast<VkImageAspectFlagBits>(aspect_mask));
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    // Mip Maps
    {
        const uint32_t corner = (ci.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) ? 1 : 0;
        const uint32_t min_size = 1 + corner;
        const std::array dimensions = {&extent.width, &extent.height, &extent.depth};
        for (uint32_t *dim : dimensions) {
            // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
            if (*dim == 0) {
                continue;
            }
            *dim >>= mip_level;
            *dim = std::max(min_size, *dim);
        }
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != ci.imageType) {
        extent.depth = ci.arrayLayers;
    }

    return extent;
}

// Returns true if [x, x + x_size) and [y, y + y_size) overlap
bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    auto intersection = GetRangeIntersection(x, x_size, y, y_size);
    return intersection.non_empty();
}