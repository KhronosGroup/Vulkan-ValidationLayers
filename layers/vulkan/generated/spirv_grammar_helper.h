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

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <spirv/unified1/spirv.hpp>

std::string string_SpvCooperativeMatrixOperands(uint32_t mask);
std::string string_SpvRayFlagsMask(uint32_t mask);
const char* string_SpvFPEncoding(spv::FPEncoding value);

// Any non supported operation will be covered with other VUs
static constexpr bool AtomicOperation(uint32_t opcode) {
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
            return true;
        default:
            return false;
    }
}

// Any non supported operation will be covered with other VUs
static constexpr bool GroupOperation(uint32_t opcode) {
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
        case spv::OpGroupNonUniformQuadAllKHR:
        case spv::OpGroupNonUniformQuadAnyKHR:
        case spv::OpGroupNonUniformPartitionEXT:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageGatherOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageBlockMatchGatherSSDQCOM:
        case spv::OpImageBlockMatchGatherSADQCOM:
        case spv::OpImageGatherQCOM:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageFetchOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageFetch:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageSampleOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageSampleWeightedQCOM:
        case spv::OpImageSampleFootprintNV:
            return true;
        default:
            return false;
    }
}

// Return number of optional parameter from ImageOperands
static constexpr uint32_t ImageOperandsParamCount(uint32_t image_operand) {
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

// Return operand position of Memory Scope <ID> or zero if there is none
static constexpr uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpMemoryBarrier:
            return 1;
        case spv::OpControlBarrier:
        case spv::OpAtomicStore:
        case spv::OpControlBarrierArriveEXT:
        case spv::OpControlBarrierWaitEXT:
            return 2;
        case spv::OpAtomicLoad:
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
            return 4;

        default:
            break;
    }
    return position;
}

// Return operand position of Execution Scope <ID> or zero if there is none
static constexpr uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpControlBarrier:
        case spv::OpControlBarrierArriveEXT:
        case spv::OpControlBarrierWaitEXT:
            return 1;
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupBroadcast:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
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
        case spv::OpGroupNonUniformRotateKHR:
        case spv::OpTypeCooperativeMatrixKHR:
        case spv::OpGroupIAddNonUniformAMD:
        case spv::OpGroupFAddNonUniformAMD:
        case spv::OpGroupFMinNonUniformAMD:
        case spv::OpGroupUMinNonUniformAMD:
        case spv::OpGroupSMinNonUniformAMD:
        case spv::OpGroupFMaxNonUniformAMD:
        case spv::OpGroupUMaxNonUniformAMD:
        case spv::OpGroupSMaxNonUniformAMD:
        case spv::OpReadClockKHR:
        case spv::OpTypeCooperativeMatrixNV:
        case spv::OpGroupIMulKHR:
        case spv::OpGroupFMulKHR:
        case spv::OpGroupBitwiseAndKHR:
        case spv::OpGroupBitwiseOrKHR:
        case spv::OpGroupBitwiseXorKHR:
        case spv::OpGroupLogicalAndKHR:
        case spv::OpGroupLogicalOrKHR:
        case spv::OpGroupLogicalXorKHR:
            return 3;

        default:
            break;
    }
    return position;
}

// Return operand position of Image Operands <ID> or zero if there is none
static constexpr uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpImageWrite:
            return 4;
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageRead:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseRead:
            return 5;
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
            return 6;
        case spv::OpImageGatherQCOM:
        case spv::OpImageSampleFootprintNV:
            return 7;

        default:
            break;
    }
    return position;
}

// Return operand position of 'Image' or 'Sampled Image' IdRef or zero if there is none.
static constexpr uint32_t OpcodeImageAccessPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpImageWrite:
            return 1;
        case spv::OpImageTexelPointer:
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
        case spv::OpImageSparseRead:
        case spv::OpImageGatherQCOM:
        case spv::OpFragmentMaskFetchAMD:
        case spv::OpFragmentFetchAMD:
        case spv::OpImageSampleFootprintNV:
            return 3;

        default:
            break;
    }
    return position;
}

