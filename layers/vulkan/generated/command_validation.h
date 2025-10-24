// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See command_validation_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2025 Valve Corporation
 * Copyright (c) 2021-2025 LunarG, Inc.
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

// NOLINTBEGIN

#pragma once

#include "generated/error_location_helper.h"

enum class CommandScope { Inside, Outside, Both };

struct CommandValidationInfo {
    const char* recording_vuid;
    const char* buffer_level_vuid;

    VkQueueFlags queue_flags;
    const char* queue_vuid;

    CommandScope render_pass_scope;
    const char* render_pass_vuid;

    CommandScope video_coding_scope;
    const char* video_coding_vuid;

    const char* suspended_vuid;

    bool state;
    bool action;
    bool synchronization;
};

const CommandValidationInfo& GetCommandValidationInfo(vvl::Func command);

// NOLINTEND
