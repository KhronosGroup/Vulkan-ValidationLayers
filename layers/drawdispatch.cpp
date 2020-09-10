/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (C) 2015-2020 Google Inc.
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
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 */

#include "chassis.h"
#include "core_validation.h"

// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
// clang-format off
static const std::map<CMD_TYPE, DrawDispatchVuid> drawdispatch_vuid = {
    {CMD_DRAW, {
        "VUID-vkCmdDraw-commandBuffer-cmdpool",
        "VUID-vkCmdDraw-renderpass",
        "VUID-vkCmdDraw-None-02700",
        "VUID-vkCmdDraw-commandBuffer-02701",
        "VUID-vkCmdDraw-None-04007",
        "VUID-vkCmdDraw-None-04008",
        "VUID-vkCmdDraw-None-02697",
        "VUID-vkCmdDraw-renderPass-02684",
        "VUID-vkCmdDraw-subpass-02685",
        "VUID-vkCmdDraw-sampleLocationsEnable-02689",
        "VUID-vkCmdDraw-None-02690",
        "VUID-vkCmdDraw-None-02692",
        kVUIDUndefined, // indirect_protected_cb
        kVUIDUndefined, // indirect_contiguous_memory;
        kVUIDUndefined, // indirect_buffer_bit
        "VUID-vkCmdDraw-viewportCount-03417",
        "VUID-vkCmdDraw-scissorCount-03418",
        "VUID-vkCmdDraw-viewportCount-03419",
        "VUID-vkCmdDraw-primitiveTopology-03420",
        "VUID-vkCmdDraw-flags-02696",
        "VUID-vkCmdDraw-None-02686",
        "VUID-vkCmdDraw-None-02691",
        "VUID-vkCmdDraw-None-02687",
    }},
    {CMD_DRAWINDEXED, {
        "VUID-vkCmdDrawIndexed-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndexed-renderpass",
        "VUID-vkCmdDrawIndexed-None-02700",
        "VUID-vkCmdDrawIndexed-commandBuffer-02701",
        "VUID-vkCmdDrawIndexed-None-04007",
        "VUID-vkCmdDrawIndexed-None-04008",
        "VUID-vkCmdDrawIndexed-None-02697",
        "VUID-vkCmdDrawIndexed-renderPass-02684",
        "VUID-vkCmdDrawIndexed-subpass-02685",
        "VUID-vkCmdDrawIndexed-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndexed-None-02690",
        "VUID-vkCmdDrawIndexed-None-02692",
        kVUIDUndefined, // indirect_protected_cb
        kVUIDUndefined, // indirect_contiguous_memory;
        kVUIDUndefined, // indirect_buffer_bit
        "VUID-vkCmdDrawIndexed-viewportCount-03417",
        "VUID-vkCmdDrawIndexed-scissorCount-03418",
        "VUID-vkCmdDrawIndexed-viewportCount-03419",
        "VUID-vkCmdDrawIndexed-primitiveTopology-03420",
        "VUID-vkCmdDrawIndexed-flags-02696",
        "VUID-vkCmdDrawIndexed-None-02686",
        "VUID-vkCmdDrawIndexed-None-02691",
        "VUID-vkCmdDrawIndexed-None-02687",
    }},
    {CMD_DRAWINDIRECT, {
        "VUID-vkCmdDrawIndirect-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndirect-renderpass",
        "VUID-vkCmdDrawIndirect-None-02700",
        "VUID-vkCmdDrawIndirect-commandBuffer-02701",
        "VUID-vkCmdDrawIndirect-None-04007",
        "VUID-vkCmdDrawIndirect-None-04008",
        "VUID-vkCmdDrawIndirect-None-02697",
        "VUID-vkCmdDrawIndirect-renderPass-02684",
        "VUID-vkCmdDrawIndirect-subpass-02685",
        "VUID-vkCmdDrawIndirect-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndirect-None-02690",
        "VUID-vkCmdDrawIndirect-None-02692",
        "VUID-vkCmdDrawIndirect-commandBuffer-02711",
        "VUID-vkCmdDrawIndirect-buffer-02708",
        "VUID-vkCmdDrawIndirect-buffer-02709",
        "VUID-vkCmdDrawIndirect-viewportCount-03417",
        "VUID-vkCmdDrawIndirect-scissorCount-03418",
        "VUID-vkCmdDrawIndirect-viewportCount-03419",
        "VUID-vkCmdDrawIndirect-primitiveTopology-03420",
        "VUID-vkCmdDrawIndirect-flags-02696",
        "VUID-vkCmdDrawIndirect-None-02686",
        "VUID-vkCmdDrawIndirect-None-02691",
        "VUID-vkCmdDrawIndirect-None-02687",
    }},
    {CMD_DRAWINDEXEDINDIRECT, {
        "VUID-vkCmdDrawIndexedIndirect-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndexedIndirect-renderpass",
        "VUID-vkCmdDrawIndexedIndirect-None-02700",
        "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701",
        "VUID-vkCmdDrawIndexedIndirect-None-04007",
        "VUID-vkCmdDrawIndexedIndirect-None-04008",
        "VUID-vkCmdDrawIndexedIndirect-None-02697",
        "VUID-vkCmdDrawIndexedIndirect-renderPass-02684",
        "VUID-vkCmdDrawIndexedIndirect-subpass-02685",
        "VUID-vkCmdDrawIndexedIndirect-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndexedIndirect-None-02690",
        "VUID-vkCmdDrawIndexedIndirect-None-02692",
        "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711",
        "VUID-vkCmdDrawIndexedIndirect-buffer-02708",
        "VUID-vkCmdDrawIndexedIndirect-buffer-02709",
        "VUID-vkCmdDrawIndexedIndirect-viewportCount-03417",
        "VUID-vkCmdDrawIndexedIndirect-scissorCount-03418",
        "VUID-vkCmdDrawIndexedIndirect-viewportCount-03419",
        "VUID-vkCmdDrawIndexedIndirect-primitiveTopology-03420",
        "VUID-vkCmdDrawIndexedIndirect-flags-02696",
        "VUID-vkCmdDrawIndexedIndirect-None-02686",
        "VUID-vkCmdDrawIndexedIndirect-None-02691",
        "VUID-vkCmdDrawIndexedIndirect-None-02687",
    }},
    {CMD_DISPATCH, {
        "VUID-vkCmdDispatch-commandBuffer-cmdpool",
        "VUID-vkCmdDispatch-renderpass",
        "VUID-vkCmdDispatch-None-02700",
         kVUIDUndefined, // dynamic_state
         kVUIDUndefined, // vertex_binding
         kVUIDUndefined, // vertex_binding_null
         "VUID-vkCmdDispatch-None-02697",
         kVUIDUndefined, // render_pass_compatible
         kVUIDUndefined, // subpass_index
         kVUIDUndefined, // sample_location
         "VUID-vkCmdDispatch-None-02690",
         "VUID-vkCmdDispatch-None-02692",
         kVUIDUndefined, // indirect_protected_cb
         kVUIDUndefined, // indirect_contiguous_memory;
         kVUIDUndefined, // indirect_buffer_bit
         kVUIDUndefined, // viewport_count
         kVUIDUndefined, // scissor_count
         kVUIDUndefined, // viewport_scissor_count
         kVUIDUndefined, // primitive_topology
         "VUID-vkCmdDispatch-flags-02696",
         kVUIDUndefined, // subpass_input
         "VUID-vkCmdDispatch-None-02691",
         kVUIDUndefined, // image_subresources
    }},
    {CMD_DISPATCHINDIRECT, {
        "VUID-vkCmdDispatchIndirect-commandBuffer-cmdpool",
        "VUID-vkCmdDispatchIndirect-renderpass",
        "VUID-vkCmdDispatchIndirect-None-02700",
        kVUIDUndefined, // dynamic_state
        kVUIDUndefined, // vertex_binding
         kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdDispatchIndirect-None-02697",
        kVUIDUndefined, // render_pass_compatible
        kVUIDUndefined, // subpass_index
        kVUIDUndefined, // sample_location
        "VUID-vkCmdDispatchIndirect-None-02690",
        "VUID-vkCmdDispatchIndirect-None-02692",
        "VUID-vkCmdDispatchIndirect-commandBuffer-02711",
        "VUID-vkCmdDispatchIndirect-buffer-02708",
        "VUID-vkCmdDispatchIndirect-buffer-02709",
        kVUIDUndefined, // viewport_count
        kVUIDUndefined, // scissor_count
        kVUIDUndefined, // viewport_scissor_count
        kVUIDUndefined, // primitive_topology
        "VUID-vkCmdDispatchIndirect-flags-02696",
        kVUIDUndefined, // subpass_input
        "VUID-vkCmdDispatchIndirect-None-02691",
        kVUIDUndefined, // image_subresources
    }},
    {CMD_DRAWINDIRECTCOUNT, {
        "VUID-vkCmdDrawIndirectCount-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndirectCount-renderpass",
        "VUID-vkCmdDrawIndirectCount-None-02700",
        "VUID-vkCmdDrawIndirectCount-commandBuffer-02701",
        "VUID-vkCmdDrawIndirectCount-None-04007",
        "VUID-vkCmdDrawIndirectCount-None-04008",
        "VUID-vkCmdDrawIndirectCount-None-02697",
        "VUID-vkCmdDrawIndirectCount-renderPass-02684",
        "VUID-vkCmdDrawIndirectCount-subpass-02685",
        "VUID-vkCmdDrawIndirectCount-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndirectCount-None-02690",
        "VUID-vkCmdDrawIndirectCount-None-02692",
        "VUID-vkCmdDrawIndirectCount-commandBuffer-02711",
        "VUID-vkCmdDrawIndirectCount-buffer-02708",
        "VUID-vkCmdDrawIndirectCount-buffer-02709",
        "VUID-vkCmdDrawIndirectCount-viewportCount-03417",
        "VUID-vkCmdDrawIndirectCount-scissorCount-03418",
        "VUID-vkCmdDrawIndirectCount-viewportCount-03419",
        "VUID-vkCmdDrawIndirectCount-primitiveTopology-03420",
        "VUID-vkCmdDrawIndirectCount-flags-02696",
        "VUID-vkCmdDrawIndirectCount-None-02686",
        "VUID-vkCmdDrawIndirectCount-None-02691",
        "VUID-vkCmdDrawIndirectCount-None-02687",
    }},
    {CMD_DRAWINDEXEDINDIRECTCOUNT,{
        "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndexedIndirectCount-renderpass",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02700",
        "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02701",
        "VUID-vkCmdDrawIndexedIndirectCount-None-04007",
        "VUID-vkCmdDrawIndexedIndirectCount-None-04008",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02697",
        "VUID-vkCmdDrawIndexedIndirectCount-renderPass-02684",
        "VUID-vkCmdDrawIndexedIndirectCount-subpass-02685",
        "VUID-vkCmdDrawIndexedIndirectCount-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02690",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02692",
        "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02711",
        "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708",
        "VUID-vkCmdDrawIndexedIndirectCount-buffer-02709",
        "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03417",
        "VUID-vkCmdDrawIndexedIndirectCount-scissorCount-03418",
        "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03419",
        "VUID-vkCmdDrawIndexedIndirectCount-primitiveTopology-03420",
        "VUID-vkCmdDrawIndexedIndirectCount-flags-02696",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02686",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02691",
        "VUID-vkCmdDrawIndexedIndirectCount-None-02687",
    }},
    {CMD_TRACERAYSNV, {
        "VUID-vkCmdTraceRaysNV-commandBuffer-cmdpool",
        "VUID-vkCmdTraceRaysNV-renderpass",
        "VUID-vkCmdTraceRaysNV-None-02700",
        "VUID-vkCmdTraceRaysNV-commandBuffer-02701",
        kVUIDUndefined, // vertex_binding
        kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdTraceRaysNV-None-02697",
        kVUIDUndefined, // render_pass_compatible
        kVUIDUndefined, // subpass_index
        kVUIDUndefined, // sample_location
        "VUID-vkCmdTraceRaysNV-None-02690",
        "VUID-vkCmdTraceRaysNV-None-02692",
        kVUIDUndefined, // indirect_protected_cb
        kVUIDUndefined, // indirect_contiguous_memory;
        kVUIDUndefined, // indirect_buffer_bit
        kVUIDUndefined, // viewport_count
        kVUIDUndefined, // scissor_count
        kVUIDUndefined, // viewport_scissor_count
        kVUIDUndefined, // primitive_topology
        "VUID-vkCmdTraceRaysNV-flags-02696",
        kVUIDUndefined, // subpass_input
        "VUID-vkCmdTraceRaysNV-None-02691",
         kVUIDUndefined, // image_subresources
    }},
    {CMD_TRACERAYSKHR, {
        "VUID-vkCmdTraceRaysKHR-commandBuffer-cmdpool",
        "VUID-vkCmdTraceRaysKHR-renderpass",
        "VUID-vkCmdTraceRaysKHR-None-02700",
        "VUID-vkCmdTraceRaysKHR-commandBuffer-02701",
        kVUIDUndefined, // vertex_binding
        kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdTraceRaysKHR-None-02697",
        kVUIDUndefined, // render_pass_compatible
        kVUIDUndefined, // subpass_index
        kVUIDUndefined, // sample_location
        "VUID-vkCmdTraceRaysKHR-None-02690",
        "VUID-vkCmdTraceRaysKHR-None-02692",
        kVUIDUndefined, // indirect_protected_cb
        kVUIDUndefined, // indirect_contiguous_memory;
        kVUIDUndefined, // indirect_buffer_bit
        kVUIDUndefined, // viewport_count
        kVUIDUndefined, // scissor_count
        kVUIDUndefined, // viewport_scissor_count
        kVUIDUndefined, // primitive_topology
        "VUID-vkCmdTraceRaysKHR-flags-02696",
        kVUIDUndefined, // subpass_input
        "VUID-vkCmdTraceRaysKHR-None-02691",
        kVUIDUndefined, // image_subresources
    }},
    {CMD_TRACERAYSINDIRECTKHR, {
        "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-cmdpool",
        "VUID-vkCmdTraceRaysIndirectKHR-renderpass",
        "VUID-vkCmdTraceRaysIndirectKHR-None-02700",
        "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02701",
        kVUIDUndefined, // vertex_binding
        kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdTraceRaysIndirectKHR-None-02697",
        kVUIDUndefined, // render_pass_compatible
        kVUIDUndefined, // subpass_index
        kVUIDUndefined, // sample_location
        "VUID-vkCmdTraceRaysIndirectKHR-None-02690",
        "VUID-vkCmdTraceRaysIndirectKHR-None-02692",
        "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02711",
        "VUID-vkCmdTraceRaysIndirectKHR-buffer-02708",
        "VUID-vkCmdTraceRaysIndirectKHR-buffer-02709",
        kVUIDUndefined, // viewport_count
        kVUIDUndefined, // scissor_count
        kVUIDUndefined, // viewport_scissor_count
        kVUIDUndefined, // primitive_topology
        "VUID-vkCmdTraceRaysIndirectKHR-flags-02696",
        kVUIDUndefined, // subpass_input
        "VUID-vkCmdTraceRaysIndirectKHR-None-02691",
        kVUIDUndefined, // image_subresources
    }},
    {CMD_DRAWMESHTASKSNV, {
        "VUID-vkCmdDrawMeshTasksNV-commandBuffer-cmdpool",
        "VUID-vkCmdDrawMeshTasksNV-renderpass",
        "VUID-vkCmdDrawMeshTasksNV-None-02700",
        "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02701",
        kVUIDUndefined, // vertex_binding
        kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdDrawMeshTasksNV-None-02697",
        "VUID-vkCmdDrawMeshTasksNV-renderPass-02684",
        "VUID-vkCmdDrawMeshTasksNV-subpass-02685",
        "VUID-vkCmdDrawMeshTasksNV-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawMeshTasksNV-None-02690",
        "VUID-vkCmdDrawMeshTasksNV-None-02692",
        kVUIDUndefined, // indirect_protected_cb
        kVUIDUndefined, // indirect_contiguous_memory;
        kVUIDUndefined, // indirect_buffer_bit
        "VUID-vkCmdDrawMeshTasksNV-viewportCount-03417",
        "VUID-vkCmdDrawMeshTasksNV-scissorCount-03418",
        "VUID-vkCmdDrawMeshTasksNV-viewportCount-03419",
        "VUID-vkCmdDrawMeshTasksNV-primitiveTopology-03420",
        "VUID-vkCmdDrawMeshTasksNV-flags-02696",
        "VUID-vkCmdDrawMeshTasksNV-None-02686",
        "VUID-vkCmdDrawMeshTasksNV-None-02691",
        "VUID-vkCmdDrawMeshTasksNV-None-02687",
    }},
    {CMD_DRAWMESHTASKSINDIRECTNV, {
        "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-cmdpool",
        "VUID-vkCmdDrawMeshTasksIndirectNV-renderpass",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02700",
        "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02701",
         kVUIDUndefined, // vertex_binding
         kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02697",
        "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-02684",
        "VUID-vkCmdDrawMeshTasksIndirectNV-subpass-02685",
        "VUID-vkCmdDrawMeshTasksIndirectNV-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02690",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02692",
        "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02711",
        "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708",
        "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709",
        "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03417",
        "VUID-vkCmdDrawMeshTasksIndirectNV-scissorCount-03418",
        "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03419",
        "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveTopology-03420",
        "VUID-vkCmdDrawMeshTasksIndirectNV-flags-02696",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02686",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02691",
        "VUID-vkCmdDrawMeshTasksIndirectNV-None-02687",
    }},
    {CMD_DRAWMESHTASKSINDIRECTCOUNTNV, {
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-cmdpool",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderpass",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02700",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02701",
        kVUIDUndefined, // vertex_binding
        kVUIDUndefined, // vertex_binding_null
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02697",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-02684",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-subpass-02685",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02690",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02692",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02711",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03417",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-scissorCount-03418",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03419",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveTopology-03420",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-flags-02696",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02686",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02691",
        "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02687",
    }},
    {CMD_DRAWINDIRECTBYTECOUNTEXT, {
        "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-cmdpool",
        "VUID-vkCmdDrawIndirectByteCountEXT-renderpass",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02700",
        "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02701",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-04007",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-04008",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02697",
        "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-02684",
        "VUID-vkCmdDrawIndirectByteCountEXT-subpass-02685",
        "VUID-vkCmdDrawIndirectByteCountEXT-sampleLocationsEnable-02689",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02690",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02692",
        "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02711",
        "VUID-vkCmdDrawIndirectByteCountEXT-buffer-02708",
        "VUID-vkCmdDrawIndirectByteCountEXT-buffer-02709",
        "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03417",
        "VUID-vkCmdDrawIndirectByteCountEXT-scissorCount-03418",
        "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03419",
        "VUID-vkCmdDrawIndirectByteCountEXT-primitiveTopology-03420",
        "VUID-vkCmdDrawIndirectByteCountEXT-flags-02696",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02686",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02691",
        "VUID-vkCmdDrawIndirectByteCountEXT-None-02687",
    }},
    {CMD_DISPATCHBASE, {
        "VUID-vkCmdDispatchBase-commandBuffer-cmdpool",
        "VUID-vkCmdDispatchBase-renderpass",
        "VUID-vkCmdDispatchBase-None-02700",
         kVUIDUndefined, // dynamic_state
         kVUIDUndefined, // vertex_binding
         kVUIDUndefined, // vertex_binding_null
         "VUID-vkCmdDispatchBase-None-02697",
         kVUIDUndefined, // render_pass_compatible
         kVUIDUndefined, // subpass_index
         kVUIDUndefined, // sample_location
         "VUID-vkCmdDispatchBase-None-02690",
         "VUID-vkCmdDispatchBase-None-02692",
         kVUIDUndefined, // indirect_protected_cb
         kVUIDUndefined, // indirect_contiguous_memory;
         kVUIDUndefined, // indirect_buffer_bit
         kVUIDUndefined, // viewport_count
         kVUIDUndefined, // scissor_count
         kVUIDUndefined, // viewport_scissor_count
         kVUIDUndefined, // primitive_topology
         "VUID-vkCmdDispatchBase-flags-02696",
         kVUIDUndefined, // subpass_input
         "VUID-vkCmdDispatchBase-None-02691",
         kVUIDUndefined, // image_subresources
    }},
    // Used if invalid cmd_type is used
    {CMD_NONE, {
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
        kVUIDUndefined,
    }}
};
// clang-format on

