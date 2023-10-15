// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See safe_struct_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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

#include "vk_safe_struct.h"
#include <vulkan/utility/vk_struct_helper.hpp>
#include "utils/vk_layer_utils.h"

#include <cstddef>
#include <cassert>
#include <cstring>
#include <vector>

#include <vulkan/vk_layer.h>

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const VkSwapchainCreateInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      surface(in_struct->surface),
      minImageCount(in_struct->minImageCount),
      imageFormat(in_struct->imageFormat),
      imageColorSpace(in_struct->imageColorSpace),
      imageExtent(in_struct->imageExtent),
      imageArrayLayers(in_struct->imageArrayLayers),
      imageUsage(in_struct->imageUsage),
      imageSharingMode(in_struct->imageSharingMode),
      queueFamilyIndexCount(0),
      pQueueFamilyIndices(nullptr),
      preTransform(in_struct->preTransform),
      compositeAlpha(in_struct->compositeAlpha),
      presentMode(in_struct->presentMode),
      clipped(in_struct->clipped),
      oldSwapchain(in_struct->oldSwapchain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if ((in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      surface(),
      minImageCount(),
      imageFormat(),
      imageColorSpace(),
      imageExtent(),
      imageArrayLayers(),
      imageUsage(),
      imageSharingMode(),
      queueFamilyIndexCount(),
      pQueueFamilyIndices(nullptr),
      preTransform(),
      compositeAlpha(),
      presentMode(),
      clipped(),
      oldSwapchain() {}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const safe_VkSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    surface = copy_src.surface;
    minImageCount = copy_src.minImageCount;
    imageFormat = copy_src.imageFormat;
    imageColorSpace = copy_src.imageColorSpace;
    imageExtent = copy_src.imageExtent;
    imageArrayLayers = copy_src.imageArrayLayers;
    imageUsage = copy_src.imageUsage;
    imageSharingMode = copy_src.imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src.preTransform;
    compositeAlpha = copy_src.compositeAlpha;
    presentMode = copy_src.presentMode;
    clipped = copy_src.clipped;
    oldSwapchain = copy_src.oldSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkSwapchainCreateInfoKHR& safe_VkSwapchainCreateInfoKHR::operator=(const safe_VkSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    surface = copy_src.surface;
    minImageCount = copy_src.minImageCount;
    imageFormat = copy_src.imageFormat;
    imageColorSpace = copy_src.imageColorSpace;
    imageExtent = copy_src.imageExtent;
    imageArrayLayers = copy_src.imageArrayLayers;
    imageUsage = copy_src.imageUsage;
    imageSharingMode = copy_src.imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src.preTransform;
    compositeAlpha = copy_src.compositeAlpha;
    presentMode = copy_src.presentMode;
    clipped = copy_src.clipped;
    oldSwapchain = copy_src.oldSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }

    return *this;
}

safe_VkSwapchainCreateInfoKHR::~safe_VkSwapchainCreateInfoKHR() {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
}

void safe_VkSwapchainCreateInfoKHR::initialize(const VkSwapchainCreateInfoKHR* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    surface = in_struct->surface;
    minImageCount = in_struct->minImageCount;
    imageFormat = in_struct->imageFormat;
    imageColorSpace = in_struct->imageColorSpace;
    imageExtent = in_struct->imageExtent;
    imageArrayLayers = in_struct->imageArrayLayers;
    imageUsage = in_struct->imageUsage;
    imageSharingMode = in_struct->imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = in_struct->preTransform;
    compositeAlpha = in_struct->compositeAlpha;
    presentMode = in_struct->presentMode;
    clipped = in_struct->clipped;
    oldSwapchain = in_struct->oldSwapchain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if ((in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

void safe_VkSwapchainCreateInfoKHR::initialize(const safe_VkSwapchainCreateInfoKHR* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    surface = copy_src->surface;
    minImageCount = copy_src->minImageCount;
    imageFormat = copy_src->imageFormat;
    imageColorSpace = copy_src->imageColorSpace;
    imageExtent = copy_src->imageExtent;
    imageArrayLayers = copy_src->imageArrayLayers;
    imageUsage = copy_src->imageUsage;
    imageSharingMode = copy_src->imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src->preTransform;
    compositeAlpha = copy_src->compositeAlpha;
    presentMode = copy_src->presentMode;
    clipped = copy_src->clipped;
    oldSwapchain = copy_src->oldSwapchain;
    pNext = SafePnextCopy(copy_src->pNext);

    if ((copy_src->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src->pQueueFamilyIndices,
               sizeof(uint32_t) * copy_src->queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const VkPresentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                             bool copy_pnext)
    : sType(in_struct->sType),
      waitSemaphoreCount(in_struct->waitSemaphoreCount),
      pWaitSemaphores(nullptr),
      swapchainCount(in_struct->swapchainCount),
      pSwapchains(nullptr),
      pImageIndices(nullptr),
      pResults(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }

    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->swapchainCount);
    }

    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy((void*)pResults, (void*)in_struct->pResults, sizeof(VkResult) * in_struct->swapchainCount);
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_INFO_KHR),
      pNext(nullptr),
      waitSemaphoreCount(),
      pWaitSemaphores(nullptr),
      swapchainCount(),
      pSwapchains(nullptr),
      pImageIndices(nullptr),
      pResults(nullptr) {}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const safe_VkPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    waitSemaphoreCount = copy_src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (waitSemaphoreCount && copy_src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src.pSwapchains[i];
        }
    }

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    if (copy_src.pResults) {
        pResults = new VkResult[copy_src.swapchainCount];
        memcpy((void*)pResults, (void*)copy_src.pResults, sizeof(VkResult) * copy_src.swapchainCount);
    }
}

safe_VkPresentInfoKHR& safe_VkPresentInfoKHR::operator=(const safe_VkPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    waitSemaphoreCount = copy_src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (waitSemaphoreCount && copy_src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src.pSwapchains[i];
        }
    }

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    if (copy_src.pResults) {
        pResults = new VkResult[copy_src.swapchainCount];
        memcpy((void*)pResults, (void*)copy_src.pResults, sizeof(VkResult) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentInfoKHR::~safe_VkPresentInfoKHR() {
    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);
}

void safe_VkPresentInfoKHR::initialize(const VkPresentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = in_struct->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }

    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->swapchainCount);
    }

    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy((void*)pResults, (void*)in_struct->pResults, sizeof(VkResult) * in_struct->swapchainCount);
    }
}

