/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_format_utils.h>
#include "containers/container_utils.h"
#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

#include "utils/vk_api_utils.h"
#include "utils/math_utils.h"

namespace stateless {

bool Device::manual_PreCallValidateCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    skip |= context.ValidateStructType(error_obj.location.dot(Field::pBuildInfo).dot(Field::input), &(pBuildInfo->input),
                                       VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV, true,
                                       "VUID-VkBuildPartitionedAccelerationStructureInfoNV-input-parameter",
                                       "VUID-VkBuildPartitionedAccelerationStructureInfoNV-sType-sType");

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    const Location input_loc = info_loc.dot(Field::input);
    const Location op_input_loc = input_loc.dot(Field::opInput);
    const auto& input = pInfo->input;

    skip |= context.ValidateStructType(input_loc, &pInfo->input, VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV,
                                       true, "VUID-VkClusterAccelerationStructureCommandsInfoNV-input-parameter",
                                       "VUID-VkClusterAccelerationStructureInputInfoNV-sType-sType");

    if (input.opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV) {
        if (input.opInput.pClustersBottomLevel) {
            skip |= ValidateClusterAccelerationStructureClustersBottomLevelInputNV(context, *input.opInput.pClustersBottomLevel,
                                                                                   op_input_loc.dot(Field::pClustersBottomLevel));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pClustersBottomLevel-parameter", commandBuffer,
                             input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV, but "
                             "opInput.pClustersBottomLevel is NULL.");
        }
    }

    if (IsValueIn(input.opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                 VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        if (input.opInput.pTriangleClusters) {
            skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(context, *input.opInput.pTriangleClusters,
                                                                               op_input_loc.dot(Field::pTriangleClusters));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pTriangleClusters-parameter", commandBuffer,
                             input_loc.dot(Field::opType), "is %s, but opInput.pTriangleClusters is NULL.",
                             string_VkClusterAccelerationStructureOpTypeNV(input.opType));
        }
    }

    if (input.opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV) {
        if (input.opInput.pMoveObjects) {
            skip |= ValidateClusterAccelerationStructureMoveObjectsInputNV(context, *input.opInput.pMoveObjects,
                                                                           op_input_loc.dot(Field::pMoveObjects));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pMoveObjects-parameter", commandBuffer,
                             input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV, but opInput.pMoveObjects is NULL.");
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateGetClusterAccelerationStructureBuildSizesNV(
    VkDevice device, const VkClusterAccelerationStructureInputInfoNV* pInfo, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
    const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    const Location input_loc = error_obj.location.dot(Field::pInfo);
    const Location op_input_loc = input_loc.dot(Field::opInput);

    if (pInfo->opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV) {
        if (pInfo->opInput.pClustersBottomLevel) {
            skip |= ValidateClusterAccelerationStructureClustersBottomLevelInputNV(context, *pInfo->opInput.pClustersBottomLevel,
                                                                                   op_input_loc.dot(Field::pClustersBottomLevel));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pClustersBottomLevel-parameter", device,
                             input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV, but "
                             "opInput.pClustersBottomLevel is NULL.");
        }
    }

    if (IsValueIn(pInfo->opType, {VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV,
                                  VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_GET_CLUSTER_TEMPLATE_INDICES_NV})) {
        if (pInfo->opInput.pTriangleClusters) {
            skip |= ValidateClusterAccelerationStructureTriangleClusterInputNV(context, *pInfo->opInput.pTriangleClusters,
                                                                               op_input_loc.dot(Field::pTriangleClusters));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pTriangleClusters-parameter", device,
                             input_loc.dot(Field::opType), "is %s, but opInput.pTriangleClusters is NULL.",
                             string_VkClusterAccelerationStructureOpTypeNV(pInfo->opType));
        }
    }

    if (pInfo->opType == VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV) {
        if (pInfo->opInput.pMoveObjects) {
            skip |= ValidateClusterAccelerationStructureMoveObjectsInputNV(context, *pInfo->opInput.pMoveObjects,
                                                                           op_input_loc.dot(Field::pMoveObjects));
        } else {
            skip |= LogError("VUID-VkClusterAccelerationStructureInputInfoNV-pMoveObjects-parameter", device,
                             input_loc.dot(Field::opType),
                             "is VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV, but opInput.pMoveObjects is NULL.");
        }
    }

    return skip;
}

}  // namespace stateless