// Getter function to provide kVUIDUndefined in case an invalid cmd_type is passed in
const DrawDispatchVuid &CoreChecks::GetDrawDispatchVuid(CMD_TYPE cmd_type) const {
    if (drawdispatch_vuid.find(cmd_type) != drawdispatch_vuid.cend()) {
        return drawdispatch_vuid.at(cmd_type);
    } else {
        return drawdispatch_vuid.at(CMD_NONE);
    }
}

// Generic function to handle validation for all CmdDraw* type functions
bool CoreChecks::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                                     const char *caller, VkQueueFlags queue_flags) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const CMD_BUFFER_STATE *cb_state = GetCBState(cmd_buffer);
    if (cb_state) {
        skip |= ValidateCmdQueueFlags(cb_state, caller, queue_flags, vuid.queue_flag);
        skip |= ValidateCmd(cb_state, cmd_type, caller);
        skip |= ValidateCmdBufDrawState(cb_state, cmd_type, indexed, bind_point, caller);
        skip |= (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) ? OutsideRenderPass(cb_state, caller, vuid.inside_renderpass)
                                                                : InsideRenderPass(cb_state, caller, vuid.inside_renderpass);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) const {
    return ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW, "vkCmdDraw()",
                               VK_QUEUE_GRAPHICS_BIT);
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED, "vkCmdDrawIndexed()",
                                    VK_QUEUE_GRAPHICS_BIT);
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (!skip && (cb_state->status & CBSTATUS_INDEX_BUFFER_BOUND)) {
        unsigned int index_size = 0;
        const auto &index_buffer_binding = cb_state->index_buffer_binding;
        if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT16) {
            index_size = 2;
        } else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT32) {
            index_size = 4;
        } else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT8_EXT) {
            index_size = 1;
        }
        VkDeviceSize end_offset = (index_size * ((VkDeviceSize)firstIndex + indexCount)) + index_buffer_binding.offset;
        if (end_offset > index_buffer_binding.size) {
            skip |=
                LogError(index_buffer_binding.buffer, "VUID-vkCmdDrawIndexed-indexSize-00463",
                         "vkCmdDrawIndexed() index size (%d) * (firstIndex (%d) + indexCount (%d)) "
                         "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                         " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                         index_size, firstIndex, indexCount, index_buffer_binding.offset, end_offset, index_buffer_binding.size);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                                uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()",
                                    VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()");
    if (count > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
        skip |=
            ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                            "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), count, offset, buffer_state);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t count, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT,
                                    "vkCmdDrawIndexedIndirect()", VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDEXEDINDIRECT, "vkCmdDrawIndexedIndirect()");
    if (count > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), count, offset,
                                                buffer_state);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH, "vkCmdDispatch()",
                                VK_QUEUE_COMPUTE_BIT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ) const {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE, "vkCmdDispatchBase()",
                                VK_QUEUE_COMPUTE_BIT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) const {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE, "vkCmdDispatchBaseKHR()",
                                VK_QUEUE_COMPUTE_BIT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT,
                                    "vkCmdDispatchIndirect()", VK_QUEUE_COMPUTE_BIT);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DISPATCHINDIRECT, "vkCmdDispatchIndirect()");
    return skip;
}
bool CoreChecks::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const char *apiName) const {
    bool skip = false;
    if ((api_version >= VK_API_VERSION_1_2) && (enabled_features.core12.drawIndirectCount == VK_FALSE)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-None-04445",
                         "%s(): Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.",
                         apiName);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirectCount-stride-03110", stride, apiName,
                                            sizeof(VkDrawIndirectCommand));
    if (maxDrawCount > 1) {
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride, apiName,
                                                sizeof(VkDrawIndirectCommand), maxDrawCount, offset, buffer_state);
    }

    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTCOUNT, apiName,
                                VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDIRECTCOUNT, apiName);
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, apiName, "VUID-vkCmdDrawIndirectCount-countBuffer-02714");
    skip |=
        ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                 "VUID-vkCmdDrawIndirectCount-countBuffer-02715", apiName, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCountKHR");
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCount");
}

