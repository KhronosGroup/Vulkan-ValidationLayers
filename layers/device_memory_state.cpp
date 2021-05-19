/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "device_memory_state.h"
#include "image_state.h"

void DEVICE_MEMORY_STATE::RemoveParent(BASE_NODE *parent) {
    // Prevent Control Flow Integrity failures from casting any parent to IMAGE_STATE,
    // Other parent types do not participate in the bound_images spider web.
    // TODO: This is not a great solution and bound_images should be cleaned up by further
    // state object refactoring.
    if (parent->Handle().type == kVulkanObjectTypeImage) {
        auto it = bound_images.find(static_cast<IMAGE_STATE *>(parent));
        if (it != bound_images.end()) {
            IMAGE_STATE *image_state = *it;
            image_state->aliasing_images.clear();
            bound_images.erase(it);
        }
    }
    BASE_NODE::RemoveParent(parent);
}

void DEVICE_MEMORY_STATE::Destroy() {
    // This is one way clear. Because the bound_images include cross references, the one way clear loop could clear the whole
    // reference. It doesn't need two ways clear.
    for (auto *bound_image : bound_images) {
        if (bound_image) {
            bound_image->aliasing_images.clear();
        }
    }
    BASE_NODE::Destroy();
}

void BINDABLE::Destroy() {
    if (!sparse) {
        if (binding.mem_state) {
            binding.mem_state->RemoveParent(this);
        }
    } else {  // Sparse, clear all bindings
        for (auto &sparse_mem_binding : sparse_bindings) {
            if (sparse_mem_binding.mem_state) {
                sparse_mem_binding.mem_state->RemoveParent(this);
            }
        }
    }
    BASE_NODE::Destroy();
}

// SetMemBinding is used to establish immutable, non-sparse binding between a single image/buffer object and memory object.
// Corresponding valid usage checks are in ValidateSetMemBinding().
void BINDABLE::SetMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, VkDeviceSize memory_offset) {
    if (!mem) {
        return;
    }
    binding.mem_state = mem;
    binding.offset = memory_offset;
    binding.size = requirements.size;
    binding.mem_state->AddParent(this);
    UpdateBoundMemorySet();  // force recreation of cached set
}

// For NULL mem case, clear any previous binding Else...
// Make sure given object is in its object map
//  IF a previous binding existed, update binding
//  Add reference from objectInfo to memoryInfo
//  Add reference off of object's binding info
// Return VK_TRUE if addition is successful, VK_FALSE otherwise
void BINDABLE::SetSparseMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize mem_offset,
                                   const VkDeviceSize mem_size) {
    if (!mem) {
        return;
    }
    assert(sparse);
    MEM_BINDING sparse_binding = {mem, mem_offset, mem_size};
    sparse_binding.mem_state->AddParent(this);
    // Need to set mem binding for this object
    sparse_bindings.insert(sparse_binding);
    UpdateBoundMemorySet();
}