void safe_VkPresentInfoKHR::initialize(const safe_VkPresentInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    waitSemaphoreCount = copy_src->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (waitSemaphoreCount && copy_src->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src->pSwapchains[i];
        }
    }

    if (copy_src->pImageIndices) {
        pImageIndices = new uint32_t[copy_src->swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src->pImageIndices, sizeof(uint32_t) * copy_src->swapchainCount);
    }

    if (copy_src->pResults) {
        pResults = new VkResult[copy_src->swapchainCount];
        memcpy((void*)pResults, (void*)copy_src->pResults, sizeof(VkResult) * copy_src->swapchainCount);
    }
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const VkImageSwapchainCreateInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchain(in_struct->swapchain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR), pNext(nullptr), swapchain() {}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const safe_VkImageSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageSwapchainCreateInfoKHR& safe_VkImageSwapchainCreateInfoKHR::operator=(
    const safe_VkImageSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageSwapchainCreateInfoKHR::~safe_VkImageSwapchainCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkImageSwapchainCreateInfoKHR::initialize(const VkImageSwapchainCreateInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageSwapchainCreateInfoKHR::initialize(const safe_VkImageSwapchainCreateInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(const VkBindImageMemorySwapchainInfoKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), swapchain(in_struct->swapchain), imageIndex(in_struct->imageIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR()
    : sType(VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR), pNext(nullptr), swapchain(), imageIndex() {}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(
    const safe_VkBindImageMemorySwapchainInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndex = copy_src.imageIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBindImageMemorySwapchainInfoKHR& safe_VkBindImageMemorySwapchainInfoKHR::operator=(
    const safe_VkBindImageMemorySwapchainInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndex = copy_src.imageIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBindImageMemorySwapchainInfoKHR::~safe_VkBindImageMemorySwapchainInfoKHR() { FreePnextChain(pNext); }

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const VkBindImageMemorySwapchainInfoKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    imageIndex = in_struct->imageIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const safe_VkBindImageMemorySwapchainInfoKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    imageIndex = copy_src->imageIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const VkAcquireNextImageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      swapchain(in_struct->swapchain),
      timeout(in_struct->timeout),
      semaphore(in_struct->semaphore),
      fence(in_struct->fence),
      deviceMask(in_struct->deviceMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR),
      pNext(nullptr),
      swapchain(),
      timeout(),
      semaphore(),
      fence(),
      deviceMask() {}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const safe_VkAcquireNextImageInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    timeout = copy_src.timeout;
    semaphore = copy_src.semaphore;
    fence = copy_src.fence;
    deviceMask = copy_src.deviceMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAcquireNextImageInfoKHR& safe_VkAcquireNextImageInfoKHR::operator=(const safe_VkAcquireNextImageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    timeout = copy_src.timeout;
    semaphore = copy_src.semaphore;
    fence = copy_src.fence;
    deviceMask = copy_src.deviceMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAcquireNextImageInfoKHR::~safe_VkAcquireNextImageInfoKHR() { FreePnextChain(pNext); }

void safe_VkAcquireNextImageInfoKHR::initialize(const VkAcquireNextImageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    timeout = in_struct->timeout;
    semaphore = in_struct->semaphore;
    fence = in_struct->fence;
    deviceMask = in_struct->deviceMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAcquireNextImageInfoKHR::initialize(const safe_VkAcquireNextImageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    timeout = copy_src->timeout;
    semaphore = copy_src->semaphore;
    fence = copy_src->fence;
    deviceMask = copy_src->deviceMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(
    const VkDeviceGroupPresentCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), modes(in_struct->modes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR), pNext(nullptr), modes() {}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(
    const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src.presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR& safe_VkDeviceGroupPresentCapabilitiesKHR::operator=(
    const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src.presentMask[i];
    }

    return *this;
}

safe_VkDeviceGroupPresentCapabilitiesKHR::~safe_VkDeviceGroupPresentCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const VkDeviceGroupPresentCapabilitiesKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    modes = in_struct->modes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const safe_VkDeviceGroupPresentCapabilitiesKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    modes = copy_src->modes;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src->presentMask[i];
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const VkDeviceGroupPresentInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pDeviceMasks(nullptr), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)in_struct->pDeviceMasks, sizeof(uint32_t) * in_struct->swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR), pNext(nullptr), swapchainCount(), pDeviceMasks(nullptr), mode() {}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const safe_VkDeviceGroupPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src.pDeviceMasks, sizeof(uint32_t) * copy_src.swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR& safe_VkDeviceGroupPresentInfoKHR::operator=(const safe_VkDeviceGroupPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src.pDeviceMasks, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkDeviceGroupPresentInfoKHR::~safe_VkDeviceGroupPresentInfoKHR() {
    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const VkDeviceGroupPresentInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pDeviceMasks = nullptr;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)in_struct->pDeviceMasks, sizeof(uint32_t) * in_struct->swapchainCount);
    }
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const safe_VkDeviceGroupPresentInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src->pDeviceMasks, sizeof(uint32_t) * copy_src->swapchainCount);
    }
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(
    const VkDeviceGroupSwapchainCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), modes(in_struct->modes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR), pNext(nullptr), modes() {}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(
    const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceGroupSwapchainCreateInfoKHR& safe_VkDeviceGroupSwapchainCreateInfoKHR::operator=(
    const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::~safe_VkDeviceGroupSwapchainCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    modes = in_struct->modes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const safe_VkDeviceGroupSwapchainCreateInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    modes = copy_src->modes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const VkDisplayModeCreateInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), parameters(in_struct->parameters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR), pNext(nullptr), flags(), parameters() {}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const safe_VkDisplayModeCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    parameters = copy_src.parameters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayModeCreateInfoKHR& safe_VkDisplayModeCreateInfoKHR::operator=(const safe_VkDisplayModeCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    parameters = copy_src.parameters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayModeCreateInfoKHR::~safe_VkDisplayModeCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplayModeCreateInfoKHR::initialize(const VkDisplayModeCreateInfoKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    parameters = in_struct->parameters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayModeCreateInfoKHR::initialize(const safe_VkDisplayModeCreateInfoKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    parameters = copy_src->parameters;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const VkDisplayPropertiesKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : display(in_struct->display),
      physicalDimensions(in_struct->physicalDimensions),
      physicalResolution(in_struct->physicalResolution),
      supportedTransforms(in_struct->supportedTransforms),
      planeReorderPossible(in_struct->planeReorderPossible),
      persistentContent(in_struct->persistentContent) {
    displayName = SafeStringCopy(in_struct->displayName);
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR()
    : display(),
      displayName(nullptr),
      physicalDimensions(),
      physicalResolution(),
      supportedTransforms(),
      planeReorderPossible(),
      persistentContent() {}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const safe_VkDisplayPropertiesKHR& copy_src) {
    display = copy_src.display;
    physicalDimensions = copy_src.physicalDimensions;
    physicalResolution = copy_src.physicalResolution;
    supportedTransforms = copy_src.supportedTransforms;
    planeReorderPossible = copy_src.planeReorderPossible;
    persistentContent = copy_src.persistentContent;
    displayName = SafeStringCopy(copy_src.displayName);
}

safe_VkDisplayPropertiesKHR& safe_VkDisplayPropertiesKHR::operator=(const safe_VkDisplayPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (displayName) delete[] displayName;

    display = copy_src.display;
    physicalDimensions = copy_src.physicalDimensions;
    physicalResolution = copy_src.physicalResolution;
    supportedTransforms = copy_src.supportedTransforms;
    planeReorderPossible = copy_src.planeReorderPossible;
    persistentContent = copy_src.persistentContent;
    displayName = SafeStringCopy(copy_src.displayName);

    return *this;
}

safe_VkDisplayPropertiesKHR::~safe_VkDisplayPropertiesKHR() {
    if (displayName) delete[] displayName;
}

void safe_VkDisplayPropertiesKHR::initialize(const VkDisplayPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (displayName) delete[] displayName;
    display = in_struct->display;
    physicalDimensions = in_struct->physicalDimensions;
    physicalResolution = in_struct->physicalResolution;
    supportedTransforms = in_struct->supportedTransforms;
    planeReorderPossible = in_struct->planeReorderPossible;
    persistentContent = in_struct->persistentContent;
    displayName = SafeStringCopy(in_struct->displayName);
}

void safe_VkDisplayPropertiesKHR::initialize(const safe_VkDisplayPropertiesKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    display = copy_src->display;
    physicalDimensions = copy_src->physicalDimensions;
    physicalResolution = copy_src->physicalResolution;
    supportedTransforms = copy_src->supportedTransforms;
    planeReorderPossible = copy_src->planeReorderPossible;
    persistentContent = copy_src->persistentContent;
    displayName = SafeStringCopy(copy_src->displayName);
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const VkDisplaySurfaceCreateInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      displayMode(in_struct->displayMode),
      planeIndex(in_struct->planeIndex),
      planeStackIndex(in_struct->planeStackIndex),
      transform(in_struct->transform),
      globalAlpha(in_struct->globalAlpha),
      alphaMode(in_struct->alphaMode),
      imageExtent(in_struct->imageExtent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      displayMode(),
      planeIndex(),
      planeStackIndex(),
      transform(),
      globalAlpha(),
      alphaMode(),
      imageExtent() {}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const safe_VkDisplaySurfaceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    displayMode = copy_src.displayMode;
    planeIndex = copy_src.planeIndex;
    planeStackIndex = copy_src.planeStackIndex;
    transform = copy_src.transform;
    globalAlpha = copy_src.globalAlpha;
    alphaMode = copy_src.alphaMode;
    imageExtent = copy_src.imageExtent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplaySurfaceCreateInfoKHR& safe_VkDisplaySurfaceCreateInfoKHR::operator=(
    const safe_VkDisplaySurfaceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    displayMode = copy_src.displayMode;
    planeIndex = copy_src.planeIndex;
    planeStackIndex = copy_src.planeStackIndex;
    transform = copy_src.transform;
    globalAlpha = copy_src.globalAlpha;
    alphaMode = copy_src.alphaMode;
    imageExtent = copy_src.imageExtent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplaySurfaceCreateInfoKHR::~safe_VkDisplaySurfaceCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const VkDisplaySurfaceCreateInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    displayMode = in_struct->displayMode;
    planeIndex = in_struct->planeIndex;
    planeStackIndex = in_struct->planeStackIndex;
    transform = in_struct->transform;
    globalAlpha = in_struct->globalAlpha;
    alphaMode = in_struct->alphaMode;
    imageExtent = in_struct->imageExtent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const safe_VkDisplaySurfaceCreateInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    displayMode = copy_src->displayMode;
    planeIndex = copy_src->planeIndex;
    planeStackIndex = copy_src->planeStackIndex;
    transform = copy_src->transform;
    globalAlpha = copy_src->globalAlpha;
    alphaMode = copy_src->alphaMode;
    imageExtent = copy_src->imageExtent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const VkDisplayPresentInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), srcRect(in_struct->srcRect), dstRect(in_struct->dstRect), persistent(in_struct->persistent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR), pNext(nullptr), srcRect(), dstRect(), persistent() {}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const safe_VkDisplayPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    srcRect = copy_src.srcRect;
    dstRect = copy_src.dstRect;
    persistent = copy_src.persistent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPresentInfoKHR& safe_VkDisplayPresentInfoKHR::operator=(const safe_VkDisplayPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    srcRect = copy_src.srcRect;
    dstRect = copy_src.dstRect;
    persistent = copy_src.persistent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPresentInfoKHR::~safe_VkDisplayPresentInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplayPresentInfoKHR::initialize(const VkDisplayPresentInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    srcRect = in_struct->srcRect;
    dstRect = in_struct->dstRect;
    persistent = in_struct->persistent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPresentInfoKHR::initialize(const safe_VkDisplayPresentInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    srcRect = copy_src->srcRect;
    dstRect = copy_src->dstRect;
    persistent = copy_src->persistent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR(
    const VkQueueFamilyQueryResultStatusPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), queryResultStatusSupport(in_struct->queryResultStatusSupport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR), pNext(nullptr), queryResultStatusSupport() {}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR(
    const safe_VkQueueFamilyQueryResultStatusPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    queryResultStatusSupport = copy_src.queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR& safe_VkQueueFamilyQueryResultStatusPropertiesKHR::operator=(
    const safe_VkQueueFamilyQueryResultStatusPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    queryResultStatusSupport = copy_src.queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::~safe_VkQueueFamilyQueryResultStatusPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyQueryResultStatusPropertiesKHR::initialize(const VkQueueFamilyQueryResultStatusPropertiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queryResultStatusSupport = in_struct->queryResultStatusSupport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyQueryResultStatusPropertiesKHR::initialize(const safe_VkQueueFamilyQueryResultStatusPropertiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queryResultStatusSupport = copy_src->queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR(const VkQueueFamilyVideoPropertiesKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), videoCodecOperations(in_struct->videoCodecOperations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR), pNext(nullptr), videoCodecOperations() {}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR(const safe_VkQueueFamilyVideoPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    videoCodecOperations = copy_src.videoCodecOperations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyVideoPropertiesKHR& safe_VkQueueFamilyVideoPropertiesKHR::operator=(
    const safe_VkQueueFamilyVideoPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoCodecOperations = copy_src.videoCodecOperations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyVideoPropertiesKHR::~safe_VkQueueFamilyVideoPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyVideoPropertiesKHR::initialize(const VkQueueFamilyVideoPropertiesKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoCodecOperations = in_struct->videoCodecOperations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyVideoPropertiesKHR::initialize(const safe_VkQueueFamilyVideoPropertiesKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoCodecOperations = copy_src->videoCodecOperations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR(const VkVideoProfileInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      videoCodecOperation(in_struct->videoCodecOperation),
      chromaSubsampling(in_struct->chromaSubsampling),
      lumaBitDepth(in_struct->lumaBitDepth),
      chromaBitDepth(in_struct->chromaBitDepth) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR),
      pNext(nullptr),
      videoCodecOperation(),
      chromaSubsampling(),
      lumaBitDepth(),
      chromaBitDepth() {}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR(const safe_VkVideoProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoCodecOperation = copy_src.videoCodecOperation;
    chromaSubsampling = copy_src.chromaSubsampling;
    lumaBitDepth = copy_src.lumaBitDepth;
    chromaBitDepth = copy_src.chromaBitDepth;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoProfileInfoKHR& safe_VkVideoProfileInfoKHR::operator=(const safe_VkVideoProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoCodecOperation = copy_src.videoCodecOperation;
    chromaSubsampling = copy_src.chromaSubsampling;
    lumaBitDepth = copy_src.lumaBitDepth;
    chromaBitDepth = copy_src.chromaBitDepth;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoProfileInfoKHR::~safe_VkVideoProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoProfileInfoKHR::initialize(const VkVideoProfileInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoCodecOperation = in_struct->videoCodecOperation;
    chromaSubsampling = in_struct->chromaSubsampling;
    lumaBitDepth = in_struct->lumaBitDepth;
    chromaBitDepth = in_struct->chromaBitDepth;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoProfileInfoKHR::initialize(const safe_VkVideoProfileInfoKHR* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoCodecOperation = copy_src->videoCodecOperation;
    chromaSubsampling = copy_src->chromaSubsampling;
    lumaBitDepth = copy_src->lumaBitDepth;
    chromaBitDepth = copy_src->chromaBitDepth;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR(const VkVideoProfileListInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), profileCount(in_struct->profileCount), pProfiles(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (profileCount && in_struct->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&in_struct->pProfiles[i]);
        }
    }
}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR), pNext(nullptr), profileCount(), pProfiles(nullptr) {}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR(const safe_VkVideoProfileListInfoKHR& copy_src) {
    sType = copy_src.sType;
    profileCount = copy_src.profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (profileCount && copy_src.pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src.pProfiles[i]);
        }
    }
}

safe_VkVideoProfileListInfoKHR& safe_VkVideoProfileListInfoKHR::operator=(const safe_VkVideoProfileListInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    profileCount = copy_src.profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (profileCount && copy_src.pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src.pProfiles[i]);
        }
    }

    return *this;
}

safe_VkVideoProfileListInfoKHR::~safe_VkVideoProfileListInfoKHR() {
    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);
}

void safe_VkVideoProfileListInfoKHR::initialize(const VkVideoProfileListInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    profileCount = in_struct->profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (profileCount && in_struct->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&in_struct->pProfiles[i]);
        }
    }
}

void safe_VkVideoProfileListInfoKHR::initialize(const safe_VkVideoProfileListInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    profileCount = copy_src->profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (profileCount && copy_src->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src->pProfiles[i]);
        }
    }
}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR(const VkVideoCapabilitiesKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      minBitstreamBufferOffsetAlignment(in_struct->minBitstreamBufferOffsetAlignment),
      minBitstreamBufferSizeAlignment(in_struct->minBitstreamBufferSizeAlignment),
      pictureAccessGranularity(in_struct->pictureAccessGranularity),
      minCodedExtent(in_struct->minCodedExtent),
      maxCodedExtent(in_struct->maxCodedExtent),
      maxDpbSlots(in_struct->maxDpbSlots),
      maxActiveReferencePictures(in_struct->maxActiveReferencePictures),
      stdHeaderVersion(in_struct->stdHeaderVersion) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      minBitstreamBufferOffsetAlignment(),
      minBitstreamBufferSizeAlignment(),
      pictureAccessGranularity(),
      minCodedExtent(),
      maxCodedExtent(),
      maxDpbSlots(),
      maxActiveReferencePictures(),
      stdHeaderVersion() {}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR(const safe_VkVideoCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    minBitstreamBufferOffsetAlignment = copy_src.minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src.minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src.pictureAccessGranularity;
    minCodedExtent = copy_src.minCodedExtent;
    maxCodedExtent = copy_src.maxCodedExtent;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    stdHeaderVersion = copy_src.stdHeaderVersion;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoCapabilitiesKHR& safe_VkVideoCapabilitiesKHR::operator=(const safe_VkVideoCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    minBitstreamBufferOffsetAlignment = copy_src.minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src.minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src.pictureAccessGranularity;
    minCodedExtent = copy_src.minCodedExtent;
    maxCodedExtent = copy_src.maxCodedExtent;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    stdHeaderVersion = copy_src.stdHeaderVersion;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoCapabilitiesKHR::~safe_VkVideoCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoCapabilitiesKHR::initialize(const VkVideoCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    minBitstreamBufferOffsetAlignment = in_struct->minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = in_struct->minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = in_struct->pictureAccessGranularity;
    minCodedExtent = in_struct->minCodedExtent;
    maxCodedExtent = in_struct->maxCodedExtent;
    maxDpbSlots = in_struct->maxDpbSlots;
    maxActiveReferencePictures = in_struct->maxActiveReferencePictures;
    stdHeaderVersion = in_struct->stdHeaderVersion;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoCapabilitiesKHR::initialize(const safe_VkVideoCapabilitiesKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    minBitstreamBufferOffsetAlignment = copy_src->minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src->minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src->pictureAccessGranularity;
    minCodedExtent = copy_src->minCodedExtent;
    maxCodedExtent = copy_src->maxCodedExtent;
    maxDpbSlots = copy_src->maxDpbSlots;
    maxActiveReferencePictures = copy_src->maxActiveReferencePictures;
    stdHeaderVersion = copy_src->stdHeaderVersion;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR(
    const VkPhysicalDeviceVideoFormatInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageUsage(in_struct->imageUsage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR), pNext(nullptr), imageUsage() {}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR(
    const safe_VkPhysicalDeviceVideoFormatInfoKHR& copy_src) {
    sType = copy_src.sType;
    imageUsage = copy_src.imageUsage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoFormatInfoKHR& safe_VkPhysicalDeviceVideoFormatInfoKHR::operator=(
    const safe_VkPhysicalDeviceVideoFormatInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageUsage = copy_src.imageUsage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::~safe_VkPhysicalDeviceVideoFormatInfoKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoFormatInfoKHR::initialize(const VkPhysicalDeviceVideoFormatInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageUsage = in_struct->imageUsage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoFormatInfoKHR::initialize(const safe_VkPhysicalDeviceVideoFormatInfoKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageUsage = copy_src->imageUsage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR(const VkVideoFormatPropertiesKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      format(in_struct->format),
      componentMapping(in_struct->componentMapping),
      imageCreateFlags(in_struct->imageCreateFlags),
      imageType(in_struct->imageType),
      imageTiling(in_struct->imageTiling),
      imageUsageFlags(in_struct->imageUsageFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR),
      pNext(nullptr),
      format(),
      componentMapping(),
      imageCreateFlags(),
      imageType(),
      imageTiling(),
      imageUsageFlags() {}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR(const safe_VkVideoFormatPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    componentMapping = copy_src.componentMapping;
    imageCreateFlags = copy_src.imageCreateFlags;
    imageType = copy_src.imageType;
    imageTiling = copy_src.imageTiling;
    imageUsageFlags = copy_src.imageUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoFormatPropertiesKHR& safe_VkVideoFormatPropertiesKHR::operator=(const safe_VkVideoFormatPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    componentMapping = copy_src.componentMapping;
    imageCreateFlags = copy_src.imageCreateFlags;
    imageType = copy_src.imageType;
    imageTiling = copy_src.imageTiling;
    imageUsageFlags = copy_src.imageUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoFormatPropertiesKHR::~safe_VkVideoFormatPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoFormatPropertiesKHR::initialize(const VkVideoFormatPropertiesKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    componentMapping = in_struct->componentMapping;
    imageCreateFlags = in_struct->imageCreateFlags;
    imageType = in_struct->imageType;
    imageTiling = in_struct->imageTiling;
    imageUsageFlags = in_struct->imageUsageFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoFormatPropertiesKHR::initialize(const safe_VkVideoFormatPropertiesKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    componentMapping = copy_src->componentMapping;
    imageCreateFlags = copy_src->imageCreateFlags;
    imageType = copy_src->imageType;
    imageTiling = copy_src->imageTiling;
    imageUsageFlags = copy_src->imageUsageFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR(const VkVideoPictureResourceInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      codedOffset(in_struct->codedOffset),
      codedExtent(in_struct->codedExtent),
      baseArrayLayer(in_struct->baseArrayLayer),
      imageViewBinding(in_struct->imageViewBinding) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR),
      pNext(nullptr),
      codedOffset(),
      codedExtent(),
      baseArrayLayer(),
      imageViewBinding() {}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR(const safe_VkVideoPictureResourceInfoKHR& copy_src) {
    sType = copy_src.sType;
    codedOffset = copy_src.codedOffset;
    codedExtent = copy_src.codedExtent;
    baseArrayLayer = copy_src.baseArrayLayer;
    imageViewBinding = copy_src.imageViewBinding;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoPictureResourceInfoKHR& safe_VkVideoPictureResourceInfoKHR::operator=(
    const safe_VkVideoPictureResourceInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    codedOffset = copy_src.codedOffset;
    codedExtent = copy_src.codedExtent;
    baseArrayLayer = copy_src.baseArrayLayer;
    imageViewBinding = copy_src.imageViewBinding;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoPictureResourceInfoKHR::~safe_VkVideoPictureResourceInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoPictureResourceInfoKHR::initialize(const VkVideoPictureResourceInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    codedOffset = in_struct->codedOffset;
    codedExtent = in_struct->codedExtent;
    baseArrayLayer = in_struct->baseArrayLayer;
    imageViewBinding = in_struct->imageViewBinding;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoPictureResourceInfoKHR::initialize(const safe_VkVideoPictureResourceInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    codedOffset = copy_src->codedOffset;
    codedExtent = copy_src->codedExtent;
    baseArrayLayer = copy_src->baseArrayLayer;
    imageViewBinding = copy_src->imageViewBinding;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR(const VkVideoReferenceSlotInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), slotIndex(in_struct->slotIndex), pPictureResource(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(in_struct->pPictureResource);
}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR), pNext(nullptr), slotIndex(), pPictureResource(nullptr) {}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR(const safe_VkVideoReferenceSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    slotIndex = copy_src.slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src.pPictureResource);
}

safe_VkVideoReferenceSlotInfoKHR& safe_VkVideoReferenceSlotInfoKHR::operator=(const safe_VkVideoReferenceSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    slotIndex = copy_src.slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src.pPictureResource);

    return *this;
}

safe_VkVideoReferenceSlotInfoKHR::~safe_VkVideoReferenceSlotInfoKHR() {
    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);
}

void safe_VkVideoReferenceSlotInfoKHR::initialize(const VkVideoReferenceSlotInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    slotIndex = in_struct->slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(in_struct->pPictureResource);
}

void safe_VkVideoReferenceSlotInfoKHR::initialize(const safe_VkVideoReferenceSlotInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    slotIndex = copy_src->slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src->pPictureResource);
}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR(
    const VkVideoSessionMemoryRequirementsKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryBindIndex(in_struct->memoryBindIndex), memoryRequirements(in_struct->memoryRequirements) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR), pNext(nullptr), memoryBindIndex(), memoryRequirements() {}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR(
    const safe_VkVideoSessionMemoryRequirementsKHR& copy_src) {
    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memoryRequirements = copy_src.memoryRequirements;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionMemoryRequirementsKHR& safe_VkVideoSessionMemoryRequirementsKHR::operator=(
    const safe_VkVideoSessionMemoryRequirementsKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memoryRequirements = copy_src.memoryRequirements;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionMemoryRequirementsKHR::~safe_VkVideoSessionMemoryRequirementsKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionMemoryRequirementsKHR::initialize(const VkVideoSessionMemoryRequirementsKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryBindIndex = in_struct->memoryBindIndex;
    memoryRequirements = in_struct->memoryRequirements;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionMemoryRequirementsKHR::initialize(const safe_VkVideoSessionMemoryRequirementsKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryBindIndex = copy_src->memoryBindIndex;
    memoryRequirements = copy_src->memoryRequirements;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR(const VkBindVideoSessionMemoryInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      memoryBindIndex(in_struct->memoryBindIndex),
      memory(in_struct->memory),
      memoryOffset(in_struct->memoryOffset),
      memorySize(in_struct->memorySize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR),
      pNext(nullptr),
      memoryBindIndex(),
      memory(),
      memoryOffset(),
      memorySize() {}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR(const safe_VkBindVideoSessionMemoryInfoKHR& copy_src) {
    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    memorySize = copy_src.memorySize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBindVideoSessionMemoryInfoKHR& safe_VkBindVideoSessionMemoryInfoKHR::operator=(
    const safe_VkBindVideoSessionMemoryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    memorySize = copy_src.memorySize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBindVideoSessionMemoryInfoKHR::~safe_VkBindVideoSessionMemoryInfoKHR() { FreePnextChain(pNext); }

void safe_VkBindVideoSessionMemoryInfoKHR::initialize(const VkBindVideoSessionMemoryInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryBindIndex = in_struct->memoryBindIndex;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
    memorySize = in_struct->memorySize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBindVideoSessionMemoryInfoKHR::initialize(const safe_VkBindVideoSessionMemoryInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryBindIndex = copy_src->memoryBindIndex;
    memory = copy_src->memory;
    memoryOffset = copy_src->memoryOffset;
    memorySize = copy_src->memorySize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR(const VkVideoSessionCreateInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      queueFamilyIndex(in_struct->queueFamilyIndex),
      flags(in_struct->flags),
      pVideoProfile(nullptr),
      pictureFormat(in_struct->pictureFormat),
      maxCodedExtent(in_struct->maxCodedExtent),
      referencePictureFormat(in_struct->referencePictureFormat),
      maxDpbSlots(in_struct->maxDpbSlots),
      maxActiveReferencePictures(in_struct->maxActiveReferencePictures),
      pStdHeaderVersion(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);

    if (in_struct->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*in_struct->pStdHeaderVersion);
    }
}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR),
      pNext(nullptr),
      queueFamilyIndex(),
      flags(),
      pVideoProfile(nullptr),
      pictureFormat(),
      maxCodedExtent(),
      referencePictureFormat(),
      maxDpbSlots(),
      maxActiveReferencePictures(),
      pStdHeaderVersion(nullptr) {}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR(const safe_VkVideoSessionCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    flags = copy_src.flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src.pictureFormat;
    maxCodedExtent = copy_src.maxCodedExtent;
    referencePictureFormat = copy_src.referencePictureFormat;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    if (copy_src.pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src.pStdHeaderVersion);
    }
}

safe_VkVideoSessionCreateInfoKHR& safe_VkVideoSessionCreateInfoKHR::operator=(const safe_VkVideoSessionCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    flags = copy_src.flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src.pictureFormat;
    maxCodedExtent = copy_src.maxCodedExtent;
    referencePictureFormat = copy_src.referencePictureFormat;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    if (copy_src.pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src.pStdHeaderVersion);
    }

    return *this;
}

safe_VkVideoSessionCreateInfoKHR::~safe_VkVideoSessionCreateInfoKHR() {
    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);
}

void safe_VkVideoSessionCreateInfoKHR::initialize(const VkVideoSessionCreateInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    flags = in_struct->flags;
    pVideoProfile = nullptr;
    pictureFormat = in_struct->pictureFormat;
    maxCodedExtent = in_struct->maxCodedExtent;
    referencePictureFormat = in_struct->referencePictureFormat;
    maxDpbSlots = in_struct->maxDpbSlots;
    maxActiveReferencePictures = in_struct->maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);

    if (in_struct->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*in_struct->pStdHeaderVersion);
    }
}

void safe_VkVideoSessionCreateInfoKHR::initialize(const safe_VkVideoSessionCreateInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queueFamilyIndex = copy_src->queueFamilyIndex;
    flags = copy_src->flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src->pictureFormat;
    maxCodedExtent = copy_src->maxCodedExtent;
    referencePictureFormat = copy_src->referencePictureFormat;
    maxDpbSlots = copy_src->maxDpbSlots;
    maxActiveReferencePictures = copy_src->maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src->pVideoProfile);

    if (copy_src->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src->pStdHeaderVersion);
    }
}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR(
    const VkVideoSessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      videoSessionParametersTemplate(in_struct->videoSessionParametersTemplate),
      videoSession(in_struct->videoSession) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      videoSessionParametersTemplate(),
      videoSession() {}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR(
    const safe_VkVideoSessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSessionParametersTemplate = copy_src.videoSessionParametersTemplate;
    videoSession = copy_src.videoSession;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionParametersCreateInfoKHR& safe_VkVideoSessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoSessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSessionParametersTemplate = copy_src.videoSessionParametersTemplate;
    videoSession = copy_src.videoSession;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionParametersCreateInfoKHR::~safe_VkVideoSessionParametersCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionParametersCreateInfoKHR::initialize(const VkVideoSessionParametersCreateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    videoSessionParametersTemplate = in_struct->videoSessionParametersTemplate;
    videoSession = in_struct->videoSession;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionParametersCreateInfoKHR::initialize(const safe_VkVideoSessionParametersCreateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    videoSessionParametersTemplate = copy_src->videoSessionParametersTemplate;
    videoSession = copy_src->videoSession;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR(
    const VkVideoSessionParametersUpdateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), updateSequenceCount(in_struct->updateSequenceCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR), pNext(nullptr), updateSequenceCount() {}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR(
    const safe_VkVideoSessionParametersUpdateInfoKHR& copy_src) {
    sType = copy_src.sType;
    updateSequenceCount = copy_src.updateSequenceCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionParametersUpdateInfoKHR& safe_VkVideoSessionParametersUpdateInfoKHR::operator=(
    const safe_VkVideoSessionParametersUpdateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    updateSequenceCount = copy_src.updateSequenceCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionParametersUpdateInfoKHR::~safe_VkVideoSessionParametersUpdateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionParametersUpdateInfoKHR::initialize(const VkVideoSessionParametersUpdateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    updateSequenceCount = in_struct->updateSequenceCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionParametersUpdateInfoKHR::initialize(const safe_VkVideoSessionParametersUpdateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    updateSequenceCount = copy_src->updateSequenceCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR(const VkVideoBeginCodingInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      videoSession(in_struct->videoSession),
      videoSessionParameters(in_struct->videoSessionParameters),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR),
      pNext(nullptr),
      flags(),
      videoSession(),
      videoSessionParameters(),
      referenceSlotCount(),
      pReferenceSlots(nullptr) {}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR(const safe_VkVideoBeginCodingInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSession = copy_src.videoSession;
    videoSessionParameters = copy_src.videoSessionParameters;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoBeginCodingInfoKHR& safe_VkVideoBeginCodingInfoKHR::operator=(const safe_VkVideoBeginCodingInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSession = copy_src.videoSession;
    videoSessionParameters = copy_src.videoSessionParameters;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoBeginCodingInfoKHR::~safe_VkVideoBeginCodingInfoKHR() {
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoBeginCodingInfoKHR::initialize(const VkVideoBeginCodingInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    videoSession = in_struct->videoSession;
    videoSessionParameters = in_struct->videoSessionParameters;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoBeginCodingInfoKHR::initialize(const safe_VkVideoBeginCodingInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    videoSession = copy_src->videoSession;
    videoSessionParameters = copy_src->videoSessionParameters;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR(const VkVideoEndCodingInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR), pNext(nullptr), flags() {}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR(const safe_VkVideoEndCodingInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEndCodingInfoKHR& safe_VkVideoEndCodingInfoKHR::operator=(const safe_VkVideoEndCodingInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEndCodingInfoKHR::~safe_VkVideoEndCodingInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEndCodingInfoKHR::initialize(const VkVideoEndCodingInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEndCodingInfoKHR::initialize(const safe_VkVideoEndCodingInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR(const VkVideoCodingControlInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR), pNext(nullptr), flags() {}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR(const safe_VkVideoCodingControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoCodingControlInfoKHR& safe_VkVideoCodingControlInfoKHR::operator=(const safe_VkVideoCodingControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoCodingControlInfoKHR::~safe_VkVideoCodingControlInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoCodingControlInfoKHR::initialize(const VkVideoCodingControlInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoCodingControlInfoKHR::initialize(const safe_VkVideoCodingControlInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR(const VkVideoDecodeCapabilitiesKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR), pNext(nullptr), flags() {}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR(const safe_VkVideoDecodeCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeCapabilitiesKHR& safe_VkVideoDecodeCapabilitiesKHR::operator=(const safe_VkVideoDecodeCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeCapabilitiesKHR::~safe_VkVideoDecodeCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeCapabilitiesKHR::initialize(const VkVideoDecodeCapabilitiesKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeCapabilitiesKHR::initialize(const safe_VkVideoDecodeCapabilitiesKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR(const VkVideoDecodeUsageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoUsageHints(in_struct->videoUsageHints) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR), pNext(nullptr), videoUsageHints() {}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR(const safe_VkVideoDecodeUsageInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeUsageInfoKHR& safe_VkVideoDecodeUsageInfoKHR::operator=(const safe_VkVideoDecodeUsageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeUsageInfoKHR::~safe_VkVideoDecodeUsageInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeUsageInfoKHR::initialize(const VkVideoDecodeUsageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoUsageHints = in_struct->videoUsageHints;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeUsageInfoKHR::initialize(const safe_VkVideoDecodeUsageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoUsageHints = copy_src->videoUsageHints;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR(const VkVideoDecodeInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      srcBuffer(in_struct->srcBuffer),
      srcBufferOffset(in_struct->srcBufferOffset),
      srcBufferRange(in_struct->srcBufferRange),
      dstPictureResource(&in_struct->dstPictureResource),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR),
      pNext(nullptr),
      flags(),
      srcBuffer(),
      srcBufferOffset(),
      srcBufferRange(),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(),
      pReferenceSlots(nullptr) {}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR(const safe_VkVideoDecodeInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    srcBuffer = copy_src.srcBuffer;
    srcBufferOffset = copy_src.srcBufferOffset;
    srcBufferRange = copy_src.srcBufferRange;
    dstPictureResource.initialize(&copy_src.dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoDecodeInfoKHR& safe_VkVideoDecodeInfoKHR::operator=(const safe_VkVideoDecodeInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    srcBuffer = copy_src.srcBuffer;
    srcBufferOffset = copy_src.srcBufferOffset;
    srcBufferRange = copy_src.srcBufferRange;
    dstPictureResource.initialize(&copy_src.dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoDecodeInfoKHR::~safe_VkVideoDecodeInfoKHR() {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeInfoKHR::initialize(const VkVideoDecodeInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    srcBuffer = in_struct->srcBuffer;
    srcBufferOffset = in_struct->srcBufferOffset;
    srcBufferRange = in_struct->srcBufferRange;
    dstPictureResource.initialize(&in_struct->dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoDecodeInfoKHR::initialize(const safe_VkVideoDecodeInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    srcBuffer = copy_src->srcBuffer;
    srcBufferOffset = copy_src->srcBufferOffset;
    srcBufferRange = copy_src->srcBufferRange;
    dstPictureResource.initialize(&copy_src->dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src->pSetupReferenceSlot);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR(const VkVideoDecodeH264ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc), pictureLayout(in_struct->pictureLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc(), pictureLayout() {}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR(const safe_VkVideoDecodeH264ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pictureLayout = copy_src.pictureLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH264ProfileInfoKHR& safe_VkVideoDecodeH264ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeH264ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pictureLayout = copy_src.pictureLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH264ProfileInfoKHR::~safe_VkVideoDecodeH264ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH264ProfileInfoKHR::initialize(const VkVideoDecodeH264ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pictureLayout = in_struct->pictureLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH264ProfileInfoKHR::initialize(const safe_VkVideoDecodeH264ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pictureLayout = copy_src->pictureLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR(const VkVideoDecodeH264CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), maxLevelIdc(in_struct->maxLevelIdc), fieldOffsetGranularity(in_struct->fieldOffsetGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR), pNext(nullptr), maxLevelIdc(), fieldOffsetGranularity() {}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR(
    const safe_VkVideoDecodeH264CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    fieldOffsetGranularity = copy_src.fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH264CapabilitiesKHR& safe_VkVideoDecodeH264CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeH264CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    fieldOffsetGranularity = copy_src.fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH264CapabilitiesKHR::~safe_VkVideoDecodeH264CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH264CapabilitiesKHR::initialize(const VkVideoDecodeH264CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevelIdc = in_struct->maxLevelIdc;
    fieldOffsetGranularity = in_struct->fieldOffsetGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH264CapabilitiesKHR::initialize(const safe_VkVideoDecodeH264CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevelIdc = copy_src->maxLevelIdc;
    fieldOffsetGranularity = copy_src->fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR(
    const VkVideoDecodeH264SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR& safe_VkVideoDecodeH264SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::~safe_VkVideoDecodeH264SessionParametersAddInfoKHR() {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264SessionParametersAddInfoKHR::initialize(const VkVideoDecodeH264SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoDecodeH264SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(
    const VkVideoDecodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::~safe_VkVideoDecodeH264SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::initialize(
    const VkVideoDecodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR(const VkVideoDecodeH264PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdPictureInfo(nullptr), sliceCount(in_struct->sliceCount), pSliceOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceOffsets) {
        pSliceOffsets = new uint32_t[in_struct->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)in_struct->pSliceOffsets, sizeof(uint32_t) * in_struct->sliceCount);
    }
}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      sliceCount(),
      pSliceOffsets(nullptr) {}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR(const safe_VkVideoDecodeH264PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src.sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src.sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src.pSliceOffsets, sizeof(uint32_t) * copy_src.sliceCount);
    }
}

safe_VkVideoDecodeH264PictureInfoKHR& safe_VkVideoDecodeH264PictureInfoKHR::operator=(
    const safe_VkVideoDecodeH264PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src.sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src.sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src.pSliceOffsets, sizeof(uint32_t) * copy_src.sliceCount);
    }

    return *this;
}

safe_VkVideoDecodeH264PictureInfoKHR::~safe_VkVideoDecodeH264PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264PictureInfoKHR::initialize(const VkVideoDecodeH264PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    sliceCount = in_struct->sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceOffsets) {
        pSliceOffsets = new uint32_t[in_struct->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)in_struct->pSliceOffsets, sizeof(uint32_t) * in_struct->sliceCount);
    }
}

void safe_VkVideoDecodeH264PictureInfoKHR::initialize(const safe_VkVideoDecodeH264PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src->sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src->pStdPictureInfo);
    }

    if (copy_src->pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src->pSliceOffsets, sizeof(uint32_t) * copy_src->sliceCount);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR(const VkVideoDecodeH264DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR(const safe_VkVideoDecodeH264DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR& safe_VkVideoDecodeH264DpbSlotInfoKHR::operator=(
    const safe_VkVideoDecodeH264DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::~safe_VkVideoDecodeH264DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264DpbSlotInfoKHR::initialize(const VkVideoDecodeH264DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoDecodeH264DpbSlotInfoKHR::initialize(const safe_VkVideoDecodeH264DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(
    const VkRenderingFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      imageView(in_struct->imageView),
      imageLayout(in_struct->imageLayout),
      shadingRateAttachmentTexelSize(in_struct->shadingRateAttachmentTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR),
      pNext(nullptr),
      imageView(),
      imageLayout(),
      shadingRateAttachmentTexelSize() {}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& copy_src) {
    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::operator=(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::~safe_VkRenderingFragmentShadingRateAttachmentInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::initialize(
    const VkRenderingFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageView = in_struct->imageView;
    imageLayout = in_struct->imageLayout;
    shadingRateAttachmentTexelSize = in_struct->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::initialize(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageView = copy_src->imageView;
    imageLayout = copy_src->imageLayout;
    shadingRateAttachmentTexelSize = copy_src->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(const VkImportMemoryWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), handle(in_struct->handle), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR), pNext(nullptr), handleType(), handle(), name() {}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(
    const safe_VkImportMemoryWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryWin32HandleInfoKHR& safe_VkImportMemoryWin32HandleInfoKHR::operator=(
    const safe_VkImportMemoryWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryWin32HandleInfoKHR::~safe_VkImportMemoryWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const VkImportMemoryWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const safe_VkImportMemoryWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(const VkExportMemoryWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(
    const safe_VkExportMemoryWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR& safe_VkExportMemoryWin32HandleInfoKHR::operator=(
    const safe_VkExportMemoryWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportMemoryWin32HandleInfoKHR::~safe_VkExportMemoryWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const VkExportMemoryWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const safe_VkExportMemoryWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(const VkMemoryWin32HandlePropertiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(
    const safe_VkMemoryWin32HandlePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryWin32HandlePropertiesKHR& safe_VkMemoryWin32HandlePropertiesKHR::operator=(
    const safe_VkMemoryWin32HandlePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryWin32HandlePropertiesKHR::~safe_VkMemoryWin32HandlePropertiesKHR() { FreePnextChain(pNext); }

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const VkMemoryWin32HandlePropertiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const safe_VkMemoryWin32HandlePropertiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const VkMemoryGetWin32HandleInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const safe_VkMemoryGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetWin32HandleInfoKHR& safe_VkMemoryGetWin32HandleInfoKHR::operator=(
    const safe_VkMemoryGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetWin32HandleInfoKHR::~safe_VkMemoryGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const VkMemoryGetWin32HandleInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const safe_VkMemoryGetWin32HandleInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const VkImportMemoryFdInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR), pNext(nullptr), handleType(), fd() {}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const safe_VkImportMemoryFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryFdInfoKHR& safe_VkImportMemoryFdInfoKHR::operator=(const safe_VkImportMemoryFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryFdInfoKHR::~safe_VkImportMemoryFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportMemoryFdInfoKHR::initialize(const VkImportMemoryFdInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryFdInfoKHR::initialize(const safe_VkImportMemoryFdInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const VkMemoryFdPropertiesKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const safe_VkMemoryFdPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryFdPropertiesKHR& safe_VkMemoryFdPropertiesKHR::operator=(const safe_VkMemoryFdPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryFdPropertiesKHR::~safe_VkMemoryFdPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkMemoryFdPropertiesKHR::initialize(const VkMemoryFdPropertiesKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryFdPropertiesKHR::initialize(const safe_VkMemoryFdPropertiesKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const VkMemoryGetFdInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const safe_VkMemoryGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetFdInfoKHR& safe_VkMemoryGetFdInfoKHR::operator=(const safe_VkMemoryGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetFdInfoKHR::~safe_VkMemoryGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryGetFdInfoKHR::initialize(const VkMemoryGetFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetFdInfoKHR::initialize(const safe_VkMemoryGetFdInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(
    const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      acquireCount(in_struct->acquireCount),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeouts(nullptr),
      releaseCount(in_struct->releaseCount),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)in_struct->pAcquireTimeouts, sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR()
    : sType(VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR),
      pNext(nullptr),
      acquireCount(),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeouts(nullptr),
      releaseCount(),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& copy_src) {
    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src.pAcquireTimeouts, sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::operator=(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src.pAcquireTimeouts, sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }

    return *this;
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::~safe_VkWin32KeyedMutexAcquireReleaseInfoKHR() {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    acquireCount = in_struct->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = in_struct->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)in_struct->pAcquireTimeouts, sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    acquireCount = copy_src->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (acquireCount && copy_src->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src->pAcquireSyncs[i];
        }
    }

    if (copy_src->pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src->pAcquireKeys, sizeof(uint64_t) * copy_src->acquireCount);
    }

    if (copy_src->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src->pAcquireTimeouts, sizeof(uint32_t) * copy_src->acquireCount);
    }
    if (releaseCount && copy_src->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src->pReleaseSyncs[i];
        }
    }

    if (copy_src->pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src->pReleaseKeys, sizeof(uint64_t) * copy_src->releaseCount);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(
    const VkImportSemaphoreWin32HandleInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      semaphore(in_struct->semaphore),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      handle(in_struct->handle),
      name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR),
      pNext(nullptr),
      semaphore(),
      flags(),
      handleType(),
      handle(),
      name() {}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(
    const safe_VkImportSemaphoreWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportSemaphoreWin32HandleInfoKHR& safe_VkImportSemaphoreWin32HandleInfoKHR::operator=(
    const safe_VkImportSemaphoreWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportSemaphoreWin32HandleInfoKHR::~safe_VkImportSemaphoreWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const VkImportSemaphoreWin32HandleInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkImportSemaphoreWin32HandleInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(
    const VkExportSemaphoreWin32HandleInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(
    const safe_VkExportSemaphoreWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR& safe_VkExportSemaphoreWin32HandleInfoKHR::operator=(
    const safe_VkExportSemaphoreWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportSemaphoreWin32HandleInfoKHR::~safe_VkExportSemaphoreWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const VkExportSemaphoreWin32HandleInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkExportSemaphoreWin32HandleInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const VkD3D12FenceSubmitInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      waitSemaphoreValuesCount(in_struct->waitSemaphoreValuesCount),
      pWaitSemaphoreValues(nullptr),
      signalSemaphoreValuesCount(in_struct->signalSemaphoreValuesCount),
      pSignalSemaphoreValues(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)in_struct->pWaitSemaphoreValues,
               sizeof(uint64_t) * in_struct->waitSemaphoreValuesCount);
    }

    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)in_struct->pSignalSemaphoreValues,
               sizeof(uint64_t) * in_struct->signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR()
    : sType(VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR),
      pNext(nullptr),
      waitSemaphoreValuesCount(),
      pWaitSemaphoreValues(nullptr),
      signalSemaphoreValuesCount(),
      pSignalSemaphoreValues(nullptr) {}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const safe_VkD3D12FenceSubmitInfoKHR& copy_src) {
    sType = copy_src.sType;
    waitSemaphoreValuesCount = copy_src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src.waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src.pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src.waitSemaphoreValuesCount);
    }

    if (copy_src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src.signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src.pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src.signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR& safe_VkD3D12FenceSubmitInfoKHR::operator=(const safe_VkD3D12FenceSubmitInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    waitSemaphoreValuesCount = copy_src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src.waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src.pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src.waitSemaphoreValuesCount);
    }

    if (copy_src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src.signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src.pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src.signalSemaphoreValuesCount);
    }

    return *this;
}

safe_VkD3D12FenceSubmitInfoKHR::~safe_VkD3D12FenceSubmitInfoKHR() {
    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const VkD3D12FenceSubmitInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    waitSemaphoreValuesCount = in_struct->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = in_struct->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)in_struct->pWaitSemaphoreValues,
               sizeof(uint64_t) * in_struct->waitSemaphoreValuesCount);
    }

    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)in_struct->pSignalSemaphoreValues,
               sizeof(uint64_t) * in_struct->signalSemaphoreValuesCount);
    }
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const safe_VkD3D12FenceSubmitInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    waitSemaphoreValuesCount = copy_src->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src->pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src->waitSemaphoreValuesCount);
    }

    if (copy_src->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src->pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src->signalSemaphoreValuesCount);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(const VkSemaphoreGetWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), semaphore(), handleType() {}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(
    const safe_VkSemaphoreGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSemaphoreGetWin32HandleInfoKHR& safe_VkSemaphoreGetWin32HandleInfoKHR::operator=(
    const safe_VkSemaphoreGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSemaphoreGetWin32HandleInfoKHR::~safe_VkSemaphoreGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const VkSemaphoreGetWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const safe_VkSemaphoreGetWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const VkImportSemaphoreFdInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      semaphore(in_struct->semaphore),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR), pNext(nullptr), semaphore(), flags(), handleType(), fd() {}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const safe_VkImportSemaphoreFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportSemaphoreFdInfoKHR& safe_VkImportSemaphoreFdInfoKHR::operator=(const safe_VkImportSemaphoreFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportSemaphoreFdInfoKHR::~safe_VkImportSemaphoreFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportSemaphoreFdInfoKHR::initialize(const VkImportSemaphoreFdInfoKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportSemaphoreFdInfoKHR::initialize(const safe_VkImportSemaphoreFdInfoKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const VkSemaphoreGetFdInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR), pNext(nullptr), semaphore(), handleType() {}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const safe_VkSemaphoreGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSemaphoreGetFdInfoKHR& safe_VkSemaphoreGetFdInfoKHR::operator=(const safe_VkSemaphoreGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSemaphoreGetFdInfoKHR::~safe_VkSemaphoreGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkSemaphoreGetFdInfoKHR::initialize(const VkSemaphoreGetFdInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSemaphoreGetFdInfoKHR::initialize(const safe_VkSemaphoreGetFdInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR(
    const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxPushDescriptors(in_struct->maxPushDescriptors) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR), pNext(nullptr), maxPushDescriptors() {}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR(
    const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    maxPushDescriptors = copy_src.maxPushDescriptors;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR& safe_VkPhysicalDevicePushDescriptorPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxPushDescriptors = copy_src.maxPushDescriptors;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::~safe_VkPhysicalDevicePushDescriptorPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePushDescriptorPropertiesKHR::initialize(const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxPushDescriptors = in_struct->maxPushDescriptors;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePushDescriptorPropertiesKHR::initialize(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxPushDescriptors = copy_src->maxPushDescriptors;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const VkPresentRegionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state)
    : rectangleCount(in_struct->rectangleCount), pRectangles(nullptr) {
    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy((void*)pRectangles, (void*)in_struct->pRectangles, sizeof(VkRectLayerKHR) * in_struct->rectangleCount);
    }
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR() : rectangleCount(), pRectangles(nullptr) {}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const safe_VkPresentRegionKHR& copy_src) {
    rectangleCount = copy_src.rectangleCount;
    pRectangles = nullptr;

    if (copy_src.pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src.rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src.pRectangles, sizeof(VkRectLayerKHR) * copy_src.rectangleCount);
    }
}

safe_VkPresentRegionKHR& safe_VkPresentRegionKHR::operator=(const safe_VkPresentRegionKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pRectangles) delete[] pRectangles;

    rectangleCount = copy_src.rectangleCount;
    pRectangles = nullptr;

    if (copy_src.pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src.rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src.pRectangles, sizeof(VkRectLayerKHR) * copy_src.rectangleCount);
    }

    return *this;
}

safe_VkPresentRegionKHR::~safe_VkPresentRegionKHR() {
    if (pRectangles) delete[] pRectangles;
}

void safe_VkPresentRegionKHR::initialize(const VkPresentRegionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRectangles) delete[] pRectangles;
    rectangleCount = in_struct->rectangleCount;
    pRectangles = nullptr;

    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy((void*)pRectangles, (void*)in_struct->pRectangles, sizeof(VkRectLayerKHR) * in_struct->rectangleCount);
    }
}

void safe_VkPresentRegionKHR::initialize(const safe_VkPresentRegionKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    rectangleCount = copy_src->rectangleCount;
    pRectangles = nullptr;

    if (copy_src->pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src->rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src->pRectangles, sizeof(VkRectLayerKHR) * copy_src->rectangleCount);
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const VkPresentRegionsKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pRegions(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR), pNext(nullptr), swapchainCount(), pRegions(nullptr) {}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const safe_VkPresentRegionsKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src.pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR& safe_VkPresentRegionsKHR::operator=(const safe_VkPresentRegionsKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src.pRegions[i]);
        }
    }

    return *this;
}

safe_VkPresentRegionsKHR::~safe_VkPresentRegionsKHR() {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
}

void safe_VkPresentRegionsKHR::initialize(const VkPresentRegionsKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

void safe_VkPresentRegionsKHR::initialize(const safe_VkPresentRegionsKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (swapchainCount && copy_src->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src->pRegions[i]);
        }
    }
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(
    const VkSharedPresentSurfaceCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), sharedPresentSupportedUsageFlags(in_struct->sharedPresentSupportedUsageFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR), pNext(nullptr), sharedPresentSupportedUsageFlags() {}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(
    const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    sharedPresentSupportedUsageFlags = copy_src.sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSharedPresentSurfaceCapabilitiesKHR& safe_VkSharedPresentSurfaceCapabilitiesKHR::operator=(
    const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sharedPresentSupportedUsageFlags = copy_src.sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::~safe_VkSharedPresentSurfaceCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sharedPresentSupportedUsageFlags = in_struct->sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const safe_VkSharedPresentSurfaceCapabilitiesKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sharedPresentSupportedUsageFlags = copy_src->sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const VkImportFenceWin32HandleInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      fence(in_struct->fence),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      handle(in_struct->handle),
      name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR),
      pNext(nullptr),
      fence(),
      flags(),
      handleType(),
      handle(),
      name() {}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const safe_VkImportFenceWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportFenceWin32HandleInfoKHR& safe_VkImportFenceWin32HandleInfoKHR::operator=(
    const safe_VkImportFenceWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportFenceWin32HandleInfoKHR::~safe_VkImportFenceWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const VkImportFenceWin32HandleInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const safe_VkImportFenceWin32HandleInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const VkExportFenceWin32HandleInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const safe_VkExportFenceWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR& safe_VkExportFenceWin32HandleInfoKHR::operator=(
    const safe_VkExportFenceWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportFenceWin32HandleInfoKHR::~safe_VkExportFenceWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const VkExportFenceWin32HandleInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const safe_VkExportFenceWin32HandleInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const VkFenceGetWin32HandleInfoKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fence(in_struct->fence), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), fence(), handleType() {}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const safe_VkFenceGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFenceGetWin32HandleInfoKHR& safe_VkFenceGetWin32HandleInfoKHR::operator=(const safe_VkFenceGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFenceGetWin32HandleInfoKHR::~safe_VkFenceGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const VkFenceGetWin32HandleInfoKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const safe_VkFenceGetWin32HandleInfoKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const VkImportFenceFdInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      fence(in_struct->fence),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR), pNext(nullptr), fence(), flags(), handleType(), fd() {}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const safe_VkImportFenceFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportFenceFdInfoKHR& safe_VkImportFenceFdInfoKHR::operator=(const safe_VkImportFenceFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportFenceFdInfoKHR::~safe_VkImportFenceFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportFenceFdInfoKHR::initialize(const VkImportFenceFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportFenceFdInfoKHR::initialize(const safe_VkImportFenceFdInfoKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const VkFenceGetFdInfoKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fence(in_struct->fence), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR), pNext(nullptr), fence(), handleType() {}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const safe_VkFenceGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFenceGetFdInfoKHR& safe_VkFenceGetFdInfoKHR::operator=(const safe_VkFenceGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFenceGetFdInfoKHR::~safe_VkFenceGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkFenceGetFdInfoKHR::initialize(const VkFenceGetFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFenceGetFdInfoKHR::initialize(const safe_VkFenceGetFdInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(
    const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      performanceCounterQueryPools(in_struct->performanceCounterQueryPools),
      performanceCounterMultipleQueryPools(in_struct->performanceCounterMultipleQueryPools) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR),
      pNext(nullptr),
      performanceCounterQueryPools(),
      performanceCounterMultipleQueryPools() {}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(
    const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    performanceCounterQueryPools = copy_src.performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src.performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    performanceCounterQueryPools = copy_src.performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src.performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::~safe_VkPhysicalDevicePerformanceQueryFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::initialize(const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    performanceCounterQueryPools = in_struct->performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = in_struct->performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::initialize(const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    performanceCounterQueryPools = copy_src->performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src->performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(
    const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), allowCommandBufferQueryCopies(in_struct->allowCommandBufferQueryCopies) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR), pNext(nullptr), allowCommandBufferQueryCopies() {}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    allowCommandBufferQueryCopies = copy_src.allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    allowCommandBufferQueryCopies = copy_src.allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::~safe_VkPhysicalDevicePerformanceQueryPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::initialize(const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    allowCommandBufferQueryCopies = in_struct->allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::initialize(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    allowCommandBufferQueryCopies = copy_src->allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR(const VkPerformanceCounterKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), unit(in_struct->unit), scope(in_struct->scope), storage(in_struct->storage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = in_struct->uuid[i];
    }
}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR), pNext(nullptr), unit(), scope(), storage() {}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR(const safe_VkPerformanceCounterKHR& copy_src) {
    sType = copy_src.sType;
    unit = copy_src.unit;
    scope = copy_src.scope;
    storage = copy_src.storage;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src.uuid[i];
    }
}

safe_VkPerformanceCounterKHR& safe_VkPerformanceCounterKHR::operator=(const safe_VkPerformanceCounterKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    unit = copy_src.unit;
    scope = copy_src.scope;
    storage = copy_src.storage;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src.uuid[i];
    }

    return *this;
}

safe_VkPerformanceCounterKHR::~safe_VkPerformanceCounterKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceCounterKHR::initialize(const VkPerformanceCounterKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    unit = in_struct->unit;
    scope = in_struct->scope;
    storage = in_struct->storage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = in_struct->uuid[i];
    }
}

void safe_VkPerformanceCounterKHR::initialize(const safe_VkPerformanceCounterKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    unit = copy_src->unit;
    scope = copy_src->scope;
    storage = copy_src->storage;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src->uuid[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR(
    const VkPerformanceCounterDescriptionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = in_struct->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR), pNext(nullptr), flags() {}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR(
    const safe_VkPerformanceCounterDescriptionKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src.category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR& safe_VkPerformanceCounterDescriptionKHR::operator=(
    const safe_VkPerformanceCounterDescriptionKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src.category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPerformanceCounterDescriptionKHR::~safe_VkPerformanceCounterDescriptionKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceCounterDescriptionKHR::initialize(const VkPerformanceCounterDescriptionKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = in_struct->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPerformanceCounterDescriptionKHR::initialize(const safe_VkPerformanceCounterDescriptionKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR(
    const VkQueryPoolPerformanceCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      queueFamilyIndex(in_struct->queueFamilyIndex),
      counterIndexCount(in_struct->counterIndexCount),
      pCounterIndices(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pCounterIndices) {
        pCounterIndices = new uint32_t[in_struct->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)in_struct->pCounterIndices, sizeof(uint32_t) * in_struct->counterIndexCount);
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR),
      pNext(nullptr),
      queueFamilyIndex(),
      counterIndexCount(),
      pCounterIndices(nullptr) {}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR(
    const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    counterIndexCount = copy_src.counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src.counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src.pCounterIndices, sizeof(uint32_t) * copy_src.counterIndexCount);
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR& safe_VkQueryPoolPerformanceCreateInfoKHR::operator=(
    const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    counterIndexCount = copy_src.counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src.counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src.pCounterIndices, sizeof(uint32_t) * copy_src.counterIndexCount);
    }

    return *this;
}

safe_VkQueryPoolPerformanceCreateInfoKHR::~safe_VkQueryPoolPerformanceCreateInfoKHR() {
    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);
}

void safe_VkQueryPoolPerformanceCreateInfoKHR::initialize(const VkQueryPoolPerformanceCreateInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    counterIndexCount = in_struct->counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pCounterIndices) {
        pCounterIndices = new uint32_t[in_struct->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)in_struct->pCounterIndices, sizeof(uint32_t) * in_struct->counterIndexCount);
    }
}

void safe_VkQueryPoolPerformanceCreateInfoKHR::initialize(const safe_VkQueryPoolPerformanceCreateInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queueFamilyIndex = copy_src->queueFamilyIndex;
    counterIndexCount = copy_src->counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src->pCounterIndices, sizeof(uint32_t) * copy_src->counterIndexCount);
    }
}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR(const VkAcquireProfilingLockInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), timeout(in_struct->timeout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR), pNext(nullptr), flags(), timeout() {}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR(const safe_VkAcquireProfilingLockInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAcquireProfilingLockInfoKHR& safe_VkAcquireProfilingLockInfoKHR::operator=(
    const safe_VkAcquireProfilingLockInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAcquireProfilingLockInfoKHR::~safe_VkAcquireProfilingLockInfoKHR() { FreePnextChain(pNext); }

void safe_VkAcquireProfilingLockInfoKHR::initialize(const VkAcquireProfilingLockInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    timeout = in_struct->timeout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAcquireProfilingLockInfoKHR::initialize(const safe_VkAcquireProfilingLockInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    timeout = copy_src->timeout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR(const VkPerformanceQuerySubmitInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), counterPassIndex(in_struct->counterPassIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR), pNext(nullptr), counterPassIndex() {}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR(const safe_VkPerformanceQuerySubmitInfoKHR& copy_src) {
    sType = copy_src.sType;
    counterPassIndex = copy_src.counterPassIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceQuerySubmitInfoKHR& safe_VkPerformanceQuerySubmitInfoKHR::operator=(
    const safe_VkPerformanceQuerySubmitInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    counterPassIndex = copy_src.counterPassIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceQuerySubmitInfoKHR::~safe_VkPerformanceQuerySubmitInfoKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceQuerySubmitInfoKHR::initialize(const VkPerformanceQuerySubmitInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    counterPassIndex = in_struct->counterPassIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceQuerySubmitInfoKHR::initialize(const safe_VkPerformanceQuerySubmitInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    counterPassIndex = copy_src->counterPassIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), surface(in_struct->surface) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR), pNext(nullptr), surface() {}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src) {
    sType = copy_src.sType;
    surface = copy_src.surface;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSurfaceInfo2KHR& safe_VkPhysicalDeviceSurfaceInfo2KHR::operator=(
    const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surface = copy_src.surface;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::~safe_VkPhysicalDeviceSurfaceInfo2KHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surface = in_struct->surface;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const safe_VkPhysicalDeviceSurfaceInfo2KHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surface = copy_src->surface;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const VkSurfaceCapabilities2KHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), surfaceCapabilities(in_struct->surfaceCapabilities) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR), pNext(nullptr), surfaceCapabilities() {}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const safe_VkSurfaceCapabilities2KHR& copy_src) {
    sType = copy_src.sType;
    surfaceCapabilities = copy_src.surfaceCapabilities;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilities2KHR& safe_VkSurfaceCapabilities2KHR::operator=(const safe_VkSurfaceCapabilities2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surfaceCapabilities = copy_src.surfaceCapabilities;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilities2KHR::~safe_VkSurfaceCapabilities2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilities2KHR::initialize(const VkSurfaceCapabilities2KHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surfaceCapabilities = in_struct->surfaceCapabilities;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilities2KHR::initialize(const safe_VkSurfaceCapabilities2KHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surfaceCapabilities = copy_src->surfaceCapabilities;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const VkSurfaceFormat2KHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), surfaceFormat(in_struct->surfaceFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR), pNext(nullptr), surfaceFormat() {}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const safe_VkSurfaceFormat2KHR& copy_src) {
    sType = copy_src.sType;
    surfaceFormat = copy_src.surfaceFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceFormat2KHR& safe_VkSurfaceFormat2KHR::operator=(const safe_VkSurfaceFormat2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surfaceFormat = copy_src.surfaceFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceFormat2KHR::~safe_VkSurfaceFormat2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceFormat2KHR::initialize(const VkSurfaceFormat2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surfaceFormat = in_struct->surfaceFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceFormat2KHR::initialize(const safe_VkSurfaceFormat2KHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surfaceFormat = copy_src->surfaceFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const VkDisplayProperties2KHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayProperties(&in_struct->displayProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR() : sType(VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR), pNext(nullptr) {}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const safe_VkDisplayProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayProperties.initialize(&copy_src.displayProperties);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayProperties2KHR& safe_VkDisplayProperties2KHR::operator=(const safe_VkDisplayProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayProperties.initialize(&copy_src.displayProperties);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayProperties2KHR::~safe_VkDisplayProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayProperties2KHR::initialize(const VkDisplayProperties2KHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayProperties.initialize(&in_struct->displayProperties);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayProperties2KHR::initialize(const safe_VkDisplayProperties2KHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayProperties.initialize(&copy_src->displayProperties);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const VkDisplayPlaneProperties2KHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayPlaneProperties(in_struct->displayPlaneProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR), pNext(nullptr), displayPlaneProperties() {}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const safe_VkDisplayPlaneProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayPlaneProperties = copy_src.displayPlaneProperties;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneProperties2KHR& safe_VkDisplayPlaneProperties2KHR::operator=(const safe_VkDisplayPlaneProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayPlaneProperties = copy_src.displayPlaneProperties;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneProperties2KHR::~safe_VkDisplayPlaneProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneProperties2KHR::initialize(const VkDisplayPlaneProperties2KHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayPlaneProperties = in_struct->displayPlaneProperties;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneProperties2KHR::initialize(const safe_VkDisplayPlaneProperties2KHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayPlaneProperties = copy_src->displayPlaneProperties;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const VkDisplayModeProperties2KHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayModeProperties(in_struct->displayModeProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR), pNext(nullptr), displayModeProperties() {}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const safe_VkDisplayModeProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayModeProperties = copy_src.displayModeProperties;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayModeProperties2KHR& safe_VkDisplayModeProperties2KHR::operator=(const safe_VkDisplayModeProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayModeProperties = copy_src.displayModeProperties;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayModeProperties2KHR::~safe_VkDisplayModeProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayModeProperties2KHR::initialize(const VkDisplayModeProperties2KHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayModeProperties = in_struct->displayModeProperties;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayModeProperties2KHR::initialize(const safe_VkDisplayModeProperties2KHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayModeProperties = copy_src->displayModeProperties;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const VkDisplayPlaneInfo2KHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), mode(in_struct->mode), planeIndex(in_struct->planeIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR), pNext(nullptr), mode(), planeIndex() {}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const safe_VkDisplayPlaneInfo2KHR& copy_src) {
    sType = copy_src.sType;
    mode = copy_src.mode;
    planeIndex = copy_src.planeIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneInfo2KHR& safe_VkDisplayPlaneInfo2KHR::operator=(const safe_VkDisplayPlaneInfo2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mode = copy_src.mode;
    planeIndex = copy_src.planeIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneInfo2KHR::~safe_VkDisplayPlaneInfo2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneInfo2KHR::initialize(const VkDisplayPlaneInfo2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mode = in_struct->mode;
    planeIndex = in_struct->planeIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneInfo2KHR::initialize(const safe_VkDisplayPlaneInfo2KHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mode = copy_src->mode;
    planeIndex = copy_src->planeIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const VkDisplayPlaneCapabilities2KHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), capabilities(in_struct->capabilities) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR), pNext(nullptr), capabilities() {}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const safe_VkDisplayPlaneCapabilities2KHR& copy_src) {
    sType = copy_src.sType;
    capabilities = copy_src.capabilities;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneCapabilities2KHR& safe_VkDisplayPlaneCapabilities2KHR::operator=(
    const safe_VkDisplayPlaneCapabilities2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    capabilities = copy_src.capabilities;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneCapabilities2KHR::~safe_VkDisplayPlaneCapabilities2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const VkDisplayPlaneCapabilities2KHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    capabilities = in_struct->capabilities;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const safe_VkDisplayPlaneCapabilities2KHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    capabilities = copy_src->capabilities;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(
    const VkPhysicalDevicePortabilitySubsetFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      constantAlphaColorBlendFactors(in_struct->constantAlphaColorBlendFactors),
      events(in_struct->events),
      imageViewFormatReinterpretation(in_struct->imageViewFormatReinterpretation),
      imageViewFormatSwizzle(in_struct->imageViewFormatSwizzle),
      imageView2DOn3DImage(in_struct->imageView2DOn3DImage),
      multisampleArrayImage(in_struct->multisampleArrayImage),
      mutableComparisonSamplers(in_struct->mutableComparisonSamplers),
      pointPolygons(in_struct->pointPolygons),
      samplerMipLodBias(in_struct->samplerMipLodBias),
      separateStencilMaskRef(in_struct->separateStencilMaskRef),
      shaderSampleRateInterpolationFunctions(in_struct->shaderSampleRateInterpolationFunctions),
      tessellationIsolines(in_struct->tessellationIsolines),
      tessellationPointMode(in_struct->tessellationPointMode),
      triangleFans(in_struct->triangleFans),
      vertexAttributeAccessBeyondStride(in_struct->vertexAttributeAccessBeyondStride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR),
      pNext(nullptr),
      constantAlphaColorBlendFactors(),
      events(),
      imageViewFormatReinterpretation(),
      imageViewFormatSwizzle(),
      imageView2DOn3DImage(),
      multisampleArrayImage(),
      mutableComparisonSamplers(),
      pointPolygons(),
      samplerMipLodBias(),
      separateStencilMaskRef(),
      shaderSampleRateInterpolationFunctions(),
      tessellationIsolines(),
      tessellationPointMode(),
      triangleFans(),
      vertexAttributeAccessBeyondStride() {}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    constantAlphaColorBlendFactors = copy_src.constantAlphaColorBlendFactors;
    events = copy_src.events;
    imageViewFormatReinterpretation = copy_src.imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src.imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src.imageView2DOn3DImage;
    multisampleArrayImage = copy_src.multisampleArrayImage;
    mutableComparisonSamplers = copy_src.mutableComparisonSamplers;
    pointPolygons = copy_src.pointPolygons;
    samplerMipLodBias = copy_src.samplerMipLodBias;
    separateStencilMaskRef = copy_src.separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src.shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src.tessellationIsolines;
    tessellationPointMode = copy_src.tessellationPointMode;
    triangleFans = copy_src.triangleFans;
    vertexAttributeAccessBeyondStride = copy_src.vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    constantAlphaColorBlendFactors = copy_src.constantAlphaColorBlendFactors;
    events = copy_src.events;
    imageViewFormatReinterpretation = copy_src.imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src.imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src.imageView2DOn3DImage;
    multisampleArrayImage = copy_src.multisampleArrayImage;
    mutableComparisonSamplers = copy_src.mutableComparisonSamplers;
    pointPolygons = copy_src.pointPolygons;
    samplerMipLodBias = copy_src.samplerMipLodBias;
    separateStencilMaskRef = copy_src.separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src.shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src.tessellationIsolines;
    tessellationPointMode = copy_src.tessellationPointMode;
    triangleFans = copy_src.triangleFans;
    vertexAttributeAccessBeyondStride = copy_src.vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::~safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::initialize(const VkPhysicalDevicePortabilitySubsetFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    constantAlphaColorBlendFactors = in_struct->constantAlphaColorBlendFactors;
    events = in_struct->events;
    imageViewFormatReinterpretation = in_struct->imageViewFormatReinterpretation;
    imageViewFormatSwizzle = in_struct->imageViewFormatSwizzle;
    imageView2DOn3DImage = in_struct->imageView2DOn3DImage;
    multisampleArrayImage = in_struct->multisampleArrayImage;
    mutableComparisonSamplers = in_struct->mutableComparisonSamplers;
    pointPolygons = in_struct->pointPolygons;
    samplerMipLodBias = in_struct->samplerMipLodBias;
    separateStencilMaskRef = in_struct->separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = in_struct->shaderSampleRateInterpolationFunctions;
    tessellationIsolines = in_struct->tessellationIsolines;
    tessellationPointMode = in_struct->tessellationPointMode;
    triangleFans = in_struct->triangleFans;
    vertexAttributeAccessBeyondStride = in_struct->vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::initialize(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    constantAlphaColorBlendFactors = copy_src->constantAlphaColorBlendFactors;
    events = copy_src->events;
    imageViewFormatReinterpretation = copy_src->imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src->imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src->imageView2DOn3DImage;
    multisampleArrayImage = copy_src->multisampleArrayImage;
    mutableComparisonSamplers = copy_src->mutableComparisonSamplers;
    pointPolygons = copy_src->pointPolygons;
    samplerMipLodBias = copy_src->samplerMipLodBias;
    separateStencilMaskRef = copy_src->separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src->shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src->tessellationIsolines;
    tessellationPointMode = copy_src->tessellationPointMode;
    triangleFans = copy_src->triangleFans;
    vertexAttributeAccessBeyondStride = copy_src->vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(
    const VkPhysicalDevicePortabilitySubsetPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minVertexInputBindingStrideAlignment(in_struct->minVertexInputBindingStrideAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR),
      pNext(nullptr),
      minVertexInputBindingStrideAlignment() {}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    minVertexInputBindingStrideAlignment = copy_src.minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minVertexInputBindingStrideAlignment = copy_src.minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::~safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::initialize(
    const VkPhysicalDevicePortabilitySubsetPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minVertexInputBindingStrideAlignment = in_struct->minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::initialize(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minVertexInputBindingStrideAlignment = copy_src->minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR(
    const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderSubgroupClock(in_struct->shaderSubgroupClock),
      shaderDeviceClock(in_struct->shaderDeviceClock) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR),
      pNext(nullptr),
      shaderSubgroupClock(),
      shaderDeviceClock() {}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR(
    const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderSubgroupClock = copy_src.shaderSubgroupClock;
    shaderDeviceClock = copy_src.shaderDeviceClock;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR& safe_VkPhysicalDeviceShaderClockFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSubgroupClock = copy_src.shaderSubgroupClock;
    shaderDeviceClock = copy_src.shaderDeviceClock;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::~safe_VkPhysicalDeviceShaderClockFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderClockFeaturesKHR::initialize(const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSubgroupClock = in_struct->shaderSubgroupClock;
    shaderDeviceClock = in_struct->shaderDeviceClock;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderClockFeaturesKHR::initialize(const safe_VkPhysicalDeviceShaderClockFeaturesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSubgroupClock = copy_src->shaderSubgroupClock;
    shaderDeviceClock = copy_src->shaderDeviceClock;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR(const VkVideoDecodeH265ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc() {}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR(const safe_VkVideoDecodeH265ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH265ProfileInfoKHR& safe_VkVideoDecodeH265ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeH265ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH265ProfileInfoKHR::~safe_VkVideoDecodeH265ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH265ProfileInfoKHR::initialize(const VkVideoDecodeH265ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH265ProfileInfoKHR::initialize(const safe_VkVideoDecodeH265ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR(const VkVideoDecodeH265CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), maxLevelIdc(in_struct->maxLevelIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR), pNext(nullptr), maxLevelIdc() {}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR(
    const safe_VkVideoDecodeH265CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH265CapabilitiesKHR& safe_VkVideoDecodeH265CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeH265CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH265CapabilitiesKHR::~safe_VkVideoDecodeH265CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH265CapabilitiesKHR::initialize(const VkVideoDecodeH265CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevelIdc = in_struct->maxLevelIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH265CapabilitiesKHR::initialize(const safe_VkVideoDecodeH265CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevelIdc = copy_src->maxLevelIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR(
    const VkVideoDecodeH265SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdVPSCount(in_struct->stdVPSCount),
      pStdVPSs(nullptr),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdVPSCount(),
      pStdVPSs(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR& safe_VkVideoDecodeH265SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::~safe_VkVideoDecodeH265SessionParametersAddInfoKHR() {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265SessionParametersAddInfoKHR::initialize(const VkVideoDecodeH265SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdVPSCount = in_struct->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoDecodeH265SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdVPSCount = copy_src->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src->stdVPSCount);
    }

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(
    const VkVideoDecodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdVPSCount(in_struct->maxStdVPSCount),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdVPSCount(),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::~safe_VkVideoDecodeH265SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::initialize(
    const VkVideoDecodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdVPSCount = in_struct->maxStdVPSCount;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdVPSCount = copy_src->maxStdVPSCount;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR(const VkVideoDecodeH265PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      pStdPictureInfo(nullptr),
      sliceSegmentCount(in_struct->sliceSegmentCount),
      pSliceSegmentOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[in_struct->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)in_struct->pSliceSegmentOffsets,
               sizeof(uint32_t) * in_struct->sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      sliceSegmentCount(),
      pSliceSegmentOffsets(nullptr) {}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR(const safe_VkVideoDecodeH265PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src.sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src.sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src.pSliceSegmentOffsets, sizeof(uint32_t) * copy_src.sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265PictureInfoKHR& safe_VkVideoDecodeH265PictureInfoKHR::operator=(
    const safe_VkVideoDecodeH265PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src.sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src.sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src.pSliceSegmentOffsets, sizeof(uint32_t) * copy_src.sliceSegmentCount);
    }

    return *this;
}

safe_VkVideoDecodeH265PictureInfoKHR::~safe_VkVideoDecodeH265PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265PictureInfoKHR::initialize(const VkVideoDecodeH265PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = in_struct->sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[in_struct->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)in_struct->pSliceSegmentOffsets,
               sizeof(uint32_t) * in_struct->sliceSegmentCount);
    }
}

void safe_VkVideoDecodeH265PictureInfoKHR::initialize(const safe_VkVideoDecodeH265PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src->sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src->pStdPictureInfo);
    }

    if (copy_src->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src->pSliceSegmentOffsets, sizeof(uint32_t) * copy_src->sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR(const VkVideoDecodeH265DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR(const safe_VkVideoDecodeH265DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR& safe_VkVideoDecodeH265DpbSlotInfoKHR::operator=(
    const safe_VkVideoDecodeH265DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::~safe_VkVideoDecodeH265DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265DpbSlotInfoKHR::initialize(const VkVideoDecodeH265DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoDecodeH265DpbSlotInfoKHR::initialize(const safe_VkVideoDecodeH265DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::safe_VkDeviceQueueGlobalPriorityCreateInfoKHR(
    const VkDeviceQueueGlobalPriorityCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), globalPriority(in_struct->globalPriority) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::safe_VkDeviceQueueGlobalPriorityCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR), pNext(nullptr), globalPriority() {}

safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::safe_VkDeviceQueueGlobalPriorityCreateInfoKHR(
    const safe_VkDeviceQueueGlobalPriorityCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    globalPriority = copy_src.globalPriority;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceQueueGlobalPriorityCreateInfoKHR& safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::operator=(
    const safe_VkDeviceQueueGlobalPriorityCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    globalPriority = copy_src.globalPriority;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::~safe_VkDeviceQueueGlobalPriorityCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::initialize(const VkDeviceQueueGlobalPriorityCreateInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    globalPriority = in_struct->globalPriority;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceQueueGlobalPriorityCreateInfoKHR::initialize(const safe_VkDeviceQueueGlobalPriorityCreateInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    globalPriority = copy_src->globalPriority;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR(
    const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), globalPriorityQuery(in_struct->globalPriorityQuery) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR), pNext(nullptr), globalPriorityQuery() {}

safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR(
    const safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    globalPriorityQuery = copy_src.globalPriorityQuery;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR& safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    globalPriorityQuery = copy_src.globalPriorityQuery;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::~safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::initialize(
    const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    globalPriorityQuery = in_struct->globalPriorityQuery;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    globalPriorityQuery = copy_src->globalPriorityQuery;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyGlobalPriorityPropertiesKHR::safe_VkQueueFamilyGlobalPriorityPropertiesKHR(
    const VkQueueFamilyGlobalPriorityPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), priorityCount(in_struct->priorityCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_GLOBAL_PRIORITY_SIZE_KHR; ++i) {
        priorities[i] = in_struct->priorities[i];
    }
}

safe_VkQueueFamilyGlobalPriorityPropertiesKHR::safe_VkQueueFamilyGlobalPriorityPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR), pNext(nullptr), priorityCount() {}

safe_VkQueueFamilyGlobalPriorityPropertiesKHR::safe_VkQueueFamilyGlobalPriorityPropertiesKHR(
    const safe_VkQueueFamilyGlobalPriorityPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    priorityCount = copy_src.priorityCount;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_GLOBAL_PRIORITY_SIZE_KHR; ++i) {
        priorities[i] = copy_src.priorities[i];
    }
}

safe_VkQueueFamilyGlobalPriorityPropertiesKHR& safe_VkQueueFamilyGlobalPriorityPropertiesKHR::operator=(
    const safe_VkQueueFamilyGlobalPriorityPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    priorityCount = copy_src.priorityCount;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_GLOBAL_PRIORITY_SIZE_KHR; ++i) {
        priorities[i] = copy_src.priorities[i];
    }

    return *this;
}

safe_VkQueueFamilyGlobalPriorityPropertiesKHR::~safe_VkQueueFamilyGlobalPriorityPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyGlobalPriorityPropertiesKHR::initialize(const VkQueueFamilyGlobalPriorityPropertiesKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    priorityCount = in_struct->priorityCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_GLOBAL_PRIORITY_SIZE_KHR; ++i) {
        priorities[i] = in_struct->priorities[i];
    }
}

void safe_VkQueueFamilyGlobalPriorityPropertiesKHR::initialize(const safe_VkQueueFamilyGlobalPriorityPropertiesKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    priorityCount = copy_src->priorityCount;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_GLOBAL_PRIORITY_SIZE_KHR; ++i) {
        priorities[i] = copy_src->priorities[i];
    }
}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR(
    const VkFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pFragmentShadingRateAttachment(nullptr),
      shadingRateAttachmentTexelSize(in_struct->shadingRateAttachmentTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(in_struct->pFragmentShadingRateAttachment);
}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR),
      pNext(nullptr),
      pFragmentShadingRateAttachment(nullptr),
      shadingRateAttachmentTexelSize() {}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR(
    const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src) {
    sType = copy_src.sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src.pFragmentShadingRateAttachment);
}

safe_VkFragmentShadingRateAttachmentInfoKHR& safe_VkFragmentShadingRateAttachmentInfoKHR::operator=(
    const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src.pFragmentShadingRateAttachment);

    return *this;
}

safe_VkFragmentShadingRateAttachmentInfoKHR::~safe_VkFragmentShadingRateAttachmentInfoKHR() {
    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);
}

void safe_VkFragmentShadingRateAttachmentInfoKHR::initialize(const VkFragmentShadingRateAttachmentInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = in_struct->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(in_struct->pFragmentShadingRateAttachment);
}

void safe_VkFragmentShadingRateAttachmentInfoKHR::initialize(const safe_VkFragmentShadingRateAttachmentInfoKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src->pFragmentShadingRateAttachment);
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(
    const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentSize(in_struct->fragmentSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR), pNext(nullptr), fragmentSize() {}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::operator=(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }

    return *this;
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::~safe_VkPipelineFragmentShadingRateStateCreateInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::initialize(
    const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentSize = in_struct->fragmentSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

void safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::initialize(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentSize = copy_src->fragmentSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src->combinerOps[i];
    }
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(
    const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pipelineFragmentShadingRate(in_struct->pipelineFragmentShadingRate),
      primitiveFragmentShadingRate(in_struct->primitiveFragmentShadingRate),
      attachmentFragmentShadingRate(in_struct->attachmentFragmentShadingRate) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR),
      pNext(nullptr),
      pipelineFragmentShadingRate(),
      primitiveFragmentShadingRate(),
      attachmentFragmentShadingRate() {}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineFragmentShadingRate = copy_src.pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src.primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src.attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineFragmentShadingRate = copy_src.pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src.primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src.attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::~safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::initialize(
    const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineFragmentShadingRate = in_struct->pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = in_struct->primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = in_struct->attachmentFragmentShadingRate;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineFragmentShadingRate = copy_src->pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src->primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src->attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(
    const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minFragmentShadingRateAttachmentTexelSize(in_struct->minFragmentShadingRateAttachmentTexelSize),
      maxFragmentShadingRateAttachmentTexelSize(in_struct->maxFragmentShadingRateAttachmentTexelSize),
      maxFragmentShadingRateAttachmentTexelSizeAspectRatio(in_struct->maxFragmentShadingRateAttachmentTexelSizeAspectRatio),
      primitiveFragmentShadingRateWithMultipleViewports(in_struct->primitiveFragmentShadingRateWithMultipleViewports),
      layeredShadingRateAttachments(in_struct->layeredShadingRateAttachments),
      fragmentShadingRateNonTrivialCombinerOps(in_struct->fragmentShadingRateNonTrivialCombinerOps),
      maxFragmentSize(in_struct->maxFragmentSize),
      maxFragmentSizeAspectRatio(in_struct->maxFragmentSizeAspectRatio),
      maxFragmentShadingRateCoverageSamples(in_struct->maxFragmentShadingRateCoverageSamples),
      maxFragmentShadingRateRasterizationSamples(in_struct->maxFragmentShadingRateRasterizationSamples),
      fragmentShadingRateWithShaderDepthStencilWrites(in_struct->fragmentShadingRateWithShaderDepthStencilWrites),
      fragmentShadingRateWithSampleMask(in_struct->fragmentShadingRateWithSampleMask),
      fragmentShadingRateWithShaderSampleMask(in_struct->fragmentShadingRateWithShaderSampleMask),
      fragmentShadingRateWithConservativeRasterization(in_struct->fragmentShadingRateWithConservativeRasterization),
      fragmentShadingRateWithFragmentShaderInterlock(in_struct->fragmentShadingRateWithFragmentShaderInterlock),
      fragmentShadingRateWithCustomSampleLocations(in_struct->fragmentShadingRateWithCustomSampleLocations),
      fragmentShadingRateStrictMultiplyCombiner(in_struct->fragmentShadingRateStrictMultiplyCombiner) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR),
      pNext(nullptr),
      minFragmentShadingRateAttachmentTexelSize(),
      maxFragmentShadingRateAttachmentTexelSize(),
      maxFragmentShadingRateAttachmentTexelSizeAspectRatio(),
      primitiveFragmentShadingRateWithMultipleViewports(),
      layeredShadingRateAttachments(),
      fragmentShadingRateNonTrivialCombinerOps(),
      maxFragmentSize(),
      maxFragmentSizeAspectRatio(),
      maxFragmentShadingRateCoverageSamples(),
      maxFragmentShadingRateRasterizationSamples(),
      fragmentShadingRateWithShaderDepthStencilWrites(),
      fragmentShadingRateWithSampleMask(),
      fragmentShadingRateWithShaderSampleMask(),
      fragmentShadingRateWithConservativeRasterization(),
      fragmentShadingRateWithFragmentShaderInterlock(),
      fragmentShadingRateWithCustomSampleLocations(),
      fragmentShadingRateStrictMultiplyCombiner() {}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src.minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src.maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src.primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src.layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src.fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src.maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src.maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src.maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src.maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src.fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src.fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src.fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src.fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src.fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src.fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src.fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src.minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src.maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src.primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src.layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src.fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src.maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src.maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src.maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src.maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src.fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src.fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src.fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src.fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src.fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src.fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src.fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::~safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::initialize(
    const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minFragmentShadingRateAttachmentTexelSize = in_struct->minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = in_struct->maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = in_struct->maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = in_struct->primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = in_struct->layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = in_struct->fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = in_struct->maxFragmentSize;
    maxFragmentSizeAspectRatio = in_struct->maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = in_struct->maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = in_struct->maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = in_struct->fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = in_struct->fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = in_struct->fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = in_struct->fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = in_struct->fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = in_struct->fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = in_struct->fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src->minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src->maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src->maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src->primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src->layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src->fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src->maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src->maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src->maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src->maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src->fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src->fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src->fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src->fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src->fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src->fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src->fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR(
    const VkPhysicalDeviceFragmentShadingRateKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), sampleCounts(in_struct->sampleCounts), fragmentSize(in_struct->fragmentSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR), pNext(nullptr), sampleCounts(), fragmentSize() {}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR(
    const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src) {
    sType = copy_src.sType;
    sampleCounts = copy_src.sampleCounts;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateKHR& safe_VkPhysicalDeviceFragmentShadingRateKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleCounts = copy_src.sampleCounts;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::~safe_VkPhysicalDeviceFragmentShadingRateKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceFragmentShadingRateKHR::initialize(const VkPhysicalDeviceFragmentShadingRateKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleCounts = in_struct->sampleCounts;
    fragmentSize = in_struct->fragmentSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateKHR::initialize(const safe_VkPhysicalDeviceFragmentShadingRateKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleCounts = copy_src->sampleCounts;
    fragmentSize = copy_src->fragmentSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(const VkSurfaceProtectedCapabilitiesKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), supportsProtected(in_struct->supportsProtected) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR), pNext(nullptr), supportsProtected() {}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(
    const safe_VkSurfaceProtectedCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    supportsProtected = copy_src.supportsProtected;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceProtectedCapabilitiesKHR& safe_VkSurfaceProtectedCapabilitiesKHR::operator=(
    const safe_VkSurfaceProtectedCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportsProtected = copy_src.supportsProtected;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceProtectedCapabilitiesKHR::~safe_VkSurfaceProtectedCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const VkSurfaceProtectedCapabilitiesKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportsProtected = in_struct->supportsProtected;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const safe_VkSurfaceProtectedCapabilitiesKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportsProtected = copy_src->supportsProtected;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR(
    const VkPhysicalDevicePresentWaitFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentWait(in_struct->presentWait) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR), pNext(nullptr), presentWait() {}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR(
    const safe_VkPhysicalDevicePresentWaitFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentWait = copy_src.presentWait;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR& safe_VkPhysicalDevicePresentWaitFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentWaitFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentWait = copy_src.presentWait;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::~safe_VkPhysicalDevicePresentWaitFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentWaitFeaturesKHR::initialize(const VkPhysicalDevicePresentWaitFeaturesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentWait = in_struct->presentWait;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentWaitFeaturesKHR::initialize(const safe_VkPhysicalDevicePresentWaitFeaturesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentWait = copy_src->presentWait;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(
    const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), pipelineExecutableInfo(in_struct->pipelineExecutableInfo) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR),
      pNext(nullptr),
      pipelineExecutableInfo() {}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineExecutableInfo = copy_src.pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR&
safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineExecutableInfo = copy_src.pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::~safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::initialize(
    const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineExecutableInfo = in_struct->pipelineExecutableInfo;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::initialize(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineExecutableInfo = copy_src->pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR(const VkPipelineInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                               bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR() : sType(VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR), pNext(nullptr), pipeline() {}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR(const safe_VkPipelineInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineInfoKHR& safe_VkPipelineInfoKHR::operator=(const safe_VkPipelineInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineInfoKHR::~safe_VkPipelineInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineInfoKHR::initialize(const VkPipelineInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineInfoKHR::initialize(const safe_VkPipelineInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR(const VkPipelineExecutablePropertiesKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), stages(in_struct->stages), subgroupSize(in_struct->subgroupSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR), pNext(nullptr), stages(), subgroupSize() {}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR(
    const safe_VkPipelineExecutablePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    stages = copy_src.stages;
    subgroupSize = copy_src.subgroupSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPipelineExecutablePropertiesKHR& safe_VkPipelineExecutablePropertiesKHR::operator=(
    const safe_VkPipelineExecutablePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stages = copy_src.stages;
    subgroupSize = copy_src.subgroupSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPipelineExecutablePropertiesKHR::~safe_VkPipelineExecutablePropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutablePropertiesKHR::initialize(const VkPipelineExecutablePropertiesKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stages = in_struct->stages;
    subgroupSize = in_struct->subgroupSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPipelineExecutablePropertiesKHR::initialize(const safe_VkPipelineExecutablePropertiesKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stages = copy_src->stages;
    subgroupSize = copy_src->subgroupSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR(const VkPipelineExecutableInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline), executableIndex(in_struct->executableIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR), pNext(nullptr), pipeline(), executableIndex() {}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR(const safe_VkPipelineExecutableInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    executableIndex = copy_src.executableIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineExecutableInfoKHR& safe_VkPipelineExecutableInfoKHR::operator=(const safe_VkPipelineExecutableInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    executableIndex = copy_src.executableIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineExecutableInfoKHR::~safe_VkPipelineExecutableInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutableInfoKHR::initialize(const VkPipelineExecutableInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    executableIndex = in_struct->executableIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineExecutableInfoKHR::initialize(const safe_VkPipelineExecutableInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    executableIndex = copy_src->executableIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR(const VkPipelineExecutableStatisticKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), format(in_struct->format), value(in_struct->value) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR), pNext(nullptr), format(), value() {}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR(
    const safe_VkPipelineExecutableStatisticKHR& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPipelineExecutableStatisticKHR& safe_VkPipelineExecutableStatisticKHR::operator=(
    const safe_VkPipelineExecutableStatisticKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPipelineExecutableStatisticKHR::~safe_VkPipelineExecutableStatisticKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutableStatisticKHR::initialize(const VkPipelineExecutableStatisticKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    value = in_struct->value;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPipelineExecutableStatisticKHR::initialize(const safe_VkPipelineExecutableStatisticKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    value = copy_src->value;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR(
    const VkPipelineExecutableInternalRepresentationKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), isText(in_struct->isText), dataSize(in_struct->dataSize), pData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR),
      pNext(nullptr),
      isText(),
      dataSize(),
      pData(nullptr) {}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR(
    const safe_VkPipelineExecutableInternalRepresentationKHR& copy_src) {
    sType = copy_src.sType;
    isText = copy_src.isText;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR& safe_VkPipelineExecutableInternalRepresentationKHR::operator=(
    const safe_VkPipelineExecutableInternalRepresentationKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    isText = copy_src.isText;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }

    return *this;
}

safe_VkPipelineExecutableInternalRepresentationKHR::~safe_VkPipelineExecutableInternalRepresentationKHR() {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
}

void safe_VkPipelineExecutableInternalRepresentationKHR::initialize(const VkPipelineExecutableInternalRepresentationKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    isText = in_struct->isText;
    dataSize = in_struct->dataSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

void safe_VkPipelineExecutableInternalRepresentationKHR::initialize(
    const safe_VkPipelineExecutableInternalRepresentationKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    isText = copy_src->isText;
    dataSize = copy_src->dataSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }

    if (copy_src->pData != nullptr) {
        auto temp = new std::byte[copy_src->dataSize];
        std::memcpy(temp, copy_src->pData, copy_src->dataSize);
        pData = temp;
    }
}

safe_VkMemoryMapInfoKHR::safe_VkMemoryMapInfoKHR(const VkMemoryMapInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                                 bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      memory(in_struct->memory),
      offset(in_struct->offset),
      size(in_struct->size) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryMapInfoKHR::safe_VkMemoryMapInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR), pNext(nullptr), flags(), memory(), offset(), size() {}

safe_VkMemoryMapInfoKHR::safe_VkMemoryMapInfoKHR(const safe_VkMemoryMapInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    memory = copy_src.memory;
    offset = copy_src.offset;
    size = copy_src.size;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryMapInfoKHR& safe_VkMemoryMapInfoKHR::operator=(const safe_VkMemoryMapInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    memory = copy_src.memory;
    offset = copy_src.offset;
    size = copy_src.size;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryMapInfoKHR::~safe_VkMemoryMapInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryMapInfoKHR::initialize(const VkMemoryMapInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    memory = in_struct->memory;
    offset = in_struct->offset;
    size = in_struct->size;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryMapInfoKHR::initialize(const safe_VkMemoryMapInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    memory = copy_src->memory;
    offset = copy_src->offset;
    size = copy_src->size;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryUnmapInfoKHR::safe_VkMemoryUnmapInfoKHR(const VkMemoryUnmapInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), memory(in_struct->memory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryUnmapInfoKHR::safe_VkMemoryUnmapInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO_KHR), pNext(nullptr), flags(), memory() {}

safe_VkMemoryUnmapInfoKHR::safe_VkMemoryUnmapInfoKHR(const safe_VkMemoryUnmapInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    memory = copy_src.memory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryUnmapInfoKHR& safe_VkMemoryUnmapInfoKHR::operator=(const safe_VkMemoryUnmapInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    memory = copy_src.memory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryUnmapInfoKHR::~safe_VkMemoryUnmapInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryUnmapInfoKHR::initialize(const VkMemoryUnmapInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    memory = in_struct->memory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryUnmapInfoKHR::initialize(const safe_VkMemoryUnmapInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    memory = copy_src->memory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR(const VkPipelineLibraryCreateInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), libraryCount(in_struct->libraryCount), pLibraries(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (libraryCount && in_struct->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = in_struct->pLibraries[i];
        }
    }
}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR), pNext(nullptr), libraryCount(), pLibraries(nullptr) {}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR(const safe_VkPipelineLibraryCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    libraryCount = copy_src.libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (libraryCount && copy_src.pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src.pLibraries[i];
        }
    }
}

safe_VkPipelineLibraryCreateInfoKHR& safe_VkPipelineLibraryCreateInfoKHR::operator=(
    const safe_VkPipelineLibraryCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    libraryCount = copy_src.libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (libraryCount && copy_src.pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src.pLibraries[i];
        }
    }

    return *this;
}

safe_VkPipelineLibraryCreateInfoKHR::~safe_VkPipelineLibraryCreateInfoKHR() {
    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);
}

void safe_VkPipelineLibraryCreateInfoKHR::initialize(const VkPipelineLibraryCreateInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    libraryCount = in_struct->libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (libraryCount && in_struct->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = in_struct->pLibraries[i];
        }
    }
}

void safe_VkPipelineLibraryCreateInfoKHR::initialize(const safe_VkPipelineLibraryCreateInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    libraryCount = copy_src->libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (libraryCount && copy_src->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src->pLibraries[i];
        }
    }
}

safe_VkPresentIdKHR::safe_VkPresentIdKHR(const VkPresentIdKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                         bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pPresentIds(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

safe_VkPresentIdKHR::safe_VkPresentIdKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_ID_KHR), pNext(nullptr), swapchainCount(), pPresentIds(nullptr) {}

safe_VkPresentIdKHR::safe_VkPresentIdKHR(const safe_VkPresentIdKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }
}

safe_VkPresentIdKHR& safe_VkPresentIdKHR::operator=(const safe_VkPresentIdKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentIdKHR::~safe_VkPresentIdKHR() {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
}

void safe_VkPresentIdKHR::initialize(const VkPresentIdKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

void safe_VkPresentIdKHR::initialize(const safe_VkPresentIdKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentIds) {
        pPresentIds = new uint64_t[copy_src->swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src->pPresentIds, sizeof(uint64_t) * copy_src->swapchainCount);
    }
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR(
    const VkPhysicalDevicePresentIdFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentId(in_struct->presentId) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR), pNext(nullptr), presentId() {}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR(
    const safe_VkPhysicalDevicePresentIdFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentId = copy_src.presentId;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentIdFeaturesKHR& safe_VkPhysicalDevicePresentIdFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentIdFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentId = copy_src.presentId;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::~safe_VkPhysicalDevicePresentIdFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentIdFeaturesKHR::initialize(const VkPhysicalDevicePresentIdFeaturesKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentId = in_struct->presentId;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentIdFeaturesKHR::initialize(const safe_VkPhysicalDevicePresentIdFeaturesKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentId = copy_src->presentId;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR(const VkVideoEncodeInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      dstBuffer(in_struct->dstBuffer),
      dstBufferOffset(in_struct->dstBufferOffset),
      dstBufferRange(in_struct->dstBufferRange),
      srcPictureResource(&in_struct->srcPictureResource),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr),
      precedingExternallyEncodedBytes(in_struct->precedingExternallyEncodedBytes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR),
      pNext(nullptr),
      flags(),
      dstBuffer(),
      dstBufferOffset(),
      dstBufferRange(),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(),
      pReferenceSlots(nullptr),
      precedingExternallyEncodedBytes() {}

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR(const safe_VkVideoEncodeInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    dstBuffer = copy_src.dstBuffer;
    dstBufferOffset = copy_src.dstBufferOffset;
    dstBufferRange = copy_src.dstBufferRange;
    srcPictureResource.initialize(&copy_src.srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src.precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeInfoKHR& safe_VkVideoEncodeInfoKHR::operator=(const safe_VkVideoEncodeInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    dstBuffer = copy_src.dstBuffer;
    dstBufferOffset = copy_src.dstBufferOffset;
    dstBufferRange = copy_src.dstBufferRange;
    srcPictureResource.initialize(&copy_src.srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src.precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoEncodeInfoKHR::~safe_VkVideoEncodeInfoKHR() {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeInfoKHR::initialize(const VkVideoEncodeInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    dstBuffer = in_struct->dstBuffer;
    dstBufferOffset = in_struct->dstBufferOffset;
    dstBufferRange = in_struct->dstBufferRange;
    srcPictureResource.initialize(&in_struct->srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = in_struct->precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoEncodeInfoKHR::initialize(const safe_VkVideoEncodeInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    dstBuffer = copy_src->dstBuffer;
    dstBufferOffset = copy_src->dstBufferOffset;
    dstBufferRange = copy_src->dstBufferRange;
    srcPictureResource.initialize(&copy_src->srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src->precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src->pSetupReferenceSlot);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR(const VkVideoEncodeCapabilitiesKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      rateControlModes(in_struct->rateControlModes),
      maxRateControlLayers(in_struct->maxRateControlLayers),
      maxBitrate(in_struct->maxBitrate),
      maxQualityLevels(in_struct->maxQualityLevels),
      encodeInputPictureGranularity(in_struct->encodeInputPictureGranularity),
      supportedEncodeFeedbackFlags(in_struct->supportedEncodeFeedbackFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      rateControlModes(),
      maxRateControlLayers(),
      maxBitrate(),
      maxQualityLevels(),
      encodeInputPictureGranularity(),
      supportedEncodeFeedbackFlags() {}

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR(const safe_VkVideoEncodeCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlModes = copy_src.rateControlModes;
    maxRateControlLayers = copy_src.maxRateControlLayers;
    maxBitrate = copy_src.maxBitrate;
    maxQualityLevels = copy_src.maxQualityLevels;
    encodeInputPictureGranularity = copy_src.encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src.supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeCapabilitiesKHR& safe_VkVideoEncodeCapabilitiesKHR::operator=(const safe_VkVideoEncodeCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlModes = copy_src.rateControlModes;
    maxRateControlLayers = copy_src.maxRateControlLayers;
    maxBitrate = copy_src.maxBitrate;
    maxQualityLevels = copy_src.maxQualityLevels;
    encodeInputPictureGranularity = copy_src.encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src.supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeCapabilitiesKHR::~safe_VkVideoEncodeCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeCapabilitiesKHR::initialize(const VkVideoEncodeCapabilitiesKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    rateControlModes = in_struct->rateControlModes;
    maxRateControlLayers = in_struct->maxRateControlLayers;
    maxBitrate = in_struct->maxBitrate;
    maxQualityLevels = in_struct->maxQualityLevels;
    encodeInputPictureGranularity = in_struct->encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = in_struct->supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeCapabilitiesKHR::initialize(const safe_VkVideoEncodeCapabilitiesKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    rateControlModes = copy_src->rateControlModes;
    maxRateControlLayers = copy_src->maxRateControlLayers;
    maxBitrate = copy_src->maxBitrate;
    maxQualityLevels = copy_src->maxQualityLevels;
    encodeInputPictureGranularity = copy_src->encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src->supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(
    const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), encodeFeedbackFlags(in_struct->encodeFeedbackFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR), pNext(nullptr), encodeFeedbackFlags() {}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(
    const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    encodeFeedbackFlags = copy_src.encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::operator=(
    const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    encodeFeedbackFlags = copy_src.encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::~safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::initialize(const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    encodeFeedbackFlags = in_struct->encodeFeedbackFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::initialize(const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    encodeFeedbackFlags = copy_src->encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR(const VkVideoEncodeUsageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      videoUsageHints(in_struct->videoUsageHints),
      videoContentHints(in_struct->videoContentHints),
      tuningMode(in_struct->tuningMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR), pNext(nullptr), videoUsageHints(), videoContentHints(), tuningMode() {}

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR(const safe_VkVideoEncodeUsageInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    videoContentHints = copy_src.videoContentHints;
    tuningMode = copy_src.tuningMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeUsageInfoKHR& safe_VkVideoEncodeUsageInfoKHR::operator=(const safe_VkVideoEncodeUsageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    videoContentHints = copy_src.videoContentHints;
    tuningMode = copy_src.tuningMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeUsageInfoKHR::~safe_VkVideoEncodeUsageInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeUsageInfoKHR::initialize(const VkVideoEncodeUsageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoUsageHints = in_struct->videoUsageHints;
    videoContentHints = in_struct->videoContentHints;
    tuningMode = in_struct->tuningMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeUsageInfoKHR::initialize(const safe_VkVideoEncodeUsageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoUsageHints = copy_src->videoUsageHints;
    videoContentHints = copy_src->videoContentHints;
    tuningMode = copy_src->tuningMode;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR(
    const VkVideoEncodeRateControlLayerInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      averageBitrate(in_struct->averageBitrate),
      maxBitrate(in_struct->maxBitrate),
      frameRateNumerator(in_struct->frameRateNumerator),
      frameRateDenominator(in_struct->frameRateDenominator) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR),
      pNext(nullptr),
      averageBitrate(),
      maxBitrate(),
      frameRateNumerator(),
      frameRateDenominator() {}

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR(
    const safe_VkVideoEncodeRateControlLayerInfoKHR& copy_src) {
    sType = copy_src.sType;
    averageBitrate = copy_src.averageBitrate;
    maxBitrate = copy_src.maxBitrate;
    frameRateNumerator = copy_src.frameRateNumerator;
    frameRateDenominator = copy_src.frameRateDenominator;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeRateControlLayerInfoKHR& safe_VkVideoEncodeRateControlLayerInfoKHR::operator=(
    const safe_VkVideoEncodeRateControlLayerInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    averageBitrate = copy_src.averageBitrate;
    maxBitrate = copy_src.maxBitrate;
    frameRateNumerator = copy_src.frameRateNumerator;
    frameRateDenominator = copy_src.frameRateDenominator;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeRateControlLayerInfoKHR::~safe_VkVideoEncodeRateControlLayerInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeRateControlLayerInfoKHR::initialize(const VkVideoEncodeRateControlLayerInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    averageBitrate = in_struct->averageBitrate;
    maxBitrate = in_struct->maxBitrate;
    frameRateNumerator = in_struct->frameRateNumerator;
    frameRateDenominator = in_struct->frameRateDenominator;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeRateControlLayerInfoKHR::initialize(const safe_VkVideoEncodeRateControlLayerInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    averageBitrate = copy_src->averageBitrate;
    maxBitrate = copy_src->maxBitrate;
    frameRateNumerator = copy_src->frameRateNumerator;
    frameRateDenominator = copy_src->frameRateDenominator;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR(const VkVideoEncodeRateControlInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      rateControlMode(in_struct->rateControlMode),
      layerCount(in_struct->layerCount),
      pLayers(nullptr),
      virtualBufferSizeInMs(in_struct->virtualBufferSizeInMs),
      initialVirtualBufferSizeInMs(in_struct->initialVirtualBufferSizeInMs) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (layerCount && in_struct->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&in_struct->pLayers[i]);
        }
    }
}

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR),
      pNext(nullptr),
      flags(),
      rateControlMode(),
      layerCount(),
      pLayers(nullptr),
      virtualBufferSizeInMs(),
      initialVirtualBufferSizeInMs() {}

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR(const safe_VkVideoEncodeRateControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlMode = copy_src.rateControlMode;
    layerCount = copy_src.layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src.virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src.initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layerCount && copy_src.pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src.pLayers[i]);
        }
    }
}

safe_VkVideoEncodeRateControlInfoKHR& safe_VkVideoEncodeRateControlInfoKHR::operator=(
    const safe_VkVideoEncodeRateControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlMode = copy_src.rateControlMode;
    layerCount = copy_src.layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src.virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src.initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layerCount && copy_src.pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src.pLayers[i]);
        }
    }

    return *this;
}

safe_VkVideoEncodeRateControlInfoKHR::~safe_VkVideoEncodeRateControlInfoKHR() {
    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeRateControlInfoKHR::initialize(const VkVideoEncodeRateControlInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    rateControlMode = in_struct->rateControlMode;
    layerCount = in_struct->layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = in_struct->virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = in_struct->initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (layerCount && in_struct->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&in_struct->pLayers[i]);
        }
    }
}

void safe_VkVideoEncodeRateControlInfoKHR::initialize(const safe_VkVideoEncodeRateControlInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    rateControlMode = copy_src->rateControlMode;
    layerCount = copy_src->layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src->virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src->initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src->pNext);
    if (layerCount && copy_src->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src->pLayers[i]);
        }
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR(
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pVideoProfile(nullptr), qualityLevel(in_struct->qualityLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR),
      pNext(nullptr),
      pVideoProfile(nullptr),
      qualityLevel() {}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& copy_src) {
    sType = copy_src.sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::operator=(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    return *this;
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::~safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR() {
    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::initialize(
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pVideoProfile = nullptr;
    qualityLevel = in_struct->qualityLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);
}

void safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::initialize(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src->qualityLevel;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src->pVideoProfile);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR(
    const VkVideoEncodeQualityLevelPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      preferredRateControlMode(in_struct->preferredRateControlMode),
      preferredRateControlLayerCount(in_struct->preferredRateControlLayerCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR),
      pNext(nullptr),
      preferredRateControlMode(),
      preferredRateControlLayerCount() {}

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR(
    const safe_VkVideoEncodeQualityLevelPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    preferredRateControlMode = copy_src.preferredRateControlMode;
    preferredRateControlLayerCount = copy_src.preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQualityLevelPropertiesKHR& safe_VkVideoEncodeQualityLevelPropertiesKHR::operator=(
    const safe_VkVideoEncodeQualityLevelPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    preferredRateControlMode = copy_src.preferredRateControlMode;
    preferredRateControlLayerCount = copy_src.preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQualityLevelPropertiesKHR::~safe_VkVideoEncodeQualityLevelPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQualityLevelPropertiesKHR::initialize(const VkVideoEncodeQualityLevelPropertiesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    preferredRateControlMode = in_struct->preferredRateControlMode;
    preferredRateControlLayerCount = in_struct->preferredRateControlLayerCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQualityLevelPropertiesKHR::initialize(const safe_VkVideoEncodeQualityLevelPropertiesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    preferredRateControlMode = copy_src->preferredRateControlMode;
    preferredRateControlLayerCount = copy_src->preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR(const VkVideoEncodeQualityLevelInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), qualityLevel(in_struct->qualityLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR), pNext(nullptr), qualityLevel() {}

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR(
    const safe_VkVideoEncodeQualityLevelInfoKHR& copy_src) {
    sType = copy_src.sType;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQualityLevelInfoKHR& safe_VkVideoEncodeQualityLevelInfoKHR::operator=(
    const safe_VkVideoEncodeQualityLevelInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQualityLevelInfoKHR::~safe_VkVideoEncodeQualityLevelInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQualityLevelInfoKHR::initialize(const VkVideoEncodeQualityLevelInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    qualityLevel = in_struct->qualityLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQualityLevelInfoKHR::initialize(const safe_VkVideoEncodeQualityLevelInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    qualityLevel = copy_src->qualityLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR(
    const VkVideoEncodeSessionParametersGetInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoSessionParameters(in_struct->videoSessionParameters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR), pNext(nullptr), videoSessionParameters() {}

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR(
    const safe_VkVideoEncodeSessionParametersGetInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoSessionParameters = copy_src.videoSessionParameters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeSessionParametersGetInfoKHR& safe_VkVideoEncodeSessionParametersGetInfoKHR::operator=(
    const safe_VkVideoEncodeSessionParametersGetInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoSessionParameters = copy_src.videoSessionParameters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeSessionParametersGetInfoKHR::~safe_VkVideoEncodeSessionParametersGetInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeSessionParametersGetInfoKHR::initialize(const VkVideoEncodeSessionParametersGetInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoSessionParameters = in_struct->videoSessionParameters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeSessionParametersGetInfoKHR::initialize(const safe_VkVideoEncodeSessionParametersGetInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoSessionParameters = copy_src->videoSessionParameters;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR(
    const VkVideoEncodeSessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), hasOverrides(in_struct->hasOverrides) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR), pNext(nullptr), hasOverrides() {}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& copy_src) {
    sType = copy_src.sType;
    hasOverrides = copy_src.hasOverrides;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::operator=(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hasOverrides = copy_src.hasOverrides;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::~safe_VkVideoEncodeSessionParametersFeedbackInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::initialize(const VkVideoEncodeSessionParametersFeedbackInfoKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hasOverrides = in_struct->hasOverrides;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::initialize(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hasOverrides = copy_src->hasOverrides;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(
    const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentShaderBarycentric(in_struct->fragmentShaderBarycentric) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR),
      pNext(nullptr),
      fragmentShaderBarycentric() {}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    fragmentShaderBarycentric = copy_src.fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentShaderBarycentric = copy_src.fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::~safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::initialize(
    const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentShaderBarycentric = in_struct->fragmentShaderBarycentric;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentShaderBarycentric = copy_src->fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(
    const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      triStripVertexOrderIndependentOfProvokingVertex(in_struct->triStripVertexOrderIndependentOfProvokingVertex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR),
      pNext(nullptr),
      triStripVertexOrderIndependentOfProvokingVertex() {}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src.triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src.triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::~safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::initialize(
    const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    triStripVertexOrderIndependentOfProvokingVertex = in_struct->triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src->triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(
    const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderSubgroupUniformControlFlow(in_struct->shaderSubgroupUniformControlFlow) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR),
      pNext(nullptr),
      shaderSubgroupUniformControlFlow() {}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderSubgroupUniformControlFlow = copy_src.shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR&
safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSubgroupUniformControlFlow = copy_src.shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::
    ~safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::initialize(
    const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSubgroupUniformControlFlow = in_struct->shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSubgroupUniformControlFlow = copy_src->shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(
    const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      workgroupMemoryExplicitLayout(in_struct->workgroupMemoryExplicitLayout),
      workgroupMemoryExplicitLayoutScalarBlockLayout(in_struct->workgroupMemoryExplicitLayoutScalarBlockLayout),
      workgroupMemoryExplicitLayout8BitAccess(in_struct->workgroupMemoryExplicitLayout8BitAccess),
      workgroupMemoryExplicitLayout16BitAccess(in_struct->workgroupMemoryExplicitLayout16BitAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR),
      pNext(nullptr),
      workgroupMemoryExplicitLayout(),
      workgroupMemoryExplicitLayoutScalarBlockLayout(),
      workgroupMemoryExplicitLayout8BitAccess(),
      workgroupMemoryExplicitLayout16BitAccess() {}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    workgroupMemoryExplicitLayout = copy_src.workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src.workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src.workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src.workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR&
safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    workgroupMemoryExplicitLayout = copy_src.workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src.workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src.workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src.workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::~safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::initialize(
    const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    workgroupMemoryExplicitLayout = in_struct->workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = in_struct->workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = in_struct->workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = in_struct->workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    workgroupMemoryExplicitLayout = copy_src->workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src->workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src->workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src->workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(
    const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      rayTracingMaintenance1(in_struct->rayTracingMaintenance1),
      rayTracingPipelineTraceRaysIndirect2(in_struct->rayTracingPipelineTraceRaysIndirect2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR),
      pNext(nullptr),
      rayTracingMaintenance1(),
      rayTracingPipelineTraceRaysIndirect2() {}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingMaintenance1 = copy_src.rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src.rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingMaintenance1 = copy_src.rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src.rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::~safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::initialize(
    const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingMaintenance1 = in_struct->rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = in_struct->rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingMaintenance1 = copy_src->rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src->rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance5FeaturesKHR::safe_VkPhysicalDeviceMaintenance5FeaturesKHR(
    const VkPhysicalDeviceMaintenance5FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maintenance5(in_struct->maintenance5) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance5FeaturesKHR::safe_VkPhysicalDeviceMaintenance5FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR), pNext(nullptr), maintenance5() {}

safe_VkPhysicalDeviceMaintenance5FeaturesKHR::safe_VkPhysicalDeviceMaintenance5FeaturesKHR(
    const safe_VkPhysicalDeviceMaintenance5FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    maintenance5 = copy_src.maintenance5;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance5FeaturesKHR& safe_VkPhysicalDeviceMaintenance5FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance5FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maintenance5 = copy_src.maintenance5;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance5FeaturesKHR::~safe_VkPhysicalDeviceMaintenance5FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance5FeaturesKHR::initialize(const VkPhysicalDeviceMaintenance5FeaturesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maintenance5 = in_struct->maintenance5;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance5FeaturesKHR::initialize(const safe_VkPhysicalDeviceMaintenance5FeaturesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maintenance5 = copy_src->maintenance5;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance5PropertiesKHR::safe_VkPhysicalDeviceMaintenance5PropertiesKHR(
    const VkPhysicalDeviceMaintenance5PropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      earlyFragmentMultisampleCoverageAfterSampleCounting(in_struct->earlyFragmentMultisampleCoverageAfterSampleCounting),
      earlyFragmentSampleMaskTestBeforeSampleCounting(in_struct->earlyFragmentSampleMaskTestBeforeSampleCounting),
      depthStencilSwizzleOneSupport(in_struct->depthStencilSwizzleOneSupport),
      polygonModePointSize(in_struct->polygonModePointSize),
      nonStrictSinglePixelWideLinesUseParallelogram(in_struct->nonStrictSinglePixelWideLinesUseParallelogram),
      nonStrictWideLinesUseParallelogram(in_struct->nonStrictWideLinesUseParallelogram) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance5PropertiesKHR::safe_VkPhysicalDeviceMaintenance5PropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR),
      pNext(nullptr),
      earlyFragmentMultisampleCoverageAfterSampleCounting(),
      earlyFragmentSampleMaskTestBeforeSampleCounting(),
      depthStencilSwizzleOneSupport(),
      polygonModePointSize(),
      nonStrictSinglePixelWideLinesUseParallelogram(),
      nonStrictWideLinesUseParallelogram() {}

safe_VkPhysicalDeviceMaintenance5PropertiesKHR::safe_VkPhysicalDeviceMaintenance5PropertiesKHR(
    const safe_VkPhysicalDeviceMaintenance5PropertiesKHR& copy_src) {
    sType = copy_src.sType;
    earlyFragmentMultisampleCoverageAfterSampleCounting = copy_src.earlyFragmentMultisampleCoverageAfterSampleCounting;
    earlyFragmentSampleMaskTestBeforeSampleCounting = copy_src.earlyFragmentSampleMaskTestBeforeSampleCounting;
    depthStencilSwizzleOneSupport = copy_src.depthStencilSwizzleOneSupport;
    polygonModePointSize = copy_src.polygonModePointSize;
    nonStrictSinglePixelWideLinesUseParallelogram = copy_src.nonStrictSinglePixelWideLinesUseParallelogram;
    nonStrictWideLinesUseParallelogram = copy_src.nonStrictWideLinesUseParallelogram;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance5PropertiesKHR& safe_VkPhysicalDeviceMaintenance5PropertiesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance5PropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    earlyFragmentMultisampleCoverageAfterSampleCounting = copy_src.earlyFragmentMultisampleCoverageAfterSampleCounting;
    earlyFragmentSampleMaskTestBeforeSampleCounting = copy_src.earlyFragmentSampleMaskTestBeforeSampleCounting;
    depthStencilSwizzleOneSupport = copy_src.depthStencilSwizzleOneSupport;
    polygonModePointSize = copy_src.polygonModePointSize;
    nonStrictSinglePixelWideLinesUseParallelogram = copy_src.nonStrictSinglePixelWideLinesUseParallelogram;
    nonStrictWideLinesUseParallelogram = copy_src.nonStrictWideLinesUseParallelogram;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance5PropertiesKHR::~safe_VkPhysicalDeviceMaintenance5PropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance5PropertiesKHR::initialize(const VkPhysicalDeviceMaintenance5PropertiesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    earlyFragmentMultisampleCoverageAfterSampleCounting = in_struct->earlyFragmentMultisampleCoverageAfterSampleCounting;
    earlyFragmentSampleMaskTestBeforeSampleCounting = in_struct->earlyFragmentSampleMaskTestBeforeSampleCounting;
    depthStencilSwizzleOneSupport = in_struct->depthStencilSwizzleOneSupport;
    polygonModePointSize = in_struct->polygonModePointSize;
    nonStrictSinglePixelWideLinesUseParallelogram = in_struct->nonStrictSinglePixelWideLinesUseParallelogram;
    nonStrictWideLinesUseParallelogram = in_struct->nonStrictWideLinesUseParallelogram;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance5PropertiesKHR::initialize(const safe_VkPhysicalDeviceMaintenance5PropertiesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    earlyFragmentMultisampleCoverageAfterSampleCounting = copy_src->earlyFragmentMultisampleCoverageAfterSampleCounting;
    earlyFragmentSampleMaskTestBeforeSampleCounting = copy_src->earlyFragmentSampleMaskTestBeforeSampleCounting;
    depthStencilSwizzleOneSupport = copy_src->depthStencilSwizzleOneSupport;
    polygonModePointSize = copy_src->polygonModePointSize;
    nonStrictSinglePixelWideLinesUseParallelogram = copy_src->nonStrictSinglePixelWideLinesUseParallelogram;
    nonStrictWideLinesUseParallelogram = copy_src->nonStrictWideLinesUseParallelogram;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderingAreaInfoKHR::safe_VkRenderingAreaInfoKHR(const VkRenderingAreaInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      viewMask(in_struct->viewMask),
      colorAttachmentCount(in_struct->colorAttachmentCount),
      pColorAttachmentFormats(nullptr),
      depthAttachmentFormat(in_struct->depthAttachmentFormat),
      stencilAttachmentFormat(in_struct->stencilAttachmentFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pColorAttachmentFormats) {
        pColorAttachmentFormats = new VkFormat[in_struct->colorAttachmentCount];
        memcpy((void*)pColorAttachmentFormats, (void*)in_struct->pColorAttachmentFormats,
               sizeof(VkFormat) * in_struct->colorAttachmentCount);
    }
}

safe_VkRenderingAreaInfoKHR::safe_VkRenderingAreaInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RENDERING_AREA_INFO_KHR),
      pNext(nullptr),
      viewMask(),
      colorAttachmentCount(),
      pColorAttachmentFormats(nullptr),
      depthAttachmentFormat(),
      stencilAttachmentFormat() {}

safe_VkRenderingAreaInfoKHR::safe_VkRenderingAreaInfoKHR(const safe_VkRenderingAreaInfoKHR& copy_src) {
    sType = copy_src.sType;
    viewMask = copy_src.viewMask;
    colorAttachmentCount = copy_src.colorAttachmentCount;
    pColorAttachmentFormats = nullptr;
    depthAttachmentFormat = copy_src.depthAttachmentFormat;
    stencilAttachmentFormat = copy_src.stencilAttachmentFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorAttachmentFormats) {
        pColorAttachmentFormats = new VkFormat[copy_src.colorAttachmentCount];
        memcpy((void*)pColorAttachmentFormats, (void*)copy_src.pColorAttachmentFormats,
               sizeof(VkFormat) * copy_src.colorAttachmentCount);
    }
}

safe_VkRenderingAreaInfoKHR& safe_VkRenderingAreaInfoKHR::operator=(const safe_VkRenderingAreaInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pColorAttachmentFormats) delete[] pColorAttachmentFormats;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    viewMask = copy_src.viewMask;
    colorAttachmentCount = copy_src.colorAttachmentCount;
    pColorAttachmentFormats = nullptr;
    depthAttachmentFormat = copy_src.depthAttachmentFormat;
    stencilAttachmentFormat = copy_src.stencilAttachmentFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorAttachmentFormats) {
        pColorAttachmentFormats = new VkFormat[copy_src.colorAttachmentCount];
        memcpy((void*)pColorAttachmentFormats, (void*)copy_src.pColorAttachmentFormats,
               sizeof(VkFormat) * copy_src.colorAttachmentCount);
    }

    return *this;
}

safe_VkRenderingAreaInfoKHR::~safe_VkRenderingAreaInfoKHR() {
    if (pColorAttachmentFormats) delete[] pColorAttachmentFormats;
    FreePnextChain(pNext);
}

void safe_VkRenderingAreaInfoKHR::initialize(const VkRenderingAreaInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pColorAttachmentFormats) delete[] pColorAttachmentFormats;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    viewMask = in_struct->viewMask;
    colorAttachmentCount = in_struct->colorAttachmentCount;
    pColorAttachmentFormats = nullptr;
    depthAttachmentFormat = in_struct->depthAttachmentFormat;
    stencilAttachmentFormat = in_struct->stencilAttachmentFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pColorAttachmentFormats) {
        pColorAttachmentFormats = new VkFormat[in_struct->colorAttachmentCount];
        memcpy((void*)pColorAttachmentFormats, (void*)in_struct->pColorAttachmentFormats,
               sizeof(VkFormat) * in_struct->colorAttachmentCount);
    }
}

void safe_VkRenderingAreaInfoKHR::initialize(const safe_VkRenderingAreaInfoKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    viewMask = copy_src->viewMask;
    colorAttachmentCount = copy_src->colorAttachmentCount;
    pColorAttachmentFormats = nullptr;
    depthAttachmentFormat = copy_src->depthAttachmentFormat;
    stencilAttachmentFormat = copy_src->stencilAttachmentFormat;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pColorAttachmentFormats) {
        pColorAttachmentFormats = new VkFormat[copy_src->colorAttachmentCount];
        memcpy((void*)pColorAttachmentFormats, (void*)copy_src->pColorAttachmentFormats,
               sizeof(VkFormat) * copy_src->colorAttachmentCount);
    }
}

safe_VkImageSubresource2KHR::safe_VkImageSubresource2KHR(const VkImageSubresource2KHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageSubresource(in_struct->imageSubresource) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageSubresource2KHR::safe_VkImageSubresource2KHR()
    : sType(VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2_KHR), pNext(nullptr), imageSubresource() {}

safe_VkImageSubresource2KHR::safe_VkImageSubresource2KHR(const safe_VkImageSubresource2KHR& copy_src) {
    sType = copy_src.sType;
    imageSubresource = copy_src.imageSubresource;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageSubresource2KHR& safe_VkImageSubresource2KHR::operator=(const safe_VkImageSubresource2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageSubresource = copy_src.imageSubresource;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageSubresource2KHR::~safe_VkImageSubresource2KHR() { FreePnextChain(pNext); }

void safe_VkImageSubresource2KHR::initialize(const VkImageSubresource2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageSubresource = in_struct->imageSubresource;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageSubresource2KHR::initialize(const safe_VkImageSubresource2KHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageSubresource = copy_src->imageSubresource;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceImageSubresourceInfoKHR::safe_VkDeviceImageSubresourceInfoKHR(const VkDeviceImageSubresourceInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pCreateInfo(nullptr), pSubresource(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pCreateInfo) pCreateInfo = new safe_VkImageCreateInfo(in_struct->pCreateInfo);
    if (in_struct->pSubresource) pSubresource = new safe_VkImageSubresource2KHR(in_struct->pSubresource);
}

safe_VkDeviceImageSubresourceInfoKHR::safe_VkDeviceImageSubresourceInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_IMAGE_SUBRESOURCE_INFO_KHR), pNext(nullptr), pCreateInfo(nullptr), pSubresource(nullptr) {}

safe_VkDeviceImageSubresourceInfoKHR::safe_VkDeviceImageSubresourceInfoKHR(const safe_VkDeviceImageSubresourceInfoKHR& copy_src) {
    sType = copy_src.sType;
    pCreateInfo = nullptr;
    pSubresource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pCreateInfo) pCreateInfo = new safe_VkImageCreateInfo(*copy_src.pCreateInfo);
    if (copy_src.pSubresource) pSubresource = new safe_VkImageSubresource2KHR(*copy_src.pSubresource);
}

safe_VkDeviceImageSubresourceInfoKHR& safe_VkDeviceImageSubresourceInfoKHR::operator=(
    const safe_VkDeviceImageSubresourceInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pCreateInfo) delete pCreateInfo;
    if (pSubresource) delete pSubresource;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pCreateInfo = nullptr;
    pSubresource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pCreateInfo) pCreateInfo = new safe_VkImageCreateInfo(*copy_src.pCreateInfo);
    if (copy_src.pSubresource) pSubresource = new safe_VkImageSubresource2KHR(*copy_src.pSubresource);

    return *this;
}

safe_VkDeviceImageSubresourceInfoKHR::~safe_VkDeviceImageSubresourceInfoKHR() {
    if (pCreateInfo) delete pCreateInfo;
    if (pSubresource) delete pSubresource;
    FreePnextChain(pNext);
}

void safe_VkDeviceImageSubresourceInfoKHR::initialize(const VkDeviceImageSubresourceInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pCreateInfo) delete pCreateInfo;
    if (pSubresource) delete pSubresource;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pCreateInfo = nullptr;
    pSubresource = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pCreateInfo) pCreateInfo = new safe_VkImageCreateInfo(in_struct->pCreateInfo);
    if (in_struct->pSubresource) pSubresource = new safe_VkImageSubresource2KHR(in_struct->pSubresource);
}

void safe_VkDeviceImageSubresourceInfoKHR::initialize(const safe_VkDeviceImageSubresourceInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pCreateInfo = nullptr;
    pSubresource = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pCreateInfo) pCreateInfo = new safe_VkImageCreateInfo(*copy_src->pCreateInfo);
    if (copy_src->pSubresource) pSubresource = new safe_VkImageSubresource2KHR(*copy_src->pSubresource);
}

safe_VkSubresourceLayout2KHR::safe_VkSubresourceLayout2KHR(const VkSubresourceLayout2KHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), subresourceLayout(in_struct->subresourceLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSubresourceLayout2KHR::safe_VkSubresourceLayout2KHR()
    : sType(VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2_KHR), pNext(nullptr), subresourceLayout() {}

safe_VkSubresourceLayout2KHR::safe_VkSubresourceLayout2KHR(const safe_VkSubresourceLayout2KHR& copy_src) {
    sType = copy_src.sType;
    subresourceLayout = copy_src.subresourceLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSubresourceLayout2KHR& safe_VkSubresourceLayout2KHR::operator=(const safe_VkSubresourceLayout2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    subresourceLayout = copy_src.subresourceLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSubresourceLayout2KHR::~safe_VkSubresourceLayout2KHR() { FreePnextChain(pNext); }

void safe_VkSubresourceLayout2KHR::initialize(const VkSubresourceLayout2KHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    subresourceLayout = in_struct->subresourceLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSubresourceLayout2KHR::initialize(const safe_VkSubresourceLayout2KHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    subresourceLayout = copy_src->subresourceLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineCreateFlags2CreateInfoKHR::safe_VkPipelineCreateFlags2CreateInfoKHR(
    const VkPipelineCreateFlags2CreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineCreateFlags2CreateInfoKHR::safe_VkPipelineCreateFlags2CreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR), pNext(nullptr), flags() {}

safe_VkPipelineCreateFlags2CreateInfoKHR::safe_VkPipelineCreateFlags2CreateInfoKHR(
    const safe_VkPipelineCreateFlags2CreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineCreateFlags2CreateInfoKHR& safe_VkPipelineCreateFlags2CreateInfoKHR::operator=(
    const safe_VkPipelineCreateFlags2CreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineCreateFlags2CreateInfoKHR::~safe_VkPipelineCreateFlags2CreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineCreateFlags2CreateInfoKHR::initialize(const VkPipelineCreateFlags2CreateInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineCreateFlags2CreateInfoKHR::initialize(const safe_VkPipelineCreateFlags2CreateInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferUsageFlags2CreateInfoKHR::safe_VkBufferUsageFlags2CreateInfoKHR(const VkBufferUsageFlags2CreateInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), usage(in_struct->usage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferUsageFlags2CreateInfoKHR::safe_VkBufferUsageFlags2CreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR), pNext(nullptr), usage() {}

safe_VkBufferUsageFlags2CreateInfoKHR::safe_VkBufferUsageFlags2CreateInfoKHR(
    const safe_VkBufferUsageFlags2CreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferUsageFlags2CreateInfoKHR& safe_VkBufferUsageFlags2CreateInfoKHR::operator=(
    const safe_VkBufferUsageFlags2CreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferUsageFlags2CreateInfoKHR::~safe_VkBufferUsageFlags2CreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkBufferUsageFlags2CreateInfoKHR::initialize(const VkBufferUsageFlags2CreateInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    usage = in_struct->usage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferUsageFlags2CreateInfoKHR::initialize(const safe_VkBufferUsageFlags2CreateInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    usage = copy_src->usage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(
    const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), rayTracingPositionFetch(in_struct->rayTracingPositionFetch) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR), pNext(nullptr), rayTracingPositionFetch() {}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingPositionFetch = copy_src.rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingPositionFetch = copy_src.rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::~safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::initialize(
    const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingPositionFetch = in_struct->rayTracingPositionFetch;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingPositionFetch = copy_src->rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR(const VkCooperativeMatrixPropertiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      MSize(in_struct->MSize),
      NSize(in_struct->NSize),
      KSize(in_struct->KSize),
      AType(in_struct->AType),
      BType(in_struct->BType),
      CType(in_struct->CType),
      ResultType(in_struct->ResultType),
      saturatingAccumulation(in_struct->saturatingAccumulation),
      scope(in_struct->scope) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR),
      pNext(nullptr),
      MSize(),
      NSize(),
      KSize(),
      AType(),
      BType(),
      CType(),
      ResultType(),
      saturatingAccumulation(),
      scope() {}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR(
    const safe_VkCooperativeMatrixPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCooperativeMatrixPropertiesKHR& safe_VkCooperativeMatrixPropertiesKHR::operator=(
    const safe_VkCooperativeMatrixPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCooperativeMatrixPropertiesKHR::~safe_VkCooperativeMatrixPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkCooperativeMatrixPropertiesKHR::initialize(const VkCooperativeMatrixPropertiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    MSize = in_struct->MSize;
    NSize = in_struct->NSize;
    KSize = in_struct->KSize;
    AType = in_struct->AType;
    BType = in_struct->BType;
    CType = in_struct->CType;
    ResultType = in_struct->ResultType;
    saturatingAccumulation = in_struct->saturatingAccumulation;
    scope = in_struct->scope;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCooperativeMatrixPropertiesKHR::initialize(const safe_VkCooperativeMatrixPropertiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    MSize = copy_src->MSize;
    NSize = copy_src->NSize;
    KSize = copy_src->KSize;
    AType = copy_src->AType;
    BType = copy_src->BType;
    CType = copy_src->CType;
    ResultType = copy_src->ResultType;
    saturatingAccumulation = copy_src->saturatingAccumulation;
    scope = copy_src->scope;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(
    const VkPhysicalDeviceCooperativeMatrixFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeMatrix(in_struct->cooperativeMatrix),
      cooperativeMatrixRobustBufferAccess(in_struct->cooperativeMatrixRobustBufferAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR),
      pNext(nullptr),
      cooperativeMatrix(),
      cooperativeMatrixRobustBufferAccess() {}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::~safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::initialize(const VkPhysicalDeviceCooperativeMatrixFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrix = in_struct->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = in_struct->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrix = copy_src->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(
    const VkPhysicalDeviceCooperativeMatrixPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cooperativeMatrixSupportedStages(in_struct->cooperativeMatrixSupportedStages) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR),
      pNext(nullptr),
      cooperativeMatrixSupportedStages() {}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::~safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::initialize(
    const VkPhysicalDeviceCooperativeMatrixPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrixSupportedStages = in_struct->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrixSupportedStages = copy_src->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR(const VkDeviceOrHostAddressConstKHR* in_struct,
                                                                       PNextCopyState*) {
    initialize(in_struct);
}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR() : hostAddress(nullptr) {}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR(const safe_VkDeviceOrHostAddressConstKHR& copy_src) {
    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;
}

safe_VkDeviceOrHostAddressConstKHR& safe_VkDeviceOrHostAddressConstKHR::operator=(
    const safe_VkDeviceOrHostAddressConstKHR& copy_src) {
    if (&copy_src == this) return *this;

    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;

    return *this;
}

safe_VkDeviceOrHostAddressConstKHR::~safe_VkDeviceOrHostAddressConstKHR() {}

void safe_VkDeviceOrHostAddressConstKHR::initialize(const VkDeviceOrHostAddressConstKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = in_struct->deviceAddress;
    hostAddress = in_struct->hostAddress;
}

void safe_VkDeviceOrHostAddressConstKHR::initialize(const safe_VkDeviceOrHostAddressConstKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = copy_src->deviceAddress;
    hostAddress = copy_src->hostAddress;
}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR(const VkDeviceOrHostAddressKHR* in_struct, PNextCopyState*) {
    initialize(in_struct);
}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR() : hostAddress(nullptr) {}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR(const safe_VkDeviceOrHostAddressKHR& copy_src) {
    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;
}

safe_VkDeviceOrHostAddressKHR& safe_VkDeviceOrHostAddressKHR::operator=(const safe_VkDeviceOrHostAddressKHR& copy_src) {
    if (&copy_src == this) return *this;

    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;

    return *this;
}

safe_VkDeviceOrHostAddressKHR::~safe_VkDeviceOrHostAddressKHR() {}

void safe_VkDeviceOrHostAddressKHR::initialize(const VkDeviceOrHostAddressKHR* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = in_struct->deviceAddress;
    hostAddress = in_struct->hostAddress;
}

void safe_VkDeviceOrHostAddressKHR::initialize(const safe_VkDeviceOrHostAddressKHR* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = copy_src->deviceAddress;
    hostAddress = copy_src->hostAddress;
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR(
    const VkAccelerationStructureGeometryTrianglesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      vertexFormat(in_struct->vertexFormat),
      vertexData(&in_struct->vertexData),
      vertexStride(in_struct->vertexStride),
      maxVertex(in_struct->maxVertex),
      indexType(in_struct->indexType),
      indexData(&in_struct->indexData),
      transformData(&in_struct->transformData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR),
      pNext(nullptr),
      vertexFormat(),
      vertexStride(),
      maxVertex(),
      indexType() {}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR& copy_src) {
    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    maxVertex = copy_src.maxVertex;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    transformData.initialize(&copy_src.transformData);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR& safe_VkAccelerationStructureGeometryTrianglesDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    maxVertex = copy_src.maxVertex;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    transformData.initialize(&copy_src.transformData);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::~safe_VkAccelerationStructureGeometryTrianglesDataKHR() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryTrianglesDataKHR::initialize(
    const VkAccelerationStructureGeometryTrianglesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexFormat = in_struct->vertexFormat;
    vertexData.initialize(&in_struct->vertexData);
    vertexStride = in_struct->vertexStride;
    maxVertex = in_struct->maxVertex;
    indexType = in_struct->indexType;
    indexData.initialize(&in_struct->indexData);
    transformData.initialize(&in_struct->transformData);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryTrianglesDataKHR::initialize(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexFormat = copy_src->vertexFormat;
    vertexData.initialize(&copy_src->vertexData);
    vertexStride = copy_src->vertexStride;
    maxVertex = copy_src->maxVertex;
    indexType = copy_src->indexType;
    indexData.initialize(&copy_src->indexData);
    transformData.initialize(&copy_src->transformData);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR(
    const VkAccelerationStructureGeometryAabbsDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), data(&in_struct->data), stride(in_struct->stride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR), pNext(nullptr), stride() {}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR(
    const safe_VkAccelerationStructureGeometryAabbsDataKHR& copy_src) {
    sType = copy_src.sType;
    data.initialize(&copy_src.data);
    stride = copy_src.stride;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryAabbsDataKHR& safe_VkAccelerationStructureGeometryAabbsDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryAabbsDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    data.initialize(&copy_src.data);
    stride = copy_src.stride;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::~safe_VkAccelerationStructureGeometryAabbsDataKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureGeometryAabbsDataKHR::initialize(const VkAccelerationStructureGeometryAabbsDataKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    data.initialize(&in_struct->data);
    stride = in_struct->stride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryAabbsDataKHR::initialize(const safe_VkAccelerationStructureGeometryAabbsDataKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    data.initialize(&copy_src->data);
    stride = copy_src->stride;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR(
    const VkAccelerationStructureGeometryInstancesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), arrayOfPointers(in_struct->arrayOfPointers), data(&in_struct->data) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR), pNext(nullptr), arrayOfPointers() {}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR& copy_src) {
    sType = copy_src.sType;
    arrayOfPointers = copy_src.arrayOfPointers;
    data.initialize(&copy_src.data);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryInstancesDataKHR& safe_VkAccelerationStructureGeometryInstancesDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    arrayOfPointers = copy_src.arrayOfPointers;
    data.initialize(&copy_src.data);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::~safe_VkAccelerationStructureGeometryInstancesDataKHR() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryInstancesDataKHR::initialize(
    const VkAccelerationStructureGeometryInstancesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    arrayOfPointers = in_struct->arrayOfPointers;
    data.initialize(&in_struct->data);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryInstancesDataKHR::initialize(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    arrayOfPointers = copy_src->arrayOfPointers;
    data.initialize(&copy_src->data);
    pNext = SafePnextCopy(copy_src->pNext);
}

struct ASGeomKHRExtraData {
    ASGeomKHRExtraData(uint8_t* alloc, uint32_t primOffset, uint32_t primCount)
        : ptr(alloc), primitiveOffset(primOffset), primitiveCount(primCount) {}
    ~ASGeomKHRExtraData() {
        if (ptr) delete[] ptr;
    }
    uint8_t* ptr;
    uint32_t primitiveOffset;
    uint32_t primitiveCount;
};

vl_concurrent_unordered_map<const safe_VkAccelerationStructureGeometryKHR*, ASGeomKHRExtraData*, 4> as_geom_khr_host_alloc;
safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR(
    const VkAccelerationStructureGeometryKHR* in_struct, const bool is_host,
    const VkAccelerationStructureBuildRangeInfoKHR* build_range_info, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), geometryType(in_struct->geometryType), geometry(in_struct->geometry), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + build_range_info->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(
                allocation + build_range_info->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {
                const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
                pInstances[i] = *(
                    reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(byte_ptr + build_range_info->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        } else {
            const auto primitive_offset = build_range_info->primitiveOffset;
            const auto primitive_count = build_range_info->primitiveCount;
            size_t array_size = primitive_offset + primitive_count * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            auto host_address = static_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
            memcpy(allocation + primitive_offset, host_address + primitive_offset,
                   primitive_count * sizeof(VkAccelerationStructureInstanceKHR));
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        }
    }
}

safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR), pNext(nullptr), geometryType(), geometry(), flags() {}

safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR(
    const safe_VkAccelerationStructureGeometryKHR& copy_src) {
    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    auto src_iter = as_geom_khr_host_alloc.find(&copy_src);
    if (src_iter != as_geom_khr_host_alloc.end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }
}

safe_VkAccelerationStructureGeometryKHR& safe_VkAccelerationStructureGeometryKHR::operator=(
    const safe_VkAccelerationStructureGeometryKHR& copy_src) {
    if (&copy_src == this) return *this;

    auto iter = as_geom_khr_host_alloc.pop(this);
    if (iter != as_geom_khr_host_alloc.end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    auto src_iter = as_geom_khr_host_alloc.find(&copy_src);
    if (src_iter != as_geom_khr_host_alloc.end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }

    return *this;
}

safe_VkAccelerationStructureGeometryKHR::~safe_VkAccelerationStructureGeometryKHR() {
    auto iter = as_geom_khr_host_alloc.pop(this);
    if (iter != as_geom_khr_host_alloc.end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryKHR::initialize(const VkAccelerationStructureGeometryKHR* in_struct, const bool is_host,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* build_range_info,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    auto iter = as_geom_khr_host_alloc.pop(this);
    if (iter != as_geom_khr_host_alloc.end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    geometryType = in_struct->geometryType;
    geometry = in_struct->geometry;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + build_range_info->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(
                allocation + build_range_info->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {
                const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
                pInstances[i] = *(
                    reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(byte_ptr + build_range_info->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        } else {
            const auto primitive_offset = build_range_info->primitiveOffset;
            const auto primitive_count = build_range_info->primitiveCount;
            size_t array_size = primitive_offset + primitive_count * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            auto host_address = static_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
            memcpy(allocation + primitive_offset, host_address + primitive_offset,
                   primitive_count * sizeof(VkAccelerationStructureInstanceKHR));
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        }
    }
}

void safe_VkAccelerationStructureGeometryKHR::initialize(const safe_VkAccelerationStructureGeometryKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    geometryType = copy_src->geometryType;
    geometry = copy_src->geometry;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
    auto src_iter = as_geom_khr_host_alloc.find(copy_src);
    if (src_iter != as_geom_khr_host_alloc.end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            as_geom_khr_host_alloc.insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR(
    const VkAccelerationStructureBuildGeometryInfoKHR* in_struct, const bool is_host,
    const VkAccelerationStructureBuildRangeInfoKHR* build_range_infos, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      flags(in_struct->flags),
      mode(in_struct->mode),
      srcAccelerationStructure(in_struct->srcAccelerationStructure),
      dstAccelerationStructure(in_struct->dstAccelerationStructure),
      geometryCount(in_struct->geometryCount),
      pGeometries(nullptr),
      ppGeometries(nullptr),
      scratchData(&in_struct->scratchData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (geometryCount) {
        if (in_struct->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] =
                    new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                (pGeometries)[i] =
                    safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);
            }
        }
    }
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR),
      pNext(nullptr),
      type(),
      flags(),
      mode(),
      srcAccelerationStructure(),
      dstAccelerationStructure(),
      geometryCount(),
      pGeometries(nullptr),
      ppGeometries(nullptr) {}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR(
    const safe_VkAccelerationStructureBuildGeometryInfoKHR& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    srcAccelerationStructure = copy_src.srcAccelerationStructure;
    dstAccelerationStructure = copy_src.dstAccelerationStructure;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src.scratchData);
    if (geometryCount) {
        if (copy_src.ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);
            }
        }
    }
}

safe_VkAccelerationStructureBuildGeometryInfoKHR& safe_VkAccelerationStructureBuildGeometryInfoKHR::operator=(
    const safe_VkAccelerationStructureBuildGeometryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    srcAccelerationStructure = copy_src.srcAccelerationStructure;
    dstAccelerationStructure = copy_src.dstAccelerationStructure;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src.scratchData);
    if (geometryCount) {
        if (copy_src.ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);
            }
        }
    }

    return *this;
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::~safe_VkAccelerationStructureBuildGeometryInfoKHR() {
    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureBuildGeometryInfoKHR::initialize(const VkAccelerationStructureBuildGeometryInfoKHR* in_struct,
                                                                  const bool is_host,
                                                                  const VkAccelerationStructureBuildRangeInfoKHR* build_range_infos,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    flags = in_struct->flags;
    mode = in_struct->mode;
    srcAccelerationStructure = in_struct->srcAccelerationStructure;
    dstAccelerationStructure = in_struct->dstAccelerationStructure;
    geometryCount = in_struct->geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&in_struct->scratchData);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (geometryCount) {
        if (in_struct->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] =
                    new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                (pGeometries)[i] =
                    safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);
            }
        }
    }
}

void safe_VkAccelerationStructureBuildGeometryInfoKHR::initialize(const safe_VkAccelerationStructureBuildGeometryInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    flags = copy_src->flags;
    mode = copy_src->mode;
    srcAccelerationStructure = copy_src->srcAccelerationStructure;
    dstAccelerationStructure = copy_src->dstAccelerationStructure;
    geometryCount = copy_src->geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src->scratchData);
    if (geometryCount) {
        if (copy_src->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src->ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src->pGeometries[i]);
            }
        }
    }
}

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR(
    const VkAccelerationStructureCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      createFlags(in_struct->createFlags),
      buffer(in_struct->buffer),
      offset(in_struct->offset),
      size(in_struct->size),
      type(in_struct->type),
      deviceAddress(in_struct->deviceAddress) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR),
      pNext(nullptr),
      createFlags(),
      buffer(),
      offset(),
      size(),
      type(),
      deviceAddress() {}

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR(
    const safe_VkAccelerationStructureCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    createFlags = copy_src.createFlags;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    size = copy_src.size;
    type = copy_src.type;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureCreateInfoKHR& safe_VkAccelerationStructureCreateInfoKHR::operator=(
    const safe_VkAccelerationStructureCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    createFlags = copy_src.createFlags;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    size = copy_src.size;
    type = copy_src.type;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureCreateInfoKHR::~safe_VkAccelerationStructureCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureCreateInfoKHR::initialize(const VkAccelerationStructureCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    createFlags = in_struct->createFlags;
    buffer = in_struct->buffer;
    offset = in_struct->offset;
    size = in_struct->size;
    type = in_struct->type;
    deviceAddress = in_struct->deviceAddress;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureCreateInfoKHR::initialize(const safe_VkAccelerationStructureCreateInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    createFlags = copy_src->createFlags;
    buffer = copy_src->buffer;
    offset = copy_src->offset;
    size = copy_src->size;
    type = copy_src->type;
    deviceAddress = copy_src->deviceAddress;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR(
    const VkWriteDescriptorSetAccelerationStructureKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), accelerationStructureCount(in_struct->accelerationStructureCount), pAccelerationStructures(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR()
    : sType(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR),
      pNext(nullptr),
      accelerationStructureCount(),
      pAccelerationStructures(nullptr) {}

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureKHR& safe_VkWriteDescriptorSetAccelerationStructureKHR::operator=(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }

    return *this;
}

safe_VkWriteDescriptorSetAccelerationStructureKHR::~safe_VkWriteDescriptorSetAccelerationStructureKHR() {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
}

void safe_VkWriteDescriptorSetAccelerationStructureKHR::initialize(const VkWriteDescriptorSetAccelerationStructureKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureCount = in_struct->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

void safe_VkWriteDescriptorSetAccelerationStructureKHR::initialize(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureCount = copy_src->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (accelerationStructureCount && copy_src->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src->pAccelerationStructures[i];
        }
    }
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(
    const VkPhysicalDeviceAccelerationStructureFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructure(in_struct->accelerationStructure),
      accelerationStructureCaptureReplay(in_struct->accelerationStructureCaptureReplay),
      accelerationStructureIndirectBuild(in_struct->accelerationStructureIndirectBuild),
      accelerationStructureHostCommands(in_struct->accelerationStructureHostCommands),
      descriptorBindingAccelerationStructureUpdateAfterBind(in_struct->descriptorBindingAccelerationStructureUpdateAfterBind) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR),
      pNext(nullptr),
      accelerationStructure(),
      accelerationStructureCaptureReplay(),
      accelerationStructureIndirectBuild(),
      accelerationStructureHostCommands(),
      descriptorBindingAccelerationStructureUpdateAfterBind() {}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureCaptureReplay = copy_src.accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src.accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src.accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src.descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureCaptureReplay = copy_src.accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src.accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src.accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src.descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::~safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::initialize(
    const VkPhysicalDeviceAccelerationStructureFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    accelerationStructureCaptureReplay = in_struct->accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = in_struct->accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = in_struct->accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = in_struct->descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    accelerationStructureCaptureReplay = copy_src->accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src->accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src->accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src->descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(
    const VkPhysicalDeviceAccelerationStructurePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxGeometryCount(in_struct->maxGeometryCount),
      maxInstanceCount(in_struct->maxInstanceCount),
      maxPrimitiveCount(in_struct->maxPrimitiveCount),
      maxPerStageDescriptorAccelerationStructures(in_struct->maxPerStageDescriptorAccelerationStructures),
      maxPerStageDescriptorUpdateAfterBindAccelerationStructures(
          in_struct->maxPerStageDescriptorUpdateAfterBindAccelerationStructures),
      maxDescriptorSetAccelerationStructures(in_struct->maxDescriptorSetAccelerationStructures),
      maxDescriptorSetUpdateAfterBindAccelerationStructures(in_struct->maxDescriptorSetUpdateAfterBindAccelerationStructures),
      minAccelerationStructureScratchOffsetAlignment(in_struct->minAccelerationStructureScratchOffsetAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR),
      pNext(nullptr),
      maxGeometryCount(),
      maxInstanceCount(),
      maxPrimitiveCount(),
      maxPerStageDescriptorAccelerationStructures(),
      maxPerStageDescriptorUpdateAfterBindAccelerationStructures(),
      maxDescriptorSetAccelerationStructures(),
      maxDescriptorSetUpdateAfterBindAccelerationStructures(),
      minAccelerationStructureScratchOffsetAlignment() {}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxPrimitiveCount = copy_src.maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src.maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src.maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src.minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxPrimitiveCount = copy_src.maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src.maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src.maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src.minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::~safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::initialize(
    const VkPhysicalDeviceAccelerationStructurePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxGeometryCount = in_struct->maxGeometryCount;
    maxInstanceCount = in_struct->maxInstanceCount;
    maxPrimitiveCount = in_struct->maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = in_struct->maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        in_struct->maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = in_struct->maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = in_struct->maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = in_struct->minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxGeometryCount = copy_src->maxGeometryCount;
    maxInstanceCount = copy_src->maxInstanceCount;
    maxPrimitiveCount = copy_src->maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src->maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src->maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src->maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src->maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src->minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR(
    const VkAccelerationStructureDeviceAddressInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), accelerationStructure(in_struct->accelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR), pNext(nullptr), accelerationStructure() {}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR(
    const safe_VkAccelerationStructureDeviceAddressInfoKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureDeviceAddressInfoKHR& safe_VkAccelerationStructureDeviceAddressInfoKHR::operator=(
    const safe_VkAccelerationStructureDeviceAddressInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::~safe_VkAccelerationStructureDeviceAddressInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureDeviceAddressInfoKHR::initialize(const VkAccelerationStructureDeviceAddressInfoKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureDeviceAddressInfoKHR::initialize(const safe_VkAccelerationStructureDeviceAddressInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR(
    const VkAccelerationStructureVersionInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pVersionData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)in_struct->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR), pNext(nullptr), pVersionData(nullptr) {}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR(
    const safe_VkAccelerationStructureVersionInfoKHR& copy_src) {
    sType = copy_src.sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src.pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkAccelerationStructureVersionInfoKHR& safe_VkAccelerationStructureVersionInfoKHR::operator=(
    const safe_VkAccelerationStructureVersionInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src.pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }

    return *this;
}

safe_VkAccelerationStructureVersionInfoKHR::~safe_VkAccelerationStructureVersionInfoKHR() {
    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureVersionInfoKHR::initialize(const VkAccelerationStructureVersionInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)in_struct->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

void safe_VkAccelerationStructureVersionInfoKHR::initialize(const safe_VkAccelerationStructureVersionInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR(
    const VkCopyAccelerationStructureToMemoryInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(&in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR), pNext(nullptr), src(), mode() {}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR(
    const safe_VkCopyAccelerationStructureToMemoryInfoKHR& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR& safe_VkCopyAccelerationStructureToMemoryInfoKHR::operator=(
    const safe_VkCopyAccelerationStructureToMemoryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::~safe_VkCopyAccelerationStructureToMemoryInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyAccelerationStructureToMemoryInfoKHR::initialize(const VkCopyAccelerationStructureToMemoryInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst.initialize(&in_struct->dst);
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyAccelerationStructureToMemoryInfoKHR::initialize(const safe_VkCopyAccelerationStructureToMemoryInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst.initialize(&copy_src->dst);
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR(
    const VkCopyMemoryToAccelerationStructureInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(&in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR), pNext(nullptr), dst(), mode() {}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR(
    const safe_VkCopyMemoryToAccelerationStructureInfoKHR& copy_src) {
    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR& safe_VkCopyMemoryToAccelerationStructureInfoKHR::operator=(
    const safe_VkCopyMemoryToAccelerationStructureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::~safe_VkCopyMemoryToAccelerationStructureInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyMemoryToAccelerationStructureInfoKHR::initialize(const VkCopyMemoryToAccelerationStructureInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src.initialize(&in_struct->src);
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyMemoryToAccelerationStructureInfoKHR::initialize(const safe_VkCopyMemoryToAccelerationStructureInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src.initialize(&copy_src->src);
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR(
    const VkCopyAccelerationStructureInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR), pNext(nullptr), src(), dst(), mode() {}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR(
    const safe_VkCopyAccelerationStructureInfoKHR& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyAccelerationStructureInfoKHR& safe_VkCopyAccelerationStructureInfoKHR::operator=(
    const safe_VkCopyAccelerationStructureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyAccelerationStructureInfoKHR::~safe_VkCopyAccelerationStructureInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyAccelerationStructureInfoKHR::initialize(const VkCopyAccelerationStructureInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyAccelerationStructureInfoKHR::initialize(const safe_VkCopyAccelerationStructureInfoKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR(
    const VkAccelerationStructureBuildSizesInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructureSize(in_struct->accelerationStructureSize),
      updateScratchSize(in_struct->updateScratchSize),
      buildScratchSize(in_struct->buildScratchSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR),
      pNext(nullptr),
      accelerationStructureSize(),
      updateScratchSize(),
      buildScratchSize() {}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR(
    const safe_VkAccelerationStructureBuildSizesInfoKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructureSize = copy_src.accelerationStructureSize;
    updateScratchSize = copy_src.updateScratchSize;
    buildScratchSize = copy_src.buildScratchSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureBuildSizesInfoKHR& safe_VkAccelerationStructureBuildSizesInfoKHR::operator=(
    const safe_VkAccelerationStructureBuildSizesInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureSize = copy_src.accelerationStructureSize;
    updateScratchSize = copy_src.updateScratchSize;
    buildScratchSize = copy_src.buildScratchSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureBuildSizesInfoKHR::~safe_VkAccelerationStructureBuildSizesInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureBuildSizesInfoKHR::initialize(const VkAccelerationStructureBuildSizesInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureSize = in_struct->accelerationStructureSize;
    updateScratchSize = in_struct->updateScratchSize;
    buildScratchSize = in_struct->buildScratchSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureBuildSizesInfoKHR::initialize(const safe_VkAccelerationStructureBuildSizesInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureSize = copy_src->accelerationStructureSize;
    updateScratchSize = copy_src->updateScratchSize;
    buildScratchSize = copy_src->buildScratchSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR(
    const VkRayTracingShaderGroupCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      generalShader(in_struct->generalShader),
      closestHitShader(in_struct->closestHitShader),
      anyHitShader(in_struct->anyHitShader),
      intersectionShader(in_struct->intersectionShader),
      pShaderGroupCaptureReplayHandle(in_struct->pShaderGroupCaptureReplayHandle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR),
      pNext(nullptr),
      type(),
      generalShader(),
      closestHitShader(),
      anyHitShader(),
      intersectionShader(),
      pShaderGroupCaptureReplayHandle(nullptr) {}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR(
    const safe_VkRayTracingShaderGroupCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src.pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingShaderGroupCreateInfoKHR& safe_VkRayTracingShaderGroupCreateInfoKHR::operator=(
    const safe_VkRayTracingShaderGroupCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src.pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingShaderGroupCreateInfoKHR::~safe_VkRayTracingShaderGroupCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkRayTracingShaderGroupCreateInfoKHR::initialize(const VkRayTracingShaderGroupCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    generalShader = in_struct->generalShader;
    closestHitShader = in_struct->closestHitShader;
    anyHitShader = in_struct->anyHitShader;
    intersectionShader = in_struct->intersectionShader;
    pShaderGroupCaptureReplayHandle = in_struct->pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingShaderGroupCreateInfoKHR::initialize(const safe_VkRayTracingShaderGroupCreateInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    generalShader = copy_src->generalShader;
    closestHitShader = copy_src->closestHitShader;
    anyHitShader = copy_src->anyHitShader;
    intersectionShader = copy_src->intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src->pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR(
    const VkRayTracingPipelineInterfaceCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxPipelineRayPayloadSize(in_struct->maxPipelineRayPayloadSize),
      maxPipelineRayHitAttributeSize(in_struct->maxPipelineRayHitAttributeSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR),
      pNext(nullptr),
      maxPipelineRayPayloadSize(),
      maxPipelineRayHitAttributeSize() {}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR(
    const safe_VkRayTracingPipelineInterfaceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxPipelineRayPayloadSize = copy_src.maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src.maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR& safe_VkRayTracingPipelineInterfaceCreateInfoKHR::operator=(
    const safe_VkRayTracingPipelineInterfaceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxPipelineRayPayloadSize = copy_src.maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src.maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::~safe_VkRayTracingPipelineInterfaceCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkRayTracingPipelineInterfaceCreateInfoKHR::initialize(const VkRayTracingPipelineInterfaceCreateInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxPipelineRayPayloadSize = in_struct->maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = in_struct->maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingPipelineInterfaceCreateInfoKHR::initialize(const safe_VkRayTracingPipelineInterfaceCreateInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxPipelineRayPayloadSize = copy_src->maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src->maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR(const VkRayTracingPipelineCreateInfoKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      groupCount(in_struct->groupCount),
      pGroups(nullptr),
      maxPipelineRayRecursionDepth(in_struct->maxPipelineRayRecursionDepth),
      pLibraryInfo(nullptr),
      pLibraryInterface(nullptr),
      pDynamicState(nullptr),
      layout(in_struct->layout),
      basePipelineHandle(in_struct->basePipelineHandle),
      basePipelineIndex(in_struct->basePipelineIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
    if (in_struct->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(in_struct->pLibraryInterface);
    if (in_struct->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      stageCount(),
      pStages(nullptr),
      groupCount(),
      pGroups(nullptr),
      maxPipelineRayRecursionDepth(),
      pLibraryInfo(nullptr),
      pLibraryInterface(nullptr),
      pDynamicState(nullptr),
      layout(),
      basePipelineHandle(),
      basePipelineIndex() {}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR(
    const safe_VkRayTracingPipelineCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src.maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);
    if (copy_src.pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src.pLibraryInterface);
    if (copy_src.pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);
}

safe_VkRayTracingPipelineCreateInfoKHR& safe_VkRayTracingPipelineCreateInfoKHR::operator=(
    const safe_VkRayTracingPipelineCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src.maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);
    if (copy_src.pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src.pLibraryInterface);
    if (copy_src.pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);

    return *this;
}

safe_VkRayTracingPipelineCreateInfoKHR::~safe_VkRayTracingPipelineCreateInfoKHR() {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
}

void safe_VkRayTracingPipelineCreateInfoKHR::initialize(const VkRayTracingPipelineCreateInfoKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    groupCount = in_struct->groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = in_struct->maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
    if (in_struct->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(in_struct->pLibraryInterface);
    if (in_struct->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
}

void safe_VkRayTracingPipelineCreateInfoKHR::initialize(const safe_VkRayTracingPipelineCreateInfoKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    groupCount = copy_src->groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src->maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src->layout;
    basePipelineHandle = copy_src->basePipelineHandle;
    basePipelineIndex = copy_src->basePipelineIndex;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (groupCount && copy_src->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src->pGroups[i]);
        }
    }
    if (copy_src->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src->pLibraryInfo);
    if (copy_src->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src->pLibraryInterface);
    if (copy_src->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src->pDynamicState);
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(
    const VkPhysicalDeviceRayTracingPipelineFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      rayTracingPipeline(in_struct->rayTracingPipeline),
      rayTracingPipelineShaderGroupHandleCaptureReplay(in_struct->rayTracingPipelineShaderGroupHandleCaptureReplay),
      rayTracingPipelineShaderGroupHandleCaptureReplayMixed(in_struct->rayTracingPipelineShaderGroupHandleCaptureReplayMixed),
      rayTracingPipelineTraceRaysIndirect(in_struct->rayTracingPipelineTraceRaysIndirect),
      rayTraversalPrimitiveCulling(in_struct->rayTraversalPrimitiveCulling) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR),
      pNext(nullptr),
      rayTracingPipeline(),
      rayTracingPipelineShaderGroupHandleCaptureReplay(),
      rayTracingPipelineShaderGroupHandleCaptureReplayMixed(),
      rayTracingPipelineTraceRaysIndirect(),
      rayTraversalPrimitiveCulling() {}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingPipeline = copy_src.rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src.rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src.rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingPipeline = copy_src.rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src.rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src.rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::~safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::initialize(const VkPhysicalDeviceRayTracingPipelineFeaturesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingPipeline = in_struct->rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = in_struct->rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = in_struct->rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = in_struct->rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = in_struct->rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingPipeline = copy_src->rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src->rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src->rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src->rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src->rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderGroupHandleSize(in_struct->shaderGroupHandleSize),
      maxRayRecursionDepth(in_struct->maxRayRecursionDepth),
      maxShaderGroupStride(in_struct->maxShaderGroupStride),
      shaderGroupBaseAlignment(in_struct->shaderGroupBaseAlignment),
      shaderGroupHandleCaptureReplaySize(in_struct->shaderGroupHandleCaptureReplaySize),
      maxRayDispatchInvocationCount(in_struct->maxRayDispatchInvocationCount),
      shaderGroupHandleAlignment(in_struct->shaderGroupHandleAlignment),
      maxRayHitAttributeSize(in_struct->maxRayHitAttributeSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR),
      pNext(nullptr),
      shaderGroupHandleSize(),
      maxRayRecursionDepth(),
      maxShaderGroupStride(),
      shaderGroupBaseAlignment(),
      shaderGroupHandleCaptureReplaySize(),
      maxRayDispatchInvocationCount(),
      shaderGroupHandleAlignment(),
      maxRayHitAttributeSize() {}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src.maxRayRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src.shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src.maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src.shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src.maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src.maxRayRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src.shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src.maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src.shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src.maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::~safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::initialize(
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderGroupHandleSize = in_struct->shaderGroupHandleSize;
    maxRayRecursionDepth = in_struct->maxRayRecursionDepth;
    maxShaderGroupStride = in_struct->maxShaderGroupStride;
    shaderGroupBaseAlignment = in_struct->shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = in_struct->shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = in_struct->maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = in_struct->shaderGroupHandleAlignment;
    maxRayHitAttributeSize = in_struct->maxRayHitAttributeSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderGroupHandleSize = copy_src->shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src->maxRayRecursionDepth;
    maxShaderGroupStride = copy_src->maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src->shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src->shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src->maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src->shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src->maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR(
    const VkPhysicalDeviceRayQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), rayQuery(in_struct->rayQuery) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR), pNext(nullptr), rayQuery() {}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR(
    const safe_VkPhysicalDeviceRayQueryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayQuery = copy_src.rayQuery;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR& safe_VkPhysicalDeviceRayQueryFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayQueryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayQuery = copy_src.rayQuery;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::~safe_VkPhysicalDeviceRayQueryFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRayQueryFeaturesKHR::initialize(const VkPhysicalDeviceRayQueryFeaturesKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayQuery = in_struct->rayQuery;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayQueryFeaturesKHR::initialize(const safe_VkPhysicalDeviceRayQueryFeaturesKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayQuery = copy_src->rayQuery;
    pNext = SafePnextCopy(copy_src->pNext);
}

// NOLINTEND