bool CoreChecks::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const char *apiName) const {
    bool skip = false;
    if ((api_version >= VK_API_VERSION_1_2) && (enabled_features.core12.drawIndirectCount == VK_FALSE)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-None-04445",
                         "%s(): Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.",
                         apiName);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride, apiName,
                                            sizeof(VkDrawIndexedIndirectCommand));
    if (maxDrawCount > 1) {
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                apiName, sizeof(VkDrawIndexedIndirectCommand), maxDrawCount, offset, buffer_state);
    }

    skip |= ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECTCOUNT, apiName,
                                VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDEXEDINDIRECTCOUNT, apiName);
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, apiName, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715", apiName,
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCountKHR");
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCount");
}

bool CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride) const {
    return ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTBYTECOUNTEXT,
                               "vkCmdDrawIndirectByteCountEXT()", VK_QUEUE_GRAPHICS_BIT);
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, CMD_TRACERAYSNV,
                                    "vkCmdTraceRaysNV()", VK_QUEUE_COMPUTE_BIT);
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    skip |= InsideRenderPass(cb_state, "vkCmdTraceRaysNV()", "VUID-vkCmdTraceRaysNV-renderpass");
    auto callable_shader_buffer_state = (BUFFER_STATE *)GetBufferState(callableShaderBindingTableBuffer);
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset %" PRIu64
                         " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->createInfo.size);
    }
    auto hit_shader_buffer_state = (BUFFER_STATE *)GetBufferState(hitShaderBindingTableBuffer);
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset %" PRIu64
                         " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->createInfo.size);
    }
    auto miss_shader_buffer_state = (BUFFER_STATE *)GetBufferState(missShaderBindingTableBuffer);
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457",
                         "vkCmdTraceRaysNV: missShaderBindingOffset %" PRIu64
                         " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->createInfo.size);
    }
    auto raygen_shader_buffer_state = (BUFFER_STATE *)GetBufferState(raygenShaderBindingTableBuffer);
    if (raygenShaderBindingOffset >= raygen_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02455",
                         "vkCmdTraceRaysNV: raygenShaderBindingOffset %" PRIu64
                         " must be less than the size of raygenShaderBindingTableBuffer %" PRIu64 " .",
                         raygenShaderBindingOffset, raygen_shader_buffer_state->createInfo.size);
    }
    return skip;
}

