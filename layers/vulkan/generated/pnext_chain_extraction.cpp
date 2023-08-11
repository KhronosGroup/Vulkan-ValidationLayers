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

#include "vk_typemap_helper.h"

#include <cassert>

namespace vvl {
       
void* PnextChainAdd(void *chain_end, void *new_struct) {
    assert(new_struct);
    if (!chain_end) {
    	return new_struct;
    }
    auto *vk_base_struct = reinterpret_cast<VkBaseOutStructure*>(chain_end);
    assert(!vk_base_struct->pNext);
    vk_base_struct->pNext = reinterpret_cast<VkBaseOutStructure*>(new_struct);
    return new_struct;
};

void PnextChainRemoveLast(void *chain) {
    if (!chain) {
        return;
    }
    auto *current = reinterpret_cast<VkBaseOutStructure *>(chain);
    auto *prev = current;
    while (current) {
        prev = current;
        current = reinterpret_cast<VkBaseOutStructure *>(current->pNext);
    }
    prev->pNext = nullptr;
}


template <>
PnextChainVkPhysicalDeviceImageFormatInfo2 ExtractPnextChain(const void *in_pnext_chain) {

	std::tuple<void *,
	VkImageCompressionControlEXT,
	VkImageFormatListCreateInfo,
	VkImageStencilUsageCreateInfo,
	VkOpticalFlowImageFormatInfoNV,
	VkPhysicalDeviceExternalImageFormatInfo,
	VkPhysicalDeviceImageDrmFormatModifierInfoEXT,
	VkPhysicalDeviceImageViewImageFormatInfoEXT,
	VkVideoProfileListInfoKHR> out_structs;

    void * &chain_end = std::get<0>(out_structs);

    if (auto *chain_struct = LvlFindInChain<VkImageCompressionControlEXT>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkImageCompressionControlEXT>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkImageFormatListCreateInfo>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkImageFormatListCreateInfo>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkImageStencilUsageCreateInfo>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkImageStencilUsageCreateInfo>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkOpticalFlowImageFormatInfoNV>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkOpticalFlowImageFormatInfoNV>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkPhysicalDeviceExternalImageFormatInfo>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkPhysicalDeviceExternalImageFormatInfo>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkPhysicalDeviceImageViewImageFormatInfoEXT>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkPhysicalDeviceImageViewImageFormatInfoEXT>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

    if (auto *chain_struct = LvlFindInChain<VkVideoProfileListInfoKHR>(in_pnext_chain)) {
    	auto &out_chain_struct = std::get<VkVideoProfileListInfoKHR>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }

	return out_structs;
}

}

// NOLINTEND
