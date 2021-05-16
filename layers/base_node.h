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
#pragma once

#include "vulkan/vulkan.h"
#include "vk_object_types.h"
#include "vk_layer_data.h"

#include <atomic>

struct CMD_BUFFER_STATE;

class BASE_NODE {
  public:
    using BindingsType = layer_data::unordered_map<CMD_BUFFER_STATE *, int>;

    template <typename Handle>
    BASE_NODE(Handle h, VulkanObjectType t) : handle_(h, t, this), in_use_(0), destroyed_(false) {}

    virtual ~BASE_NODE() { Destroy(); }

    virtual void Destroy() {
        Invalidate();
        destroyed_ = true;
    }

    bool Destroyed() const { return destroyed_; }

    const VulkanTypedHandle &Handle() const { return handle_; }

    bool InUse() const { return (in_use_.load() > 0); }

    void BeginUse() { in_use_.fetch_add(1); }

    void EndUse() { in_use_.fetch_sub(1); }

    void ResetUse() { in_use_.store(0); }

    virtual bool AddParent(CMD_BUFFER_STATE *cb_node);

    void RemoveParent(CMD_BUFFER_STATE *cb_node);

    void Invalidate(bool unlink = true);

  protected:
    VulkanTypedHandle handle_;

    // Track when object is being used by an in-flight command buffer
    std::atomic_int in_use_;
    // Set to true when the API-level object is destroyed, but this object may
    // hang around until its shared_ptr refcount goes to zero.
    bool destroyed_;

    // Track command buffers that this object is bound to
    //  binding initialized when cmd referencing object is bound to command buffer
    //  binding removed when command buffer is reset or destroyed
    // When an object is destroyed, any bound cbs are set to INVALID.
    // "int" value is an index into object_bindings where the corresponding
    // backpointer to this node is stored.
    BindingsType cb_bindings_;
};
