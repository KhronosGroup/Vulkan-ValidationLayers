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

#pragma once

#include <tuple>

#include "vulkan/vulkan.h"

namespace vvl {

// Add element to end of pNext chain: chain_end must be a pointer to the last struct of the chain
void* PnextChainAdd(void *chain_end, void *new_struct);

// Remove last element from pNext chain 
void PnextChainRemoveLast(void *chain);

// Utility to make a selective copy of a pNext chain.
// Structs listed in the returned tuple are the one extending some Vulkan structs, like VkPhysicalDeviceImageFormatInfo2.
// Copied structs are the one mentioned in the returned tuple type and found in `in_pnext_chain`.
// If the returned tuple is stack allocated, each struct is NOT a deep copy of the corresponding struct in in_pnext_chain,
// so be mindful of pointers copies.
// Beginning of pNext chain is stored in `std::get<0>(returned_tuple)` as a `void *`
template <typename T>
T ExtractPnextChain(const void *in_pnext_chain) {}


using PnextChainVkPhysicalDeviceImageFormatInfo2 = std::tuple<void *,
	VkImageCompressionControlEXT,
	VkImageFormatListCreateInfo,
	VkImageStencilUsageCreateInfo,
	VkOpticalFlowImageFormatInfoNV,
	VkPhysicalDeviceExternalImageFormatInfo,
	VkPhysicalDeviceImageDrmFormatModifierInfoEXT,
	VkPhysicalDeviceImageViewImageFormatInfoEXT,
	VkVideoProfileListInfoKHR>;
template <>
PnextChainVkPhysicalDeviceImageFormatInfo2 ExtractPnextChain(const void *in_pnext_chain);

}

// NOLINTEND
