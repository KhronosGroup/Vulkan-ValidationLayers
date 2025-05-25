/* Copyright (c) 2015-2016, 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2025 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2025 LunarG, Inc.
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
#include <vulkan/vk_enum_string_helper.h>

#include "vulkan/vulkan_core.h"
#include "vk_layer_config.h"

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
