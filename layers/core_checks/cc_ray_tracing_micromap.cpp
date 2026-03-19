/* Copyright (c) 2026 The Khronos Group Inc.
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

#include "core_validation.h"

bool CoreChecks::PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyMicromapInfoEXT* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMicromapEXT-deferredOperation-03678");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMicromapToMemoryEXT-deferredOperation-03678");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMemoryToMicromapEXT-deferredOperation-03678");
    return skip;
}
