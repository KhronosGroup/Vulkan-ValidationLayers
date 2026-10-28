// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_grammar_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2024 The Khronos Group Inc.
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

// NOLINTBEGIN

#include "containers/custom_containers.h"
#include "spirv_grammar_helper.h"

static const char* string_SpvCooperativeMatrixOperandsMask(spv::CooperativeMatrixOperandsMask mask) {
    switch (mask) {
        case spv::CooperativeMatrixOperandsMaskNone:
            return "NoneKHR";
        case spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask:
            return "MatrixASignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask:
            return "MatrixBSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask:
            return "MatrixCSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask:
            return "MatrixResultSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask:
            return "SaturatingAccumulationKHR";

        default:
            return "Unknown CooperativeMatrixOperandsMask";
    }
}

std::string string_SpvCooperativeMatrixOperands(uint32_t mask) {
    if (mask == 0) {
        return "CooperativeMatrixOperandsMask(0)";
    }
    std::string ret;
    for (uint32_t bit = 1; mask != 0; bit <<= 1, mask >>= 1) {
        if (mask & 1) {
            if (!ret.empty()) {
                ret.append("|");
            }
            ret.append(string_SpvCooperativeMatrixOperandsMask(static_cast<spv::CooperativeMatrixOperandsMask>(bit)));
        }
    }
    return ret;
}

static const char* string_SpvRayFlagsBit(spv::RayFlagsMask mask) {
    switch (mask) {
        case spv::RayFlagsMaskNone:
            return "None";
        case spv::RayFlagsOpaqueKHRMask:
            return "OpaqueKHR";
        case spv::RayFlagsNoOpaqueKHRMask:
            return "NoOpaqueKHR";
        case spv::RayFlagsTerminateOnFirstHitKHRMask:
            return "TerminateOnFirstHitKHR";
        case spv::RayFlagsSkipClosestHitShaderKHRMask:
            return "SkipClosestHitShaderKHR";
        case spv::RayFlagsCullBackFacingTrianglesKHRMask:
            return "CullBackFacingTrianglesKHR";
        case spv::RayFlagsCullFrontFacingTrianglesKHRMask:
            return "CullFrontFacingTrianglesKHR";
        case spv::RayFlagsCullOpaqueKHRMask:
            return "CullOpaqueKHR";
        case spv::RayFlagsCullNoOpaqueKHRMask:
            return "CullNoOpaqueKHR";
        case spv::RayFlagsSkipTrianglesKHRMask:
            return "SkipTrianglesKHR";
        case spv::RayFlagsSkipAABBsKHRMask:
            return "SkipAABBsKHR";
        case spv::RayFlagsForceOpacityMicromap2StateKHRMask:
            return "ForceOpacityMicromap2StateKHR";

        default:
            return "Unknown RayFlagsMask";
    }
}

std::string string_SpvRayFlagsMask(uint32_t mask) {
    if (mask == 0) {
        return "RayFlagsMask(0)";
    }
    std::string ret;
    for (uint32_t bit = 1; mask != 0; bit <<= 1, mask >>= 1) {
        if (mask & 1) {
            if (!ret.empty()) {
                ret.append("|");
            }
            ret.append(string_SpvRayFlagsBit(static_cast<spv::RayFlagsMask>(bit)));
        }
    }
    return ret;
}

const char* string_SpvFPEncoding(spv::FPEncoding value) {
    switch (value) {
        case spv::FPEncodingBFloat16KHR:
            return "BFloat16KHR";
        case spv::FPEncodingFloat8E4M3EXT:
            return "Float8E4M3EXT";
        case spv::FPEncodingFloat8E5M2EXT:
            return "Float8E5M2EXT";
        case spv::FPEncodingFloat6E2M3EXT:
            return "Float6E2M3EXT";
        case spv::FPEncodingFloat6E3M2EXT:
            return "Float6E3M2EXT";
        case spv::FPEncodingFloat4E2M1EXT:
            return "Float4E2M1EXT";
        case spv::FPEncodingFloat8UnsignedE8M0EXT:
            return "Float8UnsignedE8M0EXT";
        case spv::FPEncodingMXInt8EXT:
            return "MXInt8EXT";

        default:
            return "IEEE-754";  // default for 16-bit
    }
}

