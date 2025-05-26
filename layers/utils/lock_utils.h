/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include <memory>
#include <shared_mutex>
#include <vulkan/utility/vk_concurrent_unordered_map.hpp>

// Aliases to avoid excessive typing. We can't easily auto these away because
// there are virtual methods in vvl::base::Device which return lock guards
// and those cannot use return type deduction.
typedef std::shared_lock<std::shared_mutex> ReadLockGuard;
typedef std::unique_lock<std::shared_mutex> WriteLockGuard;

// helper class for the very common case of getting and then locking a command buffer (or other state object)
template <typename T, typename Guard>
class LockedSharedPtr : public std::shared_ptr<T> {
  public:
    LockedSharedPtr(std::shared_ptr<T> &&ptr, Guard &&guard) : std::shared_ptr<T>(std::move(ptr)), guard_(std::move(guard)) {}
    LockedSharedPtr() : std::shared_ptr<T>(), guard_() {}

  private:
    Guard guard_;
};
