/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
 */
#pragma once

#include "vulkan/vulkan.h"
#include "vk_object_types.h"
#include "vk_layer_data.h"
#include "vk_layer_logging.h"
#include "vk_layer_utils.h"

#include <atomic>

// Intentionally ignore VulkanTypedHandle::node, it is optional
inline bool operator==(const VulkanTypedHandle &a, const VulkanTypedHandle &b) noexcept {
    return a.handle == b.handle && a.type == b.type;
}
namespace std {
template <>
struct hash<VulkanTypedHandle> {
    size_t operator()(VulkanTypedHandle obj) const noexcept { return hash<uint64_t>()(obj.handle) ^ hash<uint32_t>()(obj.type); }
};
}  // namespace std

// inheriting from enable_shared_from_this<> adds a method, shared_from_this(), which
// returns a shared_ptr version of the current object. It requires the object to
// be created with std::make_shared<> and it MUST NOT be used from the constructor
class BASE_NODE : public std::enable_shared_from_this<BASE_NODE> {
  public:
    // Parent nodes are stored as weak_ptrs to avoid cyclic memory dependencies.
    // Because weak_ptrs cannot safely be used as hash keys, the parents are stored
    // in a map keyed by VulkanTypedHandle. This also allows looking for specific
    // parent types without locking every weak_ptr.
    using NodeMap = vvl::unordered_map<VulkanTypedHandle, std::weak_ptr<BASE_NODE>>;
    using NodeList = small_vector<std::shared_ptr<BASE_NODE>, 4, uint32_t>;

    template <typename Handle>
    BASE_NODE(Handle h, VulkanObjectType t) : handle_(h, t), destroyed_(false) {}

    // because shared_from_this() does not work from the constructor, this 2nd phase
    // constructor is where a state object should call AddParent() on its child nodes.
    // It is called as part of ValidationStateTracker::Add()
    virtual void LinkChildNodes() {}

    virtual ~BASE_NODE();

    // Because state objects are reference counted, they may outlive the vulkan objects
    // they represent. Destroy() is called when the vulkan object is destroyed, so that
    // it can be cleaned up before all references are gone. It also triggers notifications
    // to parent objects.
    virtual void Destroy();

    bool Destroyed() const { return destroyed_; }

    // returns true if this vulkan object or any it uses have been destroyed
    virtual bool Invalid() const { return Destroyed(); }

    // Save the tedium of two part testing...
    static bool Invalid(const BASE_NODE *node) { return !node || node->Destroyed(); }
    static bool Invalid(const std::shared_ptr<const BASE_NODE> &node) { return !node || node->Destroyed(); }

    const VulkanTypedHandle &Handle() const { return handle_; }
    VulkanObjectType Type() const { return handle_.type; }

    virtual bool InUse() const;

    virtual bool AddParent(BASE_NODE *parent_node);
    virtual void RemoveParent(BASE_NODE *parent_node);

    // Invalidate is called on a state object to inform its parents that it
    // is being destroyed (unlink == true) or otherwise becoming invalid (unlink == false)
    void Invalidate(bool unlink = true);

    // Helper to let objects examine their immediate parents without holding the tree lock.
    NodeMap ObjectBindings() const;

  protected:
    template <typename Derived, typename Shared = std::shared_ptr<Derived>>
    static Shared SharedFromThisImpl(Derived *derived) {
        using Base = typename std::conditional<std::is_const<Derived>::value, const BASE_NODE, BASE_NODE>::type;
        auto base = static_cast<Base *>(derived);
        return std::static_pointer_cast<Derived>(base->shared_from_this());
    }

    // Called recursively for every parent object of something that has become invalid
    virtual void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink);

    // returns a copy of the current set of parents so that they can be walked
    // without the tree lock held. If unlink == true, parent_nodes_ is also cleared.
    NodeMap GetParentsForInvalidate(bool unlink);

    VulkanTypedHandle handle_;

    // Set to true when the API-level object is destroyed, but this object may
    // hang around until its shared_ptr refcount goes to zero.
    std::atomic<bool> destroyed_;

  private:
    ReadLockGuard ReadLockTree() const { return ReadLockGuard(tree_lock_); }
    WriteLockGuard WriteLockTree() { return WriteLockGuard(tree_lock_); }

    // Set of immediate parent nodes for this object. For an in-use object, the
    // parent nodes should form a tree with the root being a command buffer.
    NodeMap parent_nodes_;
    // Lock guarding parent_nodes_, this lock MUST NOT be used for other purposes.
    mutable std::shared_mutex tree_lock_;
};

class REFCOUNTED_NODE : public BASE_NODE {
  private:
    // Track if command buffer is in-flight
    std::atomic_int in_use_;

  public:
    template <typename Handle>
    REFCOUNTED_NODE(Handle h, VulkanObjectType t) : BASE_NODE(h, t), in_use_(0) {}

    void BeginUse() { in_use_.fetch_add(1); }

    void EndUse() { in_use_.fetch_sub(1); }

    void ResetUse() { in_use_.store(0); }

    bool InUse() const override { return (in_use_.load() > 0) || BASE_NODE::InUse(); }
};
