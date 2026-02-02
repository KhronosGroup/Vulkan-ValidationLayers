/* Copyright (c) 2026 The Khronos Group Inc.
* Copyright (C) 2026 Qualcomm Limited.
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

bool Device::manual_PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                     const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (const auto* processing_engine_info =
        vku::FindStructInPNextChain<VkDataGraphProcessingEngineCreateInfoARM>(pCreateInfo->pNext)) {
        const Location processing_engine_ci_loc =
                            error_obj.location.dot(Field::pCreateInfo).pNext(Struct::VkDataGraphProcessingEngineCreateInfoARM);
        skip |= ValidateDataGraphProcessingEngineCreateInfoARM(*processing_engine_info, processing_engine_ci_loc);
    }

    return skip;
}

}  // namespace stateless
