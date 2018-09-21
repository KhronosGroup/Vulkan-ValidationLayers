/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Tobin Ehlis <tobin@lunarg.com>
 */

#pragma once

#include <mutex>
#include <cinttypes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

#include "vk_loader_platform.h"
#include "vulkan/vulkan.h"
#include "vk_layer_config.h"
#include "vk_layer_data.h"
#include "vk_layer_logging.h"
#include "vk_object_types.h"
#include "vulkan/vk_layer.h"
#include "vk_enum_string_helper.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "vulkan/vk_layer.h"
#include "vk_dispatch_table_helper.h"
#include "vk_extension_helper.h"
#include "object_lifetimes.h"



namespace object_tracker {

struct layer_data;
struct instance_layer_data;

extern std::unordered_map<void *, layer_data *> layer_data_map;
extern std::unordered_map<void *, instance_layer_data *> instance_layer_data_map;
extern std::mutex global_lock;
extern uint32_t loader_layer_if_version;
extern const std::unordered_map<std::string, void *> name_to_funcptr_map;

struct instance_layer_data {
    object_lifetime objdata;
    VkInstance instance;

    debug_report_data *report_data;
    std::vector<VkDebugReportCallbackEXT> logging_callback;
    std::vector<VkDebugUtilsMessengerEXT> logging_messenger;
    // The following are for keeping track of the temporary callbacks that can be used in vkCreateInstance and vkDestroyInstance
    uint32_t num_tmp_report_callbacks;
    VkDebugReportCallbackCreateInfoEXT *tmp_report_create_infos;
    VkDebugReportCallbackEXT *tmp_report_callbacks;
    uint32_t num_tmp_debug_messengers;
    VkDebugUtilsMessengerCreateInfoEXT *tmp_messenger_create_infos;
    VkDebugUtilsMessengerEXT *tmp_debug_messengers;
    VkLayerInstanceDispatchTable instance_dispatch_table;

    // Default constructor
    instance_layer_data()
        : report_data(nullptr),
          num_tmp_report_callbacks(0),
          tmp_report_create_infos(nullptr),
          tmp_report_callbacks(nullptr),
          num_tmp_debug_messengers(0),
          tmp_messenger_create_infos(nullptr),
          tmp_debug_messengers(nullptr),
          instance_dispatch_table{} {}
};

struct layer_data {
    object_lifetime objdata;

    std::unordered_set<std::string> device_extension_set;
    debug_report_data *report_data;
    VkLayerDispatchTable device_dispatch_table;

    instance_layer_data *instance_data;
    VkPhysicalDevice physical_device;

    // Default constructor
    layer_data() : instance_data(nullptr), physical_device(nullptr), report_data(nullptr), device_dispatch_table{} {}
};

}  // namespace object_tracker
