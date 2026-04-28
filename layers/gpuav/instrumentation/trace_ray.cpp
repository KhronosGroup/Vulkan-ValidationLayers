/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include "generated/spirv_grammar_helper.h"
#include "generated/gpuav_offline_spirv.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/setup/acceleration_structure_gpu_state_update.h"
#include "state_tracker/descriptor_sets.h"

namespace gpuav {

void RegisterTraceRayValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.trace_ray) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, const LastBound& last_bound) {
            uint32_t descriptor_binding_index = vvl::kNoIndex32;
            DescriptorSetBindings* desc_set_bindings = cb.shared_resources_cache.TryGet<DescriptorSetBindings>();
            if (desc_set_bindings && !desc_set_bindings->descriptor_set_binding_commands.empty()) {
                descriptor_binding_index = uint32_t(desc_set_bindings->descriptor_set_binding_commands.size() - 1);
            }

            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [&cb, descriptor_binding_index](Validator& gpuav, const Location& loc, const uint32_t* error_record,
                                                const InstrumentedShader*, std::string& out_error_msg, std::string& out_vuid_msg) {
                    using namespace glsl;
                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroup_TraceRay) {
                        return error_found;
                    }
                    error_found = true;

                    std::ostringstream strm;

                    const uint32_t error_sub_code = GetSubError(error_record);
                    const uint32_t opcode = error_record[kInst_LogError_ParameterOffset_2];

                    switch (error_sub_code) {
                        case kErrorSubCode_RayHitObject_NegativeMin: {
                            // Should use std::bit_cast but requires c++20
                            const float tmin = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            strm << string_SpvOpcode(opcode) << " operand Ray Tmin value (" << tmin << ") is negative. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879";
                        } break;
                        case kErrorSubCode_RayHitObject_NegativeMax: {
                            const float tmax = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            strm << string_SpvOpcode(opcode) << " operand Ray Tmax value (" << tmax << ") is negative. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879";
                        } break;
                        case kErrorSubCode_RayHitObject_MinMax: {
                            const float tmin = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float tmax = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            strm << string_SpvOpcode(opcode) << " operand Ray Tmax (" << tmax << ") is less than Ray Tmin (" << tmin
                                 << "). ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11880";
                        } break;
                        case kErrorSubCode_RayHitObject_MinNaN: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Tmin is NaN. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                        } break;
                        case kErrorSubCode_RayHitObject_MaxNaN: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Tmax is NaN. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                        } break;
                        case kErrorSubCode_RayHitObject_OriginNaN: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Origin contains a NaN. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                        } break;
                        case kErrorSubCode_RayHitObject_DirectionNaN: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Direction contains a NaN. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881";
                        } break;
                        case kErrorSubCode_RayHitObject_OriginFinite: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Origin contains a non-finite value. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11878";
                        } break;
                        case kErrorSubCode_RayHitObject_DirectionFinite: {
                            strm << string_SpvOpcode(opcode) << " operand Ray Direction contains a non-finite value. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11878";
                        } break;
                        case kErrorSubCode_RayHitObject_BothSkip: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11883";
                        } break;
                        case kErrorSubCode_RayHitObject_SkipCull: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11884";
                        } break;
                        case kErrorSubCode_RayHitObject_Opaque: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags is 0x" << std::hex << value << ". ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11885";
                        } break;
                        case kErrorSubCode_RayHitObject_SkipTrianglesWithPipelineSkipAABBs: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                                 << ") contains SkipTrianglesKHR, but pipeline was created with "
                                    "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11886";
                        } break;
                        case kErrorSubCode_RayHitObject_SkipAABBsWithPipelineSkipTriangles: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                                 << ") contains SkipAABBsKHR, but pipeline was created with "
                                    "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11887";
                        } break;
                        case kErrorSubCode_RayHitObject_TimeOutOfRange: {
                            strm << string_SpvOpcode(opcode) << " operand time is not between 0.0 and 1.0. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11882";
                        } break;
                        case kErrorSubCode_RayHitObject_SBTIndexExceedsLimit: {
                            // For this case, param_0 contains the SBT index and opcode_type slot contains the max SBT index
                            const uint32_t sbt_index = error_record[kInst_LogError_ParameterOffset_0];
                            const uint32_t max_sbt_index = error_record[kInst_LogError_ParameterOffset_1];
                            strm << "OpHitObjectSetShaderBindingTableRecordIndexEXT SBT index (" << std::dec << sbt_index
                                 << ") exceeds "
                                    "VkPhysicalDeviceRayTracingInvocationReorderPropertiesEXT::maxShaderBindingTableRecordIndex ("
                                 << max_sbt_index << "). ";
                            out_vuid_msg = "VUID-RuntimeSpirv-maxShaderBindingTableRecordIndex-11888";
                        } break;
                        case kErrorSubCode_TraceRay_TrianglesFlags: {
                            const uint32_t ray_flags = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "OpTraceRayKHR operand Ray Flags (" << string_SpvRayFlagsMask(ray_flags)
                                 << ") form an invalid combination of mutually exclusive flags.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06892";
                        } break;
                        case kErrorSubCode_TraceRay_OpaqueFlags: {
                            const uint32_t ray_flags = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "OpTraceRayKHR operand Ray Flags (" << string_SpvRayFlagsMask(ray_flags)
                                 << ") form an invalid combination of mutually exclusive flags.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06893";
                        } break;
                        case kErrorSubCode_TraceRay_BothSkip: {
                            const uint32_t ray_flags = error_record[kInst_LogError_ParameterOffset_0];
                            strm << "OpTraceRayKHR operand Ray Flags (" << string_SpvRayFlagsMask(ray_flags)
                                 << ") form an invalid combination of mutually exclusive flags.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06552";
                        } break;
                        case kErrorSubCode_TraceRay_OriginNaNOrInf: {
                            const float ray_origin_x = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float ray_origin_y = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            const float ray_origin_z = *(float*)(error_record + kInst_LogError_ParameterOffset_2);
                            strm << "OpTraceRayKHR operand Ray Origin (" << ray_origin_x << ", " << ray_origin_y << ", "
                                 << ray_origin_z << ") has a component with non-finite floating-point value.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06355";
                        } break;
                        case kErrorSubCode_TraceRay_DirectionNaNOrInf: {
                            const float ray_direction_x = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float ray_direction_y = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            const float ray_direction_z = *(float*)(error_record + kInst_LogError_ParameterOffset_2);
                            strm << "OpTraceRayKHR operand Ray Direction (" << ray_direction_x << ", " << ray_direction_y << ", "
                                 << ray_direction_z << ") has a component with non-finite floating-point value.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06355";
                        } break;
                        case kErrorSubCode_TraceRay_TNegative: {
                            const float t_min = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float t_max = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            strm << "OpTraceRayKHR operand Ray Tmin or Ray Tmax is negative (Tmin: " << t_min << ", Tmax: " << t_max
                                 << ").";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06356";
                        } break;
                        case kErrorSubCode_TraceRay_TMaxLessThanTMin: {
                            const float t_min = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float t_max = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            strm << "OpTraceRayKHR operand Ray Tmin (" << t_min << ") is greater than Ray Tmax (" << t_max << ").";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06357";
                        } break;
                        case kErrorSubCode_TraceRay_RayParametersNans: {
                            const float t_min = *(float*)(error_record + kInst_LogError_ParameterOffset_0);
                            const float t_max = *(float*)(error_record + kInst_LogError_ParameterOffset_1);
                            strm << "OpTraceRayKHR operand Ray Tmin (" << t_min << ") or Ray Tmax (" << t_max << ") are NaNs.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06358";
                        } break;
                        case kErrorSubCode_TraceRay_TlasNotBuilt: {
                            // Eg when using VK_EXT_descriptor_buffer, descriptor state is only tracked when using classic
                            // descriptor sets
                            if (descriptor_binding_index == vvl::kNoIndex32) {
                                return false;
                            }

                            const uint32_t encoded_set_index = error_record[kInst_LogError_ParameterOffset_0];
                            const uint32_t set_num = encoded_set_index >> kInst_DescriptorIndexing_SetShift;
                            const uint32_t global_descriptor_index = encoded_set_index & kInst_DescriptorIndexing_IndexMask;

                            const DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();
                            const auto& descriptor_sets =
                                desc_set_bindings.descriptor_set_binding_commands[descriptor_binding_index].bound_descriptor_sets;

                            const auto descriptor_set_state = descriptor_sets[set_num];
                            auto [binding_num, desc_index] = descriptor_set_state->GetBindingAndIndex(global_descriptor_index);

                            const auto* binding_state = descriptor_set_state->GetBinding(binding_num);
                            assert(binding_state->descriptor_class == vvl::DescriptorClass::AccelerationStructure);

                            const auto as_state = static_cast<const vvl::AccelerationStructureBinding*>(binding_state)
                                                      ->descriptors[desc_index]
                                                      .GetAccelerationStructureStateKHR();

                            strm << "(set = " << set_num << ", binding = " << binding_num << ", index " << desc_index << ") ";
                            strm << "OpTraceRayKHR operand Acceleration structure (" << gpuav.FormatHandle(as_state->VkHandle())
                                 << ") has not been built.";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06359";
                            break;
                        }
                        case kErrorSubCode_TraceRay_SkipTrianglesWithPipelineSkipAABBs: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                                 << ") contains SkipTrianglesKHR, but pipeline was created with "
                                    "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06553";
                        } break;
                        case kErrorSubCode_TraceRay_SkipAABBsWithPipelineSkipTriangles: {
                            const uint32_t value = error_record[kInst_LogError_ParameterOffset_0];
                            strm << string_SpvOpcode(opcode) << " operand Ray Flags (0x" << std::hex << value
                                 << ") contains SkipAABBsKHR, but pipeline was created with "
                                    "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR. ";
                            out_vuid_msg = "VUID-RuntimeSpirv-OpTraceRayKHR-06554";
                        } break;

                        default:
                            error_found = false;
                            break;
                    }
                    out_error_msg += strm.str();
                    return error_found;
                };

            return inst_error_logger;
        });
}