const OperandInfo& GetOperandInfo(uint32_t opcode) {
    static const vvl::unordered_map<uint32_t, OperandInfo> kOperandTable{
        // clang-format off
        {spv::OpNop, {{}}},
        {spv::OpUndef, {{}}},
        {spv::OpSourceContinued, {{OperandKind::LiteralString}}},
        {spv::OpSource, {{OperandKind::ValueEnum, OperandKind::Literal, OperandKind::Id, OperandKind::LiteralString}}},
        {spv::OpSourceExtension, {{OperandKind::LiteralString}}},
        {spv::OpName, {{OperandKind::Id, OperandKind::LiteralString}}},
        {spv::OpMemberName, {{OperandKind::Id, OperandKind::Literal, OperandKind::LiteralString}}},
        {spv::OpString, {{OperandKind::LiteralString}}},
        {spv::OpLine, {{OperandKind::Id, OperandKind::Literal, OperandKind::Literal}}},
        {spv::OpExtension, {{OperandKind::LiteralString}}},
        {spv::OpExtInstImport, {{OperandKind::LiteralString}}},
        {spv::OpExtInst, {{OperandKind::Id, OperandKind::Literal, OperandKind::Id}}},
        {spv::OpMemoryModel, {{OperandKind::ValueEnum, OperandKind::ValueEnum}}},
        {spv::OpEntryPoint, {{OperandKind::ValueEnum, OperandKind::Id, OperandKind::LiteralString, OperandKind::Id}}},
        {spv::OpExecutionMode, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpCapability, {{OperandKind::ValueEnum}}},
        {spv::OpTypeVoid, {{}}},
        {spv::OpTypeBool, {{}}},
        {spv::OpTypeInt, {{OperandKind::Literal, OperandKind::Literal}}},
        {spv::OpTypeFloat, {{OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpTypeVector, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpTypeMatrix, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpTypeImage, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Literal, OperandKind::Literal, OperandKind::Literal, OperandKind::Literal, OperandKind::ValueEnum, OperandKind::ValueEnum}}},
        {spv::OpTypeSampler, {{}}},
        {spv::OpTypeSampledImage, {{OperandKind::Id}}},
        {spv::OpTypeArray, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeRuntimeArray, {{OperandKind::Id}}},
        {spv::OpTypeStruct, {{OperandKind::Id}}},
        {spv::OpTypePointer, {{OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpTypeFunction, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeForwardPointer, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpConstantTrue, {{}}},
        {spv::OpConstantFalse, {{}}},
        {spv::OpConstant, {{OperandKind::Literal}}},
        {spv::OpConstantComposite, {{OperandKind::Id}}},
        {spv::OpConstantNull, {{}}},
        {spv::OpSpecConstantTrue, {{}}},
        {spv::OpSpecConstantFalse, {{}}},
        {spv::OpSpecConstant, {{OperandKind::Literal}}},
        {spv::OpSpecConstantComposite, {{OperandKind::Id}}},
        {spv::OpSpecConstantOp, {{OperandKind::Literal}}},
        {spv::OpFunction, {{OperandKind::BitEnum, OperandKind::Id}}},
        {spv::OpFunctionParameter, {{}}},
        {spv::OpFunctionEnd, {{}}},
        {spv::OpFunctionCall, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVariable, {{OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpImageTexelPointer, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpLoad, {{OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpStore, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCopyMemory, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCopyMemorySized, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpAccessChain, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpInBoundsAccessChain, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrAccessChain, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpArrayLength, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpGenericPtrMemSemantics, {{OperandKind::Id}}},
        {spv::OpInBoundsPtrAccessChain, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpDecorate, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpMemberDecorate, {{OperandKind::Id, OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpDecorationGroup, {{}}},
        {spv::OpGroupDecorate, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupMemberDecorate, {{OperandKind::Id, OperandKind::Composite}}},
        {spv::OpVectorExtractDynamic, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorInsertDynamic, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorShuffle, {{OperandKind::Id, OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCompositeConstruct, {{OperandKind::Id}}},
        {spv::OpCompositeExtract, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCompositeInsert, {{OperandKind::Id, OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCopyObject, {{OperandKind::Id}}},
        {spv::OpTranspose, {{OperandKind::Id}}},
        {spv::OpSampledImage, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageSampleImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageFetch, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageDrefGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageRead, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageWrite, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImage, {{OperandKind::Id}}},
        {spv::OpImageQuerySizeLod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageQuerySize, {{OperandKind::Id}}},
        {spv::OpImageQueryLod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageQueryLevels, {{OperandKind::Id}}},
        {spv::OpImageQuerySamples, {{OperandKind::Id}}},
        {spv::OpConvertFToU, {{OperandKind::Id}}},
        {spv::OpConvertFToS, {{OperandKind::Id}}},
        {spv::OpConvertSToF, {{OperandKind::Id}}},
        {spv::OpConvertUToF, {{OperandKind::Id}}},
        {spv::OpUConvert, {{OperandKind::Id}}},
        {spv::OpSConvert, {{OperandKind::Id}}},
        {spv::OpFConvert, {{OperandKind::Id}}},
        {spv::OpQuantizeToF16, {{OperandKind::Id}}},
        {spv::OpConvertPtrToU, {{OperandKind::Id}}},
        {spv::OpConvertUToPtr, {{OperandKind::Id}}},
        {spv::OpPtrCastToGeneric, {{OperandKind::Id}}},
        {spv::OpGenericCastToPtr, {{OperandKind::Id}}},
        {spv::OpGenericCastToPtrExplicit, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpBitcast, {{OperandKind::Id}}},
        {spv::OpSNegate, {{OperandKind::Id}}},
        {spv::OpFNegate, {{OperandKind::Id}}},
        {spv::OpIAdd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFAdd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISub, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFSub, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIMul, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFMul, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSRem, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFRem, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorTimesScalar, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesScalar, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorTimesMatrix, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesVector, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesMatrix, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpOuterProduct, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAddCarry, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISubBorrow, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMulExtended, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSMulExtended, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAny, {{OperandKind::Id}}},
        {spv::OpAll, {{OperandKind::Id}}},
        {spv::OpIsNan, {{OperandKind::Id}}},
        {spv::OpIsInf, {{OperandKind::Id}}},
        {spv::OpLogicalEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalOr, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalAnd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalNot, {{OperandKind::Id}}},
        {spv::OpSelect, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpIEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpINotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpULessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpULessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftRightLogical, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftRightArithmetic, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftLeftLogical, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseOr, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseXor, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseAnd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpNot, {{OperandKind::Id}}},
        {spv::OpBitFieldInsert, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitFieldSExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitFieldUExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitReverse, {{OperandKind::Id}}},
        {spv::OpBitCount, {{OperandKind::Id}}},
        {spv::OpDPdx, {{OperandKind::Id}}},
        {spv::OpDPdy, {{OperandKind::Id}}},
        {spv::OpFwidth, {{OperandKind::Id}}},
        {spv::OpDPdxFine, {{OperandKind::Id}}},
        {spv::OpDPdyFine, {{OperandKind::Id}}},
        {spv::OpFwidthFine, {{OperandKind::Id}}},
        {spv::OpDPdxCoarse, {{OperandKind::Id}}},
        {spv::OpDPdyCoarse, {{OperandKind::Id}}},
        {spv::OpFwidthCoarse, {{OperandKind::Id}}},
        {spv::OpEmitVertex, {{}}},
        {spv::OpEndPrimitive, {{}}},
        {spv::OpEmitStreamVertex, {{OperandKind::Id}}},
        {spv::OpEndStreamPrimitive, {{OperandKind::Id}}},
        {spv::OpControlBarrier, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpMemoryBarrier, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicLoad, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicStore, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicExchange, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicCompareExchange, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIIncrement, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIDecrement, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIAdd, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicISub, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicSMin, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicUMin, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicSMax, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicUMax, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicAnd, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicOr, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicXor, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpPhi, {{OperandKind::Composite}}},
        {spv::OpLoopMerge, {{OperandKind::Label, OperandKind::Label, OperandKind::BitEnum}}},
        {spv::OpSelectionMerge, {{OperandKind::Label, OperandKind::BitEnum}}},
        {spv::OpLabel, {{}}},
        {spv::OpBranch, {{OperandKind::Label}}},
        {spv::OpBranchConditional, {{OperandKind::Id, OperandKind::Label, OperandKind::Label, OperandKind::Literal}}},
        {spv::OpSwitch, {{OperandKind::Id, OperandKind::Label, OperandKind::Label}}},
        {spv::OpKill, {{}}},
        {spv::OpReturn, {{}}},
        {spv::OpReturnValue, {{OperandKind::Id}}},
        {spv::OpUnreachable, {{}}},
        {spv::OpGroupAll, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupAny, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupIAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpImageSparseSampleImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseFetch, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseDrefGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseTexelsResident, {{OperandKind::Id}}},
        {spv::OpNoLine, {{}}},
        {spv::OpImageSparseRead, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpSizeOf, {{OperandKind::Id}}},
        {spv::OpModuleProcessed, {{OperandKind::LiteralString}}},
        {spv::OpExecutionModeId, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpDecorateId, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpGroupNonUniformElect, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformAll, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformAny, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformAllEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBroadcastFirst, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformInverseBallot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotBitExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotBitCount, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotFindLSB, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotFindMSB, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffle, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleXor, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleUp, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleDown, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformIAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformIMul, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMul, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformSMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformUMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformSMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformUMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseAnd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseOr, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseXor, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalAnd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalOr, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalXor, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadSwap, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCopyLogical, {{OperandKind::Id}}},
        {spv::OpPtrEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrDiff, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpColorAttachmentReadEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDepthAttachmentReadEXT, {{OperandKind::Id}}},
        {spv::OpStencilAttachmentReadEXT, {{OperandKind::Id}}},
        {spv::OpTypeTensorARM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorReadARM, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpTensorWriteARM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpTensorQuerySizeARM, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGraphConstantARM, {{OperandKind::Literal}}},
        {spv::OpGraphEntryPointARM, {{OperandKind::Id, OperandKind::LiteralString, OperandKind::Id}}},
        {spv::OpGraphARM, {{}}},
        {spv::OpGraphInputARM, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGraphSetOutputARM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGraphEndARM, {{}}},
        {spv::OpTypeGraphARM, {{OperandKind::Literal, OperandKind::Id}}},
        {spv::OpBitcastExtractEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTerminateInvocation, {{}}},
        {spv::OpTypeUntypedPointerKHR, {{OperandKind::ValueEnum}}},
        {spv::OpUntypedVariableKHR, {{OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedAccessChainKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedInBoundsAccessChainKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupBallotKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupFirstInvocationKHR, {{OperandKind::Id}}},
        {spv::OpUntypedPtrAccessChainKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedInBoundsPtrAccessChainKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedArrayLengthKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Literal}}},
        {spv::OpUntypedPrefetchKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpFmaKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupAllKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupAnyKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupAllEqualKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformRotateKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupReadInvocationKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpExtInstWithForwardRefsKHR, {{OperandKind::Id, OperandKind::Literal, OperandKind::Id}}},
        {spv::OpUntypedGroupAsyncCopyKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpTraceRayKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpExecuteCallableKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpConvertUToAccelerationStructureKHR, {{OperandKind::Id}}},
        {spv::OpIgnoreIntersectionKHR, {{}}},
        {spv::OpTerminateRayKHR, {{}}},
        {spv::OpSDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpUDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSUDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpUDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSUDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpTypeCooperativeMatrixKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixMulAddKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixLengthKHR, {{OperandKind::Id}}},
        {spv::OpConstantCompositeReplicateEXT, {{OperandKind::Id}}},
        {spv::OpSpecConstantCompositeReplicateEXT, {{OperandKind::Id}}},
        {spv::OpCompositeConstructReplicateEXT, {{OperandKind::Id}}},
        {spv::OpTypeRayQueryKHR, {{}}},
        {spv::OpRayQueryInitializeKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryTerminateKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGenerateIntersectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryConfirmIntersectionKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryProceedKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTypeKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageSampleWeightedQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBoxFilterQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitCastArrayQCOM, {{OperandKind::Id}}},
        {spv::OpImageBlockMatchWindowSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchWindowSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchGatherSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchGatherSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBFloat16MulAddQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCompositeConstructCoopMatQCOM, {{OperandKind::Id}}},
        {spv::OpCompositeExtractCoopMatQCOM, {{OperandKind::Id}}},
        {spv::OpExtractSubArrayQCOM, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageGatherQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpGroupIAddNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFAddNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpFragmentMaskFetchAMD, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFragmentFetchAMD, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpReadClockKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadAllKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadAnyKHR, {{OperandKind::Id}}},
        {spv::OpTypeBufferEXT, {{OperandKind::ValueEnum}}},
        {spv::OpBufferPointerEXT, {{OperandKind::Id}}},
        {spv::OpAbortKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedImageTexelPointerEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpMemberDecorateIdEXT, {{OperandKind::Id, OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpConstantSizeOfEXT, {{OperandKind::Id}}},
        {spv::OpConstantDataKHR, {{OperandKind::Literal}}},
        {spv::OpSpecConstantDataKHR, {{OperandKind::Literal}}},
        {spv::OpHitObjectRecordHitMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitWithIndexMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetWorldToObjectNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectToWorldNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayDirectionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayOriginNV, {{OperandKind::Id}}},
        {spv::OpHitObjectTraceRayMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetShaderRecordBufferHandleNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetShaderBindingTableRecordIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectRecordEmptyNV, {{OperandKind::Id}}},
        {spv::OpHitObjectTraceRayNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitWithIndexNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectExecuteShaderNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetCurrentTimeNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetAttributesNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetHitKindNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetPrimitiveIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetGeometryIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceIdNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceCustomIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayDirectionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayOriginNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMaxNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMinNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsEmptyNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsHitNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsMissNV, {{OperandKind::Id}}},
        {spv::OpReorderThreadWithHitObjectNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpReorderThreadWithHintNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeHitObjectNV, {{}}},
        {spv::OpImageSampleFootprintNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpTypeVectorIdEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorMatrixMulNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeVectorOuterProductAccumulateNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorReduceSumAccumulateNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorMatrixMulAddNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixConvertUseEXT, {{OperandKind::Id}}},
        {spv::OpEmitMeshTasksEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSetMeshOutputsEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformPartitionEXT, {{OperandKind::Id}}},
        {spv::OpWritePackedPrimitiveIndices4x8NV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFetchMicroTriangleVertexPositionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpFetchMicroTriangleVertexBarycentricNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorLoadNV, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeVectorStoreNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpHitObjectRecordFromQueryEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissMotionEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayFlagsEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectSetShaderBindingTableRecordIndexEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectReorderExecuteShaderEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectTraceReorderExecuteEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectTraceMotionReorderExecuteEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeHitObjectEXT, {{}}},
        {spv::OpReorderThreadWithHintEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpReorderThreadWithHitObjectEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectTraceRayEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectTraceRayMotionEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordEmptyEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectExecuteShaderEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetCurrentTimeEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetAttributesEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetHitKindEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetPrimitiveIndexEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetGeometryIndexEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceIdEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceCustomIndexEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayOriginEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayDirectionEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayDirectionEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayOriginEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectToWorldEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldToObjectEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMaxEXT, {{OperandKind::Id}}},
        {spv::OpReportIntersectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIgnoreIntersectionNV, {{}}},
        {spv::OpTerminateRayNV, {{}}},
        {spv::OpTraceNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTraceMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTraceRayMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeAccelerationStructureKHR, {{}}},
        {spv::OpExecuteCallableNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionClusterIdNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetClusterIdNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMinEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetShaderBindingTableRecordIndexEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectGetShaderRecordBufferHandleEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectIsEmptyEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectIsHitEXT, {{OperandKind::Id}}},
        {spv::OpHitObjectIsMissEXT, {{OperandKind::Id}}},
        {spv::OpTypeCooperativeMatrixNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixMulAddNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLengthNV, {{OperandKind::Id}}},
        {spv::OpCooperativeMatrixGetCoordinateEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBeginInvocationInterlockEXT, {{}}},
        {spv::OpEndInvocationInterlockEXT, {{}}},
        {spv::OpCooperativeMatrixReduceEXT, {{OperandKind::Id, OperandKind::BitEnum, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadTensorNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreTensorNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixPerElementOpEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeTensorLayoutNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeTensorViewNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCreateTensorLayoutNV, {{}}},
        {spv::OpTensorLayoutSetDimensionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetStrideNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSliceNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetClampValueNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCreateTensorViewNV, {{}}},
        {spv::OpTensorViewSetDimensionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorViewSetStrideNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDemoteToHelperInvocation, {{}}},
        {spv::OpIsHelperInvocationEXT, {{}}},
        {spv::OpTensorViewSetClipNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetBlockSizeNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixTransposeNV, {{OperandKind::Id}}},
        {spv::OpConvertUToImageNV, {{OperandKind::Id}}},
        {spv::OpConvertUToSamplerNV, {{OperandKind::Id}}},
        {spv::OpConvertImageToUNV, {{OperandKind::Id}}},
        {spv::OpConvertSamplerToUNV, {{OperandKind::Id}}},
        {spv::OpConvertUToSampledImageNV, {{OperandKind::Id}}},
        {spv::OpConvertSampledImageToUNV, {{OperandKind::Id}}},
        {spv::OpSamplerImageAddressingModeNV, {{OperandKind::Literal}}},
        {spv::OpRawAccessChainNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpRayQueryGetIntersectionSpherePositionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionSphereRadiusNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSPositionsNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSRadiiNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSHitValueNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetSpherePositionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetSphereRadiusNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetLSSPositionsNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetLSSRadiiNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsSphereHitNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsLSSHitNV, {{OperandKind::Id}}},
        {spv::OpRayQueryIsSphereHitNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryIsLSSHitNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUCountLeadingZerosINTEL, {{OperandKind::Id}}},
        {spv::OpUCountTrailingZerosINTEL, {{OperandKind::Id}}},
        {spv::OpAbsISubINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAbsUSubINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAddSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAddSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAverageINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAverageINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAverageRoundedINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAverageRoundedINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISubSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUSubSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIMul32x16INTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMul32x16INTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpConstantFunctionPointerINTEL, {{OperandKind::Id}}},
        {spv::OpFunctionPointerCallINTEL, {{OperandKind::Id}}},
        {spv::OpAtomicFMinEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicFMaxEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAssumeTrueKHR, {{OperandKind::Id}}},
        {spv::OpExpectKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDecorateString, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpMemberDecorateString, {{OperandKind::Id, OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpRayQueryGetRayTMinKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetRayFlagsKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceIdKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionGeometryIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionPrimitiveIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionBarycentricsKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionFrontFaceKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectRayDirectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectRayOriginKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetWorldRayDirectionKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetWorldRayOriginKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectToWorldKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionWorldToObjectKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicFAddEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpControlBarrierArriveEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpControlBarrierWaitEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpArithmeticFenceEXT, {{OperandKind::Id}}},
        {spv::OpSubgroupBitcastShuffleINTEL, {{OperandKind::Id}}},
        {spv::OpSubgroupBlockPrefetchINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpSubgroup2DBlockLoadINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockLoadTransformINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockLoadTransposeINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockPrefetchINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockStoreINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupMatrixMultiplyAccumulateINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpBitwiseFunctionINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpUntypedVariableLengthArrayINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPredicatedLoadINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpPredicatedStoreINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpPauseINTEL, {{OperandKind::Id}}},
        {spv::OpGroupIMulKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMulKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseAndKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseOrKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseXorKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalAndKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalOrKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalXorKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpRoundFToTF32INTEL, {{OperandKind::Id}}},
        {spv::OpFDot2MixAcc32VALVE, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpFDot2MixAcc16VALVE, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpFDot4MixAcc32VALVE, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
    };  // clang-format on

    auto info = kOperandTable.find(opcode);
    if (info != kOperandTable.end()) {
        return info->second;
    }
    return kOperandTable.find(spv::OpNop)->second;
}

// NOLINTEND
