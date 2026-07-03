// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See generate_spec_error_message.py for modifications
// Based on Vulkan specification version: 1.4.356

/***************************************************************************
 *
 * Copyright (c) 2016-2026 Google Inc.
 * Copyright (c) 2016-2026 LunarG, Inc.
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
 ****************************************************************************/
#pragma once

#include "containers/custom_containers.h"
#include <string_view>

// Mapping from VUID string to the corresponding spec text
struct vuid_info {
    const std::string_view spec_text;
    const std::string_view url_id;
};

const vvl::unordered_map<std::string_view, vuid_info>& GetVuidMap();