struct AccelerationStructureGpuStateUpdateShader {
    static size_t GetSpirvSize() { return setup_acceleration_structure_gpu_state_update_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return setup_acceleration_structure_gpu_state_update_comp; }

    glsl::AccelerationStructureGpuStateUpdateShaderPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

void UpdateAccelerationStructureGpuState(Validator& gpuav, CommandBufferSubState& cb, const Location& loc, uint32_t info_count,
                                         const VkAccelerationStructureBuildGeometryInfoKHR* infos) {
    valpipe::ComputePipeline<AccelerationStructureGpuStateUpdateShader>& as_gpu_state_update_pipeline =
        cb.gpuav_.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<AccelerationStructureGpuStateUpdateShader>>(
            cb.gpuav_, Location(vvl::Func::Empty));

    if (!as_gpu_state_update_pipeline.valid) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb, VK_PIPELINE_BIND_POINT_COMPUTE);

    DispatchCmdBindPipeline(cb.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, as_gpu_state_update_pipeline.pipeline);

    for (uint32_t info_i = 0; info_i < info_count; ++info_i) {
        const VkAccelerationStructureBuildGeometryInfoKHR& info = infos[info_i];

        auto dst_as_state = gpuav.Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);
        if (!dst_as_state) {
            gpuav.InternalError(info.dstAccelerationStructure, loc,
                                "gpuav::UpdateAccelerationStructureGpuState(): Unrecognized destination acceleration structure.");
            return;
        }

