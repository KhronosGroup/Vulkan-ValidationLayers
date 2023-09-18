// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See pnext_chain_extraction_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "pnext_chain_extraction.h"

#include <vulkan/utility/vk_struct_helper.hpp>

namespace vvl {

void *PnextChainAdd(void *chain, void *new_struct) {
    assert(chain);
    assert(new_struct);
    void *chain_end = vku::FindLastStructInPNextChain(chain);
    auto *vk_base_struct = static_cast<VkBaseOutStructure *>(chain_end);
    assert(!vk_base_struct->pNext);
    vk_base_struct->pNext = static_cast<VkBaseOutStructure *>(new_struct);
    return new_struct;
}

void PnextChainRemoveLast(void *chain) {
    if (!chain) {
        return;
    }
    auto *current = static_cast<VkBaseOutStructure *>(chain);
    auto *prev = current;
    while (current->pNext) {
        prev = current;
        current = static_cast<VkBaseOutStructure *>(current->pNext);
    }
    prev->pNext = nullptr;
}

template <>
void *PnextChainExtract(const void *in_pnext_chain, PnextChainVkPhysicalDeviceImageFormatInfo2 &out) {
    void *chain_begin = nullptr;
    void *chain_end = nullptr;

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageCompressionControlEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageFormatListCreateInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageStencilUsageCreateInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkOpticalFlowImageFormatInfoNV>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkOpticalFlowImageFormatInfoNV>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceExternalImageFormatInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceExternalImageFormatInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceImageViewImageFormatInfoEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceImageViewImageFormatInfoEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkVideoProfileListInfoKHR>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    return chain_begin;
}

}  // namespace vvl

// NOLINTEND
