/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

// A vector class with "small string optimization" -- meaning that the class contains a fixed working store for N elements.
// Useful in in situations where the needed size is unknown, but the typical size is known  If size increases beyond the
// fixed capacity, a dynamically allocated working store is created.
//
// NOTE: Unlike std::vector which only requires T to be CopyAssignable and CopyConstructable, small_vector requires T to be
//       MoveAssignable and MoveConstructable
// NOTE: Unlike std::vector, iterators are invalidated by move assignment between small_vector objects effectively the
//       "small string" allocation functions as an incompatible allocator.
template <typename T, size_t N, typename SizeType = uint32_t>
class small_vector {
  public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = SizeType;
    static const size_type kSmallCapacity = N;
    static const size_type kMaxCapacity = std::numeric_limits<size_type>::max();
    static_assert(N <= kMaxCapacity, "size must be less than size_type::max");

    small_vector() : size_(0), capacity_(N), working_store_(GetSmallStore()) {}

    small_vector(std::initializer_list<T> list) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(list); }

    small_vector(const small_vector &other) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(other); }

    small_vector(small_vector &&other) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        if (other.large_store_) {
            MoveLargeStore(other);
        } else {
            PushBackFrom(std::move(other));
        }
        // Per the spec, when constructing from other, other is guaranteed to be empty after the constructor runs
        other.clear();
    }

    small_vector(size_type size, const value_type &value = value_type()) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        if (size > 0) {
            reserve(size);
            auto dest = GetWorkingStore();
            for (size_type i = 0; i < size; i++) {
                new (&dest[i]) value_type(value);
            }
            size_ = size;
        }
    }

    ~small_vector() {
        clear();
        delete[] large_store_;
    }

    bool operator==(const small_vector &rhs) const {
        if (size_ != rhs.size_) return false;
        auto value = begin();
        for (const auto &rh_value : rhs) {
            if (!(*value == rh_value)) {
                return false;
            }
            ++value;
        }
        return true;
    }

    bool operator!=(const small_vector &rhs) const { return !(*this == rhs); }

    small_vector &operator=(const small_vector &other) {
        if (this != &other) {
            if (other.size_ > capacity_) {
                // Calling reserve would move construct and destroy all current contents, so just clear them before calling
                // PushBackFrom (which does a reserve vs. the now empty this)
                clear();
                PushBackFrom(other);
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);
                // Copy assign anywhere we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = source[i];
                }

                // Copy construct anywhere we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(source[i]);
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    small_vector &operator=(small_vector &&other) {
        if (this != &other) {
            // Note: move assign doesn't require other to become empty (as does move construction)
            //       so we'll leave other alone except in the large store case, while moving the object
            //       *in* the vector from other
            if (other.large_store_) {
                // Moving the other large store intact is probably best, even if we have to destroy everything in this.
                clear();
                MoveLargeStore(other);
            } else if (other.size_ > capacity_) {
                // If we'd have to reallocate, just clean up minimally and copy normally
                clear();
                PushBackFrom(std::move(other));
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);

                // Move assign where we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = std::move(source[i]);
                }

                // Move construct where we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(std::move(source[i]));
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    reference operator[](size_type pos) {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }
    const_reference operator[](size_type pos) const {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }

    // Like std::vector:: calling front or back on an empty container causes undefined behavior
    reference front() {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    const_reference front() const {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    reference back() {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }
    const_reference back() const {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }

    bool empty() const { return size_ == 0; }

    template <class... Args>
    void emplace_back(Args &&...args) {
        assert(size_ < kMaxCapacity);
        reserve(size_ + 1);
        new (GetWorkingStore() + size_) value_type(args...);
        size_++;
    }

    // Note: probably should update this to reflect C++23 ranges
    template <typename Container>
    void PushBackFrom(const Container &from) {
        assert(from.size() <= kMaxCapacity);
        assert(size_ <= kMaxCapacity - from.size());
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (const auto &element : from) {
            new (dest) value_type(element);
            ++dest;
        }
        size_ = new_size;
    }

    template <typename Container>
    void PushBackFrom(Container &&from) {
        assert(from.size() < kMaxCapacity);
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (auto &element : from) {
            new (dest) value_type(std::move(element));
            ++dest;
        }
        size_ = new_size;
    }

    void reserve(size_type new_cap) {
        // Since this can't shrink, if we're growing we're newing
        if (new_cap > capacity_) {
            assert(capacity_ >= kSmallCapacity);
            auto new_store = new BackingStore[new_cap];
            auto working_store = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                new (new_store[i].data) value_type(std::move(working_store[i]));
                working_store[i].~value_type();
            }
            delete[] large_store_;
            large_store_ = new_store;
            assert(new_cap > kSmallCapacity);
            capacity_ = new_cap;
        }
        UpdateWorkingStore();
        // No shrink here.
    }

    void clear() {
        // Keep clear minimal to optimize reset functions for enduring objects
        // more work is deferred until destruction (freeing of large_store for example)
        // and we intentionally *aren't* shrinking.  Callers that desire shrink semantics
        // can call shrink_to_fit.
        auto working_store = GetWorkingStore();
        for (size_type i = 0; i < size_; i++) {
            working_store[i].~value_type();
        }
        size_ = 0;
    }

    void resize(size_type count) {
        struct ValueInitTag {  // tag to request value-initialization
            explicit ValueInitTag() = default;
        };
        Resize(count, ValueInitTag{});
    }

    void resize(size_type count, const value_type &value) { Resize(count, value); }

    void shrink_to_fit() {
        if (size_ == 0) {
            // shrink resets to small when empty
            capacity_ = kSmallCapacity;
            delete[] large_store_;
            large_store_ = nullptr;
            UpdateWorkingStore();
        } else if ((capacity_ > kSmallCapacity) && (capacity_ > size_)) {
            auto source = GetWorkingStore();
            // Keep the source from disappearing until the end of the function
            auto old_store = large_store_;
            large_store_ = nullptr;
            assert(!large_store_);
            if (size_ < kSmallCapacity) {
                capacity_ = kSmallCapacity;
            } else {
                large_store_ = new BackingStore[size_];
                capacity_ = size_;
            }
            UpdateWorkingStore();
            auto dest = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                dest[i] = std::move(source[i]);
                source[i].~value_type();
            }
            delete[] old_store;
        }
    }

    inline iterator begin() { return GetWorkingStore(); }
    inline const_iterator cbegin() const { return GetWorkingStore(); }
    inline const_iterator begin() const { return GetWorkingStore(); }

    inline iterator end() { return GetWorkingStore() + size_; }
    inline const_iterator cend() const { return GetWorkingStore() + size_; }
    inline const_iterator end() const { return GetWorkingStore() + size_; }
    inline size_type size() const { return size_; }
    auto capacity() const { return capacity_; }

    inline pointer data() { return GetWorkingStore(); }
    inline const_pointer data() const { return GetWorkingStore(); }

  protected:
    inline const_pointer ComputeWorkingStore() const {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        const BackingStore *store = large_store_ ? large_store_ : small_store_;
        return &store->object;
    }
    inline pointer ComputeWorkingStore() {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        BackingStore *store = large_store_ ? large_store_ : small_store_;
        return &store->object;
    }

    void UpdateWorkingStore() { working_store_ = ComputeWorkingStore(); }

    inline const_pointer GetWorkingStore() const {
        DbgWorkingStoreCheck();
        return working_store_;
    }
    inline pointer GetWorkingStore() {
        DbgWorkingStoreCheck();
        return working_store_;
    }

    inline pointer GetSmallStore() { return &small_store_->object; }

    union BackingStore {
        BackingStore() {}
        ~BackingStore() {}

        uint8_t data[sizeof(value_type)];
        value_type object;
    };
    size_type size_ = 0;
    size_type capacity_ = 0;
    BackingStore small_store_[N]{};
    // Even an empty std::unique_ptr can be costly to construct,
    // so use a raw pointer
    BackingStore *large_store_ = nullptr;
    value_type *working_store_ = nullptr;

#ifndef NDEBUG
    void DbgWorkingStoreCheck() const { assert(ComputeWorkingStore() == working_store_); };
#else
    void DbgWorkingStoreCheck() const {};
#endif

  private:
    void MoveLargeStore(small_vector &other) {
        assert(other.large_store_);
        assert(other.capacity_ > kSmallCapacity);
        // In move operations, from a small vector with a large store, we can move from it
        delete[] large_store_;
        large_store_ = other.large_store_;
        other.large_store_ = nullptr;
        capacity_ = other.capacity_;
        size_ = other.size_;
        UpdateWorkingStore();

        // We've stolen other's large store, must leave it in a valid state
        other.size_ = 0;
        other.capacity_ = kSmallCapacity;
        other.UpdateWorkingStore();
    }

    template <typename T2>
    void Resize(size_type new_size, const T2 &value) {
        if (new_size < size_) {
            auto working_store = GetWorkingStore();
            for (size_type i = new_size; i < size_; i++) {
                working_store[i].~value_type();
            }
            size_ = new_size;
        } else if (new_size > size_) {
            reserve(new_size);
            // if T2 != T and T is not DefaultInsertable, new values will be undefined
            if constexpr (std::is_same_v<T2, T> || std::is_default_constructible_v<T>) {
                for (size_type i = size_; i < new_size; ++i) {
                    if constexpr (std::is_same_v<T2, T>) {
                        emplace_back(value_type(value));
                    } else if constexpr (std::is_default_constructible_v<T>) {
                        emplace_back(value_type());
                    }
                }
                assert(size() == new_size);
            } else {
                size_ = new_size;
            }
        }
    }
};
