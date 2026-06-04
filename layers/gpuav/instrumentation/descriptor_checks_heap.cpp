/* Copyright (c) 2026 LunarG, Inc.
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
#include <cstdint>
#include "containers/limits.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "state_tracker/shader_module.h"
#include "utils/descriptor_utils.h"

namespace gpuav {

void RegisterDescriptorChecksHeapValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }
    if (!gpuav.enabled_features.descriptorHeap) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [&cb](Validator& gpuav, const Location& loc, const uint32_t* error_record, const InstrumentedShader*,
                  std::string& out_error_msg, std::string& out_vuid_msg) {
                using namespace glsl;

                bool error_found = false;
                if (GetErrorGroup(error_record) != kErrorGroup_InstDescriptorHeap) {
                    return error_found;
                }
                error_found = true;

                std::ostringstream strm;

                const uint32_t error_sub_code = GetSubError(error_record);
                switch (error_sub_code) {
                    case kErrorSubCode_DescriptorHeap_HeapOOB: {
                        // TODO - Reverse info to get good error message
                        const uint32_t index = error_record[kInst_LogError_ParameterOffset_0];
                        const uint32_t offset = error_record[kInst_LogError_ParameterOffset_1];
                        // TODO - Is this cb.heap data going to change if the heap is rebound? (do we need to save?)
                        strm << "Index " << index << " is accessing an offset of " << offset
                             << " which is OOB of the heap bound at " << string_range_hex(cb.base.descriptor_heap.resource_range)
                             << " (size of " << cb.base.descriptor_heap.resource_range.size() << " bytes)";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;
                }
                out_error_msg += strm.str();
                return error_found;
            };

        return inst_error_logger;
    });

    cb.on_instrumentation_common_desc_update_functions.emplace_back([&gpuav](CommandBufferSubState& cb, const LastBound& last_bound,
                                                                             const Location&,
                                                                             CommonDescriptorUpdate& out_update) mutable {
        const uint32_t bound_heap_info_size = sizeof(glsl::BoundHeapInfo) * 2;
        uint32_t buffer_size = bound_heap_info_size + (uint32_t)gpuav.phys_dev_ext_props.descriptor_heap_props.maxPushDataSize;
        vko::BufferRange buffer_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        glsl::BoundHeapInfo* bound_heap_info = (glsl::BoundHeapInfo*)buffer_range.offset_mapped_ptr;
        glsl::BoundHeapInfo& bound_resource_heap = bound_heap_info[0];
        glsl::BoundHeapInfo& bound_sampler_heap = bound_heap_info[1];
        auto& cb_heap = cb.base.descriptor_heap;
        assert(cb_heap.resource_range.size() < (VkDeviceAddress)vvl::kU32Max);
        assert(cb_heap.sampler_range.size() < (VkDeviceAddress)vvl::kU32Max);
        bound_resource_heap.heap_size = (uint32_t)cb_heap.resource_range.size();
        bound_resource_heap.reserved_begin = (uint32_t)(cb_heap.resource_reserved.begin - cb_heap.resource_range.begin);
        bound_resource_heap.reserved_end = (uint32_t)(cb_heap.resource_reserved.end - cb_heap.resource_range.begin);
        bound_sampler_heap.heap_size = (uint32_t)cb_heap.sampler_range.size();
        bound_sampler_heap.reserved_begin = (uint32_t)(cb_heap.sampler_reserved.begin - cb_heap.sampler_reserved.begin);
        bound_sampler_heap.reserved_end = (uint32_t)(cb_heap.sampler_reserved.end - cb_heap.sampler_reserved.begin);

        uint8_t* gpu_push_data_ptr = ((uint8_t*)buffer_range.offset_mapped_ptr) + bound_heap_info_size;
        memcpy(gpu_push_data_ptr, cb.push_data_value.data(), cb.push_data_value.size());

        out_update.buffer = buffer_range.buffer;
        out_update.offset = buffer_range.offset;
        out_update.range = buffer_range.size;
        out_update.address = buffer_range.offset_address;
        out_update.binding = glsl::kBindingInstDescriptorHeap;
    });
}

}  // namespace gpuav
