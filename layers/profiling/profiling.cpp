/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#if defined(TRACY_ENABLE)

#include "profiling/profiling.h"

#include <cstdlib>

#if defined(VVL_TRACY_CPU_MEMORY)

void *operator new(std ::size_t size) {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

void *operator new[](std::size_t size) {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete[](void *ptr) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
};

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))

#if defined(_WIN32)
#define vvl_aligned_malloc(size, alignment) _aligned_malloc(size, alignment)
#define vvl_aligned_free(ptr) _aligned_free(ptr)
#else
// TODO - Need to understand why on Linux sometimes calling aligned_alloc causes corruption
void *vvl_aligned_malloc(std::size_t size, std::size_t al) {
    void *mem = malloc(size + al + sizeof(void *));
    void **ptr = (void **)((uintptr_t)((uintptr_t)mem + al + sizeof(void *)) & ~(al - 1));
    ptr[-1] = mem;
    return ptr;
}

void vvl_aligned_free(void *ptr) { free(((void **)ptr)[-1]); }
#endif

void *operator new(std::size_t size, std::align_val_t al) noexcept(false) {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}
void *operator new[](std::size_t size, std::align_val_t al) noexcept(false) {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void *operator new(std::size_t size, std::align_val_t al, const std::nothrow_t &) noexcept {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}
void *operator new[](std::size_t size, std::align_val_t al, const std::nothrow_t &) noexcept {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr, std::align_val_t al) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
void operator delete[](void *ptr, std::align_val_t al) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}

void operator delete(void *ptr, std::align_val_t al, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
void operator delete[](void *ptr, std::align_val_t al, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
#endif

#endif  // #if defined(VVL_TRACY_CPU_MEMORY)

#if TRACY_MANUAL_LIFETIME

#if defined(_WIN32)
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            tracy::StartupProfiler();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#else

__attribute__((constructor)) static void so_attach(void) { tracy::StartupProfiler(); }

#endif  // #if defined(_WIN32)

#endif  // #if TRACY_MANUAL_LIFETIME

#endif  // #if defined(TRACY_ENABLE)