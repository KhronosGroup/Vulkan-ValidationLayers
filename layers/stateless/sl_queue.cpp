/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2026 Qualcomm Technologies, Inc.
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

#include "stateless/stateless_validation.h"

namespace stateless {

bool Device::manual_PreCallValidateQueueSetPerfHintQCOM(VkQueue queue, const VkPerfHintInfoQCOM *pPerfHintInfo,
                                                        const Context &context) const {
    bool skip = false;

    if (!enabled_features.queuePerfHint) {
        skip |= LogError("VUID-vkQueueSetPerfHintQCOM-queuePerfHint-12387", queue, context.error_obj.location,
                         "VkPhysicalDeviceQueuePerfHintFeaturesQCOM::queuePerfHint feature isn't enabled.");
    }

    if (pPerfHintInfo) {
        if (pPerfHintInfo->type != VK_PERF_HINT_TYPE_FREQUENCY_SCALED_QCOM && pPerfHintInfo->scale != 0) {
            const Location perf_hint_scale_loc = context.error_obj.location.pNext(Struct::VkPerfHintInfoQCOM, Field::scale);
            skip |= LogError("VUID-VkPerfHintInfoQCOM-type-12389", queue, perf_hint_scale_loc,
                             "(%" PRIu32 ") is non zero, but VkPerfHintInfoQCOM::type is %s.",
                             pPerfHintInfo->scale, string_VkPerfHintTypeQCOM(pPerfHintInfo->type));
        }
        if (pPerfHintInfo->scale > 100) {
            const Location perf_hint_scale_loc = context.error_obj.location.pNext(Struct::VkPerfHintInfoQCOM, Field::scale);
            skip |= LogError("VUID-VkPerfHintInfoQCOM-scale-12390", queue, perf_hint_scale_loc,
                             "(%" PRIu32 ") is greater than 100.",
                             pPerfHintInfo->scale);
        }
    }

    return skip;
}

}  // namespace stateless
