/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/test_common.h"

template <typename T, typename U>
bool HaveSameElementsUpTo(const T& l1, const U& l2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (l1[i] != l2[i]) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
bool HaveSameElements(const T& l1, const U& l2) {
    return l1.size() == l2.size() && HaveSameElementsUpTo(l1, l2, l1.size());
}

TEST(CustomContainer, SmallVectorIntResize) {
    // Resize int small vector, moving to small store
    // ---
    {
        // resize to current size
        small_vector<int, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size(), true);
        std::array ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        small_vector<int, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5, true);
        std::array ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v2, ref));
    }

    {
        // shrinking resize
        small_vector<int, 2, size_t> v3 = {1, 2, 3, 4};
        v3.resize(3, true);
        std::array ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        small_vector<int, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0, true);
        std::array<int, 0> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    {
        // resize to size limit
        small_vector<int, 2, uint8_t> v5 = {1, 2, 3, 4};
        v5.resize(std::numeric_limits<uint8_t>::max(), false);
        std::vector<int> vec = {1, 2, 3, 4};
        vec.resize(std::numeric_limits<uint8_t>::max());
        ASSERT_TRUE(HaveSameElements(v5, vec));
    }

    // Resize int small vector, not moving to small store
    // ---
    {
        // resize to current size
        small_vector<int, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size(), false);
        std::array ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        small_vector<int, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5, false);
        std::array ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v7, ref));
    }

    {
        // shrinking resize
        small_vector<int, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3, false);
        std::array ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        small_vector<int, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0, false);
        std::array<int, 0> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }

    {
        // resize to size limit
        small_vector<int, 2, uint8_t> v10 = {1, 2, 3, 4};
        v10.resize(std::numeric_limits<uint8_t>::max(), false);
        std::vector<int> vec = {1, 2, 3, 4};
        vec.resize(std::numeric_limits<uint8_t>::max());
        ASSERT_TRUE(HaveSameElements(v10, vec));
    }
}

struct NoDefaultCons {
    NoDefaultCons(int x) : x(x) {}
    int x;
};

bool operator!=(const NoDefaultCons& lhs, const NoDefaultCons& rhs) { return lhs.x != rhs.x; }

TEST(CustomContainer, SmallVectorNotDefaultInsertable) {
    // Resize NoDefault small vector, moving to small store
    // ---
    {
        // resize to current size
        small_vector<NoDefaultCons, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size(), true);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        small_vector<NoDefaultCons, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5, true);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElementsUpTo(v2, ref, ref.size()));
    }

    {
        // shrinking resize
        small_vector<NoDefaultCons, 2, size_t> v3 = {1, 2, 3, 4};
        v3.resize(3, true);
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        small_vector<NoDefaultCons, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0, true);
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    // Resize NoDefault small vector, not moving to small store
    // ---
    {
        // resize to current size
        small_vector<NoDefaultCons, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size(), false);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        small_vector<NoDefaultCons, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5, false);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElementsUpTo(v7, ref, ref.size()));
    }

    {
        // shrinking resize
        small_vector<NoDefaultCons, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3, false);
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        small_vector<NoDefaultCons, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0, false);
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }
}

TEST(CustomContainer, SmallVectorNotDefaultInsertableDefaultValue) {
    // Resize NoDefault small vector, moving to small store
    // ---
    {
        // resize to current size
        small_vector<NoDefaultCons, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size(), NoDefaultCons(0), true);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        small_vector<NoDefaultCons, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5, NoDefaultCons(0), true);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v2, ref));
    }

    {
        // shrinking resize
        small_vector<NoDefaultCons, 2, size_t> v3 = {1, 2, 3, 4};
        v3.resize(3, NoDefaultCons(0), true);
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        small_vector<NoDefaultCons, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0, NoDefaultCons(0), true);
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    // Resize NoDefault small vector, not moving to small store
    // ---
    {
        // resize to current size
        small_vector<NoDefaultCons, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size(), false);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        small_vector<NoDefaultCons, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5, NoDefaultCons(0), false);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v7, ref));
    }

    {
        // shrinking resize
        small_vector<NoDefaultCons, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3, NoDefaultCons(0), false);
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        small_vector<NoDefaultCons, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0, NoDefaultCons(0), false);
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }
}