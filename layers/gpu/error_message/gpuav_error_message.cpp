/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "gpu/core/gpuav.h"
#include "gpu/error_message/gpuav_vuids.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "state_tracker/shader_instruction.h"
#include "gpu/shaders/gpu_error_header.h"

// Generate the stage-specific part of the message.
static void GenerateStageMessage(const uint32_t *error_record, std::string &msg) {
    using namespace gpuav;
    using namespace glsl;
    std::ostringstream strm;
    switch (error_record[kHeaderStageIdOffset]) {
        case kHeaderStageIdMultiEntryPoint: {
            strm << "Stage has multiple OpEntryPoint and could not detect stage. ";
        } break;
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << error_record[kHeaderVertexIndexOffset]
                 << " Instance Index = " << error_record[kHeaderVertInstanceIndexOffset] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << error_record[kHeaderTessCltInvocationIdOffset]
                 << ", Primitive ID = " << error_record[kHeaderTessCtlPrimitiveIdOffset];
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Primitive ID = " << error_record[kHeaderTessEvalPrimitiveIdOffset]
                 << ", TessCoord (u, v) = (" << error_record[kHeaderTessEvalCoordUOffset] << ", "
                 << error_record[kHeaderTessEvalCoordVOffset] << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << error_record[kHeaderGeomPrimitiveIdOffset]
                 << " Invocation ID = " << error_record[kHeaderGeomInvocationIdOffset] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&error_record[kHeaderFragCoordXOffset]) << ", "
                 << *reinterpret_cast<const float *>(&error_record[kHeaderFragCoordYOffset]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID (x, y, z) = (" << error_record[kHeaderInvocationIdXOffset] << ", "
                 << error_record[kHeaderInvocationIdYOffset] << ", " << error_record[kHeaderInvocationIdZOffset] << ")";
        } break;
        case spv::ExecutionModelRayGenerationKHR: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset]
                 << ", " << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelIntersectionKHR: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelAnyHitKHR: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelClosestHitKHR: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelMissKHR: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelCallableKHR: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << error_record[kHeaderRayTracingLaunchIdXOffset] << ", "
                 << error_record[kHeaderRayTracingLaunchIdYOffset] << ", " << error_record[kHeaderRayTracingLaunchIdZOffset]
                 << "). ";
        } break;
        case spv::ExecutionModelTaskEXT: {
            strm << "Stage = TaskEXT. Global invocation ID (x, y, z) = (" << error_record[kHeaderTaskGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderTaskGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderTaskGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelMeshEXT: {
            strm << "Stage = MeshEXT. Global invocation ID (x, y, z) = (" << error_record[kHeaderMeshGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderMeshGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderMeshGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelTaskNV: {
            strm << "Stage = TaskNV. Global invocation ID (x, y, z) = (" << error_record[kHeaderTaskGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderTaskGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderTaskGlobalInvocationIdZOffset] << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            strm << "Stage = MeshNV. Global invocation ID (x, y, z) = (" << error_record[kHeaderMeshGlobalInvocationIdXOffset]
                 << ", " << error_record[kHeaderMeshGlobalInvocationIdYOffset] << ", "
                 << error_record[kHeaderMeshGlobalInvocationIdZOffset] << " )";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << error_record[kHeaderStageIdOffset] << "). ";
            assert(false);
        } break;
    }
    strm << "\n";
    msg = strm.str();
}

namespace gpuav {
bool Validator::LogMessageInstBindlessDescriptor(const uint32_t *error_record, std::string &out_error_msg,
                                                 std::string &out_vuid_msg, const std::vector<DescSetState> &descriptor_sets,
                                                 const Location &loc, bool uses_shader_object, bool &out_oob_access) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    const GpuVuid vuid = GetGpuVuid(loc.function);

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBindlessDescriptorBounds: {
            strm << "(set = " << error_record[kInstBindlessBoundsDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessBoundsDescBindingOffset] << ") Index of "
                 << error_record[kInstBindlessBoundsDescIndexOffset] << " used to index descriptor array of length "
                 << error_record[kInstBindlessBoundsDescBoundOffset] << ".";
            out_vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorUninit: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " is uninitialized.";
            out_vuid_msg = vuid.invalid_descriptor;
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorDestroyed: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " references a resource that was destroyed.";
            out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorOOB: {
            const uint32_t set_num = error_record[kInstBindlessBuffOOBDescSetOffset];
            const uint32_t binding_num = error_record[kInstBindlessBuffOOBDescBindingOffset];
            const uint32_t desc_index = error_record[kInstBindlessBuffOOBDescIndexOffset];
            const uint32_t size = error_record[kInstBindlessBuffOOBBuffSizeOffset];
            const uint32_t offset = error_record[kInstBindlessBuffOOBBuffOffOffset];
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            assert(binding_state);
            if (size == 0) {
                strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                     << " is uninitialized.";
                out_vuid_msg = vuid.invalid_descriptor;
                error_found = true;
                break;
            }
            out_oob_access = true;
            auto desc_class = binding_state->descriptor_class;
            if (desc_class == vvl::DescriptorClass::Mutable) {
                desc_class = static_cast<const vvl::MutableBinding *>(binding_state)->descriptors[desc_index].ActiveClass();
            }

            switch (desc_class) {
                case vvl::DescriptorClass::GeneralBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " and highest byte accessed was " << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                case vvl::DescriptorClass::TexelBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " texels and highest texel accessed was "
                         << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                        out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                default:
                    // other OOB checks are not implemented yet
                    assert(false);
            }
        } break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool Validator::LogMessageInstBufferDeviceAddress(const uint32_t *error_record, std::string &out_error_msg,
                                                  std::string &out_vuid_msg, bool &out_oob_access) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBufferDeviceAddressUnallocRef: {
            out_oob_access = true;
            const char *access_type = error_record[kInstBuffAddrAccessInstructionOffset] == spv::OpStore ? "written" : "read";
            uint64_t address = *reinterpret_cast<const uint64_t *>(error_record + kInstBuffAddrUnallocDescPtrLoOffset);
            strm << "Out of bounds access: " << error_record[kInstBuffAddrAccessByteSizeOffset] << " bytes " << access_type
                 << " at buffer device address 0x" << std::hex << address << '.';
            out_vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool Validator::LogMessageInstRayQuery(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg) const {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeRayQueryNegativeMin: {
            // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
            strm << "OpRayQueryInitializeKHR operand Ray Tmin value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryNegativeMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryMinMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is less than RayTmin. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350";
        } break;
        case kErrorSubCodeRayQueryMinNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmin is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryMaxNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryDirectionNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryDirectionFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryBothSkip: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889";
        } break;
        case kErrorSubCodeRayQuerySkipCull: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06890";
        } break;
        case kErrorSubCodeRayQueryOpaque: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg = strm.str();
    return error_found;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
bool Validator::AnalyzeAndGenerateMessage(VkCommandBuffer cmd_buffer, const LogObjectList &objlist, uint32_t operation_index,
                                          const uint32_t *error_record, const std::vector<DescSetState> &descriptor_sets,
                                          VkPipelineBindPoint pipeline_bind_point, bool uses_shader_object, bool uses_robustness,
                                          const Location &loc) {
    // The second word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor. So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record". This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record. If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.

    std::string error_msg;
    std::string vuid_msg;
    bool oob_access = false;
    bool error_found = false;
    switch (error_record[glsl::kHeaderErrorGroupOffset]) {
        case glsl::kErrorGroupInstBindlessDescriptor:
            error_found = LogMessageInstBindlessDescriptor(error_record, error_msg, vuid_msg, descriptor_sets, loc,
                                                           uses_shader_object, oob_access);
            break;
        case glsl::kErrorGroupInstBufferDeviceAddress:
            error_found = LogMessageInstBufferDeviceAddress(error_record, error_msg, vuid_msg, oob_access);
            break;
        case glsl::kErrorGroupInstRayQuery:
            error_found = LogMessageInstRayQuery(error_record, error_msg, vuid_msg);
            break;
        default:
            break;
    }

    if (error_found) {
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const gpu::GpuAssistedShaderTracker *tracker_info = nullptr;
        const uint32_t shader_id = error_record[glsl::kHeaderShaderIdOffset];
        auto it = shader_map_.find(shader_id);
        if (it != shader_map_.end()) {
            tracker_info = &it->second;
        }

        // If we somehow can't find our state, we can still report our error message
        std::vector<spirv::Instruction> instructions;
        if (tracker_info) {
            spirv::GenerateInstructions(tracker_info->instrumented_spirv, instructions);
        }
        std::string debug_info_message =
            GenerateDebugInfoMessage(cmd_buffer, instructions, error_record[gpuav::glsl::kHeaderInstructionIdOffset], tracker_info,
                                     pipeline_bind_point, operation_index);

        // TODO - Need to unify with debug printf
        std::string stage_message;
        GenerateStageMessage(error_record, stage_message);

        if (uses_robustness && oob_access) {
            if (gpuav_settings.warn_on_robust_oob) {
                LogWarning(vuid_msg.c_str(), objlist, loc, "%s\n%s%s", error_msg.c_str(), stage_message.c_str(),
                           debug_info_message.c_str());
            }
        } else {
            LogError(vuid_msg.c_str(), objlist, loc, "%s\n%s%s", error_msg.c_str(), stage_message.c_str(),
                     debug_info_message.c_str());
        }
    }

    return error_found;
}
}  // namespace gpuav