        AccelerationStructureKHRSubState& dst_as_gpuav_state = SubState(*dst_as_state);

        AccelerationStructureGpuStateUpdateShader shader_resources;
        shader_resources.push_constants.gpu_state_ptr = dst_as_gpuav_state.gpu_state.offset_address;
        shader_resources.push_constants.state = 0;
        shader_resources.push_constants.state |= 1u << glsl::kAsGpuStateValidShift;
        shader_resources.push_constants.state |= (uint32_t)info.mode << glsl::kBuildModeShift;
        VkAccelerationStructureTypeKHR type = dst_as_state->GetType();
        shader_resources.push_constants.state |= (uint32_t)type << glsl::kAsTypeShift;
        if (!as_gpu_state_update_pipeline.BindShaderResources(gpuav, cb, shader_resources)) {
            return;
        }
        VkPipelineStageFlags all_shaders_stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        if (gpuav.enabled_features.rayTracingPipeline) {
            all_shaders_stage_mask |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        }
        {
            VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
            barrier_write_after_read.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_write_after_read.buffer = dst_as_gpuav_state.gpu_state.buffer;
            barrier_write_after_read.offset = dst_as_gpuav_state.gpu_state.offset;
            barrier_write_after_read.size = dst_as_gpuav_state.gpu_state.size;

            DispatchCmdPipelineBarrier(cb.VkHandle(), all_shaders_stage_mask, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr,
                                       1, &barrier_write_after_read, 0, nullptr);
        }

        DispatchCmdDispatch(cb.VkHandle(), 1, 1, 1);

        {
            VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
            barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_read_after_write.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier_read_after_write.buffer = dst_as_gpuav_state.gpu_state.buffer;
            barrier_read_after_write.offset = dst_as_gpuav_state.gpu_state.offset;
            barrier_read_after_write.size = dst_as_gpuav_state.gpu_state.size;

            DispatchCmdPipelineBarrier(cb.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, all_shaders_stage_mask, 0, 0, nullptr,
                                       1, &barrier_read_after_write, 0, nullptr);
        }
    }
}

}  // namespace gpuav
