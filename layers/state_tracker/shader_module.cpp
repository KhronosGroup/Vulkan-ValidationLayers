/* Copyright (c) 2021-2023 The Khronos Group Inc.
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

#include "state_tracker/shader_module.h"

#include <sstream>
#include <string>

#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/descriptor_sets.h"
#include "spirv_grammar_helper.h"

void DecorationSet::Add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationLocation:
            location = value;
            break;
        case spv::DecorationPatch:
            flags |= patch_bit;
            break;
        case spv::DecorationBlock:
            flags |= block_bit;
            break;
        case spv::DecorationBufferBlock:
            flags |= buffer_block_bit;
            break;
        case spv::DecorationComponent:
            component = value;
            break;
        case spv::DecorationInputAttachmentIndex:
            input_attachment_index = value;
            break;
        case spv::DecorationDescriptorSet:
            set = value;
            break;
        case spv::DecorationBinding:
            binding = value;
            break;
        case spv::DecorationNonWritable:
            flags |= nonwritable_bit;
            break;
        case spv::DecorationBuiltIn:
            flags |= builtin_bit;
            builtin = value;
            break;
        case spv::DecorationNonReadable:
            flags |= nonreadable_bit;
            break;
        case spv::DecorationPerVertexNV:
            flags |= per_vertex_bit;
            break;
        case spv::DecorationPassthroughNV:
            flags |= passthrough_bit;
            break;
        case spv::DecorationAliased:
            flags |= aliased_bit;
            break;
    }
}

static uint32_t ExecutionModelToShaderStageFlagBits(uint32_t mode) {
    switch (mode) {
        case spv::ExecutionModelVertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelGLCompute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelRayGenerationKHR:
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case spv::ExecutionModelAnyHitKHR:
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case spv::ExecutionModelClosestHitKHR:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case spv::ExecutionModelMissKHR:
            return VK_SHADER_STAGE_MISS_BIT_KHR;
        case spv::ExecutionModelIntersectionKHR:
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case spv::ExecutionModelCallableKHR:
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case spv::ExecutionModelTaskNV:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        case spv::ExecutionModelTaskEXT:
            return VK_SHADER_STAGE_TASK_BIT_EXT;
        case spv::ExecutionModelMeshEXT:
            return VK_SHADER_STAGE_MESH_BIT_EXT;
        default:
            return 0;
    }
}

SHADER_MODULE_STATE::EntryPoint::EntryPoint(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint)
    : entrypoint_insn(entrypoint),
      stage(static_cast<VkShaderStageFlagBits>(ExecutionModelToShaderStageFlagBits(entrypoint.Word(1)))),
      name(entrypoint.GetAsString(3)) {
    if (module_state.has_valid_spirv) {
        // For some analyses, we need to know about all ids referenced by the static call tree of a particular entrypoint. This is
        // important for identifying the set of shader resources actually used by an entrypoint, for example.
        // Note: we only explore parts of the image which might actually contain ids we care about for the above analyses.
        //  - NOT the shader input/output interfaces.
        //
        // TODO: The set of interesting opcodes here was determined by eyeballing the SPIRV spec. It might be worth
        // converting parts of this to be generated from the machine-readable spec instead.
        vvl::unordered_set<uint32_t> worklist;
        worklist.insert(entrypoint_insn.Word(2));

        while (!worklist.empty()) {
            auto id_iter = worklist.begin();
            auto id = *id_iter;
            worklist.erase(id_iter);

            const Instruction* insn = module_state.FindDef(id);
            if (!insn) {
                // ID is something we didn't collect in SpirvStaticData. that's OK -- we'll stumble across all kinds of things here
                // that we may not care about.
                continue;
            }

            // Try to add to the output set
            if (!accessible_ids.insert(id).second) {
                continue;  // If we already saw this id, we don't want to walk it again.
            }

            switch (insn->Opcode()) {
                case spv::OpFunction:
                    // Scan whole body of the function, enlisting anything interesting
                    while (++insn, insn->Opcode() != spv::OpFunctionEnd) {
                        switch (insn->Opcode()) {
                            case spv::OpLoad:
                                worklist.insert(insn->Word(3));  // ptr
                                break;
                            case spv::OpStore:
                                worklist.insert(insn->Word(1));  // ptr
                                break;
                            case spv::OpAccessChain:
                            case spv::OpInBoundsAccessChain:
                                worklist.insert(insn->Word(3));  // base ptr
                                break;
                            case spv::OpSampledImage:
                            case spv::OpImageSampleImplicitLod:
                            case spv::OpImageSampleExplicitLod:
                            case spv::OpImageSampleDrefImplicitLod:
                            case spv::OpImageSampleDrefExplicitLod:
                            case spv::OpImageSampleProjImplicitLod:
                            case spv::OpImageSampleProjExplicitLod:
                            case spv::OpImageSampleProjDrefImplicitLod:
                            case spv::OpImageSampleProjDrefExplicitLod:
                            case spv::OpImageFetch:
                            case spv::OpImageGather:
                            case spv::OpImageDrefGather:
                            case spv::OpImageRead:
                            case spv::OpImage:
                            case spv::OpImageQueryFormat:
                            case spv::OpImageQueryOrder:
                            case spv::OpImageQuerySizeLod:
                            case spv::OpImageQuerySize:
                            case spv::OpImageQueryLod:
                            case spv::OpImageQueryLevels:
                            case spv::OpImageQuerySamples:
                            case spv::OpImageSparseSampleImplicitLod:
                            case spv::OpImageSparseSampleExplicitLod:
                            case spv::OpImageSparseSampleDrefImplicitLod:
                            case spv::OpImageSparseSampleDrefExplicitLod:
                            case spv::OpImageSparseSampleProjImplicitLod:
                            case spv::OpImageSparseSampleProjExplicitLod:
                            case spv::OpImageSparseSampleProjDrefImplicitLod:
                            case spv::OpImageSparseSampleProjDrefExplicitLod:
                            case spv::OpImageSparseFetch:
                            case spv::OpImageSparseGather:
                            case spv::OpImageSparseDrefGather:
                            case spv::OpImageTexelPointer:
                                worklist.insert(insn->Word(3));  // Image or sampled image
                                break;
                            case spv::OpImageWrite:
                                worklist.insert(insn->Word(1));  // Image -- different operand order to above
                                break;
                            case spv::OpFunctionCall:
                                for (uint32_t i = 3; i < insn->Length(); i++) {
                                    worklist.insert(insn->Word(i));  // fn itself, and all args
                                }
                                break;

                            case spv::OpExtInst:
                                for (uint32_t i = 5; i < insn->Length(); i++) {
                                    worklist.insert(insn->Word(i));  // Operands to ext inst
                                }
                                break;

                            default: {
                                if (AtomicOperation(insn->Opcode())) {
                                    if (insn->Opcode() == spv::OpAtomicStore) {
                                        worklist.insert(insn->Word(1));  // ptr
                                    } else {
                                        worklist.insert(insn->Word(3));  // ptr
                                    }
                                }
                                break;
                            }
                        }
                    }
                    break;
            }
        }

        // Now that the accessible_ids list is known, fill in any information that can be statically known per EntryPoint
        for (const Instruction* insn : module_state.GetDecorationInstructions()) {
            if (insn->Word(2) == spv::DecorationInputAttachmentIndex) {
                const uint32_t attachment_index = insn->Word(3);
                const uint32_t id = insn->Word(1);

                if (accessible_ids.count(id)) {
                    const Instruction* def = module_state.FindDef(id);
                    if (def->Opcode() == spv::OpVariable && def->StorageClass() == spv::StorageClassUniformConstant) {
                        const uint32_t num_locations = module_state.GetLocationsConsumedByType(def->Word(1), false);
                        for (uint32_t offset = 0; offset < num_locations; offset++) {
                            attachment_indexes.insert(attachment_index + offset);
                        }
                    }
                }
            }
        }

        for (const auto& id : accessible_ids) {
            const Instruction* insn = module_state.FindDef(id);
            if (insn->Opcode() != spv::OpVariable) {
                continue;
            }
            const uint32_t storage_class = insn->StorageClass();
            // These are the only storage classes that interface with a descriptor
            // see vkspec.html#interfaces-resources-descset
            if (storage_class == spv::StorageClassUniform || storage_class == spv::StorageClassUniformConstant ||
                storage_class == spv::StorageClassStorageBuffer) {
                resource_interface_variables.emplace_back(module_state, insn, stage);
            }
        }
    }
}

std::optional<VkPrimitiveTopology> SHADER_MODULE_STATE::GetTopology(const Instruction& entrypoint) const {
    std::optional<VkPrimitiveTopology> result;

    auto entrypoint_id = entrypoint.Word(2);
    bool is_point_mode = false;

    auto it = static_data_.execution_mode_inst.find(entrypoint_id);
    if (it != static_data_.execution_mode_inst.end()) {
        for (const Instruction* insn : it->second) {
            switch (insn->Word(2)) {
                case spv::ExecutionModePointMode:
                    // In tessellation shaders, PointMode is separate and trumps the tessellation topology.
                    is_point_mode = true;
                    break;

                case spv::ExecutionModeOutputPoints:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
                    break;

                case spv::ExecutionModeIsolines:
                case spv::ExecutionModeOutputLineStrip:
                case spv::ExecutionModeOutputLinesNV:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
                    break;

                case spv::ExecutionModeTriangles:
                case spv::ExecutionModeQuads:
                case spv::ExecutionModeOutputTriangleStrip:
                case spv::ExecutionModeOutputTrianglesNV:
                    result.emplace(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
                    break;
            }
        }
    }

    if (is_point_mode) {
        result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    }

    return result;
}

std::optional<VkPrimitiveTopology> SHADER_MODULE_STATE::GetTopology() const {
    if (static_data_.entry_points.size() > 0) {
        return GetTopology(static_data_.entry_points[0].entrypoint_insn);
    }
    return {};
}

static inline bool IsImageOperandsBiasOffset(uint32_t type) {
    return (type & (spv::ImageOperandsBiasMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask |
                    spv::ImageOperandsConstOffsetsMask)) != 0;
}

SHADER_MODULE_STATE::StaticData::StaticData(const SHADER_MODULE_STATE& module_state) {
    // Parse the words first so we have instruction class objects to use
    {
        std::vector<uint32_t>::const_iterator it = module_state.words_.cbegin();
        it += 5;  // skip first 5 word of header
        while (it != module_state.words_.cend()) {
            Instruction insn(it);
            const uint32_t opcode = insn.Opcode();

            // Check for opcodes that would require reparsing of the words
            if (opcode == spv::OpGroupDecorate || opcode == spv::OpDecorationGroup || opcode == spv::OpGroupMemberDecorate) {
                assert(has_group_decoration == false);  // if assert, spirv-opt didn't flatten it
                has_group_decoration = true;
                break;  // no need to continue parsing
            }

            instructions.push_back(insn);
            it += insn.Length();
        }
        instructions.shrink_to_fit();
    }

    std::vector<const Instruction*> entry_point_instructions;

    // Loop through once and build up the static data
    // Also process the entry points
    for (const Instruction& insn : instructions) {
        // Build definition list
        if (insn.ResultId() != 0) {
            definitions[insn.Word(insn.ResultId())] = &insn;
        }

        switch (insn.Opcode()) {
            // Specialization constants
            case spv::OpSpecConstantTrue:
            case spv::OpSpecConstantFalse:
            case spv::OpSpecConstant:
            case spv::OpSpecConstantComposite:
            case spv::OpSpecConstantOp:
                has_specialization_constants = true;
                break;

            // Decorations
            case spv::OpDecorate: {
                auto target_id = insn.Word(1);
                decorations[target_id].Add(insn.Word(2), insn.Length() > 3u ? insn.Word(3) : 0u);
                decoration_inst.push_back(&insn);
                if (insn.Word(2) == spv::DecorationBuiltIn) {
                    builtin_decoration_inst.push_back(&insn);
                } else if (insn.Word(2) == spv::DecorationSpecId) {
                    spec_const_map[insn.Word(3)] = target_id;
                }

            } break;
            case spv::OpMemberDecorate: {
                member_decoration_inst.push_back(&insn);
                if (insn.Word(3) == spv::DecorationBuiltIn) {
                    builtin_decoration_inst.push_back(&insn);
                }
            } break;

            case spv::OpCapability:
                capability_list.push_back(static_cast<spv::Capability>(insn.Word(1)));
                break;

            case spv::OpVariable:
                variable_inst.push_back(&insn);
                break;

            // Execution Mode
            case spv::OpExecutionMode:
            case spv::OpExecutionModeId: {
                execution_mode_inst[insn.Word(1)].push_back(&insn);
            } break;
            // Listed from vkspec.html#ray-tracing-repack
            case spv::OpTraceRayKHR:
            case spv::OpTraceRayMotionNV:
            case spv::OpReportIntersectionKHR:
            case spv::OpExecuteCallableKHR:
                has_invocation_repack_instruction = true;
                break;

            // Entry points
            case spv::OpEntryPoint: {
                entry_point_instructions.push_back(&insn);
                break;
            }

            // Access operations
            case spv::OpImageSampleImplicitLod:
            case spv::OpImageSampleProjImplicitLod:
            case spv::OpImageSampleProjExplicitLod:
            case spv::OpImageSparseSampleImplicitLod:
            case spv::OpImageSparseSampleProjImplicitLod:
            case spv::OpImageSparseSampleProjExplicitLod: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                sampler_load_ids.emplace_back(load_id);
                sampler_implicitLod_dref_proj_load_ids.emplace_back(load_id);
                // ImageOperands in index: 5
                if (insn.Length() > 5 && IsImageOperandsBiasOffset(insn.Word(5))) {
                    sampler_bias_offset_load_ids.emplace_back(load_id);
                }
                break;
            }
            case spv::OpImageDrefGather:
            case spv::OpImageSparseDrefGather: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(3) : insn.Word(3);
                image_dref_load_ids.emplace_back(load_id);
                break;
            }
            case spv::OpImageSampleDrefImplicitLod:
            case spv::OpImageSampleDrefExplicitLod:
            case spv::OpImageSampleProjDrefImplicitLod:
            case spv::OpImageSampleProjDrefExplicitLod:
            case spv::OpImageSparseSampleDrefImplicitLod:
            case spv::OpImageSparseSampleDrefExplicitLod:
            case spv::OpImageSparseSampleProjDrefImplicitLod:
            case spv::OpImageSparseSampleProjDrefExplicitLod: {
                // combined image samples are just OpLoad, but also can be separate image and sampler
                const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                auto sampler_load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                auto image_load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(3) : insn.Word(3);

                image_dref_load_ids.emplace_back(image_load_id);
                sampler_load_ids.emplace_back(sampler_load_id);
                sampler_implicitLod_dref_proj_load_ids.emplace_back(sampler_load_id);
                // ImageOperands in index: 6
                if (insn.Length() > 6 && IsImageOperandsBiasOffset(insn.Word(6))) {
                    sampler_bias_offset_load_ids.emplace_back(sampler_load_id);
                }
                break;
            }
            case spv::OpImageSampleExplicitLod:
            case spv::OpImageSparseSampleExplicitLod: {
                // ImageOperands in index: 5
                if (insn.Length() > 5 && IsImageOperandsBiasOffset(insn.Word(5))) {
                    // combined image samples are just OpLoad, but also can be separate image and sampler
                    const Instruction* id = module_state.FindDef(insn.Word(3));  // <id> Sampled Image
                    auto load_id = (id->Opcode() == spv::OpSampledImage) ? id->Word(4) : insn.Word(3);
                    sampler_load_ids.emplace_back(load_id);
                    sampler_bias_offset_load_ids.emplace_back(load_id);
                }
                break;
            }
            case spv::OpStore: {
                store_pointer_ids.emplace_back(insn.Word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpImageRead:
            case spv::OpImageSparseRead: {
                image_read_load_ids.emplace_back(insn.Word(3));
                break;
            }
            case spv::OpImageWrite: {
                image_write_load_ids.emplace_back(insn.Word(1));
                image_write_load_id_map.emplace(&insn, insn.Word(1));
                break;
            }
            case spv::OpSampledImage: {
                // 3: image load id, 4: sampler load id
                sampled_image_load_ids.emplace_back(std::pair<uint32_t, uint32_t>(insn.Word(3), insn.Word(4)));
                break;
            }
            case spv::OpLoad: {
                // 2: Load id, 3: object id or AccessChain id
                load_members.emplace(insn.Word(2), insn.Word(3));
                break;
            }
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain: {
                if (insn.Length() == 4) {
                    // If it is for struct, the length is only 4.
                    // 2: AccessChain id, 3: object id
                    accesschain_members.emplace(insn.Word(2), std::pair<uint32_t, uint32_t>(insn.Word(3), 0));
                } else {
                    // 2: AccessChain id, 3: object id, 4: object id of array index
                    accesschain_members.emplace(insn.Word(2), std::pair<uint32_t, uint32_t>(insn.Word(3), insn.Word(4)));
                }
                break;
            }
            case spv::OpImageTexelPointer: {
                // 2: ImageTexelPointer id, 3: object id
                image_texel_pointer_members.emplace(insn.Word(2), insn.Word(3));
                break;
            }

            default:
                if (AtomicOperation(insn.Opcode()) == true) {
                    atomic_inst.push_back(&insn);
                    if (insn.Opcode() == spv::OpAtomicStore) {
                        atomic_store_pointer_ids.emplace_back(insn.Word(1));
                        atomic_pointer_ids.emplace_back(insn.Word(1));
                    } else {
                        atomic_pointer_ids.emplace_back(insn.Word(3));
                    }
                }
                // We don't care about any other defs for now.
                break;
        }
    }

    // Need to build the definitions table for FindDef before looking for which instructions each entry point uses
    for (const auto& insn : entry_point_instructions) {
        entry_points.emplace_back(EntryPoint{module_state, *insn});
    }

    SHADER_MODULE_STATE::SetPushConstantUsedInShader(module_state, entry_points);
}

void SHADER_MODULE_STATE::PreprocessShaderBinary(const spv_target_env env) {
    if (static_data_.has_group_decoration) {
        spvtools::Optimizer optimizer(env);
        optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
        std::vector<uint32_t> optimized_binary;
        // Run optimizer to flatten decorations only, set skip_validation so as to not re-run validator
        auto result = optimizer.Run(words_.data(), words_.size(), &optimized_binary, spvtools::ValidatorOptions(), true);

        if (result) {
            // NOTE: We need to update words with the result from the spirv-tools optimizer.
            // **THIS ONLY HAPPENS ON INITIALIZATION**. words should remain const for the lifetime
            // of the SHADER_MODULE_STATE instance.
            *const_cast<std::vector<uint32_t>*>(&words_) = std::move(optimized_binary);
            // Will need to update static data now the words have changed or else the def_index will not align
            // It is really rare this will get here as Group Decorations have been deprecated and before this was added no one ever
            // raised an issue for a bug that would crash the layers that was around for many releases
            StaticData new_static_data(*this);
            *const_cast<StaticData*>(&static_data_) = std::move(new_static_data);
        }
    }
}

void SHADER_MODULE_STATE::DescribeTypeInner(std::ostringstream& ss, uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypeBool:
            ss << "bool";
            break;
        case spv::OpTypeInt:
            ss << (insn->Word(3) ? 's' : 'u') << "int" << insn->Word(2);
            break;
        case spv::OpTypeFloat:
            ss << "float" << insn->Word(2);
            break;
        case spv::OpTypeVector:
            ss << "vec" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2));
            break;
        case spv::OpTypeMatrix:
            ss << "mat" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2));
            break;
        case spv::OpTypeArray:
            ss << "arr[" << GetConstantValueById(insn->Word(3)) << "] of ";
            DescribeTypeInner(ss, insn->Word(2));
            break;
        case spv::OpTypeRuntimeArray:
            ss << "runtime arr[] of ";
            DescribeTypeInner(ss, insn->Word(2));
            break;
        case spv::OpTypePointer:
            ss << "ptr to " << string_SpvStorageClass(insn->Word(2)) << " ";
            DescribeTypeInner(ss, insn->Word(3));
            break;
        case spv::OpTypeStruct: {
            ss << "struct of (";
            for (uint32_t i = 2; i < insn->Length(); i++) {
                DescribeTypeInner(ss, insn->Word(i));
                if (i == insn->Length() - 1) {
                    ss << ")";
                } else {
                    ss << ", ";
                }
            }
            break;
        }
        case spv::OpTypeSampler:
            ss << "sampler";
            break;
        case spv::OpTypeSampledImage:
            ss << "sampler+";
            DescribeTypeInner(ss, insn->Word(2));
            break;
        case spv::OpTypeImage:
            ss << "image(dim=" << insn->Word(3) << ", sampled=" << insn->Word(7) << ")";
            break;
        case spv::OpTypeAccelerationStructureNV:
            ss << "accelerationStruture";
            break;
        default:
            ss << "oddtype";
            break;
    }
}

std::string SHADER_MODULE_STATE::DescribeType(uint32_t type) const {
    std::ostringstream ss;
    DescribeTypeInner(ss, type);
    return ss.str();
}

const SHADER_MODULE_STATE::StructInfo* SHADER_MODULE_STATE::FindEntrypointPushConstant(char const* name,
                                                                                       VkShaderStageFlagBits stageBits) const {
    for (const auto& entry_point : static_data_.entry_points) {
        if (entry_point.name.compare(name) == 0 && entry_point.stage == stageBits) {
            return &(entry_point.push_constant_used_in_shader);
        }
    }
    return nullptr;
}

std::optional<Instruction> SHADER_MODULE_STATE::FindEntrypoint(char const* name, VkShaderStageFlagBits stageBits) const {
    std::optional<Instruction> result;
    for (const auto& entry_point : static_data_.entry_points) {
        if (entry_point.name.compare(name) == 0 && entry_point.stage == stageBits) {
            result.emplace(entry_point.entrypoint_insn);
        }
    }
    return result;
}

// Because the following is legal, need the entry point
//    OpEntryPoint GLCompute %main "name_a"
//    OpEntryPoint GLCompute %main "name_b"
// Assumes shader module contains no spec constants used to set the local size values
bool SHADER_MODULE_STATE::FindLocalSize(const Instruction& entrypoint, uint32_t& local_size_x, uint32_t& local_size_y,
                                        uint32_t& local_size_z) const {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    for (const Instruction* insn : GetBuiltinDecorationList()) {
        if (insn->GetBuiltIn() == spv::BuiltInWorkgroupSize) {
            const uint32_t workgroup_size_id = insn->Word(1);
            const Instruction* composite_def = FindDef(workgroup_size_id);
            if (composite_def->Opcode() == spv::OpConstantComposite) {
                // VUID-WorkgroupSize-WorkgroupSize-04427 makes sure this is a OpTypeVector of int32
                local_size_x = GetConstantValueById(composite_def->Word(3));
                local_size_y = GetConstantValueById(composite_def->Word(4));
                local_size_z = GetConstantValueById(composite_def->Word(5));
                return true;
            }
        }
    }

    auto entrypoint_id = entrypoint.Word(2);
    auto it = static_data_.execution_mode_inst.find(entrypoint_id);
    if (it != static_data_.execution_mode_inst.end()) {
        for (const Instruction* insn : it->second) {
            if (insn->Opcode() == spv::OpExecutionMode && insn->Word(2) == spv::ExecutionModeLocalSize) {
                local_size_x = insn->Word(3);
                local_size_y = insn->Word(4);
                local_size_z = insn->Word(5);
                return true;
            } else if (insn->Opcode() == spv::OpExecutionModeId && insn->Word(2) == spv::ExecutionModeLocalSizeId) {
                local_size_x = GetConstantValueById(insn->Word(3));
                local_size_y = GetConstantValueById(insn->Word(4));
                local_size_z = GetConstantValueById(insn->Word(5));
                return true;
            }
        }
    }
    return false;  // not found
}

// If the instruction at id is a constant or copy of a constant, returns a valid iterator pointing to that instruction.
// Otherwise, returns src->end().
const Instruction* SHADER_MODULE_STATE::GetConstantDef(uint32_t id) const {
    const Instruction* value = FindDef(id);

    // If id is a copy, see where it was copied from
    if (value && ((value->Opcode() == spv::OpCopyObject) || (value->Opcode() == spv::OpCopyLogical))) {
        id = value->Word(3);
        value = FindDef(id);
    }

    if (value && (value->Opcode() == spv::OpConstant)) {
        return value;
    }
    return nullptr;
}

// Either returns the constant value described by the instruction at id, or 1
uint32_t SHADER_MODULE_STATE::GetConstantValueById(uint32_t id) const {
    const Instruction* value = GetConstantDef(id);

    if (!value) {
        // TODO: Either ensure that the specialization transform is already performed on a module we're
        //       considering here, OR -- specialize on the fly now.
        return 1;
    }

    return value->GetConstantValue();
}

// Returns spv::Dim of the given OpVariable
spv::Dim SHADER_MODULE_STATE::GetShaderResourceDimensionality(const ResourceInterfaceVariable& resource) const {
    const Instruction* type = FindDef(resource.type_id);
    while (true) {
        switch (type->Opcode()) {
            case spv::OpTypeSampledImage:
                type = FindDef(type->Word(2));
                break;
            case spv::OpTypePointer:
                type = FindDef(type->Word(3));
                break;
            case spv::OpTypeImage:
                return spv::Dim(type->Word(3));
            default:
                return spv::DimMax;
        }
    }
}

// Returns the number of Location slots used for a given ID reference to a OpType*
uint32_t SHADER_MODULE_STATE::GetLocationsConsumedByType(uint32_t type, bool strip_array_level) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetLocationsConsumedByType(insn->Word(3), strip_array_level);
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetLocationsConsumedByType(insn->Word(2), false);
            } else {
                return GetConstantValueById(insn->Word(3)) * GetLocationsConsumedByType(insn->Word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn->Word(3) * GetLocationsConsumedByType(insn->Word(2), false);
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            auto bit_width =
                (scalar_type->Opcode() == spv::OpTypeInt || scalar_type->Opcode() == spv::OpTypeFloat) ? scalar_type->Word(2) : 32;

            // Locations are 128-bit wide; 3- and 4-component vectors of 64 bit types require two.
            return (bit_width * insn->Word(3) + 127) / 128;
        }
        default:
            // Everything else is just 1.
            return 1;

            // TODO: extend to handle 64bit scalar types, whose vectors may need multiple locations.
    }
}

// Returns the number of Components slots used for a given ID reference to a OpType*
uint32_t SHADER_MODULE_STATE::GetComponentsConsumedByType(uint32_t type, bool strip_array_level) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetComponentsConsumedByType(insn->Word(3), strip_array_level);
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            for (uint32_t i = 2; i < insn->Length(); i++) {  // i=2 to skip Word(0) and Word(1)=ID of struct
                sum += GetComponentsConsumedByType(insn->Word(i), false);
            }
            return sum;
        }
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetComponentsConsumedByType(insn->Word(2), false);
            } else {
                return GetConstantValueById(insn->Word(3)) * GetComponentsConsumedByType(insn->Word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn->Word(3) * GetComponentsConsumedByType(insn->Word(2), false);
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            auto bit_width =
                (scalar_type->Opcode() == spv::OpTypeInt || scalar_type->Opcode() == spv::OpTypeFloat) ? scalar_type->Word(2) : 32;
            // One component is 32-bit
            return (bit_width * insn->Word(3) + 31) / 32;
        }
        case spv::OpTypeFloat: {
            auto bit_width = insn->Word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpTypeInt: {
            auto bit_width = insn->Word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpConstant:
            return GetComponentsConsumedByType(insn->Word(1), false);
        default:
            return 0;
    }
}

// characterizes a SPIR-V type appearing in an interface to a FF stage, for comparison to a VkFormat's characterization above.
// also used for input attachments, as we statically know their format.
uint32_t SHADER_MODULE_STATE::GetFundamentalType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypeInt:
            return insn->Word(3) ? FORMAT_TYPE_SINT : FORMAT_TYPE_UINT;
        case spv::OpTypeFloat:
            return FORMAT_TYPE_FLOAT;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeImage:
            return GetFundamentalType(insn->Word(2));
        case spv::OpTypePointer:
            return GetFundamentalType(insn->Word(3));

        default:
            return 0;
    }
}

const Instruction* SHADER_MODULE_STATE::GetStructType(const Instruction* insn, bool is_array_of_verts) const {
    while (true) {
        if (insn->Opcode() == spv::OpTypePointer) {
            insn = FindDef(insn->Word(3));
        } else if (insn->Opcode() == spv::OpTypeArray && is_array_of_verts) {
            insn = FindDef(insn->Word(2));
        } else if (insn->Opcode() == spv::OpTypeStruct) {
            return insn;
        } else {
            return nullptr;
        }
    }
}

void SHADER_MODULE_STATE::DefineStructMember(const Instruction* insn, std::vector<const Instruction*>& member_decorate_insn,
                                             StructInfo& data) const {
    const Instruction* struct_type = GetStructType(insn, false);
    data.size = 0;

    StructInfo data1;
    uint32_t element_index = 2;  // offset where first element in OpTypeStruct is
    uint32_t local_offset = 0;
    // offsets into struct
    std::vector<uint32_t> offsets;
    offsets.resize(struct_type->Length() - element_index);

    // The members of struct in SPRIV_R aren't always sort, so we need to know their order.
    for (const Instruction* member_decorate : member_decorate_insn) {
        if (member_decorate->Word(1) != struct_type->Word(1)) {
            continue;
        }

        offsets[member_decorate->Word(2)] = member_decorate->Word(4);
    }

    for (const uint32_t offset : offsets) {
        local_offset = offset;
        data1 = {};
        data1.root = data.root;
        data1.offset = local_offset;
        const Instruction* def_member = FindDef(struct_type->Word(element_index));

        // Array could be multi-dimensional
        while (def_member->Opcode() == spv::OpTypeArray) {
            const auto len_id = def_member->Word(3);
            const Instruction* def_len = FindDef(len_id);
            data1.array_length_hierarchy.emplace_back(def_len->Word(3));  // array length
            def_member = FindDef(def_member->Word(2));
        }

        if (def_member->Opcode() == spv::OpTypeStruct) {
            DefineStructMember(def_member, member_decorate_insn, data1);
        } else if (def_member->Opcode() == spv::OpTypePointer) {
            if (def_member->StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
                // If it's a pointer with PhysicalStorageBuffer class, this member is essentially a uint64_t containing an address
                // that "points to something."
                data1.size = 8;
            } else {
                // If it's OpTypePointer. it means the member is a buffer, the type will be TypePointer, and then struct
                DefineStructMember(def_member, member_decorate_insn, data1);
            }
        } else {
            if (def_member->Opcode() == spv::OpTypeMatrix) {
                data1.array_length_hierarchy.emplace_back(def_member->Word(3));  // matrix's columns. matrix's row is vector.
                def_member = FindDef(def_member->Word(2));
            }

            if (def_member->Opcode() == spv::OpTypeVector) {
                data1.array_length_hierarchy.emplace_back(def_member->Word(3));  // vector length
                def_member = FindDef(def_member->Word(2));
            }

            // Get scalar type size. The value in SPRV-R is bit. It needs to translate to byte.
            data1.size = (def_member->Word(2) / 8);
        }
        const auto array_length_hierarchy_szie = data1.array_length_hierarchy.size();
        if (array_length_hierarchy_szie > 0) {
            data1.array_block_size.resize(array_length_hierarchy_szie, 1);

            for (int i2 = static_cast<int>(array_length_hierarchy_szie - 1); i2 > 0; --i2) {
                data1.array_block_size[i2 - 1] = data1.array_length_hierarchy[i2] * data1.array_block_size[i2];
            }
        }
        data.struct_members.emplace_back(data1);
        ++element_index;
    }
    uint32_t total_array_length = 1;
    for (const auto length : data1.array_length_hierarchy) {
        total_array_length *= length;
    }
    data.size = local_offset + data1.size * total_array_length;
}

uint32_t SHADER_MODULE_STATE::UpdateOffset(uint32_t offset, const std::vector<uint32_t>& array_indices,
                                           const StructInfo& data) const {
    int array_indices_size = static_cast<int>(array_indices.size());
    if (array_indices_size) {
        uint32_t array_index = 0;
        uint32_t i = 0;
        for (const auto index : array_indices) {
            array_index += (data.array_block_size[i] * index);
            ++i;
        }
        offset += (array_index * data.size);
    }
    return offset;
}

void SHADER_MODULE_STATE::SetUsedBytes(uint32_t offset, const std::vector<uint32_t>& array_indices, const StructInfo& data) const {
    int array_indices_size = static_cast<int>(array_indices.size());
    uint32_t block_memory_size = data.size;
    for (uint32_t i = static_cast<int>(array_indices_size); i < data.array_length_hierarchy.size(); ++i) {
        block_memory_size *= data.array_length_hierarchy[i];
    }

    offset = UpdateOffset(offset, array_indices, data);

    uint32_t end = offset + block_memory_size;
    auto used_bytes = data.GetUsedbytes();
    if (used_bytes->size() < end) {
        used_bytes->resize(end, 0);
    }
    std::memset(used_bytes->data() + offset, true, static_cast<std::size_t>(block_memory_size));
}

void SHADER_MODULE_STATE::RunUsedArray(uint32_t offset, std::vector<uint32_t> array_indices, uint32_t access_chain_word_index,
                                       const Instruction* access_chain, const StructInfo& data) const {
    if (access_chain_word_index < access_chain->Length()) {
        if (data.array_length_hierarchy.size() > array_indices.size()) {
            const Instruction* def = FindDef(access_chain->Word(access_chain_word_index));
            ++access_chain_word_index;

            if (def && def->Opcode() == spv::OpConstant) {
                array_indices.emplace_back(def->Word(3));
                RunUsedArray(offset, array_indices, access_chain_word_index, access_chain, data);
            } else {
                // If it is a variable, set the all array is used.
                if (access_chain_word_index < access_chain->Length()) {
                    uint32_t array_length = data.array_length_hierarchy[array_indices.size()];
                    for (uint32_t i = 0; i < array_length; ++i) {
                        auto array_indices2 = array_indices;
                        array_indices2.emplace_back(i);
                        RunUsedArray(offset, array_indices2, access_chain_word_index, access_chain, data);
                    }
                } else {
                    SetUsedBytes(offset, array_indices, data);
                }
            }
        } else {
            offset = UpdateOffset(offset, array_indices, data);
            RunUsedStruct(offset, access_chain_word_index, access_chain, data);
        }
    } else {
        SetUsedBytes(offset, array_indices, data);
    }
}

void SHADER_MODULE_STATE::RunUsedStruct(uint32_t offset, uint32_t access_chain_word_index, const Instruction* access_chain,
                                        const StructInfo& data) const {
    std::vector<uint32_t> array_indices_emptry;

    if (access_chain_word_index < access_chain->Length()) {
        auto strcut_member_index = GetConstantValueById(access_chain->Word(access_chain_word_index));
        ++access_chain_word_index;

        auto data1 = data.struct_members[strcut_member_index];
        RunUsedArray(offset + data1.offset, array_indices_emptry, access_chain_word_index, access_chain, data1);
    }
}

void SHADER_MODULE_STATE::SetUsedStructMember(const uint32_t variable_id, vvl::unordered_set<uint32_t>& accessible_ids,
                                              const StructInfo& data) const {
    for (const auto& id : accessible_ids) {
        const Instruction* insn = FindDef(id);
        if (insn->Opcode() == spv::OpAccessChain) {
            if (insn->Word(3) == variable_id) {
                RunUsedStruct(0, 4, insn, data);
            }
        }
    }
}

void SHADER_MODULE_STATE::SetPushConstantUsedInShader(const SHADER_MODULE_STATE& module_state,
                                                      std::vector<SHADER_MODULE_STATE::EntryPoint>& entry_points) {
    for (auto& entrypoint : entry_points) {
        for (const Instruction* var_insn : module_state.GetVariableInstructions()) {
            if (var_insn->StorageClass() == spv::StorageClassPushConstant) {
                const Instruction* type = module_state.FindDef(var_insn->Word(1));
                std::vector<const Instruction*> member_decorate_insn;
                for (const Instruction* member_decorate : module_state.GetMemberDecorationInstructions()) {
                    if (member_decorate->Length() == 5 && member_decorate->Word(3) == spv::DecorationOffset) {
                        member_decorate_insn.emplace_back(member_decorate);
                    }
                }
                entrypoint.push_constant_used_in_shader.root = &entrypoint.push_constant_used_in_shader;
                module_state.DefineStructMember(type, member_decorate_insn, entrypoint.push_constant_used_in_shader);
                module_state.SetUsedStructMember(var_insn->Word(2), entrypoint.accessible_ids,
                                                 entrypoint.push_constant_used_in_shader);
            }
        }
    }
}

uint32_t SHADER_MODULE_STATE::DescriptorTypeToReqs(uint32_t type_id) const {
    const Instruction* type = FindDef(type_id);

    while (true) {
        switch (type->Opcode()) {
            case spv::OpTypeArray:
            case spv::OpTypeRuntimeArray:
            case spv::OpTypeSampledImage:
                type = FindDef(type->Word(2));
                break;
            case spv::OpTypePointer:
                type = FindDef(type->Word(3));
                break;
            case spv::OpTypeImage: {
                auto dim = type->Word(3);
                auto arrayed = type->Word(5);
                auto msaa = type->Word(6);

                uint32_t bits = 0;
                switch (GetFundamentalType(type->Word(2))) {
                    case FORMAT_TYPE_FLOAT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
                        break;
                    case FORMAT_TYPE_UINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
                        break;
                    case FORMAT_TYPE_SINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
                        break;
                    default:
                        break;
                }

                switch (dim) {
                    case spv::Dim1D:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_1D;
                        return bits;
                    case spv::Dim2D:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_2D;
                        return bits;
                    case spv::Dim3D:
                        bits |= DESCRIPTOR_REQ_VIEW_TYPE_3D;
                        return bits;
                    case spv::DimCube:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_CUBE;
                        return bits;
                    case spv::DimSubpassData:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        return bits;
                    default:  // buffer, etc.
                        return bits;
                }
            }
            default:
                return 0;
        }
    }
}

// For some built-in analysis we need to know if the variable decorated with as the built-in was actually written to.
// This function examines instructions in the static call tree for a write to this variable.
bool SHADER_MODULE_STATE::IsBuiltInWritten(const Instruction* builtin_insn, const Instruction& entrypoint) const {
    auto type = builtin_insn->Opcode();
    uint32_t target_id = builtin_insn->Word(1);
    bool init_complete = false;
    uint32_t target_member_offset = 0;

    if (type == spv::OpMemberDecorate) {
        // Built-in is part of a structure -- examine instructions up to first function body to get initial IDs
        for (const Instruction& insn : GetInstructions()) {
            if (insn.Opcode() == spv::OpFunction) {
                break;
            }
            switch (insn.Opcode()) {
                case spv::OpTypePointer:
                    if (insn.StorageClass() == spv::StorageClassOutput) {
                        const auto type_id = insn.Word(3);
                        if (type_id == target_id) {
                            target_id = insn.Word(1);
                        } else {
                            // If the output is an array, check if the element type is what we're looking for
                            const Instruction* type_def = FindDef(type_id);
                            if ((type_def->Opcode() == spv::OpTypeArray) && (type_def->Word(2) == target_id)) {
                                target_id = insn.Word(1);
                                target_member_offset = 1;
                            }
                        }
                    }
                    break;
                case spv::OpVariable:
                    if (insn.Word(1) == target_id) {
                        target_id = insn.Word(2);
                        init_complete = true;
                    }
                    break;
            }
        }
    }

    if (!init_complete && (type == spv::OpMemberDecorate)) return false;

    bool found_write = false;
    vvl::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.Word(2));

    // Follow instructions in call graph looking for writes to target
    while (!worklist.empty() && !found_write) {
        auto id_iter = worklist.begin();
        auto id = *id_iter;
        worklist.erase(id_iter);

        const Instruction* insn = FindDef(id);
        if (!insn) {
            continue;
        }

        if (insn->Opcode() == spv::OpFunction) {
            // Scan body of function looking for other function calls or items in our ID chain
            while (++insn, (insn->Opcode() != spv::OpFunctionEnd) && !found_write) {
                switch (insn->Opcode()) {
                    case spv::OpAccessChain:
                    case spv::OpInBoundsAccessChain:
                        if (insn->Word(3) == target_id) {
                            if (type == spv::OpMemberDecorate) {
                                // Get the target member of the struct
                                // NOTE: this will only work for structs and arrays of structs. Deeper levels of nesting (e.g.,
                                // arrays of structs of structs) is not currently supported.
                                const Instruction* value_def = GetConstantDef(insn->Word(4 + target_member_offset));
                                if (value_def) {
                                    auto value = value_def->GetConstantValue();
                                    if (value == builtin_insn->Word(2)) {
                                        target_id = insn->Word(2);
                                    }
                                }
                            } else {
                                target_id = insn->Word(2);
                            }
                        }
                        break;
                    case spv::OpStore:
                        if (insn->Word(1) == target_id) {
                            found_write = true;
                        }
                        break;
                    case spv::OpFunctionCall:
                        worklist.insert(insn->Word(3));
                        break;
                }
            }
        }
    }
    return found_write;
}

// Returns the id from load_members that matched the object_id, otherwise returns zero
static uint32_t CheckObjectIDFromOpLoad(
    uint32_t object_id, const std::vector<uint32_t>& operator_members,
    const vvl::unordered_map<uint32_t, uint32_t>& load_members,
    const vvl::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>>& accesschain_members) {
    for (auto load_id : operator_members) {
        if (object_id == load_id) return load_id;
        auto load_it = load_members.find(load_id);
        if (load_it == load_members.end()) {
            continue;
        }
        if (load_it->second == object_id) {
            return load_it->first;
        }

        auto accesschain_it = accesschain_members.find(load_it->second);
        if (accesschain_it == accesschain_members.end()) {
            continue;
        }
        if (accesschain_it->second.first == object_id) {
            return accesschain_it->first;
        }
    }
    return 0;
}

ResourceInterfaceVariable::ResourceInterfaceVariable(const SHADER_MODULE_STATE& module_state, const Instruction* insn,
                                                     VkShaderStageFlagBits stage)
    : id(insn->Word(2)),
      type_id(insn->Word(1)),
      storage_class(static_cast<spv::StorageClass>(insn->Word(3))),
      stage(stage),
      decorations(module_state.GetDecorationSet(id)) {
    // Takes a OpVariable and looks at the the descriptor type it uses. This will find things such as if the variable is writable,
    // image atomic operation, matching images to samplers, etc
    const Instruction* type = module_state.FindDef(type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type->Opcode() == spv::OpTypeArray || type->Opcode() == spv::OpTypePointer ||
           type->Opcode() == spv::OpTypeRuntimeArray || type->Opcode() == spv::OpTypeSampledImage) {
        if (type->Opcode() == spv::OpTypeArray || type->Opcode() == spv::OpTypeRuntimeArray ||
            type->Opcode() == spv::OpTypeSampledImage) {
            type = module_state.FindDef(type->Word(2));  // Element type
        } else {
            type = module_state.FindDef(type->Word(3));  // Pointer type
        }
    }

    const auto& static_data_ = module_state.static_data_;
    switch (type->Opcode()) {
        case spv::OpTypeImage: {
            image_sampled_type_width = module_state.GetTypeBitsSize(type);
            auto dim = type->Word(3);
            if (dim != spv::DimSubpassData) {
                // Sampled == 2 indicates used without a sampler (a storage image)
                const bool is_image_without_format = ((type->Word(7) == 2) && (type->Word(8) == spv::ImageFormatUnknown));

                const uint32_t image_write_load_id = CheckObjectIDFromOpLoad(
                    id, static_data_.image_write_load_ids, static_data_.load_members, static_data_.accesschain_members);
                if (image_write_load_id != 0) {
                    is_writable = true;
                    if (is_image_without_format) {
                        is_write_without_format = true;
                        for (const auto& entry : static_data_.image_write_load_id_map) {
                            if (image_write_load_id == entry.second) {
                                const uint32_t texel_component_count = module_state.GetTexelComponentCount(*entry.first);
                                write_without_formats_component_count_list.emplace_back(*entry.first, texel_component_count);
                            }
                        }
                    }
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.image_read_load_ids, static_data_.load_members,
                                            static_data_.accesschain_members) != 0) {
                    is_readable = true;
                    if (is_image_without_format) {
                        is_read_without_format = true;
                    }
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.sampler_load_ids, static_data_.load_members,
                                            static_data_.accesschain_members) != 0) {
                    is_sampler_sampled = true;
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.sampler_implicitLod_dref_proj_load_ids, static_data_.load_members,
                                            static_data_.accesschain_members) != 0) {
                    is_sampler_implicitLod_dref_proj = true;
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.sampler_bias_offset_load_ids, static_data_.load_members,
                                            static_data_.accesschain_members) != 0) {
                    is_sampler_bias_offset = true;
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.atomic_pointer_ids, static_data_.image_texel_pointer_members,
                                            static_data_.accesschain_members) != 0) {
                    is_atomic_operation = true;
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.image_dref_load_ids, static_data_.load_members,
                                            static_data_.accesschain_members) != 0) {
                    is_dref_operation = true;
                }

                for (auto& itp_id : static_data_.sampled_image_load_ids) {
                    // Find if image id match.
                    uint32_t image_index = 0;
                    auto load_it = static_data_.load_members.find(itp_id.first);
                    if (load_it == static_data_.load_members.end()) {
                        continue;
                    } else {
                        if (load_it->second != id) {
                            auto accesschain_it = static_data_.accesschain_members.find(load_it->second);
                            if (accesschain_it == static_data_.accesschain_members.end()) {
                                continue;
                            } else {
                                if (accesschain_it->second.first != id) {
                                    continue;
                                }

                                const Instruction* const_def = module_state.GetConstantDef(accesschain_it->second.second);
                                if (!const_def) {
                                    // access chain index not a constant, skip.
                                    break;
                                }
                                image_index = const_def->GetConstantValue();
                            }
                        }
                    }
                    // Find sampler's set binding.
                    load_it = static_data_.load_members.find(itp_id.second);
                    if (load_it == static_data_.load_members.end()) {
                        continue;
                    } else {
                        uint32_t sampler_id = load_it->second;
                        uint32_t sampler_index = 0;
                        auto accesschain_it = static_data_.accesschain_members.find(load_it->second);

                        if (accesschain_it != static_data_.accesschain_members.end()) {
                            const Instruction* const_def = module_state.GetConstantDef(accesschain_it->second.second);
                            if (!const_def) {
                                // access chain index representing sampler index is not a constant, skip.
                                break;
                            }
                            sampler_id = const_def->Word(const_def->ResultId());
                            sampler_index = const_def->GetConstantValue();
                        }
                        auto sampler_dec = module_state.GetDecorationSet(sampler_id);
                        if (image_index >= samplers_used_by_image.size()) {
                            samplers_used_by_image.resize(image_index + 1);
                        }

                        // Need to check again for these properties in case not using a combined image sampler
                        if (CheckObjectIDFromOpLoad(sampler_id, static_data_.sampler_load_ids, static_data_.load_members,
                                                    static_data_.accesschain_members) != 0) {
                            is_sampler_sampled = true;
                        }
                        if (CheckObjectIDFromOpLoad(sampler_id, static_data_.sampler_implicitLod_dref_proj_load_ids,
                                                    static_data_.load_members, static_data_.accesschain_members) != 0) {
                            is_sampler_implicitLod_dref_proj = true;
                        }
                        if (CheckObjectIDFromOpLoad(sampler_id, static_data_.sampler_bias_offset_load_ids,
                                                    static_data_.load_members, static_data_.accesschain_members) != 0) {
                            is_sampler_bias_offset = true;
                        }

                        samplers_used_by_image[image_index].emplace(
                            SamplerUsedByImage{DescriptorSlot{sampler_dec.set, sampler_dec.binding}, sampler_index});
                    }
                }
            }
            return;
        }

        case spv::OpTypeStruct: {
            vvl::unordered_set<uint32_t> nonwritable_members;
            const bool is_storage_buffer = (storage_class == spv::StorageClassStorageBuffer) ||
                                           (module_state.GetDecorationSet(type->Word(1)).Has(DecorationSet::buffer_block_bit));
            for (const Instruction* insn : static_data_.member_decoration_inst) {
                if (insn->Word(1) == type->Word(1) && insn->Word(3) == spv::DecorationNonWritable) {
                    nonwritable_members.insert(insn->Word(2));
                }
            }

            // A buffer is writable if it's either flavor of storage buffer, and has any member not decorated
            // as nonwritable.
            if (is_storage_buffer && nonwritable_members.size() != type->Length() - 2) {
                for (auto oid : static_data_.store_pointer_ids) {
                    if (id == oid) {
                        is_writable = true;
                        return;
                    }
                    auto accesschain_it = static_data_.accesschain_members.find(oid);
                    if (accesschain_it == static_data_.accesschain_members.end()) {
                        continue;
                    }
                    if (accesschain_it->second.first == id) {
                        is_writable = true;
                        return;
                    }
                }
                if (CheckObjectIDFromOpLoad(id, static_data_.atomic_store_pointer_ids, static_data_.image_texel_pointer_members,
                                            static_data_.accesschain_members) != 0) {
                    is_writable = true;
                    return;
                }
            }
        }
    }
}

vvl::unordered_set<uint32_t> SHADER_MODULE_STATE::CollectWritableOutputLocationinFS(const Instruction& entrypoint) const {
    vvl::unordered_set<uint32_t> location_list;
    const auto outputs = CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, false);
    vvl::unordered_set<uint32_t> store_pointer_ids;
    vvl::unordered_map<uint32_t, uint32_t> accesschain_members;

    for (const Instruction& insn : GetInstructions()) {
        switch (insn.Opcode()) {
            case spv::OpStore:
            case spv::OpAtomicStore: {
                store_pointer_ids.insert(insn.Word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain: {
                // 2: AccessChain id, 3: object id
                if (insn.Word(3)) accesschain_members.emplace(insn.Word(2), insn.Word(3));
                break;
            }
            default:
                break;
        }
    }
    if (store_pointer_ids.empty()) {
        return location_list;
    }
    for (const auto& output : outputs) {
        auto store_it = store_pointer_ids.find(output.second.id);
        if (store_it != store_pointer_ids.end()) {
            location_list.insert(output.first.first);
            store_pointer_ids.erase(store_it);
            continue;
        }
        store_it = store_pointer_ids.begin();
        while (store_it != store_pointer_ids.end()) {
            auto accesschain_it = accesschain_members.find(*store_it);
            if (accesschain_it == accesschain_members.end()) {
                ++store_it;
                continue;
            }
            if (accesschain_it->second == output.second.id) {
                location_list.insert(output.first.first);
                store_pointer_ids.erase(store_it);
                accesschain_members.erase(accesschain_it);
                break;
            }
            ++store_it;
        }
    }
    return location_list;
}

bool SHADER_MODULE_STATE::CollectInterfaceBlockMembers(std::map<location_t, UserDefinedInterfaceVariable>* out,
                                                       bool is_array_of_verts, bool is_patch,
                                                       const Instruction* variable_insn) const {
    // Walk down the type_id presented, trying to determine whether it's actually an interface block.
    const Instruction* struct_type = GetStructType(FindDef(variable_insn->Word(1)), is_array_of_verts && !is_patch);
    if (!struct_type || !(GetDecorationSet(struct_type->Word(1)).Has(DecorationSet::block_bit))) {
        // This isn't an interface block.
        return false;
    }

    vvl::unordered_map<uint32_t, uint32_t> member_components;
    vvl::unordered_map<uint32_t, uint32_t> member_patch;

    // Walk all the OpMemberDecorate for type's result id -- first pass, collect components.
    for (const Instruction* insn : static_data_.member_decoration_inst) {
        if (insn->Word(1) == struct_type->Word(1)) {
            const uint32_t member_index = insn->Word(2);
            const uint32_t decoration = insn->Word(3);

            if (decoration == spv::DecorationComponent) {
                member_components[member_index] = insn->Word(4);
            }

            if (decoration == spv::DecorationPatch) {
                member_patch[member_index] = 1;
            }
        }
    }

    // TODO: correctly handle location assignment from outside

    // Second pass -- produce the output, from Location decorations
    for (const Instruction* insn : static_data_.member_decoration_inst) {
        if (insn->Word(1) == struct_type->Word(1)) {
            const uint32_t member_index = insn->Word(2);
            const uint32_t member_type_id = struct_type->Word(2 + member_index);

            if (insn->Word(3) == spv::DecorationLocation) {
                const uint32_t location = insn->Word(4);
                const uint32_t num_locations = GetLocationsConsumedByType(member_type_id, false);
                const auto component_it = member_components.find(member_index);
                const uint32_t component = component_it == member_components.end() ? 0 : component_it->second;
                const bool member_is_patch = is_patch || member_patch.count(member_index) > 0;

                for (uint32_t offset = 0; offset < num_locations; offset++) {
                    UserDefinedInterfaceVariable variable = {};
                    variable.id = variable_insn->Word(2);
                    // TODO: member index in UserDefinedInterfaceVariable too?
                    variable.type_id = member_type_id;
                    variable.offset = offset;
                    variable.is_patch = member_is_patch;
                    (*out)[std::make_pair(location + offset, component)] = variable;
                }
            }
        }
    }

    return true;
}

std::map<location_t, UserDefinedInterfaceVariable> SHADER_MODULE_STATE::CollectInterfaceByLocation(const Instruction& entrypoint,
                                                                                                   spv::StorageClass sinterface,
                                                                                                   bool is_array_of_verts) const {
    // TODO: handle index=1 dual source outputs from FS -- two vars will have the same location, and we DON'T want to clobber.

    std::map<location_t, UserDefinedInterfaceVariable> out;

    for (uint32_t iid : FindEntrypointInterfaces(entrypoint)) {
        const Instruction* insn = FindDef(iid);
        assert(insn->Opcode() == spv::OpVariable);

        const auto decoration_set = GetDecorationSet(iid);
        const bool passthrough = sinterface == spv::StorageClassOutput && insn->Word(3) == spv::StorageClassInput &&
                                 (decoration_set.Has(DecorationSet::passthrough_bit));
        if (insn->Word(3) == static_cast<uint32_t>(sinterface) || passthrough) {
            const uint32_t builtin = decoration_set.builtin;
            const uint32_t component = decoration_set.component;
            const uint32_t location = decoration_set.location;
            const bool is_patch = decoration_set.Has(DecorationSet::patch_bit);
            const bool is_per_vertex = decoration_set.Has(DecorationSet::per_vertex_bit);
            if (builtin != DecorationSet::kInvalidValue) {
                continue;
            } else if (!CollectInterfaceBlockMembers(&out, is_array_of_verts, is_patch, insn) ||
                       decoration_set.location != DecorationSet::kInvalidValue) {
                // A user-defined interface variable, with a location. Where a variable occupied multiple locations, emit
                // one result for each.
                const uint32_t num_locations = GetLocationsConsumedByType(insn->Word(1), is_array_of_verts || is_per_vertex);
                for (uint32_t offset = 0; offset < num_locations; offset++) {
                    UserDefinedInterfaceVariable variable(insn);
                    variable.offset = offset;
                    variable.is_patch = is_patch;
                    out[std::make_pair(location + offset, component)] = variable;
                }
            }
        }
    }

    return out;
}

std::vector<uint32_t> SHADER_MODULE_STATE::CollectBuiltinBlockMembers(const Instruction& entrypoint, uint32_t storageClass) const {
    // Find all interface variables belonging to the entrypoint and matching the storage class
    std::vector<uint32_t> variables;
    for (uint32_t id : FindEntrypointInterfaces(entrypoint)) {
        const Instruction* def = FindDef(id);
        assert(def->Opcode() == spv::OpVariable);

        if (def->Word(3) == storageClass) variables.push_back(def->Word(1));
    }

    // Find all members belonging to the builtin block selected
    std::vector<uint32_t> builtin_block_members;
    for (auto& var : variables) {
        const Instruction* def = FindDef(FindDef(var)->Word(3));

        // It could be an array of IO blocks. The element type should be the struct defining the block contents
        if (def->Opcode() == spv::OpTypeArray) {
            def = FindDef(def->Word(2));
        }

        // Now find all members belonging to the struct defining the IO block
        if (def->Opcode() == spv::OpTypeStruct) {
            for (const Instruction* insn : GetBuiltinDecorationList()) {
                if ((insn->Opcode() == spv::OpMemberDecorate) && (def->Word(1) == insn->Word(1))) {
                    // Start with undefined builtin for each struct member.
                    // But only when confirmed the struct is the built-in inteface block (can only be one per shader)
                    if (builtin_block_members.size() == 0) {
                        builtin_block_members.resize(def->Length() - 2, spv::BuiltInMax);
                    }
                    auto struct_index = insn->Word(2);
                    assert(struct_index < builtin_block_members.size());
                    builtin_block_members[struct_index] = insn->Word(4);
                }
            }
        }
    }

    return builtin_block_members;
}

uint32_t SHADER_MODULE_STATE::GetNumComponentsInBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t component_count = 0;
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt) {
        component_count = 1;
    } else if (opcode == spv::OpTypeVector) {
        component_count = insn->Word(3);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        // Because we are calculating components for a single location we do not care about column count
        component_count = GetNumComponentsInBaseType(column_type);  // vector length
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        component_count = GetNumComponentsInBaseType(element_type);  // element length
    } else if (opcode == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            component_count += GetNumComponentsInBaseType(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        const Instruction* type = FindDef(insn->Word(3));
        component_count = GetNumComponentsInBaseType(type);
    }
    return component_count;
}

// Returns the total size in 'bits' of any OpType*
uint32_t SHADER_MODULE_STATE::GetTypeBitsSize(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t bit_size = 0;
    if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        uint32_t scalar_width = GetTypeBitsSize(component_type);
        uint32_t component_count = insn->Word(3);
        bit_size = scalar_width * component_count;
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        uint32_t vector_width = GetTypeBitsSize(column_type);
        uint32_t column_count = insn->Word(3);
        bit_size = vector_width * column_count;
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        uint32_t element_width = GetTypeBitsSize(element_type);
        const Instruction* length_type = FindDef(insn->Word(3));
        uint32_t length = length_type->GetConstantValue();
        bit_size = element_width * length;
    } else if (opcode == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            bit_size += GetTypeBitsSize(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        const Instruction* type = FindDef(insn->Word(3));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpVariable) {
        const Instruction* type = FindDef(insn->Word(1));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeImage) {
        const Instruction* type = FindDef(insn->Word(2));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeVoid) {
        // Sampled Type of OpTypeImage can be a void
        bit_size = 0;
    } else {
        bit_size = insn->GetBitWidth();
    }

    return bit_size;
}

// Returns the total size in 'bytes' of any OpType*
uint32_t SHADER_MODULE_STATE::GetTypeBytesSize(const Instruction* insn) const { return GetTypeBitsSize(insn) / 8; }

// Returns the base type (float, int or unsigned int) or struct (can have multiple different base types inside)
// Will return 0 if it can not be determined
uint32_t SHADER_MODULE_STATE::GetBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt || opcode == spv::OpTypeBool || opcode == spv::OpTypeStruct) {
        // point to itself as its the base type (or a struct that needs to be traversed still)
        return insn->Word(1);
    } else if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        return GetBaseType(component_type);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        return GetBaseType(column_type);
    } else if (opcode == spv::OpTypeArray || opcode == spv::OpTypeRuntimeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        return GetBaseType(element_type);
    } else if (opcode == spv::OpTypePointer) {
        const auto& storage_class = insn->StorageClass();
        const Instruction* type = FindDef(insn->Word(3));
        if (storage_class == spv::StorageClassPhysicalStorageBuffer && type->Opcode() == spv::OpTypeStruct) {
            // A physical storage buffer to a struct has a chance to point to itself and can't resolve a baseType
            // GLSL example:
            // layout(buffer_reference) buffer T1 {
            //     T1 b[2];
            // };
            return 0;
        }
        return GetBaseType(type);
    }
    // If we assert here, we are missing a valid base type that must be handled. Without this assert, a return value of 0 will
    // produce a hard bug to track
    assert(false);
    return 0;
}

// Returns type_id if id has type or zero otherwise
uint32_t SHADER_MODULE_STATE::GetTypeId(uint32_t id) const {
    const Instruction* type = FindDef(id);
    return type ? type->Word(type->TypeId()) : 0;
}

// Return zero if nothing is found
uint32_t SHADER_MODULE_STATE::GetTexelComponentCount(const Instruction& insn) const {
    uint32_t texel_component_count = 0;
    switch (insn.Opcode()) {
        case spv::OpImageWrite: {
            const Instruction* texel_def = FindDef(insn.Word(3));
            const Instruction* texel_type = FindDef(texel_def->Word(1));
            texel_component_count = (texel_type->Opcode() == spv::OpTypeVector) ? texel_type->Word(3) : 1;
            break;
        }
        default:
            break;
    }
    return texel_component_count;
}

std::vector<uint32_t> FindEntrypointInterfaces(const Instruction& entrypoint) {
    std::vector<uint32_t> interfaces;
    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out the
    // rest of the word - so we only need to look at the last byte in the word to determine which word contains the terminator.
    uint32_t word = 3;
    while (entrypoint.Word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    for (; word < entrypoint.Length(); word++) {
        interfaces.push_back(entrypoint.Word(word));
    }

    return interfaces;
}