// Return operand position of the 'Acceleration Structure' IdRef or zero if there is none.
static constexpr uint32_t OpcodeAccelerationStructureAccessPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpTraceRayKHR:
        case spv::OpTraceNV:
        case spv::OpTraceMotionNV:
        case spv::OpTraceRayMotionNV:
            return 1;
        case spv::OpRayQueryInitializeKHR:
        case spv::OpHitObjectRecordHitMotionNV:
        case spv::OpHitObjectRecordHitWithIndexMotionNV:
        case spv::OpHitObjectTraceRayMotionNV:
        case spv::OpHitObjectTraceRayNV:
        case spv::OpHitObjectRecordHitNV:
        case spv::OpHitObjectRecordHitWithIndexNV:
        case spv::OpHitObjectTraceReorderExecuteEXT:
        case spv::OpHitObjectTraceMotionReorderExecuteEXT:
        case spv::OpHitObjectTraceRayEXT:
        case spv::OpHitObjectTraceRayMotionEXT:
            return 2;
        case spv::OpFetchMicroTriangleVertexPositionNV:
        case spv::OpFetchMicroTriangleVertexBarycentricNV:
            return 3;

        default:
            break;
    }
    return position;
}

// All valid OpType*
enum class SpvType {
    Empty = 0,
    kVoid,
    kBool,
    kInt,
    kFloat,
    kVector,
    kMatrix,
    kImage,
    kSampler,
    kSampledImage,
    kArray,
    kRuntimeArray,
    kStruct,
    kPointer,
    kFunction,
    kForwardPointer,
    kTensorARM,
    kGraphARM,
    kUntypedPointerKHR,
    kCooperativeMatrixKHR,
    kRayQueryKHR,
    kBufferEXT,
    kHitObjectNV,
    kVectorIdEXT,
    kHitObjectEXT,
    kAccelerationStructureKHR,
    kCooperativeMatrixNV,
    kTensorLayoutNV,
    kTensorViewNV,
};

static constexpr SpvType GetSpvType(uint32_t opcode) {
    switch (opcode) {
        case spv::OpTypeVoid:
            return SpvType::kVoid;
        case spv::OpTypeBool:
            return SpvType::kBool;
        case spv::OpTypeInt:
            return SpvType::kInt;
        case spv::OpTypeFloat:
            return SpvType::kFloat;
        case spv::OpTypeVector:
            return SpvType::kVector;
        case spv::OpTypeMatrix:
            return SpvType::kMatrix;
        case spv::OpTypeImage:
            return SpvType::kImage;
        case spv::OpTypeSampler:
            return SpvType::kSampler;
        case spv::OpTypeSampledImage:
            return SpvType::kSampledImage;
        case spv::OpTypeArray:
            return SpvType::kArray;
        case spv::OpTypeRuntimeArray:
            return SpvType::kRuntimeArray;
        case spv::OpTypeStruct:
            return SpvType::kStruct;
        case spv::OpTypePointer:
            return SpvType::kPointer;
        case spv::OpTypeFunction:
            return SpvType::kFunction;
        case spv::OpTypeForwardPointer:
            return SpvType::kForwardPointer;
        case spv::OpTypeTensorARM:
            return SpvType::kTensorARM;
        case spv::OpTypeGraphARM:
            return SpvType::kGraphARM;
        case spv::OpTypeUntypedPointerKHR:
            return SpvType::kUntypedPointerKHR;
        case spv::OpTypeCooperativeMatrixKHR:
            return SpvType::kCooperativeMatrixKHR;
        case spv::OpTypeRayQueryKHR:
            return SpvType::kRayQueryKHR;
        case spv::OpTypeBufferEXT:
            return SpvType::kBufferEXT;
        case spv::OpTypeHitObjectNV:
            return SpvType::kHitObjectNV;
        case spv::OpTypeVectorIdEXT:
            return SpvType::kVectorIdEXT;
        case spv::OpTypeHitObjectEXT:
            return SpvType::kHitObjectEXT;
        case spv::OpTypeAccelerationStructureKHR:
            return SpvType::kAccelerationStructureKHR;
        case spv::OpTypeCooperativeMatrixNV:
            return SpvType::kCooperativeMatrixNV;
        case spv::OpTypeTensorLayoutNV:
            return SpvType::kTensorLayoutNV;
        case spv::OpTypeTensorViewNV:
            return SpvType::kTensorViewNV;
        default:
            return SpvType::Empty;
    }
}

enum class OperandKind {
    Invalid = 0,
    Id,
    Label,  // Id but for Control Flow
    Literal,
    LiteralString,
    Composite,
    ValueEnum,
    BitEnum,
};

struct OperandInfo {
    std::vector<OperandKind> types;
};

const OperandInfo& GetOperandInfo(uint32_t opcode);

// NOLINTEND
