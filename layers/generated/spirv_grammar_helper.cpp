// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_gramar_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021 The Khronos Group Inc.
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
 * Author: Spencer Fricke <s.fricke@samsung.com>
 *
 * This file is related to anything that is found in the SPIR-V grammar
 * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
 *
 ****************************************************************************/

#include "spirv_grammar_helper.h"

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


// Return parameter position of memory scope ID or zero if there is none
uint32_t MemoryScopeParamPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpMemoryBarrier:
            return 1;
        case spv::OpControlBarrier:
        case spv::OpAtomicStore:
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
            break;
        default:
            break;
    }
    return position;
}

// Return parameter position of execution scope ID or zero if there is none
uint32_t ExecutionScopeParamPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpControlBarrier:
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
            return 3;
            break;
        default:
            break;
    }
    return position;
}

// Return parameter position of Image Operands or zero if there is none
uint32_t ImageOperandsParamPosition(uint32_t opcode) {
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
        case spv::OpImageSampleFootprintNV:
            return 7;
            break;
        default:
            break;
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
            return 1;
        case spv::ImageOperandsGradMask:
            return 2;
            break;
        default:
            break;
    }
    return count;
}