void CoreChecks::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, CMD_TRACERAYSNV, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, "vkCmdTraceRaysNV()");
    cb_state->hasTraceRaysCmd = true;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSKHR,
                                    "vkCmdTraceRaysKHR()", VK_QUEUE_COMPUTE_BIT);
    return skip;
}

void CoreChecks::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    UpdateStateCmdDrawDispatchType(cb_state, CMD_TRACERAYSKHR, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, "vkCmdTraceRaysKHR()");
    cb_state->hasTraceRaysCmd = true;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pCallableShaderBindingTable,
                                                        VkBuffer buffer, VkDeviceSize offset) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSINDIRECTKHR,
                                    "vkCmdTraceRaysIndirectKHR()", VK_QUEUE_COMPUTE_BIT);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_TRACERAYSINDIRECTKHR, "vkCmdTraceRaysIndirectKHR()");
    return skip;
}

void CoreChecks::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pCallableShaderBindingTable, VkBuffer buffer,
                                                       VkDeviceSize offset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    UpdateStateCmdDrawDispatchType(cb_state, CMD_TRACERAYSINDIRECTKHR, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                   "vkCmdTraceRaysIndirectKHR()");
    cb_state->hasTraceRaysCmd = true;
    AddCommandBufferBindingBuffer(cb_state, buffer_state);
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV,
                                    "vkCmdDrawMeshTasksNV()", VK_QUEUE_GRAPHICS_BIT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV,
                                    "vkCmdDrawMeshTasksIndirectNV()", VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWMESHTASKSINDIRECTNV, "vkCmdDrawMeshTasksIndirectNV()");
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, buffer_state);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTCOUNTNV,
                                    "vkCmdDrawMeshTasksIndirectCountNV()", VK_QUEUE_GRAPHICS_BIT);
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWMESHTASKSINDIRECTCOUNTNV, "vkCmdDrawMeshTasksIndirectCountNV()");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715",
                                     "vkCmdDrawMeshTasksIndirectCountNV()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV));
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, buffer_state);
    }
    return skip;
}
