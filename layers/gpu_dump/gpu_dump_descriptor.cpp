/* Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <iostream>
#include "error_message/log_message_type.h"

namespace gpudump {

void CommandBufferSubState::DumpDescriptors(const LastBound& last_bound, const Location& loc) const {
    vvl::DescriptorMode descriptor_mode = last_bound.GetActionDescriptorMode();
    if (descriptor_mode != vvl::DescriptorModeBuffer && descriptor_mode != vvl::DescriptorModeHeap) {
        return;
    }
    std::ostringstream ss;
    ss << "[Dump Descriptor] (";
    // Embedded the objects into the message at the top instead of providing them in the callback we normally do
    const LogObjectList objlist = last_bound.cb_state.GetObjectList(last_bound.bind_point);
    bool first_obj = true;
    for (auto object : objlist) {
        if (first_obj) {
            first_obj = false;
        } else {
            ss << ", ";
        }
        ss << dev_data.FormatHandle(object);
    }
    ss << ")\n";

    bool found_warning = false;
    if (descriptor_mode == vvl::DescriptorModeBuffer) {
        found_warning = DumpDescriptorBuffer(ss, last_bound);
    } else if (descriptor_mode == vvl::DescriptorModeHeap) {
        found_warning = DumpDescriptorHeap(ss, last_bound);
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        const VkFlags log_level = found_warning ? kWarningBit : kInformationBit;
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(log_level, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
