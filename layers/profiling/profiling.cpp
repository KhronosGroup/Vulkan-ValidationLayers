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

#include "profiling/profiling.h"

#include <cstdlib>

#if defined(TRACY_ENABLE)

#include "profiling/profiling.h"

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
#define vvl_aligned_malloc(size, alignment) aligned_alloc(size, alignment)
#define vvl_aligned_free(ptr) free(ptr)
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

#if defined(_WIN32)
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            if (!tracy::IsProfilerStarted()) {
                tracy::StartupProfiler();
            }
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
// Being able to shutdown tracy at dll unload time would be ideal,
// but it does not work, probably a deadlock as described here:
// https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices#deadlocks-caused-by-lock-order-inversion
#if 0
            if (tracy::IsProfilerStarted()){
              tracy::ShutdownProfiler();
            }
#endif
            break;
    }
    return TRUE;
}
#else

__attribute__((constructor)) static void so_attach(void) {
    if (!tracy::IsProfilerStarted()) {
        tracy::StartupProfiler();
    }
}

// Destructor function
__attribute__((destructor)) static void detach(void) {
// Being able to shutdown tracy at dll unload time would be ideal,
// but it does not work
#if 0
            if (tracy::IsProfilerStarted()){
              tracy::ShutdownProfiler();
            }
#endif
}

#endif

#endif
