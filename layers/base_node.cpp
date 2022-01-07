/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
#include "base_node.h"
#include "vk_layer_utils.h"

BASE_NODE::~BASE_NODE() { Destroy(); }

void BASE_NODE::Destroy() {
    Invalidate();
    destroyed_ = true;
}

bool BASE_NODE::InUse() const {
    bool result = false;
    for (auto& node: parent_nodes_) {
        result |= node->InUse();
        if (result) {
            break;
        }
    }
    return result;
}

bool BASE_NODE::AddParent(BASE_NODE *parent_node) {
    auto result = parent_nodes_.emplace(parent_node);
    return result.second;
}

void BASE_NODE::RemoveParent(BASE_NODE *parent_node) {
    parent_nodes_.erase(parent_node);
}

void BASE_NODE::Invalidate(bool unlink) {
    NodeList invalid_nodes;
    invalid_nodes.emplace_back(this);
    for (auto& node: parent_nodes_) {
        node->NotifyInvalidate(invalid_nodes, unlink);
    }
    if (unlink) {
        parent_nodes_.clear();
    }
}

void BASE_NODE::NotifyInvalidate(const NodeList& invalid_nodes, bool unlink) {
    if (parent_nodes_.size() == 0) {
        return;
    }
    NodeList up_nodes = invalid_nodes;
    up_nodes.emplace_back(this);
    for (auto& node: parent_nodes_) {
        node->NotifyInvalidate(up_nodes, unlink);
    }
    if (unlink) {
        parent_nodes_.clear();
    }
}
