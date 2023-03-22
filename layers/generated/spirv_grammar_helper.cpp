// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_gramar_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021-2023 The Khronos Group Inc.
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
 * This file is related to anything that is found in the SPIR-V grammar
 * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
 *
 ****************************************************************************/

#include "vk_layer_data.h"
#include "spirv_grammar_helper.h"
#include "state_tracker/shader_instruction.h"

// All information related to each SPIR-V opcode instruction
struct InstructionInfo {
    const char* name;
    bool has_type; // always operand 0 if present
    bool has_result; // always operand 1 if present
    uint32_t memory_scope_position; // operand ID position or zero if not present
    uint32_t execution_scope_position; // operand ID position or zero if not present
    uint32_t image_operands_position; // operand ID position or zero if not present
};

// Static table to replace having many large switch statement functions for looking up each part
// of a given SPIR-V opcode instruction
//
// clang-format off
static const vvl::unordered_map<uint32_t, InstructionInfo> kInstructionTable {
    {spv::OpNop, {"OpNop", false, false, 0, 0, 0}},
    {spv::OpUndef, {"OpUndef", true, true, 0, 0, 0}},
    {spv::OpSourceContinued, {"OpSourceContinued", false, false, 0, 0, 0}},
    {spv::OpSource, {"OpSource", false, false, 0, 0, 0}},
    {spv::OpSourceExtension, {"OpSourceExtension", false, false, 0, 0, 0}},
    {spv::OpName, {"OpName", false, false, 0, 0, 0}},
    {spv::OpMemberName, {"OpMemberName", false, false, 0, 0, 0}},
    {spv::OpString, {"OpString", false, true, 0, 0, 0}},
    {spv::OpLine, {"OpLine", false, false, 0, 0, 0}},
    {spv::OpExtension, {"OpExtension", false, false, 0, 0, 0}},
    {spv::OpExtInstImport, {"OpExtInstImport", false, true, 0, 0, 0}},
    {spv::OpExtInst, {"OpExtInst", true, true, 0, 0, 0}},
    {spv::OpMemoryModel, {"OpMemoryModel", false, false, 0, 0, 0}},
    {spv::OpEntryPoint, {"OpEntryPoint", false, false, 0, 0, 0}},
    {spv::OpExecutionMode, {"OpExecutionMode", false, false, 0, 0, 0}},
    {spv::OpCapability, {"OpCapability", false, false, 0, 0, 0}},
    {spv::OpTypeVoid, {"OpTypeVoid", false, true, 0, 0, 0}},
    {spv::OpTypeBool, {"OpTypeBool", false, true, 0, 0, 0}},
    {spv::OpTypeInt, {"OpTypeInt", false, true, 0, 0, 0}},
    {spv::OpTypeFloat, {"OpTypeFloat", false, true, 0, 0, 0}},
    {spv::OpTypeVector, {"OpTypeVector", false, true, 0, 0, 0}},
    {spv::OpTypeMatrix, {"OpTypeMatrix", false, true, 0, 0, 0}},
    {spv::OpTypeImage, {"OpTypeImage", false, true, 0, 0, 0}},
    {spv::OpTypeSampler, {"OpTypeSampler", false, true, 0, 0, 0}},
    {spv::OpTypeSampledImage, {"OpTypeSampledImage", false, true, 0, 0, 0}},
    {spv::OpTypeArray, {"OpTypeArray", false, true, 0, 0, 0}},
    {spv::OpTypeRuntimeArray, {"OpTypeRuntimeArray", false, true, 0, 0, 0}},
    {spv::OpTypeStruct, {"OpTypeStruct", false, true, 0, 0, 0}},
    {spv::OpTypePointer, {"OpTypePointer", false, true, 0, 0, 0}},
    {spv::OpTypeFunction, {"OpTypeFunction", false, true, 0, 0, 0}},
    {spv::OpTypeForwardPointer, {"OpTypeForwardPointer", false, false, 0, 0, 0}},
    {spv::OpConstantTrue, {"OpConstantTrue", true, true, 0, 0, 0}},
    {spv::OpConstantFalse, {"OpConstantFalse", true, true, 0, 0, 0}},
    {spv::OpConstant, {"OpConstant", true, true, 0, 0, 0}},
    {spv::OpConstantComposite, {"OpConstantComposite", true, true, 0, 0, 0}},
    {spv::OpConstantNull, {"OpConstantNull", true, true, 0, 0, 0}},
    {spv::OpSpecConstantTrue, {"OpSpecConstantTrue", true, true, 0, 0, 0}},
    {spv::OpSpecConstantFalse, {"OpSpecConstantFalse", true, true, 0, 0, 0}},
    {spv::OpSpecConstant, {"OpSpecConstant", true, true, 0, 0, 0}},
    {spv::OpSpecConstantComposite, {"OpSpecConstantComposite", true, true, 0, 0, 0}},
    {spv::OpSpecConstantOp, {"OpSpecConstantOp", true, true, 0, 0, 0}},
    {spv::OpFunction, {"OpFunction", true, true, 0, 0, 0}},
    {spv::OpFunctionParameter, {"OpFunctionParameter", true, true, 0, 0, 0}},
    {spv::OpFunctionEnd, {"OpFunctionEnd", false, false, 0, 0, 0}},
    {spv::OpFunctionCall, {"OpFunctionCall", true, true, 0, 0, 0}},
    {spv::OpVariable, {"OpVariable", true, true, 0, 0, 0}},
    {spv::OpImageTexelPointer, {"OpImageTexelPointer", true, true, 0, 0, 0}},
    {spv::OpLoad, {"OpLoad", true, true, 0, 0, 0}},
    {spv::OpStore, {"OpStore", false, false, 0, 0, 0}},
    {spv::OpCopyMemory, {"OpCopyMemory", false, false, 0, 0, 0}},
    {spv::OpCopyMemorySized, {"OpCopyMemorySized", false, false, 0, 0, 0}},
    {spv::OpAccessChain, {"OpAccessChain", true, true, 0, 0, 0}},
    {spv::OpInBoundsAccessChain, {"OpInBoundsAccessChain", true, true, 0, 0, 0}},
    {spv::OpPtrAccessChain, {"OpPtrAccessChain", true, true, 0, 0, 0}},
    {spv::OpArrayLength, {"OpArrayLength", true, true, 0, 0, 0}},
    {spv::OpInBoundsPtrAccessChain, {"OpInBoundsPtrAccessChain", true, true, 0, 0, 0}},
    {spv::OpDecorate, {"OpDecorate", false, false, 0, 0, 0}},
    {spv::OpMemberDecorate, {"OpMemberDecorate", false, false, 0, 0, 0}},
    {spv::OpDecorationGroup, {"OpDecorationGroup", false, true, 0, 0, 0}},
    {spv::OpGroupDecorate, {"OpGroupDecorate", false, false, 0, 0, 0}},
    {spv::OpGroupMemberDecorate, {"OpGroupMemberDecorate", false, false, 0, 0, 0}},
    {spv::OpVectorExtractDynamic, {"OpVectorExtractDynamic", true, true, 0, 0, 0}},
    {spv::OpVectorInsertDynamic, {"OpVectorInsertDynamic", true, true, 0, 0, 0}},
    {spv::OpVectorShuffle, {"OpVectorShuffle", true, true, 0, 0, 0}},
    {spv::OpCompositeConstruct, {"OpCompositeConstruct", true, true, 0, 0, 0}},
    {spv::OpCompositeExtract, {"OpCompositeExtract", true, true, 0, 0, 0}},
    {spv::OpCompositeInsert, {"OpCompositeInsert", true, true, 0, 0, 0}},
    {spv::OpCopyObject, {"OpCopyObject", true, true, 0, 0, 0}},
    {spv::OpTranspose, {"OpTranspose", true, true, 0, 0, 0}},
    {spv::OpSampledImage, {"OpSampledImage", true, true, 0, 0, 0}},
    {spv::OpImageSampleImplicitLod, {"OpImageSampleImplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSampleExplicitLod, {"OpImageSampleExplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSampleDrefImplicitLod, {"OpImageSampleDrefImplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSampleDrefExplicitLod, {"OpImageSampleDrefExplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSampleProjImplicitLod, {"OpImageSampleProjImplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSampleProjExplicitLod, {"OpImageSampleProjExplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSampleProjDrefImplicitLod, {"OpImageSampleProjDrefImplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSampleProjDrefExplicitLod, {"OpImageSampleProjDrefExplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageFetch, {"OpImageFetch", true, true, 0, 0, 5}},
    {spv::OpImageGather, {"OpImageGather", true, true, 0, 0, 6}},
    {spv::OpImageDrefGather, {"OpImageDrefGather", true, true, 0, 0, 6}},
    {spv::OpImageRead, {"OpImageRead", true, true, 0, 0, 5}},
    {spv::OpImageWrite, {"OpImageWrite", false, false, 0, 0, 4}},
    {spv::OpImage, {"OpImage", true, true, 0, 0, 0}},
    {spv::OpImageQuerySizeLod, {"OpImageQuerySizeLod", true, true, 0, 0, 0}},
    {spv::OpImageQuerySize, {"OpImageQuerySize", true, true, 0, 0, 0}},
    {spv::OpImageQueryLod, {"OpImageQueryLod", true, true, 0, 0, 0}},
    {spv::OpImageQueryLevels, {"OpImageQueryLevels", true, true, 0, 0, 0}},
    {spv::OpImageQuerySamples, {"OpImageQuerySamples", true, true, 0, 0, 0}},
    {spv::OpConvertFToU, {"OpConvertFToU", true, true, 0, 0, 0}},
    {spv::OpConvertFToS, {"OpConvertFToS", true, true, 0, 0, 0}},
    {spv::OpConvertSToF, {"OpConvertSToF", true, true, 0, 0, 0}},
    {spv::OpConvertUToF, {"OpConvertUToF", true, true, 0, 0, 0}},
    {spv::OpUConvert, {"OpUConvert", true, true, 0, 0, 0}},
    {spv::OpSConvert, {"OpSConvert", true, true, 0, 0, 0}},
    {spv::OpFConvert, {"OpFConvert", true, true, 0, 0, 0}},
    {spv::OpQuantizeToF16, {"OpQuantizeToF16", true, true, 0, 0, 0}},
    {spv::OpConvertPtrToU, {"OpConvertPtrToU", true, true, 0, 0, 0}},
    {spv::OpConvertUToPtr, {"OpConvertUToPtr", true, true, 0, 0, 0}},
    {spv::OpBitcast, {"OpBitcast", true, true, 0, 0, 0}},
    {spv::OpSNegate, {"OpSNegate", true, true, 0, 0, 0}},
    {spv::OpFNegate, {"OpFNegate", true, true, 0, 0, 0}},
    {spv::OpIAdd, {"OpIAdd", true, true, 0, 0, 0}},
    {spv::OpFAdd, {"OpFAdd", true, true, 0, 0, 0}},
    {spv::OpISub, {"OpISub", true, true, 0, 0, 0}},
    {spv::OpFSub, {"OpFSub", true, true, 0, 0, 0}},
    {spv::OpIMul, {"OpIMul", true, true, 0, 0, 0}},
    {spv::OpFMul, {"OpFMul", true, true, 0, 0, 0}},
    {spv::OpUDiv, {"OpUDiv", true, true, 0, 0, 0}},
    {spv::OpSDiv, {"OpSDiv", true, true, 0, 0, 0}},
    {spv::OpFDiv, {"OpFDiv", true, true, 0, 0, 0}},
    {spv::OpUMod, {"OpUMod", true, true, 0, 0, 0}},
    {spv::OpSRem, {"OpSRem", true, true, 0, 0, 0}},
    {spv::OpSMod, {"OpSMod", true, true, 0, 0, 0}},
    {spv::OpFRem, {"OpFRem", true, true, 0, 0, 0}},
    {spv::OpFMod, {"OpFMod", true, true, 0, 0, 0}},
    {spv::OpVectorTimesScalar, {"OpVectorTimesScalar", true, true, 0, 0, 0}},
    {spv::OpMatrixTimesScalar, {"OpMatrixTimesScalar", true, true, 0, 0, 0}},
    {spv::OpVectorTimesMatrix, {"OpVectorTimesMatrix", true, true, 0, 0, 0}},
    {spv::OpMatrixTimesVector, {"OpMatrixTimesVector", true, true, 0, 0, 0}},
    {spv::OpMatrixTimesMatrix, {"OpMatrixTimesMatrix", true, true, 0, 0, 0}},
    {spv::OpOuterProduct, {"OpOuterProduct", true, true, 0, 0, 0}},
    {spv::OpDot, {"OpDot", true, true, 0, 0, 0}},
    {spv::OpIAddCarry, {"OpIAddCarry", true, true, 0, 0, 0}},
    {spv::OpISubBorrow, {"OpISubBorrow", true, true, 0, 0, 0}},
    {spv::OpUMulExtended, {"OpUMulExtended", true, true, 0, 0, 0}},
    {spv::OpSMulExtended, {"OpSMulExtended", true, true, 0, 0, 0}},
    {spv::OpAny, {"OpAny", true, true, 0, 0, 0}},
    {spv::OpAll, {"OpAll", true, true, 0, 0, 0}},
    {spv::OpIsNan, {"OpIsNan", true, true, 0, 0, 0}},
    {spv::OpIsInf, {"OpIsInf", true, true, 0, 0, 0}},
    {spv::OpLogicalEqual, {"OpLogicalEqual", true, true, 0, 0, 0}},
    {spv::OpLogicalNotEqual, {"OpLogicalNotEqual", true, true, 0, 0, 0}},
    {spv::OpLogicalOr, {"OpLogicalOr", true, true, 0, 0, 0}},
    {spv::OpLogicalAnd, {"OpLogicalAnd", true, true, 0, 0, 0}},
    {spv::OpLogicalNot, {"OpLogicalNot", true, true, 0, 0, 0}},
    {spv::OpSelect, {"OpSelect", true, true, 0, 0, 0}},
    {spv::OpIEqual, {"OpIEqual", true, true, 0, 0, 0}},
    {spv::OpINotEqual, {"OpINotEqual", true, true, 0, 0, 0}},
    {spv::OpUGreaterThan, {"OpUGreaterThan", true, true, 0, 0, 0}},
    {spv::OpSGreaterThan, {"OpSGreaterThan", true, true, 0, 0, 0}},
    {spv::OpUGreaterThanEqual, {"OpUGreaterThanEqual", true, true, 0, 0, 0}},
    {spv::OpSGreaterThanEqual, {"OpSGreaterThanEqual", true, true, 0, 0, 0}},
    {spv::OpULessThan, {"OpULessThan", true, true, 0, 0, 0}},
    {spv::OpSLessThan, {"OpSLessThan", true, true, 0, 0, 0}},
    {spv::OpULessThanEqual, {"OpULessThanEqual", true, true, 0, 0, 0}},
    {spv::OpSLessThanEqual, {"OpSLessThanEqual", true, true, 0, 0, 0}},
    {spv::OpFOrdEqual, {"OpFOrdEqual", true, true, 0, 0, 0}},
    {spv::OpFUnordEqual, {"OpFUnordEqual", true, true, 0, 0, 0}},
    {spv::OpFOrdNotEqual, {"OpFOrdNotEqual", true, true, 0, 0, 0}},
    {spv::OpFUnordNotEqual, {"OpFUnordNotEqual", true, true, 0, 0, 0}},
    {spv::OpFOrdLessThan, {"OpFOrdLessThan", true, true, 0, 0, 0}},
    {spv::OpFUnordLessThan, {"OpFUnordLessThan", true, true, 0, 0, 0}},
    {spv::OpFOrdGreaterThan, {"OpFOrdGreaterThan", true, true, 0, 0, 0}},
    {spv::OpFUnordGreaterThan, {"OpFUnordGreaterThan", true, true, 0, 0, 0}},
    {spv::OpFOrdLessThanEqual, {"OpFOrdLessThanEqual", true, true, 0, 0, 0}},
    {spv::OpFUnordLessThanEqual, {"OpFUnordLessThanEqual", true, true, 0, 0, 0}},
    {spv::OpFOrdGreaterThanEqual, {"OpFOrdGreaterThanEqual", true, true, 0, 0, 0}},
    {spv::OpFUnordGreaterThanEqual, {"OpFUnordGreaterThanEqual", true, true, 0, 0, 0}},
    {spv::OpShiftRightLogical, {"OpShiftRightLogical", true, true, 0, 0, 0}},
    {spv::OpShiftRightArithmetic, {"OpShiftRightArithmetic", true, true, 0, 0, 0}},
    {spv::OpShiftLeftLogical, {"OpShiftLeftLogical", true, true, 0, 0, 0}},
    {spv::OpBitwiseOr, {"OpBitwiseOr", true, true, 0, 0, 0}},
    {spv::OpBitwiseXor, {"OpBitwiseXor", true, true, 0, 0, 0}},
    {spv::OpBitwiseAnd, {"OpBitwiseAnd", true, true, 0, 0, 0}},
    {spv::OpNot, {"OpNot", true, true, 0, 0, 0}},
    {spv::OpBitFieldInsert, {"OpBitFieldInsert", true, true, 0, 0, 0}},
    {spv::OpBitFieldSExtract, {"OpBitFieldSExtract", true, true, 0, 0, 0}},
    {spv::OpBitFieldUExtract, {"OpBitFieldUExtract", true, true, 0, 0, 0}},
    {spv::OpBitReverse, {"OpBitReverse", true, true, 0, 0, 0}},
    {spv::OpBitCount, {"OpBitCount", true, true, 0, 0, 0}},
    {spv::OpDPdx, {"OpDPdx", true, true, 0, 0, 0}},
    {spv::OpDPdy, {"OpDPdy", true, true, 0, 0, 0}},
    {spv::OpFwidth, {"OpFwidth", true, true, 0, 0, 0}},
    {spv::OpDPdxFine, {"OpDPdxFine", true, true, 0, 0, 0}},
    {spv::OpDPdyFine, {"OpDPdyFine", true, true, 0, 0, 0}},
    {spv::OpFwidthFine, {"OpFwidthFine", true, true, 0, 0, 0}},
    {spv::OpDPdxCoarse, {"OpDPdxCoarse", true, true, 0, 0, 0}},
    {spv::OpDPdyCoarse, {"OpDPdyCoarse", true, true, 0, 0, 0}},
    {spv::OpFwidthCoarse, {"OpFwidthCoarse", true, true, 0, 0, 0}},
    {spv::OpEmitVertex, {"OpEmitVertex", false, false, 0, 0, 0}},
    {spv::OpEndPrimitive, {"OpEndPrimitive", false, false, 0, 0, 0}},
    {spv::OpEmitStreamVertex, {"OpEmitStreamVertex", false, false, 0, 0, 0}},
    {spv::OpEndStreamPrimitive, {"OpEndStreamPrimitive", false, false, 0, 0, 0}},
    {spv::OpControlBarrier, {"OpControlBarrier", false, false, 2, 1, 0}},
    {spv::OpMemoryBarrier, {"OpMemoryBarrier", false, false, 1, 0, 0}},
    {spv::OpAtomicLoad, {"OpAtomicLoad", true, true, 4, 0, 0}},
    {spv::OpAtomicStore, {"OpAtomicStore", false, false, 2, 0, 0}},
    {spv::OpAtomicExchange, {"OpAtomicExchange", true, true, 4, 0, 0}},
    {spv::OpAtomicCompareExchange, {"OpAtomicCompareExchange", true, true, 4, 0, 0}},
    {spv::OpAtomicIIncrement, {"OpAtomicIIncrement", true, true, 4, 0, 0}},
    {spv::OpAtomicIDecrement, {"OpAtomicIDecrement", true, true, 4, 0, 0}},
    {spv::OpAtomicIAdd, {"OpAtomicIAdd", true, true, 4, 0, 0}},
    {spv::OpAtomicISub, {"OpAtomicISub", true, true, 4, 0, 0}},
    {spv::OpAtomicSMin, {"OpAtomicSMin", true, true, 4, 0, 0}},
    {spv::OpAtomicUMin, {"OpAtomicUMin", true, true, 4, 0, 0}},
    {spv::OpAtomicSMax, {"OpAtomicSMax", true, true, 4, 0, 0}},
    {spv::OpAtomicUMax, {"OpAtomicUMax", true, true, 4, 0, 0}},
    {spv::OpAtomicAnd, {"OpAtomicAnd", true, true, 4, 0, 0}},
    {spv::OpAtomicOr, {"OpAtomicOr", true, true, 4, 0, 0}},
    {spv::OpAtomicXor, {"OpAtomicXor", true, true, 4, 0, 0}},
    {spv::OpPhi, {"OpPhi", true, true, 0, 0, 0}},
    {spv::OpLoopMerge, {"OpLoopMerge", false, false, 0, 0, 0}},
    {spv::OpSelectionMerge, {"OpSelectionMerge", false, false, 0, 0, 0}},
    {spv::OpLabel, {"OpLabel", false, true, 0, 0, 0}},
    {spv::OpBranch, {"OpBranch", false, false, 0, 0, 0}},
    {spv::OpBranchConditional, {"OpBranchConditional", false, false, 0, 0, 0}},
    {spv::OpSwitch, {"OpSwitch", false, false, 0, 0, 0}},
    {spv::OpKill, {"OpKill", false, false, 0, 0, 0}},
    {spv::OpReturn, {"OpReturn", false, false, 0, 0, 0}},
    {spv::OpReturnValue, {"OpReturnValue", false, false, 0, 0, 0}},
    {spv::OpUnreachable, {"OpUnreachable", false, false, 0, 0, 0}},
    {spv::OpGroupAll, {"OpGroupAll", true, true, 0, 3, 0}},
    {spv::OpGroupAny, {"OpGroupAny", true, true, 0, 3, 0}},
    {spv::OpGroupBroadcast, {"OpGroupBroadcast", true, true, 0, 3, 0}},
    {spv::OpGroupIAdd, {"OpGroupIAdd", true, true, 0, 3, 0}},
    {spv::OpGroupFAdd, {"OpGroupFAdd", true, true, 0, 3, 0}},
    {spv::OpGroupFMin, {"OpGroupFMin", true, true, 0, 3, 0}},
    {spv::OpGroupUMin, {"OpGroupUMin", true, true, 0, 3, 0}},
    {spv::OpGroupSMin, {"OpGroupSMin", true, true, 0, 3, 0}},
    {spv::OpGroupFMax, {"OpGroupFMax", true, true, 0, 3, 0}},
    {spv::OpGroupUMax, {"OpGroupUMax", true, true, 0, 3, 0}},
    {spv::OpGroupSMax, {"OpGroupSMax", true, true, 0, 3, 0}},
    {spv::OpImageSparseSampleImplicitLod, {"OpImageSparseSampleImplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSparseSampleExplicitLod, {"OpImageSparseSampleExplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSparseSampleDrefImplicitLod, {"OpImageSparseSampleDrefImplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSparseSampleDrefExplicitLod, {"OpImageSparseSampleDrefExplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSparseSampleProjImplicitLod, {"OpImageSparseSampleProjImplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSparseSampleProjExplicitLod, {"OpImageSparseSampleProjExplicitLod", true, true, 0, 0, 5}},
    {spv::OpImageSparseSampleProjDrefImplicitLod, {"OpImageSparseSampleProjDrefImplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSparseSampleProjDrefExplicitLod, {"OpImageSparseSampleProjDrefExplicitLod", true, true, 0, 0, 6}},
    {spv::OpImageSparseFetch, {"OpImageSparseFetch", true, true, 0, 0, 5}},
    {spv::OpImageSparseGather, {"OpImageSparseGather", true, true, 0, 0, 6}},
    {spv::OpImageSparseDrefGather, {"OpImageSparseDrefGather", true, true, 0, 0, 6}},
    {spv::OpImageSparseTexelsResident, {"OpImageSparseTexelsResident", true, true, 0, 0, 0}},
    {spv::OpNoLine, {"OpNoLine", false, false, 0, 0, 0}},
    {spv::OpImageSparseRead, {"OpImageSparseRead", true, true, 0, 0, 5}},
    {spv::OpSizeOf, {"OpSizeOf", true, true, 0, 0, 0}},
    {spv::OpTypePipeStorage, {"OpTypePipeStorage", false, true, 0, 0, 0}},
    {spv::OpConstantPipeStorage, {"OpConstantPipeStorage", true, true, 0, 0, 0}},
    {spv::OpCreatePipeFromPipeStorage, {"OpCreatePipeFromPipeStorage", true, true, 0, 0, 0}},
    {spv::OpGetKernelLocalSizeForSubgroupCount, {"OpGetKernelLocalSizeForSubgroupCount", true, true, 0, 0, 0}},
    {spv::OpGetKernelMaxNumSubgroups, {"OpGetKernelMaxNumSubgroups", true, true, 0, 0, 0}},
    {spv::OpModuleProcessed, {"OpModuleProcessed", false, false, 0, 0, 0}},
    {spv::OpExecutionModeId, {"OpExecutionModeId", false, false, 0, 0, 0}},
    {spv::OpDecorateId, {"OpDecorateId", false, false, 0, 0, 0}},
    {spv::OpGroupNonUniformElect, {"OpGroupNonUniformElect", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformAll, {"OpGroupNonUniformAll", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformAny, {"OpGroupNonUniformAny", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformAllEqual, {"OpGroupNonUniformAllEqual", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBroadcast, {"OpGroupNonUniformBroadcast", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBroadcastFirst, {"OpGroupNonUniformBroadcastFirst", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBallot, {"OpGroupNonUniformBallot", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformInverseBallot, {"OpGroupNonUniformInverseBallot", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBallotBitExtract, {"OpGroupNonUniformBallotBitExtract", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBallotBitCount, {"OpGroupNonUniformBallotBitCount", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBallotFindLSB, {"OpGroupNonUniformBallotFindLSB", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBallotFindMSB, {"OpGroupNonUniformBallotFindMSB", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformShuffle, {"OpGroupNonUniformShuffle", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformShuffleXor, {"OpGroupNonUniformShuffleXor", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformShuffleUp, {"OpGroupNonUniformShuffleUp", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformShuffleDown, {"OpGroupNonUniformShuffleDown", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformIAdd, {"OpGroupNonUniformIAdd", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformFAdd, {"OpGroupNonUniformFAdd", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformIMul, {"OpGroupNonUniformIMul", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformFMul, {"OpGroupNonUniformFMul", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformSMin, {"OpGroupNonUniformSMin", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformUMin, {"OpGroupNonUniformUMin", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformFMin, {"OpGroupNonUniformFMin", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformSMax, {"OpGroupNonUniformSMax", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformUMax, {"OpGroupNonUniformUMax", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformFMax, {"OpGroupNonUniformFMax", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBitwiseAnd, {"OpGroupNonUniformBitwiseAnd", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBitwiseOr, {"OpGroupNonUniformBitwiseOr", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformBitwiseXor, {"OpGroupNonUniformBitwiseXor", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformLogicalAnd, {"OpGroupNonUniformLogicalAnd", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformLogicalOr, {"OpGroupNonUniformLogicalOr", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformLogicalXor, {"OpGroupNonUniformLogicalXor", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformQuadBroadcast, {"OpGroupNonUniformQuadBroadcast", true, true, 0, 3, 0}},
    {spv::OpGroupNonUniformQuadSwap, {"OpGroupNonUniformQuadSwap", true, true, 0, 3, 0}},
    {spv::OpCopyLogical, {"OpCopyLogical", true, true, 0, 0, 0}},
    {spv::OpPtrEqual, {"OpPtrEqual", true, true, 0, 0, 0}},
    {spv::OpPtrNotEqual, {"OpPtrNotEqual", true, true, 0, 0, 0}},
    {spv::OpPtrDiff, {"OpPtrDiff", true, true, 0, 0, 0}},
    {spv::OpTerminateInvocation, {"OpTerminateInvocation", false, false, 0, 0, 0}},
    {spv::OpSubgroupBallotKHR, {"OpSubgroupBallotKHR", true, true, 0, 0, 0}},
    {spv::OpSubgroupFirstInvocationKHR, {"OpSubgroupFirstInvocationKHR", true, true, 0, 0, 0}},
    {spv::OpSubgroupAllKHR, {"OpSubgroupAllKHR", true, true, 0, 0, 0}},
    {spv::OpSubgroupAnyKHR, {"OpSubgroupAnyKHR", true, true, 0, 0, 0}},
    {spv::OpSubgroupAllEqualKHR, {"OpSubgroupAllEqualKHR", true, true, 0, 0, 0}},
    {spv::OpGroupNonUniformRotateKHR, {"OpGroupNonUniformRotateKHR", true, true, 0, 3, 0}},
    {spv::OpSubgroupReadInvocationKHR, {"OpSubgroupReadInvocationKHR", true, true, 0, 0, 0}},
    {spv::OpTraceRayKHR, {"OpTraceRayKHR", false, false, 0, 0, 0}},
    {spv::OpExecuteCallableKHR, {"OpExecuteCallableKHR", false, false, 0, 0, 0}},
    {spv::OpConvertUToAccelerationStructureKHR, {"OpConvertUToAccelerationStructureKHR", true, true, 0, 0, 0}},
    {spv::OpIgnoreIntersectionKHR, {"OpIgnoreIntersectionKHR", false, false, 0, 0, 0}},
    {spv::OpTerminateRayKHR, {"OpTerminateRayKHR", false, false, 0, 0, 0}},
    {spv::OpSDotKHR, {"OpSDotKHR", true, true, 0, 0, 0}},
    {spv::OpUDotKHR, {"OpUDotKHR", true, true, 0, 0, 0}},
    {spv::OpSUDotKHR, {"OpSUDotKHR", true, true, 0, 0, 0}},
    {spv::OpSDotAccSatKHR, {"OpSDotAccSatKHR", true, true, 0, 0, 0}},
    {spv::OpUDotAccSatKHR, {"OpUDotAccSatKHR", true, true, 0, 0, 0}},
    {spv::OpSUDotAccSatKHR, {"OpSUDotAccSatKHR", true, true, 0, 0, 0}},
    {spv::OpTypeRayQueryKHR, {"OpTypeRayQueryKHR", false, true, 0, 0, 0}},
    {spv::OpRayQueryInitializeKHR, {"OpRayQueryInitializeKHR", false, false, 0, 0, 0}},
    {spv::OpRayQueryTerminateKHR, {"OpRayQueryTerminateKHR", false, false, 0, 0, 0}},
    {spv::OpRayQueryGenerateIntersectionKHR, {"OpRayQueryGenerateIntersectionKHR", false, false, 0, 0, 0}},
    {spv::OpRayQueryConfirmIntersectionKHR, {"OpRayQueryConfirmIntersectionKHR", false, false, 0, 0, 0}},
    {spv::OpRayQueryProceedKHR, {"OpRayQueryProceedKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionTypeKHR, {"OpRayQueryGetIntersectionTypeKHR", true, true, 0, 0, 0}},
    {spv::OpGroupIAddNonUniformAMD, {"OpGroupIAddNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupFAddNonUniformAMD, {"OpGroupFAddNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupFMinNonUniformAMD, {"OpGroupFMinNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupUMinNonUniformAMD, {"OpGroupUMinNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupSMinNonUniformAMD, {"OpGroupSMinNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupFMaxNonUniformAMD, {"OpGroupFMaxNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupUMaxNonUniformAMD, {"OpGroupUMaxNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpGroupSMaxNonUniformAMD, {"OpGroupSMaxNonUniformAMD", true, true, 0, 3, 0}},
    {spv::OpFragmentMaskFetchAMD, {"OpFragmentMaskFetchAMD", true, true, 0, 0, 0}},
    {spv::OpFragmentFetchAMD, {"OpFragmentFetchAMD", true, true, 0, 0, 0}},
    {spv::OpReadClockKHR, {"OpReadClockKHR", true, true, 0, 3, 0}},
    {spv::OpHitObjectRecordHitMotionNV, {"OpHitObjectRecordHitMotionNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectRecordHitWithIndexMotionNV, {"OpHitObjectRecordHitWithIndexMotionNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectRecordMissMotionNV, {"OpHitObjectRecordMissMotionNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectGetWorldToObjectNV, {"OpHitObjectGetWorldToObjectNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetObjectToWorldNV, {"OpHitObjectGetObjectToWorldNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetObjectRayDirectionNV, {"OpHitObjectGetObjectRayDirectionNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetObjectRayOriginNV, {"OpHitObjectGetObjectRayOriginNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectTraceRayMotionNV, {"OpHitObjectTraceRayMotionNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectGetShaderRecordBufferHandleNV, {"OpHitObjectGetShaderRecordBufferHandleNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetShaderBindingTableRecordIndexNV, {"OpHitObjectGetShaderBindingTableRecordIndexNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectRecordEmptyNV, {"OpHitObjectRecordEmptyNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectTraceRayNV, {"OpHitObjectTraceRayNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectRecordHitNV, {"OpHitObjectRecordHitNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectRecordHitWithIndexNV, {"OpHitObjectRecordHitWithIndexNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectRecordMissNV, {"OpHitObjectRecordMissNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectExecuteShaderNV, {"OpHitObjectExecuteShaderNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectGetCurrentTimeNV, {"OpHitObjectGetCurrentTimeNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetAttributesNV, {"OpHitObjectGetAttributesNV", false, false, 0, 0, 0}},
    {spv::OpHitObjectGetHitKindNV, {"OpHitObjectGetHitKindNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetPrimitiveIndexNV, {"OpHitObjectGetPrimitiveIndexNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetGeometryIndexNV, {"OpHitObjectGetGeometryIndexNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetInstanceIdNV, {"OpHitObjectGetInstanceIdNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetInstanceCustomIndexNV, {"OpHitObjectGetInstanceCustomIndexNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetWorldRayDirectionNV, {"OpHitObjectGetWorldRayDirectionNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetWorldRayOriginNV, {"OpHitObjectGetWorldRayOriginNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetRayTMaxNV, {"OpHitObjectGetRayTMaxNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectGetRayTMinNV, {"OpHitObjectGetRayTMinNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectIsEmptyNV, {"OpHitObjectIsEmptyNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectIsHitNV, {"OpHitObjectIsHitNV", true, true, 0, 0, 0}},
    {spv::OpHitObjectIsMissNV, {"OpHitObjectIsMissNV", true, true, 0, 0, 0}},
    {spv::OpReorderThreadWithHitObjectNV, {"OpReorderThreadWithHitObjectNV", false, false, 0, 0, 0}},
    {spv::OpReorderThreadWithHintNV, {"OpReorderThreadWithHintNV", false, false, 0, 0, 0}},
    {spv::OpTypeHitObjectNV, {"OpTypeHitObjectNV", false, true, 0, 0, 0}},
    {spv::OpImageSampleFootprintNV, {"OpImageSampleFootprintNV", true, true, 0, 0, 7}},
    {spv::OpEmitMeshTasksEXT, {"OpEmitMeshTasksEXT", false, false, 0, 0, 0}},
    {spv::OpSetMeshOutputsEXT, {"OpSetMeshOutputsEXT", false, false, 0, 0, 0}},
    {spv::OpGroupNonUniformPartitionNV, {"OpGroupNonUniformPartitionNV", true, true, 0, 0, 0}},
    {spv::OpWritePackedPrimitiveIndices4x8NV, {"OpWritePackedPrimitiveIndices4x8NV", false, false, 0, 0, 0}},
    {spv::OpReportIntersectionKHR, {"OpReportIntersectionKHR", true, true, 0, 0, 0}},
    {spv::OpIgnoreIntersectionNV, {"OpIgnoreIntersectionNV", false, false, 0, 0, 0}},
    {spv::OpTerminateRayNV, {"OpTerminateRayNV", false, false, 0, 0, 0}},
    {spv::OpTraceNV, {"OpTraceNV", false, false, 0, 0, 0}},
    {spv::OpTraceMotionNV, {"OpTraceMotionNV", false, false, 0, 0, 0}},
    {spv::OpTraceRayMotionNV, {"OpTraceRayMotionNV", false, false, 0, 0, 0}},
    {spv::OpTypeAccelerationStructureKHR, {"OpTypeAccelerationStructureKHR", false, true, 0, 0, 0}},
    {spv::OpExecuteCallableNV, {"OpExecuteCallableNV", false, false, 0, 0, 0}},
    {spv::OpTypeCooperativeMatrixNV, {"OpTypeCooperativeMatrixNV", false, true, 0, 3, 0}},
    {spv::OpCooperativeMatrixLoadNV, {"OpCooperativeMatrixLoadNV", true, true, 0, 0, 0}},
    {spv::OpCooperativeMatrixStoreNV, {"OpCooperativeMatrixStoreNV", false, false, 0, 0, 0}},
    {spv::OpCooperativeMatrixMulAddNV, {"OpCooperativeMatrixMulAddNV", true, true, 0, 0, 0}},
    {spv::OpCooperativeMatrixLengthNV, {"OpCooperativeMatrixLengthNV", true, true, 0, 0, 0}},
    {spv::OpBeginInvocationInterlockEXT, {"OpBeginInvocationInterlockEXT", false, false, 0, 0, 0}},
    {spv::OpEndInvocationInterlockEXT, {"OpEndInvocationInterlockEXT", false, false, 0, 0, 0}},
    {spv::OpDemoteToHelperInvocationEXT, {"OpDemoteToHelperInvocationEXT", false, false, 0, 0, 0}},
    {spv::OpIsHelperInvocationEXT, {"OpIsHelperInvocationEXT", true, true, 0, 0, 0}},
    {spv::OpConvertUToImageNV, {"OpConvertUToImageNV", true, true, 0, 0, 0}},
    {spv::OpConvertUToSamplerNV, {"OpConvertUToSamplerNV", true, true, 0, 0, 0}},
    {spv::OpConvertImageToUNV, {"OpConvertImageToUNV", true, true, 0, 0, 0}},
    {spv::OpConvertSamplerToUNV, {"OpConvertSamplerToUNV", true, true, 0, 0, 0}},
    {spv::OpConvertUToSampledImageNV, {"OpConvertUToSampledImageNV", true, true, 0, 0, 0}},
    {spv::OpConvertSampledImageToUNV, {"OpConvertSampledImageToUNV", true, true, 0, 0, 0}},
    {spv::OpSamplerImageAddressingModeNV, {"OpSamplerImageAddressingModeNV", false, false, 0, 0, 0}},
    {spv::OpSubgroupShuffleINTEL, {"OpSubgroupShuffleINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupShuffleDownINTEL, {"OpSubgroupShuffleDownINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupShuffleUpINTEL, {"OpSubgroupShuffleUpINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupShuffleXorINTEL, {"OpSubgroupShuffleXorINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupBlockReadINTEL, {"OpSubgroupBlockReadINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupBlockWriteINTEL, {"OpSubgroupBlockWriteINTEL", false, false, 0, 0, 0}},
    {spv::OpSubgroupImageBlockReadINTEL, {"OpSubgroupImageBlockReadINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupImageBlockWriteINTEL, {"OpSubgroupImageBlockWriteINTEL", false, false, 0, 0, 0}},
    {spv::OpSubgroupImageMediaBlockReadINTEL, {"OpSubgroupImageMediaBlockReadINTEL", true, true, 0, 0, 0}},
    {spv::OpSubgroupImageMediaBlockWriteINTEL, {"OpSubgroupImageMediaBlockWriteINTEL", false, false, 0, 0, 0}},
    {spv::OpUCountLeadingZerosINTEL, {"OpUCountLeadingZerosINTEL", true, true, 0, 0, 0}},
    {spv::OpUCountTrailingZerosINTEL, {"OpUCountTrailingZerosINTEL", true, true, 0, 0, 0}},
    {spv::OpAbsISubINTEL, {"OpAbsISubINTEL", true, true, 0, 0, 0}},
    {spv::OpAbsUSubINTEL, {"OpAbsUSubINTEL", true, true, 0, 0, 0}},
    {spv::OpIAddSatINTEL, {"OpIAddSatINTEL", true, true, 0, 0, 0}},
    {spv::OpUAddSatINTEL, {"OpUAddSatINTEL", true, true, 0, 0, 0}},
    {spv::OpIAverageINTEL, {"OpIAverageINTEL", true, true, 0, 0, 0}},
    {spv::OpUAverageINTEL, {"OpUAverageINTEL", true, true, 0, 0, 0}},
    {spv::OpIAverageRoundedINTEL, {"OpIAverageRoundedINTEL", true, true, 0, 0, 0}},
    {spv::OpUAverageRoundedINTEL, {"OpUAverageRoundedINTEL", true, true, 0, 0, 0}},
    {spv::OpISubSatINTEL, {"OpISubSatINTEL", true, true, 0, 0, 0}},
    {spv::OpUSubSatINTEL, {"OpUSubSatINTEL", true, true, 0, 0, 0}},
    {spv::OpIMul32x16INTEL, {"OpIMul32x16INTEL", true, true, 0, 0, 0}},
    {spv::OpUMul32x16INTEL, {"OpUMul32x16INTEL", true, true, 0, 0, 0}},
    {spv::OpConstantFunctionPointerINTEL, {"OpConstantFunctionPointerINTEL", true, true, 0, 0, 0}},
    {spv::OpFunctionPointerCallINTEL, {"OpFunctionPointerCallINTEL", true, true, 0, 0, 0}},
    {spv::OpAsmTargetINTEL, {"OpAsmTargetINTEL", true, true, 0, 0, 0}},
    {spv::OpAsmINTEL, {"OpAsmINTEL", true, true, 0, 0, 0}},
    {spv::OpAsmCallINTEL, {"OpAsmCallINTEL", true, true, 0, 0, 0}},
    {spv::OpAtomicFMinEXT, {"OpAtomicFMinEXT", true, true, 4, 0, 0}},
    {spv::OpAtomicFMaxEXT, {"OpAtomicFMaxEXT", true, true, 4, 0, 0}},
    {spv::OpAssumeTrueKHR, {"OpAssumeTrueKHR", false, false, 0, 0, 0}},
    {spv::OpExpectKHR, {"OpExpectKHR", true, true, 0, 0, 0}},
    {spv::OpDecorateStringGOOGLE, {"OpDecorateStringGOOGLE", false, false, 0, 0, 0}},
    {spv::OpMemberDecorateStringGOOGLE, {"OpMemberDecorateStringGOOGLE", false, false, 0, 0, 0}},
    {spv::OpVariableLengthArrayINTEL, {"OpVariableLengthArrayINTEL", true, true, 0, 0, 0}},
    {spv::OpSaveMemoryINTEL, {"OpSaveMemoryINTEL", true, true, 0, 0, 0}},
    {spv::OpRestoreMemoryINTEL, {"OpRestoreMemoryINTEL", false, false, 0, 0, 0}},
    {spv::OpLoopControlINTEL, {"OpLoopControlINTEL", false, false, 0, 0, 0}},
    {spv::OpAliasDomainDeclINTEL, {"OpAliasDomainDeclINTEL", false, true, 0, 0, 0}},
    {spv::OpAliasScopeDeclINTEL, {"OpAliasScopeDeclINTEL", false, true, 0, 0, 0}},
    {spv::OpAliasScopeListDeclINTEL, {"OpAliasScopeListDeclINTEL", false, true, 0, 0, 0}},
    {spv::OpPtrCastToCrossWorkgroupINTEL, {"OpPtrCastToCrossWorkgroupINTEL", true, true, 0, 0, 0}},
    {spv::OpCrossWorkgroupCastToPtrINTEL, {"OpCrossWorkgroupCastToPtrINTEL", true, true, 0, 0, 0}},
    {spv::OpReadPipeBlockingINTEL, {"OpReadPipeBlockingINTEL", true, true, 0, 0, 0}},
    {spv::OpWritePipeBlockingINTEL, {"OpWritePipeBlockingINTEL", true, true, 0, 0, 0}},
    {spv::OpFPGARegINTEL, {"OpFPGARegINTEL", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetRayTMinKHR, {"OpRayQueryGetRayTMinKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetRayFlagsKHR, {"OpRayQueryGetRayFlagsKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionTKHR, {"OpRayQueryGetIntersectionTKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR, {"OpRayQueryGetIntersectionInstanceCustomIndexKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionInstanceIdKHR, {"OpRayQueryGetIntersectionInstanceIdKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, {"OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionGeometryIndexKHR, {"OpRayQueryGetIntersectionGeometryIndexKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionPrimitiveIndexKHR, {"OpRayQueryGetIntersectionPrimitiveIndexKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionBarycentricsKHR, {"OpRayQueryGetIntersectionBarycentricsKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionFrontFaceKHR, {"OpRayQueryGetIntersectionFrontFaceKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, {"OpRayQueryGetIntersectionCandidateAABBOpaqueKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionObjectRayDirectionKHR, {"OpRayQueryGetIntersectionObjectRayDirectionKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionObjectRayOriginKHR, {"OpRayQueryGetIntersectionObjectRayOriginKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetWorldRayDirectionKHR, {"OpRayQueryGetWorldRayDirectionKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetWorldRayOriginKHR, {"OpRayQueryGetWorldRayOriginKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionObjectToWorldKHR, {"OpRayQueryGetIntersectionObjectToWorldKHR", true, true, 0, 0, 0}},
    {spv::OpRayQueryGetIntersectionWorldToObjectKHR, {"OpRayQueryGetIntersectionWorldToObjectKHR", true, true, 0, 0, 0}},
    {spv::OpAtomicFAddEXT, {"OpAtomicFAddEXT", true, true, 4, 0, 0}},
    {spv::OpTypeBufferSurfaceINTEL, {"OpTypeBufferSurfaceINTEL", false, true, 0, 0, 0}},
    {spv::OpTypeStructContinuedINTEL, {"OpTypeStructContinuedINTEL", false, false, 0, 0, 0}},
    {spv::OpConstantCompositeContinuedINTEL, {"OpConstantCompositeContinuedINTEL", false, false, 0, 0, 0}},
    {spv::OpSpecConstantCompositeContinuedINTEL, {"OpSpecConstantCompositeContinuedINTEL", false, false, 0, 0, 0}},
    {spv::OpControlBarrierArriveINTEL, {"OpControlBarrierArriveINTEL", false, false, 2, 1, 0}},
    {spv::OpControlBarrierWaitINTEL, {"OpControlBarrierWaitINTEL", false, false, 2, 1, 0}},
    {spv::OpGroupIMulKHR, {"OpGroupIMulKHR", true, true, 0, 3, 0}},
    {spv::OpGroupFMulKHR, {"OpGroupFMulKHR", true, true, 0, 3, 0}},
    {spv::OpGroupBitwiseAndKHR, {"OpGroupBitwiseAndKHR", true, true, 0, 3, 0}},
    {spv::OpGroupBitwiseOrKHR, {"OpGroupBitwiseOrKHR", true, true, 0, 3, 0}},
    {spv::OpGroupBitwiseXorKHR, {"OpGroupBitwiseXorKHR", true, true, 0, 3, 0}},
    {spv::OpGroupLogicalAndKHR, {"OpGroupLogicalAndKHR", true, true, 0, 3, 0}},
    {spv::OpGroupLogicalOrKHR, {"OpGroupLogicalOrKHR", true, true, 0, 3, 0}},
    {spv::OpGroupLogicalXorKHR, {"OpGroupLogicalXorKHR", true, true, 0, 3, 0}},
};
// clang-format on

// Any non supported operation will be covered with VUID 01090
bool AtomicOperation(uint32_t opcode) {
    bool found = false;
    switch (opcode) {
        case spv::OpAtomicLoad:
        case spv::OpAtomicStore:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFMinEXT:
        case spv::OpAtomicFMaxEXT:
        case spv::OpAtomicFAddEXT:
            found = true;
            break;
        default:
            break;
    }
    return found;
}


// Any non supported operation will be covered with VUID 01090
bool GroupOperation(uint32_t opcode) {
    bool found = false;
    switch (opcode) {
        case spv::OpGroupNonUniformElect:
        case spv::OpGroupNonUniformAll:
        case spv::OpGroupNonUniformAny:
        case spv::OpGroupNonUniformAllEqual:
        case spv::OpGroupNonUniformBroadcast:
        case spv::OpGroupNonUniformBroadcastFirst:
        case spv::OpGroupNonUniformBallot:
        case spv::OpGroupNonUniformInverseBallot:
        case spv::OpGroupNonUniformBallotBitExtract:
        case spv::OpGroupNonUniformBallotBitCount:
        case spv::OpGroupNonUniformBallotFindLSB:
        case spv::OpGroupNonUniformBallotFindMSB:
        case spv::OpGroupNonUniformShuffle:
        case spv::OpGroupNonUniformShuffleXor:
        case spv::OpGroupNonUniformShuffleUp:
        case spv::OpGroupNonUniformShuffleDown:
        case spv::OpGroupNonUniformIAdd:
        case spv::OpGroupNonUniformFAdd:
        case spv::OpGroupNonUniformIMul:
        case spv::OpGroupNonUniformFMul:
        case spv::OpGroupNonUniformSMin:
        case spv::OpGroupNonUniformUMin:
        case spv::OpGroupNonUniformFMin:
        case spv::OpGroupNonUniformSMax:
        case spv::OpGroupNonUniformUMax:
        case spv::OpGroupNonUniformFMax:
        case spv::OpGroupNonUniformBitwiseAnd:
        case spv::OpGroupNonUniformBitwiseOr:
        case spv::OpGroupNonUniformBitwiseXor:
        case spv::OpGroupNonUniformLogicalAnd:
        case spv::OpGroupNonUniformLogicalOr:
        case spv::OpGroupNonUniformLogicalXor:
        case spv::OpGroupNonUniformQuadBroadcast:
        case spv::OpGroupNonUniformQuadSwap:
        case spv::OpGroupNonUniformPartitionNV:
            found = true;
            break;
        default:
            break;
    }
    return found;
}


spv::StorageClass Instruction::StorageClass() const {
    spv::StorageClass storage_class = spv::StorageClassMax;
    switch (Opcode()) {
        case spv::OpTypePointer:
            storage_class = static_cast<spv::StorageClass>(Word(2));
            break;
        case spv::OpTypeForwardPointer:
            storage_class = static_cast<spv::StorageClass>(Word(2));
            break;
        case spv::OpVariable:
            storage_class = static_cast<spv::StorageClass>(Word(3));
            break;
        default:
            break;
    }
    return storage_class;
}


bool ImageGatherOperation(uint32_t opcode) {
    bool found = false;
    switch (opcode) {
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
            found = true;
            break;
        default:
            break;
    }
    return found;
}


bool ImageFetchOperation(uint32_t opcode) {
    bool found = false;
    switch (opcode) {
        case spv::OpImageFetch:
            found = true;
            break;
        default:
            break;
    }
    return found;
}


bool ImageSampleOperation(uint32_t opcode) {
    bool found = false;
    switch (opcode) {
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageSampleFootprintNV:
            found = true;
            break;
        default:
            break;
    }
    return found;
}


bool OpcodeHasType(uint32_t opcode) {
    bool has_type = false;
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        has_type = format_info->second.has_type;
    }
    return has_type;
}

bool OpcodeHasResult(uint32_t opcode) {
    bool has_result = false;
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        has_result = format_info->second.has_result;
    }
    return has_result;
}

// Return operand position of Memory Scope <ID> or zero if there is none
uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        position = format_info->second.memory_scope_position;
    }
    return position;
}

// Return operand position of Execution Scope <ID> or zero if there is none
uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        position = format_info->second.execution_scope_position;
    }
    return position;
}

// Return operand position of Image Operands <ID> or zero if there is none
uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {
    uint32_t position = 0;
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        position = format_info->second.image_operands_position;
    }
    return position;
}

// Return number of optional parameter from ImageOperands
uint32_t ImageOperandsParamCount(uint32_t image_operand) {
    uint32_t count = 0;
    switch (image_operand) {
        case spv::ImageOperandsMaskNone:
        case spv::ImageOperandsNonPrivateTexelMask:
        case spv::ImageOperandsVolatileTexelMask:
        case spv::ImageOperandsSignExtendMask:
        case spv::ImageOperandsZeroExtendMask:
        case spv::ImageOperandsNontemporalMask:
            return 0;
        case spv::ImageOperandsBiasMask:
        case spv::ImageOperandsLodMask:
        case spv::ImageOperandsConstOffsetMask:
        case spv::ImageOperandsOffsetMask:
        case spv::ImageOperandsConstOffsetsMask:
        case spv::ImageOperandsSampleMask:
        case spv::ImageOperandsMinLodMask:
        case spv::ImageOperandsMakeTexelAvailableMask:
        case spv::ImageOperandsMakeTexelVisibleMask:
        case spv::ImageOperandsOffsetsMask:
            return 1;
        case spv::ImageOperandsGradMask:
            return 2;
        default:
            break;
    }
    return count;
}


const char* string_SpvOpcode(uint32_t opcode) {
    auto format_info = kInstructionTable.find(opcode);
    if (format_info != kInstructionTable.end()) {
        return format_info->second.name;
    } else {
        return "Unhandled Opcode";
    }
};

const char* string_SpvStorageClass(uint32_t storage_class) {
    switch(storage_class) {
        case spv::StorageClassUniformConstant:
            return "UniformConstant";
        case spv::StorageClassInput:
            return "Input";
        case spv::StorageClassUniform:
            return "Uniform";
        case spv::StorageClassOutput:
            return "Output";
        case spv::StorageClassWorkgroup:
            return "Workgroup";
        case spv::StorageClassCrossWorkgroup:
            return "CrossWorkgroup";
        case spv::StorageClassPrivate:
            return "Private";
        case spv::StorageClassFunction:
            return "Function";
        case spv::StorageClassGeneric:
            return "Generic";
        case spv::StorageClassPushConstant:
            return "PushConstant";
        case spv::StorageClassAtomicCounter:
            return "AtomicCounter";
        case spv::StorageClassImage:
            return "Image";
        case spv::StorageClassStorageBuffer:
            return "StorageBuffer";
        case spv::StorageClassCallableDataNV:
            return "CallableDataNV";
        case spv::StorageClassIncomingCallableDataNV:
            return "IncomingCallableDataNV";
        case spv::StorageClassRayPayloadNV:
            return "RayPayloadNV";
        case spv::StorageClassHitAttributeNV:
            return "HitAttributeNV";
        case spv::StorageClassIncomingRayPayloadNV:
            return "IncomingRayPayloadNV";
        case spv::StorageClassShaderRecordBufferNV:
            return "ShaderRecordBufferNV";
        case spv::StorageClassPhysicalStorageBuffer:
            return "PhysicalStorageBuffer";
        case spv::StorageClassHitObjectAttributeNV:
            return "HitObjectAttributeNV";
        case spv::StorageClassTaskPayloadWorkgroupEXT:
            return "TaskPayloadWorkgroupEXT";
        case spv::StorageClassCodeSectionINTEL:
            return "CodeSectionINTEL";
        case spv::StorageClassDeviceOnlyINTEL:
            return "DeviceOnlyINTEL";
        case spv::StorageClassHostOnlyINTEL:
            return "HostOnlyINTEL";
        default:
            return "unknown";
    }
};

const char* string_SpvExecutionModel(uint32_t execution_model) {
    switch(execution_model) {
        case spv::ExecutionModelVertex:
            return "Vertex";
        case spv::ExecutionModelTessellationControl:
            return "TessellationControl";
        case spv::ExecutionModelTessellationEvaluation:
            return "TessellationEvaluation";
        case spv::ExecutionModelGeometry:
            return "Geometry";
        case spv::ExecutionModelFragment:
            return "Fragment";
        case spv::ExecutionModelGLCompute:
            return "GLCompute";
        case spv::ExecutionModelKernel:
            return "Kernel";
        case spv::ExecutionModelTaskNV:
            return "TaskNV";
        case spv::ExecutionModelMeshNV:
            return "MeshNV";
        case spv::ExecutionModelRayGenerationNV:
            return "RayGenerationNV";
        case spv::ExecutionModelIntersectionNV:
            return "IntersectionNV";
        case spv::ExecutionModelAnyHitNV:
            return "AnyHitNV";
        case spv::ExecutionModelClosestHitNV:
            return "ClosestHitNV";
        case spv::ExecutionModelMissNV:
            return "MissNV";
        case spv::ExecutionModelCallableNV:
            return "CallableNV";
        case spv::ExecutionModelTaskEXT:
            return "TaskEXT";
        case spv::ExecutionModelMeshEXT:
            return "MeshEXT";
        default:
            return "unknown";
    }
};

