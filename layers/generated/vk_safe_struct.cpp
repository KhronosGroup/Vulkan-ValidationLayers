// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/


#include "vk_safe_struct.h"
#include <string.h>


safe_VkApplicationInfo::safe_VkApplicationInfo(const VkApplicationInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pApplicationName(in_struct->pApplicationName),
    applicationVersion(in_struct->applicationVersion),
    pEngineName(in_struct->pEngineName),
    engineVersion(in_struct->engineVersion),
    apiVersion(in_struct->apiVersion)
{
}

safe_VkApplicationInfo::safe_VkApplicationInfo()
{}

safe_VkApplicationInfo::safe_VkApplicationInfo(const safe_VkApplicationInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pApplicationName = src.pApplicationName;
    applicationVersion = src.applicationVersion;
    pEngineName = src.pEngineName;
    engineVersion = src.engineVersion;
    apiVersion = src.apiVersion;
}

safe_VkApplicationInfo& safe_VkApplicationInfo::operator=(const safe_VkApplicationInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pApplicationName = src.pApplicationName;
    applicationVersion = src.applicationVersion;
    pEngineName = src.pEngineName;
    engineVersion = src.engineVersion;
    apiVersion = src.apiVersion;

    return *this;
}

safe_VkApplicationInfo::~safe_VkApplicationInfo()
{
}

void safe_VkApplicationInfo::initialize(const VkApplicationInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pApplicationName = in_struct->pApplicationName;
    applicationVersion = in_struct->applicationVersion;
    pEngineName = in_struct->pEngineName;
    engineVersion = in_struct->engineVersion;
    apiVersion = in_struct->apiVersion;
}

void safe_VkApplicationInfo::initialize(const safe_VkApplicationInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pApplicationName = src->pApplicationName;
    applicationVersion = src->applicationVersion;
    pEngineName = src->pEngineName;
    engineVersion = src->engineVersion;
    apiVersion = src->apiVersion;
}

safe_VkInstanceCreateInfo::safe_VkInstanceCreateInfo(const VkInstanceCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    enabledLayerCount(in_struct->enabledLayerCount),
    ppEnabledLayerNames(in_struct->ppEnabledLayerNames),
    enabledExtensionCount(in_struct->enabledExtensionCount),
    ppEnabledExtensionNames(in_struct->ppEnabledExtensionNames)
{
    if (in_struct->pApplicationInfo)
        pApplicationInfo = new safe_VkApplicationInfo(in_struct->pApplicationInfo);
    else
        pApplicationInfo = NULL;
}

safe_VkInstanceCreateInfo::safe_VkInstanceCreateInfo()
{}

safe_VkInstanceCreateInfo::safe_VkInstanceCreateInfo(const safe_VkInstanceCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    enabledLayerCount = src.enabledLayerCount;
    ppEnabledLayerNames = src.ppEnabledLayerNames;
    enabledExtensionCount = src.enabledExtensionCount;
    ppEnabledExtensionNames = src.ppEnabledExtensionNames;
    if (src.pApplicationInfo)
        pApplicationInfo = new safe_VkApplicationInfo(*src.pApplicationInfo);
    else
        pApplicationInfo = NULL;
}

safe_VkInstanceCreateInfo& safe_VkInstanceCreateInfo::operator=(const safe_VkInstanceCreateInfo& src)
{
    if (&src == this) return *this;

    if (pApplicationInfo)
        delete pApplicationInfo;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    enabledLayerCount = src.enabledLayerCount;
    ppEnabledLayerNames = src.ppEnabledLayerNames;
    enabledExtensionCount = src.enabledExtensionCount;
    ppEnabledExtensionNames = src.ppEnabledExtensionNames;
    if (src.pApplicationInfo)
        pApplicationInfo = new safe_VkApplicationInfo(*src.pApplicationInfo);
    else
        pApplicationInfo = NULL;

    return *this;
}

safe_VkInstanceCreateInfo::~safe_VkInstanceCreateInfo()
{
    if (pApplicationInfo)
        delete pApplicationInfo;
}

void safe_VkInstanceCreateInfo::initialize(const VkInstanceCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    enabledLayerCount = in_struct->enabledLayerCount;
    ppEnabledLayerNames = in_struct->ppEnabledLayerNames;
    enabledExtensionCount = in_struct->enabledExtensionCount;
    ppEnabledExtensionNames = in_struct->ppEnabledExtensionNames;
    if (in_struct->pApplicationInfo)
        pApplicationInfo = new safe_VkApplicationInfo(in_struct->pApplicationInfo);
    else
        pApplicationInfo = NULL;
}

void safe_VkInstanceCreateInfo::initialize(const safe_VkInstanceCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    enabledLayerCount = src->enabledLayerCount;
    ppEnabledLayerNames = src->ppEnabledLayerNames;
    enabledExtensionCount = src->enabledExtensionCount;
    ppEnabledExtensionNames = src->ppEnabledExtensionNames;
    if (src->pApplicationInfo)
        pApplicationInfo = new safe_VkApplicationInfo(*src->pApplicationInfo);
    else
        pApplicationInfo = NULL;
}

safe_VkAllocationCallbacks::safe_VkAllocationCallbacks(const VkAllocationCallbacks* in_struct) :
    pUserData(in_struct->pUserData),
    pfnAllocation(in_struct->pfnAllocation),
    pfnReallocation(in_struct->pfnReallocation),
    pfnFree(in_struct->pfnFree),
    pfnInternalAllocation(in_struct->pfnInternalAllocation),
    pfnInternalFree(in_struct->pfnInternalFree)
{
}

safe_VkAllocationCallbacks::safe_VkAllocationCallbacks()
{}

safe_VkAllocationCallbacks::safe_VkAllocationCallbacks(const safe_VkAllocationCallbacks& src)
{
    pUserData = src.pUserData;
    pfnAllocation = src.pfnAllocation;
    pfnReallocation = src.pfnReallocation;
    pfnFree = src.pfnFree;
    pfnInternalAllocation = src.pfnInternalAllocation;
    pfnInternalFree = src.pfnInternalFree;
}

safe_VkAllocationCallbacks& safe_VkAllocationCallbacks::operator=(const safe_VkAllocationCallbacks& src)
{
    if (&src == this) return *this;


    pUserData = src.pUserData;
    pfnAllocation = src.pfnAllocation;
    pfnReallocation = src.pfnReallocation;
    pfnFree = src.pfnFree;
    pfnInternalAllocation = src.pfnInternalAllocation;
    pfnInternalFree = src.pfnInternalFree;

    return *this;
}

safe_VkAllocationCallbacks::~safe_VkAllocationCallbacks()
{
}

void safe_VkAllocationCallbacks::initialize(const VkAllocationCallbacks* in_struct)
{
    pUserData = in_struct->pUserData;
    pfnAllocation = in_struct->pfnAllocation;
    pfnReallocation = in_struct->pfnReallocation;
    pfnFree = in_struct->pfnFree;
    pfnInternalAllocation = in_struct->pfnInternalAllocation;
    pfnInternalFree = in_struct->pfnInternalFree;
}

void safe_VkAllocationCallbacks::initialize(const safe_VkAllocationCallbacks* src)
{
    pUserData = src->pUserData;
    pfnAllocation = src->pfnAllocation;
    pfnReallocation = src->pfnReallocation;
    pfnFree = src->pfnFree;
    pfnInternalAllocation = src->pfnInternalAllocation;
    pfnInternalFree = src->pfnInternalFree;
}

safe_VkDeviceQueueCreateInfo::safe_VkDeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    queueFamilyIndex(in_struct->queueFamilyIndex),
    queueCount(in_struct->queueCount),
    pQueuePriorities(nullptr)
{
    if (in_struct->pQueuePriorities) {
        pQueuePriorities = new float[in_struct->queueCount];
        memcpy ((void *)pQueuePriorities, (void *)in_struct->pQueuePriorities, sizeof(float)*in_struct->queueCount);
    }
}

safe_VkDeviceQueueCreateInfo::safe_VkDeviceQueueCreateInfo() :
    pQueuePriorities(nullptr)
{}

safe_VkDeviceQueueCreateInfo::safe_VkDeviceQueueCreateInfo(const safe_VkDeviceQueueCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;
    queueCount = src.queueCount;
    pQueuePriorities = nullptr;
    if (src.pQueuePriorities) {
        pQueuePriorities = new float[src.queueCount];
        memcpy ((void *)pQueuePriorities, (void *)src.pQueuePriorities, sizeof(float)*src.queueCount);
    }
}

safe_VkDeviceQueueCreateInfo& safe_VkDeviceQueueCreateInfo::operator=(const safe_VkDeviceQueueCreateInfo& src)
{
    if (&src == this) return *this;

    if (pQueuePriorities)
        delete[] pQueuePriorities;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;
    queueCount = src.queueCount;
    pQueuePriorities = nullptr;
    if (src.pQueuePriorities) {
        pQueuePriorities = new float[src.queueCount];
        memcpy ((void *)pQueuePriorities, (void *)src.pQueuePriorities, sizeof(float)*src.queueCount);
    }

    return *this;
}

safe_VkDeviceQueueCreateInfo::~safe_VkDeviceQueueCreateInfo()
{
    if (pQueuePriorities)
        delete[] pQueuePriorities;
}

void safe_VkDeviceQueueCreateInfo::initialize(const VkDeviceQueueCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    queueCount = in_struct->queueCount;
    pQueuePriorities = nullptr;
    if (in_struct->pQueuePriorities) {
        pQueuePriorities = new float[in_struct->queueCount];
        memcpy ((void *)pQueuePriorities, (void *)in_struct->pQueuePriorities, sizeof(float)*in_struct->queueCount);
    }
}

void safe_VkDeviceQueueCreateInfo::initialize(const safe_VkDeviceQueueCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    queueFamilyIndex = src->queueFamilyIndex;
    queueCount = src->queueCount;
    pQueuePriorities = nullptr;
    if (src->pQueuePriorities) {
        pQueuePriorities = new float[src->queueCount];
        memcpy ((void *)pQueuePriorities, (void *)src->pQueuePriorities, sizeof(float)*src->queueCount);
    }
}

safe_VkDeviceCreateInfo::safe_VkDeviceCreateInfo(const VkDeviceCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    queueCreateInfoCount(in_struct->queueCreateInfoCount),
    pQueueCreateInfos(nullptr),
    enabledLayerCount(in_struct->enabledLayerCount),
    ppEnabledLayerNames(in_struct->ppEnabledLayerNames),
    enabledExtensionCount(in_struct->enabledExtensionCount),
    ppEnabledExtensionNames(in_struct->ppEnabledExtensionNames),
    pEnabledFeatures(nullptr)
{
    if (queueCreateInfoCount && in_struct->pQueueCreateInfos) {
        pQueueCreateInfos = new safe_VkDeviceQueueCreateInfo[queueCreateInfoCount];
        for (uint32_t i=0; i<queueCreateInfoCount; ++i) {
            pQueueCreateInfos[i].initialize(&in_struct->pQueueCreateInfos[i]);
        }
    }
    if (in_struct->pEnabledFeatures) {
        pEnabledFeatures = new VkPhysicalDeviceFeatures(*in_struct->pEnabledFeatures);
    }
}

safe_VkDeviceCreateInfo::safe_VkDeviceCreateInfo() :
    pQueueCreateInfos(nullptr),
    pEnabledFeatures(nullptr)
{}

safe_VkDeviceCreateInfo::safe_VkDeviceCreateInfo(const safe_VkDeviceCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueCreateInfoCount = src.queueCreateInfoCount;
    pQueueCreateInfos = nullptr;
    enabledLayerCount = src.enabledLayerCount;
    ppEnabledLayerNames = src.ppEnabledLayerNames;
    enabledExtensionCount = src.enabledExtensionCount;
    ppEnabledExtensionNames = src.ppEnabledExtensionNames;
    pEnabledFeatures = nullptr;
    if (queueCreateInfoCount && src.pQueueCreateInfos) {
        pQueueCreateInfos = new safe_VkDeviceQueueCreateInfo[queueCreateInfoCount];
        for (uint32_t i=0; i<queueCreateInfoCount; ++i) {
            pQueueCreateInfos[i].initialize(&src.pQueueCreateInfos[i]);
        }
    }
    if (src.pEnabledFeatures) {
        pEnabledFeatures = new VkPhysicalDeviceFeatures(*src.pEnabledFeatures);
    }
}

safe_VkDeviceCreateInfo& safe_VkDeviceCreateInfo::operator=(const safe_VkDeviceCreateInfo& src)
{
    if (&src == this) return *this;

    if (pQueueCreateInfos)
        delete[] pQueueCreateInfos;
    if (pEnabledFeatures)
        delete pEnabledFeatures;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueCreateInfoCount = src.queueCreateInfoCount;
    pQueueCreateInfos = nullptr;
    enabledLayerCount = src.enabledLayerCount;
    ppEnabledLayerNames = src.ppEnabledLayerNames;
    enabledExtensionCount = src.enabledExtensionCount;
    ppEnabledExtensionNames = src.ppEnabledExtensionNames;
    pEnabledFeatures = nullptr;
    if (queueCreateInfoCount && src.pQueueCreateInfos) {
        pQueueCreateInfos = new safe_VkDeviceQueueCreateInfo[queueCreateInfoCount];
        for (uint32_t i=0; i<queueCreateInfoCount; ++i) {
            pQueueCreateInfos[i].initialize(&src.pQueueCreateInfos[i]);
        }
    }
    if (src.pEnabledFeatures) {
        pEnabledFeatures = new VkPhysicalDeviceFeatures(*src.pEnabledFeatures);
    }

    return *this;
}

safe_VkDeviceCreateInfo::~safe_VkDeviceCreateInfo()
{
    if (pQueueCreateInfos)
        delete[] pQueueCreateInfos;
    if (pEnabledFeatures)
        delete pEnabledFeatures;
}

void safe_VkDeviceCreateInfo::initialize(const VkDeviceCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    queueCreateInfoCount = in_struct->queueCreateInfoCount;
    pQueueCreateInfos = nullptr;
    enabledLayerCount = in_struct->enabledLayerCount;
    ppEnabledLayerNames = in_struct->ppEnabledLayerNames;
    enabledExtensionCount = in_struct->enabledExtensionCount;
    ppEnabledExtensionNames = in_struct->ppEnabledExtensionNames;
    pEnabledFeatures = nullptr;
    if (queueCreateInfoCount && in_struct->pQueueCreateInfos) {
        pQueueCreateInfos = new safe_VkDeviceQueueCreateInfo[queueCreateInfoCount];
        for (uint32_t i=0; i<queueCreateInfoCount; ++i) {
            pQueueCreateInfos[i].initialize(&in_struct->pQueueCreateInfos[i]);
        }
    }
    if (in_struct->pEnabledFeatures) {
        pEnabledFeatures = new VkPhysicalDeviceFeatures(*in_struct->pEnabledFeatures);
    }
}

void safe_VkDeviceCreateInfo::initialize(const safe_VkDeviceCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    queueCreateInfoCount = src->queueCreateInfoCount;
    pQueueCreateInfos = nullptr;
    enabledLayerCount = src->enabledLayerCount;
    ppEnabledLayerNames = src->ppEnabledLayerNames;
    enabledExtensionCount = src->enabledExtensionCount;
    ppEnabledExtensionNames = src->ppEnabledExtensionNames;
    pEnabledFeatures = nullptr;
    if (queueCreateInfoCount && src->pQueueCreateInfos) {
        pQueueCreateInfos = new safe_VkDeviceQueueCreateInfo[queueCreateInfoCount];
        for (uint32_t i=0; i<queueCreateInfoCount; ++i) {
            pQueueCreateInfos[i].initialize(&src->pQueueCreateInfos[i]);
        }
    }
    if (src->pEnabledFeatures) {
        pEnabledFeatures = new VkPhysicalDeviceFeatures(*src->pEnabledFeatures);
    }
}

safe_VkSubmitInfo::safe_VkSubmitInfo(const VkSubmitInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    waitSemaphoreCount(in_struct->waitSemaphoreCount),
    pWaitSemaphores(nullptr),
    pWaitDstStageMask(nullptr),
    commandBufferCount(in_struct->commandBufferCount),
    pCommandBuffers(nullptr),
    signalSemaphoreCount(in_struct->signalSemaphoreCount),
    pSignalSemaphores(nullptr)
{
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (in_struct->pWaitDstStageMask) {
        pWaitDstStageMask = new VkPipelineStageFlags[in_struct->waitSemaphoreCount];
        memcpy ((void *)pWaitDstStageMask, (void *)in_struct->pWaitDstStageMask, sizeof(VkPipelineStageFlags)*in_struct->waitSemaphoreCount);
    }
    if (in_struct->pCommandBuffers) {
        pCommandBuffers = new VkCommandBuffer[in_struct->commandBufferCount];
        memcpy ((void *)pCommandBuffers, (void *)in_struct->pCommandBuffers, sizeof(VkCommandBuffer)*in_struct->commandBufferCount);
    }
    if (signalSemaphoreCount && in_struct->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = in_struct->pSignalSemaphores[i];
        }
    }
}

safe_VkSubmitInfo::safe_VkSubmitInfo() :
    pWaitSemaphores(nullptr),
    pWaitDstStageMask(nullptr),
    pCommandBuffers(nullptr),
    pSignalSemaphores(nullptr)
{}

safe_VkSubmitInfo::safe_VkSubmitInfo(const safe_VkSubmitInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    pWaitDstStageMask = nullptr;
    commandBufferCount = src.commandBufferCount;
    pCommandBuffers = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (src.pWaitDstStageMask) {
        pWaitDstStageMask = new VkPipelineStageFlags[src.waitSemaphoreCount];
        memcpy ((void *)pWaitDstStageMask, (void *)src.pWaitDstStageMask, sizeof(VkPipelineStageFlags)*src.waitSemaphoreCount);
    }
    if (src.pCommandBuffers) {
        pCommandBuffers = new VkCommandBuffer[src.commandBufferCount];
        memcpy ((void *)pCommandBuffers, (void *)src.pCommandBuffers, sizeof(VkCommandBuffer)*src.commandBufferCount);
    }
    if (signalSemaphoreCount && src.pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src.pSignalSemaphores[i];
        }
    }
}

safe_VkSubmitInfo& safe_VkSubmitInfo::operator=(const safe_VkSubmitInfo& src)
{
    if (&src == this) return *this;

    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pWaitDstStageMask)
        delete[] pWaitDstStageMask;
    if (pCommandBuffers)
        delete[] pCommandBuffers;
    if (pSignalSemaphores)
        delete[] pSignalSemaphores;

    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    pWaitDstStageMask = nullptr;
    commandBufferCount = src.commandBufferCount;
    pCommandBuffers = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (src.pWaitDstStageMask) {
        pWaitDstStageMask = new VkPipelineStageFlags[src.waitSemaphoreCount];
        memcpy ((void *)pWaitDstStageMask, (void *)src.pWaitDstStageMask, sizeof(VkPipelineStageFlags)*src.waitSemaphoreCount);
    }
    if (src.pCommandBuffers) {
        pCommandBuffers = new VkCommandBuffer[src.commandBufferCount];
        memcpy ((void *)pCommandBuffers, (void *)src.pCommandBuffers, sizeof(VkCommandBuffer)*src.commandBufferCount);
    }
    if (signalSemaphoreCount && src.pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src.pSignalSemaphores[i];
        }
    }

    return *this;
}

safe_VkSubmitInfo::~safe_VkSubmitInfo()
{
    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pWaitDstStageMask)
        delete[] pWaitDstStageMask;
    if (pCommandBuffers)
        delete[] pCommandBuffers;
    if (pSignalSemaphores)
        delete[] pSignalSemaphores;
}

void safe_VkSubmitInfo::initialize(const VkSubmitInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    pWaitDstStageMask = nullptr;
    commandBufferCount = in_struct->commandBufferCount;
    pCommandBuffers = nullptr;
    signalSemaphoreCount = in_struct->signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (in_struct->pWaitDstStageMask) {
        pWaitDstStageMask = new VkPipelineStageFlags[in_struct->waitSemaphoreCount];
        memcpy ((void *)pWaitDstStageMask, (void *)in_struct->pWaitDstStageMask, sizeof(VkPipelineStageFlags)*in_struct->waitSemaphoreCount);
    }
    if (in_struct->pCommandBuffers) {
        pCommandBuffers = new VkCommandBuffer[in_struct->commandBufferCount];
        memcpy ((void *)pCommandBuffers, (void *)in_struct->pCommandBuffers, sizeof(VkCommandBuffer)*in_struct->commandBufferCount);
    }
    if (signalSemaphoreCount && in_struct->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = in_struct->pSignalSemaphores[i];
        }
    }
}

void safe_VkSubmitInfo::initialize(const safe_VkSubmitInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    waitSemaphoreCount = src->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    pWaitDstStageMask = nullptr;
    commandBufferCount = src->commandBufferCount;
    pCommandBuffers = nullptr;
    signalSemaphoreCount = src->signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src->pWaitSemaphores[i];
        }
    }
    if (src->pWaitDstStageMask) {
        pWaitDstStageMask = new VkPipelineStageFlags[src->waitSemaphoreCount];
        memcpy ((void *)pWaitDstStageMask, (void *)src->pWaitDstStageMask, sizeof(VkPipelineStageFlags)*src->waitSemaphoreCount);
    }
    if (src->pCommandBuffers) {
        pCommandBuffers = new VkCommandBuffer[src->commandBufferCount];
        memcpy ((void *)pCommandBuffers, (void *)src->pCommandBuffers, sizeof(VkCommandBuffer)*src->commandBufferCount);
    }
    if (signalSemaphoreCount && src->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src->pSignalSemaphores[i];
        }
    }
}

safe_VkMemoryAllocateInfo::safe_VkMemoryAllocateInfo(const VkMemoryAllocateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    allocationSize(in_struct->allocationSize),
    memoryTypeIndex(in_struct->memoryTypeIndex)
{
}

safe_VkMemoryAllocateInfo::safe_VkMemoryAllocateInfo()
{}

safe_VkMemoryAllocateInfo::safe_VkMemoryAllocateInfo(const safe_VkMemoryAllocateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    allocationSize = src.allocationSize;
    memoryTypeIndex = src.memoryTypeIndex;
}

safe_VkMemoryAllocateInfo& safe_VkMemoryAllocateInfo::operator=(const safe_VkMemoryAllocateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    allocationSize = src.allocationSize;
    memoryTypeIndex = src.memoryTypeIndex;

    return *this;
}

safe_VkMemoryAllocateInfo::~safe_VkMemoryAllocateInfo()
{
}

void safe_VkMemoryAllocateInfo::initialize(const VkMemoryAllocateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    allocationSize = in_struct->allocationSize;
    memoryTypeIndex = in_struct->memoryTypeIndex;
}

void safe_VkMemoryAllocateInfo::initialize(const safe_VkMemoryAllocateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    allocationSize = src->allocationSize;
    memoryTypeIndex = src->memoryTypeIndex;
}

safe_VkMappedMemoryRange::safe_VkMappedMemoryRange(const VkMappedMemoryRange* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memory(in_struct->memory),
    offset(in_struct->offset),
    size(in_struct->size)
{
}

safe_VkMappedMemoryRange::safe_VkMappedMemoryRange()
{}

safe_VkMappedMemoryRange::safe_VkMappedMemoryRange(const safe_VkMappedMemoryRange& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    offset = src.offset;
    size = src.size;
}

safe_VkMappedMemoryRange& safe_VkMappedMemoryRange::operator=(const safe_VkMappedMemoryRange& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    offset = src.offset;
    size = src.size;

    return *this;
}

safe_VkMappedMemoryRange::~safe_VkMappedMemoryRange()
{
}

void safe_VkMappedMemoryRange::initialize(const VkMappedMemoryRange* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memory = in_struct->memory;
    offset = in_struct->offset;
    size = in_struct->size;
}

void safe_VkMappedMemoryRange::initialize(const safe_VkMappedMemoryRange* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memory = src->memory;
    offset = src->offset;
    size = src->size;
}

safe_VkSparseBufferMemoryBindInfo::safe_VkSparseBufferMemoryBindInfo(const VkSparseBufferMemoryBindInfo* in_struct) :
    buffer(in_struct->buffer),
    bindCount(in_struct->bindCount),
    pBinds(nullptr)
{
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

safe_VkSparseBufferMemoryBindInfo::safe_VkSparseBufferMemoryBindInfo() :
    pBinds(nullptr)
{}

safe_VkSparseBufferMemoryBindInfo::safe_VkSparseBufferMemoryBindInfo(const safe_VkSparseBufferMemoryBindInfo& src)
{
    buffer = src.buffer;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }
}

safe_VkSparseBufferMemoryBindInfo& safe_VkSparseBufferMemoryBindInfo::operator=(const safe_VkSparseBufferMemoryBindInfo& src)
{
    if (&src == this) return *this;

    if (pBinds)
        delete[] pBinds;

    buffer = src.buffer;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }

    return *this;
}

safe_VkSparseBufferMemoryBindInfo::~safe_VkSparseBufferMemoryBindInfo()
{
    if (pBinds)
        delete[] pBinds;
}

void safe_VkSparseBufferMemoryBindInfo::initialize(const VkSparseBufferMemoryBindInfo* in_struct)
{
    buffer = in_struct->buffer;
    bindCount = in_struct->bindCount;
    pBinds = nullptr;
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

void safe_VkSparseBufferMemoryBindInfo::initialize(const safe_VkSparseBufferMemoryBindInfo* src)
{
    buffer = src->buffer;
    bindCount = src->bindCount;
    pBinds = nullptr;
    if (bindCount && src->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src->pBinds[i];
        }
    }
}

safe_VkSparseImageOpaqueMemoryBindInfo::safe_VkSparseImageOpaqueMemoryBindInfo(const VkSparseImageOpaqueMemoryBindInfo* in_struct) :
    image(in_struct->image),
    bindCount(in_struct->bindCount),
    pBinds(nullptr)
{
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

safe_VkSparseImageOpaqueMemoryBindInfo::safe_VkSparseImageOpaqueMemoryBindInfo() :
    pBinds(nullptr)
{}

safe_VkSparseImageOpaqueMemoryBindInfo::safe_VkSparseImageOpaqueMemoryBindInfo(const safe_VkSparseImageOpaqueMemoryBindInfo& src)
{
    image = src.image;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }
}

safe_VkSparseImageOpaqueMemoryBindInfo& safe_VkSparseImageOpaqueMemoryBindInfo::operator=(const safe_VkSparseImageOpaqueMemoryBindInfo& src)
{
    if (&src == this) return *this;

    if (pBinds)
        delete[] pBinds;

    image = src.image;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }

    return *this;
}

safe_VkSparseImageOpaqueMemoryBindInfo::~safe_VkSparseImageOpaqueMemoryBindInfo()
{
    if (pBinds)
        delete[] pBinds;
}

void safe_VkSparseImageOpaqueMemoryBindInfo::initialize(const VkSparseImageOpaqueMemoryBindInfo* in_struct)
{
    image = in_struct->image;
    bindCount = in_struct->bindCount;
    pBinds = nullptr;
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

void safe_VkSparseImageOpaqueMemoryBindInfo::initialize(const safe_VkSparseImageOpaqueMemoryBindInfo* src)
{
    image = src->image;
    bindCount = src->bindCount;
    pBinds = nullptr;
    if (bindCount && src->pBinds) {
        pBinds = new VkSparseMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src->pBinds[i];
        }
    }
}

safe_VkSparseImageMemoryBindInfo::safe_VkSparseImageMemoryBindInfo(const VkSparseImageMemoryBindInfo* in_struct) :
    image(in_struct->image),
    bindCount(in_struct->bindCount),
    pBinds(nullptr)
{
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseImageMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

safe_VkSparseImageMemoryBindInfo::safe_VkSparseImageMemoryBindInfo() :
    pBinds(nullptr)
{}

safe_VkSparseImageMemoryBindInfo::safe_VkSparseImageMemoryBindInfo(const safe_VkSparseImageMemoryBindInfo& src)
{
    image = src.image;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseImageMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }
}

safe_VkSparseImageMemoryBindInfo& safe_VkSparseImageMemoryBindInfo::operator=(const safe_VkSparseImageMemoryBindInfo& src)
{
    if (&src == this) return *this;

    if (pBinds)
        delete[] pBinds;

    image = src.image;
    bindCount = src.bindCount;
    pBinds = nullptr;
    if (bindCount && src.pBinds) {
        pBinds = new VkSparseImageMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src.pBinds[i];
        }
    }

    return *this;
}

safe_VkSparseImageMemoryBindInfo::~safe_VkSparseImageMemoryBindInfo()
{
    if (pBinds)
        delete[] pBinds;
}

void safe_VkSparseImageMemoryBindInfo::initialize(const VkSparseImageMemoryBindInfo* in_struct)
{
    image = in_struct->image;
    bindCount = in_struct->bindCount;
    pBinds = nullptr;
    if (bindCount && in_struct->pBinds) {
        pBinds = new VkSparseImageMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = in_struct->pBinds[i];
        }
    }
}

void safe_VkSparseImageMemoryBindInfo::initialize(const safe_VkSparseImageMemoryBindInfo* src)
{
    image = src->image;
    bindCount = src->bindCount;
    pBinds = nullptr;
    if (bindCount && src->pBinds) {
        pBinds = new VkSparseImageMemoryBind[bindCount];
        for (uint32_t i=0; i<bindCount; ++i) {
            pBinds[i] = src->pBinds[i];
        }
    }
}

safe_VkBindSparseInfo::safe_VkBindSparseInfo(const VkBindSparseInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    waitSemaphoreCount(in_struct->waitSemaphoreCount),
    pWaitSemaphores(nullptr),
    bufferBindCount(in_struct->bufferBindCount),
    pBufferBinds(nullptr),
    imageOpaqueBindCount(in_struct->imageOpaqueBindCount),
    pImageOpaqueBinds(nullptr),
    imageBindCount(in_struct->imageBindCount),
    pImageBinds(nullptr),
    signalSemaphoreCount(in_struct->signalSemaphoreCount),
    pSignalSemaphores(nullptr)
{
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (bufferBindCount && in_struct->pBufferBinds) {
        pBufferBinds = new safe_VkSparseBufferMemoryBindInfo[bufferBindCount];
        for (uint32_t i=0; i<bufferBindCount; ++i) {
            pBufferBinds[i].initialize(&in_struct->pBufferBinds[i]);
        }
    }
    if (imageOpaqueBindCount && in_struct->pImageOpaqueBinds) {
        pImageOpaqueBinds = new safe_VkSparseImageOpaqueMemoryBindInfo[imageOpaqueBindCount];
        for (uint32_t i=0; i<imageOpaqueBindCount; ++i) {
            pImageOpaqueBinds[i].initialize(&in_struct->pImageOpaqueBinds[i]);
        }
    }
    if (imageBindCount && in_struct->pImageBinds) {
        pImageBinds = new safe_VkSparseImageMemoryBindInfo[imageBindCount];
        for (uint32_t i=0; i<imageBindCount; ++i) {
            pImageBinds[i].initialize(&in_struct->pImageBinds[i]);
        }
    }
    if (signalSemaphoreCount && in_struct->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = in_struct->pSignalSemaphores[i];
        }
    }
}

safe_VkBindSparseInfo::safe_VkBindSparseInfo() :
    pWaitSemaphores(nullptr),
    pBufferBinds(nullptr),
    pImageOpaqueBinds(nullptr),
    pImageBinds(nullptr),
    pSignalSemaphores(nullptr)
{}

safe_VkBindSparseInfo::safe_VkBindSparseInfo(const safe_VkBindSparseInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    bufferBindCount = src.bufferBindCount;
    pBufferBinds = nullptr;
    imageOpaqueBindCount = src.imageOpaqueBindCount;
    pImageOpaqueBinds = nullptr;
    imageBindCount = src.imageBindCount;
    pImageBinds = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (bufferBindCount && src.pBufferBinds) {
        pBufferBinds = new safe_VkSparseBufferMemoryBindInfo[bufferBindCount];
        for (uint32_t i=0; i<bufferBindCount; ++i) {
            pBufferBinds[i].initialize(&src.pBufferBinds[i]);
        }
    }
    if (imageOpaqueBindCount && src.pImageOpaqueBinds) {
        pImageOpaqueBinds = new safe_VkSparseImageOpaqueMemoryBindInfo[imageOpaqueBindCount];
        for (uint32_t i=0; i<imageOpaqueBindCount; ++i) {
            pImageOpaqueBinds[i].initialize(&src.pImageOpaqueBinds[i]);
        }
    }
    if (imageBindCount && src.pImageBinds) {
        pImageBinds = new safe_VkSparseImageMemoryBindInfo[imageBindCount];
        for (uint32_t i=0; i<imageBindCount; ++i) {
            pImageBinds[i].initialize(&src.pImageBinds[i]);
        }
    }
    if (signalSemaphoreCount && src.pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src.pSignalSemaphores[i];
        }
    }
}

safe_VkBindSparseInfo& safe_VkBindSparseInfo::operator=(const safe_VkBindSparseInfo& src)
{
    if (&src == this) return *this;

    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pBufferBinds)
        delete[] pBufferBinds;
    if (pImageOpaqueBinds)
        delete[] pImageOpaqueBinds;
    if (pImageBinds)
        delete[] pImageBinds;
    if (pSignalSemaphores)
        delete[] pSignalSemaphores;

    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    bufferBindCount = src.bufferBindCount;
    pBufferBinds = nullptr;
    imageOpaqueBindCount = src.imageOpaqueBindCount;
    pImageOpaqueBinds = nullptr;
    imageBindCount = src.imageBindCount;
    pImageBinds = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (bufferBindCount && src.pBufferBinds) {
        pBufferBinds = new safe_VkSparseBufferMemoryBindInfo[bufferBindCount];
        for (uint32_t i=0; i<bufferBindCount; ++i) {
            pBufferBinds[i].initialize(&src.pBufferBinds[i]);
        }
    }
    if (imageOpaqueBindCount && src.pImageOpaqueBinds) {
        pImageOpaqueBinds = new safe_VkSparseImageOpaqueMemoryBindInfo[imageOpaqueBindCount];
        for (uint32_t i=0; i<imageOpaqueBindCount; ++i) {
            pImageOpaqueBinds[i].initialize(&src.pImageOpaqueBinds[i]);
        }
    }
    if (imageBindCount && src.pImageBinds) {
        pImageBinds = new safe_VkSparseImageMemoryBindInfo[imageBindCount];
        for (uint32_t i=0; i<imageBindCount; ++i) {
            pImageBinds[i].initialize(&src.pImageBinds[i]);
        }
    }
    if (signalSemaphoreCount && src.pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src.pSignalSemaphores[i];
        }
    }

    return *this;
}

safe_VkBindSparseInfo::~safe_VkBindSparseInfo()
{
    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pBufferBinds)
        delete[] pBufferBinds;
    if (pImageOpaqueBinds)
        delete[] pImageOpaqueBinds;
    if (pImageBinds)
        delete[] pImageBinds;
    if (pSignalSemaphores)
        delete[] pSignalSemaphores;
}

void safe_VkBindSparseInfo::initialize(const VkBindSparseInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    bufferBindCount = in_struct->bufferBindCount;
    pBufferBinds = nullptr;
    imageOpaqueBindCount = in_struct->imageOpaqueBindCount;
    pImageOpaqueBinds = nullptr;
    imageBindCount = in_struct->imageBindCount;
    pImageBinds = nullptr;
    signalSemaphoreCount = in_struct->signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (bufferBindCount && in_struct->pBufferBinds) {
        pBufferBinds = new safe_VkSparseBufferMemoryBindInfo[bufferBindCount];
        for (uint32_t i=0; i<bufferBindCount; ++i) {
            pBufferBinds[i].initialize(&in_struct->pBufferBinds[i]);
        }
    }
    if (imageOpaqueBindCount && in_struct->pImageOpaqueBinds) {
        pImageOpaqueBinds = new safe_VkSparseImageOpaqueMemoryBindInfo[imageOpaqueBindCount];
        for (uint32_t i=0; i<imageOpaqueBindCount; ++i) {
            pImageOpaqueBinds[i].initialize(&in_struct->pImageOpaqueBinds[i]);
        }
    }
    if (imageBindCount && in_struct->pImageBinds) {
        pImageBinds = new safe_VkSparseImageMemoryBindInfo[imageBindCount];
        for (uint32_t i=0; i<imageBindCount; ++i) {
            pImageBinds[i].initialize(&in_struct->pImageBinds[i]);
        }
    }
    if (signalSemaphoreCount && in_struct->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = in_struct->pSignalSemaphores[i];
        }
    }
}

void safe_VkBindSparseInfo::initialize(const safe_VkBindSparseInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    waitSemaphoreCount = src->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    bufferBindCount = src->bufferBindCount;
    pBufferBinds = nullptr;
    imageOpaqueBindCount = src->imageOpaqueBindCount;
    pImageOpaqueBinds = nullptr;
    imageBindCount = src->imageBindCount;
    pImageBinds = nullptr;
    signalSemaphoreCount = src->signalSemaphoreCount;
    pSignalSemaphores = nullptr;
    if (waitSemaphoreCount && src->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src->pWaitSemaphores[i];
        }
    }
    if (bufferBindCount && src->pBufferBinds) {
        pBufferBinds = new safe_VkSparseBufferMemoryBindInfo[bufferBindCount];
        for (uint32_t i=0; i<bufferBindCount; ++i) {
            pBufferBinds[i].initialize(&src->pBufferBinds[i]);
        }
    }
    if (imageOpaqueBindCount && src->pImageOpaqueBinds) {
        pImageOpaqueBinds = new safe_VkSparseImageOpaqueMemoryBindInfo[imageOpaqueBindCount];
        for (uint32_t i=0; i<imageOpaqueBindCount; ++i) {
            pImageOpaqueBinds[i].initialize(&src->pImageOpaqueBinds[i]);
        }
    }
    if (imageBindCount && src->pImageBinds) {
        pImageBinds = new safe_VkSparseImageMemoryBindInfo[imageBindCount];
        for (uint32_t i=0; i<imageBindCount; ++i) {
            pImageBinds[i].initialize(&src->pImageBinds[i]);
        }
    }
    if (signalSemaphoreCount && src->pSignalSemaphores) {
        pSignalSemaphores = new VkSemaphore[signalSemaphoreCount];
        for (uint32_t i=0; i<signalSemaphoreCount; ++i) {
            pSignalSemaphores[i] = src->pSignalSemaphores[i];
        }
    }
}

safe_VkFenceCreateInfo::safe_VkFenceCreateInfo(const VkFenceCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags)
{
}

safe_VkFenceCreateInfo::safe_VkFenceCreateInfo()
{}

safe_VkFenceCreateInfo::safe_VkFenceCreateInfo(const safe_VkFenceCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
}

safe_VkFenceCreateInfo& safe_VkFenceCreateInfo::operator=(const safe_VkFenceCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;

    return *this;
}

safe_VkFenceCreateInfo::~safe_VkFenceCreateInfo()
{
}

void safe_VkFenceCreateInfo::initialize(const VkFenceCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
}

void safe_VkFenceCreateInfo::initialize(const safe_VkFenceCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
}

safe_VkSemaphoreCreateInfo::safe_VkSemaphoreCreateInfo(const VkSemaphoreCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags)
{
}

safe_VkSemaphoreCreateInfo::safe_VkSemaphoreCreateInfo()
{}

safe_VkSemaphoreCreateInfo::safe_VkSemaphoreCreateInfo(const safe_VkSemaphoreCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
}

safe_VkSemaphoreCreateInfo& safe_VkSemaphoreCreateInfo::operator=(const safe_VkSemaphoreCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;

    return *this;
}

safe_VkSemaphoreCreateInfo::~safe_VkSemaphoreCreateInfo()
{
}

void safe_VkSemaphoreCreateInfo::initialize(const VkSemaphoreCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
}

void safe_VkSemaphoreCreateInfo::initialize(const safe_VkSemaphoreCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
}

safe_VkEventCreateInfo::safe_VkEventCreateInfo(const VkEventCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags)
{
}

safe_VkEventCreateInfo::safe_VkEventCreateInfo()
{}

safe_VkEventCreateInfo::safe_VkEventCreateInfo(const safe_VkEventCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
}

safe_VkEventCreateInfo& safe_VkEventCreateInfo::operator=(const safe_VkEventCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;

    return *this;
}

safe_VkEventCreateInfo::~safe_VkEventCreateInfo()
{
}

void safe_VkEventCreateInfo::initialize(const VkEventCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
}

void safe_VkEventCreateInfo::initialize(const safe_VkEventCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
}

safe_VkQueryPoolCreateInfo::safe_VkQueryPoolCreateInfo(const VkQueryPoolCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    queryType(in_struct->queryType),
    queryCount(in_struct->queryCount),
    pipelineStatistics(in_struct->pipelineStatistics)
{
}

safe_VkQueryPoolCreateInfo::safe_VkQueryPoolCreateInfo()
{}

safe_VkQueryPoolCreateInfo::safe_VkQueryPoolCreateInfo(const safe_VkQueryPoolCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queryType = src.queryType;
    queryCount = src.queryCount;
    pipelineStatistics = src.pipelineStatistics;
}

safe_VkQueryPoolCreateInfo& safe_VkQueryPoolCreateInfo::operator=(const safe_VkQueryPoolCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queryType = src.queryType;
    queryCount = src.queryCount;
    pipelineStatistics = src.pipelineStatistics;

    return *this;
}

safe_VkQueryPoolCreateInfo::~safe_VkQueryPoolCreateInfo()
{
}

void safe_VkQueryPoolCreateInfo::initialize(const VkQueryPoolCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    queryType = in_struct->queryType;
    queryCount = in_struct->queryCount;
    pipelineStatistics = in_struct->pipelineStatistics;
}

void safe_VkQueryPoolCreateInfo::initialize(const safe_VkQueryPoolCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    queryType = src->queryType;
    queryCount = src->queryCount;
    pipelineStatistics = src->pipelineStatistics;
}

safe_VkBufferCreateInfo::safe_VkBufferCreateInfo(const VkBufferCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    size(in_struct->size),
    usage(in_struct->usage),
    sharingMode(in_struct->sharingMode),
    queueFamilyIndexCount(in_struct->queueFamilyIndexCount),
    pQueueFamilyIndices(nullptr)
{
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

safe_VkBufferCreateInfo::safe_VkBufferCreateInfo() :
    pQueueFamilyIndices(nullptr)
{}

safe_VkBufferCreateInfo::safe_VkBufferCreateInfo(const safe_VkBufferCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    size = src.size;
    usage = src.usage;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }
}

safe_VkBufferCreateInfo& safe_VkBufferCreateInfo::operator=(const safe_VkBufferCreateInfo& src)
{
    if (&src == this) return *this;

    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    size = src.size;
    usage = src.usage;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }

    return *this;
}

safe_VkBufferCreateInfo::~safe_VkBufferCreateInfo()
{
    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;
}

void safe_VkBufferCreateInfo::initialize(const VkBufferCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    size = in_struct->size;
    usage = in_struct->usage;
    sharingMode = in_struct->sharingMode;
    queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

void safe_VkBufferCreateInfo::initialize(const safe_VkBufferCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    size = src->size;
    usage = src->usage;
    sharingMode = src->sharingMode;
    queueFamilyIndexCount = src->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src->pQueueFamilyIndices, sizeof(uint32_t)*src->queueFamilyIndexCount);
    }
}

safe_VkBufferViewCreateInfo::safe_VkBufferViewCreateInfo(const VkBufferViewCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    buffer(in_struct->buffer),
    format(in_struct->format),
    offset(in_struct->offset),
    range(in_struct->range)
{
}

safe_VkBufferViewCreateInfo::safe_VkBufferViewCreateInfo()
{}

safe_VkBufferViewCreateInfo::safe_VkBufferViewCreateInfo(const safe_VkBufferViewCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    buffer = src.buffer;
    format = src.format;
    offset = src.offset;
    range = src.range;
}

safe_VkBufferViewCreateInfo& safe_VkBufferViewCreateInfo::operator=(const safe_VkBufferViewCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    buffer = src.buffer;
    format = src.format;
    offset = src.offset;
    range = src.range;

    return *this;
}

safe_VkBufferViewCreateInfo::~safe_VkBufferViewCreateInfo()
{
}

void safe_VkBufferViewCreateInfo::initialize(const VkBufferViewCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    buffer = in_struct->buffer;
    format = in_struct->format;
    offset = in_struct->offset;
    range = in_struct->range;
}

void safe_VkBufferViewCreateInfo::initialize(const safe_VkBufferViewCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    buffer = src->buffer;
    format = src->format;
    offset = src->offset;
    range = src->range;
}

safe_VkImageCreateInfo::safe_VkImageCreateInfo(const VkImageCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    imageType(in_struct->imageType),
    format(in_struct->format),
    extent(in_struct->extent),
    mipLevels(in_struct->mipLevels),
    arrayLayers(in_struct->arrayLayers),
    samples(in_struct->samples),
    tiling(in_struct->tiling),
    usage(in_struct->usage),
    sharingMode(in_struct->sharingMode),
    queueFamilyIndexCount(in_struct->queueFamilyIndexCount),
    pQueueFamilyIndices(nullptr),
    initialLayout(in_struct->initialLayout)
{
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

safe_VkImageCreateInfo::safe_VkImageCreateInfo() :
    pQueueFamilyIndices(nullptr)
{}

safe_VkImageCreateInfo::safe_VkImageCreateInfo(const safe_VkImageCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    imageType = src.imageType;
    format = src.format;
    extent = src.extent;
    mipLevels = src.mipLevels;
    arrayLayers = src.arrayLayers;
    samples = src.samples;
    tiling = src.tiling;
    usage = src.usage;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    initialLayout = src.initialLayout;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }
}

safe_VkImageCreateInfo& safe_VkImageCreateInfo::operator=(const safe_VkImageCreateInfo& src)
{
    if (&src == this) return *this;

    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    imageType = src.imageType;
    format = src.format;
    extent = src.extent;
    mipLevels = src.mipLevels;
    arrayLayers = src.arrayLayers;
    samples = src.samples;
    tiling = src.tiling;
    usage = src.usage;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    initialLayout = src.initialLayout;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }

    return *this;
}

safe_VkImageCreateInfo::~safe_VkImageCreateInfo()
{
    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;
}

void safe_VkImageCreateInfo::initialize(const VkImageCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    imageType = in_struct->imageType;
    format = in_struct->format;
    extent = in_struct->extent;
    mipLevels = in_struct->mipLevels;
    arrayLayers = in_struct->arrayLayers;
    samples = in_struct->samples;
    tiling = in_struct->tiling;
    usage = in_struct->usage;
    sharingMode = in_struct->sharingMode;
    queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    initialLayout = in_struct->initialLayout;
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

void safe_VkImageCreateInfo::initialize(const safe_VkImageCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    imageType = src->imageType;
    format = src->format;
    extent = src->extent;
    mipLevels = src->mipLevels;
    arrayLayers = src->arrayLayers;
    samples = src->samples;
    tiling = src->tiling;
    usage = src->usage;
    sharingMode = src->sharingMode;
    queueFamilyIndexCount = src->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    initialLayout = src->initialLayout;
    if (src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src->pQueueFamilyIndices, sizeof(uint32_t)*src->queueFamilyIndexCount);
    }
}

safe_VkImageViewCreateInfo::safe_VkImageViewCreateInfo(const VkImageViewCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    image(in_struct->image),
    viewType(in_struct->viewType),
    format(in_struct->format),
    components(in_struct->components),
    subresourceRange(in_struct->subresourceRange)
{
}

safe_VkImageViewCreateInfo::safe_VkImageViewCreateInfo()
{}

safe_VkImageViewCreateInfo::safe_VkImageViewCreateInfo(const safe_VkImageViewCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    image = src.image;
    viewType = src.viewType;
    format = src.format;
    components = src.components;
    subresourceRange = src.subresourceRange;
}

safe_VkImageViewCreateInfo& safe_VkImageViewCreateInfo::operator=(const safe_VkImageViewCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    image = src.image;
    viewType = src.viewType;
    format = src.format;
    components = src.components;
    subresourceRange = src.subresourceRange;

    return *this;
}

safe_VkImageViewCreateInfo::~safe_VkImageViewCreateInfo()
{
}

void safe_VkImageViewCreateInfo::initialize(const VkImageViewCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    image = in_struct->image;
    viewType = in_struct->viewType;
    format = in_struct->format;
    components = in_struct->components;
    subresourceRange = in_struct->subresourceRange;
}

void safe_VkImageViewCreateInfo::initialize(const safe_VkImageViewCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    image = src->image;
    viewType = src->viewType;
    format = src->format;
    components = src->components;
    subresourceRange = src->subresourceRange;
}

safe_VkShaderModuleCreateInfo::safe_VkShaderModuleCreateInfo(const VkShaderModuleCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    codeSize(in_struct->codeSize),
    pCode(nullptr)
{
    if (in_struct->pCode) {
        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
        memcpy((void *)pCode, (void *)in_struct->pCode, codeSize);
    }
}

safe_VkShaderModuleCreateInfo::safe_VkShaderModuleCreateInfo() :
    pCode(nullptr)
{}

safe_VkShaderModuleCreateInfo::safe_VkShaderModuleCreateInfo(const safe_VkShaderModuleCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    codeSize = src.codeSize;
    pCode = nullptr;
    if (src.pCode) {
        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
        memcpy((void *)pCode, (void *)src.pCode, codeSize);
    }
}

safe_VkShaderModuleCreateInfo& safe_VkShaderModuleCreateInfo::operator=(const safe_VkShaderModuleCreateInfo& src)
{
    if (&src == this) return *this;

    if (pCode)
        delete[] reinterpret_cast<const uint8_t *>(pCode);

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    codeSize = src.codeSize;
    pCode = nullptr;
    if (src.pCode) {
        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
        memcpy((void *)pCode, (void *)src.pCode, codeSize);
    }

    return *this;
}

safe_VkShaderModuleCreateInfo::~safe_VkShaderModuleCreateInfo()
{
    if (pCode)
        delete[] reinterpret_cast<const uint8_t *>(pCode);
}

void safe_VkShaderModuleCreateInfo::initialize(const VkShaderModuleCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    codeSize = in_struct->codeSize;
    pCode = nullptr;
    if (in_struct->pCode) {
        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
        memcpy((void *)pCode, (void *)in_struct->pCode, codeSize);
    }
}

void safe_VkShaderModuleCreateInfo::initialize(const safe_VkShaderModuleCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    codeSize = src->codeSize;
    pCode = nullptr;
    if (src->pCode) {
        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
        memcpy((void *)pCode, (void *)src->pCode, codeSize);
    }
}

safe_VkPipelineCacheCreateInfo::safe_VkPipelineCacheCreateInfo(const VkPipelineCacheCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    initialDataSize(in_struct->initialDataSize),
    pInitialData(in_struct->pInitialData)
{
}

safe_VkPipelineCacheCreateInfo::safe_VkPipelineCacheCreateInfo()
{}

safe_VkPipelineCacheCreateInfo::safe_VkPipelineCacheCreateInfo(const safe_VkPipelineCacheCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    initialDataSize = src.initialDataSize;
    pInitialData = src.pInitialData;
}

safe_VkPipelineCacheCreateInfo& safe_VkPipelineCacheCreateInfo::operator=(const safe_VkPipelineCacheCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    initialDataSize = src.initialDataSize;
    pInitialData = src.pInitialData;

    return *this;
}

safe_VkPipelineCacheCreateInfo::~safe_VkPipelineCacheCreateInfo()
{
}

void safe_VkPipelineCacheCreateInfo::initialize(const VkPipelineCacheCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    initialDataSize = in_struct->initialDataSize;
    pInitialData = in_struct->pInitialData;
}

void safe_VkPipelineCacheCreateInfo::initialize(const safe_VkPipelineCacheCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    initialDataSize = src->initialDataSize;
    pInitialData = src->pInitialData;
}

safe_VkSpecializationInfo::safe_VkSpecializationInfo(const VkSpecializationInfo* in_struct) :
    mapEntryCount(in_struct->mapEntryCount),
    pMapEntries(nullptr),
    dataSize(in_struct->dataSize),
    pData(in_struct->pData)
{
    if (in_struct->pMapEntries) {
        pMapEntries = new VkSpecializationMapEntry[in_struct->mapEntryCount];
        memcpy ((void *)pMapEntries, (void *)in_struct->pMapEntries, sizeof(VkSpecializationMapEntry)*in_struct->mapEntryCount);
    }
}

safe_VkSpecializationInfo::safe_VkSpecializationInfo() :
    pMapEntries(nullptr)
{}

safe_VkSpecializationInfo::safe_VkSpecializationInfo(const safe_VkSpecializationInfo& src)
{
    mapEntryCount = src.mapEntryCount;
    pMapEntries = nullptr;
    dataSize = src.dataSize;
    pData = src.pData;
    if (src.pMapEntries) {
        pMapEntries = new VkSpecializationMapEntry[src.mapEntryCount];
        memcpy ((void *)pMapEntries, (void *)src.pMapEntries, sizeof(VkSpecializationMapEntry)*src.mapEntryCount);
    }
}

safe_VkSpecializationInfo& safe_VkSpecializationInfo::operator=(const safe_VkSpecializationInfo& src)
{
    if (&src == this) return *this;

    if (pMapEntries)
        delete[] pMapEntries;

    mapEntryCount = src.mapEntryCount;
    pMapEntries = nullptr;
    dataSize = src.dataSize;
    pData = src.pData;
    if (src.pMapEntries) {
        pMapEntries = new VkSpecializationMapEntry[src.mapEntryCount];
        memcpy ((void *)pMapEntries, (void *)src.pMapEntries, sizeof(VkSpecializationMapEntry)*src.mapEntryCount);
    }

    return *this;
}

safe_VkSpecializationInfo::~safe_VkSpecializationInfo()
{
    if (pMapEntries)
        delete[] pMapEntries;
}

void safe_VkSpecializationInfo::initialize(const VkSpecializationInfo* in_struct)
{
    mapEntryCount = in_struct->mapEntryCount;
    pMapEntries = nullptr;
    dataSize = in_struct->dataSize;
    pData = in_struct->pData;
    if (in_struct->pMapEntries) {
        pMapEntries = new VkSpecializationMapEntry[in_struct->mapEntryCount];
        memcpy ((void *)pMapEntries, (void *)in_struct->pMapEntries, sizeof(VkSpecializationMapEntry)*in_struct->mapEntryCount);
    }
}

void safe_VkSpecializationInfo::initialize(const safe_VkSpecializationInfo* src)
{
    mapEntryCount = src->mapEntryCount;
    pMapEntries = nullptr;
    dataSize = src->dataSize;
    pData = src->pData;
    if (src->pMapEntries) {
        pMapEntries = new VkSpecializationMapEntry[src->mapEntryCount];
        memcpy ((void *)pMapEntries, (void *)src->pMapEntries, sizeof(VkSpecializationMapEntry)*src->mapEntryCount);
    }
}

safe_VkPipelineShaderStageCreateInfo::safe_VkPipelineShaderStageCreateInfo(const VkPipelineShaderStageCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    stage(in_struct->stage),
    module(in_struct->module),
    pName(in_struct->pName)
{
    if (in_struct->pSpecializationInfo)
        pSpecializationInfo = new safe_VkSpecializationInfo(in_struct->pSpecializationInfo);
    else
        pSpecializationInfo = NULL;
}

safe_VkPipelineShaderStageCreateInfo::safe_VkPipelineShaderStageCreateInfo()
{}

safe_VkPipelineShaderStageCreateInfo::safe_VkPipelineShaderStageCreateInfo(const safe_VkPipelineShaderStageCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stage = src.stage;
    module = src.module;
    pName = src.pName;
    if (src.pSpecializationInfo)
        pSpecializationInfo = new safe_VkSpecializationInfo(*src.pSpecializationInfo);
    else
        pSpecializationInfo = NULL;
}

safe_VkPipelineShaderStageCreateInfo& safe_VkPipelineShaderStageCreateInfo::operator=(const safe_VkPipelineShaderStageCreateInfo& src)
{
    if (&src == this) return *this;

    if (pSpecializationInfo)
        delete pSpecializationInfo;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stage = src.stage;
    module = src.module;
    pName = src.pName;
    if (src.pSpecializationInfo)
        pSpecializationInfo = new safe_VkSpecializationInfo(*src.pSpecializationInfo);
    else
        pSpecializationInfo = NULL;

    return *this;
}

safe_VkPipelineShaderStageCreateInfo::~safe_VkPipelineShaderStageCreateInfo()
{
    if (pSpecializationInfo)
        delete pSpecializationInfo;
}

void safe_VkPipelineShaderStageCreateInfo::initialize(const VkPipelineShaderStageCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    stage = in_struct->stage;
    module = in_struct->module;
    pName = in_struct->pName;
    if (in_struct->pSpecializationInfo)
        pSpecializationInfo = new safe_VkSpecializationInfo(in_struct->pSpecializationInfo);
    else
        pSpecializationInfo = NULL;
}

void safe_VkPipelineShaderStageCreateInfo::initialize(const safe_VkPipelineShaderStageCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    stage = src->stage;
    module = src->module;
    pName = src->pName;
    if (src->pSpecializationInfo)
        pSpecializationInfo = new safe_VkSpecializationInfo(*src->pSpecializationInfo);
    else
        pSpecializationInfo = NULL;
}

safe_VkPipelineVertexInputStateCreateInfo::safe_VkPipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    vertexBindingDescriptionCount(in_struct->vertexBindingDescriptionCount),
    pVertexBindingDescriptions(nullptr),
    vertexAttributeDescriptionCount(in_struct->vertexAttributeDescriptionCount),
    pVertexAttributeDescriptions(nullptr)
{
    if (in_struct->pVertexBindingDescriptions) {
        pVertexBindingDescriptions = new VkVertexInputBindingDescription[in_struct->vertexBindingDescriptionCount];
        memcpy ((void *)pVertexBindingDescriptions, (void *)in_struct->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription)*in_struct->vertexBindingDescriptionCount);
    }
    if (in_struct->pVertexAttributeDescriptions) {
        pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[in_struct->vertexAttributeDescriptionCount];
        memcpy ((void *)pVertexAttributeDescriptions, (void *)in_struct->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription)*in_struct->vertexAttributeDescriptionCount);
    }
}

safe_VkPipelineVertexInputStateCreateInfo::safe_VkPipelineVertexInputStateCreateInfo() :
    pVertexBindingDescriptions(nullptr),
    pVertexAttributeDescriptions(nullptr)
{}

safe_VkPipelineVertexInputStateCreateInfo::safe_VkPipelineVertexInputStateCreateInfo(const safe_VkPipelineVertexInputStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    vertexBindingDescriptionCount = src.vertexBindingDescriptionCount;
    pVertexBindingDescriptions = nullptr;
    vertexAttributeDescriptionCount = src.vertexAttributeDescriptionCount;
    pVertexAttributeDescriptions = nullptr;
    if (src.pVertexBindingDescriptions) {
        pVertexBindingDescriptions = new VkVertexInputBindingDescription[src.vertexBindingDescriptionCount];
        memcpy ((void *)pVertexBindingDescriptions, (void *)src.pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription)*src.vertexBindingDescriptionCount);
    }
    if (src.pVertexAttributeDescriptions) {
        pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[src.vertexAttributeDescriptionCount];
        memcpy ((void *)pVertexAttributeDescriptions, (void *)src.pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription)*src.vertexAttributeDescriptionCount);
    }
}

safe_VkPipelineVertexInputStateCreateInfo& safe_VkPipelineVertexInputStateCreateInfo::operator=(const safe_VkPipelineVertexInputStateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pVertexBindingDescriptions)
        delete[] pVertexBindingDescriptions;
    if (pVertexAttributeDescriptions)
        delete[] pVertexAttributeDescriptions;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    vertexBindingDescriptionCount = src.vertexBindingDescriptionCount;
    pVertexBindingDescriptions = nullptr;
    vertexAttributeDescriptionCount = src.vertexAttributeDescriptionCount;
    pVertexAttributeDescriptions = nullptr;
    if (src.pVertexBindingDescriptions) {
        pVertexBindingDescriptions = new VkVertexInputBindingDescription[src.vertexBindingDescriptionCount];
        memcpy ((void *)pVertexBindingDescriptions, (void *)src.pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription)*src.vertexBindingDescriptionCount);
    }
    if (src.pVertexAttributeDescriptions) {
        pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[src.vertexAttributeDescriptionCount];
        memcpy ((void *)pVertexAttributeDescriptions, (void *)src.pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription)*src.vertexAttributeDescriptionCount);
    }

    return *this;
}

safe_VkPipelineVertexInputStateCreateInfo::~safe_VkPipelineVertexInputStateCreateInfo()
{
    if (pVertexBindingDescriptions)
        delete[] pVertexBindingDescriptions;
    if (pVertexAttributeDescriptions)
        delete[] pVertexAttributeDescriptions;
}

void safe_VkPipelineVertexInputStateCreateInfo::initialize(const VkPipelineVertexInputStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    vertexBindingDescriptionCount = in_struct->vertexBindingDescriptionCount;
    pVertexBindingDescriptions = nullptr;
    vertexAttributeDescriptionCount = in_struct->vertexAttributeDescriptionCount;
    pVertexAttributeDescriptions = nullptr;
    if (in_struct->pVertexBindingDescriptions) {
        pVertexBindingDescriptions = new VkVertexInputBindingDescription[in_struct->vertexBindingDescriptionCount];
        memcpy ((void *)pVertexBindingDescriptions, (void *)in_struct->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription)*in_struct->vertexBindingDescriptionCount);
    }
    if (in_struct->pVertexAttributeDescriptions) {
        pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[in_struct->vertexAttributeDescriptionCount];
        memcpy ((void *)pVertexAttributeDescriptions, (void *)in_struct->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription)*in_struct->vertexAttributeDescriptionCount);
    }
}

void safe_VkPipelineVertexInputStateCreateInfo::initialize(const safe_VkPipelineVertexInputStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    vertexBindingDescriptionCount = src->vertexBindingDescriptionCount;
    pVertexBindingDescriptions = nullptr;
    vertexAttributeDescriptionCount = src->vertexAttributeDescriptionCount;
    pVertexAttributeDescriptions = nullptr;
    if (src->pVertexBindingDescriptions) {
        pVertexBindingDescriptions = new VkVertexInputBindingDescription[src->vertexBindingDescriptionCount];
        memcpy ((void *)pVertexBindingDescriptions, (void *)src->pVertexBindingDescriptions, sizeof(VkVertexInputBindingDescription)*src->vertexBindingDescriptionCount);
    }
    if (src->pVertexAttributeDescriptions) {
        pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[src->vertexAttributeDescriptionCount];
        memcpy ((void *)pVertexAttributeDescriptions, (void *)src->pVertexAttributeDescriptions, sizeof(VkVertexInputAttributeDescription)*src->vertexAttributeDescriptionCount);
    }
}

safe_VkPipelineInputAssemblyStateCreateInfo::safe_VkPipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    topology(in_struct->topology),
    primitiveRestartEnable(in_struct->primitiveRestartEnable)
{
}

safe_VkPipelineInputAssemblyStateCreateInfo::safe_VkPipelineInputAssemblyStateCreateInfo()
{}

safe_VkPipelineInputAssemblyStateCreateInfo::safe_VkPipelineInputAssemblyStateCreateInfo(const safe_VkPipelineInputAssemblyStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    topology = src.topology;
    primitiveRestartEnable = src.primitiveRestartEnable;
}

safe_VkPipelineInputAssemblyStateCreateInfo& safe_VkPipelineInputAssemblyStateCreateInfo::operator=(const safe_VkPipelineInputAssemblyStateCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    topology = src.topology;
    primitiveRestartEnable = src.primitiveRestartEnable;

    return *this;
}

safe_VkPipelineInputAssemblyStateCreateInfo::~safe_VkPipelineInputAssemblyStateCreateInfo()
{
}

void safe_VkPipelineInputAssemblyStateCreateInfo::initialize(const VkPipelineInputAssemblyStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    topology = in_struct->topology;
    primitiveRestartEnable = in_struct->primitiveRestartEnable;
}

void safe_VkPipelineInputAssemblyStateCreateInfo::initialize(const safe_VkPipelineInputAssemblyStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    topology = src->topology;
    primitiveRestartEnable = src->primitiveRestartEnable;
}

safe_VkPipelineTessellationStateCreateInfo::safe_VkPipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    patchControlPoints(in_struct->patchControlPoints)
{
}

safe_VkPipelineTessellationStateCreateInfo::safe_VkPipelineTessellationStateCreateInfo()
{}

safe_VkPipelineTessellationStateCreateInfo::safe_VkPipelineTessellationStateCreateInfo(const safe_VkPipelineTessellationStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    patchControlPoints = src.patchControlPoints;
}

safe_VkPipelineTessellationStateCreateInfo& safe_VkPipelineTessellationStateCreateInfo::operator=(const safe_VkPipelineTessellationStateCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    patchControlPoints = src.patchControlPoints;

    return *this;
}

safe_VkPipelineTessellationStateCreateInfo::~safe_VkPipelineTessellationStateCreateInfo()
{
}

void safe_VkPipelineTessellationStateCreateInfo::initialize(const VkPipelineTessellationStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    patchControlPoints = in_struct->patchControlPoints;
}

void safe_VkPipelineTessellationStateCreateInfo::initialize(const safe_VkPipelineTessellationStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    patchControlPoints = src->patchControlPoints;
}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo* in_struct, const bool is_dynamic_viewports, const bool is_dynamic_scissors) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    viewportCount(in_struct->viewportCount),
    pViewports(nullptr),
    scissorCount(in_struct->scissorCount),
    pScissors(nullptr)
{
    if (in_struct->pViewports && !is_dynamic_viewports) {
        pViewports = new VkViewport[in_struct->viewportCount];
        memcpy ((void *)pViewports, (void *)in_struct->pViewports, sizeof(VkViewport)*in_struct->viewportCount);
    }
    else
        pViewports = NULL;
    if (in_struct->pScissors && !is_dynamic_scissors) {
        pScissors = new VkRect2D[in_struct->scissorCount];
        memcpy ((void *)pScissors, (void *)in_struct->pScissors, sizeof(VkRect2D)*in_struct->scissorCount);
    }
    else
        pScissors = NULL;
}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo() :
    pViewports(nullptr),
    pScissors(nullptr)
{}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo(const safe_VkPipelineViewportStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    viewportCount = src.viewportCount;
    pViewports = nullptr;
    scissorCount = src.scissorCount;
    pScissors = nullptr;
    if (src.pViewports) {
        pViewports = new VkViewport[src.viewportCount];
        memcpy ((void *)pViewports, (void *)src.pViewports, sizeof(VkViewport)*src.viewportCount);
    }
    else
        pViewports = NULL;
    if (src.pScissors) {
        pScissors = new VkRect2D[src.scissorCount];
        memcpy ((void *)pScissors, (void *)src.pScissors, sizeof(VkRect2D)*src.scissorCount);
    }
    else
        pScissors = NULL;
}

safe_VkPipelineViewportStateCreateInfo& safe_VkPipelineViewportStateCreateInfo::operator=(const safe_VkPipelineViewportStateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pViewports)
        delete[] pViewports;
    if (pScissors)
        delete[] pScissors;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    viewportCount = src.viewportCount;
    pViewports = nullptr;
    scissorCount = src.scissorCount;
    pScissors = nullptr;
    if (src.pViewports) {
        pViewports = new VkViewport[src.viewportCount];
        memcpy ((void *)pViewports, (void *)src.pViewports, sizeof(VkViewport)*src.viewportCount);
    }
    else
        pViewports = NULL;
    if (src.pScissors) {
        pScissors = new VkRect2D[src.scissorCount];
        memcpy ((void *)pScissors, (void *)src.pScissors, sizeof(VkRect2D)*src.scissorCount);
    }
    else
        pScissors = NULL;

    return *this;
}

safe_VkPipelineViewportStateCreateInfo::~safe_VkPipelineViewportStateCreateInfo()
{
    if (pViewports)
        delete[] pViewports;
    if (pScissors)
        delete[] pScissors;
}

void safe_VkPipelineViewportStateCreateInfo::initialize(const VkPipelineViewportStateCreateInfo* in_struct, const bool is_dynamic_viewports, const bool is_dynamic_scissors)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    viewportCount = in_struct->viewportCount;
    pViewports = nullptr;
    scissorCount = in_struct->scissorCount;
    pScissors = nullptr;
    if (in_struct->pViewports && !is_dynamic_viewports) {
        pViewports = new VkViewport[in_struct->viewportCount];
        memcpy ((void *)pViewports, (void *)in_struct->pViewports, sizeof(VkViewport)*in_struct->viewportCount);
    }
    else
        pViewports = NULL;
    if (in_struct->pScissors && !is_dynamic_scissors) {
        pScissors = new VkRect2D[in_struct->scissorCount];
        memcpy ((void *)pScissors, (void *)in_struct->pScissors, sizeof(VkRect2D)*in_struct->scissorCount);
    }
    else
        pScissors = NULL;
}

void safe_VkPipelineViewportStateCreateInfo::initialize(const safe_VkPipelineViewportStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    viewportCount = src->viewportCount;
    pViewports = nullptr;
    scissorCount = src->scissorCount;
    pScissors = nullptr;
    if (src->pViewports) {
        pViewports = new VkViewport[src->viewportCount];
        memcpy ((void *)pViewports, (void *)src->pViewports, sizeof(VkViewport)*src->viewportCount);
    }
    else
        pViewports = NULL;
    if (src->pScissors) {
        pScissors = new VkRect2D[src->scissorCount];
        memcpy ((void *)pScissors, (void *)src->pScissors, sizeof(VkRect2D)*src->scissorCount);
    }
    else
        pScissors = NULL;
}

safe_VkPipelineRasterizationStateCreateInfo::safe_VkPipelineRasterizationStateCreateInfo(const VkPipelineRasterizationStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    depthClampEnable(in_struct->depthClampEnable),
    rasterizerDiscardEnable(in_struct->rasterizerDiscardEnable),
    polygonMode(in_struct->polygonMode),
    cullMode(in_struct->cullMode),
    frontFace(in_struct->frontFace),
    depthBiasEnable(in_struct->depthBiasEnable),
    depthBiasConstantFactor(in_struct->depthBiasConstantFactor),
    depthBiasClamp(in_struct->depthBiasClamp),
    depthBiasSlopeFactor(in_struct->depthBiasSlopeFactor),
    lineWidth(in_struct->lineWidth)
{
}

safe_VkPipelineRasterizationStateCreateInfo::safe_VkPipelineRasterizationStateCreateInfo()
{}

safe_VkPipelineRasterizationStateCreateInfo::safe_VkPipelineRasterizationStateCreateInfo(const safe_VkPipelineRasterizationStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthClampEnable = src.depthClampEnable;
    rasterizerDiscardEnable = src.rasterizerDiscardEnable;
    polygonMode = src.polygonMode;
    cullMode = src.cullMode;
    frontFace = src.frontFace;
    depthBiasEnable = src.depthBiasEnable;
    depthBiasConstantFactor = src.depthBiasConstantFactor;
    depthBiasClamp = src.depthBiasClamp;
    depthBiasSlopeFactor = src.depthBiasSlopeFactor;
    lineWidth = src.lineWidth;
}

safe_VkPipelineRasterizationStateCreateInfo& safe_VkPipelineRasterizationStateCreateInfo::operator=(const safe_VkPipelineRasterizationStateCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthClampEnable = src.depthClampEnable;
    rasterizerDiscardEnable = src.rasterizerDiscardEnable;
    polygonMode = src.polygonMode;
    cullMode = src.cullMode;
    frontFace = src.frontFace;
    depthBiasEnable = src.depthBiasEnable;
    depthBiasConstantFactor = src.depthBiasConstantFactor;
    depthBiasClamp = src.depthBiasClamp;
    depthBiasSlopeFactor = src.depthBiasSlopeFactor;
    lineWidth = src.lineWidth;

    return *this;
}

safe_VkPipelineRasterizationStateCreateInfo::~safe_VkPipelineRasterizationStateCreateInfo()
{
}

void safe_VkPipelineRasterizationStateCreateInfo::initialize(const VkPipelineRasterizationStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    depthClampEnable = in_struct->depthClampEnable;
    rasterizerDiscardEnable = in_struct->rasterizerDiscardEnable;
    polygonMode = in_struct->polygonMode;
    cullMode = in_struct->cullMode;
    frontFace = in_struct->frontFace;
    depthBiasEnable = in_struct->depthBiasEnable;
    depthBiasConstantFactor = in_struct->depthBiasConstantFactor;
    depthBiasClamp = in_struct->depthBiasClamp;
    depthBiasSlopeFactor = in_struct->depthBiasSlopeFactor;
    lineWidth = in_struct->lineWidth;
}

void safe_VkPipelineRasterizationStateCreateInfo::initialize(const safe_VkPipelineRasterizationStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    depthClampEnable = src->depthClampEnable;
    rasterizerDiscardEnable = src->rasterizerDiscardEnable;
    polygonMode = src->polygonMode;
    cullMode = src->cullMode;
    frontFace = src->frontFace;
    depthBiasEnable = src->depthBiasEnable;
    depthBiasConstantFactor = src->depthBiasConstantFactor;
    depthBiasClamp = src->depthBiasClamp;
    depthBiasSlopeFactor = src->depthBiasSlopeFactor;
    lineWidth = src->lineWidth;
}

safe_VkPipelineMultisampleStateCreateInfo::safe_VkPipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    rasterizationSamples(in_struct->rasterizationSamples),
    sampleShadingEnable(in_struct->sampleShadingEnable),
    minSampleShading(in_struct->minSampleShading),
    pSampleMask(nullptr),
    alphaToCoverageEnable(in_struct->alphaToCoverageEnable),
    alphaToOneEnable(in_struct->alphaToOneEnable)
{
    if (in_struct->pSampleMask) {
        pSampleMask = new VkSampleMask(*in_struct->pSampleMask);
    }
}

safe_VkPipelineMultisampleStateCreateInfo::safe_VkPipelineMultisampleStateCreateInfo() :
    pSampleMask(nullptr)
{}

safe_VkPipelineMultisampleStateCreateInfo::safe_VkPipelineMultisampleStateCreateInfo(const safe_VkPipelineMultisampleStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    rasterizationSamples = src.rasterizationSamples;
    sampleShadingEnable = src.sampleShadingEnable;
    minSampleShading = src.minSampleShading;
    pSampleMask = nullptr;
    alphaToCoverageEnable = src.alphaToCoverageEnable;
    alphaToOneEnable = src.alphaToOneEnable;
    if (src.pSampleMask) {
        pSampleMask = new VkSampleMask(*src.pSampleMask);
    }
}

safe_VkPipelineMultisampleStateCreateInfo& safe_VkPipelineMultisampleStateCreateInfo::operator=(const safe_VkPipelineMultisampleStateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pSampleMask)
        delete pSampleMask;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    rasterizationSamples = src.rasterizationSamples;
    sampleShadingEnable = src.sampleShadingEnable;
    minSampleShading = src.minSampleShading;
    pSampleMask = nullptr;
    alphaToCoverageEnable = src.alphaToCoverageEnable;
    alphaToOneEnable = src.alphaToOneEnable;
    if (src.pSampleMask) {
        pSampleMask = new VkSampleMask(*src.pSampleMask);
    }

    return *this;
}

safe_VkPipelineMultisampleStateCreateInfo::~safe_VkPipelineMultisampleStateCreateInfo()
{
    if (pSampleMask)
        delete pSampleMask;
}

void safe_VkPipelineMultisampleStateCreateInfo::initialize(const VkPipelineMultisampleStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    rasterizationSamples = in_struct->rasterizationSamples;
    sampleShadingEnable = in_struct->sampleShadingEnable;
    minSampleShading = in_struct->minSampleShading;
    pSampleMask = nullptr;
    alphaToCoverageEnable = in_struct->alphaToCoverageEnable;
    alphaToOneEnable = in_struct->alphaToOneEnable;
    if (in_struct->pSampleMask) {
        pSampleMask = new VkSampleMask(*in_struct->pSampleMask);
    }
}

void safe_VkPipelineMultisampleStateCreateInfo::initialize(const safe_VkPipelineMultisampleStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    rasterizationSamples = src->rasterizationSamples;
    sampleShadingEnable = src->sampleShadingEnable;
    minSampleShading = src->minSampleShading;
    pSampleMask = nullptr;
    alphaToCoverageEnable = src->alphaToCoverageEnable;
    alphaToOneEnable = src->alphaToOneEnable;
    if (src->pSampleMask) {
        pSampleMask = new VkSampleMask(*src->pSampleMask);
    }
}

safe_VkPipelineDepthStencilStateCreateInfo::safe_VkPipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    depthTestEnable(in_struct->depthTestEnable),
    depthWriteEnable(in_struct->depthWriteEnable),
    depthCompareOp(in_struct->depthCompareOp),
    depthBoundsTestEnable(in_struct->depthBoundsTestEnable),
    stencilTestEnable(in_struct->stencilTestEnable),
    front(in_struct->front),
    back(in_struct->back),
    minDepthBounds(in_struct->minDepthBounds),
    maxDepthBounds(in_struct->maxDepthBounds)
{
}

safe_VkPipelineDepthStencilStateCreateInfo::safe_VkPipelineDepthStencilStateCreateInfo()
{}

safe_VkPipelineDepthStencilStateCreateInfo::safe_VkPipelineDepthStencilStateCreateInfo(const safe_VkPipelineDepthStencilStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthTestEnable = src.depthTestEnable;
    depthWriteEnable = src.depthWriteEnable;
    depthCompareOp = src.depthCompareOp;
    depthBoundsTestEnable = src.depthBoundsTestEnable;
    stencilTestEnable = src.stencilTestEnable;
    front = src.front;
    back = src.back;
    minDepthBounds = src.minDepthBounds;
    maxDepthBounds = src.maxDepthBounds;
}

safe_VkPipelineDepthStencilStateCreateInfo& safe_VkPipelineDepthStencilStateCreateInfo::operator=(const safe_VkPipelineDepthStencilStateCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthTestEnable = src.depthTestEnable;
    depthWriteEnable = src.depthWriteEnable;
    depthCompareOp = src.depthCompareOp;
    depthBoundsTestEnable = src.depthBoundsTestEnable;
    stencilTestEnable = src.stencilTestEnable;
    front = src.front;
    back = src.back;
    minDepthBounds = src.minDepthBounds;
    maxDepthBounds = src.maxDepthBounds;

    return *this;
}

safe_VkPipelineDepthStencilStateCreateInfo::~safe_VkPipelineDepthStencilStateCreateInfo()
{
}

void safe_VkPipelineDepthStencilStateCreateInfo::initialize(const VkPipelineDepthStencilStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    depthTestEnable = in_struct->depthTestEnable;
    depthWriteEnable = in_struct->depthWriteEnable;
    depthCompareOp = in_struct->depthCompareOp;
    depthBoundsTestEnable = in_struct->depthBoundsTestEnable;
    stencilTestEnable = in_struct->stencilTestEnable;
    front = in_struct->front;
    back = in_struct->back;
    minDepthBounds = in_struct->minDepthBounds;
    maxDepthBounds = in_struct->maxDepthBounds;
}

void safe_VkPipelineDepthStencilStateCreateInfo::initialize(const safe_VkPipelineDepthStencilStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    depthTestEnable = src->depthTestEnable;
    depthWriteEnable = src->depthWriteEnable;
    depthCompareOp = src->depthCompareOp;
    depthBoundsTestEnable = src->depthBoundsTestEnable;
    stencilTestEnable = src->stencilTestEnable;
    front = src->front;
    back = src->back;
    minDepthBounds = src->minDepthBounds;
    maxDepthBounds = src->maxDepthBounds;
}

safe_VkPipelineColorBlendStateCreateInfo::safe_VkPipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    logicOpEnable(in_struct->logicOpEnable),
    logicOp(in_struct->logicOp),
    attachmentCount(in_struct->attachmentCount),
    pAttachments(nullptr)
{
    if (in_struct->pAttachments) {
        pAttachments = new VkPipelineColorBlendAttachmentState[in_struct->attachmentCount];
        memcpy ((void *)pAttachments, (void *)in_struct->pAttachments, sizeof(VkPipelineColorBlendAttachmentState)*in_struct->attachmentCount);
    }
    for (uint32_t i=0; i<4; ++i) {
        blendConstants[i] = in_struct->blendConstants[i];
    }
}

safe_VkPipelineColorBlendStateCreateInfo::safe_VkPipelineColorBlendStateCreateInfo() :
    pAttachments(nullptr)
{}

safe_VkPipelineColorBlendStateCreateInfo::safe_VkPipelineColorBlendStateCreateInfo(const safe_VkPipelineColorBlendStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    logicOpEnable = src.logicOpEnable;
    logicOp = src.logicOp;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    if (src.pAttachments) {
        pAttachments = new VkPipelineColorBlendAttachmentState[src.attachmentCount];
        memcpy ((void *)pAttachments, (void *)src.pAttachments, sizeof(VkPipelineColorBlendAttachmentState)*src.attachmentCount);
    }
    for (uint32_t i=0; i<4; ++i) {
        blendConstants[i] = src.blendConstants[i];
    }
}

safe_VkPipelineColorBlendStateCreateInfo& safe_VkPipelineColorBlendStateCreateInfo::operator=(const safe_VkPipelineColorBlendStateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pAttachments)
        delete[] pAttachments;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    logicOpEnable = src.logicOpEnable;
    logicOp = src.logicOp;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    if (src.pAttachments) {
        pAttachments = new VkPipelineColorBlendAttachmentState[src.attachmentCount];
        memcpy ((void *)pAttachments, (void *)src.pAttachments, sizeof(VkPipelineColorBlendAttachmentState)*src.attachmentCount);
    }
    for (uint32_t i=0; i<4; ++i) {
        blendConstants[i] = src.blendConstants[i];
    }

    return *this;
}

safe_VkPipelineColorBlendStateCreateInfo::~safe_VkPipelineColorBlendStateCreateInfo()
{
    if (pAttachments)
        delete[] pAttachments;
}

void safe_VkPipelineColorBlendStateCreateInfo::initialize(const VkPipelineColorBlendStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    logicOpEnable = in_struct->logicOpEnable;
    logicOp = in_struct->logicOp;
    attachmentCount = in_struct->attachmentCount;
    pAttachments = nullptr;
    if (in_struct->pAttachments) {
        pAttachments = new VkPipelineColorBlendAttachmentState[in_struct->attachmentCount];
        memcpy ((void *)pAttachments, (void *)in_struct->pAttachments, sizeof(VkPipelineColorBlendAttachmentState)*in_struct->attachmentCount);
    }
    for (uint32_t i=0; i<4; ++i) {
        blendConstants[i] = in_struct->blendConstants[i];
    }
}

void safe_VkPipelineColorBlendStateCreateInfo::initialize(const safe_VkPipelineColorBlendStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    logicOpEnable = src->logicOpEnable;
    logicOp = src->logicOp;
    attachmentCount = src->attachmentCount;
    pAttachments = nullptr;
    if (src->pAttachments) {
        pAttachments = new VkPipelineColorBlendAttachmentState[src->attachmentCount];
        memcpy ((void *)pAttachments, (void *)src->pAttachments, sizeof(VkPipelineColorBlendAttachmentState)*src->attachmentCount);
    }
    for (uint32_t i=0; i<4; ++i) {
        blendConstants[i] = src->blendConstants[i];
    }
}

safe_VkPipelineDynamicStateCreateInfo::safe_VkPipelineDynamicStateCreateInfo(const VkPipelineDynamicStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    dynamicStateCount(in_struct->dynamicStateCount),
    pDynamicStates(nullptr)
{
    if (in_struct->pDynamicStates) {
        pDynamicStates = new VkDynamicState[in_struct->dynamicStateCount];
        memcpy ((void *)pDynamicStates, (void *)in_struct->pDynamicStates, sizeof(VkDynamicState)*in_struct->dynamicStateCount);
    }
}

safe_VkPipelineDynamicStateCreateInfo::safe_VkPipelineDynamicStateCreateInfo() :
    pDynamicStates(nullptr)
{}

safe_VkPipelineDynamicStateCreateInfo::safe_VkPipelineDynamicStateCreateInfo(const safe_VkPipelineDynamicStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    dynamicStateCount = src.dynamicStateCount;
    pDynamicStates = nullptr;
    if (src.pDynamicStates) {
        pDynamicStates = new VkDynamicState[src.dynamicStateCount];
        memcpy ((void *)pDynamicStates, (void *)src.pDynamicStates, sizeof(VkDynamicState)*src.dynamicStateCount);
    }
}

safe_VkPipelineDynamicStateCreateInfo& safe_VkPipelineDynamicStateCreateInfo::operator=(const safe_VkPipelineDynamicStateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pDynamicStates)
        delete[] pDynamicStates;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    dynamicStateCount = src.dynamicStateCount;
    pDynamicStates = nullptr;
    if (src.pDynamicStates) {
        pDynamicStates = new VkDynamicState[src.dynamicStateCount];
        memcpy ((void *)pDynamicStates, (void *)src.pDynamicStates, sizeof(VkDynamicState)*src.dynamicStateCount);
    }

    return *this;
}

safe_VkPipelineDynamicStateCreateInfo::~safe_VkPipelineDynamicStateCreateInfo()
{
    if (pDynamicStates)
        delete[] pDynamicStates;
}

void safe_VkPipelineDynamicStateCreateInfo::initialize(const VkPipelineDynamicStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    dynamicStateCount = in_struct->dynamicStateCount;
    pDynamicStates = nullptr;
    if (in_struct->pDynamicStates) {
        pDynamicStates = new VkDynamicState[in_struct->dynamicStateCount];
        memcpy ((void *)pDynamicStates, (void *)in_struct->pDynamicStates, sizeof(VkDynamicState)*in_struct->dynamicStateCount);
    }
}

void safe_VkPipelineDynamicStateCreateInfo::initialize(const safe_VkPipelineDynamicStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    dynamicStateCount = src->dynamicStateCount;
    pDynamicStates = nullptr;
    if (src->pDynamicStates) {
        pDynamicStates = new VkDynamicState[src->dynamicStateCount];
        memcpy ((void *)pDynamicStates, (void *)src->pDynamicStates, sizeof(VkDynamicState)*src->dynamicStateCount);
    }
}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment, const bool uses_depthstencil_attachment) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    stageCount(in_struct->stageCount),
    pStages(nullptr),
    layout(in_struct->layout),
    renderPass(in_struct->renderPass),
    subpass(in_struct->subpass),
    basePipelineHandle(in_struct->basePipelineHandle),
    basePipelineIndex(in_struct->basePipelineIndex)
{
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    else
        pVertexInputState = NULL;
    if (in_struct->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);
    else
        pInputAssemblyState = NULL;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i=0; i<stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (in_struct->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
    else
        pTessellationState = NULL; // original pTessellationState pointer ignored
    bool has_rasterization = in_struct->pRasterizationState ? !in_struct->pRasterizationState->rasterizerDiscardEnable : false;
    if (in_struct->pViewportState && has_rasterization) {
        bool is_dynamic_viewports = false;
        bool is_dynamic_scissors = false;
        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT)
                    is_dynamic_viewports = true;
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR)
                    is_dynamic_scissors = true;
        }
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);
    } else
        pViewportState = NULL; // original pViewportState pointer ignored
    if (in_struct->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);
    else
        pRasterizationState = NULL;
    if (in_struct->pMultisampleState && has_rasterization)
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);
    else
        pMultisampleState = NULL; // original pMultisampleState pointer ignored
    // needs a tracked subpass state uses_depthstencil_attachment
    if (in_struct->pDepthStencilState && has_rasterization && uses_depthstencil_attachment)
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);
    else
        pDepthStencilState = NULL; // original pDepthStencilState pointer ignored
    // needs a tracked subpass state usesColorAttachment
    if (in_struct->pColorBlendState && has_rasterization && uses_color_attachment)
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);
    else
        pColorBlendState = NULL; // original pColorBlendState pointer ignored
    if (in_struct->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
    else
        pDynamicState = NULL;
}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo() :
    pStages(nullptr)
{}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo(const safe_VkGraphicsPipelineCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stageCount = src.stageCount;
    pStages = nullptr;
    layout = src.layout;
    renderPass = src.renderPass;
    subpass = src.subpass;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;
    if (stageCount && src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src.pStages[i]);
        }
    }
    if (src.pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*src.pVertexInputState);
    else
        pVertexInputState = NULL;
    if (src.pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*src.pInputAssemblyState);
    else
        pInputAssemblyState = NULL;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i=0; i<stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (src.pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*src.pTessellationState);
    else
        pTessellationState = NULL; // original pTessellationState pointer ignored
    bool has_rasterization = src.pRasterizationState ? !src.pRasterizationState->rasterizerDiscardEnable : false;
    if (src.pViewportState && has_rasterization) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*src.pViewportState);
    } else
        pViewportState = NULL; // original pViewportState pointer ignored
    if (src.pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*src.pRasterizationState);
    else
        pRasterizationState = NULL;
    if (src.pMultisampleState && has_rasterization)
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*src.pMultisampleState);
    else
        pMultisampleState = NULL; // original pMultisampleState pointer ignored
    if (src.pDepthStencilState && has_rasterization)
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*src.pDepthStencilState);
    else
        pDepthStencilState = NULL; // original pDepthStencilState pointer ignored
    if (src.pColorBlendState && has_rasterization)
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*src.pColorBlendState);
    else
        pColorBlendState = NULL; // original pColorBlendState pointer ignored
    if (src.pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*src.pDynamicState);
    else
        pDynamicState = NULL;
}

safe_VkGraphicsPipelineCreateInfo& safe_VkGraphicsPipelineCreateInfo::operator=(const safe_VkGraphicsPipelineCreateInfo& src)
{
    if (&src == this) return *this;

    if (pStages)
        delete[] pStages;
    if (pVertexInputState)
        delete pVertexInputState;
    if (pInputAssemblyState)
        delete pInputAssemblyState;
    if (pTessellationState)
        delete pTessellationState;
    if (pViewportState)
        delete pViewportState;
    if (pRasterizationState)
        delete pRasterizationState;
    if (pMultisampleState)
        delete pMultisampleState;
    if (pDepthStencilState)
        delete pDepthStencilState;
    if (pColorBlendState)
        delete pColorBlendState;
    if (pDynamicState)
        delete pDynamicState;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stageCount = src.stageCount;
    pStages = nullptr;
    layout = src.layout;
    renderPass = src.renderPass;
    subpass = src.subpass;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;
    if (stageCount && src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src.pStages[i]);
        }
    }
    if (src.pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*src.pVertexInputState);
    else
        pVertexInputState = NULL;
    if (src.pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*src.pInputAssemblyState);
    else
        pInputAssemblyState = NULL;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i=0; i<stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (src.pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*src.pTessellationState);
    else
        pTessellationState = NULL; // original pTessellationState pointer ignored
    bool has_rasterization = src.pRasterizationState ? !src.pRasterizationState->rasterizerDiscardEnable : false;
    if (src.pViewportState && has_rasterization) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*src.pViewportState);
    } else
        pViewportState = NULL; // original pViewportState pointer ignored
    if (src.pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*src.pRasterizationState);
    else
        pRasterizationState = NULL;
    if (src.pMultisampleState && has_rasterization)
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*src.pMultisampleState);
    else
        pMultisampleState = NULL; // original pMultisampleState pointer ignored
    if (src.pDepthStencilState && has_rasterization)
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*src.pDepthStencilState);
    else
        pDepthStencilState = NULL; // original pDepthStencilState pointer ignored
    if (src.pColorBlendState && has_rasterization)
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*src.pColorBlendState);
    else
        pColorBlendState = NULL; // original pColorBlendState pointer ignored
    if (src.pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*src.pDynamicState);
    else
        pDynamicState = NULL;

    return *this;
}

safe_VkGraphicsPipelineCreateInfo::~safe_VkGraphicsPipelineCreateInfo()
{
    if (pStages)
        delete[] pStages;
    if (pVertexInputState)
        delete pVertexInputState;
    if (pInputAssemblyState)
        delete pInputAssemblyState;
    if (pTessellationState)
        delete pTessellationState;
    if (pViewportState)
        delete pViewportState;
    if (pRasterizationState)
        delete pRasterizationState;
    if (pMultisampleState)
        delete pMultisampleState;
    if (pDepthStencilState)
        delete pDepthStencilState;
    if (pColorBlendState)
        delete pColorBlendState;
    if (pDynamicState)
        delete pDynamicState;
}

void safe_VkGraphicsPipelineCreateInfo::initialize(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment, const bool uses_depthstencil_attachment)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    layout = in_struct->layout;
    renderPass = in_struct->renderPass;
    subpass = in_struct->subpass;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    else
        pVertexInputState = NULL;
    if (in_struct->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);
    else
        pInputAssemblyState = NULL;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i=0; i<stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (in_struct->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
    else
        pTessellationState = NULL; // original pTessellationState pointer ignored
    bool has_rasterization = in_struct->pRasterizationState ? !in_struct->pRasterizationState->rasterizerDiscardEnable : false;
    if (in_struct->pViewportState && has_rasterization) {
        bool is_dynamic_viewports = false;
        bool is_dynamic_scissors = false;
        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT)
                    is_dynamic_viewports = true;
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR)
                    is_dynamic_scissors = true;
        }
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);
    } else
        pViewportState = NULL; // original pViewportState pointer ignored
    if (in_struct->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);
    else
        pRasterizationState = NULL;
    if (in_struct->pMultisampleState && has_rasterization)
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);
    else
        pMultisampleState = NULL; // original pMultisampleState pointer ignored
    // needs a tracked subpass state uses_depthstencil_attachment
    if (in_struct->pDepthStencilState && has_rasterization && uses_depthstencil_attachment)
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);
    else
        pDepthStencilState = NULL; // original pDepthStencilState pointer ignored
    // needs a tracked subpass state usesColorAttachment
    if (in_struct->pColorBlendState && has_rasterization && uses_color_attachment)
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);
    else
        pColorBlendState = NULL; // original pColorBlendState pointer ignored
    if (in_struct->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
    else
        pDynamicState = NULL;
}

void safe_VkGraphicsPipelineCreateInfo::initialize(const safe_VkGraphicsPipelineCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    stageCount = src->stageCount;
    pStages = nullptr;
    layout = src->layout;
    renderPass = src->renderPass;
    subpass = src->subpass;
    basePipelineHandle = src->basePipelineHandle;
    basePipelineIndex = src->basePipelineIndex;
    if (stageCount && src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src->pStages[i]);
        }
    }
    if (src->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*src->pVertexInputState);
    else
        pVertexInputState = NULL;
    if (src->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*src->pInputAssemblyState);
    else
        pInputAssemblyState = NULL;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i=0; i<stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (src->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*src->pTessellationState);
    else
        pTessellationState = NULL; // original pTessellationState pointer ignored
    bool has_rasterization = src->pRasterizationState ? !src->pRasterizationState->rasterizerDiscardEnable : false;
    if (src->pViewportState && has_rasterization) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*src->pViewportState);
    } else
        pViewportState = NULL; // original pViewportState pointer ignored
    if (src->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*src->pRasterizationState);
    else
        pRasterizationState = NULL;
    if (src->pMultisampleState && has_rasterization)
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*src->pMultisampleState);
    else
        pMultisampleState = NULL; // original pMultisampleState pointer ignored
    if (src->pDepthStencilState && has_rasterization)
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*src->pDepthStencilState);
    else
        pDepthStencilState = NULL; // original pDepthStencilState pointer ignored
    if (src->pColorBlendState && has_rasterization)
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*src->pColorBlendState);
    else
        pColorBlendState = NULL; // original pColorBlendState pointer ignored
    if (src->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*src->pDynamicState);
    else
        pDynamicState = NULL;
}

safe_VkComputePipelineCreateInfo::safe_VkComputePipelineCreateInfo(const VkComputePipelineCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    stage(&in_struct->stage),
    layout(in_struct->layout),
    basePipelineHandle(in_struct->basePipelineHandle),
    basePipelineIndex(in_struct->basePipelineIndex)
{
}

safe_VkComputePipelineCreateInfo::safe_VkComputePipelineCreateInfo()
{}

safe_VkComputePipelineCreateInfo::safe_VkComputePipelineCreateInfo(const safe_VkComputePipelineCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stage.initialize(&src.stage);
    layout = src.layout;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;
}

safe_VkComputePipelineCreateInfo& safe_VkComputePipelineCreateInfo::operator=(const safe_VkComputePipelineCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stage.initialize(&src.stage);
    layout = src.layout;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;

    return *this;
}

safe_VkComputePipelineCreateInfo::~safe_VkComputePipelineCreateInfo()
{
}

void safe_VkComputePipelineCreateInfo::initialize(const VkComputePipelineCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    stage.initialize(&in_struct->stage);
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
}

void safe_VkComputePipelineCreateInfo::initialize(const safe_VkComputePipelineCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    stage.initialize(&src->stage);
    layout = src->layout;
    basePipelineHandle = src->basePipelineHandle;
    basePipelineIndex = src->basePipelineIndex;
}

safe_VkPipelineLayoutCreateInfo::safe_VkPipelineLayoutCreateInfo(const VkPipelineLayoutCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    setLayoutCount(in_struct->setLayoutCount),
    pSetLayouts(nullptr),
    pushConstantRangeCount(in_struct->pushConstantRangeCount),
    pPushConstantRanges(nullptr)
{
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i=0; i<setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy ((void *)pPushConstantRanges, (void *)in_struct->pPushConstantRanges, sizeof(VkPushConstantRange)*in_struct->pushConstantRangeCount);
    }
}

safe_VkPipelineLayoutCreateInfo::safe_VkPipelineLayoutCreateInfo() :
    pSetLayouts(nullptr),
    pPushConstantRanges(nullptr)
{}

safe_VkPipelineLayoutCreateInfo::safe_VkPipelineLayoutCreateInfo(const safe_VkPipelineLayoutCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    setLayoutCount = src.setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    if (setLayoutCount && src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i=0; i<setLayoutCount; ++i) {
            pSetLayouts[i] = src.pSetLayouts[i];
        }
    }
    if (src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[src.pushConstantRangeCount];
        memcpy ((void *)pPushConstantRanges, (void *)src.pPushConstantRanges, sizeof(VkPushConstantRange)*src.pushConstantRangeCount);
    }
}

safe_VkPipelineLayoutCreateInfo& safe_VkPipelineLayoutCreateInfo::operator=(const safe_VkPipelineLayoutCreateInfo& src)
{
    if (&src == this) return *this;

    if (pSetLayouts)
        delete[] pSetLayouts;
    if (pPushConstantRanges)
        delete[] pPushConstantRanges;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    setLayoutCount = src.setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    if (setLayoutCount && src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i=0; i<setLayoutCount; ++i) {
            pSetLayouts[i] = src.pSetLayouts[i];
        }
    }
    if (src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[src.pushConstantRangeCount];
        memcpy ((void *)pPushConstantRanges, (void *)src.pPushConstantRanges, sizeof(VkPushConstantRange)*src.pushConstantRangeCount);
    }

    return *this;
}

safe_VkPipelineLayoutCreateInfo::~safe_VkPipelineLayoutCreateInfo()
{
    if (pSetLayouts)
        delete[] pSetLayouts;
    if (pPushConstantRanges)
        delete[] pPushConstantRanges;
}

void safe_VkPipelineLayoutCreateInfo::initialize(const VkPipelineLayoutCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    setLayoutCount = in_struct->setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = in_struct->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i=0; i<setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy ((void *)pPushConstantRanges, (void *)in_struct->pPushConstantRanges, sizeof(VkPushConstantRange)*in_struct->pushConstantRangeCount);
    }
}

void safe_VkPipelineLayoutCreateInfo::initialize(const safe_VkPipelineLayoutCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    setLayoutCount = src->setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = src->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    if (setLayoutCount && src->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i=0; i<setLayoutCount; ++i) {
            pSetLayouts[i] = src->pSetLayouts[i];
        }
    }
    if (src->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[src->pushConstantRangeCount];
        memcpy ((void *)pPushConstantRanges, (void *)src->pPushConstantRanges, sizeof(VkPushConstantRange)*src->pushConstantRangeCount);
    }
}

safe_VkSamplerCreateInfo::safe_VkSamplerCreateInfo(const VkSamplerCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    magFilter(in_struct->magFilter),
    minFilter(in_struct->minFilter),
    mipmapMode(in_struct->mipmapMode),
    addressModeU(in_struct->addressModeU),
    addressModeV(in_struct->addressModeV),
    addressModeW(in_struct->addressModeW),
    mipLodBias(in_struct->mipLodBias),
    anisotropyEnable(in_struct->anisotropyEnable),
    maxAnisotropy(in_struct->maxAnisotropy),
    compareEnable(in_struct->compareEnable),
    compareOp(in_struct->compareOp),
    minLod(in_struct->minLod),
    maxLod(in_struct->maxLod),
    borderColor(in_struct->borderColor),
    unnormalizedCoordinates(in_struct->unnormalizedCoordinates)
{
}

safe_VkSamplerCreateInfo::safe_VkSamplerCreateInfo()
{}

safe_VkSamplerCreateInfo::safe_VkSamplerCreateInfo(const safe_VkSamplerCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    magFilter = src.magFilter;
    minFilter = src.minFilter;
    mipmapMode = src.mipmapMode;
    addressModeU = src.addressModeU;
    addressModeV = src.addressModeV;
    addressModeW = src.addressModeW;
    mipLodBias = src.mipLodBias;
    anisotropyEnable = src.anisotropyEnable;
    maxAnisotropy = src.maxAnisotropy;
    compareEnable = src.compareEnable;
    compareOp = src.compareOp;
    minLod = src.minLod;
    maxLod = src.maxLod;
    borderColor = src.borderColor;
    unnormalizedCoordinates = src.unnormalizedCoordinates;
}

safe_VkSamplerCreateInfo& safe_VkSamplerCreateInfo::operator=(const safe_VkSamplerCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    magFilter = src.magFilter;
    minFilter = src.minFilter;
    mipmapMode = src.mipmapMode;
    addressModeU = src.addressModeU;
    addressModeV = src.addressModeV;
    addressModeW = src.addressModeW;
    mipLodBias = src.mipLodBias;
    anisotropyEnable = src.anisotropyEnable;
    maxAnisotropy = src.maxAnisotropy;
    compareEnable = src.compareEnable;
    compareOp = src.compareOp;
    minLod = src.minLod;
    maxLod = src.maxLod;
    borderColor = src.borderColor;
    unnormalizedCoordinates = src.unnormalizedCoordinates;

    return *this;
}

safe_VkSamplerCreateInfo::~safe_VkSamplerCreateInfo()
{
}

void safe_VkSamplerCreateInfo::initialize(const VkSamplerCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    magFilter = in_struct->magFilter;
    minFilter = in_struct->minFilter;
    mipmapMode = in_struct->mipmapMode;
    addressModeU = in_struct->addressModeU;
    addressModeV = in_struct->addressModeV;
    addressModeW = in_struct->addressModeW;
    mipLodBias = in_struct->mipLodBias;
    anisotropyEnable = in_struct->anisotropyEnable;
    maxAnisotropy = in_struct->maxAnisotropy;
    compareEnable = in_struct->compareEnable;
    compareOp = in_struct->compareOp;
    minLod = in_struct->minLod;
    maxLod = in_struct->maxLod;
    borderColor = in_struct->borderColor;
    unnormalizedCoordinates = in_struct->unnormalizedCoordinates;
}

void safe_VkSamplerCreateInfo::initialize(const safe_VkSamplerCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    magFilter = src->magFilter;
    minFilter = src->minFilter;
    mipmapMode = src->mipmapMode;
    addressModeU = src->addressModeU;
    addressModeV = src->addressModeV;
    addressModeW = src->addressModeW;
    mipLodBias = src->mipLodBias;
    anisotropyEnable = src->anisotropyEnable;
    maxAnisotropy = src->maxAnisotropy;
    compareEnable = src->compareEnable;
    compareOp = src->compareOp;
    minLod = src->minLod;
    maxLod = src->maxLod;
    borderColor = src->borderColor;
    unnormalizedCoordinates = src->unnormalizedCoordinates;
}

safe_VkDescriptorSetLayoutBinding::safe_VkDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding* in_struct) :
    binding(in_struct->binding),
    descriptorType(in_struct->descriptorType),
    descriptorCount(in_struct->descriptorCount),
    stageFlags(in_struct->stageFlags),
    pImmutableSamplers(nullptr)
{
    const bool sampler_type = in_struct->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || in_struct->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (descriptorCount && in_struct->pImmutableSamplers && sampler_type) {
        pImmutableSamplers = new VkSampler[descriptorCount];
        for (uint32_t i=0; i<descriptorCount; ++i) {
            pImmutableSamplers[i] = in_struct->pImmutableSamplers[i];
        }
    }
}

safe_VkDescriptorSetLayoutBinding::safe_VkDescriptorSetLayoutBinding() :
    pImmutableSamplers(nullptr)
{}

safe_VkDescriptorSetLayoutBinding::safe_VkDescriptorSetLayoutBinding(const safe_VkDescriptorSetLayoutBinding& src)
{
    binding = src.binding;
    descriptorType = src.descriptorType;
    descriptorCount = src.descriptorCount;
    stageFlags = src.stageFlags;
    pImmutableSamplers = nullptr;
    const bool sampler_type = src.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || src.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (descriptorCount && src.pImmutableSamplers && sampler_type) {
        pImmutableSamplers = new VkSampler[descriptorCount];
        for (uint32_t i=0; i<descriptorCount; ++i) {
            pImmutableSamplers[i] = src.pImmutableSamplers[i];
        }
    }
}

safe_VkDescriptorSetLayoutBinding& safe_VkDescriptorSetLayoutBinding::operator=(const safe_VkDescriptorSetLayoutBinding& src)
{
    if (&src == this) return *this;

    if (pImmutableSamplers)
        delete[] pImmutableSamplers;

    binding = src.binding;
    descriptorType = src.descriptorType;
    descriptorCount = src.descriptorCount;
    stageFlags = src.stageFlags;
    pImmutableSamplers = nullptr;
    const bool sampler_type = src.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || src.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (descriptorCount && src.pImmutableSamplers && sampler_type) {
        pImmutableSamplers = new VkSampler[descriptorCount];
        for (uint32_t i=0; i<descriptorCount; ++i) {
            pImmutableSamplers[i] = src.pImmutableSamplers[i];
        }
    }

    return *this;
}

safe_VkDescriptorSetLayoutBinding::~safe_VkDescriptorSetLayoutBinding()
{
    if (pImmutableSamplers)
        delete[] pImmutableSamplers;
}

void safe_VkDescriptorSetLayoutBinding::initialize(const VkDescriptorSetLayoutBinding* in_struct)
{
    binding = in_struct->binding;
    descriptorType = in_struct->descriptorType;
    descriptorCount = in_struct->descriptorCount;
    stageFlags = in_struct->stageFlags;
    pImmutableSamplers = nullptr;
    const bool sampler_type = in_struct->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || in_struct->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (descriptorCount && in_struct->pImmutableSamplers && sampler_type) {
        pImmutableSamplers = new VkSampler[descriptorCount];
        for (uint32_t i=0; i<descriptorCount; ++i) {
            pImmutableSamplers[i] = in_struct->pImmutableSamplers[i];
        }
    }
}

void safe_VkDescriptorSetLayoutBinding::initialize(const safe_VkDescriptorSetLayoutBinding* src)
{
    binding = src->binding;
    descriptorType = src->descriptorType;
    descriptorCount = src->descriptorCount;
    stageFlags = src->stageFlags;
    pImmutableSamplers = nullptr;
    const bool sampler_type = src->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || src->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (descriptorCount && src->pImmutableSamplers && sampler_type) {
        pImmutableSamplers = new VkSampler[descriptorCount];
        for (uint32_t i=0; i<descriptorCount; ++i) {
            pImmutableSamplers[i] = src->pImmutableSamplers[i];
        }
    }
}

safe_VkDescriptorSetLayoutCreateInfo::safe_VkDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    bindingCount(in_struct->bindingCount),
    pBindings(nullptr)
{
    if (bindingCount && in_struct->pBindings) {
        pBindings = new safe_VkDescriptorSetLayoutBinding[bindingCount];
        for (uint32_t i=0; i<bindingCount; ++i) {
            pBindings[i].initialize(&in_struct->pBindings[i]);
        }
    }
}

safe_VkDescriptorSetLayoutCreateInfo::safe_VkDescriptorSetLayoutCreateInfo() :
    pBindings(nullptr)
{}

safe_VkDescriptorSetLayoutCreateInfo::safe_VkDescriptorSetLayoutCreateInfo(const safe_VkDescriptorSetLayoutCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    bindingCount = src.bindingCount;
    pBindings = nullptr;
    if (bindingCount && src.pBindings) {
        pBindings = new safe_VkDescriptorSetLayoutBinding[bindingCount];
        for (uint32_t i=0; i<bindingCount; ++i) {
            pBindings[i].initialize(&src.pBindings[i]);
        }
    }
}

safe_VkDescriptorSetLayoutCreateInfo& safe_VkDescriptorSetLayoutCreateInfo::operator=(const safe_VkDescriptorSetLayoutCreateInfo& src)
{
    if (&src == this) return *this;

    if (pBindings)
        delete[] pBindings;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    bindingCount = src.bindingCount;
    pBindings = nullptr;
    if (bindingCount && src.pBindings) {
        pBindings = new safe_VkDescriptorSetLayoutBinding[bindingCount];
        for (uint32_t i=0; i<bindingCount; ++i) {
            pBindings[i].initialize(&src.pBindings[i]);
        }
    }

    return *this;
}

safe_VkDescriptorSetLayoutCreateInfo::~safe_VkDescriptorSetLayoutCreateInfo()
{
    if (pBindings)
        delete[] pBindings;
}

void safe_VkDescriptorSetLayoutCreateInfo::initialize(const VkDescriptorSetLayoutCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    bindingCount = in_struct->bindingCount;
    pBindings = nullptr;
    if (bindingCount && in_struct->pBindings) {
        pBindings = new safe_VkDescriptorSetLayoutBinding[bindingCount];
        for (uint32_t i=0; i<bindingCount; ++i) {
            pBindings[i].initialize(&in_struct->pBindings[i]);
        }
    }
}

void safe_VkDescriptorSetLayoutCreateInfo::initialize(const safe_VkDescriptorSetLayoutCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    bindingCount = src->bindingCount;
    pBindings = nullptr;
    if (bindingCount && src->pBindings) {
        pBindings = new safe_VkDescriptorSetLayoutBinding[bindingCount];
        for (uint32_t i=0; i<bindingCount; ++i) {
            pBindings[i].initialize(&src->pBindings[i]);
        }
    }
}

safe_VkDescriptorPoolCreateInfo::safe_VkDescriptorPoolCreateInfo(const VkDescriptorPoolCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    maxSets(in_struct->maxSets),
    poolSizeCount(in_struct->poolSizeCount),
    pPoolSizes(nullptr)
{
    if (in_struct->pPoolSizes) {
        pPoolSizes = new VkDescriptorPoolSize[in_struct->poolSizeCount];
        memcpy ((void *)pPoolSizes, (void *)in_struct->pPoolSizes, sizeof(VkDescriptorPoolSize)*in_struct->poolSizeCount);
    }
}

safe_VkDescriptorPoolCreateInfo::safe_VkDescriptorPoolCreateInfo() :
    pPoolSizes(nullptr)
{}

safe_VkDescriptorPoolCreateInfo::safe_VkDescriptorPoolCreateInfo(const safe_VkDescriptorPoolCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    maxSets = src.maxSets;
    poolSizeCount = src.poolSizeCount;
    pPoolSizes = nullptr;
    if (src.pPoolSizes) {
        pPoolSizes = new VkDescriptorPoolSize[src.poolSizeCount];
        memcpy ((void *)pPoolSizes, (void *)src.pPoolSizes, sizeof(VkDescriptorPoolSize)*src.poolSizeCount);
    }
}

safe_VkDescriptorPoolCreateInfo& safe_VkDescriptorPoolCreateInfo::operator=(const safe_VkDescriptorPoolCreateInfo& src)
{
    if (&src == this) return *this;

    if (pPoolSizes)
        delete[] pPoolSizes;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    maxSets = src.maxSets;
    poolSizeCount = src.poolSizeCount;
    pPoolSizes = nullptr;
    if (src.pPoolSizes) {
        pPoolSizes = new VkDescriptorPoolSize[src.poolSizeCount];
        memcpy ((void *)pPoolSizes, (void *)src.pPoolSizes, sizeof(VkDescriptorPoolSize)*src.poolSizeCount);
    }

    return *this;
}

safe_VkDescriptorPoolCreateInfo::~safe_VkDescriptorPoolCreateInfo()
{
    if (pPoolSizes)
        delete[] pPoolSizes;
}

void safe_VkDescriptorPoolCreateInfo::initialize(const VkDescriptorPoolCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    maxSets = in_struct->maxSets;
    poolSizeCount = in_struct->poolSizeCount;
    pPoolSizes = nullptr;
    if (in_struct->pPoolSizes) {
        pPoolSizes = new VkDescriptorPoolSize[in_struct->poolSizeCount];
        memcpy ((void *)pPoolSizes, (void *)in_struct->pPoolSizes, sizeof(VkDescriptorPoolSize)*in_struct->poolSizeCount);
    }
}

void safe_VkDescriptorPoolCreateInfo::initialize(const safe_VkDescriptorPoolCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    maxSets = src->maxSets;
    poolSizeCount = src->poolSizeCount;
    pPoolSizes = nullptr;
    if (src->pPoolSizes) {
        pPoolSizes = new VkDescriptorPoolSize[src->poolSizeCount];
        memcpy ((void *)pPoolSizes, (void *)src->pPoolSizes, sizeof(VkDescriptorPoolSize)*src->poolSizeCount);
    }
}

safe_VkDescriptorSetAllocateInfo::safe_VkDescriptorSetAllocateInfo(const VkDescriptorSetAllocateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    descriptorPool(in_struct->descriptorPool),
    descriptorSetCount(in_struct->descriptorSetCount),
    pSetLayouts(nullptr)
{
    if (descriptorSetCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
        for (uint32_t i=0; i<descriptorSetCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
}

safe_VkDescriptorSetAllocateInfo::safe_VkDescriptorSetAllocateInfo() :
    pSetLayouts(nullptr)
{}

safe_VkDescriptorSetAllocateInfo::safe_VkDescriptorSetAllocateInfo(const safe_VkDescriptorSetAllocateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    descriptorPool = src.descriptorPool;
    descriptorSetCount = src.descriptorSetCount;
    pSetLayouts = nullptr;
    if (descriptorSetCount && src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
        for (uint32_t i=0; i<descriptorSetCount; ++i) {
            pSetLayouts[i] = src.pSetLayouts[i];
        }
    }
}

safe_VkDescriptorSetAllocateInfo& safe_VkDescriptorSetAllocateInfo::operator=(const safe_VkDescriptorSetAllocateInfo& src)
{
    if (&src == this) return *this;

    if (pSetLayouts)
        delete[] pSetLayouts;

    sType = src.sType;
    pNext = src.pNext;
    descriptorPool = src.descriptorPool;
    descriptorSetCount = src.descriptorSetCount;
    pSetLayouts = nullptr;
    if (descriptorSetCount && src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
        for (uint32_t i=0; i<descriptorSetCount; ++i) {
            pSetLayouts[i] = src.pSetLayouts[i];
        }
    }

    return *this;
}

safe_VkDescriptorSetAllocateInfo::~safe_VkDescriptorSetAllocateInfo()
{
    if (pSetLayouts)
        delete[] pSetLayouts;
}

void safe_VkDescriptorSetAllocateInfo::initialize(const VkDescriptorSetAllocateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    descriptorPool = in_struct->descriptorPool;
    descriptorSetCount = in_struct->descriptorSetCount;
    pSetLayouts = nullptr;
    if (descriptorSetCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
        for (uint32_t i=0; i<descriptorSetCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
}

void safe_VkDescriptorSetAllocateInfo::initialize(const safe_VkDescriptorSetAllocateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    descriptorPool = src->descriptorPool;
    descriptorSetCount = src->descriptorSetCount;
    pSetLayouts = nullptr;
    if (descriptorSetCount && src->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
        for (uint32_t i=0; i<descriptorSetCount; ++i) {
            pSetLayouts[i] = src->pSetLayouts[i];
        }
    }
}

safe_VkWriteDescriptorSet::safe_VkWriteDescriptorSet(const VkWriteDescriptorSet* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    dstSet(in_struct->dstSet),
    dstBinding(in_struct->dstBinding),
    dstArrayElement(in_struct->dstArrayElement),
    descriptorCount(in_struct->descriptorCount),
    descriptorType(in_struct->descriptorType),
    pImageInfo(nullptr),
    pBufferInfo(nullptr),
    pTexelBufferView(nullptr)
{
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if (descriptorCount && in_struct->pImageInfo) {
            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pImageInfo[i] = in_struct->pImageInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        if (descriptorCount && in_struct->pBufferInfo) {
            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pBufferInfo[i] = in_struct->pBufferInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        if (descriptorCount && in_struct->pTexelBufferView) {
            pTexelBufferView = new VkBufferView[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pTexelBufferView[i] = in_struct->pTexelBufferView[i];
            }
        }
        break;
        default:
        break;
    }
}

safe_VkWriteDescriptorSet::safe_VkWriteDescriptorSet() :
    pImageInfo(nullptr),
    pBufferInfo(nullptr),
    pTexelBufferView(nullptr)
{}

safe_VkWriteDescriptorSet::safe_VkWriteDescriptorSet(const safe_VkWriteDescriptorSet& src)
{
    sType = src.sType;
    pNext = src.pNext;
    dstSet = src.dstSet;
    dstBinding = src.dstBinding;
    dstArrayElement = src.dstArrayElement;
    descriptorCount = src.descriptorCount;
    descriptorType = src.descriptorType;
    pImageInfo = nullptr;
    pBufferInfo = nullptr;
    pTexelBufferView = nullptr;
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if (descriptorCount && src.pImageInfo) {
            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pImageInfo[i] = src.pImageInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        if (descriptorCount && src.pBufferInfo) {
            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pBufferInfo[i] = src.pBufferInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        if (descriptorCount && src.pTexelBufferView) {
            pTexelBufferView = new VkBufferView[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pTexelBufferView[i] = src.pTexelBufferView[i];
            }
        }
        break;
        default:
        break;
    }
}

safe_VkWriteDescriptorSet& safe_VkWriteDescriptorSet::operator=(const safe_VkWriteDescriptorSet& src)
{
    if (&src == this) return *this;

    if (pImageInfo)
        delete[] pImageInfo;
    if (pBufferInfo)
        delete[] pBufferInfo;
    if (pTexelBufferView)
        delete[] pTexelBufferView;

    sType = src.sType;
    pNext = src.pNext;
    dstSet = src.dstSet;
    dstBinding = src.dstBinding;
    dstArrayElement = src.dstArrayElement;
    descriptorCount = src.descriptorCount;
    descriptorType = src.descriptorType;
    pImageInfo = nullptr;
    pBufferInfo = nullptr;
    pTexelBufferView = nullptr;
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if (descriptorCount && src.pImageInfo) {
            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pImageInfo[i] = src.pImageInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        if (descriptorCount && src.pBufferInfo) {
            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pBufferInfo[i] = src.pBufferInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        if (descriptorCount && src.pTexelBufferView) {
            pTexelBufferView = new VkBufferView[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pTexelBufferView[i] = src.pTexelBufferView[i];
            }
        }
        break;
        default:
        break;
    }

    return *this;
}

safe_VkWriteDescriptorSet::~safe_VkWriteDescriptorSet()
{
    if (pImageInfo)
        delete[] pImageInfo;
    if (pBufferInfo)
        delete[] pBufferInfo;
    if (pTexelBufferView)
        delete[] pTexelBufferView;
}

void safe_VkWriteDescriptorSet::initialize(const VkWriteDescriptorSet* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    dstSet = in_struct->dstSet;
    dstBinding = in_struct->dstBinding;
    dstArrayElement = in_struct->dstArrayElement;
    descriptorCount = in_struct->descriptorCount;
    descriptorType = in_struct->descriptorType;
    pImageInfo = nullptr;
    pBufferInfo = nullptr;
    pTexelBufferView = nullptr;
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if (descriptorCount && in_struct->pImageInfo) {
            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pImageInfo[i] = in_struct->pImageInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        if (descriptorCount && in_struct->pBufferInfo) {
            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pBufferInfo[i] = in_struct->pBufferInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        if (descriptorCount && in_struct->pTexelBufferView) {
            pTexelBufferView = new VkBufferView[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pTexelBufferView[i] = in_struct->pTexelBufferView[i];
            }
        }
        break;
        default:
        break;
    }
}

void safe_VkWriteDescriptorSet::initialize(const safe_VkWriteDescriptorSet* src)
{
    sType = src->sType;
    pNext = src->pNext;
    dstSet = src->dstSet;
    dstBinding = src->dstBinding;
    dstArrayElement = src->dstArrayElement;
    descriptorCount = src->descriptorCount;
    descriptorType = src->descriptorType;
    pImageInfo = nullptr;
    pBufferInfo = nullptr;
    pTexelBufferView = nullptr;
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if (descriptorCount && src->pImageInfo) {
            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pImageInfo[i] = src->pImageInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        if (descriptorCount && src->pBufferInfo) {
            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pBufferInfo[i] = src->pBufferInfo[i];
            }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        if (descriptorCount && src->pTexelBufferView) {
            pTexelBufferView = new VkBufferView[descriptorCount];
            for (uint32_t i=0; i<descriptorCount; ++i) {
                pTexelBufferView[i] = src->pTexelBufferView[i];
            }
        }
        break;
        default:
        break;
    }
}

safe_VkCopyDescriptorSet::safe_VkCopyDescriptorSet(const VkCopyDescriptorSet* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcSet(in_struct->srcSet),
    srcBinding(in_struct->srcBinding),
    srcArrayElement(in_struct->srcArrayElement),
    dstSet(in_struct->dstSet),
    dstBinding(in_struct->dstBinding),
    dstArrayElement(in_struct->dstArrayElement),
    descriptorCount(in_struct->descriptorCount)
{
}

safe_VkCopyDescriptorSet::safe_VkCopyDescriptorSet()
{}

safe_VkCopyDescriptorSet::safe_VkCopyDescriptorSet(const safe_VkCopyDescriptorSet& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcSet = src.srcSet;
    srcBinding = src.srcBinding;
    srcArrayElement = src.srcArrayElement;
    dstSet = src.dstSet;
    dstBinding = src.dstBinding;
    dstArrayElement = src.dstArrayElement;
    descriptorCount = src.descriptorCount;
}

safe_VkCopyDescriptorSet& safe_VkCopyDescriptorSet::operator=(const safe_VkCopyDescriptorSet& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcSet = src.srcSet;
    srcBinding = src.srcBinding;
    srcArrayElement = src.srcArrayElement;
    dstSet = src.dstSet;
    dstBinding = src.dstBinding;
    dstArrayElement = src.dstArrayElement;
    descriptorCount = src.descriptorCount;

    return *this;
}

safe_VkCopyDescriptorSet::~safe_VkCopyDescriptorSet()
{
}

void safe_VkCopyDescriptorSet::initialize(const VkCopyDescriptorSet* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcSet = in_struct->srcSet;
    srcBinding = in_struct->srcBinding;
    srcArrayElement = in_struct->srcArrayElement;
    dstSet = in_struct->dstSet;
    dstBinding = in_struct->dstBinding;
    dstArrayElement = in_struct->dstArrayElement;
    descriptorCount = in_struct->descriptorCount;
}

void safe_VkCopyDescriptorSet::initialize(const safe_VkCopyDescriptorSet* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcSet = src->srcSet;
    srcBinding = src->srcBinding;
    srcArrayElement = src->srcArrayElement;
    dstSet = src->dstSet;
    dstBinding = src->dstBinding;
    dstArrayElement = src->dstArrayElement;
    descriptorCount = src->descriptorCount;
}

safe_VkFramebufferCreateInfo::safe_VkFramebufferCreateInfo(const VkFramebufferCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    renderPass(in_struct->renderPass),
    attachmentCount(in_struct->attachmentCount),
    pAttachments(nullptr),
    width(in_struct->width),
    height(in_struct->height),
    layers(in_struct->layers)
{
    if (attachmentCount && in_struct->pAttachments) {
        pAttachments = new VkImageView[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i] = in_struct->pAttachments[i];
        }
    }
}

safe_VkFramebufferCreateInfo::safe_VkFramebufferCreateInfo() :
    pAttachments(nullptr)
{}

safe_VkFramebufferCreateInfo::safe_VkFramebufferCreateInfo(const safe_VkFramebufferCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    renderPass = src.renderPass;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    width = src.width;
    height = src.height;
    layers = src.layers;
    if (attachmentCount && src.pAttachments) {
        pAttachments = new VkImageView[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i] = src.pAttachments[i];
        }
    }
}

safe_VkFramebufferCreateInfo& safe_VkFramebufferCreateInfo::operator=(const safe_VkFramebufferCreateInfo& src)
{
    if (&src == this) return *this;

    if (pAttachments)
        delete[] pAttachments;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    renderPass = src.renderPass;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    width = src.width;
    height = src.height;
    layers = src.layers;
    if (attachmentCount && src.pAttachments) {
        pAttachments = new VkImageView[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i] = src.pAttachments[i];
        }
    }

    return *this;
}

safe_VkFramebufferCreateInfo::~safe_VkFramebufferCreateInfo()
{
    if (pAttachments)
        delete[] pAttachments;
}

void safe_VkFramebufferCreateInfo::initialize(const VkFramebufferCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    renderPass = in_struct->renderPass;
    attachmentCount = in_struct->attachmentCount;
    pAttachments = nullptr;
    width = in_struct->width;
    height = in_struct->height;
    layers = in_struct->layers;
    if (attachmentCount && in_struct->pAttachments) {
        pAttachments = new VkImageView[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i] = in_struct->pAttachments[i];
        }
    }
}

void safe_VkFramebufferCreateInfo::initialize(const safe_VkFramebufferCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    renderPass = src->renderPass;
    attachmentCount = src->attachmentCount;
    pAttachments = nullptr;
    width = src->width;
    height = src->height;
    layers = src->layers;
    if (attachmentCount && src->pAttachments) {
        pAttachments = new VkImageView[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i] = src->pAttachments[i];
        }
    }
}

safe_VkSubpassDescription::safe_VkSubpassDescription(const VkSubpassDescription* in_struct) :
    flags(in_struct->flags),
    pipelineBindPoint(in_struct->pipelineBindPoint),
    inputAttachmentCount(in_struct->inputAttachmentCount),
    pInputAttachments(nullptr),
    colorAttachmentCount(in_struct->colorAttachmentCount),
    pColorAttachments(nullptr),
    pResolveAttachments(nullptr),
    pDepthStencilAttachment(nullptr),
    preserveAttachmentCount(in_struct->preserveAttachmentCount),
    pPreserveAttachments(nullptr)
{
    if (in_struct->pInputAttachments) {
        pInputAttachments = new VkAttachmentReference[in_struct->inputAttachmentCount];
        memcpy ((void *)pInputAttachments, (void *)in_struct->pInputAttachments, sizeof(VkAttachmentReference)*in_struct->inputAttachmentCount);
    }
    if (in_struct->pColorAttachments) {
        pColorAttachments = new VkAttachmentReference[in_struct->colorAttachmentCount];
        memcpy ((void *)pColorAttachments, (void *)in_struct->pColorAttachments, sizeof(VkAttachmentReference)*in_struct->colorAttachmentCount);
    }
    if (in_struct->pResolveAttachments) {
        pResolveAttachments = new VkAttachmentReference[in_struct->colorAttachmentCount];
        memcpy ((void *)pResolveAttachments, (void *)in_struct->pResolveAttachments, sizeof(VkAttachmentReference)*in_struct->colorAttachmentCount);
    }
    if (in_struct->pDepthStencilAttachment) {
        pDepthStencilAttachment = new VkAttachmentReference(*in_struct->pDepthStencilAttachment);
    }
    if (in_struct->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[in_struct->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)in_struct->pPreserveAttachments, sizeof(uint32_t)*in_struct->preserveAttachmentCount);
    }
}

safe_VkSubpassDescription::safe_VkSubpassDescription() :
    pInputAttachments(nullptr),
    pColorAttachments(nullptr),
    pResolveAttachments(nullptr),
    pDepthStencilAttachment(nullptr),
    pPreserveAttachments(nullptr)
{}

safe_VkSubpassDescription::safe_VkSubpassDescription(const safe_VkSubpassDescription& src)
{
    flags = src.flags;
    pipelineBindPoint = src.pipelineBindPoint;
    inputAttachmentCount = src.inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src.colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    pDepthStencilAttachment = nullptr;
    preserveAttachmentCount = src.preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (src.pInputAttachments) {
        pInputAttachments = new VkAttachmentReference[src.inputAttachmentCount];
        memcpy ((void *)pInputAttachments, (void *)src.pInputAttachments, sizeof(VkAttachmentReference)*src.inputAttachmentCount);
    }
    if (src.pColorAttachments) {
        pColorAttachments = new VkAttachmentReference[src.colorAttachmentCount];
        memcpy ((void *)pColorAttachments, (void *)src.pColorAttachments, sizeof(VkAttachmentReference)*src.colorAttachmentCount);
    }
    if (src.pResolveAttachments) {
        pResolveAttachments = new VkAttachmentReference[src.colorAttachmentCount];
        memcpy ((void *)pResolveAttachments, (void *)src.pResolveAttachments, sizeof(VkAttachmentReference)*src.colorAttachmentCount);
    }
    if (src.pDepthStencilAttachment) {
        pDepthStencilAttachment = new VkAttachmentReference(*src.pDepthStencilAttachment);
    }
    if (src.pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src.preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src.pPreserveAttachments, sizeof(uint32_t)*src.preserveAttachmentCount);
    }
}

safe_VkSubpassDescription& safe_VkSubpassDescription::operator=(const safe_VkSubpassDescription& src)
{
    if (&src == this) return *this;

    if (pInputAttachments)
        delete[] pInputAttachments;
    if (pColorAttachments)
        delete[] pColorAttachments;
    if (pResolveAttachments)
        delete[] pResolveAttachments;
    if (pDepthStencilAttachment)
        delete pDepthStencilAttachment;
    if (pPreserveAttachments)
        delete[] pPreserveAttachments;

    flags = src.flags;
    pipelineBindPoint = src.pipelineBindPoint;
    inputAttachmentCount = src.inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src.colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    pDepthStencilAttachment = nullptr;
    preserveAttachmentCount = src.preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (src.pInputAttachments) {
        pInputAttachments = new VkAttachmentReference[src.inputAttachmentCount];
        memcpy ((void *)pInputAttachments, (void *)src.pInputAttachments, sizeof(VkAttachmentReference)*src.inputAttachmentCount);
    }
    if (src.pColorAttachments) {
        pColorAttachments = new VkAttachmentReference[src.colorAttachmentCount];
        memcpy ((void *)pColorAttachments, (void *)src.pColorAttachments, sizeof(VkAttachmentReference)*src.colorAttachmentCount);
    }
    if (src.pResolveAttachments) {
        pResolveAttachments = new VkAttachmentReference[src.colorAttachmentCount];
        memcpy ((void *)pResolveAttachments, (void *)src.pResolveAttachments, sizeof(VkAttachmentReference)*src.colorAttachmentCount);
    }
    if (src.pDepthStencilAttachment) {
        pDepthStencilAttachment = new VkAttachmentReference(*src.pDepthStencilAttachment);
    }
    if (src.pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src.preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src.pPreserveAttachments, sizeof(uint32_t)*src.preserveAttachmentCount);
    }

    return *this;
}

safe_VkSubpassDescription::~safe_VkSubpassDescription()
{
    if (pInputAttachments)
        delete[] pInputAttachments;
    if (pColorAttachments)
        delete[] pColorAttachments;
    if (pResolveAttachments)
        delete[] pResolveAttachments;
    if (pDepthStencilAttachment)
        delete pDepthStencilAttachment;
    if (pPreserveAttachments)
        delete[] pPreserveAttachments;
}

void safe_VkSubpassDescription::initialize(const VkSubpassDescription* in_struct)
{
    flags = in_struct->flags;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    inputAttachmentCount = in_struct->inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = in_struct->colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    pDepthStencilAttachment = nullptr;
    preserveAttachmentCount = in_struct->preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (in_struct->pInputAttachments) {
        pInputAttachments = new VkAttachmentReference[in_struct->inputAttachmentCount];
        memcpy ((void *)pInputAttachments, (void *)in_struct->pInputAttachments, sizeof(VkAttachmentReference)*in_struct->inputAttachmentCount);
    }
    if (in_struct->pColorAttachments) {
        pColorAttachments = new VkAttachmentReference[in_struct->colorAttachmentCount];
        memcpy ((void *)pColorAttachments, (void *)in_struct->pColorAttachments, sizeof(VkAttachmentReference)*in_struct->colorAttachmentCount);
    }
    if (in_struct->pResolveAttachments) {
        pResolveAttachments = new VkAttachmentReference[in_struct->colorAttachmentCount];
        memcpy ((void *)pResolveAttachments, (void *)in_struct->pResolveAttachments, sizeof(VkAttachmentReference)*in_struct->colorAttachmentCount);
    }
    if (in_struct->pDepthStencilAttachment) {
        pDepthStencilAttachment = new VkAttachmentReference(*in_struct->pDepthStencilAttachment);
    }
    if (in_struct->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[in_struct->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)in_struct->pPreserveAttachments, sizeof(uint32_t)*in_struct->preserveAttachmentCount);
    }
}

void safe_VkSubpassDescription::initialize(const safe_VkSubpassDescription* src)
{
    flags = src->flags;
    pipelineBindPoint = src->pipelineBindPoint;
    inputAttachmentCount = src->inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src->colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    pDepthStencilAttachment = nullptr;
    preserveAttachmentCount = src->preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (src->pInputAttachments) {
        pInputAttachments = new VkAttachmentReference[src->inputAttachmentCount];
        memcpy ((void *)pInputAttachments, (void *)src->pInputAttachments, sizeof(VkAttachmentReference)*src->inputAttachmentCount);
    }
    if (src->pColorAttachments) {
        pColorAttachments = new VkAttachmentReference[src->colorAttachmentCount];
        memcpy ((void *)pColorAttachments, (void *)src->pColorAttachments, sizeof(VkAttachmentReference)*src->colorAttachmentCount);
    }
    if (src->pResolveAttachments) {
        pResolveAttachments = new VkAttachmentReference[src->colorAttachmentCount];
        memcpy ((void *)pResolveAttachments, (void *)src->pResolveAttachments, sizeof(VkAttachmentReference)*src->colorAttachmentCount);
    }
    if (src->pDepthStencilAttachment) {
        pDepthStencilAttachment = new VkAttachmentReference(*src->pDepthStencilAttachment);
    }
    if (src->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src->pPreserveAttachments, sizeof(uint32_t)*src->preserveAttachmentCount);
    }
}

safe_VkRenderPassCreateInfo::safe_VkRenderPassCreateInfo(const VkRenderPassCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    attachmentCount(in_struct->attachmentCount),
    pAttachments(nullptr),
    subpassCount(in_struct->subpassCount),
    pSubpasses(nullptr),
    dependencyCount(in_struct->dependencyCount),
    pDependencies(nullptr)
{
    if (in_struct->pAttachments) {
        pAttachments = new VkAttachmentDescription[in_struct->attachmentCount];
        memcpy ((void *)pAttachments, (void *)in_struct->pAttachments, sizeof(VkAttachmentDescription)*in_struct->attachmentCount);
    }
    if (subpassCount && in_struct->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&in_struct->pSubpasses[i]);
        }
    }
    if (in_struct->pDependencies) {
        pDependencies = new VkSubpassDependency[in_struct->dependencyCount];
        memcpy ((void *)pDependencies, (void *)in_struct->pDependencies, sizeof(VkSubpassDependency)*in_struct->dependencyCount);
    }
}

safe_VkRenderPassCreateInfo::safe_VkRenderPassCreateInfo() :
    pAttachments(nullptr),
    pSubpasses(nullptr),
    pDependencies(nullptr)
{}

safe_VkRenderPassCreateInfo::safe_VkRenderPassCreateInfo(const safe_VkRenderPassCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    subpassCount = src.subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src.dependencyCount;
    pDependencies = nullptr;
    if (src.pAttachments) {
        pAttachments = new VkAttachmentDescription[src.attachmentCount];
        memcpy ((void *)pAttachments, (void *)src.pAttachments, sizeof(VkAttachmentDescription)*src.attachmentCount);
    }
    if (subpassCount && src.pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src.pSubpasses[i]);
        }
    }
    if (src.pDependencies) {
        pDependencies = new VkSubpassDependency[src.dependencyCount];
        memcpy ((void *)pDependencies, (void *)src.pDependencies, sizeof(VkSubpassDependency)*src.dependencyCount);
    }
}

safe_VkRenderPassCreateInfo& safe_VkRenderPassCreateInfo::operator=(const safe_VkRenderPassCreateInfo& src)
{
    if (&src == this) return *this;

    if (pAttachments)
        delete[] pAttachments;
    if (pSubpasses)
        delete[] pSubpasses;
    if (pDependencies)
        delete[] pDependencies;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    subpassCount = src.subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src.dependencyCount;
    pDependencies = nullptr;
    if (src.pAttachments) {
        pAttachments = new VkAttachmentDescription[src.attachmentCount];
        memcpy ((void *)pAttachments, (void *)src.pAttachments, sizeof(VkAttachmentDescription)*src.attachmentCount);
    }
    if (subpassCount && src.pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src.pSubpasses[i]);
        }
    }
    if (src.pDependencies) {
        pDependencies = new VkSubpassDependency[src.dependencyCount];
        memcpy ((void *)pDependencies, (void *)src.pDependencies, sizeof(VkSubpassDependency)*src.dependencyCount);
    }

    return *this;
}

safe_VkRenderPassCreateInfo::~safe_VkRenderPassCreateInfo()
{
    if (pAttachments)
        delete[] pAttachments;
    if (pSubpasses)
        delete[] pSubpasses;
    if (pDependencies)
        delete[] pDependencies;
}

void safe_VkRenderPassCreateInfo::initialize(const VkRenderPassCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    attachmentCount = in_struct->attachmentCount;
    pAttachments = nullptr;
    subpassCount = in_struct->subpassCount;
    pSubpasses = nullptr;
    dependencyCount = in_struct->dependencyCount;
    pDependencies = nullptr;
    if (in_struct->pAttachments) {
        pAttachments = new VkAttachmentDescription[in_struct->attachmentCount];
        memcpy ((void *)pAttachments, (void *)in_struct->pAttachments, sizeof(VkAttachmentDescription)*in_struct->attachmentCount);
    }
    if (subpassCount && in_struct->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&in_struct->pSubpasses[i]);
        }
    }
    if (in_struct->pDependencies) {
        pDependencies = new VkSubpassDependency[in_struct->dependencyCount];
        memcpy ((void *)pDependencies, (void *)in_struct->pDependencies, sizeof(VkSubpassDependency)*in_struct->dependencyCount);
    }
}

void safe_VkRenderPassCreateInfo::initialize(const safe_VkRenderPassCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    attachmentCount = src->attachmentCount;
    pAttachments = nullptr;
    subpassCount = src->subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src->dependencyCount;
    pDependencies = nullptr;
    if (src->pAttachments) {
        pAttachments = new VkAttachmentDescription[src->attachmentCount];
        memcpy ((void *)pAttachments, (void *)src->pAttachments, sizeof(VkAttachmentDescription)*src->attachmentCount);
    }
    if (subpassCount && src->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src->pSubpasses[i]);
        }
    }
    if (src->pDependencies) {
        pDependencies = new VkSubpassDependency[src->dependencyCount];
        memcpy ((void *)pDependencies, (void *)src->pDependencies, sizeof(VkSubpassDependency)*src->dependencyCount);
    }
}

safe_VkCommandPoolCreateInfo::safe_VkCommandPoolCreateInfo(const VkCommandPoolCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    queueFamilyIndex(in_struct->queueFamilyIndex)
{
}

safe_VkCommandPoolCreateInfo::safe_VkCommandPoolCreateInfo()
{}

safe_VkCommandPoolCreateInfo::safe_VkCommandPoolCreateInfo(const safe_VkCommandPoolCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;
}

safe_VkCommandPoolCreateInfo& safe_VkCommandPoolCreateInfo::operator=(const safe_VkCommandPoolCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;

    return *this;
}

safe_VkCommandPoolCreateInfo::~safe_VkCommandPoolCreateInfo()
{
}

void safe_VkCommandPoolCreateInfo::initialize(const VkCommandPoolCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    queueFamilyIndex = in_struct->queueFamilyIndex;
}

void safe_VkCommandPoolCreateInfo::initialize(const safe_VkCommandPoolCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    queueFamilyIndex = src->queueFamilyIndex;
}

safe_VkCommandBufferAllocateInfo::safe_VkCommandBufferAllocateInfo(const VkCommandBufferAllocateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    commandPool(in_struct->commandPool),
    level(in_struct->level),
    commandBufferCount(in_struct->commandBufferCount)
{
}

safe_VkCommandBufferAllocateInfo::safe_VkCommandBufferAllocateInfo()
{}

safe_VkCommandBufferAllocateInfo::safe_VkCommandBufferAllocateInfo(const safe_VkCommandBufferAllocateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    commandPool = src.commandPool;
    level = src.level;
    commandBufferCount = src.commandBufferCount;
}

safe_VkCommandBufferAllocateInfo& safe_VkCommandBufferAllocateInfo::operator=(const safe_VkCommandBufferAllocateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    commandPool = src.commandPool;
    level = src.level;
    commandBufferCount = src.commandBufferCount;

    return *this;
}

safe_VkCommandBufferAllocateInfo::~safe_VkCommandBufferAllocateInfo()
{
}

void safe_VkCommandBufferAllocateInfo::initialize(const VkCommandBufferAllocateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    commandPool = in_struct->commandPool;
    level = in_struct->level;
    commandBufferCount = in_struct->commandBufferCount;
}

void safe_VkCommandBufferAllocateInfo::initialize(const safe_VkCommandBufferAllocateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    commandPool = src->commandPool;
    level = src->level;
    commandBufferCount = src->commandBufferCount;
}

safe_VkCommandBufferInheritanceInfo::safe_VkCommandBufferInheritanceInfo(const VkCommandBufferInheritanceInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    renderPass(in_struct->renderPass),
    subpass(in_struct->subpass),
    framebuffer(in_struct->framebuffer),
    occlusionQueryEnable(in_struct->occlusionQueryEnable),
    queryFlags(in_struct->queryFlags),
    pipelineStatistics(in_struct->pipelineStatistics)
{
}

safe_VkCommandBufferInheritanceInfo::safe_VkCommandBufferInheritanceInfo()
{}

safe_VkCommandBufferInheritanceInfo::safe_VkCommandBufferInheritanceInfo(const safe_VkCommandBufferInheritanceInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    renderPass = src.renderPass;
    subpass = src.subpass;
    framebuffer = src.framebuffer;
    occlusionQueryEnable = src.occlusionQueryEnable;
    queryFlags = src.queryFlags;
    pipelineStatistics = src.pipelineStatistics;
}

safe_VkCommandBufferInheritanceInfo& safe_VkCommandBufferInheritanceInfo::operator=(const safe_VkCommandBufferInheritanceInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    renderPass = src.renderPass;
    subpass = src.subpass;
    framebuffer = src.framebuffer;
    occlusionQueryEnable = src.occlusionQueryEnable;
    queryFlags = src.queryFlags;
    pipelineStatistics = src.pipelineStatistics;

    return *this;
}

safe_VkCommandBufferInheritanceInfo::~safe_VkCommandBufferInheritanceInfo()
{
}

void safe_VkCommandBufferInheritanceInfo::initialize(const VkCommandBufferInheritanceInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    renderPass = in_struct->renderPass;
    subpass = in_struct->subpass;
    framebuffer = in_struct->framebuffer;
    occlusionQueryEnable = in_struct->occlusionQueryEnable;
    queryFlags = in_struct->queryFlags;
    pipelineStatistics = in_struct->pipelineStatistics;
}

void safe_VkCommandBufferInheritanceInfo::initialize(const safe_VkCommandBufferInheritanceInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    renderPass = src->renderPass;
    subpass = src->subpass;
    framebuffer = src->framebuffer;
    occlusionQueryEnable = src->occlusionQueryEnable;
    queryFlags = src->queryFlags;
    pipelineStatistics = src->pipelineStatistics;
}

safe_VkCommandBufferBeginInfo::safe_VkCommandBufferBeginInfo(const VkCommandBufferBeginInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags)
{
    if (in_struct->pInheritanceInfo)
        pInheritanceInfo = new safe_VkCommandBufferInheritanceInfo(in_struct->pInheritanceInfo);
    else
        pInheritanceInfo = NULL;
}

safe_VkCommandBufferBeginInfo::safe_VkCommandBufferBeginInfo()
{}

safe_VkCommandBufferBeginInfo::safe_VkCommandBufferBeginInfo(const safe_VkCommandBufferBeginInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    if (src.pInheritanceInfo)
        pInheritanceInfo = new safe_VkCommandBufferInheritanceInfo(*src.pInheritanceInfo);
    else
        pInheritanceInfo = NULL;
}

safe_VkCommandBufferBeginInfo& safe_VkCommandBufferBeginInfo::operator=(const safe_VkCommandBufferBeginInfo& src)
{
    if (&src == this) return *this;

    if (pInheritanceInfo)
        delete pInheritanceInfo;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    if (src.pInheritanceInfo)
        pInheritanceInfo = new safe_VkCommandBufferInheritanceInfo(*src.pInheritanceInfo);
    else
        pInheritanceInfo = NULL;

    return *this;
}

safe_VkCommandBufferBeginInfo::~safe_VkCommandBufferBeginInfo()
{
    if (pInheritanceInfo)
        delete pInheritanceInfo;
}

void safe_VkCommandBufferBeginInfo::initialize(const VkCommandBufferBeginInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    if (in_struct->pInheritanceInfo)
        pInheritanceInfo = new safe_VkCommandBufferInheritanceInfo(in_struct->pInheritanceInfo);
    else
        pInheritanceInfo = NULL;
}

void safe_VkCommandBufferBeginInfo::initialize(const safe_VkCommandBufferBeginInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    if (src->pInheritanceInfo)
        pInheritanceInfo = new safe_VkCommandBufferInheritanceInfo(*src->pInheritanceInfo);
    else
        pInheritanceInfo = NULL;
}

safe_VkMemoryBarrier::safe_VkMemoryBarrier(const VkMemoryBarrier* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcAccessMask(in_struct->srcAccessMask),
    dstAccessMask(in_struct->dstAccessMask)
{
}

safe_VkMemoryBarrier::safe_VkMemoryBarrier()
{}

safe_VkMemoryBarrier::safe_VkMemoryBarrier(const safe_VkMemoryBarrier& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
}

safe_VkMemoryBarrier& safe_VkMemoryBarrier::operator=(const safe_VkMemoryBarrier& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;

    return *this;
}

safe_VkMemoryBarrier::~safe_VkMemoryBarrier()
{
}

void safe_VkMemoryBarrier::initialize(const VkMemoryBarrier* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcAccessMask = in_struct->srcAccessMask;
    dstAccessMask = in_struct->dstAccessMask;
}

void safe_VkMemoryBarrier::initialize(const safe_VkMemoryBarrier* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcAccessMask = src->srcAccessMask;
    dstAccessMask = src->dstAccessMask;
}

safe_VkBufferMemoryBarrier::safe_VkBufferMemoryBarrier(const VkBufferMemoryBarrier* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcAccessMask(in_struct->srcAccessMask),
    dstAccessMask(in_struct->dstAccessMask),
    srcQueueFamilyIndex(in_struct->srcQueueFamilyIndex),
    dstQueueFamilyIndex(in_struct->dstQueueFamilyIndex),
    buffer(in_struct->buffer),
    offset(in_struct->offset),
    size(in_struct->size)
{
}

safe_VkBufferMemoryBarrier::safe_VkBufferMemoryBarrier()
{}

safe_VkBufferMemoryBarrier::safe_VkBufferMemoryBarrier(const safe_VkBufferMemoryBarrier& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    srcQueueFamilyIndex = src.srcQueueFamilyIndex;
    dstQueueFamilyIndex = src.dstQueueFamilyIndex;
    buffer = src.buffer;
    offset = src.offset;
    size = src.size;
}

safe_VkBufferMemoryBarrier& safe_VkBufferMemoryBarrier::operator=(const safe_VkBufferMemoryBarrier& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    srcQueueFamilyIndex = src.srcQueueFamilyIndex;
    dstQueueFamilyIndex = src.dstQueueFamilyIndex;
    buffer = src.buffer;
    offset = src.offset;
    size = src.size;

    return *this;
}

safe_VkBufferMemoryBarrier::~safe_VkBufferMemoryBarrier()
{
}

void safe_VkBufferMemoryBarrier::initialize(const VkBufferMemoryBarrier* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcAccessMask = in_struct->srcAccessMask;
    dstAccessMask = in_struct->dstAccessMask;
    srcQueueFamilyIndex = in_struct->srcQueueFamilyIndex;
    dstQueueFamilyIndex = in_struct->dstQueueFamilyIndex;
    buffer = in_struct->buffer;
    offset = in_struct->offset;
    size = in_struct->size;
}

void safe_VkBufferMemoryBarrier::initialize(const safe_VkBufferMemoryBarrier* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcAccessMask = src->srcAccessMask;
    dstAccessMask = src->dstAccessMask;
    srcQueueFamilyIndex = src->srcQueueFamilyIndex;
    dstQueueFamilyIndex = src->dstQueueFamilyIndex;
    buffer = src->buffer;
    offset = src->offset;
    size = src->size;
}

safe_VkImageMemoryBarrier::safe_VkImageMemoryBarrier(const VkImageMemoryBarrier* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcAccessMask(in_struct->srcAccessMask),
    dstAccessMask(in_struct->dstAccessMask),
    oldLayout(in_struct->oldLayout),
    newLayout(in_struct->newLayout),
    srcQueueFamilyIndex(in_struct->srcQueueFamilyIndex),
    dstQueueFamilyIndex(in_struct->dstQueueFamilyIndex),
    image(in_struct->image),
    subresourceRange(in_struct->subresourceRange)
{
}

safe_VkImageMemoryBarrier::safe_VkImageMemoryBarrier()
{}

safe_VkImageMemoryBarrier::safe_VkImageMemoryBarrier(const safe_VkImageMemoryBarrier& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    oldLayout = src.oldLayout;
    newLayout = src.newLayout;
    srcQueueFamilyIndex = src.srcQueueFamilyIndex;
    dstQueueFamilyIndex = src.dstQueueFamilyIndex;
    image = src.image;
    subresourceRange = src.subresourceRange;
}

safe_VkImageMemoryBarrier& safe_VkImageMemoryBarrier::operator=(const safe_VkImageMemoryBarrier& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    oldLayout = src.oldLayout;
    newLayout = src.newLayout;
    srcQueueFamilyIndex = src.srcQueueFamilyIndex;
    dstQueueFamilyIndex = src.dstQueueFamilyIndex;
    image = src.image;
    subresourceRange = src.subresourceRange;

    return *this;
}

safe_VkImageMemoryBarrier::~safe_VkImageMemoryBarrier()
{
}

void safe_VkImageMemoryBarrier::initialize(const VkImageMemoryBarrier* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcAccessMask = in_struct->srcAccessMask;
    dstAccessMask = in_struct->dstAccessMask;
    oldLayout = in_struct->oldLayout;
    newLayout = in_struct->newLayout;
    srcQueueFamilyIndex = in_struct->srcQueueFamilyIndex;
    dstQueueFamilyIndex = in_struct->dstQueueFamilyIndex;
    image = in_struct->image;
    subresourceRange = in_struct->subresourceRange;
}

void safe_VkImageMemoryBarrier::initialize(const safe_VkImageMemoryBarrier* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcAccessMask = src->srcAccessMask;
    dstAccessMask = src->dstAccessMask;
    oldLayout = src->oldLayout;
    newLayout = src->newLayout;
    srcQueueFamilyIndex = src->srcQueueFamilyIndex;
    dstQueueFamilyIndex = src->dstQueueFamilyIndex;
    image = src->image;
    subresourceRange = src->subresourceRange;
}

safe_VkRenderPassBeginInfo::safe_VkRenderPassBeginInfo(const VkRenderPassBeginInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    renderPass(in_struct->renderPass),
    framebuffer(in_struct->framebuffer),
    renderArea(in_struct->renderArea),
    clearValueCount(in_struct->clearValueCount),
    pClearValues(nullptr)
{
    if (in_struct->pClearValues) {
        pClearValues = new VkClearValue[in_struct->clearValueCount];
        memcpy ((void *)pClearValues, (void *)in_struct->pClearValues, sizeof(VkClearValue)*in_struct->clearValueCount);
    }
}

safe_VkRenderPassBeginInfo::safe_VkRenderPassBeginInfo() :
    pClearValues(nullptr)
{}

safe_VkRenderPassBeginInfo::safe_VkRenderPassBeginInfo(const safe_VkRenderPassBeginInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    renderPass = src.renderPass;
    framebuffer = src.framebuffer;
    renderArea = src.renderArea;
    clearValueCount = src.clearValueCount;
    pClearValues = nullptr;
    if (src.pClearValues) {
        pClearValues = new VkClearValue[src.clearValueCount];
        memcpy ((void *)pClearValues, (void *)src.pClearValues, sizeof(VkClearValue)*src.clearValueCount);
    }
}

safe_VkRenderPassBeginInfo& safe_VkRenderPassBeginInfo::operator=(const safe_VkRenderPassBeginInfo& src)
{
    if (&src == this) return *this;

    if (pClearValues)
        delete[] pClearValues;

    sType = src.sType;
    pNext = src.pNext;
    renderPass = src.renderPass;
    framebuffer = src.framebuffer;
    renderArea = src.renderArea;
    clearValueCount = src.clearValueCount;
    pClearValues = nullptr;
    if (src.pClearValues) {
        pClearValues = new VkClearValue[src.clearValueCount];
        memcpy ((void *)pClearValues, (void *)src.pClearValues, sizeof(VkClearValue)*src.clearValueCount);
    }

    return *this;
}

safe_VkRenderPassBeginInfo::~safe_VkRenderPassBeginInfo()
{
    if (pClearValues)
        delete[] pClearValues;
}

void safe_VkRenderPassBeginInfo::initialize(const VkRenderPassBeginInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    renderPass = in_struct->renderPass;
    framebuffer = in_struct->framebuffer;
    renderArea = in_struct->renderArea;
    clearValueCount = in_struct->clearValueCount;
    pClearValues = nullptr;
    if (in_struct->pClearValues) {
        pClearValues = new VkClearValue[in_struct->clearValueCount];
        memcpy ((void *)pClearValues, (void *)in_struct->pClearValues, sizeof(VkClearValue)*in_struct->clearValueCount);
    }
}

void safe_VkRenderPassBeginInfo::initialize(const safe_VkRenderPassBeginInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    renderPass = src->renderPass;
    framebuffer = src->framebuffer;
    renderArea = src->renderArea;
    clearValueCount = src->clearValueCount;
    pClearValues = nullptr;
    if (src->pClearValues) {
        pClearValues = new VkClearValue[src->clearValueCount];
        memcpy ((void *)pClearValues, (void *)src->pClearValues, sizeof(VkClearValue)*src->clearValueCount);
    }
}

safe_VkBaseOutStructure::safe_VkBaseOutStructure(const VkBaseOutStructure* in_struct) :
    sType(in_struct->sType)
{
    if (in_struct->pNext)
        pNext = new safe_VkBaseOutStructure(in_struct->pNext);
    else
        pNext = NULL;
}

safe_VkBaseOutStructure::safe_VkBaseOutStructure()
{}

safe_VkBaseOutStructure::safe_VkBaseOutStructure(const safe_VkBaseOutStructure& src)
{
    sType = src.sType;
    if (src.pNext)
        pNext = new safe_VkBaseOutStructure(*src.pNext);
    else
        pNext = NULL;
}

safe_VkBaseOutStructure& safe_VkBaseOutStructure::operator=(const safe_VkBaseOutStructure& src)
{
    if (&src == this) return *this;

    if (pNext)
        delete pNext;

    sType = src.sType;
    if (src.pNext)
        pNext = new safe_VkBaseOutStructure(*src.pNext);
    else
        pNext = NULL;

    return *this;
}

safe_VkBaseOutStructure::~safe_VkBaseOutStructure()
{
    if (pNext)
        delete pNext;
}

void safe_VkBaseOutStructure::initialize(const VkBaseOutStructure* in_struct)
{
    sType = in_struct->sType;
    if (in_struct->pNext)
        pNext = new safe_VkBaseOutStructure(in_struct->pNext);
    else
        pNext = NULL;
}

void safe_VkBaseOutStructure::initialize(const safe_VkBaseOutStructure* src)
{
    sType = src->sType;
    if (src->pNext)
        pNext = new safe_VkBaseOutStructure(*src->pNext);
    else
        pNext = NULL;
}

safe_VkBaseInStructure::safe_VkBaseInStructure(const VkBaseInStructure* in_struct) :
    sType(in_struct->sType)
{
    if (in_struct->pNext)
        pNext = new safe_VkBaseInStructure(in_struct->pNext);
    else
        pNext = NULL;
}

safe_VkBaseInStructure::safe_VkBaseInStructure()
{}

safe_VkBaseInStructure::safe_VkBaseInStructure(const safe_VkBaseInStructure& src)
{
    sType = src.sType;
    if (src.pNext)
        pNext = new safe_VkBaseInStructure(*src.pNext);
    else
        pNext = NULL;
}

safe_VkBaseInStructure& safe_VkBaseInStructure::operator=(const safe_VkBaseInStructure& src)
{
    if (&src == this) return *this;

    if (pNext)
        delete pNext;

    sType = src.sType;
    if (src.pNext)
        pNext = new safe_VkBaseInStructure(*src.pNext);
    else
        pNext = NULL;

    return *this;
}

safe_VkBaseInStructure::~safe_VkBaseInStructure()
{
    if (pNext)
        delete pNext;
}

void safe_VkBaseInStructure::initialize(const VkBaseInStructure* in_struct)
{
    sType = in_struct->sType;
    if (in_struct->pNext)
        pNext = new safe_VkBaseInStructure(in_struct->pNext);
    else
        pNext = NULL;
}

void safe_VkBaseInStructure::initialize(const safe_VkBaseInStructure* src)
{
    sType = src->sType;
    if (src->pNext)
        pNext = new safe_VkBaseInStructure(*src->pNext);
    else
        pNext = NULL;
}

safe_VkPhysicalDeviceSubgroupProperties::safe_VkPhysicalDeviceSubgroupProperties(const VkPhysicalDeviceSubgroupProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    subgroupSize(in_struct->subgroupSize),
    supportedStages(in_struct->supportedStages),
    supportedOperations(in_struct->supportedOperations),
    quadOperationsInAllStages(in_struct->quadOperationsInAllStages)
{
}

safe_VkPhysicalDeviceSubgroupProperties::safe_VkPhysicalDeviceSubgroupProperties()
{}

safe_VkPhysicalDeviceSubgroupProperties::safe_VkPhysicalDeviceSubgroupProperties(const safe_VkPhysicalDeviceSubgroupProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    subgroupSize = src.subgroupSize;
    supportedStages = src.supportedStages;
    supportedOperations = src.supportedOperations;
    quadOperationsInAllStages = src.quadOperationsInAllStages;
}

safe_VkPhysicalDeviceSubgroupProperties& safe_VkPhysicalDeviceSubgroupProperties::operator=(const safe_VkPhysicalDeviceSubgroupProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    subgroupSize = src.subgroupSize;
    supportedStages = src.supportedStages;
    supportedOperations = src.supportedOperations;
    quadOperationsInAllStages = src.quadOperationsInAllStages;

    return *this;
}

safe_VkPhysicalDeviceSubgroupProperties::~safe_VkPhysicalDeviceSubgroupProperties()
{
}

void safe_VkPhysicalDeviceSubgroupProperties::initialize(const VkPhysicalDeviceSubgroupProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    subgroupSize = in_struct->subgroupSize;
    supportedStages = in_struct->supportedStages;
    supportedOperations = in_struct->supportedOperations;
    quadOperationsInAllStages = in_struct->quadOperationsInAllStages;
}

void safe_VkPhysicalDeviceSubgroupProperties::initialize(const safe_VkPhysicalDeviceSubgroupProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    subgroupSize = src->subgroupSize;
    supportedStages = src->supportedStages;
    supportedOperations = src->supportedOperations;
    quadOperationsInAllStages = src->quadOperationsInAllStages;
}

safe_VkBindBufferMemoryInfo::safe_VkBindBufferMemoryInfo(const VkBindBufferMemoryInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    buffer(in_struct->buffer),
    memory(in_struct->memory),
    memoryOffset(in_struct->memoryOffset)
{
}

safe_VkBindBufferMemoryInfo::safe_VkBindBufferMemoryInfo()
{}

safe_VkBindBufferMemoryInfo::safe_VkBindBufferMemoryInfo(const safe_VkBindBufferMemoryInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
    memory = src.memory;
    memoryOffset = src.memoryOffset;
}

safe_VkBindBufferMemoryInfo& safe_VkBindBufferMemoryInfo::operator=(const safe_VkBindBufferMemoryInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
    memory = src.memory;
    memoryOffset = src.memoryOffset;

    return *this;
}

safe_VkBindBufferMemoryInfo::~safe_VkBindBufferMemoryInfo()
{
}

void safe_VkBindBufferMemoryInfo::initialize(const VkBindBufferMemoryInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    buffer = in_struct->buffer;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
}

void safe_VkBindBufferMemoryInfo::initialize(const safe_VkBindBufferMemoryInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    buffer = src->buffer;
    memory = src->memory;
    memoryOffset = src->memoryOffset;
}

safe_VkBindImageMemoryInfo::safe_VkBindImageMemoryInfo(const VkBindImageMemoryInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    image(in_struct->image),
    memory(in_struct->memory),
    memoryOffset(in_struct->memoryOffset)
{
}

safe_VkBindImageMemoryInfo::safe_VkBindImageMemoryInfo()
{}

safe_VkBindImageMemoryInfo::safe_VkBindImageMemoryInfo(const safe_VkBindImageMemoryInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    memory = src.memory;
    memoryOffset = src.memoryOffset;
}

safe_VkBindImageMemoryInfo& safe_VkBindImageMemoryInfo::operator=(const safe_VkBindImageMemoryInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    memory = src.memory;
    memoryOffset = src.memoryOffset;

    return *this;
}

safe_VkBindImageMemoryInfo::~safe_VkBindImageMemoryInfo()
{
}

void safe_VkBindImageMemoryInfo::initialize(const VkBindImageMemoryInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    image = in_struct->image;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
}

void safe_VkBindImageMemoryInfo::initialize(const safe_VkBindImageMemoryInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    image = src->image;
    memory = src->memory;
    memoryOffset = src->memoryOffset;
}

safe_VkPhysicalDevice16BitStorageFeatures::safe_VkPhysicalDevice16BitStorageFeatures(const VkPhysicalDevice16BitStorageFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    storageBuffer16BitAccess(in_struct->storageBuffer16BitAccess),
    uniformAndStorageBuffer16BitAccess(in_struct->uniformAndStorageBuffer16BitAccess),
    storagePushConstant16(in_struct->storagePushConstant16),
    storageInputOutput16(in_struct->storageInputOutput16)
{
}

safe_VkPhysicalDevice16BitStorageFeatures::safe_VkPhysicalDevice16BitStorageFeatures()
{}

safe_VkPhysicalDevice16BitStorageFeatures::safe_VkPhysicalDevice16BitStorageFeatures(const safe_VkPhysicalDevice16BitStorageFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    storageBuffer16BitAccess = src.storageBuffer16BitAccess;
    uniformAndStorageBuffer16BitAccess = src.uniformAndStorageBuffer16BitAccess;
    storagePushConstant16 = src.storagePushConstant16;
    storageInputOutput16 = src.storageInputOutput16;
}

safe_VkPhysicalDevice16BitStorageFeatures& safe_VkPhysicalDevice16BitStorageFeatures::operator=(const safe_VkPhysicalDevice16BitStorageFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    storageBuffer16BitAccess = src.storageBuffer16BitAccess;
    uniformAndStorageBuffer16BitAccess = src.uniformAndStorageBuffer16BitAccess;
    storagePushConstant16 = src.storagePushConstant16;
    storageInputOutput16 = src.storageInputOutput16;

    return *this;
}

safe_VkPhysicalDevice16BitStorageFeatures::~safe_VkPhysicalDevice16BitStorageFeatures()
{
}

void safe_VkPhysicalDevice16BitStorageFeatures::initialize(const VkPhysicalDevice16BitStorageFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    storageBuffer16BitAccess = in_struct->storageBuffer16BitAccess;
    uniformAndStorageBuffer16BitAccess = in_struct->uniformAndStorageBuffer16BitAccess;
    storagePushConstant16 = in_struct->storagePushConstant16;
    storageInputOutput16 = in_struct->storageInputOutput16;
}

void safe_VkPhysicalDevice16BitStorageFeatures::initialize(const safe_VkPhysicalDevice16BitStorageFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    storageBuffer16BitAccess = src->storageBuffer16BitAccess;
    uniformAndStorageBuffer16BitAccess = src->uniformAndStorageBuffer16BitAccess;
    storagePushConstant16 = src->storagePushConstant16;
    storageInputOutput16 = src->storageInputOutput16;
}

safe_VkMemoryDedicatedRequirements::safe_VkMemoryDedicatedRequirements(const VkMemoryDedicatedRequirements* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    prefersDedicatedAllocation(in_struct->prefersDedicatedAllocation),
    requiresDedicatedAllocation(in_struct->requiresDedicatedAllocation)
{
}

safe_VkMemoryDedicatedRequirements::safe_VkMemoryDedicatedRequirements()
{}

safe_VkMemoryDedicatedRequirements::safe_VkMemoryDedicatedRequirements(const safe_VkMemoryDedicatedRequirements& src)
{
    sType = src.sType;
    pNext = src.pNext;
    prefersDedicatedAllocation = src.prefersDedicatedAllocation;
    requiresDedicatedAllocation = src.requiresDedicatedAllocation;
}

safe_VkMemoryDedicatedRequirements& safe_VkMemoryDedicatedRequirements::operator=(const safe_VkMemoryDedicatedRequirements& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    prefersDedicatedAllocation = src.prefersDedicatedAllocation;
    requiresDedicatedAllocation = src.requiresDedicatedAllocation;

    return *this;
}

safe_VkMemoryDedicatedRequirements::~safe_VkMemoryDedicatedRequirements()
{
}

void safe_VkMemoryDedicatedRequirements::initialize(const VkMemoryDedicatedRequirements* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    prefersDedicatedAllocation = in_struct->prefersDedicatedAllocation;
    requiresDedicatedAllocation = in_struct->requiresDedicatedAllocation;
}

void safe_VkMemoryDedicatedRequirements::initialize(const safe_VkMemoryDedicatedRequirements* src)
{
    sType = src->sType;
    pNext = src->pNext;
    prefersDedicatedAllocation = src->prefersDedicatedAllocation;
    requiresDedicatedAllocation = src->requiresDedicatedAllocation;
}

safe_VkMemoryDedicatedAllocateInfo::safe_VkMemoryDedicatedAllocateInfo(const VkMemoryDedicatedAllocateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    image(in_struct->image),
    buffer(in_struct->buffer)
{
}

safe_VkMemoryDedicatedAllocateInfo::safe_VkMemoryDedicatedAllocateInfo()
{}

safe_VkMemoryDedicatedAllocateInfo::safe_VkMemoryDedicatedAllocateInfo(const safe_VkMemoryDedicatedAllocateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    buffer = src.buffer;
}

safe_VkMemoryDedicatedAllocateInfo& safe_VkMemoryDedicatedAllocateInfo::operator=(const safe_VkMemoryDedicatedAllocateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    buffer = src.buffer;

    return *this;
}

safe_VkMemoryDedicatedAllocateInfo::~safe_VkMemoryDedicatedAllocateInfo()
{
}

void safe_VkMemoryDedicatedAllocateInfo::initialize(const VkMemoryDedicatedAllocateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    image = in_struct->image;
    buffer = in_struct->buffer;
}

void safe_VkMemoryDedicatedAllocateInfo::initialize(const safe_VkMemoryDedicatedAllocateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    image = src->image;
    buffer = src->buffer;
}

safe_VkMemoryAllocateFlagsInfo::safe_VkMemoryAllocateFlagsInfo(const VkMemoryAllocateFlagsInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    deviceMask(in_struct->deviceMask)
{
}

safe_VkMemoryAllocateFlagsInfo::safe_VkMemoryAllocateFlagsInfo()
{}

safe_VkMemoryAllocateFlagsInfo::safe_VkMemoryAllocateFlagsInfo(const safe_VkMemoryAllocateFlagsInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    deviceMask = src.deviceMask;
}

safe_VkMemoryAllocateFlagsInfo& safe_VkMemoryAllocateFlagsInfo::operator=(const safe_VkMemoryAllocateFlagsInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    deviceMask = src.deviceMask;

    return *this;
}

safe_VkMemoryAllocateFlagsInfo::~safe_VkMemoryAllocateFlagsInfo()
{
}

void safe_VkMemoryAllocateFlagsInfo::initialize(const VkMemoryAllocateFlagsInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    deviceMask = in_struct->deviceMask;
}

void safe_VkMemoryAllocateFlagsInfo::initialize(const safe_VkMemoryAllocateFlagsInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    deviceMask = src->deviceMask;
}

safe_VkDeviceGroupRenderPassBeginInfo::safe_VkDeviceGroupRenderPassBeginInfo(const VkDeviceGroupRenderPassBeginInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceMask(in_struct->deviceMask),
    deviceRenderAreaCount(in_struct->deviceRenderAreaCount),
    pDeviceRenderAreas(nullptr)
{
    if (in_struct->pDeviceRenderAreas) {
        pDeviceRenderAreas = new VkRect2D[in_struct->deviceRenderAreaCount];
        memcpy ((void *)pDeviceRenderAreas, (void *)in_struct->pDeviceRenderAreas, sizeof(VkRect2D)*in_struct->deviceRenderAreaCount);
    }
}

safe_VkDeviceGroupRenderPassBeginInfo::safe_VkDeviceGroupRenderPassBeginInfo() :
    pDeviceRenderAreas(nullptr)
{}

safe_VkDeviceGroupRenderPassBeginInfo::safe_VkDeviceGroupRenderPassBeginInfo(const safe_VkDeviceGroupRenderPassBeginInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceMask = src.deviceMask;
    deviceRenderAreaCount = src.deviceRenderAreaCount;
    pDeviceRenderAreas = nullptr;
    if (src.pDeviceRenderAreas) {
        pDeviceRenderAreas = new VkRect2D[src.deviceRenderAreaCount];
        memcpy ((void *)pDeviceRenderAreas, (void *)src.pDeviceRenderAreas, sizeof(VkRect2D)*src.deviceRenderAreaCount);
    }
}

safe_VkDeviceGroupRenderPassBeginInfo& safe_VkDeviceGroupRenderPassBeginInfo::operator=(const safe_VkDeviceGroupRenderPassBeginInfo& src)
{
    if (&src == this) return *this;

    if (pDeviceRenderAreas)
        delete[] pDeviceRenderAreas;

    sType = src.sType;
    pNext = src.pNext;
    deviceMask = src.deviceMask;
    deviceRenderAreaCount = src.deviceRenderAreaCount;
    pDeviceRenderAreas = nullptr;
    if (src.pDeviceRenderAreas) {
        pDeviceRenderAreas = new VkRect2D[src.deviceRenderAreaCount];
        memcpy ((void *)pDeviceRenderAreas, (void *)src.pDeviceRenderAreas, sizeof(VkRect2D)*src.deviceRenderAreaCount);
    }

    return *this;
}

safe_VkDeviceGroupRenderPassBeginInfo::~safe_VkDeviceGroupRenderPassBeginInfo()
{
    if (pDeviceRenderAreas)
        delete[] pDeviceRenderAreas;
}

void safe_VkDeviceGroupRenderPassBeginInfo::initialize(const VkDeviceGroupRenderPassBeginInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceMask = in_struct->deviceMask;
    deviceRenderAreaCount = in_struct->deviceRenderAreaCount;
    pDeviceRenderAreas = nullptr;
    if (in_struct->pDeviceRenderAreas) {
        pDeviceRenderAreas = new VkRect2D[in_struct->deviceRenderAreaCount];
        memcpy ((void *)pDeviceRenderAreas, (void *)in_struct->pDeviceRenderAreas, sizeof(VkRect2D)*in_struct->deviceRenderAreaCount);
    }
}

void safe_VkDeviceGroupRenderPassBeginInfo::initialize(const safe_VkDeviceGroupRenderPassBeginInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceMask = src->deviceMask;
    deviceRenderAreaCount = src->deviceRenderAreaCount;
    pDeviceRenderAreas = nullptr;
    if (src->pDeviceRenderAreas) {
        pDeviceRenderAreas = new VkRect2D[src->deviceRenderAreaCount];
        memcpy ((void *)pDeviceRenderAreas, (void *)src->pDeviceRenderAreas, sizeof(VkRect2D)*src->deviceRenderAreaCount);
    }
}

safe_VkDeviceGroupCommandBufferBeginInfo::safe_VkDeviceGroupCommandBufferBeginInfo(const VkDeviceGroupCommandBufferBeginInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceMask(in_struct->deviceMask)
{
}

safe_VkDeviceGroupCommandBufferBeginInfo::safe_VkDeviceGroupCommandBufferBeginInfo()
{}

safe_VkDeviceGroupCommandBufferBeginInfo::safe_VkDeviceGroupCommandBufferBeginInfo(const safe_VkDeviceGroupCommandBufferBeginInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceMask = src.deviceMask;
}

safe_VkDeviceGroupCommandBufferBeginInfo& safe_VkDeviceGroupCommandBufferBeginInfo::operator=(const safe_VkDeviceGroupCommandBufferBeginInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    deviceMask = src.deviceMask;

    return *this;
}

safe_VkDeviceGroupCommandBufferBeginInfo::~safe_VkDeviceGroupCommandBufferBeginInfo()
{
}

void safe_VkDeviceGroupCommandBufferBeginInfo::initialize(const VkDeviceGroupCommandBufferBeginInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceMask = in_struct->deviceMask;
}

void safe_VkDeviceGroupCommandBufferBeginInfo::initialize(const safe_VkDeviceGroupCommandBufferBeginInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceMask = src->deviceMask;
}

safe_VkDeviceGroupSubmitInfo::safe_VkDeviceGroupSubmitInfo(const VkDeviceGroupSubmitInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    waitSemaphoreCount(in_struct->waitSemaphoreCount),
    pWaitSemaphoreDeviceIndices(nullptr),
    commandBufferCount(in_struct->commandBufferCount),
    pCommandBufferDeviceMasks(nullptr),
    signalSemaphoreCount(in_struct->signalSemaphoreCount),
    pSignalSemaphoreDeviceIndices(nullptr)
{
    if (in_struct->pWaitSemaphoreDeviceIndices) {
        pWaitSemaphoreDeviceIndices = new uint32_t[in_struct->waitSemaphoreCount];
        memcpy ((void *)pWaitSemaphoreDeviceIndices, (void *)in_struct->pWaitSemaphoreDeviceIndices, sizeof(uint32_t)*in_struct->waitSemaphoreCount);
    }
    if (in_struct->pCommandBufferDeviceMasks) {
        pCommandBufferDeviceMasks = new uint32_t[in_struct->commandBufferCount];
        memcpy ((void *)pCommandBufferDeviceMasks, (void *)in_struct->pCommandBufferDeviceMasks, sizeof(uint32_t)*in_struct->commandBufferCount);
    }
    if (in_struct->pSignalSemaphoreDeviceIndices) {
        pSignalSemaphoreDeviceIndices = new uint32_t[in_struct->signalSemaphoreCount];
        memcpy ((void *)pSignalSemaphoreDeviceIndices, (void *)in_struct->pSignalSemaphoreDeviceIndices, sizeof(uint32_t)*in_struct->signalSemaphoreCount);
    }
}

safe_VkDeviceGroupSubmitInfo::safe_VkDeviceGroupSubmitInfo() :
    pWaitSemaphoreDeviceIndices(nullptr),
    pCommandBufferDeviceMasks(nullptr),
    pSignalSemaphoreDeviceIndices(nullptr)
{}

safe_VkDeviceGroupSubmitInfo::safe_VkDeviceGroupSubmitInfo(const safe_VkDeviceGroupSubmitInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphoreDeviceIndices = nullptr;
    commandBufferCount = src.commandBufferCount;
    pCommandBufferDeviceMasks = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphoreDeviceIndices = nullptr;
    if (src.pWaitSemaphoreDeviceIndices) {
        pWaitSemaphoreDeviceIndices = new uint32_t[src.waitSemaphoreCount];
        memcpy ((void *)pWaitSemaphoreDeviceIndices, (void *)src.pWaitSemaphoreDeviceIndices, sizeof(uint32_t)*src.waitSemaphoreCount);
    }
    if (src.pCommandBufferDeviceMasks) {
        pCommandBufferDeviceMasks = new uint32_t[src.commandBufferCount];
        memcpy ((void *)pCommandBufferDeviceMasks, (void *)src.pCommandBufferDeviceMasks, sizeof(uint32_t)*src.commandBufferCount);
    }
    if (src.pSignalSemaphoreDeviceIndices) {
        pSignalSemaphoreDeviceIndices = new uint32_t[src.signalSemaphoreCount];
        memcpy ((void *)pSignalSemaphoreDeviceIndices, (void *)src.pSignalSemaphoreDeviceIndices, sizeof(uint32_t)*src.signalSemaphoreCount);
    }
}

safe_VkDeviceGroupSubmitInfo& safe_VkDeviceGroupSubmitInfo::operator=(const safe_VkDeviceGroupSubmitInfo& src)
{
    if (&src == this) return *this;

    if (pWaitSemaphoreDeviceIndices)
        delete[] pWaitSemaphoreDeviceIndices;
    if (pCommandBufferDeviceMasks)
        delete[] pCommandBufferDeviceMasks;
    if (pSignalSemaphoreDeviceIndices)
        delete[] pSignalSemaphoreDeviceIndices;

    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphoreDeviceIndices = nullptr;
    commandBufferCount = src.commandBufferCount;
    pCommandBufferDeviceMasks = nullptr;
    signalSemaphoreCount = src.signalSemaphoreCount;
    pSignalSemaphoreDeviceIndices = nullptr;
    if (src.pWaitSemaphoreDeviceIndices) {
        pWaitSemaphoreDeviceIndices = new uint32_t[src.waitSemaphoreCount];
        memcpy ((void *)pWaitSemaphoreDeviceIndices, (void *)src.pWaitSemaphoreDeviceIndices, sizeof(uint32_t)*src.waitSemaphoreCount);
    }
    if (src.pCommandBufferDeviceMasks) {
        pCommandBufferDeviceMasks = new uint32_t[src.commandBufferCount];
        memcpy ((void *)pCommandBufferDeviceMasks, (void *)src.pCommandBufferDeviceMasks, sizeof(uint32_t)*src.commandBufferCount);
    }
    if (src.pSignalSemaphoreDeviceIndices) {
        pSignalSemaphoreDeviceIndices = new uint32_t[src.signalSemaphoreCount];
        memcpy ((void *)pSignalSemaphoreDeviceIndices, (void *)src.pSignalSemaphoreDeviceIndices, sizeof(uint32_t)*src.signalSemaphoreCount);
    }

    return *this;
}

safe_VkDeviceGroupSubmitInfo::~safe_VkDeviceGroupSubmitInfo()
{
    if (pWaitSemaphoreDeviceIndices)
        delete[] pWaitSemaphoreDeviceIndices;
    if (pCommandBufferDeviceMasks)
        delete[] pCommandBufferDeviceMasks;
    if (pSignalSemaphoreDeviceIndices)
        delete[] pSignalSemaphoreDeviceIndices;
}

void safe_VkDeviceGroupSubmitInfo::initialize(const VkDeviceGroupSubmitInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphoreDeviceIndices = nullptr;
    commandBufferCount = in_struct->commandBufferCount;
    pCommandBufferDeviceMasks = nullptr;
    signalSemaphoreCount = in_struct->signalSemaphoreCount;
    pSignalSemaphoreDeviceIndices = nullptr;
    if (in_struct->pWaitSemaphoreDeviceIndices) {
        pWaitSemaphoreDeviceIndices = new uint32_t[in_struct->waitSemaphoreCount];
        memcpy ((void *)pWaitSemaphoreDeviceIndices, (void *)in_struct->pWaitSemaphoreDeviceIndices, sizeof(uint32_t)*in_struct->waitSemaphoreCount);
    }
    if (in_struct->pCommandBufferDeviceMasks) {
        pCommandBufferDeviceMasks = new uint32_t[in_struct->commandBufferCount];
        memcpy ((void *)pCommandBufferDeviceMasks, (void *)in_struct->pCommandBufferDeviceMasks, sizeof(uint32_t)*in_struct->commandBufferCount);
    }
    if (in_struct->pSignalSemaphoreDeviceIndices) {
        pSignalSemaphoreDeviceIndices = new uint32_t[in_struct->signalSemaphoreCount];
        memcpy ((void *)pSignalSemaphoreDeviceIndices, (void *)in_struct->pSignalSemaphoreDeviceIndices, sizeof(uint32_t)*in_struct->signalSemaphoreCount);
    }
}

void safe_VkDeviceGroupSubmitInfo::initialize(const safe_VkDeviceGroupSubmitInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    waitSemaphoreCount = src->waitSemaphoreCount;
    pWaitSemaphoreDeviceIndices = nullptr;
    commandBufferCount = src->commandBufferCount;
    pCommandBufferDeviceMasks = nullptr;
    signalSemaphoreCount = src->signalSemaphoreCount;
    pSignalSemaphoreDeviceIndices = nullptr;
    if (src->pWaitSemaphoreDeviceIndices) {
        pWaitSemaphoreDeviceIndices = new uint32_t[src->waitSemaphoreCount];
        memcpy ((void *)pWaitSemaphoreDeviceIndices, (void *)src->pWaitSemaphoreDeviceIndices, sizeof(uint32_t)*src->waitSemaphoreCount);
    }
    if (src->pCommandBufferDeviceMasks) {
        pCommandBufferDeviceMasks = new uint32_t[src->commandBufferCount];
        memcpy ((void *)pCommandBufferDeviceMasks, (void *)src->pCommandBufferDeviceMasks, sizeof(uint32_t)*src->commandBufferCount);
    }
    if (src->pSignalSemaphoreDeviceIndices) {
        pSignalSemaphoreDeviceIndices = new uint32_t[src->signalSemaphoreCount];
        memcpy ((void *)pSignalSemaphoreDeviceIndices, (void *)src->pSignalSemaphoreDeviceIndices, sizeof(uint32_t)*src->signalSemaphoreCount);
    }
}

safe_VkDeviceGroupBindSparseInfo::safe_VkDeviceGroupBindSparseInfo(const VkDeviceGroupBindSparseInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    resourceDeviceIndex(in_struct->resourceDeviceIndex),
    memoryDeviceIndex(in_struct->memoryDeviceIndex)
{
}

safe_VkDeviceGroupBindSparseInfo::safe_VkDeviceGroupBindSparseInfo()
{}

safe_VkDeviceGroupBindSparseInfo::safe_VkDeviceGroupBindSparseInfo(const safe_VkDeviceGroupBindSparseInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    resourceDeviceIndex = src.resourceDeviceIndex;
    memoryDeviceIndex = src.memoryDeviceIndex;
}

safe_VkDeviceGroupBindSparseInfo& safe_VkDeviceGroupBindSparseInfo::operator=(const safe_VkDeviceGroupBindSparseInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    resourceDeviceIndex = src.resourceDeviceIndex;
    memoryDeviceIndex = src.memoryDeviceIndex;

    return *this;
}

safe_VkDeviceGroupBindSparseInfo::~safe_VkDeviceGroupBindSparseInfo()
{
}

void safe_VkDeviceGroupBindSparseInfo::initialize(const VkDeviceGroupBindSparseInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    resourceDeviceIndex = in_struct->resourceDeviceIndex;
    memoryDeviceIndex = in_struct->memoryDeviceIndex;
}

void safe_VkDeviceGroupBindSparseInfo::initialize(const safe_VkDeviceGroupBindSparseInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    resourceDeviceIndex = src->resourceDeviceIndex;
    memoryDeviceIndex = src->memoryDeviceIndex;
}

safe_VkBindBufferMemoryDeviceGroupInfo::safe_VkBindBufferMemoryDeviceGroupInfo(const VkBindBufferMemoryDeviceGroupInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceIndexCount(in_struct->deviceIndexCount),
    pDeviceIndices(nullptr)
{
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
}

safe_VkBindBufferMemoryDeviceGroupInfo::safe_VkBindBufferMemoryDeviceGroupInfo() :
    pDeviceIndices(nullptr)
{}

safe_VkBindBufferMemoryDeviceGroupInfo::safe_VkBindBufferMemoryDeviceGroupInfo(const safe_VkBindBufferMemoryDeviceGroupInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }
}

safe_VkBindBufferMemoryDeviceGroupInfo& safe_VkBindBufferMemoryDeviceGroupInfo::operator=(const safe_VkBindBufferMemoryDeviceGroupInfo& src)
{
    if (&src == this) return *this;

    if (pDeviceIndices)
        delete[] pDeviceIndices;

    sType = src.sType;
    pNext = src.pNext;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }

    return *this;
}

safe_VkBindBufferMemoryDeviceGroupInfo::~safe_VkBindBufferMemoryDeviceGroupInfo()
{
    if (pDeviceIndices)
        delete[] pDeviceIndices;
}

void safe_VkBindBufferMemoryDeviceGroupInfo::initialize(const VkBindBufferMemoryDeviceGroupInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceIndexCount = in_struct->deviceIndexCount;
    pDeviceIndices = nullptr;
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
}

void safe_VkBindBufferMemoryDeviceGroupInfo::initialize(const safe_VkBindBufferMemoryDeviceGroupInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceIndexCount = src->deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src->pDeviceIndices) {
        pDeviceIndices = new uint32_t[src->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src->pDeviceIndices, sizeof(uint32_t)*src->deviceIndexCount);
    }
}

safe_VkBindImageMemoryDeviceGroupInfo::safe_VkBindImageMemoryDeviceGroupInfo(const VkBindImageMemoryDeviceGroupInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceIndexCount(in_struct->deviceIndexCount),
    pDeviceIndices(nullptr),
    splitInstanceBindRegionCount(in_struct->splitInstanceBindRegionCount),
    pSplitInstanceBindRegions(nullptr)
{
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
    if (in_struct->pSplitInstanceBindRegions) {
        pSplitInstanceBindRegions = new VkRect2D[in_struct->splitInstanceBindRegionCount];
        memcpy ((void *)pSplitInstanceBindRegions, (void *)in_struct->pSplitInstanceBindRegions, sizeof(VkRect2D)*in_struct->splitInstanceBindRegionCount);
    }
}

safe_VkBindImageMemoryDeviceGroupInfo::safe_VkBindImageMemoryDeviceGroupInfo() :
    pDeviceIndices(nullptr),
    pSplitInstanceBindRegions(nullptr)
{}

safe_VkBindImageMemoryDeviceGroupInfo::safe_VkBindImageMemoryDeviceGroupInfo(const safe_VkBindImageMemoryDeviceGroupInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    splitInstanceBindRegionCount = src.splitInstanceBindRegionCount;
    pSplitInstanceBindRegions = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }
    if (src.pSplitInstanceBindRegions) {
        pSplitInstanceBindRegions = new VkRect2D[src.splitInstanceBindRegionCount];
        memcpy ((void *)pSplitInstanceBindRegions, (void *)src.pSplitInstanceBindRegions, sizeof(VkRect2D)*src.splitInstanceBindRegionCount);
    }
}

safe_VkBindImageMemoryDeviceGroupInfo& safe_VkBindImageMemoryDeviceGroupInfo::operator=(const safe_VkBindImageMemoryDeviceGroupInfo& src)
{
    if (&src == this) return *this;

    if (pDeviceIndices)
        delete[] pDeviceIndices;
    if (pSplitInstanceBindRegions)
        delete[] pSplitInstanceBindRegions;

    sType = src.sType;
    pNext = src.pNext;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    splitInstanceBindRegionCount = src.splitInstanceBindRegionCount;
    pSplitInstanceBindRegions = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }
    if (src.pSplitInstanceBindRegions) {
        pSplitInstanceBindRegions = new VkRect2D[src.splitInstanceBindRegionCount];
        memcpy ((void *)pSplitInstanceBindRegions, (void *)src.pSplitInstanceBindRegions, sizeof(VkRect2D)*src.splitInstanceBindRegionCount);
    }

    return *this;
}

safe_VkBindImageMemoryDeviceGroupInfo::~safe_VkBindImageMemoryDeviceGroupInfo()
{
    if (pDeviceIndices)
        delete[] pDeviceIndices;
    if (pSplitInstanceBindRegions)
        delete[] pSplitInstanceBindRegions;
}

void safe_VkBindImageMemoryDeviceGroupInfo::initialize(const VkBindImageMemoryDeviceGroupInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceIndexCount = in_struct->deviceIndexCount;
    pDeviceIndices = nullptr;
    splitInstanceBindRegionCount = in_struct->splitInstanceBindRegionCount;
    pSplitInstanceBindRegions = nullptr;
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
    if (in_struct->pSplitInstanceBindRegions) {
        pSplitInstanceBindRegions = new VkRect2D[in_struct->splitInstanceBindRegionCount];
        memcpy ((void *)pSplitInstanceBindRegions, (void *)in_struct->pSplitInstanceBindRegions, sizeof(VkRect2D)*in_struct->splitInstanceBindRegionCount);
    }
}

void safe_VkBindImageMemoryDeviceGroupInfo::initialize(const safe_VkBindImageMemoryDeviceGroupInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceIndexCount = src->deviceIndexCount;
    pDeviceIndices = nullptr;
    splitInstanceBindRegionCount = src->splitInstanceBindRegionCount;
    pSplitInstanceBindRegions = nullptr;
    if (src->pDeviceIndices) {
        pDeviceIndices = new uint32_t[src->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src->pDeviceIndices, sizeof(uint32_t)*src->deviceIndexCount);
    }
    if (src->pSplitInstanceBindRegions) {
        pSplitInstanceBindRegions = new VkRect2D[src->splitInstanceBindRegionCount];
        memcpy ((void *)pSplitInstanceBindRegions, (void *)src->pSplitInstanceBindRegions, sizeof(VkRect2D)*src->splitInstanceBindRegionCount);
    }
}

safe_VkPhysicalDeviceGroupProperties::safe_VkPhysicalDeviceGroupProperties(const VkPhysicalDeviceGroupProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    physicalDeviceCount(in_struct->physicalDeviceCount),
    subsetAllocation(in_struct->subsetAllocation)
{
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        physicalDevices[i] = in_struct->physicalDevices[i];
    }
}

safe_VkPhysicalDeviceGroupProperties::safe_VkPhysicalDeviceGroupProperties()
{}

safe_VkPhysicalDeviceGroupProperties::safe_VkPhysicalDeviceGroupProperties(const safe_VkPhysicalDeviceGroupProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    physicalDeviceCount = src.physicalDeviceCount;
    subsetAllocation = src.subsetAllocation;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        physicalDevices[i] = src.physicalDevices[i];
    }
}

safe_VkPhysicalDeviceGroupProperties& safe_VkPhysicalDeviceGroupProperties::operator=(const safe_VkPhysicalDeviceGroupProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    physicalDeviceCount = src.physicalDeviceCount;
    subsetAllocation = src.subsetAllocation;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        physicalDevices[i] = src.physicalDevices[i];
    }

    return *this;
}

safe_VkPhysicalDeviceGroupProperties::~safe_VkPhysicalDeviceGroupProperties()
{
}

void safe_VkPhysicalDeviceGroupProperties::initialize(const VkPhysicalDeviceGroupProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    physicalDeviceCount = in_struct->physicalDeviceCount;
    subsetAllocation = in_struct->subsetAllocation;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        physicalDevices[i] = in_struct->physicalDevices[i];
    }
}

void safe_VkPhysicalDeviceGroupProperties::initialize(const safe_VkPhysicalDeviceGroupProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    physicalDeviceCount = src->physicalDeviceCount;
    subsetAllocation = src->subsetAllocation;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        physicalDevices[i] = src->physicalDevices[i];
    }
}

safe_VkDeviceGroupDeviceCreateInfo::safe_VkDeviceGroupDeviceCreateInfo(const VkDeviceGroupDeviceCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    physicalDeviceCount(in_struct->physicalDeviceCount),
    pPhysicalDevices(nullptr)
{
    if (in_struct->pPhysicalDevices) {
        pPhysicalDevices = new VkPhysicalDevice[in_struct->physicalDeviceCount];
        memcpy ((void *)pPhysicalDevices, (void *)in_struct->pPhysicalDevices, sizeof(VkPhysicalDevice)*in_struct->physicalDeviceCount);
    }
}

safe_VkDeviceGroupDeviceCreateInfo::safe_VkDeviceGroupDeviceCreateInfo() :
    pPhysicalDevices(nullptr)
{}

safe_VkDeviceGroupDeviceCreateInfo::safe_VkDeviceGroupDeviceCreateInfo(const safe_VkDeviceGroupDeviceCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    physicalDeviceCount = src.physicalDeviceCount;
    pPhysicalDevices = nullptr;
    if (src.pPhysicalDevices) {
        pPhysicalDevices = new VkPhysicalDevice[src.physicalDeviceCount];
        memcpy ((void *)pPhysicalDevices, (void *)src.pPhysicalDevices, sizeof(VkPhysicalDevice)*src.physicalDeviceCount);
    }
}

safe_VkDeviceGroupDeviceCreateInfo& safe_VkDeviceGroupDeviceCreateInfo::operator=(const safe_VkDeviceGroupDeviceCreateInfo& src)
{
    if (&src == this) return *this;

    if (pPhysicalDevices)
        delete[] pPhysicalDevices;

    sType = src.sType;
    pNext = src.pNext;
    physicalDeviceCount = src.physicalDeviceCount;
    pPhysicalDevices = nullptr;
    if (src.pPhysicalDevices) {
        pPhysicalDevices = new VkPhysicalDevice[src.physicalDeviceCount];
        memcpy ((void *)pPhysicalDevices, (void *)src.pPhysicalDevices, sizeof(VkPhysicalDevice)*src.physicalDeviceCount);
    }

    return *this;
}

safe_VkDeviceGroupDeviceCreateInfo::~safe_VkDeviceGroupDeviceCreateInfo()
{
    if (pPhysicalDevices)
        delete[] pPhysicalDevices;
}

void safe_VkDeviceGroupDeviceCreateInfo::initialize(const VkDeviceGroupDeviceCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    physicalDeviceCount = in_struct->physicalDeviceCount;
    pPhysicalDevices = nullptr;
    if (in_struct->pPhysicalDevices) {
        pPhysicalDevices = new VkPhysicalDevice[in_struct->physicalDeviceCount];
        memcpy ((void *)pPhysicalDevices, (void *)in_struct->pPhysicalDevices, sizeof(VkPhysicalDevice)*in_struct->physicalDeviceCount);
    }
}

void safe_VkDeviceGroupDeviceCreateInfo::initialize(const safe_VkDeviceGroupDeviceCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    physicalDeviceCount = src->physicalDeviceCount;
    pPhysicalDevices = nullptr;
    if (src->pPhysicalDevices) {
        pPhysicalDevices = new VkPhysicalDevice[src->physicalDeviceCount];
        memcpy ((void *)pPhysicalDevices, (void *)src->pPhysicalDevices, sizeof(VkPhysicalDevice)*src->physicalDeviceCount);
    }
}

safe_VkBufferMemoryRequirementsInfo2::safe_VkBufferMemoryRequirementsInfo2(const VkBufferMemoryRequirementsInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    buffer(in_struct->buffer)
{
}

safe_VkBufferMemoryRequirementsInfo2::safe_VkBufferMemoryRequirementsInfo2()
{}

safe_VkBufferMemoryRequirementsInfo2::safe_VkBufferMemoryRequirementsInfo2(const safe_VkBufferMemoryRequirementsInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
}

safe_VkBufferMemoryRequirementsInfo2& safe_VkBufferMemoryRequirementsInfo2::operator=(const safe_VkBufferMemoryRequirementsInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;

    return *this;
}

safe_VkBufferMemoryRequirementsInfo2::~safe_VkBufferMemoryRequirementsInfo2()
{
}

void safe_VkBufferMemoryRequirementsInfo2::initialize(const VkBufferMemoryRequirementsInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    buffer = in_struct->buffer;
}

void safe_VkBufferMemoryRequirementsInfo2::initialize(const safe_VkBufferMemoryRequirementsInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    buffer = src->buffer;
}

safe_VkImageMemoryRequirementsInfo2::safe_VkImageMemoryRequirementsInfo2(const VkImageMemoryRequirementsInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    image(in_struct->image)
{
}

safe_VkImageMemoryRequirementsInfo2::safe_VkImageMemoryRequirementsInfo2()
{}

safe_VkImageMemoryRequirementsInfo2::safe_VkImageMemoryRequirementsInfo2(const safe_VkImageMemoryRequirementsInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
}

safe_VkImageMemoryRequirementsInfo2& safe_VkImageMemoryRequirementsInfo2::operator=(const safe_VkImageMemoryRequirementsInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    image = src.image;

    return *this;
}

safe_VkImageMemoryRequirementsInfo2::~safe_VkImageMemoryRequirementsInfo2()
{
}

void safe_VkImageMemoryRequirementsInfo2::initialize(const VkImageMemoryRequirementsInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    image = in_struct->image;
}

void safe_VkImageMemoryRequirementsInfo2::initialize(const safe_VkImageMemoryRequirementsInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    image = src->image;
}

safe_VkImageSparseMemoryRequirementsInfo2::safe_VkImageSparseMemoryRequirementsInfo2(const VkImageSparseMemoryRequirementsInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    image(in_struct->image)
{
}

safe_VkImageSparseMemoryRequirementsInfo2::safe_VkImageSparseMemoryRequirementsInfo2()
{}

safe_VkImageSparseMemoryRequirementsInfo2::safe_VkImageSparseMemoryRequirementsInfo2(const safe_VkImageSparseMemoryRequirementsInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
}

safe_VkImageSparseMemoryRequirementsInfo2& safe_VkImageSparseMemoryRequirementsInfo2::operator=(const safe_VkImageSparseMemoryRequirementsInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    image = src.image;

    return *this;
}

safe_VkImageSparseMemoryRequirementsInfo2::~safe_VkImageSparseMemoryRequirementsInfo2()
{
}

void safe_VkImageSparseMemoryRequirementsInfo2::initialize(const VkImageSparseMemoryRequirementsInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    image = in_struct->image;
}

void safe_VkImageSparseMemoryRequirementsInfo2::initialize(const safe_VkImageSparseMemoryRequirementsInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    image = src->image;
}

safe_VkMemoryRequirements2::safe_VkMemoryRequirements2(const VkMemoryRequirements2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryRequirements(in_struct->memoryRequirements)
{
}

safe_VkMemoryRequirements2::safe_VkMemoryRequirements2()
{}

safe_VkMemoryRequirements2::safe_VkMemoryRequirements2(const safe_VkMemoryRequirements2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryRequirements = src.memoryRequirements;
}

safe_VkMemoryRequirements2& safe_VkMemoryRequirements2::operator=(const safe_VkMemoryRequirements2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryRequirements = src.memoryRequirements;

    return *this;
}

safe_VkMemoryRequirements2::~safe_VkMemoryRequirements2()
{
}

void safe_VkMemoryRequirements2::initialize(const VkMemoryRequirements2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryRequirements = in_struct->memoryRequirements;
}

void safe_VkMemoryRequirements2::initialize(const safe_VkMemoryRequirements2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryRequirements = src->memoryRequirements;
}

safe_VkSparseImageMemoryRequirements2::safe_VkSparseImageMemoryRequirements2(const VkSparseImageMemoryRequirements2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryRequirements(in_struct->memoryRequirements)
{
}

safe_VkSparseImageMemoryRequirements2::safe_VkSparseImageMemoryRequirements2()
{}

safe_VkSparseImageMemoryRequirements2::safe_VkSparseImageMemoryRequirements2(const safe_VkSparseImageMemoryRequirements2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryRequirements = src.memoryRequirements;
}

safe_VkSparseImageMemoryRequirements2& safe_VkSparseImageMemoryRequirements2::operator=(const safe_VkSparseImageMemoryRequirements2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryRequirements = src.memoryRequirements;

    return *this;
}

safe_VkSparseImageMemoryRequirements2::~safe_VkSparseImageMemoryRequirements2()
{
}

void safe_VkSparseImageMemoryRequirements2::initialize(const VkSparseImageMemoryRequirements2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryRequirements = in_struct->memoryRequirements;
}

void safe_VkSparseImageMemoryRequirements2::initialize(const safe_VkSparseImageMemoryRequirements2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryRequirements = src->memoryRequirements;
}

safe_VkPhysicalDeviceFeatures2::safe_VkPhysicalDeviceFeatures2(const VkPhysicalDeviceFeatures2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    features(in_struct->features)
{
}

safe_VkPhysicalDeviceFeatures2::safe_VkPhysicalDeviceFeatures2()
{}

safe_VkPhysicalDeviceFeatures2::safe_VkPhysicalDeviceFeatures2(const safe_VkPhysicalDeviceFeatures2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    features = src.features;
}

safe_VkPhysicalDeviceFeatures2& safe_VkPhysicalDeviceFeatures2::operator=(const safe_VkPhysicalDeviceFeatures2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    features = src.features;

    return *this;
}

safe_VkPhysicalDeviceFeatures2::~safe_VkPhysicalDeviceFeatures2()
{
}

void safe_VkPhysicalDeviceFeatures2::initialize(const VkPhysicalDeviceFeatures2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    features = in_struct->features;
}

void safe_VkPhysicalDeviceFeatures2::initialize(const safe_VkPhysicalDeviceFeatures2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    features = src->features;
}

safe_VkPhysicalDeviceProperties2::safe_VkPhysicalDeviceProperties2(const VkPhysicalDeviceProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    properties(in_struct->properties)
{
}

safe_VkPhysicalDeviceProperties2::safe_VkPhysicalDeviceProperties2()
{}

safe_VkPhysicalDeviceProperties2::safe_VkPhysicalDeviceProperties2(const safe_VkPhysicalDeviceProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    properties = src.properties;
}

safe_VkPhysicalDeviceProperties2& safe_VkPhysicalDeviceProperties2::operator=(const safe_VkPhysicalDeviceProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    properties = src.properties;

    return *this;
}

safe_VkPhysicalDeviceProperties2::~safe_VkPhysicalDeviceProperties2()
{
}

void safe_VkPhysicalDeviceProperties2::initialize(const VkPhysicalDeviceProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    properties = in_struct->properties;
}

void safe_VkPhysicalDeviceProperties2::initialize(const safe_VkPhysicalDeviceProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    properties = src->properties;
}

safe_VkFormatProperties2::safe_VkFormatProperties2(const VkFormatProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    formatProperties(in_struct->formatProperties)
{
}

safe_VkFormatProperties2::safe_VkFormatProperties2()
{}

safe_VkFormatProperties2::safe_VkFormatProperties2(const safe_VkFormatProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    formatProperties = src.formatProperties;
}

safe_VkFormatProperties2& safe_VkFormatProperties2::operator=(const safe_VkFormatProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    formatProperties = src.formatProperties;

    return *this;
}

safe_VkFormatProperties2::~safe_VkFormatProperties2()
{
}

void safe_VkFormatProperties2::initialize(const VkFormatProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    formatProperties = in_struct->formatProperties;
}

void safe_VkFormatProperties2::initialize(const safe_VkFormatProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    formatProperties = src->formatProperties;
}

safe_VkImageFormatProperties2::safe_VkImageFormatProperties2(const VkImageFormatProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    imageFormatProperties(in_struct->imageFormatProperties)
{
}

safe_VkImageFormatProperties2::safe_VkImageFormatProperties2()
{}

safe_VkImageFormatProperties2::safe_VkImageFormatProperties2(const safe_VkImageFormatProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    imageFormatProperties = src.imageFormatProperties;
}

safe_VkImageFormatProperties2& safe_VkImageFormatProperties2::operator=(const safe_VkImageFormatProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    imageFormatProperties = src.imageFormatProperties;

    return *this;
}

safe_VkImageFormatProperties2::~safe_VkImageFormatProperties2()
{
}

void safe_VkImageFormatProperties2::initialize(const VkImageFormatProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    imageFormatProperties = in_struct->imageFormatProperties;
}

void safe_VkImageFormatProperties2::initialize(const safe_VkImageFormatProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    imageFormatProperties = src->imageFormatProperties;
}

safe_VkPhysicalDeviceImageFormatInfo2::safe_VkPhysicalDeviceImageFormatInfo2(const VkPhysicalDeviceImageFormatInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    format(in_struct->format),
    type(in_struct->type),
    tiling(in_struct->tiling),
    usage(in_struct->usage),
    flags(in_struct->flags)
{
}

safe_VkPhysicalDeviceImageFormatInfo2::safe_VkPhysicalDeviceImageFormatInfo2()
{}

safe_VkPhysicalDeviceImageFormatInfo2::safe_VkPhysicalDeviceImageFormatInfo2(const safe_VkPhysicalDeviceImageFormatInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    type = src.type;
    tiling = src.tiling;
    usage = src.usage;
    flags = src.flags;
}

safe_VkPhysicalDeviceImageFormatInfo2& safe_VkPhysicalDeviceImageFormatInfo2::operator=(const safe_VkPhysicalDeviceImageFormatInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    type = src.type;
    tiling = src.tiling;
    usage = src.usage;
    flags = src.flags;

    return *this;
}

safe_VkPhysicalDeviceImageFormatInfo2::~safe_VkPhysicalDeviceImageFormatInfo2()
{
}

void safe_VkPhysicalDeviceImageFormatInfo2::initialize(const VkPhysicalDeviceImageFormatInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    format = in_struct->format;
    type = in_struct->type;
    tiling = in_struct->tiling;
    usage = in_struct->usage;
    flags = in_struct->flags;
}

void safe_VkPhysicalDeviceImageFormatInfo2::initialize(const safe_VkPhysicalDeviceImageFormatInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    format = src->format;
    type = src->type;
    tiling = src->tiling;
    usage = src->usage;
    flags = src->flags;
}

safe_VkQueueFamilyProperties2::safe_VkQueueFamilyProperties2(const VkQueueFamilyProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    queueFamilyProperties(in_struct->queueFamilyProperties)
{
}

safe_VkQueueFamilyProperties2::safe_VkQueueFamilyProperties2()
{}

safe_VkQueueFamilyProperties2::safe_VkQueueFamilyProperties2(const safe_VkQueueFamilyProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    queueFamilyProperties = src.queueFamilyProperties;
}

safe_VkQueueFamilyProperties2& safe_VkQueueFamilyProperties2::operator=(const safe_VkQueueFamilyProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    queueFamilyProperties = src.queueFamilyProperties;

    return *this;
}

safe_VkQueueFamilyProperties2::~safe_VkQueueFamilyProperties2()
{
}

void safe_VkQueueFamilyProperties2::initialize(const VkQueueFamilyProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    queueFamilyProperties = in_struct->queueFamilyProperties;
}

void safe_VkQueueFamilyProperties2::initialize(const safe_VkQueueFamilyProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    queueFamilyProperties = src->queueFamilyProperties;
}

safe_VkPhysicalDeviceMemoryProperties2::safe_VkPhysicalDeviceMemoryProperties2(const VkPhysicalDeviceMemoryProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryProperties(in_struct->memoryProperties)
{
}

safe_VkPhysicalDeviceMemoryProperties2::safe_VkPhysicalDeviceMemoryProperties2()
{}

safe_VkPhysicalDeviceMemoryProperties2::safe_VkPhysicalDeviceMemoryProperties2(const safe_VkPhysicalDeviceMemoryProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryProperties = src.memoryProperties;
}

safe_VkPhysicalDeviceMemoryProperties2& safe_VkPhysicalDeviceMemoryProperties2::operator=(const safe_VkPhysicalDeviceMemoryProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryProperties = src.memoryProperties;

    return *this;
}

safe_VkPhysicalDeviceMemoryProperties2::~safe_VkPhysicalDeviceMemoryProperties2()
{
}

void safe_VkPhysicalDeviceMemoryProperties2::initialize(const VkPhysicalDeviceMemoryProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryProperties = in_struct->memoryProperties;
}

void safe_VkPhysicalDeviceMemoryProperties2::initialize(const safe_VkPhysicalDeviceMemoryProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryProperties = src->memoryProperties;
}

safe_VkSparseImageFormatProperties2::safe_VkSparseImageFormatProperties2(const VkSparseImageFormatProperties2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    properties(in_struct->properties)
{
}

safe_VkSparseImageFormatProperties2::safe_VkSparseImageFormatProperties2()
{}

safe_VkSparseImageFormatProperties2::safe_VkSparseImageFormatProperties2(const safe_VkSparseImageFormatProperties2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    properties = src.properties;
}

safe_VkSparseImageFormatProperties2& safe_VkSparseImageFormatProperties2::operator=(const safe_VkSparseImageFormatProperties2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    properties = src.properties;

    return *this;
}

safe_VkSparseImageFormatProperties2::~safe_VkSparseImageFormatProperties2()
{
}

void safe_VkSparseImageFormatProperties2::initialize(const VkSparseImageFormatProperties2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    properties = in_struct->properties;
}

void safe_VkSparseImageFormatProperties2::initialize(const safe_VkSparseImageFormatProperties2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    properties = src->properties;
}

safe_VkPhysicalDeviceSparseImageFormatInfo2::safe_VkPhysicalDeviceSparseImageFormatInfo2(const VkPhysicalDeviceSparseImageFormatInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    format(in_struct->format),
    type(in_struct->type),
    samples(in_struct->samples),
    usage(in_struct->usage),
    tiling(in_struct->tiling)
{
}

safe_VkPhysicalDeviceSparseImageFormatInfo2::safe_VkPhysicalDeviceSparseImageFormatInfo2()
{}

safe_VkPhysicalDeviceSparseImageFormatInfo2::safe_VkPhysicalDeviceSparseImageFormatInfo2(const safe_VkPhysicalDeviceSparseImageFormatInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    type = src.type;
    samples = src.samples;
    usage = src.usage;
    tiling = src.tiling;
}

safe_VkPhysicalDeviceSparseImageFormatInfo2& safe_VkPhysicalDeviceSparseImageFormatInfo2::operator=(const safe_VkPhysicalDeviceSparseImageFormatInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    type = src.type;
    samples = src.samples;
    usage = src.usage;
    tiling = src.tiling;

    return *this;
}

safe_VkPhysicalDeviceSparseImageFormatInfo2::~safe_VkPhysicalDeviceSparseImageFormatInfo2()
{
}

void safe_VkPhysicalDeviceSparseImageFormatInfo2::initialize(const VkPhysicalDeviceSparseImageFormatInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    format = in_struct->format;
    type = in_struct->type;
    samples = in_struct->samples;
    usage = in_struct->usage;
    tiling = in_struct->tiling;
}

void safe_VkPhysicalDeviceSparseImageFormatInfo2::initialize(const safe_VkPhysicalDeviceSparseImageFormatInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    format = src->format;
    type = src->type;
    samples = src->samples;
    usage = src->usage;
    tiling = src->tiling;
}

safe_VkPhysicalDevicePointClippingProperties::safe_VkPhysicalDevicePointClippingProperties(const VkPhysicalDevicePointClippingProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pointClippingBehavior(in_struct->pointClippingBehavior)
{
}

safe_VkPhysicalDevicePointClippingProperties::safe_VkPhysicalDevicePointClippingProperties()
{}

safe_VkPhysicalDevicePointClippingProperties::safe_VkPhysicalDevicePointClippingProperties(const safe_VkPhysicalDevicePointClippingProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pointClippingBehavior = src.pointClippingBehavior;
}

safe_VkPhysicalDevicePointClippingProperties& safe_VkPhysicalDevicePointClippingProperties::operator=(const safe_VkPhysicalDevicePointClippingProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pointClippingBehavior = src.pointClippingBehavior;

    return *this;
}

safe_VkPhysicalDevicePointClippingProperties::~safe_VkPhysicalDevicePointClippingProperties()
{
}

void safe_VkPhysicalDevicePointClippingProperties::initialize(const VkPhysicalDevicePointClippingProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pointClippingBehavior = in_struct->pointClippingBehavior;
}

void safe_VkPhysicalDevicePointClippingProperties::initialize(const safe_VkPhysicalDevicePointClippingProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pointClippingBehavior = src->pointClippingBehavior;
}

safe_VkRenderPassInputAttachmentAspectCreateInfo::safe_VkRenderPassInputAttachmentAspectCreateInfo(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    aspectReferenceCount(in_struct->aspectReferenceCount),
    pAspectReferences(nullptr)
{
    if (in_struct->pAspectReferences) {
        pAspectReferences = new VkInputAttachmentAspectReference[in_struct->aspectReferenceCount];
        memcpy ((void *)pAspectReferences, (void *)in_struct->pAspectReferences, sizeof(VkInputAttachmentAspectReference)*in_struct->aspectReferenceCount);
    }
}

safe_VkRenderPassInputAttachmentAspectCreateInfo::safe_VkRenderPassInputAttachmentAspectCreateInfo() :
    pAspectReferences(nullptr)
{}

safe_VkRenderPassInputAttachmentAspectCreateInfo::safe_VkRenderPassInputAttachmentAspectCreateInfo(const safe_VkRenderPassInputAttachmentAspectCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    aspectReferenceCount = src.aspectReferenceCount;
    pAspectReferences = nullptr;
    if (src.pAspectReferences) {
        pAspectReferences = new VkInputAttachmentAspectReference[src.aspectReferenceCount];
        memcpy ((void *)pAspectReferences, (void *)src.pAspectReferences, sizeof(VkInputAttachmentAspectReference)*src.aspectReferenceCount);
    }
}

safe_VkRenderPassInputAttachmentAspectCreateInfo& safe_VkRenderPassInputAttachmentAspectCreateInfo::operator=(const safe_VkRenderPassInputAttachmentAspectCreateInfo& src)
{
    if (&src == this) return *this;

    if (pAspectReferences)
        delete[] pAspectReferences;

    sType = src.sType;
    pNext = src.pNext;
    aspectReferenceCount = src.aspectReferenceCount;
    pAspectReferences = nullptr;
    if (src.pAspectReferences) {
        pAspectReferences = new VkInputAttachmentAspectReference[src.aspectReferenceCount];
        memcpy ((void *)pAspectReferences, (void *)src.pAspectReferences, sizeof(VkInputAttachmentAspectReference)*src.aspectReferenceCount);
    }

    return *this;
}

safe_VkRenderPassInputAttachmentAspectCreateInfo::~safe_VkRenderPassInputAttachmentAspectCreateInfo()
{
    if (pAspectReferences)
        delete[] pAspectReferences;
}

void safe_VkRenderPassInputAttachmentAspectCreateInfo::initialize(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    aspectReferenceCount = in_struct->aspectReferenceCount;
    pAspectReferences = nullptr;
    if (in_struct->pAspectReferences) {
        pAspectReferences = new VkInputAttachmentAspectReference[in_struct->aspectReferenceCount];
        memcpy ((void *)pAspectReferences, (void *)in_struct->pAspectReferences, sizeof(VkInputAttachmentAspectReference)*in_struct->aspectReferenceCount);
    }
}

void safe_VkRenderPassInputAttachmentAspectCreateInfo::initialize(const safe_VkRenderPassInputAttachmentAspectCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    aspectReferenceCount = src->aspectReferenceCount;
    pAspectReferences = nullptr;
    if (src->pAspectReferences) {
        pAspectReferences = new VkInputAttachmentAspectReference[src->aspectReferenceCount];
        memcpy ((void *)pAspectReferences, (void *)src->pAspectReferences, sizeof(VkInputAttachmentAspectReference)*src->aspectReferenceCount);
    }
}

safe_VkImageViewUsageCreateInfo::safe_VkImageViewUsageCreateInfo(const VkImageViewUsageCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    usage(in_struct->usage)
{
}

safe_VkImageViewUsageCreateInfo::safe_VkImageViewUsageCreateInfo()
{}

safe_VkImageViewUsageCreateInfo::safe_VkImageViewUsageCreateInfo(const safe_VkImageViewUsageCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    usage = src.usage;
}

safe_VkImageViewUsageCreateInfo& safe_VkImageViewUsageCreateInfo::operator=(const safe_VkImageViewUsageCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    usage = src.usage;

    return *this;
}

safe_VkImageViewUsageCreateInfo::~safe_VkImageViewUsageCreateInfo()
{
}

void safe_VkImageViewUsageCreateInfo::initialize(const VkImageViewUsageCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    usage = in_struct->usage;
}

void safe_VkImageViewUsageCreateInfo::initialize(const safe_VkImageViewUsageCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    usage = src->usage;
}

safe_VkPipelineTessellationDomainOriginStateCreateInfo::safe_VkPipelineTessellationDomainOriginStateCreateInfo(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    domainOrigin(in_struct->domainOrigin)
{
}

safe_VkPipelineTessellationDomainOriginStateCreateInfo::safe_VkPipelineTessellationDomainOriginStateCreateInfo()
{}

safe_VkPipelineTessellationDomainOriginStateCreateInfo::safe_VkPipelineTessellationDomainOriginStateCreateInfo(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    domainOrigin = src.domainOrigin;
}

safe_VkPipelineTessellationDomainOriginStateCreateInfo& safe_VkPipelineTessellationDomainOriginStateCreateInfo::operator=(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    domainOrigin = src.domainOrigin;

    return *this;
}

safe_VkPipelineTessellationDomainOriginStateCreateInfo::~safe_VkPipelineTessellationDomainOriginStateCreateInfo()
{
}

void safe_VkPipelineTessellationDomainOriginStateCreateInfo::initialize(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    domainOrigin = in_struct->domainOrigin;
}

void safe_VkPipelineTessellationDomainOriginStateCreateInfo::initialize(const safe_VkPipelineTessellationDomainOriginStateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    domainOrigin = src->domainOrigin;
}

safe_VkRenderPassMultiviewCreateInfo::safe_VkRenderPassMultiviewCreateInfo(const VkRenderPassMultiviewCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    subpassCount(in_struct->subpassCount),
    pViewMasks(nullptr),
    dependencyCount(in_struct->dependencyCount),
    pViewOffsets(nullptr),
    correlationMaskCount(in_struct->correlationMaskCount),
    pCorrelationMasks(nullptr)
{
    if (in_struct->pViewMasks) {
        pViewMasks = new uint32_t[in_struct->subpassCount];
        memcpy ((void *)pViewMasks, (void *)in_struct->pViewMasks, sizeof(uint32_t)*in_struct->subpassCount);
    }
    if (in_struct->pViewOffsets) {
        pViewOffsets = new int32_t[in_struct->dependencyCount];
        memcpy ((void *)pViewOffsets, (void *)in_struct->pViewOffsets, sizeof(int32_t)*in_struct->dependencyCount);
    }
    if (in_struct->pCorrelationMasks) {
        pCorrelationMasks = new uint32_t[in_struct->correlationMaskCount];
        memcpy ((void *)pCorrelationMasks, (void *)in_struct->pCorrelationMasks, sizeof(uint32_t)*in_struct->correlationMaskCount);
    }
}

safe_VkRenderPassMultiviewCreateInfo::safe_VkRenderPassMultiviewCreateInfo() :
    pViewMasks(nullptr),
    pViewOffsets(nullptr),
    pCorrelationMasks(nullptr)
{}

safe_VkRenderPassMultiviewCreateInfo::safe_VkRenderPassMultiviewCreateInfo(const safe_VkRenderPassMultiviewCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    subpassCount = src.subpassCount;
    pViewMasks = nullptr;
    dependencyCount = src.dependencyCount;
    pViewOffsets = nullptr;
    correlationMaskCount = src.correlationMaskCount;
    pCorrelationMasks = nullptr;
    if (src.pViewMasks) {
        pViewMasks = new uint32_t[src.subpassCount];
        memcpy ((void *)pViewMasks, (void *)src.pViewMasks, sizeof(uint32_t)*src.subpassCount);
    }
    if (src.pViewOffsets) {
        pViewOffsets = new int32_t[src.dependencyCount];
        memcpy ((void *)pViewOffsets, (void *)src.pViewOffsets, sizeof(int32_t)*src.dependencyCount);
    }
    if (src.pCorrelationMasks) {
        pCorrelationMasks = new uint32_t[src.correlationMaskCount];
        memcpy ((void *)pCorrelationMasks, (void *)src.pCorrelationMasks, sizeof(uint32_t)*src.correlationMaskCount);
    }
}

safe_VkRenderPassMultiviewCreateInfo& safe_VkRenderPassMultiviewCreateInfo::operator=(const safe_VkRenderPassMultiviewCreateInfo& src)
{
    if (&src == this) return *this;

    if (pViewMasks)
        delete[] pViewMasks;
    if (pViewOffsets)
        delete[] pViewOffsets;
    if (pCorrelationMasks)
        delete[] pCorrelationMasks;

    sType = src.sType;
    pNext = src.pNext;
    subpassCount = src.subpassCount;
    pViewMasks = nullptr;
    dependencyCount = src.dependencyCount;
    pViewOffsets = nullptr;
    correlationMaskCount = src.correlationMaskCount;
    pCorrelationMasks = nullptr;
    if (src.pViewMasks) {
        pViewMasks = new uint32_t[src.subpassCount];
        memcpy ((void *)pViewMasks, (void *)src.pViewMasks, sizeof(uint32_t)*src.subpassCount);
    }
    if (src.pViewOffsets) {
        pViewOffsets = new int32_t[src.dependencyCount];
        memcpy ((void *)pViewOffsets, (void *)src.pViewOffsets, sizeof(int32_t)*src.dependencyCount);
    }
    if (src.pCorrelationMasks) {
        pCorrelationMasks = new uint32_t[src.correlationMaskCount];
        memcpy ((void *)pCorrelationMasks, (void *)src.pCorrelationMasks, sizeof(uint32_t)*src.correlationMaskCount);
    }

    return *this;
}

safe_VkRenderPassMultiviewCreateInfo::~safe_VkRenderPassMultiviewCreateInfo()
{
    if (pViewMasks)
        delete[] pViewMasks;
    if (pViewOffsets)
        delete[] pViewOffsets;
    if (pCorrelationMasks)
        delete[] pCorrelationMasks;
}

void safe_VkRenderPassMultiviewCreateInfo::initialize(const VkRenderPassMultiviewCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    subpassCount = in_struct->subpassCount;
    pViewMasks = nullptr;
    dependencyCount = in_struct->dependencyCount;
    pViewOffsets = nullptr;
    correlationMaskCount = in_struct->correlationMaskCount;
    pCorrelationMasks = nullptr;
    if (in_struct->pViewMasks) {
        pViewMasks = new uint32_t[in_struct->subpassCount];
        memcpy ((void *)pViewMasks, (void *)in_struct->pViewMasks, sizeof(uint32_t)*in_struct->subpassCount);
    }
    if (in_struct->pViewOffsets) {
        pViewOffsets = new int32_t[in_struct->dependencyCount];
        memcpy ((void *)pViewOffsets, (void *)in_struct->pViewOffsets, sizeof(int32_t)*in_struct->dependencyCount);
    }
    if (in_struct->pCorrelationMasks) {
        pCorrelationMasks = new uint32_t[in_struct->correlationMaskCount];
        memcpy ((void *)pCorrelationMasks, (void *)in_struct->pCorrelationMasks, sizeof(uint32_t)*in_struct->correlationMaskCount);
    }
}

void safe_VkRenderPassMultiviewCreateInfo::initialize(const safe_VkRenderPassMultiviewCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    subpassCount = src->subpassCount;
    pViewMasks = nullptr;
    dependencyCount = src->dependencyCount;
    pViewOffsets = nullptr;
    correlationMaskCount = src->correlationMaskCount;
    pCorrelationMasks = nullptr;
    if (src->pViewMasks) {
        pViewMasks = new uint32_t[src->subpassCount];
        memcpy ((void *)pViewMasks, (void *)src->pViewMasks, sizeof(uint32_t)*src->subpassCount);
    }
    if (src->pViewOffsets) {
        pViewOffsets = new int32_t[src->dependencyCount];
        memcpy ((void *)pViewOffsets, (void *)src->pViewOffsets, sizeof(int32_t)*src->dependencyCount);
    }
    if (src->pCorrelationMasks) {
        pCorrelationMasks = new uint32_t[src->correlationMaskCount];
        memcpy ((void *)pCorrelationMasks, (void *)src->pCorrelationMasks, sizeof(uint32_t)*src->correlationMaskCount);
    }
}

safe_VkPhysicalDeviceMultiviewFeatures::safe_VkPhysicalDeviceMultiviewFeatures(const VkPhysicalDeviceMultiviewFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    multiview(in_struct->multiview),
    multiviewGeometryShader(in_struct->multiviewGeometryShader),
    multiviewTessellationShader(in_struct->multiviewTessellationShader)
{
}

safe_VkPhysicalDeviceMultiviewFeatures::safe_VkPhysicalDeviceMultiviewFeatures()
{}

safe_VkPhysicalDeviceMultiviewFeatures::safe_VkPhysicalDeviceMultiviewFeatures(const safe_VkPhysicalDeviceMultiviewFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    multiview = src.multiview;
    multiviewGeometryShader = src.multiviewGeometryShader;
    multiviewTessellationShader = src.multiviewTessellationShader;
}

safe_VkPhysicalDeviceMultiviewFeatures& safe_VkPhysicalDeviceMultiviewFeatures::operator=(const safe_VkPhysicalDeviceMultiviewFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    multiview = src.multiview;
    multiviewGeometryShader = src.multiviewGeometryShader;
    multiviewTessellationShader = src.multiviewTessellationShader;

    return *this;
}

safe_VkPhysicalDeviceMultiviewFeatures::~safe_VkPhysicalDeviceMultiviewFeatures()
{
}

void safe_VkPhysicalDeviceMultiviewFeatures::initialize(const VkPhysicalDeviceMultiviewFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    multiview = in_struct->multiview;
    multiviewGeometryShader = in_struct->multiviewGeometryShader;
    multiviewTessellationShader = in_struct->multiviewTessellationShader;
}

void safe_VkPhysicalDeviceMultiviewFeatures::initialize(const safe_VkPhysicalDeviceMultiviewFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    multiview = src->multiview;
    multiviewGeometryShader = src->multiviewGeometryShader;
    multiviewTessellationShader = src->multiviewTessellationShader;
}

safe_VkPhysicalDeviceMultiviewProperties::safe_VkPhysicalDeviceMultiviewProperties(const VkPhysicalDeviceMultiviewProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxMultiviewViewCount(in_struct->maxMultiviewViewCount),
    maxMultiviewInstanceIndex(in_struct->maxMultiviewInstanceIndex)
{
}

safe_VkPhysicalDeviceMultiviewProperties::safe_VkPhysicalDeviceMultiviewProperties()
{}

safe_VkPhysicalDeviceMultiviewProperties::safe_VkPhysicalDeviceMultiviewProperties(const safe_VkPhysicalDeviceMultiviewProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxMultiviewViewCount = src.maxMultiviewViewCount;
    maxMultiviewInstanceIndex = src.maxMultiviewInstanceIndex;
}

safe_VkPhysicalDeviceMultiviewProperties& safe_VkPhysicalDeviceMultiviewProperties::operator=(const safe_VkPhysicalDeviceMultiviewProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxMultiviewViewCount = src.maxMultiviewViewCount;
    maxMultiviewInstanceIndex = src.maxMultiviewInstanceIndex;

    return *this;
}

safe_VkPhysicalDeviceMultiviewProperties::~safe_VkPhysicalDeviceMultiviewProperties()
{
}

void safe_VkPhysicalDeviceMultiviewProperties::initialize(const VkPhysicalDeviceMultiviewProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxMultiviewViewCount = in_struct->maxMultiviewViewCount;
    maxMultiviewInstanceIndex = in_struct->maxMultiviewInstanceIndex;
}

void safe_VkPhysicalDeviceMultiviewProperties::initialize(const safe_VkPhysicalDeviceMultiviewProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxMultiviewViewCount = src->maxMultiviewViewCount;
    maxMultiviewInstanceIndex = src->maxMultiviewInstanceIndex;
}

safe_VkPhysicalDeviceVariablePointersFeatures::safe_VkPhysicalDeviceVariablePointersFeatures(const VkPhysicalDeviceVariablePointersFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    variablePointersStorageBuffer(in_struct->variablePointersStorageBuffer),
    variablePointers(in_struct->variablePointers)
{
}

safe_VkPhysicalDeviceVariablePointersFeatures::safe_VkPhysicalDeviceVariablePointersFeatures()
{}

safe_VkPhysicalDeviceVariablePointersFeatures::safe_VkPhysicalDeviceVariablePointersFeatures(const safe_VkPhysicalDeviceVariablePointersFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    variablePointersStorageBuffer = src.variablePointersStorageBuffer;
    variablePointers = src.variablePointers;
}

safe_VkPhysicalDeviceVariablePointersFeatures& safe_VkPhysicalDeviceVariablePointersFeatures::operator=(const safe_VkPhysicalDeviceVariablePointersFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    variablePointersStorageBuffer = src.variablePointersStorageBuffer;
    variablePointers = src.variablePointers;

    return *this;
}

safe_VkPhysicalDeviceVariablePointersFeatures::~safe_VkPhysicalDeviceVariablePointersFeatures()
{
}

void safe_VkPhysicalDeviceVariablePointersFeatures::initialize(const VkPhysicalDeviceVariablePointersFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    variablePointersStorageBuffer = in_struct->variablePointersStorageBuffer;
    variablePointers = in_struct->variablePointers;
}

void safe_VkPhysicalDeviceVariablePointersFeatures::initialize(const safe_VkPhysicalDeviceVariablePointersFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    variablePointersStorageBuffer = src->variablePointersStorageBuffer;
    variablePointers = src->variablePointers;
}

safe_VkPhysicalDeviceProtectedMemoryFeatures::safe_VkPhysicalDeviceProtectedMemoryFeatures(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    protectedMemory(in_struct->protectedMemory)
{
}

safe_VkPhysicalDeviceProtectedMemoryFeatures::safe_VkPhysicalDeviceProtectedMemoryFeatures()
{}

safe_VkPhysicalDeviceProtectedMemoryFeatures::safe_VkPhysicalDeviceProtectedMemoryFeatures(const safe_VkPhysicalDeviceProtectedMemoryFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    protectedMemory = src.protectedMemory;
}

safe_VkPhysicalDeviceProtectedMemoryFeatures& safe_VkPhysicalDeviceProtectedMemoryFeatures::operator=(const safe_VkPhysicalDeviceProtectedMemoryFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    protectedMemory = src.protectedMemory;

    return *this;
}

safe_VkPhysicalDeviceProtectedMemoryFeatures::~safe_VkPhysicalDeviceProtectedMemoryFeatures()
{
}

void safe_VkPhysicalDeviceProtectedMemoryFeatures::initialize(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    protectedMemory = in_struct->protectedMemory;
}

void safe_VkPhysicalDeviceProtectedMemoryFeatures::initialize(const safe_VkPhysicalDeviceProtectedMemoryFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    protectedMemory = src->protectedMemory;
}

safe_VkPhysicalDeviceProtectedMemoryProperties::safe_VkPhysicalDeviceProtectedMemoryProperties(const VkPhysicalDeviceProtectedMemoryProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    protectedNoFault(in_struct->protectedNoFault)
{
}

safe_VkPhysicalDeviceProtectedMemoryProperties::safe_VkPhysicalDeviceProtectedMemoryProperties()
{}

safe_VkPhysicalDeviceProtectedMemoryProperties::safe_VkPhysicalDeviceProtectedMemoryProperties(const safe_VkPhysicalDeviceProtectedMemoryProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    protectedNoFault = src.protectedNoFault;
}

safe_VkPhysicalDeviceProtectedMemoryProperties& safe_VkPhysicalDeviceProtectedMemoryProperties::operator=(const safe_VkPhysicalDeviceProtectedMemoryProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    protectedNoFault = src.protectedNoFault;

    return *this;
}

safe_VkPhysicalDeviceProtectedMemoryProperties::~safe_VkPhysicalDeviceProtectedMemoryProperties()
{
}

void safe_VkPhysicalDeviceProtectedMemoryProperties::initialize(const VkPhysicalDeviceProtectedMemoryProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    protectedNoFault = in_struct->protectedNoFault;
}

void safe_VkPhysicalDeviceProtectedMemoryProperties::initialize(const safe_VkPhysicalDeviceProtectedMemoryProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    protectedNoFault = src->protectedNoFault;
}

safe_VkDeviceQueueInfo2::safe_VkDeviceQueueInfo2(const VkDeviceQueueInfo2* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    queueFamilyIndex(in_struct->queueFamilyIndex),
    queueIndex(in_struct->queueIndex)
{
}

safe_VkDeviceQueueInfo2::safe_VkDeviceQueueInfo2()
{}

safe_VkDeviceQueueInfo2::safe_VkDeviceQueueInfo2(const safe_VkDeviceQueueInfo2& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;
    queueIndex = src.queueIndex;
}

safe_VkDeviceQueueInfo2& safe_VkDeviceQueueInfo2::operator=(const safe_VkDeviceQueueInfo2& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    queueFamilyIndex = src.queueFamilyIndex;
    queueIndex = src.queueIndex;

    return *this;
}

safe_VkDeviceQueueInfo2::~safe_VkDeviceQueueInfo2()
{
}

void safe_VkDeviceQueueInfo2::initialize(const VkDeviceQueueInfo2* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    queueIndex = in_struct->queueIndex;
}

void safe_VkDeviceQueueInfo2::initialize(const safe_VkDeviceQueueInfo2* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    queueFamilyIndex = src->queueFamilyIndex;
    queueIndex = src->queueIndex;
}

safe_VkProtectedSubmitInfo::safe_VkProtectedSubmitInfo(const VkProtectedSubmitInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    protectedSubmit(in_struct->protectedSubmit)
{
}

safe_VkProtectedSubmitInfo::safe_VkProtectedSubmitInfo()
{}

safe_VkProtectedSubmitInfo::safe_VkProtectedSubmitInfo(const safe_VkProtectedSubmitInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    protectedSubmit = src.protectedSubmit;
}

safe_VkProtectedSubmitInfo& safe_VkProtectedSubmitInfo::operator=(const safe_VkProtectedSubmitInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    protectedSubmit = src.protectedSubmit;

    return *this;
}

safe_VkProtectedSubmitInfo::~safe_VkProtectedSubmitInfo()
{
}

void safe_VkProtectedSubmitInfo::initialize(const VkProtectedSubmitInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    protectedSubmit = in_struct->protectedSubmit;
}

void safe_VkProtectedSubmitInfo::initialize(const safe_VkProtectedSubmitInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    protectedSubmit = src->protectedSubmit;
}

safe_VkSamplerYcbcrConversionCreateInfo::safe_VkSamplerYcbcrConversionCreateInfo(const VkSamplerYcbcrConversionCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    format(in_struct->format),
    ycbcrModel(in_struct->ycbcrModel),
    ycbcrRange(in_struct->ycbcrRange),
    components(in_struct->components),
    xChromaOffset(in_struct->xChromaOffset),
    yChromaOffset(in_struct->yChromaOffset),
    chromaFilter(in_struct->chromaFilter),
    forceExplicitReconstruction(in_struct->forceExplicitReconstruction)
{
}

safe_VkSamplerYcbcrConversionCreateInfo::safe_VkSamplerYcbcrConversionCreateInfo()
{}

safe_VkSamplerYcbcrConversionCreateInfo::safe_VkSamplerYcbcrConversionCreateInfo(const safe_VkSamplerYcbcrConversionCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    ycbcrModel = src.ycbcrModel;
    ycbcrRange = src.ycbcrRange;
    components = src.components;
    xChromaOffset = src.xChromaOffset;
    yChromaOffset = src.yChromaOffset;
    chromaFilter = src.chromaFilter;
    forceExplicitReconstruction = src.forceExplicitReconstruction;
}

safe_VkSamplerYcbcrConversionCreateInfo& safe_VkSamplerYcbcrConversionCreateInfo::operator=(const safe_VkSamplerYcbcrConversionCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    ycbcrModel = src.ycbcrModel;
    ycbcrRange = src.ycbcrRange;
    components = src.components;
    xChromaOffset = src.xChromaOffset;
    yChromaOffset = src.yChromaOffset;
    chromaFilter = src.chromaFilter;
    forceExplicitReconstruction = src.forceExplicitReconstruction;

    return *this;
}

safe_VkSamplerYcbcrConversionCreateInfo::~safe_VkSamplerYcbcrConversionCreateInfo()
{
}

void safe_VkSamplerYcbcrConversionCreateInfo::initialize(const VkSamplerYcbcrConversionCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    format = in_struct->format;
    ycbcrModel = in_struct->ycbcrModel;
    ycbcrRange = in_struct->ycbcrRange;
    components = in_struct->components;
    xChromaOffset = in_struct->xChromaOffset;
    yChromaOffset = in_struct->yChromaOffset;
    chromaFilter = in_struct->chromaFilter;
    forceExplicitReconstruction = in_struct->forceExplicitReconstruction;
}

void safe_VkSamplerYcbcrConversionCreateInfo::initialize(const safe_VkSamplerYcbcrConversionCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    format = src->format;
    ycbcrModel = src->ycbcrModel;
    ycbcrRange = src->ycbcrRange;
    components = src->components;
    xChromaOffset = src->xChromaOffset;
    yChromaOffset = src->yChromaOffset;
    chromaFilter = src->chromaFilter;
    forceExplicitReconstruction = src->forceExplicitReconstruction;
}

safe_VkSamplerYcbcrConversionInfo::safe_VkSamplerYcbcrConversionInfo(const VkSamplerYcbcrConversionInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    conversion(in_struct->conversion)
{
}

safe_VkSamplerYcbcrConversionInfo::safe_VkSamplerYcbcrConversionInfo()
{}

safe_VkSamplerYcbcrConversionInfo::safe_VkSamplerYcbcrConversionInfo(const safe_VkSamplerYcbcrConversionInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    conversion = src.conversion;
}

safe_VkSamplerYcbcrConversionInfo& safe_VkSamplerYcbcrConversionInfo::operator=(const safe_VkSamplerYcbcrConversionInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    conversion = src.conversion;

    return *this;
}

safe_VkSamplerYcbcrConversionInfo::~safe_VkSamplerYcbcrConversionInfo()
{
}

void safe_VkSamplerYcbcrConversionInfo::initialize(const VkSamplerYcbcrConversionInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    conversion = in_struct->conversion;
}

void safe_VkSamplerYcbcrConversionInfo::initialize(const safe_VkSamplerYcbcrConversionInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    conversion = src->conversion;
}

safe_VkBindImagePlaneMemoryInfo::safe_VkBindImagePlaneMemoryInfo(const VkBindImagePlaneMemoryInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    planeAspect(in_struct->planeAspect)
{
}

safe_VkBindImagePlaneMemoryInfo::safe_VkBindImagePlaneMemoryInfo()
{}

safe_VkBindImagePlaneMemoryInfo::safe_VkBindImagePlaneMemoryInfo(const safe_VkBindImagePlaneMemoryInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    planeAspect = src.planeAspect;
}

safe_VkBindImagePlaneMemoryInfo& safe_VkBindImagePlaneMemoryInfo::operator=(const safe_VkBindImagePlaneMemoryInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    planeAspect = src.planeAspect;

    return *this;
}

safe_VkBindImagePlaneMemoryInfo::~safe_VkBindImagePlaneMemoryInfo()
{
}

void safe_VkBindImagePlaneMemoryInfo::initialize(const VkBindImagePlaneMemoryInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    planeAspect = in_struct->planeAspect;
}

void safe_VkBindImagePlaneMemoryInfo::initialize(const safe_VkBindImagePlaneMemoryInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    planeAspect = src->planeAspect;
}

safe_VkImagePlaneMemoryRequirementsInfo::safe_VkImagePlaneMemoryRequirementsInfo(const VkImagePlaneMemoryRequirementsInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    planeAspect(in_struct->planeAspect)
{
}

safe_VkImagePlaneMemoryRequirementsInfo::safe_VkImagePlaneMemoryRequirementsInfo()
{}

safe_VkImagePlaneMemoryRequirementsInfo::safe_VkImagePlaneMemoryRequirementsInfo(const safe_VkImagePlaneMemoryRequirementsInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    planeAspect = src.planeAspect;
}

safe_VkImagePlaneMemoryRequirementsInfo& safe_VkImagePlaneMemoryRequirementsInfo::operator=(const safe_VkImagePlaneMemoryRequirementsInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    planeAspect = src.planeAspect;

    return *this;
}

safe_VkImagePlaneMemoryRequirementsInfo::~safe_VkImagePlaneMemoryRequirementsInfo()
{
}

void safe_VkImagePlaneMemoryRequirementsInfo::initialize(const VkImagePlaneMemoryRequirementsInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    planeAspect = in_struct->planeAspect;
}

void safe_VkImagePlaneMemoryRequirementsInfo::initialize(const safe_VkImagePlaneMemoryRequirementsInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    planeAspect = src->planeAspect;
}

safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    samplerYcbcrConversion(in_struct->samplerYcbcrConversion)
{
}

safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures()
{}

safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    samplerYcbcrConversion = src.samplerYcbcrConversion;
}

safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::operator=(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    samplerYcbcrConversion = src.samplerYcbcrConversion;

    return *this;
}

safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::~safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures()
{
}

void safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::initialize(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    samplerYcbcrConversion = in_struct->samplerYcbcrConversion;
}

void safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures::initialize(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    samplerYcbcrConversion = src->samplerYcbcrConversion;
}

safe_VkSamplerYcbcrConversionImageFormatProperties::safe_VkSamplerYcbcrConversionImageFormatProperties(const VkSamplerYcbcrConversionImageFormatProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    combinedImageSamplerDescriptorCount(in_struct->combinedImageSamplerDescriptorCount)
{
}

safe_VkSamplerYcbcrConversionImageFormatProperties::safe_VkSamplerYcbcrConversionImageFormatProperties()
{}

safe_VkSamplerYcbcrConversionImageFormatProperties::safe_VkSamplerYcbcrConversionImageFormatProperties(const safe_VkSamplerYcbcrConversionImageFormatProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    combinedImageSamplerDescriptorCount = src.combinedImageSamplerDescriptorCount;
}

safe_VkSamplerYcbcrConversionImageFormatProperties& safe_VkSamplerYcbcrConversionImageFormatProperties::operator=(const safe_VkSamplerYcbcrConversionImageFormatProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    combinedImageSamplerDescriptorCount = src.combinedImageSamplerDescriptorCount;

    return *this;
}

safe_VkSamplerYcbcrConversionImageFormatProperties::~safe_VkSamplerYcbcrConversionImageFormatProperties()
{
}

void safe_VkSamplerYcbcrConversionImageFormatProperties::initialize(const VkSamplerYcbcrConversionImageFormatProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    combinedImageSamplerDescriptorCount = in_struct->combinedImageSamplerDescriptorCount;
}

void safe_VkSamplerYcbcrConversionImageFormatProperties::initialize(const safe_VkSamplerYcbcrConversionImageFormatProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    combinedImageSamplerDescriptorCount = src->combinedImageSamplerDescriptorCount;
}

safe_VkDescriptorUpdateTemplateCreateInfo::safe_VkDescriptorUpdateTemplateCreateInfo(const VkDescriptorUpdateTemplateCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    descriptorUpdateEntryCount(in_struct->descriptorUpdateEntryCount),
    pDescriptorUpdateEntries(nullptr),
    templateType(in_struct->templateType),
    descriptorSetLayout(in_struct->descriptorSetLayout),
    pipelineBindPoint(in_struct->pipelineBindPoint),
    pipelineLayout(in_struct->pipelineLayout),
    set(in_struct->set)
{
    if (in_struct->pDescriptorUpdateEntries) {
        pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry[in_struct->descriptorUpdateEntryCount];
        memcpy ((void *)pDescriptorUpdateEntries, (void *)in_struct->pDescriptorUpdateEntries, sizeof(VkDescriptorUpdateTemplateEntry)*in_struct->descriptorUpdateEntryCount);
    }
}

safe_VkDescriptorUpdateTemplateCreateInfo::safe_VkDescriptorUpdateTemplateCreateInfo() :
    pDescriptorUpdateEntries(nullptr)
{}

safe_VkDescriptorUpdateTemplateCreateInfo::safe_VkDescriptorUpdateTemplateCreateInfo(const safe_VkDescriptorUpdateTemplateCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    descriptorUpdateEntryCount = src.descriptorUpdateEntryCount;
    pDescriptorUpdateEntries = nullptr;
    templateType = src.templateType;
    descriptorSetLayout = src.descriptorSetLayout;
    pipelineBindPoint = src.pipelineBindPoint;
    pipelineLayout = src.pipelineLayout;
    set = src.set;
    if (src.pDescriptorUpdateEntries) {
        pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry[src.descriptorUpdateEntryCount];
        memcpy ((void *)pDescriptorUpdateEntries, (void *)src.pDescriptorUpdateEntries, sizeof(VkDescriptorUpdateTemplateEntry)*src.descriptorUpdateEntryCount);
    }
}

safe_VkDescriptorUpdateTemplateCreateInfo& safe_VkDescriptorUpdateTemplateCreateInfo::operator=(const safe_VkDescriptorUpdateTemplateCreateInfo& src)
{
    if (&src == this) return *this;

    if (pDescriptorUpdateEntries)
        delete[] pDescriptorUpdateEntries;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    descriptorUpdateEntryCount = src.descriptorUpdateEntryCount;
    pDescriptorUpdateEntries = nullptr;
    templateType = src.templateType;
    descriptorSetLayout = src.descriptorSetLayout;
    pipelineBindPoint = src.pipelineBindPoint;
    pipelineLayout = src.pipelineLayout;
    set = src.set;
    if (src.pDescriptorUpdateEntries) {
        pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry[src.descriptorUpdateEntryCount];
        memcpy ((void *)pDescriptorUpdateEntries, (void *)src.pDescriptorUpdateEntries, sizeof(VkDescriptorUpdateTemplateEntry)*src.descriptorUpdateEntryCount);
    }

    return *this;
}

safe_VkDescriptorUpdateTemplateCreateInfo::~safe_VkDescriptorUpdateTemplateCreateInfo()
{
    if (pDescriptorUpdateEntries)
        delete[] pDescriptorUpdateEntries;
}

void safe_VkDescriptorUpdateTemplateCreateInfo::initialize(const VkDescriptorUpdateTemplateCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    descriptorUpdateEntryCount = in_struct->descriptorUpdateEntryCount;
    pDescriptorUpdateEntries = nullptr;
    templateType = in_struct->templateType;
    descriptorSetLayout = in_struct->descriptorSetLayout;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    pipelineLayout = in_struct->pipelineLayout;
    set = in_struct->set;
    if (in_struct->pDescriptorUpdateEntries) {
        pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry[in_struct->descriptorUpdateEntryCount];
        memcpy ((void *)pDescriptorUpdateEntries, (void *)in_struct->pDescriptorUpdateEntries, sizeof(VkDescriptorUpdateTemplateEntry)*in_struct->descriptorUpdateEntryCount);
    }
}

void safe_VkDescriptorUpdateTemplateCreateInfo::initialize(const safe_VkDescriptorUpdateTemplateCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    descriptorUpdateEntryCount = src->descriptorUpdateEntryCount;
    pDescriptorUpdateEntries = nullptr;
    templateType = src->templateType;
    descriptorSetLayout = src->descriptorSetLayout;
    pipelineBindPoint = src->pipelineBindPoint;
    pipelineLayout = src->pipelineLayout;
    set = src->set;
    if (src->pDescriptorUpdateEntries) {
        pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry[src->descriptorUpdateEntryCount];
        memcpy ((void *)pDescriptorUpdateEntries, (void *)src->pDescriptorUpdateEntries, sizeof(VkDescriptorUpdateTemplateEntry)*src->descriptorUpdateEntryCount);
    }
}

safe_VkPhysicalDeviceExternalImageFormatInfo::safe_VkPhysicalDeviceExternalImageFormatInfo(const VkPhysicalDeviceExternalImageFormatInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType)
{
}

safe_VkPhysicalDeviceExternalImageFormatInfo::safe_VkPhysicalDeviceExternalImageFormatInfo()
{}

safe_VkPhysicalDeviceExternalImageFormatInfo::safe_VkPhysicalDeviceExternalImageFormatInfo(const safe_VkPhysicalDeviceExternalImageFormatInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
}

safe_VkPhysicalDeviceExternalImageFormatInfo& safe_VkPhysicalDeviceExternalImageFormatInfo::operator=(const safe_VkPhysicalDeviceExternalImageFormatInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;

    return *this;
}

safe_VkPhysicalDeviceExternalImageFormatInfo::~safe_VkPhysicalDeviceExternalImageFormatInfo()
{
}

void safe_VkPhysicalDeviceExternalImageFormatInfo::initialize(const VkPhysicalDeviceExternalImageFormatInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
}

void safe_VkPhysicalDeviceExternalImageFormatInfo::initialize(const safe_VkPhysicalDeviceExternalImageFormatInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
}

safe_VkExternalImageFormatProperties::safe_VkExternalImageFormatProperties(const VkExternalImageFormatProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    externalMemoryProperties(in_struct->externalMemoryProperties)
{
}

safe_VkExternalImageFormatProperties::safe_VkExternalImageFormatProperties()
{}

safe_VkExternalImageFormatProperties::safe_VkExternalImageFormatProperties(const safe_VkExternalImageFormatProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    externalMemoryProperties = src.externalMemoryProperties;
}

safe_VkExternalImageFormatProperties& safe_VkExternalImageFormatProperties::operator=(const safe_VkExternalImageFormatProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    externalMemoryProperties = src.externalMemoryProperties;

    return *this;
}

safe_VkExternalImageFormatProperties::~safe_VkExternalImageFormatProperties()
{
}

void safe_VkExternalImageFormatProperties::initialize(const VkExternalImageFormatProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    externalMemoryProperties = in_struct->externalMemoryProperties;
}

void safe_VkExternalImageFormatProperties::initialize(const safe_VkExternalImageFormatProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    externalMemoryProperties = src->externalMemoryProperties;
}

safe_VkPhysicalDeviceExternalBufferInfo::safe_VkPhysicalDeviceExternalBufferInfo(const VkPhysicalDeviceExternalBufferInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    usage(in_struct->usage),
    handleType(in_struct->handleType)
{
}

safe_VkPhysicalDeviceExternalBufferInfo::safe_VkPhysicalDeviceExternalBufferInfo()
{}

safe_VkPhysicalDeviceExternalBufferInfo::safe_VkPhysicalDeviceExternalBufferInfo(const safe_VkPhysicalDeviceExternalBufferInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    usage = src.usage;
    handleType = src.handleType;
}

safe_VkPhysicalDeviceExternalBufferInfo& safe_VkPhysicalDeviceExternalBufferInfo::operator=(const safe_VkPhysicalDeviceExternalBufferInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    usage = src.usage;
    handleType = src.handleType;

    return *this;
}

safe_VkPhysicalDeviceExternalBufferInfo::~safe_VkPhysicalDeviceExternalBufferInfo()
{
}

void safe_VkPhysicalDeviceExternalBufferInfo::initialize(const VkPhysicalDeviceExternalBufferInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    usage = in_struct->usage;
    handleType = in_struct->handleType;
}

void safe_VkPhysicalDeviceExternalBufferInfo::initialize(const safe_VkPhysicalDeviceExternalBufferInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    usage = src->usage;
    handleType = src->handleType;
}

safe_VkExternalBufferProperties::safe_VkExternalBufferProperties(const VkExternalBufferProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    externalMemoryProperties(in_struct->externalMemoryProperties)
{
}

safe_VkExternalBufferProperties::safe_VkExternalBufferProperties()
{}

safe_VkExternalBufferProperties::safe_VkExternalBufferProperties(const safe_VkExternalBufferProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    externalMemoryProperties = src.externalMemoryProperties;
}

safe_VkExternalBufferProperties& safe_VkExternalBufferProperties::operator=(const safe_VkExternalBufferProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    externalMemoryProperties = src.externalMemoryProperties;

    return *this;
}

safe_VkExternalBufferProperties::~safe_VkExternalBufferProperties()
{
}

void safe_VkExternalBufferProperties::initialize(const VkExternalBufferProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    externalMemoryProperties = in_struct->externalMemoryProperties;
}

void safe_VkExternalBufferProperties::initialize(const safe_VkExternalBufferProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    externalMemoryProperties = src->externalMemoryProperties;
}

safe_VkPhysicalDeviceIDProperties::safe_VkPhysicalDeviceIDProperties(const VkPhysicalDeviceIDProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceNodeMask(in_struct->deviceNodeMask),
    deviceLUIDValid(in_struct->deviceLUIDValid)
{
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        deviceUUID[i] = in_struct->deviceUUID[i];
    }
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        driverUUID[i] = in_struct->driverUUID[i];
    }
    for (uint32_t i=0; i<VK_LUID_SIZE; ++i) {
        deviceLUID[i] = in_struct->deviceLUID[i];
    }
}

safe_VkPhysicalDeviceIDProperties::safe_VkPhysicalDeviceIDProperties()
{}

safe_VkPhysicalDeviceIDProperties::safe_VkPhysicalDeviceIDProperties(const safe_VkPhysicalDeviceIDProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceNodeMask = src.deviceNodeMask;
    deviceLUIDValid = src.deviceLUIDValid;
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        deviceUUID[i] = src.deviceUUID[i];
    }
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        driverUUID[i] = src.driverUUID[i];
    }
    for (uint32_t i=0; i<VK_LUID_SIZE; ++i) {
        deviceLUID[i] = src.deviceLUID[i];
    }
}

safe_VkPhysicalDeviceIDProperties& safe_VkPhysicalDeviceIDProperties::operator=(const safe_VkPhysicalDeviceIDProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    deviceNodeMask = src.deviceNodeMask;
    deviceLUIDValid = src.deviceLUIDValid;
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        deviceUUID[i] = src.deviceUUID[i];
    }
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        driverUUID[i] = src.driverUUID[i];
    }
    for (uint32_t i=0; i<VK_LUID_SIZE; ++i) {
        deviceLUID[i] = src.deviceLUID[i];
    }

    return *this;
}

safe_VkPhysicalDeviceIDProperties::~safe_VkPhysicalDeviceIDProperties()
{
}

void safe_VkPhysicalDeviceIDProperties::initialize(const VkPhysicalDeviceIDProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceNodeMask = in_struct->deviceNodeMask;
    deviceLUIDValid = in_struct->deviceLUIDValid;
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        deviceUUID[i] = in_struct->deviceUUID[i];
    }
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        driverUUID[i] = in_struct->driverUUID[i];
    }
    for (uint32_t i=0; i<VK_LUID_SIZE; ++i) {
        deviceLUID[i] = in_struct->deviceLUID[i];
    }
}

void safe_VkPhysicalDeviceIDProperties::initialize(const safe_VkPhysicalDeviceIDProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceNodeMask = src->deviceNodeMask;
    deviceLUIDValid = src->deviceLUIDValid;
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        deviceUUID[i] = src->deviceUUID[i];
    }
    for (uint32_t i=0; i<VK_UUID_SIZE; ++i) {
        driverUUID[i] = src->driverUUID[i];
    }
    for (uint32_t i=0; i<VK_LUID_SIZE; ++i) {
        deviceLUID[i] = src->deviceLUID[i];
    }
}

safe_VkExternalMemoryImageCreateInfo::safe_VkExternalMemoryImageCreateInfo(const VkExternalMemoryImageCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExternalMemoryImageCreateInfo::safe_VkExternalMemoryImageCreateInfo()
{}

safe_VkExternalMemoryImageCreateInfo::safe_VkExternalMemoryImageCreateInfo(const safe_VkExternalMemoryImageCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExternalMemoryImageCreateInfo& safe_VkExternalMemoryImageCreateInfo::operator=(const safe_VkExternalMemoryImageCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExternalMemoryImageCreateInfo::~safe_VkExternalMemoryImageCreateInfo()
{
}

void safe_VkExternalMemoryImageCreateInfo::initialize(const VkExternalMemoryImageCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExternalMemoryImageCreateInfo::initialize(const safe_VkExternalMemoryImageCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkExternalMemoryBufferCreateInfo::safe_VkExternalMemoryBufferCreateInfo(const VkExternalMemoryBufferCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExternalMemoryBufferCreateInfo::safe_VkExternalMemoryBufferCreateInfo()
{}

safe_VkExternalMemoryBufferCreateInfo::safe_VkExternalMemoryBufferCreateInfo(const safe_VkExternalMemoryBufferCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExternalMemoryBufferCreateInfo& safe_VkExternalMemoryBufferCreateInfo::operator=(const safe_VkExternalMemoryBufferCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExternalMemoryBufferCreateInfo::~safe_VkExternalMemoryBufferCreateInfo()
{
}

void safe_VkExternalMemoryBufferCreateInfo::initialize(const VkExternalMemoryBufferCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExternalMemoryBufferCreateInfo::initialize(const safe_VkExternalMemoryBufferCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkExportMemoryAllocateInfo::safe_VkExportMemoryAllocateInfo(const VkExportMemoryAllocateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExportMemoryAllocateInfo::safe_VkExportMemoryAllocateInfo()
{}

safe_VkExportMemoryAllocateInfo::safe_VkExportMemoryAllocateInfo(const safe_VkExportMemoryAllocateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExportMemoryAllocateInfo& safe_VkExportMemoryAllocateInfo::operator=(const safe_VkExportMemoryAllocateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExportMemoryAllocateInfo::~safe_VkExportMemoryAllocateInfo()
{
}

void safe_VkExportMemoryAllocateInfo::initialize(const VkExportMemoryAllocateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExportMemoryAllocateInfo::initialize(const safe_VkExportMemoryAllocateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkPhysicalDeviceExternalFenceInfo::safe_VkPhysicalDeviceExternalFenceInfo(const VkPhysicalDeviceExternalFenceInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType)
{
}

safe_VkPhysicalDeviceExternalFenceInfo::safe_VkPhysicalDeviceExternalFenceInfo()
{}

safe_VkPhysicalDeviceExternalFenceInfo::safe_VkPhysicalDeviceExternalFenceInfo(const safe_VkPhysicalDeviceExternalFenceInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
}

safe_VkPhysicalDeviceExternalFenceInfo& safe_VkPhysicalDeviceExternalFenceInfo::operator=(const safe_VkPhysicalDeviceExternalFenceInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;

    return *this;
}

safe_VkPhysicalDeviceExternalFenceInfo::~safe_VkPhysicalDeviceExternalFenceInfo()
{
}

void safe_VkPhysicalDeviceExternalFenceInfo::initialize(const VkPhysicalDeviceExternalFenceInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
}

void safe_VkPhysicalDeviceExternalFenceInfo::initialize(const safe_VkPhysicalDeviceExternalFenceInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
}

safe_VkExternalFenceProperties::safe_VkExternalFenceProperties(const VkExternalFenceProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    exportFromImportedHandleTypes(in_struct->exportFromImportedHandleTypes),
    compatibleHandleTypes(in_struct->compatibleHandleTypes),
    externalFenceFeatures(in_struct->externalFenceFeatures)
{
}

safe_VkExternalFenceProperties::safe_VkExternalFenceProperties()
{}

safe_VkExternalFenceProperties::safe_VkExternalFenceProperties(const safe_VkExternalFenceProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    exportFromImportedHandleTypes = src.exportFromImportedHandleTypes;
    compatibleHandleTypes = src.compatibleHandleTypes;
    externalFenceFeatures = src.externalFenceFeatures;
}

safe_VkExternalFenceProperties& safe_VkExternalFenceProperties::operator=(const safe_VkExternalFenceProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    exportFromImportedHandleTypes = src.exportFromImportedHandleTypes;
    compatibleHandleTypes = src.compatibleHandleTypes;
    externalFenceFeatures = src.externalFenceFeatures;

    return *this;
}

safe_VkExternalFenceProperties::~safe_VkExternalFenceProperties()
{
}

void safe_VkExternalFenceProperties::initialize(const VkExternalFenceProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    exportFromImportedHandleTypes = in_struct->exportFromImportedHandleTypes;
    compatibleHandleTypes = in_struct->compatibleHandleTypes;
    externalFenceFeatures = in_struct->externalFenceFeatures;
}

void safe_VkExternalFenceProperties::initialize(const safe_VkExternalFenceProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    exportFromImportedHandleTypes = src->exportFromImportedHandleTypes;
    compatibleHandleTypes = src->compatibleHandleTypes;
    externalFenceFeatures = src->externalFenceFeatures;
}

safe_VkExportFenceCreateInfo::safe_VkExportFenceCreateInfo(const VkExportFenceCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExportFenceCreateInfo::safe_VkExportFenceCreateInfo()
{}

safe_VkExportFenceCreateInfo::safe_VkExportFenceCreateInfo(const safe_VkExportFenceCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExportFenceCreateInfo& safe_VkExportFenceCreateInfo::operator=(const safe_VkExportFenceCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExportFenceCreateInfo::~safe_VkExportFenceCreateInfo()
{
}

void safe_VkExportFenceCreateInfo::initialize(const VkExportFenceCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExportFenceCreateInfo::initialize(const safe_VkExportFenceCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkExportSemaphoreCreateInfo::safe_VkExportSemaphoreCreateInfo(const VkExportSemaphoreCreateInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExportSemaphoreCreateInfo::safe_VkExportSemaphoreCreateInfo()
{}

safe_VkExportSemaphoreCreateInfo::safe_VkExportSemaphoreCreateInfo(const safe_VkExportSemaphoreCreateInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExportSemaphoreCreateInfo& safe_VkExportSemaphoreCreateInfo::operator=(const safe_VkExportSemaphoreCreateInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExportSemaphoreCreateInfo::~safe_VkExportSemaphoreCreateInfo()
{
}

void safe_VkExportSemaphoreCreateInfo::initialize(const VkExportSemaphoreCreateInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExportSemaphoreCreateInfo::initialize(const safe_VkExportSemaphoreCreateInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkPhysicalDeviceExternalSemaphoreInfo::safe_VkPhysicalDeviceExternalSemaphoreInfo(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType)
{
}

safe_VkPhysicalDeviceExternalSemaphoreInfo::safe_VkPhysicalDeviceExternalSemaphoreInfo()
{}

safe_VkPhysicalDeviceExternalSemaphoreInfo::safe_VkPhysicalDeviceExternalSemaphoreInfo(const safe_VkPhysicalDeviceExternalSemaphoreInfo& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
}

safe_VkPhysicalDeviceExternalSemaphoreInfo& safe_VkPhysicalDeviceExternalSemaphoreInfo::operator=(const safe_VkPhysicalDeviceExternalSemaphoreInfo& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;

    return *this;
}

safe_VkPhysicalDeviceExternalSemaphoreInfo::~safe_VkPhysicalDeviceExternalSemaphoreInfo()
{
}

void safe_VkPhysicalDeviceExternalSemaphoreInfo::initialize(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
}

void safe_VkPhysicalDeviceExternalSemaphoreInfo::initialize(const safe_VkPhysicalDeviceExternalSemaphoreInfo* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
}

safe_VkExternalSemaphoreProperties::safe_VkExternalSemaphoreProperties(const VkExternalSemaphoreProperties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    exportFromImportedHandleTypes(in_struct->exportFromImportedHandleTypes),
    compatibleHandleTypes(in_struct->compatibleHandleTypes),
    externalSemaphoreFeatures(in_struct->externalSemaphoreFeatures)
{
}

safe_VkExternalSemaphoreProperties::safe_VkExternalSemaphoreProperties()
{}

safe_VkExternalSemaphoreProperties::safe_VkExternalSemaphoreProperties(const safe_VkExternalSemaphoreProperties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    exportFromImportedHandleTypes = src.exportFromImportedHandleTypes;
    compatibleHandleTypes = src.compatibleHandleTypes;
    externalSemaphoreFeatures = src.externalSemaphoreFeatures;
}

safe_VkExternalSemaphoreProperties& safe_VkExternalSemaphoreProperties::operator=(const safe_VkExternalSemaphoreProperties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    exportFromImportedHandleTypes = src.exportFromImportedHandleTypes;
    compatibleHandleTypes = src.compatibleHandleTypes;
    externalSemaphoreFeatures = src.externalSemaphoreFeatures;

    return *this;
}

safe_VkExternalSemaphoreProperties::~safe_VkExternalSemaphoreProperties()
{
}

void safe_VkExternalSemaphoreProperties::initialize(const VkExternalSemaphoreProperties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    exportFromImportedHandleTypes = in_struct->exportFromImportedHandleTypes;
    compatibleHandleTypes = in_struct->compatibleHandleTypes;
    externalSemaphoreFeatures = in_struct->externalSemaphoreFeatures;
}

void safe_VkExternalSemaphoreProperties::initialize(const safe_VkExternalSemaphoreProperties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    exportFromImportedHandleTypes = src->exportFromImportedHandleTypes;
    compatibleHandleTypes = src->compatibleHandleTypes;
    externalSemaphoreFeatures = src->externalSemaphoreFeatures;
}

safe_VkPhysicalDeviceMaintenance3Properties::safe_VkPhysicalDeviceMaintenance3Properties(const VkPhysicalDeviceMaintenance3Properties* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxPerSetDescriptors(in_struct->maxPerSetDescriptors),
    maxMemoryAllocationSize(in_struct->maxMemoryAllocationSize)
{
}

safe_VkPhysicalDeviceMaintenance3Properties::safe_VkPhysicalDeviceMaintenance3Properties()
{}

safe_VkPhysicalDeviceMaintenance3Properties::safe_VkPhysicalDeviceMaintenance3Properties(const safe_VkPhysicalDeviceMaintenance3Properties& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxPerSetDescriptors = src.maxPerSetDescriptors;
    maxMemoryAllocationSize = src.maxMemoryAllocationSize;
}

safe_VkPhysicalDeviceMaintenance3Properties& safe_VkPhysicalDeviceMaintenance3Properties::operator=(const safe_VkPhysicalDeviceMaintenance3Properties& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxPerSetDescriptors = src.maxPerSetDescriptors;
    maxMemoryAllocationSize = src.maxMemoryAllocationSize;

    return *this;
}

safe_VkPhysicalDeviceMaintenance3Properties::~safe_VkPhysicalDeviceMaintenance3Properties()
{
}

void safe_VkPhysicalDeviceMaintenance3Properties::initialize(const VkPhysicalDeviceMaintenance3Properties* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxPerSetDescriptors = in_struct->maxPerSetDescriptors;
    maxMemoryAllocationSize = in_struct->maxMemoryAllocationSize;
}

void safe_VkPhysicalDeviceMaintenance3Properties::initialize(const safe_VkPhysicalDeviceMaintenance3Properties* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxPerSetDescriptors = src->maxPerSetDescriptors;
    maxMemoryAllocationSize = src->maxMemoryAllocationSize;
}

safe_VkDescriptorSetLayoutSupport::safe_VkDescriptorSetLayoutSupport(const VkDescriptorSetLayoutSupport* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    supported(in_struct->supported)
{
}

safe_VkDescriptorSetLayoutSupport::safe_VkDescriptorSetLayoutSupport()
{}

safe_VkDescriptorSetLayoutSupport::safe_VkDescriptorSetLayoutSupport(const safe_VkDescriptorSetLayoutSupport& src)
{
    sType = src.sType;
    pNext = src.pNext;
    supported = src.supported;
}

safe_VkDescriptorSetLayoutSupport& safe_VkDescriptorSetLayoutSupport::operator=(const safe_VkDescriptorSetLayoutSupport& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    supported = src.supported;

    return *this;
}

safe_VkDescriptorSetLayoutSupport::~safe_VkDescriptorSetLayoutSupport()
{
}

void safe_VkDescriptorSetLayoutSupport::initialize(const VkDescriptorSetLayoutSupport* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    supported = in_struct->supported;
}

void safe_VkDescriptorSetLayoutSupport::initialize(const safe_VkDescriptorSetLayoutSupport* src)
{
    sType = src->sType;
    pNext = src->pNext;
    supported = src->supported;
}

safe_VkPhysicalDeviceShaderDrawParametersFeatures::safe_VkPhysicalDeviceShaderDrawParametersFeatures(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderDrawParameters(in_struct->shaderDrawParameters)
{
}

safe_VkPhysicalDeviceShaderDrawParametersFeatures::safe_VkPhysicalDeviceShaderDrawParametersFeatures()
{}

safe_VkPhysicalDeviceShaderDrawParametersFeatures::safe_VkPhysicalDeviceShaderDrawParametersFeatures(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderDrawParameters = src.shaderDrawParameters;
}

safe_VkPhysicalDeviceShaderDrawParametersFeatures& safe_VkPhysicalDeviceShaderDrawParametersFeatures::operator=(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderDrawParameters = src.shaderDrawParameters;

    return *this;
}

safe_VkPhysicalDeviceShaderDrawParametersFeatures::~safe_VkPhysicalDeviceShaderDrawParametersFeatures()
{
}

void safe_VkPhysicalDeviceShaderDrawParametersFeatures::initialize(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderDrawParameters = in_struct->shaderDrawParameters;
}

void safe_VkPhysicalDeviceShaderDrawParametersFeatures::initialize(const safe_VkPhysicalDeviceShaderDrawParametersFeatures* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderDrawParameters = src->shaderDrawParameters;
}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const VkSwapchainCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    surface(in_struct->surface),
    minImageCount(in_struct->minImageCount),
    imageFormat(in_struct->imageFormat),
    imageColorSpace(in_struct->imageColorSpace),
    imageExtent(in_struct->imageExtent),
    imageArrayLayers(in_struct->imageArrayLayers),
    imageUsage(in_struct->imageUsage),
    imageSharingMode(in_struct->imageSharingMode),
    queueFamilyIndexCount(in_struct->queueFamilyIndexCount),
    pQueueFamilyIndices(nullptr),
    preTransform(in_struct->preTransform),
    compositeAlpha(in_struct->compositeAlpha),
    presentMode(in_struct->presentMode),
    clipped(in_struct->clipped),
    oldSwapchain(in_struct->oldSwapchain)
{
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR() :
    pQueueFamilyIndices(nullptr)
{}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const safe_VkSwapchainCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    surface = src.surface;
    minImageCount = src.minImageCount;
    imageFormat = src.imageFormat;
    imageColorSpace = src.imageColorSpace;
    imageExtent = src.imageExtent;
    imageArrayLayers = src.imageArrayLayers;
    imageUsage = src.imageUsage;
    imageSharingMode = src.imageSharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    preTransform = src.preTransform;
    compositeAlpha = src.compositeAlpha;
    presentMode = src.presentMode;
    clipped = src.clipped;
    oldSwapchain = src.oldSwapchain;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }
}

safe_VkSwapchainCreateInfoKHR& safe_VkSwapchainCreateInfoKHR::operator=(const safe_VkSwapchainCreateInfoKHR& src)
{
    if (&src == this) return *this;

    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    surface = src.surface;
    minImageCount = src.minImageCount;
    imageFormat = src.imageFormat;
    imageColorSpace = src.imageColorSpace;
    imageExtent = src.imageExtent;
    imageArrayLayers = src.imageArrayLayers;
    imageUsage = src.imageUsage;
    imageSharingMode = src.imageSharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    preTransform = src.preTransform;
    compositeAlpha = src.compositeAlpha;
    presentMode = src.presentMode;
    clipped = src.clipped;
    oldSwapchain = src.oldSwapchain;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }

    return *this;
}

safe_VkSwapchainCreateInfoKHR::~safe_VkSwapchainCreateInfoKHR()
{
    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;
}

void safe_VkSwapchainCreateInfoKHR::initialize(const VkSwapchainCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    surface = in_struct->surface;
    minImageCount = in_struct->minImageCount;
    imageFormat = in_struct->imageFormat;
    imageColorSpace = in_struct->imageColorSpace;
    imageExtent = in_struct->imageExtent;
    imageArrayLayers = in_struct->imageArrayLayers;
    imageUsage = in_struct->imageUsage;
    imageSharingMode = in_struct->imageSharingMode;
    queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    preTransform = in_struct->preTransform;
    compositeAlpha = in_struct->compositeAlpha;
    presentMode = in_struct->presentMode;
    clipped = in_struct->clipped;
    oldSwapchain = in_struct->oldSwapchain;
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

void safe_VkSwapchainCreateInfoKHR::initialize(const safe_VkSwapchainCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    surface = src->surface;
    minImageCount = src->minImageCount;
    imageFormat = src->imageFormat;
    imageColorSpace = src->imageColorSpace;
    imageExtent = src->imageExtent;
    imageArrayLayers = src->imageArrayLayers;
    imageUsage = src->imageUsage;
    imageSharingMode = src->imageSharingMode;
    queueFamilyIndexCount = src->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    preTransform = src->preTransform;
    compositeAlpha = src->compositeAlpha;
    presentMode = src->presentMode;
    clipped = src->clipped;
    oldSwapchain = src->oldSwapchain;
    if (src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src->pQueueFamilyIndices, sizeof(uint32_t)*src->queueFamilyIndexCount);
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const VkPresentInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    waitSemaphoreCount(in_struct->waitSemaphoreCount),
    pWaitSemaphores(nullptr),
    swapchainCount(in_struct->swapchainCount),
    pSwapchains(nullptr),
    pImageIndices(nullptr),
    pResults(nullptr)
{
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }
    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy ((void *)pImageIndices, (void *)in_struct->pImageIndices, sizeof(uint32_t)*in_struct->swapchainCount);
    }
    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy ((void *)pResults, (void *)in_struct->pResults, sizeof(VkResult)*in_struct->swapchainCount);
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR() :
    pWaitSemaphores(nullptr),
    pSwapchains(nullptr),
    pImageIndices(nullptr),
    pResults(nullptr)
{}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const safe_VkPresentInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pSwapchains[i] = src.pSwapchains[i];
        }
    }
    if (src.pImageIndices) {
        pImageIndices = new uint32_t[src.swapchainCount];
        memcpy ((void *)pImageIndices, (void *)src.pImageIndices, sizeof(uint32_t)*src.swapchainCount);
    }
    if (src.pResults) {
        pResults = new VkResult[src.swapchainCount];
        memcpy ((void *)pResults, (void *)src.pResults, sizeof(VkResult)*src.swapchainCount);
    }
}

safe_VkPresentInfoKHR& safe_VkPresentInfoKHR::operator=(const safe_VkPresentInfoKHR& src)
{
    if (&src == this) return *this;

    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pSwapchains)
        delete[] pSwapchains;
    if (pImageIndices)
        delete[] pImageIndices;
    if (pResults)
        delete[] pResults;

    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreCount = src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    if (waitSemaphoreCount && src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pSwapchains[i] = src.pSwapchains[i];
        }
    }
    if (src.pImageIndices) {
        pImageIndices = new uint32_t[src.swapchainCount];
        memcpy ((void *)pImageIndices, (void *)src.pImageIndices, sizeof(uint32_t)*src.swapchainCount);
    }
    if (src.pResults) {
        pResults = new VkResult[src.swapchainCount];
        memcpy ((void *)pResults, (void *)src.pResults, sizeof(VkResult)*src.swapchainCount);
    }

    return *this;
}

safe_VkPresentInfoKHR::~safe_VkPresentInfoKHR()
{
    if (pWaitSemaphores)
        delete[] pWaitSemaphores;
    if (pSwapchains)
        delete[] pSwapchains;
    if (pImageIndices)
        delete[] pImageIndices;
    if (pResults)
        delete[] pResults;
}

void safe_VkPresentInfoKHR::initialize(const VkPresentInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = in_struct->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }
    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy ((void *)pImageIndices, (void *)in_struct->pImageIndices, sizeof(uint32_t)*in_struct->swapchainCount);
    }
    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy ((void *)pResults, (void *)in_struct->pResults, sizeof(VkResult)*in_struct->swapchainCount);
    }
}

void safe_VkPresentInfoKHR::initialize(const safe_VkPresentInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    waitSemaphoreCount = src->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = src->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    if (waitSemaphoreCount && src->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i=0; i<waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = src->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && src->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pSwapchains[i] = src->pSwapchains[i];
        }
    }
    if (src->pImageIndices) {
        pImageIndices = new uint32_t[src->swapchainCount];
        memcpy ((void *)pImageIndices, (void *)src->pImageIndices, sizeof(uint32_t)*src->swapchainCount);
    }
    if (src->pResults) {
        pResults = new VkResult[src->swapchainCount];
        memcpy ((void *)pResults, (void *)src->pResults, sizeof(VkResult)*src->swapchainCount);
    }
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const VkImageSwapchainCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchain(in_struct->swapchain)
{
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR()
{}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const safe_VkImageSwapchainCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;
}

safe_VkImageSwapchainCreateInfoKHR& safe_VkImageSwapchainCreateInfoKHR::operator=(const safe_VkImageSwapchainCreateInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;

    return *this;
}

safe_VkImageSwapchainCreateInfoKHR::~safe_VkImageSwapchainCreateInfoKHR()
{
}

void safe_VkImageSwapchainCreateInfoKHR::initialize(const VkImageSwapchainCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchain = in_struct->swapchain;
}

void safe_VkImageSwapchainCreateInfoKHR::initialize(const safe_VkImageSwapchainCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchain = src->swapchain;
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(const VkBindImageMemorySwapchainInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchain(in_struct->swapchain),
    imageIndex(in_struct->imageIndex)
{
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR()
{}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(const safe_VkBindImageMemorySwapchainInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;
    imageIndex = src.imageIndex;
}

safe_VkBindImageMemorySwapchainInfoKHR& safe_VkBindImageMemorySwapchainInfoKHR::operator=(const safe_VkBindImageMemorySwapchainInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;
    imageIndex = src.imageIndex;

    return *this;
}

safe_VkBindImageMemorySwapchainInfoKHR::~safe_VkBindImageMemorySwapchainInfoKHR()
{
}

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const VkBindImageMemorySwapchainInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchain = in_struct->swapchain;
    imageIndex = in_struct->imageIndex;
}

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const safe_VkBindImageMemorySwapchainInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchain = src->swapchain;
    imageIndex = src->imageIndex;
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const VkAcquireNextImageInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchain(in_struct->swapchain),
    timeout(in_struct->timeout),
    semaphore(in_struct->semaphore),
    fence(in_struct->fence),
    deviceMask(in_struct->deviceMask)
{
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR()
{}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const safe_VkAcquireNextImageInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;
    timeout = src.timeout;
    semaphore = src.semaphore;
    fence = src.fence;
    deviceMask = src.deviceMask;
}

safe_VkAcquireNextImageInfoKHR& safe_VkAcquireNextImageInfoKHR::operator=(const safe_VkAcquireNextImageInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    swapchain = src.swapchain;
    timeout = src.timeout;
    semaphore = src.semaphore;
    fence = src.fence;
    deviceMask = src.deviceMask;

    return *this;
}

safe_VkAcquireNextImageInfoKHR::~safe_VkAcquireNextImageInfoKHR()
{
}

void safe_VkAcquireNextImageInfoKHR::initialize(const VkAcquireNextImageInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchain = in_struct->swapchain;
    timeout = in_struct->timeout;
    semaphore = in_struct->semaphore;
    fence = in_struct->fence;
    deviceMask = in_struct->deviceMask;
}

void safe_VkAcquireNextImageInfoKHR::initialize(const safe_VkAcquireNextImageInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchain = src->swapchain;
    timeout = src->timeout;
    semaphore = src->semaphore;
    fence = src->fence;
    deviceMask = src->deviceMask;
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(const VkDeviceGroupPresentCapabilitiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    modes(in_struct->modes)
{
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR()
{}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(const safe_VkDeviceGroupPresentCapabilitiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    modes = src.modes;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = src.presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR& safe_VkDeviceGroupPresentCapabilitiesKHR::operator=(const safe_VkDeviceGroupPresentCapabilitiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    modes = src.modes;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = src.presentMask[i];
    }

    return *this;
}

safe_VkDeviceGroupPresentCapabilitiesKHR::~safe_VkDeviceGroupPresentCapabilitiesKHR()
{
}

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const VkDeviceGroupPresentCapabilitiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    modes = in_struct->modes;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const safe_VkDeviceGroupPresentCapabilitiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    modes = src->modes;
    for (uint32_t i=0; i<VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = src->presentMask[i];
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const VkDeviceGroupPresentInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchainCount(in_struct->swapchainCount),
    pDeviceMasks(nullptr),
    mode(in_struct->mode)
{
    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy ((void *)pDeviceMasks, (void *)in_struct->pDeviceMasks, sizeof(uint32_t)*in_struct->swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR() :
    pDeviceMasks(nullptr)
{}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const safe_VkDeviceGroupPresentInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = src.mode;
    if (src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[src.swapchainCount];
        memcpy ((void *)pDeviceMasks, (void *)src.pDeviceMasks, sizeof(uint32_t)*src.swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR& safe_VkDeviceGroupPresentInfoKHR::operator=(const safe_VkDeviceGroupPresentInfoKHR& src)
{
    if (&src == this) return *this;

    if (pDeviceMasks)
        delete[] pDeviceMasks;

    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = src.mode;
    if (src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[src.swapchainCount];
        memcpy ((void *)pDeviceMasks, (void *)src.pDeviceMasks, sizeof(uint32_t)*src.swapchainCount);
    }

    return *this;
}

safe_VkDeviceGroupPresentInfoKHR::~safe_VkDeviceGroupPresentInfoKHR()
{
    if (pDeviceMasks)
        delete[] pDeviceMasks;
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const VkDeviceGroupPresentInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchainCount = in_struct->swapchainCount;
    pDeviceMasks = nullptr;
    mode = in_struct->mode;
    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy ((void *)pDeviceMasks, (void *)in_struct->pDeviceMasks, sizeof(uint32_t)*in_struct->swapchainCount);
    }
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const safe_VkDeviceGroupPresentInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchainCount = src->swapchainCount;
    pDeviceMasks = nullptr;
    mode = src->mode;
    if (src->pDeviceMasks) {
        pDeviceMasks = new uint32_t[src->swapchainCount];
        memcpy ((void *)pDeviceMasks, (void *)src->pDeviceMasks, sizeof(uint32_t)*src->swapchainCount);
    }
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    modes(in_struct->modes)
{
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR()
{}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(const safe_VkDeviceGroupSwapchainCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    modes = src.modes;
}

safe_VkDeviceGroupSwapchainCreateInfoKHR& safe_VkDeviceGroupSwapchainCreateInfoKHR::operator=(const safe_VkDeviceGroupSwapchainCreateInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    modes = src.modes;

    return *this;
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::~safe_VkDeviceGroupSwapchainCreateInfoKHR()
{
}

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    modes = in_struct->modes;
}

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const safe_VkDeviceGroupSwapchainCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    modes = src->modes;
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const VkDisplayPropertiesKHR* in_struct) :
    display(in_struct->display),
    displayName(in_struct->displayName),
    physicalDimensions(in_struct->physicalDimensions),
    physicalResolution(in_struct->physicalResolution),
    supportedTransforms(in_struct->supportedTransforms),
    planeReorderPossible(in_struct->planeReorderPossible),
    persistentContent(in_struct->persistentContent)
{
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR()
{}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const safe_VkDisplayPropertiesKHR& src)
{
    display = src.display;
    displayName = src.displayName;
    physicalDimensions = src.physicalDimensions;
    physicalResolution = src.physicalResolution;
    supportedTransforms = src.supportedTransforms;
    planeReorderPossible = src.planeReorderPossible;
    persistentContent = src.persistentContent;
}

safe_VkDisplayPropertiesKHR& safe_VkDisplayPropertiesKHR::operator=(const safe_VkDisplayPropertiesKHR& src)
{
    if (&src == this) return *this;


    display = src.display;
    displayName = src.displayName;
    physicalDimensions = src.physicalDimensions;
    physicalResolution = src.physicalResolution;
    supportedTransforms = src.supportedTransforms;
    planeReorderPossible = src.planeReorderPossible;
    persistentContent = src.persistentContent;

    return *this;
}

safe_VkDisplayPropertiesKHR::~safe_VkDisplayPropertiesKHR()
{
}

void safe_VkDisplayPropertiesKHR::initialize(const VkDisplayPropertiesKHR* in_struct)
{
    display = in_struct->display;
    displayName = in_struct->displayName;
    physicalDimensions = in_struct->physicalDimensions;
    physicalResolution = in_struct->physicalResolution;
    supportedTransforms = in_struct->supportedTransforms;
    planeReorderPossible = in_struct->planeReorderPossible;
    persistentContent = in_struct->persistentContent;
}

void safe_VkDisplayPropertiesKHR::initialize(const safe_VkDisplayPropertiesKHR* src)
{
    display = src->display;
    displayName = src->displayName;
    physicalDimensions = src->physicalDimensions;
    physicalResolution = src->physicalResolution;
    supportedTransforms = src->supportedTransforms;
    planeReorderPossible = src->planeReorderPossible;
    persistentContent = src->persistentContent;
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const VkDisplayModeCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    parameters(in_struct->parameters)
{
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR()
{}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const safe_VkDisplayModeCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    parameters = src.parameters;
}

safe_VkDisplayModeCreateInfoKHR& safe_VkDisplayModeCreateInfoKHR::operator=(const safe_VkDisplayModeCreateInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    parameters = src.parameters;

    return *this;
}

safe_VkDisplayModeCreateInfoKHR::~safe_VkDisplayModeCreateInfoKHR()
{
}

void safe_VkDisplayModeCreateInfoKHR::initialize(const VkDisplayModeCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    parameters = in_struct->parameters;
}

void safe_VkDisplayModeCreateInfoKHR::initialize(const safe_VkDisplayModeCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    parameters = src->parameters;
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const VkDisplaySurfaceCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    displayMode(in_struct->displayMode),
    planeIndex(in_struct->planeIndex),
    planeStackIndex(in_struct->planeStackIndex),
    transform(in_struct->transform),
    globalAlpha(in_struct->globalAlpha),
    alphaMode(in_struct->alphaMode),
    imageExtent(in_struct->imageExtent)
{
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR()
{}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const safe_VkDisplaySurfaceCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    displayMode = src.displayMode;
    planeIndex = src.planeIndex;
    planeStackIndex = src.planeStackIndex;
    transform = src.transform;
    globalAlpha = src.globalAlpha;
    alphaMode = src.alphaMode;
    imageExtent = src.imageExtent;
}

safe_VkDisplaySurfaceCreateInfoKHR& safe_VkDisplaySurfaceCreateInfoKHR::operator=(const safe_VkDisplaySurfaceCreateInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    displayMode = src.displayMode;
    planeIndex = src.planeIndex;
    planeStackIndex = src.planeStackIndex;
    transform = src.transform;
    globalAlpha = src.globalAlpha;
    alphaMode = src.alphaMode;
    imageExtent = src.imageExtent;

    return *this;
}

safe_VkDisplaySurfaceCreateInfoKHR::~safe_VkDisplaySurfaceCreateInfoKHR()
{
}

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const VkDisplaySurfaceCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    displayMode = in_struct->displayMode;
    planeIndex = in_struct->planeIndex;
    planeStackIndex = in_struct->planeStackIndex;
    transform = in_struct->transform;
    globalAlpha = in_struct->globalAlpha;
    alphaMode = in_struct->alphaMode;
    imageExtent = in_struct->imageExtent;
}

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const safe_VkDisplaySurfaceCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    displayMode = src->displayMode;
    planeIndex = src->planeIndex;
    planeStackIndex = src->planeStackIndex;
    transform = src->transform;
    globalAlpha = src->globalAlpha;
    alphaMode = src->alphaMode;
    imageExtent = src->imageExtent;
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const VkDisplayPresentInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcRect(in_struct->srcRect),
    dstRect(in_struct->dstRect),
    persistent(in_struct->persistent)
{
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR()
{}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const safe_VkDisplayPresentInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcRect = src.srcRect;
    dstRect = src.dstRect;
    persistent = src.persistent;
}

safe_VkDisplayPresentInfoKHR& safe_VkDisplayPresentInfoKHR::operator=(const safe_VkDisplayPresentInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcRect = src.srcRect;
    dstRect = src.dstRect;
    persistent = src.persistent;

    return *this;
}

safe_VkDisplayPresentInfoKHR::~safe_VkDisplayPresentInfoKHR()
{
}

void safe_VkDisplayPresentInfoKHR::initialize(const VkDisplayPresentInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcRect = in_struct->srcRect;
    dstRect = in_struct->dstRect;
    persistent = in_struct->persistent;
}

void safe_VkDisplayPresentInfoKHR::initialize(const safe_VkDisplayPresentInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcRect = src->srcRect;
    dstRect = src->dstRect;
    persistent = src->persistent;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(const VkImportMemoryWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType),
    handle(in_struct->handle),
    name(in_struct->name)
{
}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR()
{}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(const safe_VkImportMemoryWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;
}

safe_VkImportMemoryWin32HandleInfoKHR& safe_VkImportMemoryWin32HandleInfoKHR::operator=(const safe_VkImportMemoryWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;

    return *this;
}

safe_VkImportMemoryWin32HandleInfoKHR::~safe_VkImportMemoryWin32HandleInfoKHR()
{
}

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const VkImportMemoryWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
}

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const safe_VkImportMemoryWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
    handle = src->handle;
    name = src->name;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(const VkExportMemoryWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pAttributes(nullptr),
    dwAccess(in_struct->dwAccess),
    name(in_struct->name)
{
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR() :
    pAttributes(nullptr)
{}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(const safe_VkExportMemoryWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR& safe_VkExportMemoryWin32HandleInfoKHR::operator=(const safe_VkExportMemoryWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;

    if (pAttributes)
        delete pAttributes;

    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }

    return *this;
}

safe_VkExportMemoryWin32HandleInfoKHR::~safe_VkExportMemoryWin32HandleInfoKHR()
{
    if (pAttributes)
        delete pAttributes;
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const VkExportMemoryWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const safe_VkExportMemoryWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pAttributes = nullptr;
    dwAccess = src->dwAccess;
    name = src->name;
    if (src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src->pAttributes);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(const VkMemoryWin32HandlePropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryTypeBits(in_struct->memoryTypeBits)
{
}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR()
{}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(const safe_VkMemoryWin32HandlePropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;
}

safe_VkMemoryWin32HandlePropertiesKHR& safe_VkMemoryWin32HandlePropertiesKHR::operator=(const safe_VkMemoryWin32HandlePropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;

    return *this;
}

safe_VkMemoryWin32HandlePropertiesKHR::~safe_VkMemoryWin32HandlePropertiesKHR()
{
}

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const VkMemoryWin32HandlePropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryTypeBits = in_struct->memoryTypeBits;
}

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const safe_VkMemoryWin32HandlePropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryTypeBits = src->memoryTypeBits;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const VkMemoryGetWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memory(in_struct->memory),
    handleType(in_struct->handleType)
{
}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR()
{}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const safe_VkMemoryGetWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    handleType = src.handleType;
}

safe_VkMemoryGetWin32HandleInfoKHR& safe_VkMemoryGetWin32HandleInfoKHR::operator=(const safe_VkMemoryGetWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    handleType = src.handleType;

    return *this;
}

safe_VkMemoryGetWin32HandleInfoKHR::~safe_VkMemoryGetWin32HandleInfoKHR()
{
}

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const VkMemoryGetWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
}

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const safe_VkMemoryGetWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memory = src->memory;
    handleType = src->handleType;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const VkImportMemoryFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType),
    fd(in_struct->fd)
{
}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR()
{}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const safe_VkImportMemoryFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    fd = src.fd;
}

safe_VkImportMemoryFdInfoKHR& safe_VkImportMemoryFdInfoKHR::operator=(const safe_VkImportMemoryFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    fd = src.fd;

    return *this;
}

safe_VkImportMemoryFdInfoKHR::~safe_VkImportMemoryFdInfoKHR()
{
}

void safe_VkImportMemoryFdInfoKHR::initialize(const VkImportMemoryFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
}

void safe_VkImportMemoryFdInfoKHR::initialize(const safe_VkImportMemoryFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
    fd = src->fd;
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const VkMemoryFdPropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryTypeBits(in_struct->memoryTypeBits)
{
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR()
{}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const safe_VkMemoryFdPropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;
}

safe_VkMemoryFdPropertiesKHR& safe_VkMemoryFdPropertiesKHR::operator=(const safe_VkMemoryFdPropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;

    return *this;
}

safe_VkMemoryFdPropertiesKHR::~safe_VkMemoryFdPropertiesKHR()
{
}

void safe_VkMemoryFdPropertiesKHR::initialize(const VkMemoryFdPropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryTypeBits = in_struct->memoryTypeBits;
}

void safe_VkMemoryFdPropertiesKHR::initialize(const safe_VkMemoryFdPropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryTypeBits = src->memoryTypeBits;
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const VkMemoryGetFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memory(in_struct->memory),
    handleType(in_struct->handleType)
{
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR()
{}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const safe_VkMemoryGetFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    handleType = src.handleType;
}

safe_VkMemoryGetFdInfoKHR& safe_VkMemoryGetFdInfoKHR::operator=(const safe_VkMemoryGetFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
    handleType = src.handleType;

    return *this;
}

safe_VkMemoryGetFdInfoKHR::~safe_VkMemoryGetFdInfoKHR()
{
}

void safe_VkMemoryGetFdInfoKHR::initialize(const VkMemoryGetFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
}

void safe_VkMemoryGetFdInfoKHR::initialize(const safe_VkMemoryGetFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memory = src->memory;
    handleType = src->handleType;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    acquireCount(in_struct->acquireCount),
    pAcquireSyncs(nullptr),
    pAcquireKeys(nullptr),
    pAcquireTimeouts(nullptr),
    releaseCount(in_struct->releaseCount),
    pReleaseSyncs(nullptr),
    pReleaseKeys(nullptr)
{
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }
    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)in_struct->pAcquireKeys, sizeof(uint64_t)*in_struct->acquireCount);
    }
    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireTimeouts, (void *)in_struct->pAcquireTimeouts, sizeof(uint32_t)*in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }
    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)in_struct->pReleaseKeys, sizeof(uint64_t)*in_struct->releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR() :
    pAcquireSyncs(nullptr),
    pAcquireKeys(nullptr),
    pAcquireTimeouts(nullptr),
    pReleaseSyncs(nullptr),
    pReleaseKeys(nullptr)
{}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    acquireCount = src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src.pAcquireSyncs[i];
        }
    }
    if (src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[src.acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src.pAcquireKeys, sizeof(uint64_t)*src.acquireCount);
    }
    if (src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[src.acquireCount];
        memcpy ((void *)pAcquireTimeouts, (void *)src.pAcquireTimeouts, sizeof(uint32_t)*src.acquireCount);
    }
    if (releaseCount && src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src.pReleaseSyncs[i];
        }
    }
    if (src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[src.releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src.pReleaseKeys, sizeof(uint64_t)*src.releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::operator=(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& src)
{
    if (&src == this) return *this;

    if (pAcquireSyncs)
        delete[] pAcquireSyncs;
    if (pAcquireKeys)
        delete[] pAcquireKeys;
    if (pAcquireTimeouts)
        delete[] pAcquireTimeouts;
    if (pReleaseSyncs)
        delete[] pReleaseSyncs;
    if (pReleaseKeys)
        delete[] pReleaseKeys;

    sType = src.sType;
    pNext = src.pNext;
    acquireCount = src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src.pAcquireSyncs[i];
        }
    }
    if (src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[src.acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src.pAcquireKeys, sizeof(uint64_t)*src.acquireCount);
    }
    if (src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[src.acquireCount];
        memcpy ((void *)pAcquireTimeouts, (void *)src.pAcquireTimeouts, sizeof(uint32_t)*src.acquireCount);
    }
    if (releaseCount && src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src.pReleaseSyncs[i];
        }
    }
    if (src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[src.releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src.pReleaseKeys, sizeof(uint64_t)*src.releaseCount);
    }

    return *this;
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::~safe_VkWin32KeyedMutexAcquireReleaseInfoKHR()
{
    if (pAcquireSyncs)
        delete[] pAcquireSyncs;
    if (pAcquireKeys)
        delete[] pAcquireKeys;
    if (pAcquireTimeouts)
        delete[] pAcquireTimeouts;
    if (pReleaseSyncs)
        delete[] pReleaseSyncs;
    if (pReleaseKeys)
        delete[] pReleaseKeys;
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    acquireCount = in_struct->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = in_struct->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }
    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)in_struct->pAcquireKeys, sizeof(uint64_t)*in_struct->acquireCount);
    }
    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireTimeouts, (void *)in_struct->pAcquireTimeouts, sizeof(uint32_t)*in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }
    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)in_struct->pReleaseKeys, sizeof(uint64_t)*in_struct->releaseCount);
    }
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    acquireCount = src->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = src->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src->pAcquireSyncs[i];
        }
    }
    if (src->pAcquireKeys) {
        pAcquireKeys = new uint64_t[src->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src->pAcquireKeys, sizeof(uint64_t)*src->acquireCount);
    }
    if (src->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[src->acquireCount];
        memcpy ((void *)pAcquireTimeouts, (void *)src->pAcquireTimeouts, sizeof(uint32_t)*src->acquireCount);
    }
    if (releaseCount && src->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src->pReleaseSyncs[i];
        }
    }
    if (src->pReleaseKeys) {
        pReleaseKeys = new uint64_t[src->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src->pReleaseKeys, sizeof(uint64_t)*src->releaseCount);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(const VkImportSemaphoreWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    semaphore(in_struct->semaphore),
    flags(in_struct->flags),
    handleType(in_struct->handleType),
    handle(in_struct->handle),
    name(in_struct->name)
{
}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR()
{}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(const safe_VkImportSemaphoreWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    flags = src.flags;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;
}

safe_VkImportSemaphoreWin32HandleInfoKHR& safe_VkImportSemaphoreWin32HandleInfoKHR::operator=(const safe_VkImportSemaphoreWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    flags = src.flags;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;

    return *this;
}

safe_VkImportSemaphoreWin32HandleInfoKHR::~safe_VkImportSemaphoreWin32HandleInfoKHR()
{
}

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const VkImportSemaphoreWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
}

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkImportSemaphoreWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    semaphore = src->semaphore;
    flags = src->flags;
    handleType = src->handleType;
    handle = src->handle;
    name = src->name;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(const VkExportSemaphoreWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pAttributes(nullptr),
    dwAccess(in_struct->dwAccess),
    name(in_struct->name)
{
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR() :
    pAttributes(nullptr)
{}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(const safe_VkExportSemaphoreWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR& safe_VkExportSemaphoreWin32HandleInfoKHR::operator=(const safe_VkExportSemaphoreWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;

    if (pAttributes)
        delete pAttributes;

    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }

    return *this;
}

safe_VkExportSemaphoreWin32HandleInfoKHR::~safe_VkExportSemaphoreWin32HandleInfoKHR()
{
    if (pAttributes)
        delete pAttributes;
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const VkExportSemaphoreWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkExportSemaphoreWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pAttributes = nullptr;
    dwAccess = src->dwAccess;
    name = src->name;
    if (src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src->pAttributes);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const VkD3D12FenceSubmitInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    waitSemaphoreValuesCount(in_struct->waitSemaphoreValuesCount),
    pWaitSemaphoreValues(nullptr),
    signalSemaphoreValuesCount(in_struct->signalSemaphoreValuesCount),
    pSignalSemaphoreValues(nullptr)
{
    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy ((void *)pWaitSemaphoreValues, (void *)in_struct->pWaitSemaphoreValues, sizeof(uint64_t)*in_struct->waitSemaphoreValuesCount);
    }
    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy ((void *)pSignalSemaphoreValues, (void *)in_struct->pSignalSemaphoreValues, sizeof(uint64_t)*in_struct->signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR() :
    pWaitSemaphoreValues(nullptr),
    pSignalSemaphoreValues(nullptr)
{}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const safe_VkD3D12FenceSubmitInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreValuesCount = src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    if (src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[src.waitSemaphoreValuesCount];
        memcpy ((void *)pWaitSemaphoreValues, (void *)src.pWaitSemaphoreValues, sizeof(uint64_t)*src.waitSemaphoreValuesCount);
    }
    if (src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[src.signalSemaphoreValuesCount];
        memcpy ((void *)pSignalSemaphoreValues, (void *)src.pSignalSemaphoreValues, sizeof(uint64_t)*src.signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR& safe_VkD3D12FenceSubmitInfoKHR::operator=(const safe_VkD3D12FenceSubmitInfoKHR& src)
{
    if (&src == this) return *this;

    if (pWaitSemaphoreValues)
        delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues)
        delete[] pSignalSemaphoreValues;

    sType = src.sType;
    pNext = src.pNext;
    waitSemaphoreValuesCount = src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    if (src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[src.waitSemaphoreValuesCount];
        memcpy ((void *)pWaitSemaphoreValues, (void *)src.pWaitSemaphoreValues, sizeof(uint64_t)*src.waitSemaphoreValuesCount);
    }
    if (src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[src.signalSemaphoreValuesCount];
        memcpy ((void *)pSignalSemaphoreValues, (void *)src.pSignalSemaphoreValues, sizeof(uint64_t)*src.signalSemaphoreValuesCount);
    }

    return *this;
}

safe_VkD3D12FenceSubmitInfoKHR::~safe_VkD3D12FenceSubmitInfoKHR()
{
    if (pWaitSemaphoreValues)
        delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues)
        delete[] pSignalSemaphoreValues;
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const VkD3D12FenceSubmitInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    waitSemaphoreValuesCount = in_struct->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = in_struct->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy ((void *)pWaitSemaphoreValues, (void *)in_struct->pWaitSemaphoreValues, sizeof(uint64_t)*in_struct->waitSemaphoreValuesCount);
    }
    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy ((void *)pSignalSemaphoreValues, (void *)in_struct->pSignalSemaphoreValues, sizeof(uint64_t)*in_struct->signalSemaphoreValuesCount);
    }
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const safe_VkD3D12FenceSubmitInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    waitSemaphoreValuesCount = src->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = src->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    if (src->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[src->waitSemaphoreValuesCount];
        memcpy ((void *)pWaitSemaphoreValues, (void *)src->pWaitSemaphoreValues, sizeof(uint64_t)*src->waitSemaphoreValuesCount);
    }
    if (src->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[src->signalSemaphoreValuesCount];
        memcpy ((void *)pSignalSemaphoreValues, (void *)src->pSignalSemaphoreValues, sizeof(uint64_t)*src->signalSemaphoreValuesCount);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(const VkSemaphoreGetWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    semaphore(in_struct->semaphore),
    handleType(in_struct->handleType)
{
}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR()
{}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(const safe_VkSemaphoreGetWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    handleType = src.handleType;
}

safe_VkSemaphoreGetWin32HandleInfoKHR& safe_VkSemaphoreGetWin32HandleInfoKHR::operator=(const safe_VkSemaphoreGetWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    handleType = src.handleType;

    return *this;
}

safe_VkSemaphoreGetWin32HandleInfoKHR::~safe_VkSemaphoreGetWin32HandleInfoKHR()
{
}

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const VkSemaphoreGetWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
}

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const safe_VkSemaphoreGetWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    semaphore = src->semaphore;
    handleType = src->handleType;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const VkImportSemaphoreFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    semaphore(in_struct->semaphore),
    flags(in_struct->flags),
    handleType(in_struct->handleType),
    fd(in_struct->fd)
{
}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR()
{}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const safe_VkImportSemaphoreFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    flags = src.flags;
    handleType = src.handleType;
    fd = src.fd;
}

safe_VkImportSemaphoreFdInfoKHR& safe_VkImportSemaphoreFdInfoKHR::operator=(const safe_VkImportSemaphoreFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    flags = src.flags;
    handleType = src.handleType;
    fd = src.fd;

    return *this;
}

safe_VkImportSemaphoreFdInfoKHR::~safe_VkImportSemaphoreFdInfoKHR()
{
}

void safe_VkImportSemaphoreFdInfoKHR::initialize(const VkImportSemaphoreFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
}

void safe_VkImportSemaphoreFdInfoKHR::initialize(const safe_VkImportSemaphoreFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    semaphore = src->semaphore;
    flags = src->flags;
    handleType = src->handleType;
    fd = src->fd;
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const VkSemaphoreGetFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    semaphore(in_struct->semaphore),
    handleType(in_struct->handleType)
{
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR()
{}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const safe_VkSemaphoreGetFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    handleType = src.handleType;
}

safe_VkSemaphoreGetFdInfoKHR& safe_VkSemaphoreGetFdInfoKHR::operator=(const safe_VkSemaphoreGetFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    semaphore = src.semaphore;
    handleType = src.handleType;

    return *this;
}

safe_VkSemaphoreGetFdInfoKHR::~safe_VkSemaphoreGetFdInfoKHR()
{
}

void safe_VkSemaphoreGetFdInfoKHR::initialize(const VkSemaphoreGetFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
}

void safe_VkSemaphoreGetFdInfoKHR::initialize(const safe_VkSemaphoreGetFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    semaphore = src->semaphore;
    handleType = src->handleType;
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR(const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxPushDescriptors(in_struct->maxPushDescriptors)
{
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR()
{}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::safe_VkPhysicalDevicePushDescriptorPropertiesKHR(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxPushDescriptors = src.maxPushDescriptors;
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR& safe_VkPhysicalDevicePushDescriptorPropertiesKHR::operator=(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxPushDescriptors = src.maxPushDescriptors;

    return *this;
}

safe_VkPhysicalDevicePushDescriptorPropertiesKHR::~safe_VkPhysicalDevicePushDescriptorPropertiesKHR()
{
}

void safe_VkPhysicalDevicePushDescriptorPropertiesKHR::initialize(const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxPushDescriptors = in_struct->maxPushDescriptors;
}

void safe_VkPhysicalDevicePushDescriptorPropertiesKHR::initialize(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxPushDescriptors = src->maxPushDescriptors;
}

safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::safe_VkPhysicalDeviceFloat16Int8FeaturesKHR(const VkPhysicalDeviceFloat16Int8FeaturesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderFloat16(in_struct->shaderFloat16),
    shaderInt8(in_struct->shaderInt8)
{
}

safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::safe_VkPhysicalDeviceFloat16Int8FeaturesKHR()
{}

safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::safe_VkPhysicalDeviceFloat16Int8FeaturesKHR(const safe_VkPhysicalDeviceFloat16Int8FeaturesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderFloat16 = src.shaderFloat16;
    shaderInt8 = src.shaderInt8;
}

safe_VkPhysicalDeviceFloat16Int8FeaturesKHR& safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::operator=(const safe_VkPhysicalDeviceFloat16Int8FeaturesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderFloat16 = src.shaderFloat16;
    shaderInt8 = src.shaderInt8;

    return *this;
}

safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::~safe_VkPhysicalDeviceFloat16Int8FeaturesKHR()
{
}

void safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::initialize(const VkPhysicalDeviceFloat16Int8FeaturesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderFloat16 = in_struct->shaderFloat16;
    shaderInt8 = in_struct->shaderInt8;
}

void safe_VkPhysicalDeviceFloat16Int8FeaturesKHR::initialize(const safe_VkPhysicalDeviceFloat16Int8FeaturesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderFloat16 = src->shaderFloat16;
    shaderInt8 = src->shaderInt8;
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const VkPresentRegionKHR* in_struct) :
    rectangleCount(in_struct->rectangleCount),
    pRectangles(nullptr)
{
    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy ((void *)pRectangles, (void *)in_struct->pRectangles, sizeof(VkRectLayerKHR)*in_struct->rectangleCount);
    }
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR() :
    pRectangles(nullptr)
{}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const safe_VkPresentRegionKHR& src)
{
    rectangleCount = src.rectangleCount;
    pRectangles = nullptr;
    if (src.pRectangles) {
        pRectangles = new VkRectLayerKHR[src.rectangleCount];
        memcpy ((void *)pRectangles, (void *)src.pRectangles, sizeof(VkRectLayerKHR)*src.rectangleCount);
    }
}

safe_VkPresentRegionKHR& safe_VkPresentRegionKHR::operator=(const safe_VkPresentRegionKHR& src)
{
    if (&src == this) return *this;

    if (pRectangles)
        delete[] pRectangles;

    rectangleCount = src.rectangleCount;
    pRectangles = nullptr;
    if (src.pRectangles) {
        pRectangles = new VkRectLayerKHR[src.rectangleCount];
        memcpy ((void *)pRectangles, (void *)src.pRectangles, sizeof(VkRectLayerKHR)*src.rectangleCount);
    }

    return *this;
}

safe_VkPresentRegionKHR::~safe_VkPresentRegionKHR()
{
    if (pRectangles)
        delete[] pRectangles;
}

void safe_VkPresentRegionKHR::initialize(const VkPresentRegionKHR* in_struct)
{
    rectangleCount = in_struct->rectangleCount;
    pRectangles = nullptr;
    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy ((void *)pRectangles, (void *)in_struct->pRectangles, sizeof(VkRectLayerKHR)*in_struct->rectangleCount);
    }
}

void safe_VkPresentRegionKHR::initialize(const safe_VkPresentRegionKHR* src)
{
    rectangleCount = src->rectangleCount;
    pRectangles = nullptr;
    if (src->pRectangles) {
        pRectangles = new VkRectLayerKHR[src->rectangleCount];
        memcpy ((void *)pRectangles, (void *)src->pRectangles, sizeof(VkRectLayerKHR)*src->rectangleCount);
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const VkPresentRegionsKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchainCount(in_struct->swapchainCount),
    pRegions(nullptr)
{
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR() :
    pRegions(nullptr)
{}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const safe_VkPresentRegionsKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pRegions = nullptr;
    if (swapchainCount && src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pRegions[i].initialize(&src.pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR& safe_VkPresentRegionsKHR::operator=(const safe_VkPresentRegionsKHR& src)
{
    if (&src == this) return *this;

    if (pRegions)
        delete[] pRegions;

    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pRegions = nullptr;
    if (swapchainCount && src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pRegions[i].initialize(&src.pRegions[i]);
        }
    }

    return *this;
}

safe_VkPresentRegionsKHR::~safe_VkPresentRegionsKHR()
{
    if (pRegions)
        delete[] pRegions;
}

void safe_VkPresentRegionsKHR::initialize(const VkPresentRegionsKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchainCount = in_struct->swapchainCount;
    pRegions = nullptr;
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

void safe_VkPresentRegionsKHR::initialize(const safe_VkPresentRegionsKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchainCount = src->swapchainCount;
    pRegions = nullptr;
    if (swapchainCount && src->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i=0; i<swapchainCount; ++i) {
            pRegions[i].initialize(&src->pRegions[i]);
        }
    }
}

safe_VkAttachmentDescription2KHR::safe_VkAttachmentDescription2KHR(const VkAttachmentDescription2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    format(in_struct->format),
    samples(in_struct->samples),
    loadOp(in_struct->loadOp),
    storeOp(in_struct->storeOp),
    stencilLoadOp(in_struct->stencilLoadOp),
    stencilStoreOp(in_struct->stencilStoreOp),
    initialLayout(in_struct->initialLayout),
    finalLayout(in_struct->finalLayout)
{
}

safe_VkAttachmentDescription2KHR::safe_VkAttachmentDescription2KHR()
{}

safe_VkAttachmentDescription2KHR::safe_VkAttachmentDescription2KHR(const safe_VkAttachmentDescription2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    format = src.format;
    samples = src.samples;
    loadOp = src.loadOp;
    storeOp = src.storeOp;
    stencilLoadOp = src.stencilLoadOp;
    stencilStoreOp = src.stencilStoreOp;
    initialLayout = src.initialLayout;
    finalLayout = src.finalLayout;
}

safe_VkAttachmentDescription2KHR& safe_VkAttachmentDescription2KHR::operator=(const safe_VkAttachmentDescription2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    format = src.format;
    samples = src.samples;
    loadOp = src.loadOp;
    storeOp = src.storeOp;
    stencilLoadOp = src.stencilLoadOp;
    stencilStoreOp = src.stencilStoreOp;
    initialLayout = src.initialLayout;
    finalLayout = src.finalLayout;

    return *this;
}

safe_VkAttachmentDescription2KHR::~safe_VkAttachmentDescription2KHR()
{
}

void safe_VkAttachmentDescription2KHR::initialize(const VkAttachmentDescription2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    format = in_struct->format;
    samples = in_struct->samples;
    loadOp = in_struct->loadOp;
    storeOp = in_struct->storeOp;
    stencilLoadOp = in_struct->stencilLoadOp;
    stencilStoreOp = in_struct->stencilStoreOp;
    initialLayout = in_struct->initialLayout;
    finalLayout = in_struct->finalLayout;
}

void safe_VkAttachmentDescription2KHR::initialize(const safe_VkAttachmentDescription2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    format = src->format;
    samples = src->samples;
    loadOp = src->loadOp;
    storeOp = src->storeOp;
    stencilLoadOp = src->stencilLoadOp;
    stencilStoreOp = src->stencilStoreOp;
    initialLayout = src->initialLayout;
    finalLayout = src->finalLayout;
}

safe_VkAttachmentReference2KHR::safe_VkAttachmentReference2KHR(const VkAttachmentReference2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    attachment(in_struct->attachment),
    layout(in_struct->layout),
    aspectMask(in_struct->aspectMask)
{
}

safe_VkAttachmentReference2KHR::safe_VkAttachmentReference2KHR()
{}

safe_VkAttachmentReference2KHR::safe_VkAttachmentReference2KHR(const safe_VkAttachmentReference2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    attachment = src.attachment;
    layout = src.layout;
    aspectMask = src.aspectMask;
}

safe_VkAttachmentReference2KHR& safe_VkAttachmentReference2KHR::operator=(const safe_VkAttachmentReference2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    attachment = src.attachment;
    layout = src.layout;
    aspectMask = src.aspectMask;

    return *this;
}

safe_VkAttachmentReference2KHR::~safe_VkAttachmentReference2KHR()
{
}

void safe_VkAttachmentReference2KHR::initialize(const VkAttachmentReference2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    attachment = in_struct->attachment;
    layout = in_struct->layout;
    aspectMask = in_struct->aspectMask;
}

void safe_VkAttachmentReference2KHR::initialize(const safe_VkAttachmentReference2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    attachment = src->attachment;
    layout = src->layout;
    aspectMask = src->aspectMask;
}

safe_VkSubpassDescription2KHR::safe_VkSubpassDescription2KHR(const VkSubpassDescription2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pipelineBindPoint(in_struct->pipelineBindPoint),
    viewMask(in_struct->viewMask),
    inputAttachmentCount(in_struct->inputAttachmentCount),
    pInputAttachments(nullptr),
    colorAttachmentCount(in_struct->colorAttachmentCount),
    pColorAttachments(nullptr),
    pResolveAttachments(nullptr),
    preserveAttachmentCount(in_struct->preserveAttachmentCount),
    pPreserveAttachments(nullptr)
{
    if (inputAttachmentCount && in_struct->pInputAttachments) {
        pInputAttachments = new safe_VkAttachmentReference2KHR[inputAttachmentCount];
        for (uint32_t i=0; i<inputAttachmentCount; ++i) {
            pInputAttachments[i].initialize(&in_struct->pInputAttachments[i]);
        }
    }
    if (colorAttachmentCount && in_struct->pColorAttachments) {
        pColorAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pColorAttachments[i].initialize(&in_struct->pColorAttachments[i]);
        }
    }
    if (colorAttachmentCount && in_struct->pResolveAttachments) {
        pResolveAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pResolveAttachments[i].initialize(&in_struct->pResolveAttachments[i]);
        }
    }
    if (in_struct->pDepthStencilAttachment)
        pDepthStencilAttachment = new safe_VkAttachmentReference2KHR(in_struct->pDepthStencilAttachment);
    else
        pDepthStencilAttachment = NULL;
    if (in_struct->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[in_struct->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)in_struct->pPreserveAttachments, sizeof(uint32_t)*in_struct->preserveAttachmentCount);
    }
}

safe_VkSubpassDescription2KHR::safe_VkSubpassDescription2KHR() :
    pInputAttachments(nullptr),
    pColorAttachments(nullptr),
    pResolveAttachments(nullptr),
    pPreserveAttachments(nullptr)
{}

safe_VkSubpassDescription2KHR::safe_VkSubpassDescription2KHR(const safe_VkSubpassDescription2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pipelineBindPoint = src.pipelineBindPoint;
    viewMask = src.viewMask;
    inputAttachmentCount = src.inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src.colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    preserveAttachmentCount = src.preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (inputAttachmentCount && src.pInputAttachments) {
        pInputAttachments = new safe_VkAttachmentReference2KHR[inputAttachmentCount];
        for (uint32_t i=0; i<inputAttachmentCount; ++i) {
            pInputAttachments[i].initialize(&src.pInputAttachments[i]);
        }
    }
    if (colorAttachmentCount && src.pColorAttachments) {
        pColorAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pColorAttachments[i].initialize(&src.pColorAttachments[i]);
        }
    }
    if (colorAttachmentCount && src.pResolveAttachments) {
        pResolveAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pResolveAttachments[i].initialize(&src.pResolveAttachments[i]);
        }
    }
    if (src.pDepthStencilAttachment)
        pDepthStencilAttachment = new safe_VkAttachmentReference2KHR(*src.pDepthStencilAttachment);
    else
        pDepthStencilAttachment = NULL;
    if (src.pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src.preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src.pPreserveAttachments, sizeof(uint32_t)*src.preserveAttachmentCount);
    }
}

safe_VkSubpassDescription2KHR& safe_VkSubpassDescription2KHR::operator=(const safe_VkSubpassDescription2KHR& src)
{
    if (&src == this) return *this;

    if (pInputAttachments)
        delete[] pInputAttachments;
    if (pColorAttachments)
        delete[] pColorAttachments;
    if (pResolveAttachments)
        delete[] pResolveAttachments;
    if (pDepthStencilAttachment)
        delete pDepthStencilAttachment;
    if (pPreserveAttachments)
        delete[] pPreserveAttachments;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pipelineBindPoint = src.pipelineBindPoint;
    viewMask = src.viewMask;
    inputAttachmentCount = src.inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src.colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    preserveAttachmentCount = src.preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (inputAttachmentCount && src.pInputAttachments) {
        pInputAttachments = new safe_VkAttachmentReference2KHR[inputAttachmentCount];
        for (uint32_t i=0; i<inputAttachmentCount; ++i) {
            pInputAttachments[i].initialize(&src.pInputAttachments[i]);
        }
    }
    if (colorAttachmentCount && src.pColorAttachments) {
        pColorAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pColorAttachments[i].initialize(&src.pColorAttachments[i]);
        }
    }
    if (colorAttachmentCount && src.pResolveAttachments) {
        pResolveAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pResolveAttachments[i].initialize(&src.pResolveAttachments[i]);
        }
    }
    if (src.pDepthStencilAttachment)
        pDepthStencilAttachment = new safe_VkAttachmentReference2KHR(*src.pDepthStencilAttachment);
    else
        pDepthStencilAttachment = NULL;
    if (src.pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src.preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src.pPreserveAttachments, sizeof(uint32_t)*src.preserveAttachmentCount);
    }

    return *this;
}

safe_VkSubpassDescription2KHR::~safe_VkSubpassDescription2KHR()
{
    if (pInputAttachments)
        delete[] pInputAttachments;
    if (pColorAttachments)
        delete[] pColorAttachments;
    if (pResolveAttachments)
        delete[] pResolveAttachments;
    if (pDepthStencilAttachment)
        delete pDepthStencilAttachment;
    if (pPreserveAttachments)
        delete[] pPreserveAttachments;
}

void safe_VkSubpassDescription2KHR::initialize(const VkSubpassDescription2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    viewMask = in_struct->viewMask;
    inputAttachmentCount = in_struct->inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = in_struct->colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    preserveAttachmentCount = in_struct->preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (inputAttachmentCount && in_struct->pInputAttachments) {
        pInputAttachments = new safe_VkAttachmentReference2KHR[inputAttachmentCount];
        for (uint32_t i=0; i<inputAttachmentCount; ++i) {
            pInputAttachments[i].initialize(&in_struct->pInputAttachments[i]);
        }
    }
    if (colorAttachmentCount && in_struct->pColorAttachments) {
        pColorAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pColorAttachments[i].initialize(&in_struct->pColorAttachments[i]);
        }
    }
    if (colorAttachmentCount && in_struct->pResolveAttachments) {
        pResolveAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pResolveAttachments[i].initialize(&in_struct->pResolveAttachments[i]);
        }
    }
    if (in_struct->pDepthStencilAttachment)
        pDepthStencilAttachment = new safe_VkAttachmentReference2KHR(in_struct->pDepthStencilAttachment);
    else
        pDepthStencilAttachment = NULL;
    if (in_struct->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[in_struct->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)in_struct->pPreserveAttachments, sizeof(uint32_t)*in_struct->preserveAttachmentCount);
    }
}

void safe_VkSubpassDescription2KHR::initialize(const safe_VkSubpassDescription2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pipelineBindPoint = src->pipelineBindPoint;
    viewMask = src->viewMask;
    inputAttachmentCount = src->inputAttachmentCount;
    pInputAttachments = nullptr;
    colorAttachmentCount = src->colorAttachmentCount;
    pColorAttachments = nullptr;
    pResolveAttachments = nullptr;
    preserveAttachmentCount = src->preserveAttachmentCount;
    pPreserveAttachments = nullptr;
    if (inputAttachmentCount && src->pInputAttachments) {
        pInputAttachments = new safe_VkAttachmentReference2KHR[inputAttachmentCount];
        for (uint32_t i=0; i<inputAttachmentCount; ++i) {
            pInputAttachments[i].initialize(&src->pInputAttachments[i]);
        }
    }
    if (colorAttachmentCount && src->pColorAttachments) {
        pColorAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pColorAttachments[i].initialize(&src->pColorAttachments[i]);
        }
    }
    if (colorAttachmentCount && src->pResolveAttachments) {
        pResolveAttachments = new safe_VkAttachmentReference2KHR[colorAttachmentCount];
        for (uint32_t i=0; i<colorAttachmentCount; ++i) {
            pResolveAttachments[i].initialize(&src->pResolveAttachments[i]);
        }
    }
    if (src->pDepthStencilAttachment)
        pDepthStencilAttachment = new safe_VkAttachmentReference2KHR(*src->pDepthStencilAttachment);
    else
        pDepthStencilAttachment = NULL;
    if (src->pPreserveAttachments) {
        pPreserveAttachments = new uint32_t[src->preserveAttachmentCount];
        memcpy ((void *)pPreserveAttachments, (void *)src->pPreserveAttachments, sizeof(uint32_t)*src->preserveAttachmentCount);
    }
}

safe_VkSubpassDependency2KHR::safe_VkSubpassDependency2KHR(const VkSubpassDependency2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcSubpass(in_struct->srcSubpass),
    dstSubpass(in_struct->dstSubpass),
    srcStageMask(in_struct->srcStageMask),
    dstStageMask(in_struct->dstStageMask),
    srcAccessMask(in_struct->srcAccessMask),
    dstAccessMask(in_struct->dstAccessMask),
    dependencyFlags(in_struct->dependencyFlags),
    viewOffset(in_struct->viewOffset)
{
}

safe_VkSubpassDependency2KHR::safe_VkSubpassDependency2KHR()
{}

safe_VkSubpassDependency2KHR::safe_VkSubpassDependency2KHR(const safe_VkSubpassDependency2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcSubpass = src.srcSubpass;
    dstSubpass = src.dstSubpass;
    srcStageMask = src.srcStageMask;
    dstStageMask = src.dstStageMask;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    dependencyFlags = src.dependencyFlags;
    viewOffset = src.viewOffset;
}

safe_VkSubpassDependency2KHR& safe_VkSubpassDependency2KHR::operator=(const safe_VkSubpassDependency2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcSubpass = src.srcSubpass;
    dstSubpass = src.dstSubpass;
    srcStageMask = src.srcStageMask;
    dstStageMask = src.dstStageMask;
    srcAccessMask = src.srcAccessMask;
    dstAccessMask = src.dstAccessMask;
    dependencyFlags = src.dependencyFlags;
    viewOffset = src.viewOffset;

    return *this;
}

safe_VkSubpassDependency2KHR::~safe_VkSubpassDependency2KHR()
{
}

void safe_VkSubpassDependency2KHR::initialize(const VkSubpassDependency2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcSubpass = in_struct->srcSubpass;
    dstSubpass = in_struct->dstSubpass;
    srcStageMask = in_struct->srcStageMask;
    dstStageMask = in_struct->dstStageMask;
    srcAccessMask = in_struct->srcAccessMask;
    dstAccessMask = in_struct->dstAccessMask;
    dependencyFlags = in_struct->dependencyFlags;
    viewOffset = in_struct->viewOffset;
}

void safe_VkSubpassDependency2KHR::initialize(const safe_VkSubpassDependency2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcSubpass = src->srcSubpass;
    dstSubpass = src->dstSubpass;
    srcStageMask = src->srcStageMask;
    dstStageMask = src->dstStageMask;
    srcAccessMask = src->srcAccessMask;
    dstAccessMask = src->dstAccessMask;
    dependencyFlags = src->dependencyFlags;
    viewOffset = src->viewOffset;
}

safe_VkRenderPassCreateInfo2KHR::safe_VkRenderPassCreateInfo2KHR(const VkRenderPassCreateInfo2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    attachmentCount(in_struct->attachmentCount),
    pAttachments(nullptr),
    subpassCount(in_struct->subpassCount),
    pSubpasses(nullptr),
    dependencyCount(in_struct->dependencyCount),
    pDependencies(nullptr),
    correlatedViewMaskCount(in_struct->correlatedViewMaskCount),
    pCorrelatedViewMasks(nullptr)
{
    if (attachmentCount && in_struct->pAttachments) {
        pAttachments = new safe_VkAttachmentDescription2KHR[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i].initialize(&in_struct->pAttachments[i]);
        }
    }
    if (subpassCount && in_struct->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription2KHR[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&in_struct->pSubpasses[i]);
        }
    }
    if (dependencyCount && in_struct->pDependencies) {
        pDependencies = new safe_VkSubpassDependency2KHR[dependencyCount];
        for (uint32_t i=0; i<dependencyCount; ++i) {
            pDependencies[i].initialize(&in_struct->pDependencies[i]);
        }
    }
    if (in_struct->pCorrelatedViewMasks) {
        pCorrelatedViewMasks = new uint32_t[in_struct->correlatedViewMaskCount];
        memcpy ((void *)pCorrelatedViewMasks, (void *)in_struct->pCorrelatedViewMasks, sizeof(uint32_t)*in_struct->correlatedViewMaskCount);
    }
}

safe_VkRenderPassCreateInfo2KHR::safe_VkRenderPassCreateInfo2KHR() :
    pAttachments(nullptr),
    pSubpasses(nullptr),
    pDependencies(nullptr),
    pCorrelatedViewMasks(nullptr)
{}

safe_VkRenderPassCreateInfo2KHR::safe_VkRenderPassCreateInfo2KHR(const safe_VkRenderPassCreateInfo2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    subpassCount = src.subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src.dependencyCount;
    pDependencies = nullptr;
    correlatedViewMaskCount = src.correlatedViewMaskCount;
    pCorrelatedViewMasks = nullptr;
    if (attachmentCount && src.pAttachments) {
        pAttachments = new safe_VkAttachmentDescription2KHR[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i].initialize(&src.pAttachments[i]);
        }
    }
    if (subpassCount && src.pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription2KHR[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src.pSubpasses[i]);
        }
    }
    if (dependencyCount && src.pDependencies) {
        pDependencies = new safe_VkSubpassDependency2KHR[dependencyCount];
        for (uint32_t i=0; i<dependencyCount; ++i) {
            pDependencies[i].initialize(&src.pDependencies[i]);
        }
    }
    if (src.pCorrelatedViewMasks) {
        pCorrelatedViewMasks = new uint32_t[src.correlatedViewMaskCount];
        memcpy ((void *)pCorrelatedViewMasks, (void *)src.pCorrelatedViewMasks, sizeof(uint32_t)*src.correlatedViewMaskCount);
    }
}

safe_VkRenderPassCreateInfo2KHR& safe_VkRenderPassCreateInfo2KHR::operator=(const safe_VkRenderPassCreateInfo2KHR& src)
{
    if (&src == this) return *this;

    if (pAttachments)
        delete[] pAttachments;
    if (pSubpasses)
        delete[] pSubpasses;
    if (pDependencies)
        delete[] pDependencies;
    if (pCorrelatedViewMasks)
        delete[] pCorrelatedViewMasks;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    attachmentCount = src.attachmentCount;
    pAttachments = nullptr;
    subpassCount = src.subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src.dependencyCount;
    pDependencies = nullptr;
    correlatedViewMaskCount = src.correlatedViewMaskCount;
    pCorrelatedViewMasks = nullptr;
    if (attachmentCount && src.pAttachments) {
        pAttachments = new safe_VkAttachmentDescription2KHR[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i].initialize(&src.pAttachments[i]);
        }
    }
    if (subpassCount && src.pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription2KHR[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src.pSubpasses[i]);
        }
    }
    if (dependencyCount && src.pDependencies) {
        pDependencies = new safe_VkSubpassDependency2KHR[dependencyCount];
        for (uint32_t i=0; i<dependencyCount; ++i) {
            pDependencies[i].initialize(&src.pDependencies[i]);
        }
    }
    if (src.pCorrelatedViewMasks) {
        pCorrelatedViewMasks = new uint32_t[src.correlatedViewMaskCount];
        memcpy ((void *)pCorrelatedViewMasks, (void *)src.pCorrelatedViewMasks, sizeof(uint32_t)*src.correlatedViewMaskCount);
    }

    return *this;
}

safe_VkRenderPassCreateInfo2KHR::~safe_VkRenderPassCreateInfo2KHR()
{
    if (pAttachments)
        delete[] pAttachments;
    if (pSubpasses)
        delete[] pSubpasses;
    if (pDependencies)
        delete[] pDependencies;
    if (pCorrelatedViewMasks)
        delete[] pCorrelatedViewMasks;
}

void safe_VkRenderPassCreateInfo2KHR::initialize(const VkRenderPassCreateInfo2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    attachmentCount = in_struct->attachmentCount;
    pAttachments = nullptr;
    subpassCount = in_struct->subpassCount;
    pSubpasses = nullptr;
    dependencyCount = in_struct->dependencyCount;
    pDependencies = nullptr;
    correlatedViewMaskCount = in_struct->correlatedViewMaskCount;
    pCorrelatedViewMasks = nullptr;
    if (attachmentCount && in_struct->pAttachments) {
        pAttachments = new safe_VkAttachmentDescription2KHR[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i].initialize(&in_struct->pAttachments[i]);
        }
    }
    if (subpassCount && in_struct->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription2KHR[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&in_struct->pSubpasses[i]);
        }
    }
    if (dependencyCount && in_struct->pDependencies) {
        pDependencies = new safe_VkSubpassDependency2KHR[dependencyCount];
        for (uint32_t i=0; i<dependencyCount; ++i) {
            pDependencies[i].initialize(&in_struct->pDependencies[i]);
        }
    }
    if (in_struct->pCorrelatedViewMasks) {
        pCorrelatedViewMasks = new uint32_t[in_struct->correlatedViewMaskCount];
        memcpy ((void *)pCorrelatedViewMasks, (void *)in_struct->pCorrelatedViewMasks, sizeof(uint32_t)*in_struct->correlatedViewMaskCount);
    }
}

void safe_VkRenderPassCreateInfo2KHR::initialize(const safe_VkRenderPassCreateInfo2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    attachmentCount = src->attachmentCount;
    pAttachments = nullptr;
    subpassCount = src->subpassCount;
    pSubpasses = nullptr;
    dependencyCount = src->dependencyCount;
    pDependencies = nullptr;
    correlatedViewMaskCount = src->correlatedViewMaskCount;
    pCorrelatedViewMasks = nullptr;
    if (attachmentCount && src->pAttachments) {
        pAttachments = new safe_VkAttachmentDescription2KHR[attachmentCount];
        for (uint32_t i=0; i<attachmentCount; ++i) {
            pAttachments[i].initialize(&src->pAttachments[i]);
        }
    }
    if (subpassCount && src->pSubpasses) {
        pSubpasses = new safe_VkSubpassDescription2KHR[subpassCount];
        for (uint32_t i=0; i<subpassCount; ++i) {
            pSubpasses[i].initialize(&src->pSubpasses[i]);
        }
    }
    if (dependencyCount && src->pDependencies) {
        pDependencies = new safe_VkSubpassDependency2KHR[dependencyCount];
        for (uint32_t i=0; i<dependencyCount; ++i) {
            pDependencies[i].initialize(&src->pDependencies[i]);
        }
    }
    if (src->pCorrelatedViewMasks) {
        pCorrelatedViewMasks = new uint32_t[src->correlatedViewMaskCount];
        memcpy ((void *)pCorrelatedViewMasks, (void *)src->pCorrelatedViewMasks, sizeof(uint32_t)*src->correlatedViewMaskCount);
    }
}

safe_VkSubpassBeginInfoKHR::safe_VkSubpassBeginInfoKHR(const VkSubpassBeginInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    contents(in_struct->contents)
{
}

safe_VkSubpassBeginInfoKHR::safe_VkSubpassBeginInfoKHR()
{}

safe_VkSubpassBeginInfoKHR::safe_VkSubpassBeginInfoKHR(const safe_VkSubpassBeginInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    contents = src.contents;
}

safe_VkSubpassBeginInfoKHR& safe_VkSubpassBeginInfoKHR::operator=(const safe_VkSubpassBeginInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    contents = src.contents;

    return *this;
}

safe_VkSubpassBeginInfoKHR::~safe_VkSubpassBeginInfoKHR()
{
}

void safe_VkSubpassBeginInfoKHR::initialize(const VkSubpassBeginInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    contents = in_struct->contents;
}

void safe_VkSubpassBeginInfoKHR::initialize(const safe_VkSubpassBeginInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    contents = src->contents;
}

safe_VkSubpassEndInfoKHR::safe_VkSubpassEndInfoKHR(const VkSubpassEndInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext)
{
}

safe_VkSubpassEndInfoKHR::safe_VkSubpassEndInfoKHR()
{}

safe_VkSubpassEndInfoKHR::safe_VkSubpassEndInfoKHR(const safe_VkSubpassEndInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
}

safe_VkSubpassEndInfoKHR& safe_VkSubpassEndInfoKHR::operator=(const safe_VkSubpassEndInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;

    return *this;
}

safe_VkSubpassEndInfoKHR::~safe_VkSubpassEndInfoKHR()
{
}

void safe_VkSubpassEndInfoKHR::initialize(const VkSubpassEndInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
}

void safe_VkSubpassEndInfoKHR::initialize(const safe_VkSubpassEndInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    sharedPresentSupportedUsageFlags(in_struct->sharedPresentSupportedUsageFlags)
{
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR()
{}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(const safe_VkSharedPresentSurfaceCapabilitiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    sharedPresentSupportedUsageFlags = src.sharedPresentSupportedUsageFlags;
}

safe_VkSharedPresentSurfaceCapabilitiesKHR& safe_VkSharedPresentSurfaceCapabilitiesKHR::operator=(const safe_VkSharedPresentSurfaceCapabilitiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    sharedPresentSupportedUsageFlags = src.sharedPresentSupportedUsageFlags;

    return *this;
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::~safe_VkSharedPresentSurfaceCapabilitiesKHR()
{
}

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    sharedPresentSupportedUsageFlags = in_struct->sharedPresentSupportedUsageFlags;
}

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const safe_VkSharedPresentSurfaceCapabilitiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    sharedPresentSupportedUsageFlags = src->sharedPresentSupportedUsageFlags;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const VkImportFenceWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fence(in_struct->fence),
    flags(in_struct->flags),
    handleType(in_struct->handleType),
    handle(in_struct->handle),
    name(in_struct->name)
{
}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR()
{}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const safe_VkImportFenceWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    flags = src.flags;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;
}

safe_VkImportFenceWin32HandleInfoKHR& safe_VkImportFenceWin32HandleInfoKHR::operator=(const safe_VkImportFenceWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    flags = src.flags;
    handleType = src.handleType;
    handle = src.handle;
    name = src.name;

    return *this;
}

safe_VkImportFenceWin32HandleInfoKHR::~safe_VkImportFenceWin32HandleInfoKHR()
{
}

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const VkImportFenceWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
}

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const safe_VkImportFenceWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fence = src->fence;
    flags = src->flags;
    handleType = src->handleType;
    handle = src->handle;
    name = src->name;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const VkExportFenceWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pAttributes(nullptr),
    dwAccess(in_struct->dwAccess),
    name(in_struct->name)
{
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR() :
    pAttributes(nullptr)
{}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const safe_VkExportFenceWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR& safe_VkExportFenceWin32HandleInfoKHR::operator=(const safe_VkExportFenceWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;

    if (pAttributes)
        delete pAttributes;

    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    name = src.name;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }

    return *this;
}

safe_VkExportFenceWin32HandleInfoKHR::~safe_VkExportFenceWin32HandleInfoKHR()
{
    if (pAttributes)
        delete pAttributes;
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const VkExportFenceWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const safe_VkExportFenceWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pAttributes = nullptr;
    dwAccess = src->dwAccess;
    name = src->name;
    if (src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src->pAttributes);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const VkFenceGetWin32HandleInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fence(in_struct->fence),
    handleType(in_struct->handleType)
{
}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR()
{}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const safe_VkFenceGetWin32HandleInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    handleType = src.handleType;
}

safe_VkFenceGetWin32HandleInfoKHR& safe_VkFenceGetWin32HandleInfoKHR::operator=(const safe_VkFenceGetWin32HandleInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    handleType = src.handleType;

    return *this;
}

safe_VkFenceGetWin32HandleInfoKHR::~safe_VkFenceGetWin32HandleInfoKHR()
{
}

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const VkFenceGetWin32HandleInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
}

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const safe_VkFenceGetWin32HandleInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fence = src->fence;
    handleType = src->handleType;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const VkImportFenceFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fence(in_struct->fence),
    flags(in_struct->flags),
    handleType(in_struct->handleType),
    fd(in_struct->fd)
{
}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR()
{}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const safe_VkImportFenceFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    flags = src.flags;
    handleType = src.handleType;
    fd = src.fd;
}

safe_VkImportFenceFdInfoKHR& safe_VkImportFenceFdInfoKHR::operator=(const safe_VkImportFenceFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    flags = src.flags;
    handleType = src.handleType;
    fd = src.fd;

    return *this;
}

safe_VkImportFenceFdInfoKHR::~safe_VkImportFenceFdInfoKHR()
{
}

void safe_VkImportFenceFdInfoKHR::initialize(const VkImportFenceFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
}

void safe_VkImportFenceFdInfoKHR::initialize(const safe_VkImportFenceFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fence = src->fence;
    flags = src->flags;
    handleType = src->handleType;
    fd = src->fd;
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const VkFenceGetFdInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fence(in_struct->fence),
    handleType(in_struct->handleType)
{
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR()
{}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const safe_VkFenceGetFdInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    handleType = src.handleType;
}

safe_VkFenceGetFdInfoKHR& safe_VkFenceGetFdInfoKHR::operator=(const safe_VkFenceGetFdInfoKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fence = src.fence;
    handleType = src.handleType;

    return *this;
}

safe_VkFenceGetFdInfoKHR::~safe_VkFenceGetFdInfoKHR()
{
}

void safe_VkFenceGetFdInfoKHR::initialize(const VkFenceGetFdInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
}

void safe_VkFenceGetFdInfoKHR::initialize(const safe_VkFenceGetFdInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fence = src->fence;
    handleType = src->handleType;
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    surface(in_struct->surface)
{
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR()
{}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const safe_VkPhysicalDeviceSurfaceInfo2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    surface = src.surface;
}

safe_VkPhysicalDeviceSurfaceInfo2KHR& safe_VkPhysicalDeviceSurfaceInfo2KHR::operator=(const safe_VkPhysicalDeviceSurfaceInfo2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    surface = src.surface;

    return *this;
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::~safe_VkPhysicalDeviceSurfaceInfo2KHR()
{
}

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    surface = in_struct->surface;
}

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const safe_VkPhysicalDeviceSurfaceInfo2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    surface = src->surface;
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const VkSurfaceCapabilities2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    surfaceCapabilities(in_struct->surfaceCapabilities)
{
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR()
{}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const safe_VkSurfaceCapabilities2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    surfaceCapabilities = src.surfaceCapabilities;
}

safe_VkSurfaceCapabilities2KHR& safe_VkSurfaceCapabilities2KHR::operator=(const safe_VkSurfaceCapabilities2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    surfaceCapabilities = src.surfaceCapabilities;

    return *this;
}

safe_VkSurfaceCapabilities2KHR::~safe_VkSurfaceCapabilities2KHR()
{
}

void safe_VkSurfaceCapabilities2KHR::initialize(const VkSurfaceCapabilities2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    surfaceCapabilities = in_struct->surfaceCapabilities;
}

void safe_VkSurfaceCapabilities2KHR::initialize(const safe_VkSurfaceCapabilities2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    surfaceCapabilities = src->surfaceCapabilities;
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const VkSurfaceFormat2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    surfaceFormat(in_struct->surfaceFormat)
{
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR()
{}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const safe_VkSurfaceFormat2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    surfaceFormat = src.surfaceFormat;
}

safe_VkSurfaceFormat2KHR& safe_VkSurfaceFormat2KHR::operator=(const safe_VkSurfaceFormat2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    surfaceFormat = src.surfaceFormat;

    return *this;
}

safe_VkSurfaceFormat2KHR::~safe_VkSurfaceFormat2KHR()
{
}

void safe_VkSurfaceFormat2KHR::initialize(const VkSurfaceFormat2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    surfaceFormat = in_struct->surfaceFormat;
}

void safe_VkSurfaceFormat2KHR::initialize(const safe_VkSurfaceFormat2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    surfaceFormat = src->surfaceFormat;
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const VkDisplayProperties2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    displayProperties(&in_struct->displayProperties)
{
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR()
{}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const safe_VkDisplayProperties2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    displayProperties.initialize(&src.displayProperties);
}

safe_VkDisplayProperties2KHR& safe_VkDisplayProperties2KHR::operator=(const safe_VkDisplayProperties2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    displayProperties.initialize(&src.displayProperties);

    return *this;
}

safe_VkDisplayProperties2KHR::~safe_VkDisplayProperties2KHR()
{
}

void safe_VkDisplayProperties2KHR::initialize(const VkDisplayProperties2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    displayProperties.initialize(&in_struct->displayProperties);
}

void safe_VkDisplayProperties2KHR::initialize(const safe_VkDisplayProperties2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    displayProperties.initialize(&src->displayProperties);
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const VkDisplayPlaneProperties2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    displayPlaneProperties(in_struct->displayPlaneProperties)
{
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR()
{}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const safe_VkDisplayPlaneProperties2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    displayPlaneProperties = src.displayPlaneProperties;
}

safe_VkDisplayPlaneProperties2KHR& safe_VkDisplayPlaneProperties2KHR::operator=(const safe_VkDisplayPlaneProperties2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    displayPlaneProperties = src.displayPlaneProperties;

    return *this;
}

safe_VkDisplayPlaneProperties2KHR::~safe_VkDisplayPlaneProperties2KHR()
{
}

void safe_VkDisplayPlaneProperties2KHR::initialize(const VkDisplayPlaneProperties2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    displayPlaneProperties = in_struct->displayPlaneProperties;
}

void safe_VkDisplayPlaneProperties2KHR::initialize(const safe_VkDisplayPlaneProperties2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    displayPlaneProperties = src->displayPlaneProperties;
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const VkDisplayModeProperties2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    displayModeProperties(in_struct->displayModeProperties)
{
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR()
{}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const safe_VkDisplayModeProperties2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    displayModeProperties = src.displayModeProperties;
}

safe_VkDisplayModeProperties2KHR& safe_VkDisplayModeProperties2KHR::operator=(const safe_VkDisplayModeProperties2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    displayModeProperties = src.displayModeProperties;

    return *this;
}

safe_VkDisplayModeProperties2KHR::~safe_VkDisplayModeProperties2KHR()
{
}

void safe_VkDisplayModeProperties2KHR::initialize(const VkDisplayModeProperties2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    displayModeProperties = in_struct->displayModeProperties;
}

void safe_VkDisplayModeProperties2KHR::initialize(const safe_VkDisplayModeProperties2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    displayModeProperties = src->displayModeProperties;
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const VkDisplayPlaneInfo2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    mode(in_struct->mode),
    planeIndex(in_struct->planeIndex)
{
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR()
{}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const safe_VkDisplayPlaneInfo2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    mode = src.mode;
    planeIndex = src.planeIndex;
}

safe_VkDisplayPlaneInfo2KHR& safe_VkDisplayPlaneInfo2KHR::operator=(const safe_VkDisplayPlaneInfo2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    mode = src.mode;
    planeIndex = src.planeIndex;

    return *this;
}

safe_VkDisplayPlaneInfo2KHR::~safe_VkDisplayPlaneInfo2KHR()
{
}

void safe_VkDisplayPlaneInfo2KHR::initialize(const VkDisplayPlaneInfo2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    mode = in_struct->mode;
    planeIndex = in_struct->planeIndex;
}

void safe_VkDisplayPlaneInfo2KHR::initialize(const safe_VkDisplayPlaneInfo2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    mode = src->mode;
    planeIndex = src->planeIndex;
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const VkDisplayPlaneCapabilities2KHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    capabilities(in_struct->capabilities)
{
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR()
{}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const safe_VkDisplayPlaneCapabilities2KHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    capabilities = src.capabilities;
}

safe_VkDisplayPlaneCapabilities2KHR& safe_VkDisplayPlaneCapabilities2KHR::operator=(const safe_VkDisplayPlaneCapabilities2KHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    capabilities = src.capabilities;

    return *this;
}

safe_VkDisplayPlaneCapabilities2KHR::~safe_VkDisplayPlaneCapabilities2KHR()
{
}

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const VkDisplayPlaneCapabilities2KHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    capabilities = in_struct->capabilities;
}

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const safe_VkDisplayPlaneCapabilities2KHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    capabilities = src->capabilities;
}

safe_VkImageFormatListCreateInfoKHR::safe_VkImageFormatListCreateInfoKHR(const VkImageFormatListCreateInfoKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    viewFormatCount(in_struct->viewFormatCount),
    pViewFormats(nullptr)
{
    if (in_struct->pViewFormats) {
        pViewFormats = new VkFormat[in_struct->viewFormatCount];
        memcpy ((void *)pViewFormats, (void *)in_struct->pViewFormats, sizeof(VkFormat)*in_struct->viewFormatCount);
    }
}

safe_VkImageFormatListCreateInfoKHR::safe_VkImageFormatListCreateInfoKHR() :
    pViewFormats(nullptr)
{}

safe_VkImageFormatListCreateInfoKHR::safe_VkImageFormatListCreateInfoKHR(const safe_VkImageFormatListCreateInfoKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    viewFormatCount = src.viewFormatCount;
    pViewFormats = nullptr;
    if (src.pViewFormats) {
        pViewFormats = new VkFormat[src.viewFormatCount];
        memcpy ((void *)pViewFormats, (void *)src.pViewFormats, sizeof(VkFormat)*src.viewFormatCount);
    }
}

safe_VkImageFormatListCreateInfoKHR& safe_VkImageFormatListCreateInfoKHR::operator=(const safe_VkImageFormatListCreateInfoKHR& src)
{
    if (&src == this) return *this;

    if (pViewFormats)
        delete[] pViewFormats;

    sType = src.sType;
    pNext = src.pNext;
    viewFormatCount = src.viewFormatCount;
    pViewFormats = nullptr;
    if (src.pViewFormats) {
        pViewFormats = new VkFormat[src.viewFormatCount];
        memcpy ((void *)pViewFormats, (void *)src.pViewFormats, sizeof(VkFormat)*src.viewFormatCount);
    }

    return *this;
}

safe_VkImageFormatListCreateInfoKHR::~safe_VkImageFormatListCreateInfoKHR()
{
    if (pViewFormats)
        delete[] pViewFormats;
}

void safe_VkImageFormatListCreateInfoKHR::initialize(const VkImageFormatListCreateInfoKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    viewFormatCount = in_struct->viewFormatCount;
    pViewFormats = nullptr;
    if (in_struct->pViewFormats) {
        pViewFormats = new VkFormat[in_struct->viewFormatCount];
        memcpy ((void *)pViewFormats, (void *)in_struct->pViewFormats, sizeof(VkFormat)*in_struct->viewFormatCount);
    }
}

void safe_VkImageFormatListCreateInfoKHR::initialize(const safe_VkImageFormatListCreateInfoKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    viewFormatCount = src->viewFormatCount;
    pViewFormats = nullptr;
    if (src->pViewFormats) {
        pViewFormats = new VkFormat[src->viewFormatCount];
        memcpy ((void *)pViewFormats, (void *)src->pViewFormats, sizeof(VkFormat)*src->viewFormatCount);
    }
}

safe_VkPhysicalDevice8BitStorageFeaturesKHR::safe_VkPhysicalDevice8BitStorageFeaturesKHR(const VkPhysicalDevice8BitStorageFeaturesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    storageBuffer8BitAccess(in_struct->storageBuffer8BitAccess),
    uniformAndStorageBuffer8BitAccess(in_struct->uniformAndStorageBuffer8BitAccess),
    storagePushConstant8(in_struct->storagePushConstant8)
{
}

safe_VkPhysicalDevice8BitStorageFeaturesKHR::safe_VkPhysicalDevice8BitStorageFeaturesKHR()
{}

safe_VkPhysicalDevice8BitStorageFeaturesKHR::safe_VkPhysicalDevice8BitStorageFeaturesKHR(const safe_VkPhysicalDevice8BitStorageFeaturesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    storageBuffer8BitAccess = src.storageBuffer8BitAccess;
    uniformAndStorageBuffer8BitAccess = src.uniformAndStorageBuffer8BitAccess;
    storagePushConstant8 = src.storagePushConstant8;
}

safe_VkPhysicalDevice8BitStorageFeaturesKHR& safe_VkPhysicalDevice8BitStorageFeaturesKHR::operator=(const safe_VkPhysicalDevice8BitStorageFeaturesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    storageBuffer8BitAccess = src.storageBuffer8BitAccess;
    uniformAndStorageBuffer8BitAccess = src.uniformAndStorageBuffer8BitAccess;
    storagePushConstant8 = src.storagePushConstant8;

    return *this;
}

safe_VkPhysicalDevice8BitStorageFeaturesKHR::~safe_VkPhysicalDevice8BitStorageFeaturesKHR()
{
}

void safe_VkPhysicalDevice8BitStorageFeaturesKHR::initialize(const VkPhysicalDevice8BitStorageFeaturesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    storageBuffer8BitAccess = in_struct->storageBuffer8BitAccess;
    uniformAndStorageBuffer8BitAccess = in_struct->uniformAndStorageBuffer8BitAccess;
    storagePushConstant8 = in_struct->storagePushConstant8;
}

void safe_VkPhysicalDevice8BitStorageFeaturesKHR::initialize(const safe_VkPhysicalDevice8BitStorageFeaturesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    storageBuffer8BitAccess = src->storageBuffer8BitAccess;
    uniformAndStorageBuffer8BitAccess = src->uniformAndStorageBuffer8BitAccess;
    storagePushConstant8 = src->storagePushConstant8;
}

safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR(const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderBufferInt64Atomics(in_struct->shaderBufferInt64Atomics),
    shaderSharedInt64Atomics(in_struct->shaderSharedInt64Atomics)
{
}

safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR()
{}

safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderBufferInt64Atomics = src.shaderBufferInt64Atomics;
    shaderSharedInt64Atomics = src.shaderSharedInt64Atomics;
}

safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::operator=(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderBufferInt64Atomics = src.shaderBufferInt64Atomics;
    shaderSharedInt64Atomics = src.shaderSharedInt64Atomics;

    return *this;
}

safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::~safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR()
{
}

void safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::initialize(const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderBufferInt64Atomics = in_struct->shaderBufferInt64Atomics;
    shaderSharedInt64Atomics = in_struct->shaderSharedInt64Atomics;
}

void safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR::initialize(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderBufferInt64Atomics = src->shaderBufferInt64Atomics;
    shaderSharedInt64Atomics = src->shaderSharedInt64Atomics;
}

safe_VkPhysicalDeviceDriverPropertiesKHR::safe_VkPhysicalDeviceDriverPropertiesKHR(const VkPhysicalDeviceDriverPropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    driverID(in_struct->driverID),
    conformanceVersion(in_struct->conformanceVersion)
{
    for (uint32_t i=0; i<VK_MAX_DRIVER_NAME_SIZE_KHR; ++i) {
        driverName[i] = in_struct->driverName[i];
    }
    for (uint32_t i=0; i<VK_MAX_DRIVER_INFO_SIZE_KHR; ++i) {
        driverInfo[i] = in_struct->driverInfo[i];
    }
}

safe_VkPhysicalDeviceDriverPropertiesKHR::safe_VkPhysicalDeviceDriverPropertiesKHR()
{}

safe_VkPhysicalDeviceDriverPropertiesKHR::safe_VkPhysicalDeviceDriverPropertiesKHR(const safe_VkPhysicalDeviceDriverPropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    driverID = src.driverID;
    conformanceVersion = src.conformanceVersion;
    for (uint32_t i=0; i<VK_MAX_DRIVER_NAME_SIZE_KHR; ++i) {
        driverName[i] = src.driverName[i];
    }
    for (uint32_t i=0; i<VK_MAX_DRIVER_INFO_SIZE_KHR; ++i) {
        driverInfo[i] = src.driverInfo[i];
    }
}

safe_VkPhysicalDeviceDriverPropertiesKHR& safe_VkPhysicalDeviceDriverPropertiesKHR::operator=(const safe_VkPhysicalDeviceDriverPropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    driverID = src.driverID;
    conformanceVersion = src.conformanceVersion;
    for (uint32_t i=0; i<VK_MAX_DRIVER_NAME_SIZE_KHR; ++i) {
        driverName[i] = src.driverName[i];
    }
    for (uint32_t i=0; i<VK_MAX_DRIVER_INFO_SIZE_KHR; ++i) {
        driverInfo[i] = src.driverInfo[i];
    }

    return *this;
}

safe_VkPhysicalDeviceDriverPropertiesKHR::~safe_VkPhysicalDeviceDriverPropertiesKHR()
{
}

void safe_VkPhysicalDeviceDriverPropertiesKHR::initialize(const VkPhysicalDeviceDriverPropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    driverID = in_struct->driverID;
    conformanceVersion = in_struct->conformanceVersion;
    for (uint32_t i=0; i<VK_MAX_DRIVER_NAME_SIZE_KHR; ++i) {
        driverName[i] = in_struct->driverName[i];
    }
    for (uint32_t i=0; i<VK_MAX_DRIVER_INFO_SIZE_KHR; ++i) {
        driverInfo[i] = in_struct->driverInfo[i];
    }
}

void safe_VkPhysicalDeviceDriverPropertiesKHR::initialize(const safe_VkPhysicalDeviceDriverPropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    driverID = src->driverID;
    conformanceVersion = src->conformanceVersion;
    for (uint32_t i=0; i<VK_MAX_DRIVER_NAME_SIZE_KHR; ++i) {
        driverName[i] = src->driverName[i];
    }
    for (uint32_t i=0; i<VK_MAX_DRIVER_INFO_SIZE_KHR; ++i) {
        driverInfo[i] = src->driverInfo[i];
    }
}

safe_VkPhysicalDeviceFloatControlsPropertiesKHR::safe_VkPhysicalDeviceFloatControlsPropertiesKHR(const VkPhysicalDeviceFloatControlsPropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    separateDenormSettings(in_struct->separateDenormSettings),
    separateRoundingModeSettings(in_struct->separateRoundingModeSettings),
    shaderSignedZeroInfNanPreserveFloat16(in_struct->shaderSignedZeroInfNanPreserveFloat16),
    shaderSignedZeroInfNanPreserveFloat32(in_struct->shaderSignedZeroInfNanPreserveFloat32),
    shaderSignedZeroInfNanPreserveFloat64(in_struct->shaderSignedZeroInfNanPreserveFloat64),
    shaderDenormPreserveFloat16(in_struct->shaderDenormPreserveFloat16),
    shaderDenormPreserveFloat32(in_struct->shaderDenormPreserveFloat32),
    shaderDenormPreserveFloat64(in_struct->shaderDenormPreserveFloat64),
    shaderDenormFlushToZeroFloat16(in_struct->shaderDenormFlushToZeroFloat16),
    shaderDenormFlushToZeroFloat32(in_struct->shaderDenormFlushToZeroFloat32),
    shaderDenormFlushToZeroFloat64(in_struct->shaderDenormFlushToZeroFloat64),
    shaderRoundingModeRTEFloat16(in_struct->shaderRoundingModeRTEFloat16),
    shaderRoundingModeRTEFloat32(in_struct->shaderRoundingModeRTEFloat32),
    shaderRoundingModeRTEFloat64(in_struct->shaderRoundingModeRTEFloat64),
    shaderRoundingModeRTZFloat16(in_struct->shaderRoundingModeRTZFloat16),
    shaderRoundingModeRTZFloat32(in_struct->shaderRoundingModeRTZFloat32),
    shaderRoundingModeRTZFloat64(in_struct->shaderRoundingModeRTZFloat64)
{
}

safe_VkPhysicalDeviceFloatControlsPropertiesKHR::safe_VkPhysicalDeviceFloatControlsPropertiesKHR()
{}

safe_VkPhysicalDeviceFloatControlsPropertiesKHR::safe_VkPhysicalDeviceFloatControlsPropertiesKHR(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    separateDenormSettings = src.separateDenormSettings;
    separateRoundingModeSettings = src.separateRoundingModeSettings;
    shaderSignedZeroInfNanPreserveFloat16 = src.shaderSignedZeroInfNanPreserveFloat16;
    shaderSignedZeroInfNanPreserveFloat32 = src.shaderSignedZeroInfNanPreserveFloat32;
    shaderSignedZeroInfNanPreserveFloat64 = src.shaderSignedZeroInfNanPreserveFloat64;
    shaderDenormPreserveFloat16 = src.shaderDenormPreserveFloat16;
    shaderDenormPreserveFloat32 = src.shaderDenormPreserveFloat32;
    shaderDenormPreserveFloat64 = src.shaderDenormPreserveFloat64;
    shaderDenormFlushToZeroFloat16 = src.shaderDenormFlushToZeroFloat16;
    shaderDenormFlushToZeroFloat32 = src.shaderDenormFlushToZeroFloat32;
    shaderDenormFlushToZeroFloat64 = src.shaderDenormFlushToZeroFloat64;
    shaderRoundingModeRTEFloat16 = src.shaderRoundingModeRTEFloat16;
    shaderRoundingModeRTEFloat32 = src.shaderRoundingModeRTEFloat32;
    shaderRoundingModeRTEFloat64 = src.shaderRoundingModeRTEFloat64;
    shaderRoundingModeRTZFloat16 = src.shaderRoundingModeRTZFloat16;
    shaderRoundingModeRTZFloat32 = src.shaderRoundingModeRTZFloat32;
    shaderRoundingModeRTZFloat64 = src.shaderRoundingModeRTZFloat64;
}

safe_VkPhysicalDeviceFloatControlsPropertiesKHR& safe_VkPhysicalDeviceFloatControlsPropertiesKHR::operator=(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    separateDenormSettings = src.separateDenormSettings;
    separateRoundingModeSettings = src.separateRoundingModeSettings;
    shaderSignedZeroInfNanPreserveFloat16 = src.shaderSignedZeroInfNanPreserveFloat16;
    shaderSignedZeroInfNanPreserveFloat32 = src.shaderSignedZeroInfNanPreserveFloat32;
    shaderSignedZeroInfNanPreserveFloat64 = src.shaderSignedZeroInfNanPreserveFloat64;
    shaderDenormPreserveFloat16 = src.shaderDenormPreserveFloat16;
    shaderDenormPreserveFloat32 = src.shaderDenormPreserveFloat32;
    shaderDenormPreserveFloat64 = src.shaderDenormPreserveFloat64;
    shaderDenormFlushToZeroFloat16 = src.shaderDenormFlushToZeroFloat16;
    shaderDenormFlushToZeroFloat32 = src.shaderDenormFlushToZeroFloat32;
    shaderDenormFlushToZeroFloat64 = src.shaderDenormFlushToZeroFloat64;
    shaderRoundingModeRTEFloat16 = src.shaderRoundingModeRTEFloat16;
    shaderRoundingModeRTEFloat32 = src.shaderRoundingModeRTEFloat32;
    shaderRoundingModeRTEFloat64 = src.shaderRoundingModeRTEFloat64;
    shaderRoundingModeRTZFloat16 = src.shaderRoundingModeRTZFloat16;
    shaderRoundingModeRTZFloat32 = src.shaderRoundingModeRTZFloat32;
    shaderRoundingModeRTZFloat64 = src.shaderRoundingModeRTZFloat64;

    return *this;
}

safe_VkPhysicalDeviceFloatControlsPropertiesKHR::~safe_VkPhysicalDeviceFloatControlsPropertiesKHR()
{
}

void safe_VkPhysicalDeviceFloatControlsPropertiesKHR::initialize(const VkPhysicalDeviceFloatControlsPropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    separateDenormSettings = in_struct->separateDenormSettings;
    separateRoundingModeSettings = in_struct->separateRoundingModeSettings;
    shaderSignedZeroInfNanPreserveFloat16 = in_struct->shaderSignedZeroInfNanPreserveFloat16;
    shaderSignedZeroInfNanPreserveFloat32 = in_struct->shaderSignedZeroInfNanPreserveFloat32;
    shaderSignedZeroInfNanPreserveFloat64 = in_struct->shaderSignedZeroInfNanPreserveFloat64;
    shaderDenormPreserveFloat16 = in_struct->shaderDenormPreserveFloat16;
    shaderDenormPreserveFloat32 = in_struct->shaderDenormPreserveFloat32;
    shaderDenormPreserveFloat64 = in_struct->shaderDenormPreserveFloat64;
    shaderDenormFlushToZeroFloat16 = in_struct->shaderDenormFlushToZeroFloat16;
    shaderDenormFlushToZeroFloat32 = in_struct->shaderDenormFlushToZeroFloat32;
    shaderDenormFlushToZeroFloat64 = in_struct->shaderDenormFlushToZeroFloat64;
    shaderRoundingModeRTEFloat16 = in_struct->shaderRoundingModeRTEFloat16;
    shaderRoundingModeRTEFloat32 = in_struct->shaderRoundingModeRTEFloat32;
    shaderRoundingModeRTEFloat64 = in_struct->shaderRoundingModeRTEFloat64;
    shaderRoundingModeRTZFloat16 = in_struct->shaderRoundingModeRTZFloat16;
    shaderRoundingModeRTZFloat32 = in_struct->shaderRoundingModeRTZFloat32;
    shaderRoundingModeRTZFloat64 = in_struct->shaderRoundingModeRTZFloat64;
}

void safe_VkPhysicalDeviceFloatControlsPropertiesKHR::initialize(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    separateDenormSettings = src->separateDenormSettings;
    separateRoundingModeSettings = src->separateRoundingModeSettings;
    shaderSignedZeroInfNanPreserveFloat16 = src->shaderSignedZeroInfNanPreserveFloat16;
    shaderSignedZeroInfNanPreserveFloat32 = src->shaderSignedZeroInfNanPreserveFloat32;
    shaderSignedZeroInfNanPreserveFloat64 = src->shaderSignedZeroInfNanPreserveFloat64;
    shaderDenormPreserveFloat16 = src->shaderDenormPreserveFloat16;
    shaderDenormPreserveFloat32 = src->shaderDenormPreserveFloat32;
    shaderDenormPreserveFloat64 = src->shaderDenormPreserveFloat64;
    shaderDenormFlushToZeroFloat16 = src->shaderDenormFlushToZeroFloat16;
    shaderDenormFlushToZeroFloat32 = src->shaderDenormFlushToZeroFloat32;
    shaderDenormFlushToZeroFloat64 = src->shaderDenormFlushToZeroFloat64;
    shaderRoundingModeRTEFloat16 = src->shaderRoundingModeRTEFloat16;
    shaderRoundingModeRTEFloat32 = src->shaderRoundingModeRTEFloat32;
    shaderRoundingModeRTEFloat64 = src->shaderRoundingModeRTEFloat64;
    shaderRoundingModeRTZFloat16 = src->shaderRoundingModeRTZFloat16;
    shaderRoundingModeRTZFloat32 = src->shaderRoundingModeRTZFloat32;
    shaderRoundingModeRTZFloat64 = src->shaderRoundingModeRTZFloat64;
}

safe_VkSubpassDescriptionDepthStencilResolveKHR::safe_VkSubpassDescriptionDepthStencilResolveKHR(const VkSubpassDescriptionDepthStencilResolveKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    depthResolveMode(in_struct->depthResolveMode),
    stencilResolveMode(in_struct->stencilResolveMode)
{
    if (in_struct->pDepthStencilResolveAttachment)
        pDepthStencilResolveAttachment = new safe_VkAttachmentReference2KHR(in_struct->pDepthStencilResolveAttachment);
    else
        pDepthStencilResolveAttachment = NULL;
}

safe_VkSubpassDescriptionDepthStencilResolveKHR::safe_VkSubpassDescriptionDepthStencilResolveKHR()
{}

safe_VkSubpassDescriptionDepthStencilResolveKHR::safe_VkSubpassDescriptionDepthStencilResolveKHR(const safe_VkSubpassDescriptionDepthStencilResolveKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    depthResolveMode = src.depthResolveMode;
    stencilResolveMode = src.stencilResolveMode;
    if (src.pDepthStencilResolveAttachment)
        pDepthStencilResolveAttachment = new safe_VkAttachmentReference2KHR(*src.pDepthStencilResolveAttachment);
    else
        pDepthStencilResolveAttachment = NULL;
}

safe_VkSubpassDescriptionDepthStencilResolveKHR& safe_VkSubpassDescriptionDepthStencilResolveKHR::operator=(const safe_VkSubpassDescriptionDepthStencilResolveKHR& src)
{
    if (&src == this) return *this;

    if (pDepthStencilResolveAttachment)
        delete pDepthStencilResolveAttachment;

    sType = src.sType;
    pNext = src.pNext;
    depthResolveMode = src.depthResolveMode;
    stencilResolveMode = src.stencilResolveMode;
    if (src.pDepthStencilResolveAttachment)
        pDepthStencilResolveAttachment = new safe_VkAttachmentReference2KHR(*src.pDepthStencilResolveAttachment);
    else
        pDepthStencilResolveAttachment = NULL;

    return *this;
}

safe_VkSubpassDescriptionDepthStencilResolveKHR::~safe_VkSubpassDescriptionDepthStencilResolveKHR()
{
    if (pDepthStencilResolveAttachment)
        delete pDepthStencilResolveAttachment;
}

void safe_VkSubpassDescriptionDepthStencilResolveKHR::initialize(const VkSubpassDescriptionDepthStencilResolveKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    depthResolveMode = in_struct->depthResolveMode;
    stencilResolveMode = in_struct->stencilResolveMode;
    if (in_struct->pDepthStencilResolveAttachment)
        pDepthStencilResolveAttachment = new safe_VkAttachmentReference2KHR(in_struct->pDepthStencilResolveAttachment);
    else
        pDepthStencilResolveAttachment = NULL;
}

void safe_VkSubpassDescriptionDepthStencilResolveKHR::initialize(const safe_VkSubpassDescriptionDepthStencilResolveKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    depthResolveMode = src->depthResolveMode;
    stencilResolveMode = src->stencilResolveMode;
    if (src->pDepthStencilResolveAttachment)
        pDepthStencilResolveAttachment = new safe_VkAttachmentReference2KHR(*src->pDepthStencilResolveAttachment);
    else
        pDepthStencilResolveAttachment = NULL;
}

safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR(const VkPhysicalDeviceDepthStencilResolvePropertiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    supportedDepthResolveModes(in_struct->supportedDepthResolveModes),
    supportedStencilResolveModes(in_struct->supportedStencilResolveModes),
    independentResolveNone(in_struct->independentResolveNone),
    independentResolve(in_struct->independentResolve)
{
}

safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR()
{}

safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    supportedDepthResolveModes = src.supportedDepthResolveModes;
    supportedStencilResolveModes = src.supportedStencilResolveModes;
    independentResolveNone = src.independentResolveNone;
    independentResolve = src.independentResolve;
}

safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::operator=(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    supportedDepthResolveModes = src.supportedDepthResolveModes;
    supportedStencilResolveModes = src.supportedStencilResolveModes;
    independentResolveNone = src.independentResolveNone;
    independentResolve = src.independentResolve;

    return *this;
}

safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::~safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR()
{
}

void safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::initialize(const VkPhysicalDeviceDepthStencilResolvePropertiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    supportedDepthResolveModes = in_struct->supportedDepthResolveModes;
    supportedStencilResolveModes = in_struct->supportedStencilResolveModes;
    independentResolveNone = in_struct->independentResolveNone;
    independentResolve = in_struct->independentResolve;
}

void safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR::initialize(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    supportedDepthResolveModes = src->supportedDepthResolveModes;
    supportedStencilResolveModes = src->supportedStencilResolveModes;
    independentResolveNone = src->independentResolveNone;
    independentResolve = src->independentResolve;
}

safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR(const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    vulkanMemoryModel(in_struct->vulkanMemoryModel),
    vulkanMemoryModelDeviceScope(in_struct->vulkanMemoryModelDeviceScope),
    vulkanMemoryModelAvailabilityVisibilityChains(in_struct->vulkanMemoryModelAvailabilityVisibilityChains)
{
}

safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR()
{}

safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    vulkanMemoryModel = src.vulkanMemoryModel;
    vulkanMemoryModelDeviceScope = src.vulkanMemoryModelDeviceScope;
    vulkanMemoryModelAvailabilityVisibilityChains = src.vulkanMemoryModelAvailabilityVisibilityChains;
}

safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::operator=(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    vulkanMemoryModel = src.vulkanMemoryModel;
    vulkanMemoryModelDeviceScope = src.vulkanMemoryModelDeviceScope;
    vulkanMemoryModelAvailabilityVisibilityChains = src.vulkanMemoryModelAvailabilityVisibilityChains;

    return *this;
}

safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::~safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR()
{
}

void safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::initialize(const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    vulkanMemoryModel = in_struct->vulkanMemoryModel;
    vulkanMemoryModelDeviceScope = in_struct->vulkanMemoryModelDeviceScope;
    vulkanMemoryModelAvailabilityVisibilityChains = in_struct->vulkanMemoryModelAvailabilityVisibilityChains;
}

void safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR::initialize(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    vulkanMemoryModel = src->vulkanMemoryModel;
    vulkanMemoryModelDeviceScope = src->vulkanMemoryModelDeviceScope;
    vulkanMemoryModelAvailabilityVisibilityChains = src->vulkanMemoryModelAvailabilityVisibilityChains;
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(const VkSurfaceProtectedCapabilitiesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    supportsProtected(in_struct->supportsProtected)
{
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR()
{}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(const safe_VkSurfaceProtectedCapabilitiesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    supportsProtected = src.supportsProtected;
}

safe_VkSurfaceProtectedCapabilitiesKHR& safe_VkSurfaceProtectedCapabilitiesKHR::operator=(const safe_VkSurfaceProtectedCapabilitiesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    supportsProtected = src.supportsProtected;

    return *this;
}

safe_VkSurfaceProtectedCapabilitiesKHR::~safe_VkSurfaceProtectedCapabilitiesKHR()
{
}

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const VkSurfaceProtectedCapabilitiesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    supportsProtected = in_struct->supportsProtected;
}

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const safe_VkSurfaceProtectedCapabilitiesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    supportsProtected = src->supportsProtected;
}

safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    uniformBufferStandardLayout(in_struct->uniformBufferStandardLayout)
{
}

safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR()
{}

safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& src)
{
    sType = src.sType;
    pNext = src.pNext;
    uniformBufferStandardLayout = src.uniformBufferStandardLayout;
}

safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::operator=(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    uniformBufferStandardLayout = src.uniformBufferStandardLayout;

    return *this;
}

safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::~safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR()
{
}

void safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::initialize(const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    uniformBufferStandardLayout = in_struct->uniformBufferStandardLayout;
}

void safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR::initialize(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* src)
{
    sType = src->sType;
    pNext = src->pNext;
    uniformBufferStandardLayout = src->uniformBufferStandardLayout;
}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT(const VkDebugReportCallbackCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pfnCallback(in_struct->pfnCallback),
    pUserData(in_struct->pUserData)
{
}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT()
{}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT(const safe_VkDebugReportCallbackCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pfnCallback = src.pfnCallback;
    pUserData = src.pUserData;
}

safe_VkDebugReportCallbackCreateInfoEXT& safe_VkDebugReportCallbackCreateInfoEXT::operator=(const safe_VkDebugReportCallbackCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pfnCallback = src.pfnCallback;
    pUserData = src.pUserData;

    return *this;
}

safe_VkDebugReportCallbackCreateInfoEXT::~safe_VkDebugReportCallbackCreateInfoEXT()
{
}

void safe_VkDebugReportCallbackCreateInfoEXT::initialize(const VkDebugReportCallbackCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pfnCallback = in_struct->pfnCallback;
    pUserData = in_struct->pUserData;
}

void safe_VkDebugReportCallbackCreateInfoEXT::initialize(const safe_VkDebugReportCallbackCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pfnCallback = src->pfnCallback;
    pUserData = src->pUserData;
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD(const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    rasterizationOrder(in_struct->rasterizationOrder)
{
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD()
{}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD(const safe_VkPipelineRasterizationStateRasterizationOrderAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    rasterizationOrder = src.rasterizationOrder;
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD& safe_VkPipelineRasterizationStateRasterizationOrderAMD::operator=(const safe_VkPipelineRasterizationStateRasterizationOrderAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    rasterizationOrder = src.rasterizationOrder;

    return *this;
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::~safe_VkPipelineRasterizationStateRasterizationOrderAMD()
{
}

void safe_VkPipelineRasterizationStateRasterizationOrderAMD::initialize(const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    rasterizationOrder = in_struct->rasterizationOrder;
}

void safe_VkPipelineRasterizationStateRasterizationOrderAMD::initialize(const safe_VkPipelineRasterizationStateRasterizationOrderAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    rasterizationOrder = src->rasterizationOrder;
}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT(const VkDebugMarkerObjectNameInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectType(in_struct->objectType),
    object(in_struct->object),
    pObjectName(in_struct->pObjectName)
{
}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT()
{}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT(const safe_VkDebugMarkerObjectNameInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    object = src.object;
    pObjectName = src.pObjectName;
}

safe_VkDebugMarkerObjectNameInfoEXT& safe_VkDebugMarkerObjectNameInfoEXT::operator=(const safe_VkDebugMarkerObjectNameInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    object = src.object;
    pObjectName = src.pObjectName;

    return *this;
}

safe_VkDebugMarkerObjectNameInfoEXT::~safe_VkDebugMarkerObjectNameInfoEXT()
{
}

void safe_VkDebugMarkerObjectNameInfoEXT::initialize(const VkDebugMarkerObjectNameInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectType = in_struct->objectType;
    object = in_struct->object;
    pObjectName = in_struct->pObjectName;
}

void safe_VkDebugMarkerObjectNameInfoEXT::initialize(const safe_VkDebugMarkerObjectNameInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectType = src->objectType;
    object = src->object;
    pObjectName = src->pObjectName;
}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT(const VkDebugMarkerObjectTagInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectType(in_struct->objectType),
    object(in_struct->object),
    tagName(in_struct->tagName),
    tagSize(in_struct->tagSize),
    pTag(in_struct->pTag)
{
}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT()
{}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT(const safe_VkDebugMarkerObjectTagInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    object = src.object;
    tagName = src.tagName;
    tagSize = src.tagSize;
    pTag = src.pTag;
}

safe_VkDebugMarkerObjectTagInfoEXT& safe_VkDebugMarkerObjectTagInfoEXT::operator=(const safe_VkDebugMarkerObjectTagInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    object = src.object;
    tagName = src.tagName;
    tagSize = src.tagSize;
    pTag = src.pTag;

    return *this;
}

safe_VkDebugMarkerObjectTagInfoEXT::~safe_VkDebugMarkerObjectTagInfoEXT()
{
}

void safe_VkDebugMarkerObjectTagInfoEXT::initialize(const VkDebugMarkerObjectTagInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectType = in_struct->objectType;
    object = in_struct->object;
    tagName = in_struct->tagName;
    tagSize = in_struct->tagSize;
    pTag = in_struct->pTag;
}

void safe_VkDebugMarkerObjectTagInfoEXT::initialize(const safe_VkDebugMarkerObjectTagInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectType = src->objectType;
    object = src->object;
    tagName = src->tagName;
    tagSize = src->tagSize;
    pTag = src->pTag;
}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT(const VkDebugMarkerMarkerInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pMarkerName(in_struct->pMarkerName)
{
    for (uint32_t i=0; i<4; ++i) {
        color[i] = in_struct->color[i];
    }
}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT()
{}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT(const safe_VkDebugMarkerMarkerInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pMarkerName = src.pMarkerName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src.color[i];
    }
}

safe_VkDebugMarkerMarkerInfoEXT& safe_VkDebugMarkerMarkerInfoEXT::operator=(const safe_VkDebugMarkerMarkerInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pMarkerName = src.pMarkerName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src.color[i];
    }

    return *this;
}

safe_VkDebugMarkerMarkerInfoEXT::~safe_VkDebugMarkerMarkerInfoEXT()
{
}

void safe_VkDebugMarkerMarkerInfoEXT::initialize(const VkDebugMarkerMarkerInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pMarkerName = in_struct->pMarkerName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = in_struct->color[i];
    }
}

void safe_VkDebugMarkerMarkerInfoEXT::initialize(const safe_VkDebugMarkerMarkerInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pMarkerName = src->pMarkerName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src->color[i];
    }
}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV(const VkDedicatedAllocationImageCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    dedicatedAllocation(in_struct->dedicatedAllocation)
{
}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV()
{}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV(const safe_VkDedicatedAllocationImageCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocation = src.dedicatedAllocation;
}

safe_VkDedicatedAllocationImageCreateInfoNV& safe_VkDedicatedAllocationImageCreateInfoNV::operator=(const safe_VkDedicatedAllocationImageCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocation = src.dedicatedAllocation;

    return *this;
}

safe_VkDedicatedAllocationImageCreateInfoNV::~safe_VkDedicatedAllocationImageCreateInfoNV()
{
}

void safe_VkDedicatedAllocationImageCreateInfoNV::initialize(const VkDedicatedAllocationImageCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    dedicatedAllocation = in_struct->dedicatedAllocation;
}

void safe_VkDedicatedAllocationImageCreateInfoNV::initialize(const safe_VkDedicatedAllocationImageCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    dedicatedAllocation = src->dedicatedAllocation;
}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV(const VkDedicatedAllocationBufferCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    dedicatedAllocation(in_struct->dedicatedAllocation)
{
}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV()
{}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV(const safe_VkDedicatedAllocationBufferCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocation = src.dedicatedAllocation;
}

safe_VkDedicatedAllocationBufferCreateInfoNV& safe_VkDedicatedAllocationBufferCreateInfoNV::operator=(const safe_VkDedicatedAllocationBufferCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocation = src.dedicatedAllocation;

    return *this;
}

safe_VkDedicatedAllocationBufferCreateInfoNV::~safe_VkDedicatedAllocationBufferCreateInfoNV()
{
}

void safe_VkDedicatedAllocationBufferCreateInfoNV::initialize(const VkDedicatedAllocationBufferCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    dedicatedAllocation = in_struct->dedicatedAllocation;
}

void safe_VkDedicatedAllocationBufferCreateInfoNV::initialize(const safe_VkDedicatedAllocationBufferCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    dedicatedAllocation = src->dedicatedAllocation;
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV(const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    image(in_struct->image),
    buffer(in_struct->buffer)
{
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV()
{}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV(const safe_VkDedicatedAllocationMemoryAllocateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    buffer = src.buffer;
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV& safe_VkDedicatedAllocationMemoryAllocateInfoNV::operator=(const safe_VkDedicatedAllocationMemoryAllocateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    image = src.image;
    buffer = src.buffer;

    return *this;
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::~safe_VkDedicatedAllocationMemoryAllocateInfoNV()
{
}

void safe_VkDedicatedAllocationMemoryAllocateInfoNV::initialize(const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    image = in_struct->image;
    buffer = in_struct->buffer;
}

void safe_VkDedicatedAllocationMemoryAllocateInfoNV::initialize(const safe_VkDedicatedAllocationMemoryAllocateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    image = src->image;
    buffer = src->buffer;
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    transformFeedback(in_struct->transformFeedback),
    geometryStreams(in_struct->geometryStreams)
{
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT()
{}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    transformFeedback = src.transformFeedback;
    geometryStreams = src.geometryStreams;
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::operator=(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    transformFeedback = src.transformFeedback;
    geometryStreams = src.geometryStreams;

    return *this;
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::~safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT()
{
}

void safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::initialize(const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    transformFeedback = in_struct->transformFeedback;
    geometryStreams = in_struct->geometryStreams;
}

void safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::initialize(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    transformFeedback = src->transformFeedback;
    geometryStreams = src->geometryStreams;
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxTransformFeedbackStreams(in_struct->maxTransformFeedbackStreams),
    maxTransformFeedbackBuffers(in_struct->maxTransformFeedbackBuffers),
    maxTransformFeedbackBufferSize(in_struct->maxTransformFeedbackBufferSize),
    maxTransformFeedbackStreamDataSize(in_struct->maxTransformFeedbackStreamDataSize),
    maxTransformFeedbackBufferDataSize(in_struct->maxTransformFeedbackBufferDataSize),
    maxTransformFeedbackBufferDataStride(in_struct->maxTransformFeedbackBufferDataStride),
    transformFeedbackQueries(in_struct->transformFeedbackQueries),
    transformFeedbackStreamsLinesTriangles(in_struct->transformFeedbackStreamsLinesTriangles),
    transformFeedbackRasterizationStreamSelect(in_struct->transformFeedbackRasterizationStreamSelect),
    transformFeedbackDraw(in_struct->transformFeedbackDraw)
{
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT()
{}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxTransformFeedbackStreams = src.maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = src.maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = src.maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = src.maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = src.maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = src.maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = src.transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = src.transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = src.transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = src.transformFeedbackDraw;
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::operator=(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxTransformFeedbackStreams = src.maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = src.maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = src.maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = src.maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = src.maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = src.maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = src.transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = src.transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = src.transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = src.transformFeedbackDraw;

    return *this;
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::~safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT()
{
}

void safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::initialize(const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxTransformFeedbackStreams = in_struct->maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = in_struct->maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = in_struct->maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = in_struct->maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = in_struct->maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = in_struct->maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = in_struct->transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = in_struct->transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = in_struct->transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = in_struct->transformFeedbackDraw;
}

void safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::initialize(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxTransformFeedbackStreams = src->maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = src->maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = src->maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = src->maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = src->maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = src->maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = src->transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = src->transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = src->transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = src->transformFeedbackDraw;
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT(const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    rasterizationStream(in_struct->rasterizationStream)
{
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT()
{}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    rasterizationStream = src.rasterizationStream;
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT& safe_VkPipelineRasterizationStateStreamCreateInfoEXT::operator=(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    rasterizationStream = src.rasterizationStream;

    return *this;
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::~safe_VkPipelineRasterizationStateStreamCreateInfoEXT()
{
}

void safe_VkPipelineRasterizationStateStreamCreateInfoEXT::initialize(const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    rasterizationStream = in_struct->rasterizationStream;
}

void safe_VkPipelineRasterizationStateStreamCreateInfoEXT::initialize(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    rasterizationStream = src->rasterizationStream;
}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX(const VkImageViewHandleInfoNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    imageView(in_struct->imageView),
    descriptorType(in_struct->descriptorType),
    sampler(in_struct->sampler)
{
}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX()
{}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX(const safe_VkImageViewHandleInfoNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    imageView = src.imageView;
    descriptorType = src.descriptorType;
    sampler = src.sampler;
}

safe_VkImageViewHandleInfoNVX& safe_VkImageViewHandleInfoNVX::operator=(const safe_VkImageViewHandleInfoNVX& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    imageView = src.imageView;
    descriptorType = src.descriptorType;
    sampler = src.sampler;

    return *this;
}

safe_VkImageViewHandleInfoNVX::~safe_VkImageViewHandleInfoNVX()
{
}

void safe_VkImageViewHandleInfoNVX::initialize(const VkImageViewHandleInfoNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    imageView = in_struct->imageView;
    descriptorType = in_struct->descriptorType;
    sampler = in_struct->sampler;
}

void safe_VkImageViewHandleInfoNVX::initialize(const safe_VkImageViewHandleInfoNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    imageView = src->imageView;
    descriptorType = src->descriptorType;
    sampler = src->sampler;
}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD(const VkTextureLODGatherFormatPropertiesAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    supportsTextureGatherLODBiasAMD(in_struct->supportsTextureGatherLODBiasAMD)
{
}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD()
{}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD(const safe_VkTextureLODGatherFormatPropertiesAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    supportsTextureGatherLODBiasAMD = src.supportsTextureGatherLODBiasAMD;
}

safe_VkTextureLODGatherFormatPropertiesAMD& safe_VkTextureLODGatherFormatPropertiesAMD::operator=(const safe_VkTextureLODGatherFormatPropertiesAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    supportsTextureGatherLODBiasAMD = src.supportsTextureGatherLODBiasAMD;

    return *this;
}

safe_VkTextureLODGatherFormatPropertiesAMD::~safe_VkTextureLODGatherFormatPropertiesAMD()
{
}

void safe_VkTextureLODGatherFormatPropertiesAMD::initialize(const VkTextureLODGatherFormatPropertiesAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    supportsTextureGatherLODBiasAMD = in_struct->supportsTextureGatherLODBiasAMD;
}

void safe_VkTextureLODGatherFormatPropertiesAMD::initialize(const safe_VkTextureLODGatherFormatPropertiesAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    supportsTextureGatherLODBiasAMD = src->supportsTextureGatherLODBiasAMD;
}
#ifdef VK_USE_PLATFORM_GGP


safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP(const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    streamDescriptor(in_struct->streamDescriptor)
{
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP()
{}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP(const safe_VkStreamDescriptorSurfaceCreateInfoGGP& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    streamDescriptor = src.streamDescriptor;
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP& safe_VkStreamDescriptorSurfaceCreateInfoGGP::operator=(const safe_VkStreamDescriptorSurfaceCreateInfoGGP& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    streamDescriptor = src.streamDescriptor;

    return *this;
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::~safe_VkStreamDescriptorSurfaceCreateInfoGGP()
{
}

void safe_VkStreamDescriptorSurfaceCreateInfoGGP::initialize(const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    streamDescriptor = in_struct->streamDescriptor;
}

void safe_VkStreamDescriptorSurfaceCreateInfoGGP::initialize(const safe_VkStreamDescriptorSurfaceCreateInfoGGP* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    streamDescriptor = src->streamDescriptor;
}
#endif // VK_USE_PLATFORM_GGP


safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    cornerSampledImage(in_struct->cornerSampledImage)
{
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV()
{}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    cornerSampledImage = src.cornerSampledImage;
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::operator=(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    cornerSampledImage = src.cornerSampledImage;

    return *this;
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::~safe_VkPhysicalDeviceCornerSampledImageFeaturesNV()
{
}

void safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::initialize(const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    cornerSampledImage = in_struct->cornerSampledImage;
}

void safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::initialize(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    cornerSampledImage = src->cornerSampledImage;
}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV(const VkExternalMemoryImageCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV()
{}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV(const safe_VkExternalMemoryImageCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExternalMemoryImageCreateInfoNV& safe_VkExternalMemoryImageCreateInfoNV::operator=(const safe_VkExternalMemoryImageCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExternalMemoryImageCreateInfoNV::~safe_VkExternalMemoryImageCreateInfoNV()
{
}

void safe_VkExternalMemoryImageCreateInfoNV::initialize(const VkExternalMemoryImageCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExternalMemoryImageCreateInfoNV::initialize(const safe_VkExternalMemoryImageCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV(const VkExportMemoryAllocateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleTypes(in_struct->handleTypes)
{
}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV()
{}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV(const safe_VkExportMemoryAllocateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;
}

safe_VkExportMemoryAllocateInfoNV& safe_VkExportMemoryAllocateInfoNV::operator=(const safe_VkExportMemoryAllocateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleTypes = src.handleTypes;

    return *this;
}

safe_VkExportMemoryAllocateInfoNV::~safe_VkExportMemoryAllocateInfoNV()
{
}

void safe_VkExportMemoryAllocateInfoNV::initialize(const VkExportMemoryAllocateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleTypes = in_struct->handleTypes;
}

void safe_VkExportMemoryAllocateInfoNV::initialize(const safe_VkExportMemoryAllocateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleTypes = src->handleTypes;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV(const VkImportMemoryWin32HandleInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType),
    handle(in_struct->handle)
{
}

safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV()
{}

safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV(const safe_VkImportMemoryWin32HandleInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    handle = src.handle;
}

safe_VkImportMemoryWin32HandleInfoNV& safe_VkImportMemoryWin32HandleInfoNV::operator=(const safe_VkImportMemoryWin32HandleInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    handle = src.handle;

    return *this;
}

safe_VkImportMemoryWin32HandleInfoNV::~safe_VkImportMemoryWin32HandleInfoNV()
{
}

void safe_VkImportMemoryWin32HandleInfoNV::initialize(const VkImportMemoryWin32HandleInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
}

void safe_VkImportMemoryWin32HandleInfoNV::initialize(const safe_VkImportMemoryWin32HandleInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
    handle = src->handle;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV(const VkExportMemoryWin32HandleInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pAttributes(nullptr),
    dwAccess(in_struct->dwAccess)
{
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV() :
    pAttributes(nullptr)
{}

safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV(const safe_VkExportMemoryWin32HandleInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoNV& safe_VkExportMemoryWin32HandleInfoNV::operator=(const safe_VkExportMemoryWin32HandleInfoNV& src)
{
    if (&src == this) return *this;

    if (pAttributes)
        delete pAttributes;

    sType = src.sType;
    pNext = src.pNext;
    pAttributes = nullptr;
    dwAccess = src.dwAccess;
    if (src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src.pAttributes);
    }

    return *this;
}

safe_VkExportMemoryWin32HandleInfoNV::~safe_VkExportMemoryWin32HandleInfoNV()
{
    if (pAttributes)
        delete pAttributes;
}

void safe_VkExportMemoryWin32HandleInfoNV::initialize(const VkExportMemoryWin32HandleInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportMemoryWin32HandleInfoNV::initialize(const safe_VkExportMemoryWin32HandleInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pAttributes = nullptr;
    dwAccess = src->dwAccess;
    if (src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*src->pAttributes);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV(const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    acquireCount(in_struct->acquireCount),
    pAcquireSyncs(nullptr),
    pAcquireKeys(nullptr),
    pAcquireTimeoutMilliseconds(nullptr),
    releaseCount(in_struct->releaseCount),
    pReleaseSyncs(nullptr),
    pReleaseKeys(nullptr)
{
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }
    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)in_struct->pAcquireKeys, sizeof(uint64_t)*in_struct->acquireCount);
    }
    if (in_struct->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireTimeoutMilliseconds, (void *)in_struct->pAcquireTimeoutMilliseconds, sizeof(uint32_t)*in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }
    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)in_struct->pReleaseKeys, sizeof(uint64_t)*in_struct->releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV() :
    pAcquireSyncs(nullptr),
    pAcquireKeys(nullptr),
    pAcquireTimeoutMilliseconds(nullptr),
    pReleaseSyncs(nullptr),
    pReleaseKeys(nullptr)
{}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    acquireCount = src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src.pAcquireSyncs[i];
        }
    }
    if (src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[src.acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src.pAcquireKeys, sizeof(uint64_t)*src.acquireCount);
    }
    if (src.pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[src.acquireCount];
        memcpy ((void *)pAcquireTimeoutMilliseconds, (void *)src.pAcquireTimeoutMilliseconds, sizeof(uint32_t)*src.acquireCount);
    }
    if (releaseCount && src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src.pReleaseSyncs[i];
        }
    }
    if (src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[src.releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src.pReleaseKeys, sizeof(uint64_t)*src.releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV& safe_VkWin32KeyedMutexAcquireReleaseInfoNV::operator=(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& src)
{
    if (&src == this) return *this;

    if (pAcquireSyncs)
        delete[] pAcquireSyncs;
    if (pAcquireKeys)
        delete[] pAcquireKeys;
    if (pAcquireTimeoutMilliseconds)
        delete[] pAcquireTimeoutMilliseconds;
    if (pReleaseSyncs)
        delete[] pReleaseSyncs;
    if (pReleaseKeys)
        delete[] pReleaseKeys;

    sType = src.sType;
    pNext = src.pNext;
    acquireCount = src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src.pAcquireSyncs[i];
        }
    }
    if (src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[src.acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src.pAcquireKeys, sizeof(uint64_t)*src.acquireCount);
    }
    if (src.pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[src.acquireCount];
        memcpy ((void *)pAcquireTimeoutMilliseconds, (void *)src.pAcquireTimeoutMilliseconds, sizeof(uint32_t)*src.acquireCount);
    }
    if (releaseCount && src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src.pReleaseSyncs[i];
        }
    }
    if (src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[src.releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src.pReleaseKeys, sizeof(uint64_t)*src.releaseCount);
    }

    return *this;
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::~safe_VkWin32KeyedMutexAcquireReleaseInfoNV()
{
    if (pAcquireSyncs)
        delete[] pAcquireSyncs;
    if (pAcquireKeys)
        delete[] pAcquireKeys;
    if (pAcquireTimeoutMilliseconds)
        delete[] pAcquireTimeoutMilliseconds;
    if (pReleaseSyncs)
        delete[] pReleaseSyncs;
    if (pReleaseKeys)
        delete[] pReleaseKeys;
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoNV::initialize(const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    acquireCount = in_struct->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = in_struct->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }
    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)in_struct->pAcquireKeys, sizeof(uint64_t)*in_struct->acquireCount);
    }
    if (in_struct->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[in_struct->acquireCount];
        memcpy ((void *)pAcquireTimeoutMilliseconds, (void *)in_struct->pAcquireTimeoutMilliseconds, sizeof(uint32_t)*in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }
    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)in_struct->pReleaseKeys, sizeof(uint64_t)*in_struct->releaseCount);
    }
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoNV::initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    acquireCount = src->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = src->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    if (acquireCount && src->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i=0; i<acquireCount; ++i) {
            pAcquireSyncs[i] = src->pAcquireSyncs[i];
        }
    }
    if (src->pAcquireKeys) {
        pAcquireKeys = new uint64_t[src->acquireCount];
        memcpy ((void *)pAcquireKeys, (void *)src->pAcquireKeys, sizeof(uint64_t)*src->acquireCount);
    }
    if (src->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[src->acquireCount];
        memcpy ((void *)pAcquireTimeoutMilliseconds, (void *)src->pAcquireTimeoutMilliseconds, sizeof(uint32_t)*src->acquireCount);
    }
    if (releaseCount && src->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i=0; i<releaseCount; ++i) {
            pReleaseSyncs[i] = src->pReleaseSyncs[i];
        }
    }
    if (src->pReleaseKeys) {
        pReleaseKeys = new uint64_t[src->releaseCount];
        memcpy ((void *)pReleaseKeys, (void *)src->pReleaseKeys, sizeof(uint64_t)*src->releaseCount);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR


safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT(const VkValidationFlagsEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    disabledValidationCheckCount(in_struct->disabledValidationCheckCount),
    pDisabledValidationChecks(nullptr)
{
    if (in_struct->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[in_struct->disabledValidationCheckCount];
        memcpy ((void *)pDisabledValidationChecks, (void *)in_struct->pDisabledValidationChecks, sizeof(VkValidationCheckEXT)*in_struct->disabledValidationCheckCount);
    }
}

safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT() :
    pDisabledValidationChecks(nullptr)
{}

safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT(const safe_VkValidationFlagsEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    disabledValidationCheckCount = src.disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    if (src.pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[src.disabledValidationCheckCount];
        memcpy ((void *)pDisabledValidationChecks, (void *)src.pDisabledValidationChecks, sizeof(VkValidationCheckEXT)*src.disabledValidationCheckCount);
    }
}

safe_VkValidationFlagsEXT& safe_VkValidationFlagsEXT::operator=(const safe_VkValidationFlagsEXT& src)
{
    if (&src == this) return *this;

    if (pDisabledValidationChecks)
        delete[] pDisabledValidationChecks;

    sType = src.sType;
    pNext = src.pNext;
    disabledValidationCheckCount = src.disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    if (src.pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[src.disabledValidationCheckCount];
        memcpy ((void *)pDisabledValidationChecks, (void *)src.pDisabledValidationChecks, sizeof(VkValidationCheckEXT)*src.disabledValidationCheckCount);
    }

    return *this;
}

safe_VkValidationFlagsEXT::~safe_VkValidationFlagsEXT()
{
    if (pDisabledValidationChecks)
        delete[] pDisabledValidationChecks;
}

void safe_VkValidationFlagsEXT::initialize(const VkValidationFlagsEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    disabledValidationCheckCount = in_struct->disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    if (in_struct->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[in_struct->disabledValidationCheckCount];
        memcpy ((void *)pDisabledValidationChecks, (void *)in_struct->pDisabledValidationChecks, sizeof(VkValidationCheckEXT)*in_struct->disabledValidationCheckCount);
    }
}

void safe_VkValidationFlagsEXT::initialize(const safe_VkValidationFlagsEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    disabledValidationCheckCount = src->disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    if (src->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[src->disabledValidationCheckCount];
        memcpy ((void *)pDisabledValidationChecks, (void *)src->pDisabledValidationChecks, sizeof(VkValidationCheckEXT)*src->disabledValidationCheckCount);
    }
}
#ifdef VK_USE_PLATFORM_VI_NN


safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN(const VkViSurfaceCreateInfoNN* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    window(in_struct->window)
{
}

safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN()
{}

safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN(const safe_VkViSurfaceCreateInfoNN& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    window = src.window;
}

safe_VkViSurfaceCreateInfoNN& safe_VkViSurfaceCreateInfoNN::operator=(const safe_VkViSurfaceCreateInfoNN& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    window = src.window;

    return *this;
}

safe_VkViSurfaceCreateInfoNN::~safe_VkViSurfaceCreateInfoNN()
{
}

void safe_VkViSurfaceCreateInfoNN::initialize(const VkViSurfaceCreateInfoNN* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    window = in_struct->window;
}

void safe_VkViSurfaceCreateInfoNN::initialize(const safe_VkViSurfaceCreateInfoNN* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    window = src->window;
}
#endif // VK_USE_PLATFORM_VI_NN


safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT(const VkImageViewASTCDecodeModeEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    decodeMode(in_struct->decodeMode)
{
}

safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT()
{}

safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT(const safe_VkImageViewASTCDecodeModeEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    decodeMode = src.decodeMode;
}

safe_VkImageViewASTCDecodeModeEXT& safe_VkImageViewASTCDecodeModeEXT::operator=(const safe_VkImageViewASTCDecodeModeEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    decodeMode = src.decodeMode;

    return *this;
}

safe_VkImageViewASTCDecodeModeEXT::~safe_VkImageViewASTCDecodeModeEXT()
{
}

void safe_VkImageViewASTCDecodeModeEXT::initialize(const VkImageViewASTCDecodeModeEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    decodeMode = in_struct->decodeMode;
}

void safe_VkImageViewASTCDecodeModeEXT::initialize(const safe_VkImageViewASTCDecodeModeEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    decodeMode = src->decodeMode;
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    decodeModeSharedExponent(in_struct->decodeModeSharedExponent)
{
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT()
{}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    decodeModeSharedExponent = src.decodeModeSharedExponent;
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::operator=(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    decodeModeSharedExponent = src.decodeModeSharedExponent;

    return *this;
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::~safe_VkPhysicalDeviceASTCDecodeFeaturesEXT()
{
}

void safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::initialize(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    decodeModeSharedExponent = in_struct->decodeModeSharedExponent;
}

void safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::initialize(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    decodeModeSharedExponent = src->decodeModeSharedExponent;
}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT(const VkConditionalRenderingBeginInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    buffer(in_struct->buffer),
    offset(in_struct->offset),
    flags(in_struct->flags)
{
}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT()
{}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT(const safe_VkConditionalRenderingBeginInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
    offset = src.offset;
    flags = src.flags;
}

safe_VkConditionalRenderingBeginInfoEXT& safe_VkConditionalRenderingBeginInfoEXT::operator=(const safe_VkConditionalRenderingBeginInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
    offset = src.offset;
    flags = src.flags;

    return *this;
}

safe_VkConditionalRenderingBeginInfoEXT::~safe_VkConditionalRenderingBeginInfoEXT()
{
}

void safe_VkConditionalRenderingBeginInfoEXT::initialize(const VkConditionalRenderingBeginInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    buffer = in_struct->buffer;
    offset = in_struct->offset;
    flags = in_struct->flags;
}

void safe_VkConditionalRenderingBeginInfoEXT::initialize(const safe_VkConditionalRenderingBeginInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    buffer = src->buffer;
    offset = src->offset;
    flags = src->flags;
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    conditionalRendering(in_struct->conditionalRendering),
    inheritedConditionalRendering(in_struct->inheritedConditionalRendering)
{
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT()
{}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    conditionalRendering = src.conditionalRendering;
    inheritedConditionalRendering = src.inheritedConditionalRendering;
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::operator=(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    conditionalRendering = src.conditionalRendering;
    inheritedConditionalRendering = src.inheritedConditionalRendering;

    return *this;
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::~safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT()
{
}

void safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::initialize(const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    conditionalRendering = in_struct->conditionalRendering;
    inheritedConditionalRendering = in_struct->inheritedConditionalRendering;
}

void safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::initialize(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    conditionalRendering = src->conditionalRendering;
    inheritedConditionalRendering = src->inheritedConditionalRendering;
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    conditionalRenderingEnable(in_struct->conditionalRenderingEnable)
{
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT()
{}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    conditionalRenderingEnable = src.conditionalRenderingEnable;
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::operator=(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    conditionalRenderingEnable = src.conditionalRenderingEnable;

    return *this;
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::~safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT()
{
}

void safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::initialize(const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    conditionalRenderingEnable = in_struct->conditionalRenderingEnable;
}

void safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::initialize(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    conditionalRenderingEnable = src->conditionalRenderingEnable;
}

safe_VkDeviceGeneratedCommandsFeaturesNVX::safe_VkDeviceGeneratedCommandsFeaturesNVX(const VkDeviceGeneratedCommandsFeaturesNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    computeBindingPointSupport(in_struct->computeBindingPointSupport)
{
}

safe_VkDeviceGeneratedCommandsFeaturesNVX::safe_VkDeviceGeneratedCommandsFeaturesNVX()
{}

safe_VkDeviceGeneratedCommandsFeaturesNVX::safe_VkDeviceGeneratedCommandsFeaturesNVX(const safe_VkDeviceGeneratedCommandsFeaturesNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    computeBindingPointSupport = src.computeBindingPointSupport;
}

safe_VkDeviceGeneratedCommandsFeaturesNVX& safe_VkDeviceGeneratedCommandsFeaturesNVX::operator=(const safe_VkDeviceGeneratedCommandsFeaturesNVX& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    computeBindingPointSupport = src.computeBindingPointSupport;

    return *this;
}

safe_VkDeviceGeneratedCommandsFeaturesNVX::~safe_VkDeviceGeneratedCommandsFeaturesNVX()
{
}

void safe_VkDeviceGeneratedCommandsFeaturesNVX::initialize(const VkDeviceGeneratedCommandsFeaturesNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    computeBindingPointSupport = in_struct->computeBindingPointSupport;
}

void safe_VkDeviceGeneratedCommandsFeaturesNVX::initialize(const safe_VkDeviceGeneratedCommandsFeaturesNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    computeBindingPointSupport = src->computeBindingPointSupport;
}

safe_VkDeviceGeneratedCommandsLimitsNVX::safe_VkDeviceGeneratedCommandsLimitsNVX(const VkDeviceGeneratedCommandsLimitsNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxIndirectCommandsLayoutTokenCount(in_struct->maxIndirectCommandsLayoutTokenCount),
    maxObjectEntryCounts(in_struct->maxObjectEntryCounts),
    minSequenceCountBufferOffsetAlignment(in_struct->minSequenceCountBufferOffsetAlignment),
    minSequenceIndexBufferOffsetAlignment(in_struct->minSequenceIndexBufferOffsetAlignment),
    minCommandsTokenBufferOffsetAlignment(in_struct->minCommandsTokenBufferOffsetAlignment)
{
}

safe_VkDeviceGeneratedCommandsLimitsNVX::safe_VkDeviceGeneratedCommandsLimitsNVX()
{}

safe_VkDeviceGeneratedCommandsLimitsNVX::safe_VkDeviceGeneratedCommandsLimitsNVX(const safe_VkDeviceGeneratedCommandsLimitsNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxIndirectCommandsLayoutTokenCount = src.maxIndirectCommandsLayoutTokenCount;
    maxObjectEntryCounts = src.maxObjectEntryCounts;
    minSequenceCountBufferOffsetAlignment = src.minSequenceCountBufferOffsetAlignment;
    minSequenceIndexBufferOffsetAlignment = src.minSequenceIndexBufferOffsetAlignment;
    minCommandsTokenBufferOffsetAlignment = src.minCommandsTokenBufferOffsetAlignment;
}

safe_VkDeviceGeneratedCommandsLimitsNVX& safe_VkDeviceGeneratedCommandsLimitsNVX::operator=(const safe_VkDeviceGeneratedCommandsLimitsNVX& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxIndirectCommandsLayoutTokenCount = src.maxIndirectCommandsLayoutTokenCount;
    maxObjectEntryCounts = src.maxObjectEntryCounts;
    minSequenceCountBufferOffsetAlignment = src.minSequenceCountBufferOffsetAlignment;
    minSequenceIndexBufferOffsetAlignment = src.minSequenceIndexBufferOffsetAlignment;
    minCommandsTokenBufferOffsetAlignment = src.minCommandsTokenBufferOffsetAlignment;

    return *this;
}

safe_VkDeviceGeneratedCommandsLimitsNVX::~safe_VkDeviceGeneratedCommandsLimitsNVX()
{
}

void safe_VkDeviceGeneratedCommandsLimitsNVX::initialize(const VkDeviceGeneratedCommandsLimitsNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxIndirectCommandsLayoutTokenCount = in_struct->maxIndirectCommandsLayoutTokenCount;
    maxObjectEntryCounts = in_struct->maxObjectEntryCounts;
    minSequenceCountBufferOffsetAlignment = in_struct->minSequenceCountBufferOffsetAlignment;
    minSequenceIndexBufferOffsetAlignment = in_struct->minSequenceIndexBufferOffsetAlignment;
    minCommandsTokenBufferOffsetAlignment = in_struct->minCommandsTokenBufferOffsetAlignment;
}

void safe_VkDeviceGeneratedCommandsLimitsNVX::initialize(const safe_VkDeviceGeneratedCommandsLimitsNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxIndirectCommandsLayoutTokenCount = src->maxIndirectCommandsLayoutTokenCount;
    maxObjectEntryCounts = src->maxObjectEntryCounts;
    minSequenceCountBufferOffsetAlignment = src->minSequenceCountBufferOffsetAlignment;
    minSequenceIndexBufferOffsetAlignment = src->minSequenceIndexBufferOffsetAlignment;
    minCommandsTokenBufferOffsetAlignment = src->minCommandsTokenBufferOffsetAlignment;
}

safe_VkIndirectCommandsLayoutCreateInfoNVX::safe_VkIndirectCommandsLayoutCreateInfoNVX(const VkIndirectCommandsLayoutCreateInfoNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pipelineBindPoint(in_struct->pipelineBindPoint),
    flags(in_struct->flags),
    tokenCount(in_struct->tokenCount),
    pTokens(nullptr)
{
    if (in_struct->pTokens) {
        pTokens = new VkIndirectCommandsLayoutTokenNVX[in_struct->tokenCount];
        memcpy ((void *)pTokens, (void *)in_struct->pTokens, sizeof(VkIndirectCommandsLayoutTokenNVX)*in_struct->tokenCount);
    }
}

safe_VkIndirectCommandsLayoutCreateInfoNVX::safe_VkIndirectCommandsLayoutCreateInfoNVX() :
    pTokens(nullptr)
{}

safe_VkIndirectCommandsLayoutCreateInfoNVX::safe_VkIndirectCommandsLayoutCreateInfoNVX(const safe_VkIndirectCommandsLayoutCreateInfoNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pipelineBindPoint = src.pipelineBindPoint;
    flags = src.flags;
    tokenCount = src.tokenCount;
    pTokens = nullptr;
    if (src.pTokens) {
        pTokens = new VkIndirectCommandsLayoutTokenNVX[src.tokenCount];
        memcpy ((void *)pTokens, (void *)src.pTokens, sizeof(VkIndirectCommandsLayoutTokenNVX)*src.tokenCount);
    }
}

safe_VkIndirectCommandsLayoutCreateInfoNVX& safe_VkIndirectCommandsLayoutCreateInfoNVX::operator=(const safe_VkIndirectCommandsLayoutCreateInfoNVX& src)
{
    if (&src == this) return *this;

    if (pTokens)
        delete[] pTokens;

    sType = src.sType;
    pNext = src.pNext;
    pipelineBindPoint = src.pipelineBindPoint;
    flags = src.flags;
    tokenCount = src.tokenCount;
    pTokens = nullptr;
    if (src.pTokens) {
        pTokens = new VkIndirectCommandsLayoutTokenNVX[src.tokenCount];
        memcpy ((void *)pTokens, (void *)src.pTokens, sizeof(VkIndirectCommandsLayoutTokenNVX)*src.tokenCount);
    }

    return *this;
}

safe_VkIndirectCommandsLayoutCreateInfoNVX::~safe_VkIndirectCommandsLayoutCreateInfoNVX()
{
    if (pTokens)
        delete[] pTokens;
}

void safe_VkIndirectCommandsLayoutCreateInfoNVX::initialize(const VkIndirectCommandsLayoutCreateInfoNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    flags = in_struct->flags;
    tokenCount = in_struct->tokenCount;
    pTokens = nullptr;
    if (in_struct->pTokens) {
        pTokens = new VkIndirectCommandsLayoutTokenNVX[in_struct->tokenCount];
        memcpy ((void *)pTokens, (void *)in_struct->pTokens, sizeof(VkIndirectCommandsLayoutTokenNVX)*in_struct->tokenCount);
    }
}

void safe_VkIndirectCommandsLayoutCreateInfoNVX::initialize(const safe_VkIndirectCommandsLayoutCreateInfoNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pipelineBindPoint = src->pipelineBindPoint;
    flags = src->flags;
    tokenCount = src->tokenCount;
    pTokens = nullptr;
    if (src->pTokens) {
        pTokens = new VkIndirectCommandsLayoutTokenNVX[src->tokenCount];
        memcpy ((void *)pTokens, (void *)src->pTokens, sizeof(VkIndirectCommandsLayoutTokenNVX)*src->tokenCount);
    }
}

safe_VkCmdProcessCommandsInfoNVX::safe_VkCmdProcessCommandsInfoNVX(const VkCmdProcessCommandsInfoNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectTable(in_struct->objectTable),
    indirectCommandsLayout(in_struct->indirectCommandsLayout),
    indirectCommandsTokenCount(in_struct->indirectCommandsTokenCount),
    pIndirectCommandsTokens(nullptr),
    maxSequencesCount(in_struct->maxSequencesCount),
    targetCommandBuffer(in_struct->targetCommandBuffer),
    sequencesCountBuffer(in_struct->sequencesCountBuffer),
    sequencesCountOffset(in_struct->sequencesCountOffset),
    sequencesIndexBuffer(in_struct->sequencesIndexBuffer),
    sequencesIndexOffset(in_struct->sequencesIndexOffset)
{
    if (indirectCommandsTokenCount && in_struct->pIndirectCommandsTokens) {
        pIndirectCommandsTokens = new VkIndirectCommandsTokenNVX[indirectCommandsTokenCount];
        for (uint32_t i=0; i<indirectCommandsTokenCount; ++i) {
            pIndirectCommandsTokens[i] = in_struct->pIndirectCommandsTokens[i];
        }
    }
}

safe_VkCmdProcessCommandsInfoNVX::safe_VkCmdProcessCommandsInfoNVX() :
    pIndirectCommandsTokens(nullptr)
{}

safe_VkCmdProcessCommandsInfoNVX::safe_VkCmdProcessCommandsInfoNVX(const safe_VkCmdProcessCommandsInfoNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectTable = src.objectTable;
    indirectCommandsLayout = src.indirectCommandsLayout;
    indirectCommandsTokenCount = src.indirectCommandsTokenCount;
    pIndirectCommandsTokens = nullptr;
    maxSequencesCount = src.maxSequencesCount;
    targetCommandBuffer = src.targetCommandBuffer;
    sequencesCountBuffer = src.sequencesCountBuffer;
    sequencesCountOffset = src.sequencesCountOffset;
    sequencesIndexBuffer = src.sequencesIndexBuffer;
    sequencesIndexOffset = src.sequencesIndexOffset;
    if (indirectCommandsTokenCount && src.pIndirectCommandsTokens) {
        pIndirectCommandsTokens = new VkIndirectCommandsTokenNVX[indirectCommandsTokenCount];
        for (uint32_t i=0; i<indirectCommandsTokenCount; ++i) {
            pIndirectCommandsTokens[i] = src.pIndirectCommandsTokens[i];
        }
    }
}

safe_VkCmdProcessCommandsInfoNVX& safe_VkCmdProcessCommandsInfoNVX::operator=(const safe_VkCmdProcessCommandsInfoNVX& src)
{
    if (&src == this) return *this;

    if (pIndirectCommandsTokens)
        delete[] pIndirectCommandsTokens;

    sType = src.sType;
    pNext = src.pNext;
    objectTable = src.objectTable;
    indirectCommandsLayout = src.indirectCommandsLayout;
    indirectCommandsTokenCount = src.indirectCommandsTokenCount;
    pIndirectCommandsTokens = nullptr;
    maxSequencesCount = src.maxSequencesCount;
    targetCommandBuffer = src.targetCommandBuffer;
    sequencesCountBuffer = src.sequencesCountBuffer;
    sequencesCountOffset = src.sequencesCountOffset;
    sequencesIndexBuffer = src.sequencesIndexBuffer;
    sequencesIndexOffset = src.sequencesIndexOffset;
    if (indirectCommandsTokenCount && src.pIndirectCommandsTokens) {
        pIndirectCommandsTokens = new VkIndirectCommandsTokenNVX[indirectCommandsTokenCount];
        for (uint32_t i=0; i<indirectCommandsTokenCount; ++i) {
            pIndirectCommandsTokens[i] = src.pIndirectCommandsTokens[i];
        }
    }

    return *this;
}

safe_VkCmdProcessCommandsInfoNVX::~safe_VkCmdProcessCommandsInfoNVX()
{
    if (pIndirectCommandsTokens)
        delete[] pIndirectCommandsTokens;
}

void safe_VkCmdProcessCommandsInfoNVX::initialize(const VkCmdProcessCommandsInfoNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectTable = in_struct->objectTable;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    indirectCommandsTokenCount = in_struct->indirectCommandsTokenCount;
    pIndirectCommandsTokens = nullptr;
    maxSequencesCount = in_struct->maxSequencesCount;
    targetCommandBuffer = in_struct->targetCommandBuffer;
    sequencesCountBuffer = in_struct->sequencesCountBuffer;
    sequencesCountOffset = in_struct->sequencesCountOffset;
    sequencesIndexBuffer = in_struct->sequencesIndexBuffer;
    sequencesIndexOffset = in_struct->sequencesIndexOffset;
    if (indirectCommandsTokenCount && in_struct->pIndirectCommandsTokens) {
        pIndirectCommandsTokens = new VkIndirectCommandsTokenNVX[indirectCommandsTokenCount];
        for (uint32_t i=0; i<indirectCommandsTokenCount; ++i) {
            pIndirectCommandsTokens[i] = in_struct->pIndirectCommandsTokens[i];
        }
    }
}

void safe_VkCmdProcessCommandsInfoNVX::initialize(const safe_VkCmdProcessCommandsInfoNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectTable = src->objectTable;
    indirectCommandsLayout = src->indirectCommandsLayout;
    indirectCommandsTokenCount = src->indirectCommandsTokenCount;
    pIndirectCommandsTokens = nullptr;
    maxSequencesCount = src->maxSequencesCount;
    targetCommandBuffer = src->targetCommandBuffer;
    sequencesCountBuffer = src->sequencesCountBuffer;
    sequencesCountOffset = src->sequencesCountOffset;
    sequencesIndexBuffer = src->sequencesIndexBuffer;
    sequencesIndexOffset = src->sequencesIndexOffset;
    if (indirectCommandsTokenCount && src->pIndirectCommandsTokens) {
        pIndirectCommandsTokens = new VkIndirectCommandsTokenNVX[indirectCommandsTokenCount];
        for (uint32_t i=0; i<indirectCommandsTokenCount; ++i) {
            pIndirectCommandsTokens[i] = src->pIndirectCommandsTokens[i];
        }
    }
}

safe_VkCmdReserveSpaceForCommandsInfoNVX::safe_VkCmdReserveSpaceForCommandsInfoNVX(const VkCmdReserveSpaceForCommandsInfoNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectTable(in_struct->objectTable),
    indirectCommandsLayout(in_struct->indirectCommandsLayout),
    maxSequencesCount(in_struct->maxSequencesCount)
{
}

safe_VkCmdReserveSpaceForCommandsInfoNVX::safe_VkCmdReserveSpaceForCommandsInfoNVX()
{}

safe_VkCmdReserveSpaceForCommandsInfoNVX::safe_VkCmdReserveSpaceForCommandsInfoNVX(const safe_VkCmdReserveSpaceForCommandsInfoNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectTable = src.objectTable;
    indirectCommandsLayout = src.indirectCommandsLayout;
    maxSequencesCount = src.maxSequencesCount;
}

safe_VkCmdReserveSpaceForCommandsInfoNVX& safe_VkCmdReserveSpaceForCommandsInfoNVX::operator=(const safe_VkCmdReserveSpaceForCommandsInfoNVX& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    objectTable = src.objectTable;
    indirectCommandsLayout = src.indirectCommandsLayout;
    maxSequencesCount = src.maxSequencesCount;

    return *this;
}

safe_VkCmdReserveSpaceForCommandsInfoNVX::~safe_VkCmdReserveSpaceForCommandsInfoNVX()
{
}

void safe_VkCmdReserveSpaceForCommandsInfoNVX::initialize(const VkCmdReserveSpaceForCommandsInfoNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectTable = in_struct->objectTable;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    maxSequencesCount = in_struct->maxSequencesCount;
}

void safe_VkCmdReserveSpaceForCommandsInfoNVX::initialize(const safe_VkCmdReserveSpaceForCommandsInfoNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectTable = src->objectTable;
    indirectCommandsLayout = src->indirectCommandsLayout;
    maxSequencesCount = src->maxSequencesCount;
}

safe_VkObjectTableCreateInfoNVX::safe_VkObjectTableCreateInfoNVX(const VkObjectTableCreateInfoNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectCount(in_struct->objectCount),
    pObjectEntryTypes(nullptr),
    pObjectEntryCounts(nullptr),
    pObjectEntryUsageFlags(nullptr),
    maxUniformBuffersPerDescriptor(in_struct->maxUniformBuffersPerDescriptor),
    maxStorageBuffersPerDescriptor(in_struct->maxStorageBuffersPerDescriptor),
    maxStorageImagesPerDescriptor(in_struct->maxStorageImagesPerDescriptor),
    maxSampledImagesPerDescriptor(in_struct->maxSampledImagesPerDescriptor),
    maxPipelineLayouts(in_struct->maxPipelineLayouts)
{
    if (in_struct->pObjectEntryTypes) {
        pObjectEntryTypes = new VkObjectEntryTypeNVX[in_struct->objectCount];
        memcpy ((void *)pObjectEntryTypes, (void *)in_struct->pObjectEntryTypes, sizeof(VkObjectEntryTypeNVX)*in_struct->objectCount);
    }
    if (in_struct->pObjectEntryCounts) {
        pObjectEntryCounts = new uint32_t[in_struct->objectCount];
        memcpy ((void *)pObjectEntryCounts, (void *)in_struct->pObjectEntryCounts, sizeof(uint32_t)*in_struct->objectCount);
    }
    if (in_struct->pObjectEntryUsageFlags) {
        pObjectEntryUsageFlags = new VkObjectEntryUsageFlagsNVX[in_struct->objectCount];
        memcpy ((void *)pObjectEntryUsageFlags, (void *)in_struct->pObjectEntryUsageFlags, sizeof(VkObjectEntryUsageFlagsNVX)*in_struct->objectCount);
    }
}

safe_VkObjectTableCreateInfoNVX::safe_VkObjectTableCreateInfoNVX() :
    pObjectEntryTypes(nullptr),
    pObjectEntryCounts(nullptr),
    pObjectEntryUsageFlags(nullptr)
{}

safe_VkObjectTableCreateInfoNVX::safe_VkObjectTableCreateInfoNVX(const safe_VkObjectTableCreateInfoNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectCount = src.objectCount;
    pObjectEntryTypes = nullptr;
    pObjectEntryCounts = nullptr;
    pObjectEntryUsageFlags = nullptr;
    maxUniformBuffersPerDescriptor = src.maxUniformBuffersPerDescriptor;
    maxStorageBuffersPerDescriptor = src.maxStorageBuffersPerDescriptor;
    maxStorageImagesPerDescriptor = src.maxStorageImagesPerDescriptor;
    maxSampledImagesPerDescriptor = src.maxSampledImagesPerDescriptor;
    maxPipelineLayouts = src.maxPipelineLayouts;
    if (src.pObjectEntryTypes) {
        pObjectEntryTypes = new VkObjectEntryTypeNVX[src.objectCount];
        memcpy ((void *)pObjectEntryTypes, (void *)src.pObjectEntryTypes, sizeof(VkObjectEntryTypeNVX)*src.objectCount);
    }
    if (src.pObjectEntryCounts) {
        pObjectEntryCounts = new uint32_t[src.objectCount];
        memcpy ((void *)pObjectEntryCounts, (void *)src.pObjectEntryCounts, sizeof(uint32_t)*src.objectCount);
    }
    if (src.pObjectEntryUsageFlags) {
        pObjectEntryUsageFlags = new VkObjectEntryUsageFlagsNVX[src.objectCount];
        memcpy ((void *)pObjectEntryUsageFlags, (void *)src.pObjectEntryUsageFlags, sizeof(VkObjectEntryUsageFlagsNVX)*src.objectCount);
    }
}

safe_VkObjectTableCreateInfoNVX& safe_VkObjectTableCreateInfoNVX::operator=(const safe_VkObjectTableCreateInfoNVX& src)
{
    if (&src == this) return *this;

    if (pObjectEntryTypes)
        delete[] pObjectEntryTypes;
    if (pObjectEntryCounts)
        delete[] pObjectEntryCounts;
    if (pObjectEntryUsageFlags)
        delete[] pObjectEntryUsageFlags;

    sType = src.sType;
    pNext = src.pNext;
    objectCount = src.objectCount;
    pObjectEntryTypes = nullptr;
    pObjectEntryCounts = nullptr;
    pObjectEntryUsageFlags = nullptr;
    maxUniformBuffersPerDescriptor = src.maxUniformBuffersPerDescriptor;
    maxStorageBuffersPerDescriptor = src.maxStorageBuffersPerDescriptor;
    maxStorageImagesPerDescriptor = src.maxStorageImagesPerDescriptor;
    maxSampledImagesPerDescriptor = src.maxSampledImagesPerDescriptor;
    maxPipelineLayouts = src.maxPipelineLayouts;
    if (src.pObjectEntryTypes) {
        pObjectEntryTypes = new VkObjectEntryTypeNVX[src.objectCount];
        memcpy ((void *)pObjectEntryTypes, (void *)src.pObjectEntryTypes, sizeof(VkObjectEntryTypeNVX)*src.objectCount);
    }
    if (src.pObjectEntryCounts) {
        pObjectEntryCounts = new uint32_t[src.objectCount];
        memcpy ((void *)pObjectEntryCounts, (void *)src.pObjectEntryCounts, sizeof(uint32_t)*src.objectCount);
    }
    if (src.pObjectEntryUsageFlags) {
        pObjectEntryUsageFlags = new VkObjectEntryUsageFlagsNVX[src.objectCount];
        memcpy ((void *)pObjectEntryUsageFlags, (void *)src.pObjectEntryUsageFlags, sizeof(VkObjectEntryUsageFlagsNVX)*src.objectCount);
    }

    return *this;
}

safe_VkObjectTableCreateInfoNVX::~safe_VkObjectTableCreateInfoNVX()
{
    if (pObjectEntryTypes)
        delete[] pObjectEntryTypes;
    if (pObjectEntryCounts)
        delete[] pObjectEntryCounts;
    if (pObjectEntryUsageFlags)
        delete[] pObjectEntryUsageFlags;
}

void safe_VkObjectTableCreateInfoNVX::initialize(const VkObjectTableCreateInfoNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectCount = in_struct->objectCount;
    pObjectEntryTypes = nullptr;
    pObjectEntryCounts = nullptr;
    pObjectEntryUsageFlags = nullptr;
    maxUniformBuffersPerDescriptor = in_struct->maxUniformBuffersPerDescriptor;
    maxStorageBuffersPerDescriptor = in_struct->maxStorageBuffersPerDescriptor;
    maxStorageImagesPerDescriptor = in_struct->maxStorageImagesPerDescriptor;
    maxSampledImagesPerDescriptor = in_struct->maxSampledImagesPerDescriptor;
    maxPipelineLayouts = in_struct->maxPipelineLayouts;
    if (in_struct->pObjectEntryTypes) {
        pObjectEntryTypes = new VkObjectEntryTypeNVX[in_struct->objectCount];
        memcpy ((void *)pObjectEntryTypes, (void *)in_struct->pObjectEntryTypes, sizeof(VkObjectEntryTypeNVX)*in_struct->objectCount);
    }
    if (in_struct->pObjectEntryCounts) {
        pObjectEntryCounts = new uint32_t[in_struct->objectCount];
        memcpy ((void *)pObjectEntryCounts, (void *)in_struct->pObjectEntryCounts, sizeof(uint32_t)*in_struct->objectCount);
    }
    if (in_struct->pObjectEntryUsageFlags) {
        pObjectEntryUsageFlags = new VkObjectEntryUsageFlagsNVX[in_struct->objectCount];
        memcpy ((void *)pObjectEntryUsageFlags, (void *)in_struct->pObjectEntryUsageFlags, sizeof(VkObjectEntryUsageFlagsNVX)*in_struct->objectCount);
    }
}

void safe_VkObjectTableCreateInfoNVX::initialize(const safe_VkObjectTableCreateInfoNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectCount = src->objectCount;
    pObjectEntryTypes = nullptr;
    pObjectEntryCounts = nullptr;
    pObjectEntryUsageFlags = nullptr;
    maxUniformBuffersPerDescriptor = src->maxUniformBuffersPerDescriptor;
    maxStorageBuffersPerDescriptor = src->maxStorageBuffersPerDescriptor;
    maxStorageImagesPerDescriptor = src->maxStorageImagesPerDescriptor;
    maxSampledImagesPerDescriptor = src->maxSampledImagesPerDescriptor;
    maxPipelineLayouts = src->maxPipelineLayouts;
    if (src->pObjectEntryTypes) {
        pObjectEntryTypes = new VkObjectEntryTypeNVX[src->objectCount];
        memcpy ((void *)pObjectEntryTypes, (void *)src->pObjectEntryTypes, sizeof(VkObjectEntryTypeNVX)*src->objectCount);
    }
    if (src->pObjectEntryCounts) {
        pObjectEntryCounts = new uint32_t[src->objectCount];
        memcpy ((void *)pObjectEntryCounts, (void *)src->pObjectEntryCounts, sizeof(uint32_t)*src->objectCount);
    }
    if (src->pObjectEntryUsageFlags) {
        pObjectEntryUsageFlags = new VkObjectEntryUsageFlagsNVX[src->objectCount];
        memcpy ((void *)pObjectEntryUsageFlags, (void *)src->pObjectEntryUsageFlags, sizeof(VkObjectEntryUsageFlagsNVX)*src->objectCount);
    }
}

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV(const VkPipelineViewportWScalingStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    viewportWScalingEnable(in_struct->viewportWScalingEnable),
    viewportCount(in_struct->viewportCount),
    pViewportWScalings(nullptr)
{
    if (in_struct->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[in_struct->viewportCount];
        memcpy ((void *)pViewportWScalings, (void *)in_struct->pViewportWScalings, sizeof(VkViewportWScalingNV)*in_struct->viewportCount);
    }
}

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV() :
    pViewportWScalings(nullptr)
{}

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV(const safe_VkPipelineViewportWScalingStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    viewportWScalingEnable = src.viewportWScalingEnable;
    viewportCount = src.viewportCount;
    pViewportWScalings = nullptr;
    if (src.pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[src.viewportCount];
        memcpy ((void *)pViewportWScalings, (void *)src.pViewportWScalings, sizeof(VkViewportWScalingNV)*src.viewportCount);
    }
}

safe_VkPipelineViewportWScalingStateCreateInfoNV& safe_VkPipelineViewportWScalingStateCreateInfoNV::operator=(const safe_VkPipelineViewportWScalingStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pViewportWScalings)
        delete[] pViewportWScalings;

    sType = src.sType;
    pNext = src.pNext;
    viewportWScalingEnable = src.viewportWScalingEnable;
    viewportCount = src.viewportCount;
    pViewportWScalings = nullptr;
    if (src.pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[src.viewportCount];
        memcpy ((void *)pViewportWScalings, (void *)src.pViewportWScalings, sizeof(VkViewportWScalingNV)*src.viewportCount);
    }

    return *this;
}

safe_VkPipelineViewportWScalingStateCreateInfoNV::~safe_VkPipelineViewportWScalingStateCreateInfoNV()
{
    if (pViewportWScalings)
        delete[] pViewportWScalings;
}

void safe_VkPipelineViewportWScalingStateCreateInfoNV::initialize(const VkPipelineViewportWScalingStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    viewportWScalingEnable = in_struct->viewportWScalingEnable;
    viewportCount = in_struct->viewportCount;
    pViewportWScalings = nullptr;
    if (in_struct->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[in_struct->viewportCount];
        memcpy ((void *)pViewportWScalings, (void *)in_struct->pViewportWScalings, sizeof(VkViewportWScalingNV)*in_struct->viewportCount);
    }
}

void safe_VkPipelineViewportWScalingStateCreateInfoNV::initialize(const safe_VkPipelineViewportWScalingStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    viewportWScalingEnable = src->viewportWScalingEnable;
    viewportCount = src->viewportCount;
    pViewportWScalings = nullptr;
    if (src->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[src->viewportCount];
        memcpy ((void *)pViewportWScalings, (void *)src->pViewportWScalings, sizeof(VkViewportWScalingNV)*src->viewportCount);
    }
}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT(const VkSurfaceCapabilities2EXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    minImageCount(in_struct->minImageCount),
    maxImageCount(in_struct->maxImageCount),
    currentExtent(in_struct->currentExtent),
    minImageExtent(in_struct->minImageExtent),
    maxImageExtent(in_struct->maxImageExtent),
    maxImageArrayLayers(in_struct->maxImageArrayLayers),
    supportedTransforms(in_struct->supportedTransforms),
    currentTransform(in_struct->currentTransform),
    supportedCompositeAlpha(in_struct->supportedCompositeAlpha),
    supportedUsageFlags(in_struct->supportedUsageFlags),
    supportedSurfaceCounters(in_struct->supportedSurfaceCounters)
{
}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT()
{}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT(const safe_VkSurfaceCapabilities2EXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    minImageCount = src.minImageCount;
    maxImageCount = src.maxImageCount;
    currentExtent = src.currentExtent;
    minImageExtent = src.minImageExtent;
    maxImageExtent = src.maxImageExtent;
    maxImageArrayLayers = src.maxImageArrayLayers;
    supportedTransforms = src.supportedTransforms;
    currentTransform = src.currentTransform;
    supportedCompositeAlpha = src.supportedCompositeAlpha;
    supportedUsageFlags = src.supportedUsageFlags;
    supportedSurfaceCounters = src.supportedSurfaceCounters;
}

safe_VkSurfaceCapabilities2EXT& safe_VkSurfaceCapabilities2EXT::operator=(const safe_VkSurfaceCapabilities2EXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    minImageCount = src.minImageCount;
    maxImageCount = src.maxImageCount;
    currentExtent = src.currentExtent;
    minImageExtent = src.minImageExtent;
    maxImageExtent = src.maxImageExtent;
    maxImageArrayLayers = src.maxImageArrayLayers;
    supportedTransforms = src.supportedTransforms;
    currentTransform = src.currentTransform;
    supportedCompositeAlpha = src.supportedCompositeAlpha;
    supportedUsageFlags = src.supportedUsageFlags;
    supportedSurfaceCounters = src.supportedSurfaceCounters;

    return *this;
}

safe_VkSurfaceCapabilities2EXT::~safe_VkSurfaceCapabilities2EXT()
{
}

void safe_VkSurfaceCapabilities2EXT::initialize(const VkSurfaceCapabilities2EXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    minImageCount = in_struct->minImageCount;
    maxImageCount = in_struct->maxImageCount;
    currentExtent = in_struct->currentExtent;
    minImageExtent = in_struct->minImageExtent;
    maxImageExtent = in_struct->maxImageExtent;
    maxImageArrayLayers = in_struct->maxImageArrayLayers;
    supportedTransforms = in_struct->supportedTransforms;
    currentTransform = in_struct->currentTransform;
    supportedCompositeAlpha = in_struct->supportedCompositeAlpha;
    supportedUsageFlags = in_struct->supportedUsageFlags;
    supportedSurfaceCounters = in_struct->supportedSurfaceCounters;
}

void safe_VkSurfaceCapabilities2EXT::initialize(const safe_VkSurfaceCapabilities2EXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    minImageCount = src->minImageCount;
    maxImageCount = src->maxImageCount;
    currentExtent = src->currentExtent;
    minImageExtent = src->minImageExtent;
    maxImageExtent = src->maxImageExtent;
    maxImageArrayLayers = src->maxImageArrayLayers;
    supportedTransforms = src->supportedTransforms;
    currentTransform = src->currentTransform;
    supportedCompositeAlpha = src->supportedCompositeAlpha;
    supportedUsageFlags = src->supportedUsageFlags;
    supportedSurfaceCounters = src->supportedSurfaceCounters;
}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT(const VkDisplayPowerInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    powerState(in_struct->powerState)
{
}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT()
{}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT(const safe_VkDisplayPowerInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    powerState = src.powerState;
}

safe_VkDisplayPowerInfoEXT& safe_VkDisplayPowerInfoEXT::operator=(const safe_VkDisplayPowerInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    powerState = src.powerState;

    return *this;
}

safe_VkDisplayPowerInfoEXT::~safe_VkDisplayPowerInfoEXT()
{
}

void safe_VkDisplayPowerInfoEXT::initialize(const VkDisplayPowerInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    powerState = in_struct->powerState;
}

void safe_VkDisplayPowerInfoEXT::initialize(const safe_VkDisplayPowerInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    powerState = src->powerState;
}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT(const VkDeviceEventInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceEvent(in_struct->deviceEvent)
{
}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT()
{}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT(const safe_VkDeviceEventInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceEvent = src.deviceEvent;
}

safe_VkDeviceEventInfoEXT& safe_VkDeviceEventInfoEXT::operator=(const safe_VkDeviceEventInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    deviceEvent = src.deviceEvent;

    return *this;
}

safe_VkDeviceEventInfoEXT::~safe_VkDeviceEventInfoEXT()
{
}

void safe_VkDeviceEventInfoEXT::initialize(const VkDeviceEventInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceEvent = in_struct->deviceEvent;
}

void safe_VkDeviceEventInfoEXT::initialize(const safe_VkDeviceEventInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceEvent = src->deviceEvent;
}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT(const VkDisplayEventInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    displayEvent(in_struct->displayEvent)
{
}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT()
{}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT(const safe_VkDisplayEventInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    displayEvent = src.displayEvent;
}

safe_VkDisplayEventInfoEXT& safe_VkDisplayEventInfoEXT::operator=(const safe_VkDisplayEventInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    displayEvent = src.displayEvent;

    return *this;
}

safe_VkDisplayEventInfoEXT::~safe_VkDisplayEventInfoEXT()
{
}

void safe_VkDisplayEventInfoEXT::initialize(const VkDisplayEventInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    displayEvent = in_struct->displayEvent;
}

void safe_VkDisplayEventInfoEXT::initialize(const safe_VkDisplayEventInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    displayEvent = src->displayEvent;
}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT(const VkSwapchainCounterCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    surfaceCounters(in_struct->surfaceCounters)
{
}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT()
{}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT(const safe_VkSwapchainCounterCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    surfaceCounters = src.surfaceCounters;
}

safe_VkSwapchainCounterCreateInfoEXT& safe_VkSwapchainCounterCreateInfoEXT::operator=(const safe_VkSwapchainCounterCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    surfaceCounters = src.surfaceCounters;

    return *this;
}

safe_VkSwapchainCounterCreateInfoEXT::~safe_VkSwapchainCounterCreateInfoEXT()
{
}

void safe_VkSwapchainCounterCreateInfoEXT::initialize(const VkSwapchainCounterCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    surfaceCounters = in_struct->surfaceCounters;
}

void safe_VkSwapchainCounterCreateInfoEXT::initialize(const safe_VkSwapchainCounterCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    surfaceCounters = src->surfaceCounters;
}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE(const VkPresentTimesInfoGOOGLE* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    swapchainCount(in_struct->swapchainCount),
    pTimes(nullptr)
{
    if (in_struct->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[in_struct->swapchainCount];
        memcpy ((void *)pTimes, (void *)in_struct->pTimes, sizeof(VkPresentTimeGOOGLE)*in_struct->swapchainCount);
    }
}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE() :
    pTimes(nullptr)
{}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE(const safe_VkPresentTimesInfoGOOGLE& src)
{
    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pTimes = nullptr;
    if (src.pTimes) {
        pTimes = new VkPresentTimeGOOGLE[src.swapchainCount];
        memcpy ((void *)pTimes, (void *)src.pTimes, sizeof(VkPresentTimeGOOGLE)*src.swapchainCount);
    }
}

safe_VkPresentTimesInfoGOOGLE& safe_VkPresentTimesInfoGOOGLE::operator=(const safe_VkPresentTimesInfoGOOGLE& src)
{
    if (&src == this) return *this;

    if (pTimes)
        delete[] pTimes;

    sType = src.sType;
    pNext = src.pNext;
    swapchainCount = src.swapchainCount;
    pTimes = nullptr;
    if (src.pTimes) {
        pTimes = new VkPresentTimeGOOGLE[src.swapchainCount];
        memcpy ((void *)pTimes, (void *)src.pTimes, sizeof(VkPresentTimeGOOGLE)*src.swapchainCount);
    }

    return *this;
}

safe_VkPresentTimesInfoGOOGLE::~safe_VkPresentTimesInfoGOOGLE()
{
    if (pTimes)
        delete[] pTimes;
}

void safe_VkPresentTimesInfoGOOGLE::initialize(const VkPresentTimesInfoGOOGLE* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    swapchainCount = in_struct->swapchainCount;
    pTimes = nullptr;
    if (in_struct->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[in_struct->swapchainCount];
        memcpy ((void *)pTimes, (void *)in_struct->pTimes, sizeof(VkPresentTimeGOOGLE)*in_struct->swapchainCount);
    }
}

void safe_VkPresentTimesInfoGOOGLE::initialize(const safe_VkPresentTimesInfoGOOGLE* src)
{
    sType = src->sType;
    pNext = src->pNext;
    swapchainCount = src->swapchainCount;
    pTimes = nullptr;
    if (src->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[src->swapchainCount];
        memcpy ((void *)pTimes, (void *)src->pTimes, sizeof(VkPresentTimeGOOGLE)*src->swapchainCount);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    perViewPositionAllComponents(in_struct->perViewPositionAllComponents)
{
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX()
{}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& src)
{
    sType = src.sType;
    pNext = src.pNext;
    perViewPositionAllComponents = src.perViewPositionAllComponents;
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::operator=(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    perViewPositionAllComponents = src.perViewPositionAllComponents;

    return *this;
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::~safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX()
{
}

void safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::initialize(const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    perViewPositionAllComponents = in_struct->perViewPositionAllComponents;
}

void safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::initialize(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* src)
{
    sType = src->sType;
    pNext = src->pNext;
    perViewPositionAllComponents = src->perViewPositionAllComponents;
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV(const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    viewportCount(in_struct->viewportCount),
    pViewportSwizzles(nullptr)
{
    if (in_struct->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[in_struct->viewportCount];
        memcpy ((void *)pViewportSwizzles, (void *)in_struct->pViewportSwizzles, sizeof(VkViewportSwizzleNV)*in_struct->viewportCount);
    }
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV() :
    pViewportSwizzles(nullptr)
{}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV(const safe_VkPipelineViewportSwizzleStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    viewportCount = src.viewportCount;
    pViewportSwizzles = nullptr;
    if (src.pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[src.viewportCount];
        memcpy ((void *)pViewportSwizzles, (void *)src.pViewportSwizzles, sizeof(VkViewportSwizzleNV)*src.viewportCount);
    }
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV& safe_VkPipelineViewportSwizzleStateCreateInfoNV::operator=(const safe_VkPipelineViewportSwizzleStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pViewportSwizzles)
        delete[] pViewportSwizzles;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    viewportCount = src.viewportCount;
    pViewportSwizzles = nullptr;
    if (src.pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[src.viewportCount];
        memcpy ((void *)pViewportSwizzles, (void *)src.pViewportSwizzles, sizeof(VkViewportSwizzleNV)*src.viewportCount);
    }

    return *this;
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::~safe_VkPipelineViewportSwizzleStateCreateInfoNV()
{
    if (pViewportSwizzles)
        delete[] pViewportSwizzles;
}

void safe_VkPipelineViewportSwizzleStateCreateInfoNV::initialize(const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    viewportCount = in_struct->viewportCount;
    pViewportSwizzles = nullptr;
    if (in_struct->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[in_struct->viewportCount];
        memcpy ((void *)pViewportSwizzles, (void *)in_struct->pViewportSwizzles, sizeof(VkViewportSwizzleNV)*in_struct->viewportCount);
    }
}

void safe_VkPipelineViewportSwizzleStateCreateInfoNV::initialize(const safe_VkPipelineViewportSwizzleStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    viewportCount = src->viewportCount;
    pViewportSwizzles = nullptr;
    if (src->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[src->viewportCount];
        memcpy ((void *)pViewportSwizzles, (void *)src->pViewportSwizzles, sizeof(VkViewportSwizzleNV)*src->viewportCount);
    }
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxDiscardRectangles(in_struct->maxDiscardRectangles)
{
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT()
{}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxDiscardRectangles = src.maxDiscardRectangles;
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::operator=(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxDiscardRectangles = src.maxDiscardRectangles;

    return *this;
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::~safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT()
{
}

void safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::initialize(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxDiscardRectangles = in_struct->maxDiscardRectangles;
}

void safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::initialize(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxDiscardRectangles = src->maxDiscardRectangles;
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    discardRectangleMode(in_struct->discardRectangleMode),
    discardRectangleCount(in_struct->discardRectangleCount),
    pDiscardRectangles(nullptr)
{
    if (in_struct->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[in_struct->discardRectangleCount];
        memcpy ((void *)pDiscardRectangles, (void *)in_struct->pDiscardRectangles, sizeof(VkRect2D)*in_struct->discardRectangleCount);
    }
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT() :
    pDiscardRectangles(nullptr)
{}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    discardRectangleMode = src.discardRectangleMode;
    discardRectangleCount = src.discardRectangleCount;
    pDiscardRectangles = nullptr;
    if (src.pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[src.discardRectangleCount];
        memcpy ((void *)pDiscardRectangles, (void *)src.pDiscardRectangles, sizeof(VkRect2D)*src.discardRectangleCount);
    }
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT& safe_VkPipelineDiscardRectangleStateCreateInfoEXT::operator=(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pDiscardRectangles)
        delete[] pDiscardRectangles;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    discardRectangleMode = src.discardRectangleMode;
    discardRectangleCount = src.discardRectangleCount;
    pDiscardRectangles = nullptr;
    if (src.pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[src.discardRectangleCount];
        memcpy ((void *)pDiscardRectangles, (void *)src.pDiscardRectangles, sizeof(VkRect2D)*src.discardRectangleCount);
    }

    return *this;
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::~safe_VkPipelineDiscardRectangleStateCreateInfoEXT()
{
    if (pDiscardRectangles)
        delete[] pDiscardRectangles;
}

void safe_VkPipelineDiscardRectangleStateCreateInfoEXT::initialize(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    discardRectangleMode = in_struct->discardRectangleMode;
    discardRectangleCount = in_struct->discardRectangleCount;
    pDiscardRectangles = nullptr;
    if (in_struct->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[in_struct->discardRectangleCount];
        memcpy ((void *)pDiscardRectangles, (void *)in_struct->pDiscardRectangles, sizeof(VkRect2D)*in_struct->discardRectangleCount);
    }
}

void safe_VkPipelineDiscardRectangleStateCreateInfoEXT::initialize(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    discardRectangleMode = src->discardRectangleMode;
    discardRectangleCount = src->discardRectangleCount;
    pDiscardRectangles = nullptr;
    if (src->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[src->discardRectangleCount];
        memcpy ((void *)pDiscardRectangles, (void *)src->pDiscardRectangles, sizeof(VkRect2D)*src->discardRectangleCount);
    }
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    primitiveOverestimationSize(in_struct->primitiveOverestimationSize),
    maxExtraPrimitiveOverestimationSize(in_struct->maxExtraPrimitiveOverestimationSize),
    extraPrimitiveOverestimationSizeGranularity(in_struct->extraPrimitiveOverestimationSizeGranularity),
    primitiveUnderestimation(in_struct->primitiveUnderestimation),
    conservativePointAndLineRasterization(in_struct->conservativePointAndLineRasterization),
    degenerateTrianglesRasterized(in_struct->degenerateTrianglesRasterized),
    degenerateLinesRasterized(in_struct->degenerateLinesRasterized),
    fullyCoveredFragmentShaderInputVariable(in_struct->fullyCoveredFragmentShaderInputVariable),
    conservativeRasterizationPostDepthCoverage(in_struct->conservativeRasterizationPostDepthCoverage)
{
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT()
{}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    primitiveOverestimationSize = src.primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = src.maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = src.extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = src.primitiveUnderestimation;
    conservativePointAndLineRasterization = src.conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = src.degenerateTrianglesRasterized;
    degenerateLinesRasterized = src.degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = src.fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = src.conservativeRasterizationPostDepthCoverage;
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::operator=(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    primitiveOverestimationSize = src.primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = src.maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = src.extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = src.primitiveUnderestimation;
    conservativePointAndLineRasterization = src.conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = src.degenerateTrianglesRasterized;
    degenerateLinesRasterized = src.degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = src.fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = src.conservativeRasterizationPostDepthCoverage;

    return *this;
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::~safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT()
{
}

void safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::initialize(const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    primitiveOverestimationSize = in_struct->primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = in_struct->maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = in_struct->extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = in_struct->primitiveUnderestimation;
    conservativePointAndLineRasterization = in_struct->conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = in_struct->degenerateTrianglesRasterized;
    degenerateLinesRasterized = in_struct->degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = in_struct->fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = in_struct->conservativeRasterizationPostDepthCoverage;
}

void safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::initialize(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    primitiveOverestimationSize = src->primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = src->maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = src->extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = src->primitiveUnderestimation;
    conservativePointAndLineRasterization = src->conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = src->degenerateTrianglesRasterized;
    degenerateLinesRasterized = src->degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = src->fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = src->conservativeRasterizationPostDepthCoverage;
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    conservativeRasterizationMode(in_struct->conservativeRasterizationMode),
    extraPrimitiveOverestimationSize(in_struct->extraPrimitiveOverestimationSize)
{
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT()
{}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    conservativeRasterizationMode = src.conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = src.extraPrimitiveOverestimationSize;
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::operator=(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    conservativeRasterizationMode = src.conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = src.extraPrimitiveOverestimationSize;

    return *this;
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::~safe_VkPipelineRasterizationConservativeStateCreateInfoEXT()
{
}

void safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::initialize(const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    conservativeRasterizationMode = in_struct->conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = in_struct->extraPrimitiveOverestimationSize;
}

void safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::initialize(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    conservativeRasterizationMode = src->conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = src->extraPrimitiveOverestimationSize;
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    depthClipEnable(in_struct->depthClipEnable)
{
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT()
{}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    depthClipEnable = src.depthClipEnable;
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::operator=(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    depthClipEnable = src.depthClipEnable;

    return *this;
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::~safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT()
{
}

void safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::initialize(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    depthClipEnable = in_struct->depthClipEnable;
}

void safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::initialize(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    depthClipEnable = src->depthClipEnable;
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    depthClipEnable(in_struct->depthClipEnable)
{
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT()
{}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthClipEnable = src.depthClipEnable;
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::operator=(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    depthClipEnable = src.depthClipEnable;

    return *this;
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::~safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT()
{
}

void safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::initialize(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    depthClipEnable = in_struct->depthClipEnable;
}

void safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::initialize(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    depthClipEnable = src->depthClipEnable;
}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT(const VkHdrMetadataEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    displayPrimaryRed(in_struct->displayPrimaryRed),
    displayPrimaryGreen(in_struct->displayPrimaryGreen),
    displayPrimaryBlue(in_struct->displayPrimaryBlue),
    whitePoint(in_struct->whitePoint),
    maxLuminance(in_struct->maxLuminance),
    minLuminance(in_struct->minLuminance),
    maxContentLightLevel(in_struct->maxContentLightLevel),
    maxFrameAverageLightLevel(in_struct->maxFrameAverageLightLevel)
{
}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT()
{}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT(const safe_VkHdrMetadataEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    displayPrimaryRed = src.displayPrimaryRed;
    displayPrimaryGreen = src.displayPrimaryGreen;
    displayPrimaryBlue = src.displayPrimaryBlue;
    whitePoint = src.whitePoint;
    maxLuminance = src.maxLuminance;
    minLuminance = src.minLuminance;
    maxContentLightLevel = src.maxContentLightLevel;
    maxFrameAverageLightLevel = src.maxFrameAverageLightLevel;
}

safe_VkHdrMetadataEXT& safe_VkHdrMetadataEXT::operator=(const safe_VkHdrMetadataEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    displayPrimaryRed = src.displayPrimaryRed;
    displayPrimaryGreen = src.displayPrimaryGreen;
    displayPrimaryBlue = src.displayPrimaryBlue;
    whitePoint = src.whitePoint;
    maxLuminance = src.maxLuminance;
    minLuminance = src.minLuminance;
    maxContentLightLevel = src.maxContentLightLevel;
    maxFrameAverageLightLevel = src.maxFrameAverageLightLevel;

    return *this;
}

safe_VkHdrMetadataEXT::~safe_VkHdrMetadataEXT()
{
}

void safe_VkHdrMetadataEXT::initialize(const VkHdrMetadataEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    displayPrimaryRed = in_struct->displayPrimaryRed;
    displayPrimaryGreen = in_struct->displayPrimaryGreen;
    displayPrimaryBlue = in_struct->displayPrimaryBlue;
    whitePoint = in_struct->whitePoint;
    maxLuminance = in_struct->maxLuminance;
    minLuminance = in_struct->minLuminance;
    maxContentLightLevel = in_struct->maxContentLightLevel;
    maxFrameAverageLightLevel = in_struct->maxFrameAverageLightLevel;
}

void safe_VkHdrMetadataEXT::initialize(const safe_VkHdrMetadataEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    displayPrimaryRed = src->displayPrimaryRed;
    displayPrimaryGreen = src->displayPrimaryGreen;
    displayPrimaryBlue = src->displayPrimaryBlue;
    whitePoint = src->whitePoint;
    maxLuminance = src->maxLuminance;
    minLuminance = src->minLuminance;
    maxContentLightLevel = src->maxContentLightLevel;
    maxFrameAverageLightLevel = src->maxFrameAverageLightLevel;
}
#ifdef VK_USE_PLATFORM_IOS_MVK


safe_VkIOSSurfaceCreateInfoMVK::safe_VkIOSSurfaceCreateInfoMVK(const VkIOSSurfaceCreateInfoMVK* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pView(in_struct->pView)
{
}

safe_VkIOSSurfaceCreateInfoMVK::safe_VkIOSSurfaceCreateInfoMVK()
{}

safe_VkIOSSurfaceCreateInfoMVK::safe_VkIOSSurfaceCreateInfoMVK(const safe_VkIOSSurfaceCreateInfoMVK& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pView = src.pView;
}

safe_VkIOSSurfaceCreateInfoMVK& safe_VkIOSSurfaceCreateInfoMVK::operator=(const safe_VkIOSSurfaceCreateInfoMVK& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pView = src.pView;

    return *this;
}

safe_VkIOSSurfaceCreateInfoMVK::~safe_VkIOSSurfaceCreateInfoMVK()
{
}

void safe_VkIOSSurfaceCreateInfoMVK::initialize(const VkIOSSurfaceCreateInfoMVK* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pView = in_struct->pView;
}

void safe_VkIOSSurfaceCreateInfoMVK::initialize(const safe_VkIOSSurfaceCreateInfoMVK* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pView = src->pView;
}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK


safe_VkMacOSSurfaceCreateInfoMVK::safe_VkMacOSSurfaceCreateInfoMVK(const VkMacOSSurfaceCreateInfoMVK* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pView(in_struct->pView)
{
}

safe_VkMacOSSurfaceCreateInfoMVK::safe_VkMacOSSurfaceCreateInfoMVK()
{}

safe_VkMacOSSurfaceCreateInfoMVK::safe_VkMacOSSurfaceCreateInfoMVK(const safe_VkMacOSSurfaceCreateInfoMVK& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pView = src.pView;
}

safe_VkMacOSSurfaceCreateInfoMVK& safe_VkMacOSSurfaceCreateInfoMVK::operator=(const safe_VkMacOSSurfaceCreateInfoMVK& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pView = src.pView;

    return *this;
}

safe_VkMacOSSurfaceCreateInfoMVK::~safe_VkMacOSSurfaceCreateInfoMVK()
{
}

void safe_VkMacOSSurfaceCreateInfoMVK::initialize(const VkMacOSSurfaceCreateInfoMVK* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pView = in_struct->pView;
}

void safe_VkMacOSSurfaceCreateInfoMVK::initialize(const safe_VkMacOSSurfaceCreateInfoMVK* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pView = src->pView;
}
#endif // VK_USE_PLATFORM_MACOS_MVK


safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT(const VkDebugUtilsObjectNameInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectType(in_struct->objectType),
    objectHandle(in_struct->objectHandle),
    pObjectName(in_struct->pObjectName)
{
}

safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT()
{}

safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT(const safe_VkDebugUtilsObjectNameInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    objectHandle = src.objectHandle;
    pObjectName = src.pObjectName;
}

safe_VkDebugUtilsObjectNameInfoEXT& safe_VkDebugUtilsObjectNameInfoEXT::operator=(const safe_VkDebugUtilsObjectNameInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    objectHandle = src.objectHandle;
    pObjectName = src.pObjectName;

    return *this;
}

safe_VkDebugUtilsObjectNameInfoEXT::~safe_VkDebugUtilsObjectNameInfoEXT()
{
}

void safe_VkDebugUtilsObjectNameInfoEXT::initialize(const VkDebugUtilsObjectNameInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectType = in_struct->objectType;
    objectHandle = in_struct->objectHandle;
    pObjectName = in_struct->pObjectName;
}

void safe_VkDebugUtilsObjectNameInfoEXT::initialize(const safe_VkDebugUtilsObjectNameInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectType = src->objectType;
    objectHandle = src->objectHandle;
    pObjectName = src->pObjectName;
}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT(const VkDebugUtilsObjectTagInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    objectType(in_struct->objectType),
    objectHandle(in_struct->objectHandle),
    tagName(in_struct->tagName),
    tagSize(in_struct->tagSize),
    pTag(in_struct->pTag)
{
}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT()
{}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT(const safe_VkDebugUtilsObjectTagInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    objectHandle = src.objectHandle;
    tagName = src.tagName;
    tagSize = src.tagSize;
    pTag = src.pTag;
}

safe_VkDebugUtilsObjectTagInfoEXT& safe_VkDebugUtilsObjectTagInfoEXT::operator=(const safe_VkDebugUtilsObjectTagInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    objectType = src.objectType;
    objectHandle = src.objectHandle;
    tagName = src.tagName;
    tagSize = src.tagSize;
    pTag = src.pTag;

    return *this;
}

safe_VkDebugUtilsObjectTagInfoEXT::~safe_VkDebugUtilsObjectTagInfoEXT()
{
}

void safe_VkDebugUtilsObjectTagInfoEXT::initialize(const VkDebugUtilsObjectTagInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    objectType = in_struct->objectType;
    objectHandle = in_struct->objectHandle;
    tagName = in_struct->tagName;
    tagSize = in_struct->tagSize;
    pTag = in_struct->pTag;
}

void safe_VkDebugUtilsObjectTagInfoEXT::initialize(const safe_VkDebugUtilsObjectTagInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    objectType = src->objectType;
    objectHandle = src->objectHandle;
    tagName = src->tagName;
    tagSize = src->tagSize;
    pTag = src->pTag;
}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pLabelName(in_struct->pLabelName)
{
    for (uint32_t i=0; i<4; ++i) {
        color[i] = in_struct->color[i];
    }
}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT()
{}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT(const safe_VkDebugUtilsLabelEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pLabelName = src.pLabelName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src.color[i];
    }
}

safe_VkDebugUtilsLabelEXT& safe_VkDebugUtilsLabelEXT::operator=(const safe_VkDebugUtilsLabelEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pLabelName = src.pLabelName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src.color[i];
    }

    return *this;
}

safe_VkDebugUtilsLabelEXT::~safe_VkDebugUtilsLabelEXT()
{
}

void safe_VkDebugUtilsLabelEXT::initialize(const VkDebugUtilsLabelEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pLabelName = in_struct->pLabelName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = in_struct->color[i];
    }
}

void safe_VkDebugUtilsLabelEXT::initialize(const safe_VkDebugUtilsLabelEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pLabelName = src->pLabelName;
    for (uint32_t i=0; i<4; ++i) {
        color[i] = src->color[i];
    }
}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT(const VkDebugUtilsMessengerCallbackDataEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pMessageIdName(in_struct->pMessageIdName),
    messageIdNumber(in_struct->messageIdNumber),
    pMessage(in_struct->pMessage),
    queueLabelCount(in_struct->queueLabelCount),
    pQueueLabels(nullptr),
    cmdBufLabelCount(in_struct->cmdBufLabelCount),
    pCmdBufLabels(nullptr),
    objectCount(in_struct->objectCount),
    pObjects(nullptr)
{
    if (queueLabelCount && in_struct->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i=0; i<queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&in_struct->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && in_struct->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i=0; i<cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&in_struct->pCmdBufLabels[i]);
        }
    }
    if (objectCount && in_struct->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i=0; i<objectCount; ++i) {
            pObjects[i].initialize(&in_struct->pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT() :
    pQueueLabels(nullptr),
    pCmdBufLabels(nullptr),
    pObjects(nullptr)
{}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT(const safe_VkDebugUtilsMessengerCallbackDataEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pMessageIdName = src.pMessageIdName;
    messageIdNumber = src.messageIdNumber;
    pMessage = src.pMessage;
    queueLabelCount = src.queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = src.cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = src.objectCount;
    pObjects = nullptr;
    if (queueLabelCount && src.pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i=0; i<queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&src.pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && src.pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i=0; i<cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&src.pCmdBufLabels[i]);
        }
    }
    if (objectCount && src.pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i=0; i<objectCount; ++i) {
            pObjects[i].initialize(&src.pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCallbackDataEXT& safe_VkDebugUtilsMessengerCallbackDataEXT::operator=(const safe_VkDebugUtilsMessengerCallbackDataEXT& src)
{
    if (&src == this) return *this;

    if (pQueueLabels)
        delete[] pQueueLabels;
    if (pCmdBufLabels)
        delete[] pCmdBufLabels;
    if (pObjects)
        delete[] pObjects;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pMessageIdName = src.pMessageIdName;
    messageIdNumber = src.messageIdNumber;
    pMessage = src.pMessage;
    queueLabelCount = src.queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = src.cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = src.objectCount;
    pObjects = nullptr;
    if (queueLabelCount && src.pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i=0; i<queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&src.pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && src.pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i=0; i<cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&src.pCmdBufLabels[i]);
        }
    }
    if (objectCount && src.pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i=0; i<objectCount; ++i) {
            pObjects[i].initialize(&src.pObjects[i]);
        }
    }

    return *this;
}

safe_VkDebugUtilsMessengerCallbackDataEXT::~safe_VkDebugUtilsMessengerCallbackDataEXT()
{
    if (pQueueLabels)
        delete[] pQueueLabels;
    if (pCmdBufLabels)
        delete[] pCmdBufLabels;
    if (pObjects)
        delete[] pObjects;
}

void safe_VkDebugUtilsMessengerCallbackDataEXT::initialize(const VkDebugUtilsMessengerCallbackDataEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pMessageIdName = in_struct->pMessageIdName;
    messageIdNumber = in_struct->messageIdNumber;
    pMessage = in_struct->pMessage;
    queueLabelCount = in_struct->queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = in_struct->cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = in_struct->objectCount;
    pObjects = nullptr;
    if (queueLabelCount && in_struct->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i=0; i<queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&in_struct->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && in_struct->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i=0; i<cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&in_struct->pCmdBufLabels[i]);
        }
    }
    if (objectCount && in_struct->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i=0; i<objectCount; ++i) {
            pObjects[i].initialize(&in_struct->pObjects[i]);
        }
    }
}

void safe_VkDebugUtilsMessengerCallbackDataEXT::initialize(const safe_VkDebugUtilsMessengerCallbackDataEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pMessageIdName = src->pMessageIdName;
    messageIdNumber = src->messageIdNumber;
    pMessage = src->pMessage;
    queueLabelCount = src->queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = src->cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = src->objectCount;
    pObjects = nullptr;
    if (queueLabelCount && src->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i=0; i<queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&src->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && src->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i=0; i<cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&src->pCmdBufLabels[i]);
        }
    }
    if (objectCount && src->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i=0; i<objectCount; ++i) {
            pObjects[i].initialize(&src->pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT(const VkDebugUtilsMessengerCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    messageSeverity(in_struct->messageSeverity),
    messageType(in_struct->messageType),
    pfnUserCallback(in_struct->pfnUserCallback),
    pUserData(in_struct->pUserData)
{
}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT()
{}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT(const safe_VkDebugUtilsMessengerCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    messageSeverity = src.messageSeverity;
    messageType = src.messageType;
    pfnUserCallback = src.pfnUserCallback;
    pUserData = src.pUserData;
}

safe_VkDebugUtilsMessengerCreateInfoEXT& safe_VkDebugUtilsMessengerCreateInfoEXT::operator=(const safe_VkDebugUtilsMessengerCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    messageSeverity = src.messageSeverity;
    messageType = src.messageType;
    pfnUserCallback = src.pfnUserCallback;
    pUserData = src.pUserData;

    return *this;
}

safe_VkDebugUtilsMessengerCreateInfoEXT::~safe_VkDebugUtilsMessengerCreateInfoEXT()
{
}

void safe_VkDebugUtilsMessengerCreateInfoEXT::initialize(const VkDebugUtilsMessengerCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    messageSeverity = in_struct->messageSeverity;
    messageType = in_struct->messageType;
    pfnUserCallback = in_struct->pfnUserCallback;
    pUserData = in_struct->pUserData;
}

void safe_VkDebugUtilsMessengerCreateInfoEXT::initialize(const safe_VkDebugUtilsMessengerCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    messageSeverity = src->messageSeverity;
    messageType = src->messageType;
    pfnUserCallback = src->pfnUserCallback;
    pUserData = src->pUserData;
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID(const VkAndroidHardwareBufferUsageANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    androidHardwareBufferUsage(in_struct->androidHardwareBufferUsage)
{
}

safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID()
{}

safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID(const safe_VkAndroidHardwareBufferUsageANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    androidHardwareBufferUsage = src.androidHardwareBufferUsage;
}

safe_VkAndroidHardwareBufferUsageANDROID& safe_VkAndroidHardwareBufferUsageANDROID::operator=(const safe_VkAndroidHardwareBufferUsageANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    androidHardwareBufferUsage = src.androidHardwareBufferUsage;

    return *this;
}

safe_VkAndroidHardwareBufferUsageANDROID::~safe_VkAndroidHardwareBufferUsageANDROID()
{
}

void safe_VkAndroidHardwareBufferUsageANDROID::initialize(const VkAndroidHardwareBufferUsageANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    androidHardwareBufferUsage = in_struct->androidHardwareBufferUsage;
}

void safe_VkAndroidHardwareBufferUsageANDROID::initialize(const safe_VkAndroidHardwareBufferUsageANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    androidHardwareBufferUsage = src->androidHardwareBufferUsage;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID(const VkAndroidHardwareBufferPropertiesANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    allocationSize(in_struct->allocationSize),
    memoryTypeBits(in_struct->memoryTypeBits)
{
}

safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID()
{}

safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID(const safe_VkAndroidHardwareBufferPropertiesANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    allocationSize = src.allocationSize;
    memoryTypeBits = src.memoryTypeBits;
}

safe_VkAndroidHardwareBufferPropertiesANDROID& safe_VkAndroidHardwareBufferPropertiesANDROID::operator=(const safe_VkAndroidHardwareBufferPropertiesANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    allocationSize = src.allocationSize;
    memoryTypeBits = src.memoryTypeBits;

    return *this;
}

safe_VkAndroidHardwareBufferPropertiesANDROID::~safe_VkAndroidHardwareBufferPropertiesANDROID()
{
}

void safe_VkAndroidHardwareBufferPropertiesANDROID::initialize(const VkAndroidHardwareBufferPropertiesANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    allocationSize = in_struct->allocationSize;
    memoryTypeBits = in_struct->memoryTypeBits;
}

void safe_VkAndroidHardwareBufferPropertiesANDROID::initialize(const safe_VkAndroidHardwareBufferPropertiesANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    allocationSize = src->allocationSize;
    memoryTypeBits = src->memoryTypeBits;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID(const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    format(in_struct->format),
    externalFormat(in_struct->externalFormat),
    formatFeatures(in_struct->formatFeatures),
    samplerYcbcrConversionComponents(in_struct->samplerYcbcrConversionComponents),
    suggestedYcbcrModel(in_struct->suggestedYcbcrModel),
    suggestedYcbcrRange(in_struct->suggestedYcbcrRange),
    suggestedXChromaOffset(in_struct->suggestedXChromaOffset),
    suggestedYChromaOffset(in_struct->suggestedYChromaOffset)
{
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID()
{}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    externalFormat = src.externalFormat;
    formatFeatures = src.formatFeatures;
    samplerYcbcrConversionComponents = src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = src.suggestedYcbcrModel;
    suggestedYcbcrRange = src.suggestedYcbcrRange;
    suggestedXChromaOffset = src.suggestedXChromaOffset;
    suggestedYChromaOffset = src.suggestedYChromaOffset;
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID& safe_VkAndroidHardwareBufferFormatPropertiesANDROID::operator=(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    format = src.format;
    externalFormat = src.externalFormat;
    formatFeatures = src.formatFeatures;
    samplerYcbcrConversionComponents = src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = src.suggestedYcbcrModel;
    suggestedYcbcrRange = src.suggestedYcbcrRange;
    suggestedXChromaOffset = src.suggestedXChromaOffset;
    suggestedYChromaOffset = src.suggestedYChromaOffset;

    return *this;
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::~safe_VkAndroidHardwareBufferFormatPropertiesANDROID()
{
}

void safe_VkAndroidHardwareBufferFormatPropertiesANDROID::initialize(const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    format = in_struct->format;
    externalFormat = in_struct->externalFormat;
    formatFeatures = in_struct->formatFeatures;
    samplerYcbcrConversionComponents = in_struct->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = in_struct->suggestedYcbcrModel;
    suggestedYcbcrRange = in_struct->suggestedYcbcrRange;
    suggestedXChromaOffset = in_struct->suggestedXChromaOffset;
    suggestedYChromaOffset = in_struct->suggestedYChromaOffset;
}

void safe_VkAndroidHardwareBufferFormatPropertiesANDROID::initialize(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    format = src->format;
    externalFormat = src->externalFormat;
    formatFeatures = src->formatFeatures;
    samplerYcbcrConversionComponents = src->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = src->suggestedYcbcrModel;
    suggestedYcbcrRange = src->suggestedYcbcrRange;
    suggestedXChromaOffset = src->suggestedXChromaOffset;
    suggestedYChromaOffset = src->suggestedYChromaOffset;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID(const VkImportAndroidHardwareBufferInfoANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    buffer(nullptr)
{
    buffer = in_struct->buffer;
}

safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID() :
    buffer(nullptr)
{}

safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID(const safe_VkImportAndroidHardwareBufferInfoANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
}

safe_VkImportAndroidHardwareBufferInfoANDROID& safe_VkImportAndroidHardwareBufferInfoANDROID::operator=(const safe_VkImportAndroidHardwareBufferInfoANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;

    return *this;
}

safe_VkImportAndroidHardwareBufferInfoANDROID::~safe_VkImportAndroidHardwareBufferInfoANDROID()
{
}

void safe_VkImportAndroidHardwareBufferInfoANDROID::initialize(const VkImportAndroidHardwareBufferInfoANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    buffer = in_struct->buffer;
}

void safe_VkImportAndroidHardwareBufferInfoANDROID::initialize(const safe_VkImportAndroidHardwareBufferInfoANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    buffer = src->buffer;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memory(in_struct->memory)
{
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID()
{}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::operator=(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memory = src.memory;

    return *this;
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::~safe_VkMemoryGetAndroidHardwareBufferInfoANDROID()
{
}

void safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::initialize(const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memory = in_struct->memory;
}

void safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::initialize(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memory = src->memory;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR


safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID(const VkExternalFormatANDROID* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    externalFormat(in_struct->externalFormat)
{
}

safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID()
{}

safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID(const safe_VkExternalFormatANDROID& src)
{
    sType = src.sType;
    pNext = src.pNext;
    externalFormat = src.externalFormat;
}

safe_VkExternalFormatANDROID& safe_VkExternalFormatANDROID::operator=(const safe_VkExternalFormatANDROID& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    externalFormat = src.externalFormat;

    return *this;
}

safe_VkExternalFormatANDROID::~safe_VkExternalFormatANDROID()
{
}

void safe_VkExternalFormatANDROID::initialize(const VkExternalFormatANDROID* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    externalFormat = in_struct->externalFormat;
}

void safe_VkExternalFormatANDROID::initialize(const safe_VkExternalFormatANDROID* src)
{
    sType = src->sType;
    pNext = src->pNext;
    externalFormat = src->externalFormat;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR


safe_VkSamplerReductionModeCreateInfoEXT::safe_VkSamplerReductionModeCreateInfoEXT(const VkSamplerReductionModeCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    reductionMode(in_struct->reductionMode)
{
}

safe_VkSamplerReductionModeCreateInfoEXT::safe_VkSamplerReductionModeCreateInfoEXT()
{}

safe_VkSamplerReductionModeCreateInfoEXT::safe_VkSamplerReductionModeCreateInfoEXT(const safe_VkSamplerReductionModeCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    reductionMode = src.reductionMode;
}

safe_VkSamplerReductionModeCreateInfoEXT& safe_VkSamplerReductionModeCreateInfoEXT::operator=(const safe_VkSamplerReductionModeCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    reductionMode = src.reductionMode;

    return *this;
}

safe_VkSamplerReductionModeCreateInfoEXT::~safe_VkSamplerReductionModeCreateInfoEXT()
{
}

void safe_VkSamplerReductionModeCreateInfoEXT::initialize(const VkSamplerReductionModeCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    reductionMode = in_struct->reductionMode;
}

void safe_VkSamplerReductionModeCreateInfoEXT::initialize(const safe_VkSamplerReductionModeCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    reductionMode = src->reductionMode;
}

safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT(const VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    filterMinmaxSingleComponentFormats(in_struct->filterMinmaxSingleComponentFormats),
    filterMinmaxImageComponentMapping(in_struct->filterMinmaxImageComponentMapping)
{
}

safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT()
{}

safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    filterMinmaxSingleComponentFormats = src.filterMinmaxSingleComponentFormats;
    filterMinmaxImageComponentMapping = src.filterMinmaxImageComponentMapping;
}

safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::operator=(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    filterMinmaxSingleComponentFormats = src.filterMinmaxSingleComponentFormats;
    filterMinmaxImageComponentMapping = src.filterMinmaxImageComponentMapping;

    return *this;
}

safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::~safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT()
{
}

void safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::initialize(const VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    filterMinmaxSingleComponentFormats = in_struct->filterMinmaxSingleComponentFormats;
    filterMinmaxImageComponentMapping = in_struct->filterMinmaxImageComponentMapping;
}

void safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT::initialize(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    filterMinmaxSingleComponentFormats = src->filterMinmaxSingleComponentFormats;
    filterMinmaxImageComponentMapping = src->filterMinmaxImageComponentMapping;
}

safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT(const VkPhysicalDeviceInlineUniformBlockFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    inlineUniformBlock(in_struct->inlineUniformBlock),
    descriptorBindingInlineUniformBlockUpdateAfterBind(in_struct->descriptorBindingInlineUniformBlockUpdateAfterBind)
{
}

safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT()
{}

safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    inlineUniformBlock = src.inlineUniformBlock;
    descriptorBindingInlineUniformBlockUpdateAfterBind = src.descriptorBindingInlineUniformBlockUpdateAfterBind;
}

safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::operator=(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    inlineUniformBlock = src.inlineUniformBlock;
    descriptorBindingInlineUniformBlockUpdateAfterBind = src.descriptorBindingInlineUniformBlockUpdateAfterBind;

    return *this;
}

safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::~safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT()
{
}

void safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::initialize(const VkPhysicalDeviceInlineUniformBlockFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    inlineUniformBlock = in_struct->inlineUniformBlock;
    descriptorBindingInlineUniformBlockUpdateAfterBind = in_struct->descriptorBindingInlineUniformBlockUpdateAfterBind;
}

void safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT::initialize(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    inlineUniformBlock = src->inlineUniformBlock;
    descriptorBindingInlineUniformBlockUpdateAfterBind = src->descriptorBindingInlineUniformBlockUpdateAfterBind;
}

safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT(const VkPhysicalDeviceInlineUniformBlockPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxInlineUniformBlockSize(in_struct->maxInlineUniformBlockSize),
    maxPerStageDescriptorInlineUniformBlocks(in_struct->maxPerStageDescriptorInlineUniformBlocks),
    maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks(in_struct->maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks),
    maxDescriptorSetInlineUniformBlocks(in_struct->maxDescriptorSetInlineUniformBlocks),
    maxDescriptorSetUpdateAfterBindInlineUniformBlocks(in_struct->maxDescriptorSetUpdateAfterBindInlineUniformBlocks)
{
}

safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT()
{}

safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxInlineUniformBlockSize = src.maxInlineUniformBlockSize;
    maxPerStageDescriptorInlineUniformBlocks = src.maxPerStageDescriptorInlineUniformBlocks;
    maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = src.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks;
    maxDescriptorSetInlineUniformBlocks = src.maxDescriptorSetInlineUniformBlocks;
    maxDescriptorSetUpdateAfterBindInlineUniformBlocks = src.maxDescriptorSetUpdateAfterBindInlineUniformBlocks;
}

safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::operator=(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxInlineUniformBlockSize = src.maxInlineUniformBlockSize;
    maxPerStageDescriptorInlineUniformBlocks = src.maxPerStageDescriptorInlineUniformBlocks;
    maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = src.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks;
    maxDescriptorSetInlineUniformBlocks = src.maxDescriptorSetInlineUniformBlocks;
    maxDescriptorSetUpdateAfterBindInlineUniformBlocks = src.maxDescriptorSetUpdateAfterBindInlineUniformBlocks;

    return *this;
}

safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::~safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT()
{
}

void safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::initialize(const VkPhysicalDeviceInlineUniformBlockPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxInlineUniformBlockSize = in_struct->maxInlineUniformBlockSize;
    maxPerStageDescriptorInlineUniformBlocks = in_struct->maxPerStageDescriptorInlineUniformBlocks;
    maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = in_struct->maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks;
    maxDescriptorSetInlineUniformBlocks = in_struct->maxDescriptorSetInlineUniformBlocks;
    maxDescriptorSetUpdateAfterBindInlineUniformBlocks = in_struct->maxDescriptorSetUpdateAfterBindInlineUniformBlocks;
}

void safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT::initialize(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxInlineUniformBlockSize = src->maxInlineUniformBlockSize;
    maxPerStageDescriptorInlineUniformBlocks = src->maxPerStageDescriptorInlineUniformBlocks;
    maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = src->maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks;
    maxDescriptorSetInlineUniformBlocks = src->maxDescriptorSetInlineUniformBlocks;
    maxDescriptorSetUpdateAfterBindInlineUniformBlocks = src->maxDescriptorSetUpdateAfterBindInlineUniformBlocks;
}

safe_VkWriteDescriptorSetInlineUniformBlockEXT::safe_VkWriteDescriptorSetInlineUniformBlockEXT(const VkWriteDescriptorSetInlineUniformBlockEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    dataSize(in_struct->dataSize),
    pData(in_struct->pData)
{
}

safe_VkWriteDescriptorSetInlineUniformBlockEXT::safe_VkWriteDescriptorSetInlineUniformBlockEXT()
{}

safe_VkWriteDescriptorSetInlineUniformBlockEXT::safe_VkWriteDescriptorSetInlineUniformBlockEXT(const safe_VkWriteDescriptorSetInlineUniformBlockEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    dataSize = src.dataSize;
    pData = src.pData;
}

safe_VkWriteDescriptorSetInlineUniformBlockEXT& safe_VkWriteDescriptorSetInlineUniformBlockEXT::operator=(const safe_VkWriteDescriptorSetInlineUniformBlockEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    dataSize = src.dataSize;
    pData = src.pData;

    return *this;
}

safe_VkWriteDescriptorSetInlineUniformBlockEXT::~safe_VkWriteDescriptorSetInlineUniformBlockEXT()
{
}

void safe_VkWriteDescriptorSetInlineUniformBlockEXT::initialize(const VkWriteDescriptorSetInlineUniformBlockEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    dataSize = in_struct->dataSize;
    pData = in_struct->pData;
}

void safe_VkWriteDescriptorSetInlineUniformBlockEXT::initialize(const safe_VkWriteDescriptorSetInlineUniformBlockEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    dataSize = src->dataSize;
    pData = src->pData;
}

safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT(const VkDescriptorPoolInlineUniformBlockCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxInlineUniformBlockBindings(in_struct->maxInlineUniformBlockBindings)
{
}

safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT()
{}

safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxInlineUniformBlockBindings = src.maxInlineUniformBlockBindings;
}

safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::operator=(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxInlineUniformBlockBindings = src.maxInlineUniformBlockBindings;

    return *this;
}

safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::~safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT()
{
}

void safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::initialize(const VkDescriptorPoolInlineUniformBlockCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxInlineUniformBlockBindings = in_struct->maxInlineUniformBlockBindings;
}

void safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT::initialize(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxInlineUniformBlockBindings = src->maxInlineUniformBlockBindings;
}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT(const VkSampleLocationsInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    sampleLocationsPerPixel(in_struct->sampleLocationsPerPixel),
    sampleLocationGridSize(in_struct->sampleLocationGridSize),
    sampleLocationsCount(in_struct->sampleLocationsCount),
    pSampleLocations(nullptr)
{
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[in_struct->sampleLocationsCount];
        memcpy ((void *)pSampleLocations, (void *)in_struct->pSampleLocations, sizeof(VkSampleLocationEXT)*in_struct->sampleLocationsCount);
    }
}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT() :
    pSampleLocations(nullptr)
{}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT(const safe_VkSampleLocationsInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    sampleLocationsPerPixel = src.sampleLocationsPerPixel;
    sampleLocationGridSize = src.sampleLocationGridSize;
    sampleLocationsCount = src.sampleLocationsCount;
    pSampleLocations = nullptr;
    if (src.pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[src.sampleLocationsCount];
        memcpy ((void *)pSampleLocations, (void *)src.pSampleLocations, sizeof(VkSampleLocationEXT)*src.sampleLocationsCount);
    }
}

safe_VkSampleLocationsInfoEXT& safe_VkSampleLocationsInfoEXT::operator=(const safe_VkSampleLocationsInfoEXT& src)
{
    if (&src == this) return *this;

    if (pSampleLocations)
        delete[] pSampleLocations;

    sType = src.sType;
    pNext = src.pNext;
    sampleLocationsPerPixel = src.sampleLocationsPerPixel;
    sampleLocationGridSize = src.sampleLocationGridSize;
    sampleLocationsCount = src.sampleLocationsCount;
    pSampleLocations = nullptr;
    if (src.pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[src.sampleLocationsCount];
        memcpy ((void *)pSampleLocations, (void *)src.pSampleLocations, sizeof(VkSampleLocationEXT)*src.sampleLocationsCount);
    }

    return *this;
}

safe_VkSampleLocationsInfoEXT::~safe_VkSampleLocationsInfoEXT()
{
    if (pSampleLocations)
        delete[] pSampleLocations;
}

void safe_VkSampleLocationsInfoEXT::initialize(const VkSampleLocationsInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    sampleLocationsPerPixel = in_struct->sampleLocationsPerPixel;
    sampleLocationGridSize = in_struct->sampleLocationGridSize;
    sampleLocationsCount = in_struct->sampleLocationsCount;
    pSampleLocations = nullptr;
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[in_struct->sampleLocationsCount];
        memcpy ((void *)pSampleLocations, (void *)in_struct->pSampleLocations, sizeof(VkSampleLocationEXT)*in_struct->sampleLocationsCount);
    }
}

void safe_VkSampleLocationsInfoEXT::initialize(const safe_VkSampleLocationsInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    sampleLocationsPerPixel = src->sampleLocationsPerPixel;
    sampleLocationGridSize = src->sampleLocationGridSize;
    sampleLocationsCount = src->sampleLocationsCount;
    pSampleLocations = nullptr;
    if (src->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[src->sampleLocationsCount];
        memcpy ((void *)pSampleLocations, (void *)src->pSampleLocations, sizeof(VkSampleLocationEXT)*src->sampleLocationsCount);
    }
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    attachmentInitialSampleLocationsCount(in_struct->attachmentInitialSampleLocationsCount),
    pAttachmentInitialSampleLocations(nullptr),
    postSubpassSampleLocationsCount(in_struct->postSubpassSampleLocationsCount),
    pPostSubpassSampleLocations(nullptr)
{
    if (in_struct->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new VkAttachmentSampleLocationsEXT[in_struct->attachmentInitialSampleLocationsCount];
        memcpy ((void *)pAttachmentInitialSampleLocations, (void *)in_struct->pAttachmentInitialSampleLocations, sizeof(VkAttachmentSampleLocationsEXT)*in_struct->attachmentInitialSampleLocationsCount);
    }
    if (in_struct->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new VkSubpassSampleLocationsEXT[in_struct->postSubpassSampleLocationsCount];
        memcpy ((void *)pPostSubpassSampleLocations, (void *)in_struct->pPostSubpassSampleLocations, sizeof(VkSubpassSampleLocationsEXT)*in_struct->postSubpassSampleLocationsCount);
    }
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT() :
    pAttachmentInitialSampleLocations(nullptr),
    pPostSubpassSampleLocations(nullptr)
{}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT(const safe_VkRenderPassSampleLocationsBeginInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    attachmentInitialSampleLocationsCount = src.attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = src.postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    if (src.pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new VkAttachmentSampleLocationsEXT[src.attachmentInitialSampleLocationsCount];
        memcpy ((void *)pAttachmentInitialSampleLocations, (void *)src.pAttachmentInitialSampleLocations, sizeof(VkAttachmentSampleLocationsEXT)*src.attachmentInitialSampleLocationsCount);
    }
    if (src.pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new VkSubpassSampleLocationsEXT[src.postSubpassSampleLocationsCount];
        memcpy ((void *)pPostSubpassSampleLocations, (void *)src.pPostSubpassSampleLocations, sizeof(VkSubpassSampleLocationsEXT)*src.postSubpassSampleLocationsCount);
    }
}

safe_VkRenderPassSampleLocationsBeginInfoEXT& safe_VkRenderPassSampleLocationsBeginInfoEXT::operator=(const safe_VkRenderPassSampleLocationsBeginInfoEXT& src)
{
    if (&src == this) return *this;

    if (pAttachmentInitialSampleLocations)
        delete[] pAttachmentInitialSampleLocations;
    if (pPostSubpassSampleLocations)
        delete[] pPostSubpassSampleLocations;

    sType = src.sType;
    pNext = src.pNext;
    attachmentInitialSampleLocationsCount = src.attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = src.postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    if (src.pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new VkAttachmentSampleLocationsEXT[src.attachmentInitialSampleLocationsCount];
        memcpy ((void *)pAttachmentInitialSampleLocations, (void *)src.pAttachmentInitialSampleLocations, sizeof(VkAttachmentSampleLocationsEXT)*src.attachmentInitialSampleLocationsCount);
    }
    if (src.pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new VkSubpassSampleLocationsEXT[src.postSubpassSampleLocationsCount];
        memcpy ((void *)pPostSubpassSampleLocations, (void *)src.pPostSubpassSampleLocations, sizeof(VkSubpassSampleLocationsEXT)*src.postSubpassSampleLocationsCount);
    }

    return *this;
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::~safe_VkRenderPassSampleLocationsBeginInfoEXT()
{
    if (pAttachmentInitialSampleLocations)
        delete[] pAttachmentInitialSampleLocations;
    if (pPostSubpassSampleLocations)
        delete[] pPostSubpassSampleLocations;
}

void safe_VkRenderPassSampleLocationsBeginInfoEXT::initialize(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    attachmentInitialSampleLocationsCount = in_struct->attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = in_struct->postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    if (in_struct->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new VkAttachmentSampleLocationsEXT[in_struct->attachmentInitialSampleLocationsCount];
        memcpy ((void *)pAttachmentInitialSampleLocations, (void *)in_struct->pAttachmentInitialSampleLocations, sizeof(VkAttachmentSampleLocationsEXT)*in_struct->attachmentInitialSampleLocationsCount);
    }
    if (in_struct->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new VkSubpassSampleLocationsEXT[in_struct->postSubpassSampleLocationsCount];
        memcpy ((void *)pPostSubpassSampleLocations, (void *)in_struct->pPostSubpassSampleLocations, sizeof(VkSubpassSampleLocationsEXT)*in_struct->postSubpassSampleLocationsCount);
    }
}

void safe_VkRenderPassSampleLocationsBeginInfoEXT::initialize(const safe_VkRenderPassSampleLocationsBeginInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    attachmentInitialSampleLocationsCount = src->attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = src->postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    if (src->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new VkAttachmentSampleLocationsEXT[src->attachmentInitialSampleLocationsCount];
        memcpy ((void *)pAttachmentInitialSampleLocations, (void *)src->pAttachmentInitialSampleLocations, sizeof(VkAttachmentSampleLocationsEXT)*src->attachmentInitialSampleLocationsCount);
    }
    if (src->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new VkSubpassSampleLocationsEXT[src->postSubpassSampleLocationsCount];
        memcpy ((void *)pPostSubpassSampleLocations, (void *)src->pPostSubpassSampleLocations, sizeof(VkSubpassSampleLocationsEXT)*src->postSubpassSampleLocationsCount);
    }
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    sampleLocationsEnable(in_struct->sampleLocationsEnable),
    sampleLocationsInfo(&in_struct->sampleLocationsInfo)
{
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT()
{}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    sampleLocationsEnable = src.sampleLocationsEnable;
    sampleLocationsInfo.initialize(&src.sampleLocationsInfo);
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT& safe_VkPipelineSampleLocationsStateCreateInfoEXT::operator=(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    sampleLocationsEnable = src.sampleLocationsEnable;
    sampleLocationsInfo.initialize(&src.sampleLocationsInfo);

    return *this;
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::~safe_VkPipelineSampleLocationsStateCreateInfoEXT()
{
}

void safe_VkPipelineSampleLocationsStateCreateInfoEXT::initialize(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    sampleLocationsEnable = in_struct->sampleLocationsEnable;
    sampleLocationsInfo.initialize(&in_struct->sampleLocationsInfo);
}

void safe_VkPipelineSampleLocationsStateCreateInfoEXT::initialize(const safe_VkPipelineSampleLocationsStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    sampleLocationsEnable = src->sampleLocationsEnable;
    sampleLocationsInfo.initialize(&src->sampleLocationsInfo);
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    sampleLocationSampleCounts(in_struct->sampleLocationSampleCounts),
    maxSampleLocationGridSize(in_struct->maxSampleLocationGridSize),
    sampleLocationSubPixelBits(in_struct->sampleLocationSubPixelBits),
    variableSampleLocations(in_struct->variableSampleLocations)
{
    for (uint32_t i=0; i<2; ++i) {
        sampleLocationCoordinateRange[i] = in_struct->sampleLocationCoordinateRange[i];
    }
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT()
{}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    sampleLocationSampleCounts = src.sampleLocationSampleCounts;
    maxSampleLocationGridSize = src.maxSampleLocationGridSize;
    sampleLocationSubPixelBits = src.sampleLocationSubPixelBits;
    variableSampleLocations = src.variableSampleLocations;
    for (uint32_t i=0; i<2; ++i) {
        sampleLocationCoordinateRange[i] = src.sampleLocationCoordinateRange[i];
    }
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::operator=(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    sampleLocationSampleCounts = src.sampleLocationSampleCounts;
    maxSampleLocationGridSize = src.maxSampleLocationGridSize;
    sampleLocationSubPixelBits = src.sampleLocationSubPixelBits;
    variableSampleLocations = src.variableSampleLocations;
    for (uint32_t i=0; i<2; ++i) {
        sampleLocationCoordinateRange[i] = src.sampleLocationCoordinateRange[i];
    }

    return *this;
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::~safe_VkPhysicalDeviceSampleLocationsPropertiesEXT()
{
}

void safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::initialize(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    sampleLocationSampleCounts = in_struct->sampleLocationSampleCounts;
    maxSampleLocationGridSize = in_struct->maxSampleLocationGridSize;
    sampleLocationSubPixelBits = in_struct->sampleLocationSubPixelBits;
    variableSampleLocations = in_struct->variableSampleLocations;
    for (uint32_t i=0; i<2; ++i) {
        sampleLocationCoordinateRange[i] = in_struct->sampleLocationCoordinateRange[i];
    }
}

void safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::initialize(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    sampleLocationSampleCounts = src->sampleLocationSampleCounts;
    maxSampleLocationGridSize = src->maxSampleLocationGridSize;
    sampleLocationSubPixelBits = src->sampleLocationSubPixelBits;
    variableSampleLocations = src->variableSampleLocations;
    for (uint32_t i=0; i<2; ++i) {
        sampleLocationCoordinateRange[i] = src->sampleLocationCoordinateRange[i];
    }
}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT(const VkMultisamplePropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxSampleLocationGridSize(in_struct->maxSampleLocationGridSize)
{
}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT()
{}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT(const safe_VkMultisamplePropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxSampleLocationGridSize = src.maxSampleLocationGridSize;
}

safe_VkMultisamplePropertiesEXT& safe_VkMultisamplePropertiesEXT::operator=(const safe_VkMultisamplePropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxSampleLocationGridSize = src.maxSampleLocationGridSize;

    return *this;
}

safe_VkMultisamplePropertiesEXT::~safe_VkMultisamplePropertiesEXT()
{
}

void safe_VkMultisamplePropertiesEXT::initialize(const VkMultisamplePropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxSampleLocationGridSize = in_struct->maxSampleLocationGridSize;
}

void safe_VkMultisamplePropertiesEXT::initialize(const safe_VkMultisamplePropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxSampleLocationGridSize = src->maxSampleLocationGridSize;
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    advancedBlendCoherentOperations(in_struct->advancedBlendCoherentOperations)
{
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT()
{}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    advancedBlendCoherentOperations = src.advancedBlendCoherentOperations;
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    advancedBlendCoherentOperations = src.advancedBlendCoherentOperations;

    return *this;
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::~safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT()
{
}

void safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::initialize(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    advancedBlendCoherentOperations = in_struct->advancedBlendCoherentOperations;
}

void safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    advancedBlendCoherentOperations = src->advancedBlendCoherentOperations;
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    advancedBlendMaxColorAttachments(in_struct->advancedBlendMaxColorAttachments),
    advancedBlendIndependentBlend(in_struct->advancedBlendIndependentBlend),
    advancedBlendNonPremultipliedSrcColor(in_struct->advancedBlendNonPremultipliedSrcColor),
    advancedBlendNonPremultipliedDstColor(in_struct->advancedBlendNonPremultipliedDstColor),
    advancedBlendCorrelatedOverlap(in_struct->advancedBlendCorrelatedOverlap),
    advancedBlendAllOperations(in_struct->advancedBlendAllOperations)
{
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT()
{}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    advancedBlendMaxColorAttachments = src.advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = src.advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = src.advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = src.advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = src.advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = src.advancedBlendAllOperations;
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    advancedBlendMaxColorAttachments = src.advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = src.advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = src.advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = src.advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = src.advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = src.advancedBlendAllOperations;

    return *this;
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::~safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT()
{
}

void safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::initialize(const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    advancedBlendMaxColorAttachments = in_struct->advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = in_struct->advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = in_struct->advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = in_struct->advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = in_struct->advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = in_struct->advancedBlendAllOperations;
}

void safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    advancedBlendMaxColorAttachments = src->advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = src->advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = src->advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = src->advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = src->advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = src->advancedBlendAllOperations;
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    srcPremultiplied(in_struct->srcPremultiplied),
    dstPremultiplied(in_struct->dstPremultiplied),
    blendOverlap(in_struct->blendOverlap)
{
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT()
{}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    srcPremultiplied = src.srcPremultiplied;
    dstPremultiplied = src.dstPremultiplied;
    blendOverlap = src.blendOverlap;
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::operator=(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    srcPremultiplied = src.srcPremultiplied;
    dstPremultiplied = src.dstPremultiplied;
    blendOverlap = src.blendOverlap;

    return *this;
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::~safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT()
{
}

void safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::initialize(const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    srcPremultiplied = in_struct->srcPremultiplied;
    dstPremultiplied = in_struct->dstPremultiplied;
    blendOverlap = in_struct->blendOverlap;
}

void safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::initialize(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    srcPremultiplied = src->srcPremultiplied;
    dstPremultiplied = src->dstPremultiplied;
    blendOverlap = src->blendOverlap;
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV(const VkPipelineCoverageToColorStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    coverageToColorEnable(in_struct->coverageToColorEnable),
    coverageToColorLocation(in_struct->coverageToColorLocation)
{
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV()
{}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV(const safe_VkPipelineCoverageToColorStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageToColorEnable = src.coverageToColorEnable;
    coverageToColorLocation = src.coverageToColorLocation;
}

safe_VkPipelineCoverageToColorStateCreateInfoNV& safe_VkPipelineCoverageToColorStateCreateInfoNV::operator=(const safe_VkPipelineCoverageToColorStateCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageToColorEnable = src.coverageToColorEnable;
    coverageToColorLocation = src.coverageToColorLocation;

    return *this;
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::~safe_VkPipelineCoverageToColorStateCreateInfoNV()
{
}

void safe_VkPipelineCoverageToColorStateCreateInfoNV::initialize(const VkPipelineCoverageToColorStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    coverageToColorEnable = in_struct->coverageToColorEnable;
    coverageToColorLocation = in_struct->coverageToColorLocation;
}

void safe_VkPipelineCoverageToColorStateCreateInfoNV::initialize(const safe_VkPipelineCoverageToColorStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    coverageToColorEnable = src->coverageToColorEnable;
    coverageToColorLocation = src->coverageToColorLocation;
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV(const VkPipelineCoverageModulationStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    coverageModulationMode(in_struct->coverageModulationMode),
    coverageModulationTableEnable(in_struct->coverageModulationTableEnable),
    coverageModulationTableCount(in_struct->coverageModulationTableCount),
    pCoverageModulationTable(nullptr)
{
    if (in_struct->pCoverageModulationTable) {
        pCoverageModulationTable = new float[in_struct->coverageModulationTableCount];
        memcpy ((void *)pCoverageModulationTable, (void *)in_struct->pCoverageModulationTable, sizeof(float)*in_struct->coverageModulationTableCount);
    }
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV() :
    pCoverageModulationTable(nullptr)
{}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV(const safe_VkPipelineCoverageModulationStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageModulationMode = src.coverageModulationMode;
    coverageModulationTableEnable = src.coverageModulationTableEnable;
    coverageModulationTableCount = src.coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    if (src.pCoverageModulationTable) {
        pCoverageModulationTable = new float[src.coverageModulationTableCount];
        memcpy ((void *)pCoverageModulationTable, (void *)src.pCoverageModulationTable, sizeof(float)*src.coverageModulationTableCount);
    }
}

safe_VkPipelineCoverageModulationStateCreateInfoNV& safe_VkPipelineCoverageModulationStateCreateInfoNV::operator=(const safe_VkPipelineCoverageModulationStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pCoverageModulationTable)
        delete[] pCoverageModulationTable;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageModulationMode = src.coverageModulationMode;
    coverageModulationTableEnable = src.coverageModulationTableEnable;
    coverageModulationTableCount = src.coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    if (src.pCoverageModulationTable) {
        pCoverageModulationTable = new float[src.coverageModulationTableCount];
        memcpy ((void *)pCoverageModulationTable, (void *)src.pCoverageModulationTable, sizeof(float)*src.coverageModulationTableCount);
    }

    return *this;
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::~safe_VkPipelineCoverageModulationStateCreateInfoNV()
{
    if (pCoverageModulationTable)
        delete[] pCoverageModulationTable;
}

void safe_VkPipelineCoverageModulationStateCreateInfoNV::initialize(const VkPipelineCoverageModulationStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    coverageModulationMode = in_struct->coverageModulationMode;
    coverageModulationTableEnable = in_struct->coverageModulationTableEnable;
    coverageModulationTableCount = in_struct->coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    if (in_struct->pCoverageModulationTable) {
        pCoverageModulationTable = new float[in_struct->coverageModulationTableCount];
        memcpy ((void *)pCoverageModulationTable, (void *)in_struct->pCoverageModulationTable, sizeof(float)*in_struct->coverageModulationTableCount);
    }
}

void safe_VkPipelineCoverageModulationStateCreateInfoNV::initialize(const safe_VkPipelineCoverageModulationStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    coverageModulationMode = src->coverageModulationMode;
    coverageModulationTableEnable = src->coverageModulationTableEnable;
    coverageModulationTableCount = src->coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    if (src->pCoverageModulationTable) {
        pCoverageModulationTable = new float[src->coverageModulationTableCount];
        memcpy ((void *)pCoverageModulationTable, (void *)src->pCoverageModulationTable, sizeof(float)*src->coverageModulationTableCount);
    }
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderSMCount(in_struct->shaderSMCount),
    shaderWarpsPerSM(in_struct->shaderWarpsPerSM)
{
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV()
{}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderSMCount = src.shaderSMCount;
    shaderWarpsPerSM = src.shaderWarpsPerSM;
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::operator=(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderSMCount = src.shaderSMCount;
    shaderWarpsPerSM = src.shaderWarpsPerSM;

    return *this;
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::~safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV()
{
}

void safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::initialize(const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderSMCount = in_struct->shaderSMCount;
    shaderWarpsPerSM = in_struct->shaderWarpsPerSM;
}

void safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::initialize(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderSMCount = src->shaderSMCount;
    shaderWarpsPerSM = src->shaderWarpsPerSM;
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderSMBuiltins(in_struct->shaderSMBuiltins)
{
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV()
{}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderSMBuiltins = src.shaderSMBuiltins;
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::operator=(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderSMBuiltins = src.shaderSMBuiltins;

    return *this;
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::~safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV()
{
}

void safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::initialize(const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderSMBuiltins = in_struct->shaderSMBuiltins;
}

void safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::initialize(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderSMBuiltins = src->shaderSMBuiltins;
}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT(const VkDrmFormatModifierPropertiesListEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    drmFormatModifierCount(in_struct->drmFormatModifierCount),
    pDrmFormatModifierProperties(nullptr)
{
    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[in_struct->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifierProperties, (void *)in_struct->pDrmFormatModifierProperties, sizeof(VkDrmFormatModifierPropertiesEXT)*in_struct->drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT() :
    pDrmFormatModifierProperties(nullptr)
{}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT(const safe_VkDrmFormatModifierPropertiesListEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifierCount = src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    if (src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[src.drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifierProperties, (void *)src.pDrmFormatModifierProperties, sizeof(VkDrmFormatModifierPropertiesEXT)*src.drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesListEXT& safe_VkDrmFormatModifierPropertiesListEXT::operator=(const safe_VkDrmFormatModifierPropertiesListEXT& src)
{
    if (&src == this) return *this;

    if (pDrmFormatModifierProperties)
        delete[] pDrmFormatModifierProperties;

    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifierCount = src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    if (src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[src.drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifierProperties, (void *)src.pDrmFormatModifierProperties, sizeof(VkDrmFormatModifierPropertiesEXT)*src.drmFormatModifierCount);
    }

    return *this;
}

safe_VkDrmFormatModifierPropertiesListEXT::~safe_VkDrmFormatModifierPropertiesListEXT()
{
    if (pDrmFormatModifierProperties)
        delete[] pDrmFormatModifierProperties;
}

void safe_VkDrmFormatModifierPropertiesListEXT::initialize(const VkDrmFormatModifierPropertiesListEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    drmFormatModifierCount = in_struct->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[in_struct->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifierProperties, (void *)in_struct->pDrmFormatModifierProperties, sizeof(VkDrmFormatModifierPropertiesEXT)*in_struct->drmFormatModifierCount);
    }
}

void safe_VkDrmFormatModifierPropertiesListEXT::initialize(const safe_VkDrmFormatModifierPropertiesListEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    drmFormatModifierCount = src->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    if (src->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[src->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifierProperties, (void *)src->pDrmFormatModifierProperties, sizeof(VkDrmFormatModifierPropertiesEXT)*src->drmFormatModifierCount);
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    drmFormatModifier(in_struct->drmFormatModifier),
    sharingMode(in_struct->sharingMode),
    queueFamilyIndexCount(in_struct->queueFamilyIndexCount),
    pQueueFamilyIndices(nullptr)
{
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT() :
    pQueueFamilyIndices(nullptr)
{}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::operator=(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& src)
{
    if (&src == this) return *this;

    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;

    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;
    sharingMode = src.sharingMode;
    queueFamilyIndexCount = src.queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src.queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src.pQueueFamilyIndices, sizeof(uint32_t)*src.queueFamilyIndexCount);
    }

    return *this;
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::~safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT()
{
    if (pQueueFamilyIndices)
        delete[] pQueueFamilyIndices;
}

void safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::initialize(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    drmFormatModifier = in_struct->drmFormatModifier;
    sharingMode = in_struct->sharingMode;
    queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)in_struct->pQueueFamilyIndices, sizeof(uint32_t)*in_struct->queueFamilyIndexCount);
    }
}

void safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::initialize(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    drmFormatModifier = src->drmFormatModifier;
    sharingMode = src->sharingMode;
    queueFamilyIndexCount = src->queueFamilyIndexCount;
    pQueueFamilyIndices = nullptr;
    if (src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[src->queueFamilyIndexCount];
        memcpy ((void *)pQueueFamilyIndices, (void *)src->pQueueFamilyIndices, sizeof(uint32_t)*src->queueFamilyIndexCount);
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    drmFormatModifierCount(in_struct->drmFormatModifierCount),
    pDrmFormatModifiers(nullptr)
{
    if (in_struct->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[in_struct->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifiers, (void *)in_struct->pDrmFormatModifiers, sizeof(uint64_t)*in_struct->drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT() :
    pDrmFormatModifiers(nullptr)
{}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT(const safe_VkImageDrmFormatModifierListCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifierCount = src.drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    if (src.pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[src.drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifiers, (void *)src.pDrmFormatModifiers, sizeof(uint64_t)*src.drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT& safe_VkImageDrmFormatModifierListCreateInfoEXT::operator=(const safe_VkImageDrmFormatModifierListCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pDrmFormatModifiers)
        delete[] pDrmFormatModifiers;

    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifierCount = src.drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    if (src.pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[src.drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifiers, (void *)src.pDrmFormatModifiers, sizeof(uint64_t)*src.drmFormatModifierCount);
    }

    return *this;
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::~safe_VkImageDrmFormatModifierListCreateInfoEXT()
{
    if (pDrmFormatModifiers)
        delete[] pDrmFormatModifiers;
}

void safe_VkImageDrmFormatModifierListCreateInfoEXT::initialize(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    drmFormatModifierCount = in_struct->drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    if (in_struct->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[in_struct->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifiers, (void *)in_struct->pDrmFormatModifiers, sizeof(uint64_t)*in_struct->drmFormatModifierCount);
    }
}

void safe_VkImageDrmFormatModifierListCreateInfoEXT::initialize(const safe_VkImageDrmFormatModifierListCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    drmFormatModifierCount = src->drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    if (src->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[src->drmFormatModifierCount];
        memcpy ((void *)pDrmFormatModifiers, (void *)src->pDrmFormatModifiers, sizeof(uint64_t)*src->drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    drmFormatModifier(in_struct->drmFormatModifier),
    drmFormatModifierPlaneCount(in_struct->drmFormatModifierPlaneCount),
    pPlaneLayouts(nullptr)
{
    if (in_struct->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[in_struct->drmFormatModifierPlaneCount];
        memcpy ((void *)pPlaneLayouts, (void *)in_struct->pPlaneLayouts, sizeof(VkSubresourceLayout)*in_struct->drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT() :
    pPlaneLayouts(nullptr)
{}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;
    drmFormatModifierPlaneCount = src.drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    if (src.pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[src.drmFormatModifierPlaneCount];
        memcpy ((void *)pPlaneLayouts, (void *)src.pPlaneLayouts, sizeof(VkSubresourceLayout)*src.drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::operator=(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pPlaneLayouts)
        delete[] pPlaneLayouts;

    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;
    drmFormatModifierPlaneCount = src.drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    if (src.pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[src.drmFormatModifierPlaneCount];
        memcpy ((void *)pPlaneLayouts, (void *)src.pPlaneLayouts, sizeof(VkSubresourceLayout)*src.drmFormatModifierPlaneCount);
    }

    return *this;
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::~safe_VkImageDrmFormatModifierExplicitCreateInfoEXT()
{
    if (pPlaneLayouts)
        delete[] pPlaneLayouts;
}

void safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::initialize(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    drmFormatModifier = in_struct->drmFormatModifier;
    drmFormatModifierPlaneCount = in_struct->drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    if (in_struct->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[in_struct->drmFormatModifierPlaneCount];
        memcpy ((void *)pPlaneLayouts, (void *)in_struct->pPlaneLayouts, sizeof(VkSubresourceLayout)*in_struct->drmFormatModifierPlaneCount);
    }
}

void safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::initialize(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    drmFormatModifier = src->drmFormatModifier;
    drmFormatModifierPlaneCount = src->drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    if (src->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[src->drmFormatModifierPlaneCount];
        memcpy ((void *)pPlaneLayouts, (void *)src->pPlaneLayouts, sizeof(VkSubresourceLayout)*src->drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT(const VkImageDrmFormatModifierPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    drmFormatModifier(in_struct->drmFormatModifier)
{
}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT()
{}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT(const safe_VkImageDrmFormatModifierPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;
}

safe_VkImageDrmFormatModifierPropertiesEXT& safe_VkImageDrmFormatModifierPropertiesEXT::operator=(const safe_VkImageDrmFormatModifierPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    drmFormatModifier = src.drmFormatModifier;

    return *this;
}

safe_VkImageDrmFormatModifierPropertiesEXT::~safe_VkImageDrmFormatModifierPropertiesEXT()
{
}

void safe_VkImageDrmFormatModifierPropertiesEXT::initialize(const VkImageDrmFormatModifierPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    drmFormatModifier = in_struct->drmFormatModifier;
}

void safe_VkImageDrmFormatModifierPropertiesEXT::initialize(const safe_VkImageDrmFormatModifierPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    drmFormatModifier = src->drmFormatModifier;
}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT(const VkValidationCacheCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    initialDataSize(in_struct->initialDataSize),
    pInitialData(in_struct->pInitialData)
{
}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT()
{}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT(const safe_VkValidationCacheCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    initialDataSize = src.initialDataSize;
    pInitialData = src.pInitialData;
}

safe_VkValidationCacheCreateInfoEXT& safe_VkValidationCacheCreateInfoEXT::operator=(const safe_VkValidationCacheCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    initialDataSize = src.initialDataSize;
    pInitialData = src.pInitialData;

    return *this;
}

safe_VkValidationCacheCreateInfoEXT::~safe_VkValidationCacheCreateInfoEXT()
{
}

void safe_VkValidationCacheCreateInfoEXT::initialize(const VkValidationCacheCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    initialDataSize = in_struct->initialDataSize;
    pInitialData = in_struct->pInitialData;
}

void safe_VkValidationCacheCreateInfoEXT::initialize(const safe_VkValidationCacheCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    initialDataSize = src->initialDataSize;
    pInitialData = src->pInitialData;
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT(const VkShaderModuleValidationCacheCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    validationCache(in_struct->validationCache)
{
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT()
{}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT(const safe_VkShaderModuleValidationCacheCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    validationCache = src.validationCache;
}

safe_VkShaderModuleValidationCacheCreateInfoEXT& safe_VkShaderModuleValidationCacheCreateInfoEXT::operator=(const safe_VkShaderModuleValidationCacheCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    validationCache = src.validationCache;

    return *this;
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::~safe_VkShaderModuleValidationCacheCreateInfoEXT()
{
}

void safe_VkShaderModuleValidationCacheCreateInfoEXT::initialize(const VkShaderModuleValidationCacheCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    validationCache = in_struct->validationCache;
}

void safe_VkShaderModuleValidationCacheCreateInfoEXT::initialize(const safe_VkShaderModuleValidationCacheCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    validationCache = src->validationCache;
}

safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT(const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    bindingCount(in_struct->bindingCount),
    pBindingFlags(nullptr)
{
    if (in_struct->pBindingFlags) {
        pBindingFlags = new VkDescriptorBindingFlagsEXT[in_struct->bindingCount];
        memcpy ((void *)pBindingFlags, (void *)in_struct->pBindingFlags, sizeof(VkDescriptorBindingFlagsEXT)*in_struct->bindingCount);
    }
}

safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT() :
    pBindingFlags(nullptr)
{}

safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    bindingCount = src.bindingCount;
    pBindingFlags = nullptr;
    if (src.pBindingFlags) {
        pBindingFlags = new VkDescriptorBindingFlagsEXT[src.bindingCount];
        memcpy ((void *)pBindingFlags, (void *)src.pBindingFlags, sizeof(VkDescriptorBindingFlagsEXT)*src.bindingCount);
    }
}

safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::operator=(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pBindingFlags)
        delete[] pBindingFlags;

    sType = src.sType;
    pNext = src.pNext;
    bindingCount = src.bindingCount;
    pBindingFlags = nullptr;
    if (src.pBindingFlags) {
        pBindingFlags = new VkDescriptorBindingFlagsEXT[src.bindingCount];
        memcpy ((void *)pBindingFlags, (void *)src.pBindingFlags, sizeof(VkDescriptorBindingFlagsEXT)*src.bindingCount);
    }

    return *this;
}

safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::~safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT()
{
    if (pBindingFlags)
        delete[] pBindingFlags;
}

void safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::initialize(const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    bindingCount = in_struct->bindingCount;
    pBindingFlags = nullptr;
    if (in_struct->pBindingFlags) {
        pBindingFlags = new VkDescriptorBindingFlagsEXT[in_struct->bindingCount];
        memcpy ((void *)pBindingFlags, (void *)in_struct->pBindingFlags, sizeof(VkDescriptorBindingFlagsEXT)*in_struct->bindingCount);
    }
}

void safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::initialize(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    bindingCount = src->bindingCount;
    pBindingFlags = nullptr;
    if (src->pBindingFlags) {
        pBindingFlags = new VkDescriptorBindingFlagsEXT[src->bindingCount];
        memcpy ((void *)pBindingFlags, (void *)src->pBindingFlags, sizeof(VkDescriptorBindingFlagsEXT)*src->bindingCount);
    }
}

safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT(const VkPhysicalDeviceDescriptorIndexingFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderInputAttachmentArrayDynamicIndexing(in_struct->shaderInputAttachmentArrayDynamicIndexing),
    shaderUniformTexelBufferArrayDynamicIndexing(in_struct->shaderUniformTexelBufferArrayDynamicIndexing),
    shaderStorageTexelBufferArrayDynamicIndexing(in_struct->shaderStorageTexelBufferArrayDynamicIndexing),
    shaderUniformBufferArrayNonUniformIndexing(in_struct->shaderUniformBufferArrayNonUniformIndexing),
    shaderSampledImageArrayNonUniformIndexing(in_struct->shaderSampledImageArrayNonUniformIndexing),
    shaderStorageBufferArrayNonUniformIndexing(in_struct->shaderStorageBufferArrayNonUniformIndexing),
    shaderStorageImageArrayNonUniformIndexing(in_struct->shaderStorageImageArrayNonUniformIndexing),
    shaderInputAttachmentArrayNonUniformIndexing(in_struct->shaderInputAttachmentArrayNonUniformIndexing),
    shaderUniformTexelBufferArrayNonUniformIndexing(in_struct->shaderUniformTexelBufferArrayNonUniformIndexing),
    shaderStorageTexelBufferArrayNonUniformIndexing(in_struct->shaderStorageTexelBufferArrayNonUniformIndexing),
    descriptorBindingUniformBufferUpdateAfterBind(in_struct->descriptorBindingUniformBufferUpdateAfterBind),
    descriptorBindingSampledImageUpdateAfterBind(in_struct->descriptorBindingSampledImageUpdateAfterBind),
    descriptorBindingStorageImageUpdateAfterBind(in_struct->descriptorBindingStorageImageUpdateAfterBind),
    descriptorBindingStorageBufferUpdateAfterBind(in_struct->descriptorBindingStorageBufferUpdateAfterBind),
    descriptorBindingUniformTexelBufferUpdateAfterBind(in_struct->descriptorBindingUniformTexelBufferUpdateAfterBind),
    descriptorBindingStorageTexelBufferUpdateAfterBind(in_struct->descriptorBindingStorageTexelBufferUpdateAfterBind),
    descriptorBindingUpdateUnusedWhilePending(in_struct->descriptorBindingUpdateUnusedWhilePending),
    descriptorBindingPartiallyBound(in_struct->descriptorBindingPartiallyBound),
    descriptorBindingVariableDescriptorCount(in_struct->descriptorBindingVariableDescriptorCount),
    runtimeDescriptorArray(in_struct->runtimeDescriptorArray)
{
}

safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT()
{}

safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderInputAttachmentArrayDynamicIndexing = src.shaderInputAttachmentArrayDynamicIndexing;
    shaderUniformTexelBufferArrayDynamicIndexing = src.shaderUniformTexelBufferArrayDynamicIndexing;
    shaderStorageTexelBufferArrayDynamicIndexing = src.shaderStorageTexelBufferArrayDynamicIndexing;
    shaderUniformBufferArrayNonUniformIndexing = src.shaderUniformBufferArrayNonUniformIndexing;
    shaderSampledImageArrayNonUniformIndexing = src.shaderSampledImageArrayNonUniformIndexing;
    shaderStorageBufferArrayNonUniformIndexing = src.shaderStorageBufferArrayNonUniformIndexing;
    shaderStorageImageArrayNonUniformIndexing = src.shaderStorageImageArrayNonUniformIndexing;
    shaderInputAttachmentArrayNonUniformIndexing = src.shaderInputAttachmentArrayNonUniformIndexing;
    shaderUniformTexelBufferArrayNonUniformIndexing = src.shaderUniformTexelBufferArrayNonUniformIndexing;
    shaderStorageTexelBufferArrayNonUniformIndexing = src.shaderStorageTexelBufferArrayNonUniformIndexing;
    descriptorBindingUniformBufferUpdateAfterBind = src.descriptorBindingUniformBufferUpdateAfterBind;
    descriptorBindingSampledImageUpdateAfterBind = src.descriptorBindingSampledImageUpdateAfterBind;
    descriptorBindingStorageImageUpdateAfterBind = src.descriptorBindingStorageImageUpdateAfterBind;
    descriptorBindingStorageBufferUpdateAfterBind = src.descriptorBindingStorageBufferUpdateAfterBind;
    descriptorBindingUniformTexelBufferUpdateAfterBind = src.descriptorBindingUniformTexelBufferUpdateAfterBind;
    descriptorBindingStorageTexelBufferUpdateAfterBind = src.descriptorBindingStorageTexelBufferUpdateAfterBind;
    descriptorBindingUpdateUnusedWhilePending = src.descriptorBindingUpdateUnusedWhilePending;
    descriptorBindingPartiallyBound = src.descriptorBindingPartiallyBound;
    descriptorBindingVariableDescriptorCount = src.descriptorBindingVariableDescriptorCount;
    runtimeDescriptorArray = src.runtimeDescriptorArray;
}

safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::operator=(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderInputAttachmentArrayDynamicIndexing = src.shaderInputAttachmentArrayDynamicIndexing;
    shaderUniformTexelBufferArrayDynamicIndexing = src.shaderUniformTexelBufferArrayDynamicIndexing;
    shaderStorageTexelBufferArrayDynamicIndexing = src.shaderStorageTexelBufferArrayDynamicIndexing;
    shaderUniformBufferArrayNonUniformIndexing = src.shaderUniformBufferArrayNonUniformIndexing;
    shaderSampledImageArrayNonUniformIndexing = src.shaderSampledImageArrayNonUniformIndexing;
    shaderStorageBufferArrayNonUniformIndexing = src.shaderStorageBufferArrayNonUniformIndexing;
    shaderStorageImageArrayNonUniformIndexing = src.shaderStorageImageArrayNonUniformIndexing;
    shaderInputAttachmentArrayNonUniformIndexing = src.shaderInputAttachmentArrayNonUniformIndexing;
    shaderUniformTexelBufferArrayNonUniformIndexing = src.shaderUniformTexelBufferArrayNonUniformIndexing;
    shaderStorageTexelBufferArrayNonUniformIndexing = src.shaderStorageTexelBufferArrayNonUniformIndexing;
    descriptorBindingUniformBufferUpdateAfterBind = src.descriptorBindingUniformBufferUpdateAfterBind;
    descriptorBindingSampledImageUpdateAfterBind = src.descriptorBindingSampledImageUpdateAfterBind;
    descriptorBindingStorageImageUpdateAfterBind = src.descriptorBindingStorageImageUpdateAfterBind;
    descriptorBindingStorageBufferUpdateAfterBind = src.descriptorBindingStorageBufferUpdateAfterBind;
    descriptorBindingUniformTexelBufferUpdateAfterBind = src.descriptorBindingUniformTexelBufferUpdateAfterBind;
    descriptorBindingStorageTexelBufferUpdateAfterBind = src.descriptorBindingStorageTexelBufferUpdateAfterBind;
    descriptorBindingUpdateUnusedWhilePending = src.descriptorBindingUpdateUnusedWhilePending;
    descriptorBindingPartiallyBound = src.descriptorBindingPartiallyBound;
    descriptorBindingVariableDescriptorCount = src.descriptorBindingVariableDescriptorCount;
    runtimeDescriptorArray = src.runtimeDescriptorArray;

    return *this;
}

safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::~safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT()
{
}

void safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::initialize(const VkPhysicalDeviceDescriptorIndexingFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderInputAttachmentArrayDynamicIndexing = in_struct->shaderInputAttachmentArrayDynamicIndexing;
    shaderUniformTexelBufferArrayDynamicIndexing = in_struct->shaderUniformTexelBufferArrayDynamicIndexing;
    shaderStorageTexelBufferArrayDynamicIndexing = in_struct->shaderStorageTexelBufferArrayDynamicIndexing;
    shaderUniformBufferArrayNonUniformIndexing = in_struct->shaderUniformBufferArrayNonUniformIndexing;
    shaderSampledImageArrayNonUniformIndexing = in_struct->shaderSampledImageArrayNonUniformIndexing;
    shaderStorageBufferArrayNonUniformIndexing = in_struct->shaderStorageBufferArrayNonUniformIndexing;
    shaderStorageImageArrayNonUniformIndexing = in_struct->shaderStorageImageArrayNonUniformIndexing;
    shaderInputAttachmentArrayNonUniformIndexing = in_struct->shaderInputAttachmentArrayNonUniformIndexing;
    shaderUniformTexelBufferArrayNonUniformIndexing = in_struct->shaderUniformTexelBufferArrayNonUniformIndexing;
    shaderStorageTexelBufferArrayNonUniformIndexing = in_struct->shaderStorageTexelBufferArrayNonUniformIndexing;
    descriptorBindingUniformBufferUpdateAfterBind = in_struct->descriptorBindingUniformBufferUpdateAfterBind;
    descriptorBindingSampledImageUpdateAfterBind = in_struct->descriptorBindingSampledImageUpdateAfterBind;
    descriptorBindingStorageImageUpdateAfterBind = in_struct->descriptorBindingStorageImageUpdateAfterBind;
    descriptorBindingStorageBufferUpdateAfterBind = in_struct->descriptorBindingStorageBufferUpdateAfterBind;
    descriptorBindingUniformTexelBufferUpdateAfterBind = in_struct->descriptorBindingUniformTexelBufferUpdateAfterBind;
    descriptorBindingStorageTexelBufferUpdateAfterBind = in_struct->descriptorBindingStorageTexelBufferUpdateAfterBind;
    descriptorBindingUpdateUnusedWhilePending = in_struct->descriptorBindingUpdateUnusedWhilePending;
    descriptorBindingPartiallyBound = in_struct->descriptorBindingPartiallyBound;
    descriptorBindingVariableDescriptorCount = in_struct->descriptorBindingVariableDescriptorCount;
    runtimeDescriptorArray = in_struct->runtimeDescriptorArray;
}

void safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT::initialize(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderInputAttachmentArrayDynamicIndexing = src->shaderInputAttachmentArrayDynamicIndexing;
    shaderUniformTexelBufferArrayDynamicIndexing = src->shaderUniformTexelBufferArrayDynamicIndexing;
    shaderStorageTexelBufferArrayDynamicIndexing = src->shaderStorageTexelBufferArrayDynamicIndexing;
    shaderUniformBufferArrayNonUniformIndexing = src->shaderUniformBufferArrayNonUniformIndexing;
    shaderSampledImageArrayNonUniformIndexing = src->shaderSampledImageArrayNonUniformIndexing;
    shaderStorageBufferArrayNonUniformIndexing = src->shaderStorageBufferArrayNonUniformIndexing;
    shaderStorageImageArrayNonUniformIndexing = src->shaderStorageImageArrayNonUniformIndexing;
    shaderInputAttachmentArrayNonUniformIndexing = src->shaderInputAttachmentArrayNonUniformIndexing;
    shaderUniformTexelBufferArrayNonUniformIndexing = src->shaderUniformTexelBufferArrayNonUniformIndexing;
    shaderStorageTexelBufferArrayNonUniformIndexing = src->shaderStorageTexelBufferArrayNonUniformIndexing;
    descriptorBindingUniformBufferUpdateAfterBind = src->descriptorBindingUniformBufferUpdateAfterBind;
    descriptorBindingSampledImageUpdateAfterBind = src->descriptorBindingSampledImageUpdateAfterBind;
    descriptorBindingStorageImageUpdateAfterBind = src->descriptorBindingStorageImageUpdateAfterBind;
    descriptorBindingStorageBufferUpdateAfterBind = src->descriptorBindingStorageBufferUpdateAfterBind;
    descriptorBindingUniformTexelBufferUpdateAfterBind = src->descriptorBindingUniformTexelBufferUpdateAfterBind;
    descriptorBindingStorageTexelBufferUpdateAfterBind = src->descriptorBindingStorageTexelBufferUpdateAfterBind;
    descriptorBindingUpdateUnusedWhilePending = src->descriptorBindingUpdateUnusedWhilePending;
    descriptorBindingPartiallyBound = src->descriptorBindingPartiallyBound;
    descriptorBindingVariableDescriptorCount = src->descriptorBindingVariableDescriptorCount;
    runtimeDescriptorArray = src->runtimeDescriptorArray;
}

safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT(const VkPhysicalDeviceDescriptorIndexingPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxUpdateAfterBindDescriptorsInAllPools(in_struct->maxUpdateAfterBindDescriptorsInAllPools),
    shaderUniformBufferArrayNonUniformIndexingNative(in_struct->shaderUniformBufferArrayNonUniformIndexingNative),
    shaderSampledImageArrayNonUniformIndexingNative(in_struct->shaderSampledImageArrayNonUniformIndexingNative),
    shaderStorageBufferArrayNonUniformIndexingNative(in_struct->shaderStorageBufferArrayNonUniformIndexingNative),
    shaderStorageImageArrayNonUniformIndexingNative(in_struct->shaderStorageImageArrayNonUniformIndexingNative),
    shaderInputAttachmentArrayNonUniformIndexingNative(in_struct->shaderInputAttachmentArrayNonUniformIndexingNative),
    robustBufferAccessUpdateAfterBind(in_struct->robustBufferAccessUpdateAfterBind),
    quadDivergentImplicitLod(in_struct->quadDivergentImplicitLod),
    maxPerStageDescriptorUpdateAfterBindSamplers(in_struct->maxPerStageDescriptorUpdateAfterBindSamplers),
    maxPerStageDescriptorUpdateAfterBindUniformBuffers(in_struct->maxPerStageDescriptorUpdateAfterBindUniformBuffers),
    maxPerStageDescriptorUpdateAfterBindStorageBuffers(in_struct->maxPerStageDescriptorUpdateAfterBindStorageBuffers),
    maxPerStageDescriptorUpdateAfterBindSampledImages(in_struct->maxPerStageDescriptorUpdateAfterBindSampledImages),
    maxPerStageDescriptorUpdateAfterBindStorageImages(in_struct->maxPerStageDescriptorUpdateAfterBindStorageImages),
    maxPerStageDescriptorUpdateAfterBindInputAttachments(in_struct->maxPerStageDescriptorUpdateAfterBindInputAttachments),
    maxPerStageUpdateAfterBindResources(in_struct->maxPerStageUpdateAfterBindResources),
    maxDescriptorSetUpdateAfterBindSamplers(in_struct->maxDescriptorSetUpdateAfterBindSamplers),
    maxDescriptorSetUpdateAfterBindUniformBuffers(in_struct->maxDescriptorSetUpdateAfterBindUniformBuffers),
    maxDescriptorSetUpdateAfterBindUniformBuffersDynamic(in_struct->maxDescriptorSetUpdateAfterBindUniformBuffersDynamic),
    maxDescriptorSetUpdateAfterBindStorageBuffers(in_struct->maxDescriptorSetUpdateAfterBindStorageBuffers),
    maxDescriptorSetUpdateAfterBindStorageBuffersDynamic(in_struct->maxDescriptorSetUpdateAfterBindStorageBuffersDynamic),
    maxDescriptorSetUpdateAfterBindSampledImages(in_struct->maxDescriptorSetUpdateAfterBindSampledImages),
    maxDescriptorSetUpdateAfterBindStorageImages(in_struct->maxDescriptorSetUpdateAfterBindStorageImages),
    maxDescriptorSetUpdateAfterBindInputAttachments(in_struct->maxDescriptorSetUpdateAfterBindInputAttachments)
{
}

safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT()
{}

safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxUpdateAfterBindDescriptorsInAllPools = src.maxUpdateAfterBindDescriptorsInAllPools;
    shaderUniformBufferArrayNonUniformIndexingNative = src.shaderUniformBufferArrayNonUniformIndexingNative;
    shaderSampledImageArrayNonUniformIndexingNative = src.shaderSampledImageArrayNonUniformIndexingNative;
    shaderStorageBufferArrayNonUniformIndexingNative = src.shaderStorageBufferArrayNonUniformIndexingNative;
    shaderStorageImageArrayNonUniformIndexingNative = src.shaderStorageImageArrayNonUniformIndexingNative;
    shaderInputAttachmentArrayNonUniformIndexingNative = src.shaderInputAttachmentArrayNonUniformIndexingNative;
    robustBufferAccessUpdateAfterBind = src.robustBufferAccessUpdateAfterBind;
    quadDivergentImplicitLod = src.quadDivergentImplicitLod;
    maxPerStageDescriptorUpdateAfterBindSamplers = src.maxPerStageDescriptorUpdateAfterBindSamplers;
    maxPerStageDescriptorUpdateAfterBindUniformBuffers = src.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    maxPerStageDescriptorUpdateAfterBindStorageBuffers = src.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    maxPerStageDescriptorUpdateAfterBindSampledImages = src.maxPerStageDescriptorUpdateAfterBindSampledImages;
    maxPerStageDescriptorUpdateAfterBindStorageImages = src.maxPerStageDescriptorUpdateAfterBindStorageImages;
    maxPerStageDescriptorUpdateAfterBindInputAttachments = src.maxPerStageDescriptorUpdateAfterBindInputAttachments;
    maxPerStageUpdateAfterBindResources = src.maxPerStageUpdateAfterBindResources;
    maxDescriptorSetUpdateAfterBindSamplers = src.maxDescriptorSetUpdateAfterBindSamplers;
    maxDescriptorSetUpdateAfterBindUniformBuffers = src.maxDescriptorSetUpdateAfterBindUniformBuffers;
    maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = src.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindStorageBuffers = src.maxDescriptorSetUpdateAfterBindStorageBuffers;
    maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = src.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindSampledImages = src.maxDescriptorSetUpdateAfterBindSampledImages;
    maxDescriptorSetUpdateAfterBindStorageImages = src.maxDescriptorSetUpdateAfterBindStorageImages;
    maxDescriptorSetUpdateAfterBindInputAttachments = src.maxDescriptorSetUpdateAfterBindInputAttachments;
}

safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::operator=(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxUpdateAfterBindDescriptorsInAllPools = src.maxUpdateAfterBindDescriptorsInAllPools;
    shaderUniformBufferArrayNonUniformIndexingNative = src.shaderUniformBufferArrayNonUniformIndexingNative;
    shaderSampledImageArrayNonUniformIndexingNative = src.shaderSampledImageArrayNonUniformIndexingNative;
    shaderStorageBufferArrayNonUniformIndexingNative = src.shaderStorageBufferArrayNonUniformIndexingNative;
    shaderStorageImageArrayNonUniformIndexingNative = src.shaderStorageImageArrayNonUniformIndexingNative;
    shaderInputAttachmentArrayNonUniformIndexingNative = src.shaderInputAttachmentArrayNonUniformIndexingNative;
    robustBufferAccessUpdateAfterBind = src.robustBufferAccessUpdateAfterBind;
    quadDivergentImplicitLod = src.quadDivergentImplicitLod;
    maxPerStageDescriptorUpdateAfterBindSamplers = src.maxPerStageDescriptorUpdateAfterBindSamplers;
    maxPerStageDescriptorUpdateAfterBindUniformBuffers = src.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    maxPerStageDescriptorUpdateAfterBindStorageBuffers = src.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    maxPerStageDescriptorUpdateAfterBindSampledImages = src.maxPerStageDescriptorUpdateAfterBindSampledImages;
    maxPerStageDescriptorUpdateAfterBindStorageImages = src.maxPerStageDescriptorUpdateAfterBindStorageImages;
    maxPerStageDescriptorUpdateAfterBindInputAttachments = src.maxPerStageDescriptorUpdateAfterBindInputAttachments;
    maxPerStageUpdateAfterBindResources = src.maxPerStageUpdateAfterBindResources;
    maxDescriptorSetUpdateAfterBindSamplers = src.maxDescriptorSetUpdateAfterBindSamplers;
    maxDescriptorSetUpdateAfterBindUniformBuffers = src.maxDescriptorSetUpdateAfterBindUniformBuffers;
    maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = src.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindStorageBuffers = src.maxDescriptorSetUpdateAfterBindStorageBuffers;
    maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = src.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindSampledImages = src.maxDescriptorSetUpdateAfterBindSampledImages;
    maxDescriptorSetUpdateAfterBindStorageImages = src.maxDescriptorSetUpdateAfterBindStorageImages;
    maxDescriptorSetUpdateAfterBindInputAttachments = src.maxDescriptorSetUpdateAfterBindInputAttachments;

    return *this;
}

safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::~safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT()
{
}

void safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::initialize(const VkPhysicalDeviceDescriptorIndexingPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxUpdateAfterBindDescriptorsInAllPools = in_struct->maxUpdateAfterBindDescriptorsInAllPools;
    shaderUniformBufferArrayNonUniformIndexingNative = in_struct->shaderUniformBufferArrayNonUniformIndexingNative;
    shaderSampledImageArrayNonUniformIndexingNative = in_struct->shaderSampledImageArrayNonUniformIndexingNative;
    shaderStorageBufferArrayNonUniformIndexingNative = in_struct->shaderStorageBufferArrayNonUniformIndexingNative;
    shaderStorageImageArrayNonUniformIndexingNative = in_struct->shaderStorageImageArrayNonUniformIndexingNative;
    shaderInputAttachmentArrayNonUniformIndexingNative = in_struct->shaderInputAttachmentArrayNonUniformIndexingNative;
    robustBufferAccessUpdateAfterBind = in_struct->robustBufferAccessUpdateAfterBind;
    quadDivergentImplicitLod = in_struct->quadDivergentImplicitLod;
    maxPerStageDescriptorUpdateAfterBindSamplers = in_struct->maxPerStageDescriptorUpdateAfterBindSamplers;
    maxPerStageDescriptorUpdateAfterBindUniformBuffers = in_struct->maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    maxPerStageDescriptorUpdateAfterBindStorageBuffers = in_struct->maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    maxPerStageDescriptorUpdateAfterBindSampledImages = in_struct->maxPerStageDescriptorUpdateAfterBindSampledImages;
    maxPerStageDescriptorUpdateAfterBindStorageImages = in_struct->maxPerStageDescriptorUpdateAfterBindStorageImages;
    maxPerStageDescriptorUpdateAfterBindInputAttachments = in_struct->maxPerStageDescriptorUpdateAfterBindInputAttachments;
    maxPerStageUpdateAfterBindResources = in_struct->maxPerStageUpdateAfterBindResources;
    maxDescriptorSetUpdateAfterBindSamplers = in_struct->maxDescriptorSetUpdateAfterBindSamplers;
    maxDescriptorSetUpdateAfterBindUniformBuffers = in_struct->maxDescriptorSetUpdateAfterBindUniformBuffers;
    maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = in_struct->maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindStorageBuffers = in_struct->maxDescriptorSetUpdateAfterBindStorageBuffers;
    maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = in_struct->maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindSampledImages = in_struct->maxDescriptorSetUpdateAfterBindSampledImages;
    maxDescriptorSetUpdateAfterBindStorageImages = in_struct->maxDescriptorSetUpdateAfterBindStorageImages;
    maxDescriptorSetUpdateAfterBindInputAttachments = in_struct->maxDescriptorSetUpdateAfterBindInputAttachments;
}

void safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT::initialize(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxUpdateAfterBindDescriptorsInAllPools = src->maxUpdateAfterBindDescriptorsInAllPools;
    shaderUniformBufferArrayNonUniformIndexingNative = src->shaderUniformBufferArrayNonUniformIndexingNative;
    shaderSampledImageArrayNonUniformIndexingNative = src->shaderSampledImageArrayNonUniformIndexingNative;
    shaderStorageBufferArrayNonUniformIndexingNative = src->shaderStorageBufferArrayNonUniformIndexingNative;
    shaderStorageImageArrayNonUniformIndexingNative = src->shaderStorageImageArrayNonUniformIndexingNative;
    shaderInputAttachmentArrayNonUniformIndexingNative = src->shaderInputAttachmentArrayNonUniformIndexingNative;
    robustBufferAccessUpdateAfterBind = src->robustBufferAccessUpdateAfterBind;
    quadDivergentImplicitLod = src->quadDivergentImplicitLod;
    maxPerStageDescriptorUpdateAfterBindSamplers = src->maxPerStageDescriptorUpdateAfterBindSamplers;
    maxPerStageDescriptorUpdateAfterBindUniformBuffers = src->maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    maxPerStageDescriptorUpdateAfterBindStorageBuffers = src->maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    maxPerStageDescriptorUpdateAfterBindSampledImages = src->maxPerStageDescriptorUpdateAfterBindSampledImages;
    maxPerStageDescriptorUpdateAfterBindStorageImages = src->maxPerStageDescriptorUpdateAfterBindStorageImages;
    maxPerStageDescriptorUpdateAfterBindInputAttachments = src->maxPerStageDescriptorUpdateAfterBindInputAttachments;
    maxPerStageUpdateAfterBindResources = src->maxPerStageUpdateAfterBindResources;
    maxDescriptorSetUpdateAfterBindSamplers = src->maxDescriptorSetUpdateAfterBindSamplers;
    maxDescriptorSetUpdateAfterBindUniformBuffers = src->maxDescriptorSetUpdateAfterBindUniformBuffers;
    maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = src->maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindStorageBuffers = src->maxDescriptorSetUpdateAfterBindStorageBuffers;
    maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = src->maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindSampledImages = src->maxDescriptorSetUpdateAfterBindSampledImages;
    maxDescriptorSetUpdateAfterBindStorageImages = src->maxDescriptorSetUpdateAfterBindStorageImages;
    maxDescriptorSetUpdateAfterBindInputAttachments = src->maxDescriptorSetUpdateAfterBindInputAttachments;
}

safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT(const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    descriptorSetCount(in_struct->descriptorSetCount),
    pDescriptorCounts(nullptr)
{
    if (in_struct->pDescriptorCounts) {
        pDescriptorCounts = new uint32_t[in_struct->descriptorSetCount];
        memcpy ((void *)pDescriptorCounts, (void *)in_struct->pDescriptorCounts, sizeof(uint32_t)*in_struct->descriptorSetCount);
    }
}

safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT() :
    pDescriptorCounts(nullptr)
{}

safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    descriptorSetCount = src.descriptorSetCount;
    pDescriptorCounts = nullptr;
    if (src.pDescriptorCounts) {
        pDescriptorCounts = new uint32_t[src.descriptorSetCount];
        memcpy ((void *)pDescriptorCounts, (void *)src.pDescriptorCounts, sizeof(uint32_t)*src.descriptorSetCount);
    }
}

safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::operator=(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pDescriptorCounts)
        delete[] pDescriptorCounts;

    sType = src.sType;
    pNext = src.pNext;
    descriptorSetCount = src.descriptorSetCount;
    pDescriptorCounts = nullptr;
    if (src.pDescriptorCounts) {
        pDescriptorCounts = new uint32_t[src.descriptorSetCount];
        memcpy ((void *)pDescriptorCounts, (void *)src.pDescriptorCounts, sizeof(uint32_t)*src.descriptorSetCount);
    }

    return *this;
}

safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::~safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT()
{
    if (pDescriptorCounts)
        delete[] pDescriptorCounts;
}

void safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::initialize(const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    descriptorSetCount = in_struct->descriptorSetCount;
    pDescriptorCounts = nullptr;
    if (in_struct->pDescriptorCounts) {
        pDescriptorCounts = new uint32_t[in_struct->descriptorSetCount];
        memcpy ((void *)pDescriptorCounts, (void *)in_struct->pDescriptorCounts, sizeof(uint32_t)*in_struct->descriptorSetCount);
    }
}

void safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::initialize(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    descriptorSetCount = src->descriptorSetCount;
    pDescriptorCounts = nullptr;
    if (src->pDescriptorCounts) {
        pDescriptorCounts = new uint32_t[src->descriptorSetCount];
        memcpy ((void *)pDescriptorCounts, (void *)src->pDescriptorCounts, sizeof(uint32_t)*src->descriptorSetCount);
    }
}

safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT(const VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxVariableDescriptorCount(in_struct->maxVariableDescriptorCount)
{
}

safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT()
{}

safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxVariableDescriptorCount = src.maxVariableDescriptorCount;
}

safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::operator=(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxVariableDescriptorCount = src.maxVariableDescriptorCount;

    return *this;
}

safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::~safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT()
{
}

void safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::initialize(const VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxVariableDescriptorCount = in_struct->maxVariableDescriptorCount;
}

void safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT::initialize(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxVariableDescriptorCount = src->maxVariableDescriptorCount;
}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV(const VkShadingRatePaletteNV* in_struct) :
    shadingRatePaletteEntryCount(in_struct->shadingRatePaletteEntryCount),
    pShadingRatePaletteEntries(nullptr)
{
    if (in_struct->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[in_struct->shadingRatePaletteEntryCount];
        memcpy ((void *)pShadingRatePaletteEntries, (void *)in_struct->pShadingRatePaletteEntries, sizeof(VkShadingRatePaletteEntryNV)*in_struct->shadingRatePaletteEntryCount);
    }
}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV() :
    pShadingRatePaletteEntries(nullptr)
{}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV(const safe_VkShadingRatePaletteNV& src)
{
    shadingRatePaletteEntryCount = src.shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;
    if (src.pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[src.shadingRatePaletteEntryCount];
        memcpy ((void *)pShadingRatePaletteEntries, (void *)src.pShadingRatePaletteEntries, sizeof(VkShadingRatePaletteEntryNV)*src.shadingRatePaletteEntryCount);
    }
}

safe_VkShadingRatePaletteNV& safe_VkShadingRatePaletteNV::operator=(const safe_VkShadingRatePaletteNV& src)
{
    if (&src == this) return *this;

    if (pShadingRatePaletteEntries)
        delete[] pShadingRatePaletteEntries;

    shadingRatePaletteEntryCount = src.shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;
    if (src.pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[src.shadingRatePaletteEntryCount];
        memcpy ((void *)pShadingRatePaletteEntries, (void *)src.pShadingRatePaletteEntries, sizeof(VkShadingRatePaletteEntryNV)*src.shadingRatePaletteEntryCount);
    }

    return *this;
}

safe_VkShadingRatePaletteNV::~safe_VkShadingRatePaletteNV()
{
    if (pShadingRatePaletteEntries)
        delete[] pShadingRatePaletteEntries;
}

void safe_VkShadingRatePaletteNV::initialize(const VkShadingRatePaletteNV* in_struct)
{
    shadingRatePaletteEntryCount = in_struct->shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;
    if (in_struct->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[in_struct->shadingRatePaletteEntryCount];
        memcpy ((void *)pShadingRatePaletteEntries, (void *)in_struct->pShadingRatePaletteEntries, sizeof(VkShadingRatePaletteEntryNV)*in_struct->shadingRatePaletteEntryCount);
    }
}

void safe_VkShadingRatePaletteNV::initialize(const safe_VkShadingRatePaletteNV* src)
{
    shadingRatePaletteEntryCount = src->shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;
    if (src->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[src->shadingRatePaletteEntryCount];
        memcpy ((void *)pShadingRatePaletteEntries, (void *)src->pShadingRatePaletteEntries, sizeof(VkShadingRatePaletteEntryNV)*src->shadingRatePaletteEntryCount);
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shadingRateImageEnable(in_struct->shadingRateImageEnable),
    viewportCount(in_struct->viewportCount),
    pShadingRatePalettes(nullptr)
{
    if (viewportCount && in_struct->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i=0; i<viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&in_struct->pShadingRatePalettes[i]);
        }
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV() :
    pShadingRatePalettes(nullptr)
{}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shadingRateImageEnable = src.shadingRateImageEnable;
    viewportCount = src.viewportCount;
    pShadingRatePalettes = nullptr;
    if (viewportCount && src.pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i=0; i<viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&src.pShadingRatePalettes[i]);
        }
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::operator=(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pShadingRatePalettes)
        delete[] pShadingRatePalettes;

    sType = src.sType;
    pNext = src.pNext;
    shadingRateImageEnable = src.shadingRateImageEnable;
    viewportCount = src.viewportCount;
    pShadingRatePalettes = nullptr;
    if (viewportCount && src.pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i=0; i<viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&src.pShadingRatePalettes[i]);
        }
    }

    return *this;
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::~safe_VkPipelineViewportShadingRateImageStateCreateInfoNV()
{
    if (pShadingRatePalettes)
        delete[] pShadingRatePalettes;
}

void safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::initialize(const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shadingRateImageEnable = in_struct->shadingRateImageEnable;
    viewportCount = in_struct->viewportCount;
    pShadingRatePalettes = nullptr;
    if (viewportCount && in_struct->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i=0; i<viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&in_struct->pShadingRatePalettes[i]);
        }
    }
}

void safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::initialize(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shadingRateImageEnable = src->shadingRateImageEnable;
    viewportCount = src->viewportCount;
    pShadingRatePalettes = nullptr;
    if (viewportCount && src->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i=0; i<viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&src->pShadingRatePalettes[i]);
        }
    }
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV(const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shadingRateImage(in_struct->shadingRateImage),
    shadingRateCoarseSampleOrder(in_struct->shadingRateCoarseSampleOrder)
{
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV()
{}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shadingRateImage = src.shadingRateImage;
    shadingRateCoarseSampleOrder = src.shadingRateCoarseSampleOrder;
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV& safe_VkPhysicalDeviceShadingRateImageFeaturesNV::operator=(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shadingRateImage = src.shadingRateImage;
    shadingRateCoarseSampleOrder = src.shadingRateCoarseSampleOrder;

    return *this;
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::~safe_VkPhysicalDeviceShadingRateImageFeaturesNV()
{
}

void safe_VkPhysicalDeviceShadingRateImageFeaturesNV::initialize(const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shadingRateImage = in_struct->shadingRateImage;
    shadingRateCoarseSampleOrder = in_struct->shadingRateCoarseSampleOrder;
}

void safe_VkPhysicalDeviceShadingRateImageFeaturesNV::initialize(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shadingRateImage = src->shadingRateImage;
    shadingRateCoarseSampleOrder = src->shadingRateCoarseSampleOrder;
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV(const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shadingRateTexelSize(in_struct->shadingRateTexelSize),
    shadingRatePaletteSize(in_struct->shadingRatePaletteSize),
    shadingRateMaxCoarseSamples(in_struct->shadingRateMaxCoarseSamples)
{
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV()
{}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shadingRateTexelSize = src.shadingRateTexelSize;
    shadingRatePaletteSize = src.shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = src.shadingRateMaxCoarseSamples;
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV& safe_VkPhysicalDeviceShadingRateImagePropertiesNV::operator=(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shadingRateTexelSize = src.shadingRateTexelSize;
    shadingRatePaletteSize = src.shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = src.shadingRateMaxCoarseSamples;

    return *this;
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::~safe_VkPhysicalDeviceShadingRateImagePropertiesNV()
{
}

void safe_VkPhysicalDeviceShadingRateImagePropertiesNV::initialize(const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shadingRateTexelSize = in_struct->shadingRateTexelSize;
    shadingRatePaletteSize = in_struct->shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = in_struct->shadingRateMaxCoarseSamples;
}

void safe_VkPhysicalDeviceShadingRateImagePropertiesNV::initialize(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shadingRateTexelSize = src->shadingRateTexelSize;
    shadingRatePaletteSize = src->shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = src->shadingRateMaxCoarseSamples;
}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV* in_struct) :
    shadingRate(in_struct->shadingRate),
    sampleCount(in_struct->sampleCount),
    sampleLocationCount(in_struct->sampleLocationCount),
    pSampleLocations(nullptr)
{
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[in_struct->sampleLocationCount];
        memcpy ((void *)pSampleLocations, (void *)in_struct->pSampleLocations, sizeof(VkCoarseSampleLocationNV)*in_struct->sampleLocationCount);
    }
}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV() :
    pSampleLocations(nullptr)
{}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV(const safe_VkCoarseSampleOrderCustomNV& src)
{
    shadingRate = src.shadingRate;
    sampleCount = src.sampleCount;
    sampleLocationCount = src.sampleLocationCount;
    pSampleLocations = nullptr;
    if (src.pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[src.sampleLocationCount];
        memcpy ((void *)pSampleLocations, (void *)src.pSampleLocations, sizeof(VkCoarseSampleLocationNV)*src.sampleLocationCount);
    }
}

safe_VkCoarseSampleOrderCustomNV& safe_VkCoarseSampleOrderCustomNV::operator=(const safe_VkCoarseSampleOrderCustomNV& src)
{
    if (&src == this) return *this;

    if (pSampleLocations)
        delete[] pSampleLocations;

    shadingRate = src.shadingRate;
    sampleCount = src.sampleCount;
    sampleLocationCount = src.sampleLocationCount;
    pSampleLocations = nullptr;
    if (src.pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[src.sampleLocationCount];
        memcpy ((void *)pSampleLocations, (void *)src.pSampleLocations, sizeof(VkCoarseSampleLocationNV)*src.sampleLocationCount);
    }

    return *this;
}

safe_VkCoarseSampleOrderCustomNV::~safe_VkCoarseSampleOrderCustomNV()
{
    if (pSampleLocations)
        delete[] pSampleLocations;
}

void safe_VkCoarseSampleOrderCustomNV::initialize(const VkCoarseSampleOrderCustomNV* in_struct)
{
    shadingRate = in_struct->shadingRate;
    sampleCount = in_struct->sampleCount;
    sampleLocationCount = in_struct->sampleLocationCount;
    pSampleLocations = nullptr;
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[in_struct->sampleLocationCount];
        memcpy ((void *)pSampleLocations, (void *)in_struct->pSampleLocations, sizeof(VkCoarseSampleLocationNV)*in_struct->sampleLocationCount);
    }
}

void safe_VkCoarseSampleOrderCustomNV::initialize(const safe_VkCoarseSampleOrderCustomNV* src)
{
    shadingRate = src->shadingRate;
    sampleCount = src->sampleCount;
    sampleLocationCount = src->sampleLocationCount;
    pSampleLocations = nullptr;
    if (src->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[src->sampleLocationCount];
        memcpy ((void *)pSampleLocations, (void *)src->pSampleLocations, sizeof(VkCoarseSampleLocationNV)*src->sampleLocationCount);
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    sampleOrderType(in_struct->sampleOrderType),
    customSampleOrderCount(in_struct->customSampleOrderCount),
    pCustomSampleOrders(nullptr)
{
    if (customSampleOrderCount && in_struct->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i=0; i<customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&in_struct->pCustomSampleOrders[i]);
        }
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV() :
    pCustomSampleOrders(nullptr)
{}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    sampleOrderType = src.sampleOrderType;
    customSampleOrderCount = src.customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    if (customSampleOrderCount && src.pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i=0; i<customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&src.pCustomSampleOrders[i]);
        }
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::operator=(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pCustomSampleOrders)
        delete[] pCustomSampleOrders;

    sType = src.sType;
    pNext = src.pNext;
    sampleOrderType = src.sampleOrderType;
    customSampleOrderCount = src.customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    if (customSampleOrderCount && src.pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i=0; i<customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&src.pCustomSampleOrders[i]);
        }
    }

    return *this;
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::~safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV()
{
    if (pCustomSampleOrders)
        delete[] pCustomSampleOrders;
}

void safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::initialize(const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    sampleOrderType = in_struct->sampleOrderType;
    customSampleOrderCount = in_struct->customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    if (customSampleOrderCount && in_struct->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i=0; i<customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&in_struct->pCustomSampleOrders[i]);
        }
    }
}

void safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::initialize(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    sampleOrderType = src->sampleOrderType;
    customSampleOrderCount = src->customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    if (customSampleOrderCount && src->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i=0; i<customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&src->pCustomSampleOrders[i]);
        }
    }
}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV(const VkRayTracingShaderGroupCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    type(in_struct->type),
    generalShader(in_struct->generalShader),
    closestHitShader(in_struct->closestHitShader),
    anyHitShader(in_struct->anyHitShader),
    intersectionShader(in_struct->intersectionShader)
{
}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV()
{}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV(const safe_VkRayTracingShaderGroupCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    generalShader = src.generalShader;
    closestHitShader = src.closestHitShader;
    anyHitShader = src.anyHitShader;
    intersectionShader = src.intersectionShader;
}

safe_VkRayTracingShaderGroupCreateInfoNV& safe_VkRayTracingShaderGroupCreateInfoNV::operator=(const safe_VkRayTracingShaderGroupCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    generalShader = src.generalShader;
    closestHitShader = src.closestHitShader;
    anyHitShader = src.anyHitShader;
    intersectionShader = src.intersectionShader;

    return *this;
}

safe_VkRayTracingShaderGroupCreateInfoNV::~safe_VkRayTracingShaderGroupCreateInfoNV()
{
}

void safe_VkRayTracingShaderGroupCreateInfoNV::initialize(const VkRayTracingShaderGroupCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    type = in_struct->type;
    generalShader = in_struct->generalShader;
    closestHitShader = in_struct->closestHitShader;
    anyHitShader = in_struct->anyHitShader;
    intersectionShader = in_struct->intersectionShader;
}

void safe_VkRayTracingShaderGroupCreateInfoNV::initialize(const safe_VkRayTracingShaderGroupCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    type = src->type;
    generalShader = src->generalShader;
    closestHitShader = src->closestHitShader;
    anyHitShader = src->anyHitShader;
    intersectionShader = src->intersectionShader;
}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV(const VkRayTracingPipelineCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    stageCount(in_struct->stageCount),
    pStages(nullptr),
    groupCount(in_struct->groupCount),
    pGroups(nullptr),
    maxRecursionDepth(in_struct->maxRecursionDepth),
    layout(in_struct->layout),
    basePipelineHandle(in_struct->basePipelineHandle),
    basePipelineIndex(in_struct->basePipelineIndex)
{
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i=0; i<groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV() :
    pStages(nullptr),
    pGroups(nullptr)
{}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV(const safe_VkRayTracingPipelineCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stageCount = src.stageCount;
    pStages = nullptr;
    groupCount = src.groupCount;
    pGroups = nullptr;
    maxRecursionDepth = src.maxRecursionDepth;
    layout = src.layout;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;
    if (stageCount && src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src.pStages[i]);
        }
    }
    if (groupCount && src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i=0; i<groupCount; ++i) {
            pGroups[i].initialize(&src.pGroups[i]);
        }
    }
}

safe_VkRayTracingPipelineCreateInfoNV& safe_VkRayTracingPipelineCreateInfoNV::operator=(const safe_VkRayTracingPipelineCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pStages)
        delete[] pStages;
    if (pGroups)
        delete[] pGroups;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    stageCount = src.stageCount;
    pStages = nullptr;
    groupCount = src.groupCount;
    pGroups = nullptr;
    maxRecursionDepth = src.maxRecursionDepth;
    layout = src.layout;
    basePipelineHandle = src.basePipelineHandle;
    basePipelineIndex = src.basePipelineIndex;
    if (stageCount && src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src.pStages[i]);
        }
    }
    if (groupCount && src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i=0; i<groupCount; ++i) {
            pGroups[i].initialize(&src.pGroups[i]);
        }
    }

    return *this;
}

safe_VkRayTracingPipelineCreateInfoNV::~safe_VkRayTracingPipelineCreateInfoNV()
{
    if (pStages)
        delete[] pStages;
    if (pGroups)
        delete[] pGroups;
}

void safe_VkRayTracingPipelineCreateInfoNV::initialize(const VkRayTracingPipelineCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    groupCount = in_struct->groupCount;
    pGroups = nullptr;
    maxRecursionDepth = in_struct->maxRecursionDepth;
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i=0; i<groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
}

void safe_VkRayTracingPipelineCreateInfoNV::initialize(const safe_VkRayTracingPipelineCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    stageCount = src->stageCount;
    pStages = nullptr;
    groupCount = src->groupCount;
    pGroups = nullptr;
    maxRecursionDepth = src->maxRecursionDepth;
    layout = src->layout;
    basePipelineHandle = src->basePipelineHandle;
    basePipelineIndex = src->basePipelineIndex;
    if (stageCount && src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i=0; i<stageCount; ++i) {
            pStages[i].initialize(&src->pStages[i]);
        }
    }
    if (groupCount && src->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i=0; i<groupCount; ++i) {
            pGroups[i].initialize(&src->pGroups[i]);
        }
    }
}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV(const VkGeometryTrianglesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    vertexData(in_struct->vertexData),
    vertexOffset(in_struct->vertexOffset),
    vertexCount(in_struct->vertexCount),
    vertexStride(in_struct->vertexStride),
    vertexFormat(in_struct->vertexFormat),
    indexData(in_struct->indexData),
    indexOffset(in_struct->indexOffset),
    indexCount(in_struct->indexCount),
    indexType(in_struct->indexType),
    transformData(in_struct->transformData),
    transformOffset(in_struct->transformOffset)
{
}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV()
{}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV(const safe_VkGeometryTrianglesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    vertexData = src.vertexData;
    vertexOffset = src.vertexOffset;
    vertexCount = src.vertexCount;
    vertexStride = src.vertexStride;
    vertexFormat = src.vertexFormat;
    indexData = src.indexData;
    indexOffset = src.indexOffset;
    indexCount = src.indexCount;
    indexType = src.indexType;
    transformData = src.transformData;
    transformOffset = src.transformOffset;
}

safe_VkGeometryTrianglesNV& safe_VkGeometryTrianglesNV::operator=(const safe_VkGeometryTrianglesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    vertexData = src.vertexData;
    vertexOffset = src.vertexOffset;
    vertexCount = src.vertexCount;
    vertexStride = src.vertexStride;
    vertexFormat = src.vertexFormat;
    indexData = src.indexData;
    indexOffset = src.indexOffset;
    indexCount = src.indexCount;
    indexType = src.indexType;
    transformData = src.transformData;
    transformOffset = src.transformOffset;

    return *this;
}

safe_VkGeometryTrianglesNV::~safe_VkGeometryTrianglesNV()
{
}

void safe_VkGeometryTrianglesNV::initialize(const VkGeometryTrianglesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    vertexData = in_struct->vertexData;
    vertexOffset = in_struct->vertexOffset;
    vertexCount = in_struct->vertexCount;
    vertexStride = in_struct->vertexStride;
    vertexFormat = in_struct->vertexFormat;
    indexData = in_struct->indexData;
    indexOffset = in_struct->indexOffset;
    indexCount = in_struct->indexCount;
    indexType = in_struct->indexType;
    transformData = in_struct->transformData;
    transformOffset = in_struct->transformOffset;
}

void safe_VkGeometryTrianglesNV::initialize(const safe_VkGeometryTrianglesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    vertexData = src->vertexData;
    vertexOffset = src->vertexOffset;
    vertexCount = src->vertexCount;
    vertexStride = src->vertexStride;
    vertexFormat = src->vertexFormat;
    indexData = src->indexData;
    indexOffset = src->indexOffset;
    indexCount = src->indexCount;
    indexType = src->indexType;
    transformData = src->transformData;
    transformOffset = src->transformOffset;
}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV(const VkGeometryAABBNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    aabbData(in_struct->aabbData),
    numAABBs(in_struct->numAABBs),
    stride(in_struct->stride),
    offset(in_struct->offset)
{
}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV()
{}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV(const safe_VkGeometryAABBNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    aabbData = src.aabbData;
    numAABBs = src.numAABBs;
    stride = src.stride;
    offset = src.offset;
}

safe_VkGeometryAABBNV& safe_VkGeometryAABBNV::operator=(const safe_VkGeometryAABBNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    aabbData = src.aabbData;
    numAABBs = src.numAABBs;
    stride = src.stride;
    offset = src.offset;

    return *this;
}

safe_VkGeometryAABBNV::~safe_VkGeometryAABBNV()
{
}

void safe_VkGeometryAABBNV::initialize(const VkGeometryAABBNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    aabbData = in_struct->aabbData;
    numAABBs = in_struct->numAABBs;
    stride = in_struct->stride;
    offset = in_struct->offset;
}

void safe_VkGeometryAABBNV::initialize(const safe_VkGeometryAABBNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    aabbData = src->aabbData;
    numAABBs = src->numAABBs;
    stride = src->stride;
    offset = src->offset;
}

safe_VkGeometryNV::safe_VkGeometryNV(const VkGeometryNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    geometryType(in_struct->geometryType),
    geometry(in_struct->geometry),
    flags(in_struct->flags)
{
}

safe_VkGeometryNV::safe_VkGeometryNV()
{}

safe_VkGeometryNV::safe_VkGeometryNV(const safe_VkGeometryNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    geometryType = src.geometryType;
    geometry = src.geometry;
    flags = src.flags;
}

safe_VkGeometryNV& safe_VkGeometryNV::operator=(const safe_VkGeometryNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    geometryType = src.geometryType;
    geometry = src.geometry;
    flags = src.flags;

    return *this;
}

safe_VkGeometryNV::~safe_VkGeometryNV()
{
}

void safe_VkGeometryNV::initialize(const VkGeometryNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    geometryType = in_struct->geometryType;
    geometry = in_struct->geometry;
    flags = in_struct->flags;
}

void safe_VkGeometryNV::initialize(const safe_VkGeometryNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    geometryType = src->geometryType;
    geometry = src->geometry;
    flags = src->flags;
}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    type(in_struct->type),
    flags(in_struct->flags),
    instanceCount(in_struct->instanceCount),
    geometryCount(in_struct->geometryCount),
    pGeometries(nullptr)
{
    if (geometryCount && in_struct->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i=0; i<geometryCount; ++i) {
            pGeometries[i].initialize(&in_struct->pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV() :
    pGeometries(nullptr)
{}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV(const safe_VkAccelerationStructureInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    flags = src.flags;
    instanceCount = src.instanceCount;
    geometryCount = src.geometryCount;
    pGeometries = nullptr;
    if (geometryCount && src.pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i=0; i<geometryCount; ++i) {
            pGeometries[i].initialize(&src.pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureInfoNV& safe_VkAccelerationStructureInfoNV::operator=(const safe_VkAccelerationStructureInfoNV& src)
{
    if (&src == this) return *this;

    if (pGeometries)
        delete[] pGeometries;

    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    flags = src.flags;
    instanceCount = src.instanceCount;
    geometryCount = src.geometryCount;
    pGeometries = nullptr;
    if (geometryCount && src.pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i=0; i<geometryCount; ++i) {
            pGeometries[i].initialize(&src.pGeometries[i]);
        }
    }

    return *this;
}

safe_VkAccelerationStructureInfoNV::~safe_VkAccelerationStructureInfoNV()
{
    if (pGeometries)
        delete[] pGeometries;
}

void safe_VkAccelerationStructureInfoNV::initialize(const VkAccelerationStructureInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    type = in_struct->type;
    flags = in_struct->flags;
    instanceCount = in_struct->instanceCount;
    geometryCount = in_struct->geometryCount;
    pGeometries = nullptr;
    if (geometryCount && in_struct->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i=0; i<geometryCount; ++i) {
            pGeometries[i].initialize(&in_struct->pGeometries[i]);
        }
    }
}

void safe_VkAccelerationStructureInfoNV::initialize(const safe_VkAccelerationStructureInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    type = src->type;
    flags = src->flags;
    instanceCount = src->instanceCount;
    geometryCount = src->geometryCount;
    pGeometries = nullptr;
    if (geometryCount && src->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i=0; i<geometryCount; ++i) {
            pGeometries[i].initialize(&src->pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV(const VkAccelerationStructureCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    compactedSize(in_struct->compactedSize),
    info(&in_struct->info)
{
}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV()
{}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV(const safe_VkAccelerationStructureCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    compactedSize = src.compactedSize;
    info.initialize(&src.info);
}

safe_VkAccelerationStructureCreateInfoNV& safe_VkAccelerationStructureCreateInfoNV::operator=(const safe_VkAccelerationStructureCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    compactedSize = src.compactedSize;
    info.initialize(&src.info);

    return *this;
}

safe_VkAccelerationStructureCreateInfoNV::~safe_VkAccelerationStructureCreateInfoNV()
{
}

void safe_VkAccelerationStructureCreateInfoNV::initialize(const VkAccelerationStructureCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    compactedSize = in_struct->compactedSize;
    info.initialize(&in_struct->info);
}

void safe_VkAccelerationStructureCreateInfoNV::initialize(const safe_VkAccelerationStructureCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    compactedSize = src->compactedSize;
    info.initialize(&src->info);
}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV(const VkBindAccelerationStructureMemoryInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    accelerationStructure(in_struct->accelerationStructure),
    memory(in_struct->memory),
    memoryOffset(in_struct->memoryOffset),
    deviceIndexCount(in_struct->deviceIndexCount),
    pDeviceIndices(nullptr)
{
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV() :
    pDeviceIndices(nullptr)
{}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV(const safe_VkBindAccelerationStructureMemoryInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    accelerationStructure = src.accelerationStructure;
    memory = src.memory;
    memoryOffset = src.memoryOffset;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }
}

safe_VkBindAccelerationStructureMemoryInfoNV& safe_VkBindAccelerationStructureMemoryInfoNV::operator=(const safe_VkBindAccelerationStructureMemoryInfoNV& src)
{
    if (&src == this) return *this;

    if (pDeviceIndices)
        delete[] pDeviceIndices;

    sType = src.sType;
    pNext = src.pNext;
    accelerationStructure = src.accelerationStructure;
    memory = src.memory;
    memoryOffset = src.memoryOffset;
    deviceIndexCount = src.deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[src.deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src.pDeviceIndices, sizeof(uint32_t)*src.deviceIndexCount);
    }

    return *this;
}

safe_VkBindAccelerationStructureMemoryInfoNV::~safe_VkBindAccelerationStructureMemoryInfoNV()
{
    if (pDeviceIndices)
        delete[] pDeviceIndices;
}

void safe_VkBindAccelerationStructureMemoryInfoNV::initialize(const VkBindAccelerationStructureMemoryInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    accelerationStructure = in_struct->accelerationStructure;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
    deviceIndexCount = in_struct->deviceIndexCount;
    pDeviceIndices = nullptr;
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)in_struct->pDeviceIndices, sizeof(uint32_t)*in_struct->deviceIndexCount);
    }
}

void safe_VkBindAccelerationStructureMemoryInfoNV::initialize(const safe_VkBindAccelerationStructureMemoryInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    accelerationStructure = src->accelerationStructure;
    memory = src->memory;
    memoryOffset = src->memoryOffset;
    deviceIndexCount = src->deviceIndexCount;
    pDeviceIndices = nullptr;
    if (src->pDeviceIndices) {
        pDeviceIndices = new uint32_t[src->deviceIndexCount];
        memcpy ((void *)pDeviceIndices, (void *)src->pDeviceIndices, sizeof(uint32_t)*src->deviceIndexCount);
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV(const VkWriteDescriptorSetAccelerationStructureNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    accelerationStructureCount(in_struct->accelerationStructureCount),
    pAccelerationStructures(nullptr)
{
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i=0; i<accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV() :
    pAccelerationStructures(nullptr)
{}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV(const safe_VkWriteDescriptorSetAccelerationStructureNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    accelerationStructureCount = src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    if (accelerationStructureCount && src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i=0; i<accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = src.pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV& safe_VkWriteDescriptorSetAccelerationStructureNV::operator=(const safe_VkWriteDescriptorSetAccelerationStructureNV& src)
{
    if (&src == this) return *this;

    if (pAccelerationStructures)
        delete[] pAccelerationStructures;

    sType = src.sType;
    pNext = src.pNext;
    accelerationStructureCount = src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    if (accelerationStructureCount && src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i=0; i<accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = src.pAccelerationStructures[i];
        }
    }

    return *this;
}

safe_VkWriteDescriptorSetAccelerationStructureNV::~safe_VkWriteDescriptorSetAccelerationStructureNV()
{
    if (pAccelerationStructures)
        delete[] pAccelerationStructures;
}

void safe_VkWriteDescriptorSetAccelerationStructureNV::initialize(const VkWriteDescriptorSetAccelerationStructureNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    accelerationStructureCount = in_struct->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i=0; i<accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

void safe_VkWriteDescriptorSetAccelerationStructureNV::initialize(const safe_VkWriteDescriptorSetAccelerationStructureNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    accelerationStructureCount = src->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    if (accelerationStructureCount && src->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i=0; i<accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = src->pAccelerationStructures[i];
        }
    }
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV(const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    type(in_struct->type),
    accelerationStructure(in_struct->accelerationStructure)
{
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV()
{}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV(const safe_VkAccelerationStructureMemoryRequirementsInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    accelerationStructure = src.accelerationStructure;
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV& safe_VkAccelerationStructureMemoryRequirementsInfoNV::operator=(const safe_VkAccelerationStructureMemoryRequirementsInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    accelerationStructure = src.accelerationStructure;

    return *this;
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::~safe_VkAccelerationStructureMemoryRequirementsInfoNV()
{
}

void safe_VkAccelerationStructureMemoryRequirementsInfoNV::initialize(const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    type = in_struct->type;
    accelerationStructure = in_struct->accelerationStructure;
}

void safe_VkAccelerationStructureMemoryRequirementsInfoNV::initialize(const safe_VkAccelerationStructureMemoryRequirementsInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    type = src->type;
    accelerationStructure = src->accelerationStructure;
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV(const VkPhysicalDeviceRayTracingPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderGroupHandleSize(in_struct->shaderGroupHandleSize),
    maxRecursionDepth(in_struct->maxRecursionDepth),
    maxShaderGroupStride(in_struct->maxShaderGroupStride),
    shaderGroupBaseAlignment(in_struct->shaderGroupBaseAlignment),
    maxGeometryCount(in_struct->maxGeometryCount),
    maxInstanceCount(in_struct->maxInstanceCount),
    maxTriangleCount(in_struct->maxTriangleCount),
    maxDescriptorSetAccelerationStructures(in_struct->maxDescriptorSetAccelerationStructures)
{
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV()
{}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV(const safe_VkPhysicalDeviceRayTracingPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderGroupHandleSize = src.shaderGroupHandleSize;
    maxRecursionDepth = src.maxRecursionDepth;
    maxShaderGroupStride = src.maxShaderGroupStride;
    shaderGroupBaseAlignment = src.shaderGroupBaseAlignment;
    maxGeometryCount = src.maxGeometryCount;
    maxInstanceCount = src.maxInstanceCount;
    maxTriangleCount = src.maxTriangleCount;
    maxDescriptorSetAccelerationStructures = src.maxDescriptorSetAccelerationStructures;
}

safe_VkPhysicalDeviceRayTracingPropertiesNV& safe_VkPhysicalDeviceRayTracingPropertiesNV::operator=(const safe_VkPhysicalDeviceRayTracingPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderGroupHandleSize = src.shaderGroupHandleSize;
    maxRecursionDepth = src.maxRecursionDepth;
    maxShaderGroupStride = src.maxShaderGroupStride;
    shaderGroupBaseAlignment = src.shaderGroupBaseAlignment;
    maxGeometryCount = src.maxGeometryCount;
    maxInstanceCount = src.maxInstanceCount;
    maxTriangleCount = src.maxTriangleCount;
    maxDescriptorSetAccelerationStructures = src.maxDescriptorSetAccelerationStructures;

    return *this;
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::~safe_VkPhysicalDeviceRayTracingPropertiesNV()
{
}

void safe_VkPhysicalDeviceRayTracingPropertiesNV::initialize(const VkPhysicalDeviceRayTracingPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderGroupHandleSize = in_struct->shaderGroupHandleSize;
    maxRecursionDepth = in_struct->maxRecursionDepth;
    maxShaderGroupStride = in_struct->maxShaderGroupStride;
    shaderGroupBaseAlignment = in_struct->shaderGroupBaseAlignment;
    maxGeometryCount = in_struct->maxGeometryCount;
    maxInstanceCount = in_struct->maxInstanceCount;
    maxTriangleCount = in_struct->maxTriangleCount;
    maxDescriptorSetAccelerationStructures = in_struct->maxDescriptorSetAccelerationStructures;
}

void safe_VkPhysicalDeviceRayTracingPropertiesNV::initialize(const safe_VkPhysicalDeviceRayTracingPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderGroupHandleSize = src->shaderGroupHandleSize;
    maxRecursionDepth = src->maxRecursionDepth;
    maxShaderGroupStride = src->maxShaderGroupStride;
    shaderGroupBaseAlignment = src->shaderGroupBaseAlignment;
    maxGeometryCount = src->maxGeometryCount;
    maxInstanceCount = src->maxInstanceCount;
    maxTriangleCount = src->maxTriangleCount;
    maxDescriptorSetAccelerationStructures = src->maxDescriptorSetAccelerationStructures;
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    representativeFragmentTest(in_struct->representativeFragmentTest)
{
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV()
{}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    representativeFragmentTest = src.representativeFragmentTest;
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::operator=(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    representativeFragmentTest = src.representativeFragmentTest;

    return *this;
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::~safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV()
{
}

void safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::initialize(const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    representativeFragmentTest = in_struct->representativeFragmentTest;
}

void safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::initialize(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    representativeFragmentTest = src->representativeFragmentTest;
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    representativeFragmentTestEnable(in_struct->representativeFragmentTestEnable)
{
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV()
{}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    representativeFragmentTestEnable = src.representativeFragmentTestEnable;
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::operator=(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    representativeFragmentTestEnable = src.representativeFragmentTestEnable;

    return *this;
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::~safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV()
{
}

void safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::initialize(const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    representativeFragmentTestEnable = in_struct->representativeFragmentTestEnable;
}

void safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::initialize(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    representativeFragmentTestEnable = src->representativeFragmentTestEnable;
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    imageViewType(in_struct->imageViewType)
{
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT()
{}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    imageViewType = src.imageViewType;
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::operator=(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    imageViewType = src.imageViewType;

    return *this;
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::~safe_VkPhysicalDeviceImageViewImageFormatInfoEXT()
{
}

void safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::initialize(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    imageViewType = in_struct->imageViewType;
}

void safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::initialize(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    imageViewType = src->imageViewType;
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    filterCubic(in_struct->filterCubic),
    filterCubicMinmax(in_struct->filterCubicMinmax)
{
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT()
{}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    filterCubic = src.filterCubic;
    filterCubicMinmax = src.filterCubicMinmax;
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT& safe_VkFilterCubicImageViewImageFormatPropertiesEXT::operator=(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    filterCubic = src.filterCubic;
    filterCubicMinmax = src.filterCubicMinmax;

    return *this;
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::~safe_VkFilterCubicImageViewImageFormatPropertiesEXT()
{
}

void safe_VkFilterCubicImageViewImageFormatPropertiesEXT::initialize(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    filterCubic = in_struct->filterCubic;
    filterCubicMinmax = in_struct->filterCubicMinmax;
}

void safe_VkFilterCubicImageViewImageFormatPropertiesEXT::initialize(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    filterCubic = src->filterCubic;
    filterCubicMinmax = src->filterCubicMinmax;
}

safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    globalPriority(in_struct->globalPriority)
{
}

safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::safe_VkDeviceQueueGlobalPriorityCreateInfoEXT()
{}

safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    globalPriority = src.globalPriority;
}

safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::operator=(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    globalPriority = src.globalPriority;

    return *this;
}

safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::~safe_VkDeviceQueueGlobalPriorityCreateInfoEXT()
{
}

void safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::initialize(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    globalPriority = in_struct->globalPriority;
}

void safe_VkDeviceQueueGlobalPriorityCreateInfoEXT::initialize(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    globalPriority = src->globalPriority;
}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT(const VkImportMemoryHostPointerInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    handleType(in_struct->handleType),
    pHostPointer(in_struct->pHostPointer)
{
}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT()
{}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT(const safe_VkImportMemoryHostPointerInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    pHostPointer = src.pHostPointer;
}

safe_VkImportMemoryHostPointerInfoEXT& safe_VkImportMemoryHostPointerInfoEXT::operator=(const safe_VkImportMemoryHostPointerInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    handleType = src.handleType;
    pHostPointer = src.pHostPointer;

    return *this;
}

safe_VkImportMemoryHostPointerInfoEXT::~safe_VkImportMemoryHostPointerInfoEXT()
{
}

void safe_VkImportMemoryHostPointerInfoEXT::initialize(const VkImportMemoryHostPointerInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    handleType = in_struct->handleType;
    pHostPointer = in_struct->pHostPointer;
}

void safe_VkImportMemoryHostPointerInfoEXT::initialize(const safe_VkImportMemoryHostPointerInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    handleType = src->handleType;
    pHostPointer = src->pHostPointer;
}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT(const VkMemoryHostPointerPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryTypeBits(in_struct->memoryTypeBits)
{
}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT()
{}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT(const safe_VkMemoryHostPointerPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;
}

safe_VkMemoryHostPointerPropertiesEXT& safe_VkMemoryHostPointerPropertiesEXT::operator=(const safe_VkMemoryHostPointerPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryTypeBits = src.memoryTypeBits;

    return *this;
}

safe_VkMemoryHostPointerPropertiesEXT::~safe_VkMemoryHostPointerPropertiesEXT()
{
}

void safe_VkMemoryHostPointerPropertiesEXT::initialize(const VkMemoryHostPointerPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryTypeBits = in_struct->memoryTypeBits;
}

void safe_VkMemoryHostPointerPropertiesEXT::initialize(const safe_VkMemoryHostPointerPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryTypeBits = src->memoryTypeBits;
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    minImportedHostPointerAlignment(in_struct->minImportedHostPointerAlignment)
{
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT()
{}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    minImportedHostPointerAlignment = src.minImportedHostPointerAlignment;
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::operator=(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    minImportedHostPointerAlignment = src.minImportedHostPointerAlignment;

    return *this;
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::~safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT()
{
}

void safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::initialize(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    minImportedHostPointerAlignment = in_struct->minImportedHostPointerAlignment;
}

void safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::initialize(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    minImportedHostPointerAlignment = src->minImportedHostPointerAlignment;
}

safe_VkCalibratedTimestampInfoEXT::safe_VkCalibratedTimestampInfoEXT(const VkCalibratedTimestampInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    timeDomain(in_struct->timeDomain)
{
}

safe_VkCalibratedTimestampInfoEXT::safe_VkCalibratedTimestampInfoEXT()
{}

safe_VkCalibratedTimestampInfoEXT::safe_VkCalibratedTimestampInfoEXT(const safe_VkCalibratedTimestampInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    timeDomain = src.timeDomain;
}

safe_VkCalibratedTimestampInfoEXT& safe_VkCalibratedTimestampInfoEXT::operator=(const safe_VkCalibratedTimestampInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    timeDomain = src.timeDomain;

    return *this;
}

safe_VkCalibratedTimestampInfoEXT::~safe_VkCalibratedTimestampInfoEXT()
{
}

void safe_VkCalibratedTimestampInfoEXT::initialize(const VkCalibratedTimestampInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    timeDomain = in_struct->timeDomain;
}

void safe_VkCalibratedTimestampInfoEXT::initialize(const safe_VkCalibratedTimestampInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    timeDomain = src->timeDomain;
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD(const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderEngineCount(in_struct->shaderEngineCount),
    shaderArraysPerEngineCount(in_struct->shaderArraysPerEngineCount),
    computeUnitsPerShaderArray(in_struct->computeUnitsPerShaderArray),
    simdPerComputeUnit(in_struct->simdPerComputeUnit),
    wavefrontsPerSimd(in_struct->wavefrontsPerSimd),
    wavefrontSize(in_struct->wavefrontSize),
    sgprsPerSimd(in_struct->sgprsPerSimd),
    minSgprAllocation(in_struct->minSgprAllocation),
    maxSgprAllocation(in_struct->maxSgprAllocation),
    sgprAllocationGranularity(in_struct->sgprAllocationGranularity),
    vgprsPerSimd(in_struct->vgprsPerSimd),
    minVgprAllocation(in_struct->minVgprAllocation),
    maxVgprAllocation(in_struct->maxVgprAllocation),
    vgprAllocationGranularity(in_struct->vgprAllocationGranularity)
{
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD()
{}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD(const safe_VkPhysicalDeviceShaderCorePropertiesAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderEngineCount = src.shaderEngineCount;
    shaderArraysPerEngineCount = src.shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = src.computeUnitsPerShaderArray;
    simdPerComputeUnit = src.simdPerComputeUnit;
    wavefrontsPerSimd = src.wavefrontsPerSimd;
    wavefrontSize = src.wavefrontSize;
    sgprsPerSimd = src.sgprsPerSimd;
    minSgprAllocation = src.minSgprAllocation;
    maxSgprAllocation = src.maxSgprAllocation;
    sgprAllocationGranularity = src.sgprAllocationGranularity;
    vgprsPerSimd = src.vgprsPerSimd;
    minVgprAllocation = src.minVgprAllocation;
    maxVgprAllocation = src.maxVgprAllocation;
    vgprAllocationGranularity = src.vgprAllocationGranularity;
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD& safe_VkPhysicalDeviceShaderCorePropertiesAMD::operator=(const safe_VkPhysicalDeviceShaderCorePropertiesAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderEngineCount = src.shaderEngineCount;
    shaderArraysPerEngineCount = src.shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = src.computeUnitsPerShaderArray;
    simdPerComputeUnit = src.simdPerComputeUnit;
    wavefrontsPerSimd = src.wavefrontsPerSimd;
    wavefrontSize = src.wavefrontSize;
    sgprsPerSimd = src.sgprsPerSimd;
    minSgprAllocation = src.minSgprAllocation;
    maxSgprAllocation = src.maxSgprAllocation;
    sgprAllocationGranularity = src.sgprAllocationGranularity;
    vgprsPerSimd = src.vgprsPerSimd;
    minVgprAllocation = src.minVgprAllocation;
    maxVgprAllocation = src.maxVgprAllocation;
    vgprAllocationGranularity = src.vgprAllocationGranularity;

    return *this;
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::~safe_VkPhysicalDeviceShaderCorePropertiesAMD()
{
}

void safe_VkPhysicalDeviceShaderCorePropertiesAMD::initialize(const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderEngineCount = in_struct->shaderEngineCount;
    shaderArraysPerEngineCount = in_struct->shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = in_struct->computeUnitsPerShaderArray;
    simdPerComputeUnit = in_struct->simdPerComputeUnit;
    wavefrontsPerSimd = in_struct->wavefrontsPerSimd;
    wavefrontSize = in_struct->wavefrontSize;
    sgprsPerSimd = in_struct->sgprsPerSimd;
    minSgprAllocation = in_struct->minSgprAllocation;
    maxSgprAllocation = in_struct->maxSgprAllocation;
    sgprAllocationGranularity = in_struct->sgprAllocationGranularity;
    vgprsPerSimd = in_struct->vgprsPerSimd;
    minVgprAllocation = in_struct->minVgprAllocation;
    maxVgprAllocation = in_struct->maxVgprAllocation;
    vgprAllocationGranularity = in_struct->vgprAllocationGranularity;
}

void safe_VkPhysicalDeviceShaderCorePropertiesAMD::initialize(const safe_VkPhysicalDeviceShaderCorePropertiesAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderEngineCount = src->shaderEngineCount;
    shaderArraysPerEngineCount = src->shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = src->computeUnitsPerShaderArray;
    simdPerComputeUnit = src->simdPerComputeUnit;
    wavefrontsPerSimd = src->wavefrontsPerSimd;
    wavefrontSize = src->wavefrontSize;
    sgprsPerSimd = src->sgprsPerSimd;
    minSgprAllocation = src->minSgprAllocation;
    maxSgprAllocation = src->maxSgprAllocation;
    sgprAllocationGranularity = src->sgprAllocationGranularity;
    vgprsPerSimd = src->vgprsPerSimd;
    minVgprAllocation = src->minVgprAllocation;
    maxVgprAllocation = src->maxVgprAllocation;
    vgprAllocationGranularity = src->vgprAllocationGranularity;
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD(const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    overallocationBehavior(in_struct->overallocationBehavior)
{
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD()
{}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD(const safe_VkDeviceMemoryOverallocationCreateInfoAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    overallocationBehavior = src.overallocationBehavior;
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD& safe_VkDeviceMemoryOverallocationCreateInfoAMD::operator=(const safe_VkDeviceMemoryOverallocationCreateInfoAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    overallocationBehavior = src.overallocationBehavior;

    return *this;
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::~safe_VkDeviceMemoryOverallocationCreateInfoAMD()
{
}

void safe_VkDeviceMemoryOverallocationCreateInfoAMD::initialize(const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    overallocationBehavior = in_struct->overallocationBehavior;
}

void safe_VkDeviceMemoryOverallocationCreateInfoAMD::initialize(const safe_VkDeviceMemoryOverallocationCreateInfoAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    overallocationBehavior = src->overallocationBehavior;
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxVertexAttribDivisor(in_struct->maxVertexAttribDivisor)
{
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT()
{}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxVertexAttribDivisor = src.maxVertexAttribDivisor;
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxVertexAttribDivisor = src.maxVertexAttribDivisor;

    return *this;
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::~safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT()
{
}

void safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::initialize(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxVertexAttribDivisor = in_struct->maxVertexAttribDivisor;
}

void safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxVertexAttribDivisor = src->maxVertexAttribDivisor;
}

safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    vertexBindingDivisorCount(in_struct->vertexBindingDivisorCount),
    pVertexBindingDivisors(nullptr)
{
    if (in_struct->pVertexBindingDivisors) {
        pVertexBindingDivisors = new VkVertexInputBindingDivisorDescriptionEXT[in_struct->vertexBindingDivisorCount];
        memcpy ((void *)pVertexBindingDivisors, (void *)in_struct->pVertexBindingDivisors, sizeof(VkVertexInputBindingDivisorDescriptionEXT)*in_struct->vertexBindingDivisorCount);
    }
}

safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::safe_VkPipelineVertexInputDivisorStateCreateInfoEXT() :
    pVertexBindingDivisors(nullptr)
{}

safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    vertexBindingDivisorCount = src.vertexBindingDivisorCount;
    pVertexBindingDivisors = nullptr;
    if (src.pVertexBindingDivisors) {
        pVertexBindingDivisors = new VkVertexInputBindingDivisorDescriptionEXT[src.vertexBindingDivisorCount];
        memcpy ((void *)pVertexBindingDivisors, (void *)src.pVertexBindingDivisors, sizeof(VkVertexInputBindingDivisorDescriptionEXT)*src.vertexBindingDivisorCount);
    }
}

safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::operator=(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pVertexBindingDivisors)
        delete[] pVertexBindingDivisors;

    sType = src.sType;
    pNext = src.pNext;
    vertexBindingDivisorCount = src.vertexBindingDivisorCount;
    pVertexBindingDivisors = nullptr;
    if (src.pVertexBindingDivisors) {
        pVertexBindingDivisors = new VkVertexInputBindingDivisorDescriptionEXT[src.vertexBindingDivisorCount];
        memcpy ((void *)pVertexBindingDivisors, (void *)src.pVertexBindingDivisors, sizeof(VkVertexInputBindingDivisorDescriptionEXT)*src.vertexBindingDivisorCount);
    }

    return *this;
}

safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::~safe_VkPipelineVertexInputDivisorStateCreateInfoEXT()
{
    if (pVertexBindingDivisors)
        delete[] pVertexBindingDivisors;
}

void safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::initialize(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    vertexBindingDivisorCount = in_struct->vertexBindingDivisorCount;
    pVertexBindingDivisors = nullptr;
    if (in_struct->pVertexBindingDivisors) {
        pVertexBindingDivisors = new VkVertexInputBindingDivisorDescriptionEXT[in_struct->vertexBindingDivisorCount];
        memcpy ((void *)pVertexBindingDivisors, (void *)in_struct->pVertexBindingDivisors, sizeof(VkVertexInputBindingDivisorDescriptionEXT)*in_struct->vertexBindingDivisorCount);
    }
}

void safe_VkPipelineVertexInputDivisorStateCreateInfoEXT::initialize(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    vertexBindingDivisorCount = src->vertexBindingDivisorCount;
    pVertexBindingDivisors = nullptr;
    if (src->pVertexBindingDivisors) {
        pVertexBindingDivisors = new VkVertexInputBindingDivisorDescriptionEXT[src->vertexBindingDivisorCount];
        memcpy ((void *)pVertexBindingDivisors, (void *)src->pVertexBindingDivisors, sizeof(VkVertexInputBindingDivisorDescriptionEXT)*src->vertexBindingDivisorCount);
    }
}

safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    vertexAttributeInstanceRateDivisor(in_struct->vertexAttributeInstanceRateDivisor),
    vertexAttributeInstanceRateZeroDivisor(in_struct->vertexAttributeInstanceRateZeroDivisor)
{
}

safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT()
{}

safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    vertexAttributeInstanceRateDivisor = src.vertexAttributeInstanceRateDivisor;
    vertexAttributeInstanceRateZeroDivisor = src.vertexAttributeInstanceRateZeroDivisor;
}

safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    vertexAttributeInstanceRateDivisor = src.vertexAttributeInstanceRateDivisor;
    vertexAttributeInstanceRateZeroDivisor = src.vertexAttributeInstanceRateZeroDivisor;

    return *this;
}

safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::~safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT()
{
}

void safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::initialize(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    vertexAttributeInstanceRateDivisor = in_struct->vertexAttributeInstanceRateDivisor;
    vertexAttributeInstanceRateZeroDivisor = in_struct->vertexAttributeInstanceRateZeroDivisor;
}

void safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    vertexAttributeInstanceRateDivisor = src->vertexAttributeInstanceRateDivisor;
    vertexAttributeInstanceRateZeroDivisor = src->vertexAttributeInstanceRateZeroDivisor;
}
#ifdef VK_USE_PLATFORM_GGP


safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP(const VkPresentFrameTokenGGP* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    frameToken(in_struct->frameToken)
{
}

safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP()
{}

safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP(const safe_VkPresentFrameTokenGGP& src)
{
    sType = src.sType;
    pNext = src.pNext;
    frameToken = src.frameToken;
}

safe_VkPresentFrameTokenGGP& safe_VkPresentFrameTokenGGP::operator=(const safe_VkPresentFrameTokenGGP& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    frameToken = src.frameToken;

    return *this;
}

safe_VkPresentFrameTokenGGP::~safe_VkPresentFrameTokenGGP()
{
}

void safe_VkPresentFrameTokenGGP::initialize(const VkPresentFrameTokenGGP* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    frameToken = in_struct->frameToken;
}

void safe_VkPresentFrameTokenGGP::initialize(const safe_VkPresentFrameTokenGGP* src)
{
    sType = src->sType;
    pNext = src->pNext;
    frameToken = src->frameToken;
}
#endif // VK_USE_PLATFORM_GGP


safe_VkPipelineCreationFeedbackCreateInfoEXT::safe_VkPipelineCreationFeedbackCreateInfoEXT(const VkPipelineCreationFeedbackCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pPipelineCreationFeedback(nullptr),
    pipelineStageCreationFeedbackCount(in_struct->pipelineStageCreationFeedbackCount),
    pPipelineStageCreationFeedbacks(nullptr)
{
    if (in_struct->pPipelineCreationFeedback) {
        pPipelineCreationFeedback = new VkPipelineCreationFeedbackEXT(*in_struct->pPipelineCreationFeedback);
    }
    if (in_struct->pPipelineStageCreationFeedbacks) {
        pPipelineStageCreationFeedbacks = new VkPipelineCreationFeedbackEXT[in_struct->pipelineStageCreationFeedbackCount];
        memcpy ((void *)pPipelineStageCreationFeedbacks, (void *)in_struct->pPipelineStageCreationFeedbacks, sizeof(VkPipelineCreationFeedbackEXT)*in_struct->pipelineStageCreationFeedbackCount);
    }
}

safe_VkPipelineCreationFeedbackCreateInfoEXT::safe_VkPipelineCreationFeedbackCreateInfoEXT() :
    pPipelineCreationFeedback(nullptr),
    pPipelineStageCreationFeedbacks(nullptr)
{}

safe_VkPipelineCreationFeedbackCreateInfoEXT::safe_VkPipelineCreationFeedbackCreateInfoEXT(const safe_VkPipelineCreationFeedbackCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pPipelineCreationFeedback = nullptr;
    pipelineStageCreationFeedbackCount = src.pipelineStageCreationFeedbackCount;
    pPipelineStageCreationFeedbacks = nullptr;
    if (src.pPipelineCreationFeedback) {
        pPipelineCreationFeedback = new VkPipelineCreationFeedbackEXT(*src.pPipelineCreationFeedback);
    }
    if (src.pPipelineStageCreationFeedbacks) {
        pPipelineStageCreationFeedbacks = new VkPipelineCreationFeedbackEXT[src.pipelineStageCreationFeedbackCount];
        memcpy ((void *)pPipelineStageCreationFeedbacks, (void *)src.pPipelineStageCreationFeedbacks, sizeof(VkPipelineCreationFeedbackEXT)*src.pipelineStageCreationFeedbackCount);
    }
}

safe_VkPipelineCreationFeedbackCreateInfoEXT& safe_VkPipelineCreationFeedbackCreateInfoEXT::operator=(const safe_VkPipelineCreationFeedbackCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pPipelineCreationFeedback)
        delete pPipelineCreationFeedback;
    if (pPipelineStageCreationFeedbacks)
        delete[] pPipelineStageCreationFeedbacks;

    sType = src.sType;
    pNext = src.pNext;
    pPipelineCreationFeedback = nullptr;
    pipelineStageCreationFeedbackCount = src.pipelineStageCreationFeedbackCount;
    pPipelineStageCreationFeedbacks = nullptr;
    if (src.pPipelineCreationFeedback) {
        pPipelineCreationFeedback = new VkPipelineCreationFeedbackEXT(*src.pPipelineCreationFeedback);
    }
    if (src.pPipelineStageCreationFeedbacks) {
        pPipelineStageCreationFeedbacks = new VkPipelineCreationFeedbackEXT[src.pipelineStageCreationFeedbackCount];
        memcpy ((void *)pPipelineStageCreationFeedbacks, (void *)src.pPipelineStageCreationFeedbacks, sizeof(VkPipelineCreationFeedbackEXT)*src.pipelineStageCreationFeedbackCount);
    }

    return *this;
}

safe_VkPipelineCreationFeedbackCreateInfoEXT::~safe_VkPipelineCreationFeedbackCreateInfoEXT()
{
    if (pPipelineCreationFeedback)
        delete pPipelineCreationFeedback;
    if (pPipelineStageCreationFeedbacks)
        delete[] pPipelineStageCreationFeedbacks;
}

void safe_VkPipelineCreationFeedbackCreateInfoEXT::initialize(const VkPipelineCreationFeedbackCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pPipelineCreationFeedback = nullptr;
    pipelineStageCreationFeedbackCount = in_struct->pipelineStageCreationFeedbackCount;
    pPipelineStageCreationFeedbacks = nullptr;
    if (in_struct->pPipelineCreationFeedback) {
        pPipelineCreationFeedback = new VkPipelineCreationFeedbackEXT(*in_struct->pPipelineCreationFeedback);
    }
    if (in_struct->pPipelineStageCreationFeedbacks) {
        pPipelineStageCreationFeedbacks = new VkPipelineCreationFeedbackEXT[in_struct->pipelineStageCreationFeedbackCount];
        memcpy ((void *)pPipelineStageCreationFeedbacks, (void *)in_struct->pPipelineStageCreationFeedbacks, sizeof(VkPipelineCreationFeedbackEXT)*in_struct->pipelineStageCreationFeedbackCount);
    }
}

void safe_VkPipelineCreationFeedbackCreateInfoEXT::initialize(const safe_VkPipelineCreationFeedbackCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pPipelineCreationFeedback = nullptr;
    pipelineStageCreationFeedbackCount = src->pipelineStageCreationFeedbackCount;
    pPipelineStageCreationFeedbacks = nullptr;
    if (src->pPipelineCreationFeedback) {
        pPipelineCreationFeedback = new VkPipelineCreationFeedbackEXT(*src->pPipelineCreationFeedback);
    }
    if (src->pPipelineStageCreationFeedbacks) {
        pPipelineStageCreationFeedbacks = new VkPipelineCreationFeedbackEXT[src->pipelineStageCreationFeedbackCount];
        memcpy ((void *)pPipelineStageCreationFeedbacks, (void *)src->pPipelineStageCreationFeedbacks, sizeof(VkPipelineCreationFeedbackEXT)*src->pipelineStageCreationFeedbackCount);
    }
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    computeDerivativeGroupQuads(in_struct->computeDerivativeGroupQuads),
    computeDerivativeGroupLinear(in_struct->computeDerivativeGroupLinear)
{
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV()
{}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    computeDerivativeGroupQuads = src.computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = src.computeDerivativeGroupLinear;
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::operator=(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    computeDerivativeGroupQuads = src.computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = src.computeDerivativeGroupLinear;

    return *this;
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::~safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV()
{
}

void safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::initialize(const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    computeDerivativeGroupQuads = in_struct->computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = in_struct->computeDerivativeGroupLinear;
}

void safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::initialize(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    computeDerivativeGroupQuads = src->computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = src->computeDerivativeGroupLinear;
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV(const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    taskShader(in_struct->taskShader),
    meshShader(in_struct->meshShader)
{
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV()
{}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV(const safe_VkPhysicalDeviceMeshShaderFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    taskShader = src.taskShader;
    meshShader = src.meshShader;
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV& safe_VkPhysicalDeviceMeshShaderFeaturesNV::operator=(const safe_VkPhysicalDeviceMeshShaderFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    taskShader = src.taskShader;
    meshShader = src.meshShader;

    return *this;
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::~safe_VkPhysicalDeviceMeshShaderFeaturesNV()
{
}

void safe_VkPhysicalDeviceMeshShaderFeaturesNV::initialize(const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    taskShader = in_struct->taskShader;
    meshShader = in_struct->meshShader;
}

void safe_VkPhysicalDeviceMeshShaderFeaturesNV::initialize(const safe_VkPhysicalDeviceMeshShaderFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    taskShader = src->taskShader;
    meshShader = src->meshShader;
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV(const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    maxDrawMeshTasksCount(in_struct->maxDrawMeshTasksCount),
    maxTaskWorkGroupInvocations(in_struct->maxTaskWorkGroupInvocations),
    maxTaskTotalMemorySize(in_struct->maxTaskTotalMemorySize),
    maxTaskOutputCount(in_struct->maxTaskOutputCount),
    maxMeshWorkGroupInvocations(in_struct->maxMeshWorkGroupInvocations),
    maxMeshTotalMemorySize(in_struct->maxMeshTotalMemorySize),
    maxMeshOutputVertices(in_struct->maxMeshOutputVertices),
    maxMeshOutputPrimitives(in_struct->maxMeshOutputPrimitives),
    maxMeshMultiviewViewCount(in_struct->maxMeshMultiviewViewCount),
    meshOutputPerVertexGranularity(in_struct->meshOutputPerVertexGranularity),
    meshOutputPerPrimitiveGranularity(in_struct->meshOutputPerPrimitiveGranularity)
{
    for (uint32_t i=0; i<3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }
    for (uint32_t i=0; i<3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV()
{}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV(const safe_VkPhysicalDeviceMeshShaderPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    maxDrawMeshTasksCount = src.maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = src.maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = src.maxTaskTotalMemorySize;
    maxTaskOutputCount = src.maxTaskOutputCount;
    maxMeshWorkGroupInvocations = src.maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = src.maxMeshTotalMemorySize;
    maxMeshOutputVertices = src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = src.maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = src.meshOutputPerPrimitiveGranularity;
    for (uint32_t i=0; i<3; ++i) {
        maxTaskWorkGroupSize[i] = src.maxTaskWorkGroupSize[i];
    }
    for (uint32_t i=0; i<3; ++i) {
        maxMeshWorkGroupSize[i] = src.maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV& safe_VkPhysicalDeviceMeshShaderPropertiesNV::operator=(const safe_VkPhysicalDeviceMeshShaderPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    maxDrawMeshTasksCount = src.maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = src.maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = src.maxTaskTotalMemorySize;
    maxTaskOutputCount = src.maxTaskOutputCount;
    maxMeshWorkGroupInvocations = src.maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = src.maxMeshTotalMemorySize;
    maxMeshOutputVertices = src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = src.maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = src.meshOutputPerPrimitiveGranularity;
    for (uint32_t i=0; i<3; ++i) {
        maxTaskWorkGroupSize[i] = src.maxTaskWorkGroupSize[i];
    }
    for (uint32_t i=0; i<3; ++i) {
        maxMeshWorkGroupSize[i] = src.maxMeshWorkGroupSize[i];
    }

    return *this;
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::~safe_VkPhysicalDeviceMeshShaderPropertiesNV()
{
}

void safe_VkPhysicalDeviceMeshShaderPropertiesNV::initialize(const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    maxDrawMeshTasksCount = in_struct->maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = in_struct->maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = in_struct->maxTaskTotalMemorySize;
    maxTaskOutputCount = in_struct->maxTaskOutputCount;
    maxMeshWorkGroupInvocations = in_struct->maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = in_struct->maxMeshTotalMemorySize;
    maxMeshOutputVertices = in_struct->maxMeshOutputVertices;
    maxMeshOutputPrimitives = in_struct->maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = in_struct->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = in_struct->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = in_struct->meshOutputPerPrimitiveGranularity;
    for (uint32_t i=0; i<3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }
    for (uint32_t i=0; i<3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

void safe_VkPhysicalDeviceMeshShaderPropertiesNV::initialize(const safe_VkPhysicalDeviceMeshShaderPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    maxDrawMeshTasksCount = src->maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = src->maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = src->maxTaskTotalMemorySize;
    maxTaskOutputCount = src->maxTaskOutputCount;
    maxMeshWorkGroupInvocations = src->maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = src->maxMeshTotalMemorySize;
    maxMeshOutputVertices = src->maxMeshOutputVertices;
    maxMeshOutputPrimitives = src->maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = src->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = src->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = src->meshOutputPerPrimitiveGranularity;
    for (uint32_t i=0; i<3; ++i) {
        maxTaskWorkGroupSize[i] = src->maxTaskWorkGroupSize[i];
    }
    for (uint32_t i=0; i<3; ++i) {
        maxMeshWorkGroupSize[i] = src->maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fragmentShaderBarycentric(in_struct->fragmentShaderBarycentric)
{
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV()
{}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fragmentShaderBarycentric = src.fragmentShaderBarycentric;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::operator=(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fragmentShaderBarycentric = src.fragmentShaderBarycentric;

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::~safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV()
{
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::initialize(const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fragmentShaderBarycentric = in_struct->fragmentShaderBarycentric;
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::initialize(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fragmentShaderBarycentric = src->fragmentShaderBarycentric;
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    imageFootprint(in_struct->imageFootprint)
{
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV()
{}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    imageFootprint = src.imageFootprint;
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::operator=(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    imageFootprint = src.imageFootprint;

    return *this;
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::~safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV()
{
}

void safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::initialize(const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    imageFootprint = in_struct->imageFootprint;
}

void safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::initialize(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    imageFootprint = src->imageFootprint;
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    exclusiveScissorCount(in_struct->exclusiveScissorCount),
    pExclusiveScissors(nullptr)
{
    if (in_struct->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[in_struct->exclusiveScissorCount];
        memcpy ((void *)pExclusiveScissors, (void *)in_struct->pExclusiveScissors, sizeof(VkRect2D)*in_struct->exclusiveScissorCount);
    }
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV() :
    pExclusiveScissors(nullptr)
{}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    exclusiveScissorCount = src.exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    if (src.pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[src.exclusiveScissorCount];
        memcpy ((void *)pExclusiveScissors, (void *)src.pExclusiveScissors, sizeof(VkRect2D)*src.exclusiveScissorCount);
    }
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::operator=(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& src)
{
    if (&src == this) return *this;

    if (pExclusiveScissors)
        delete[] pExclusiveScissors;

    sType = src.sType;
    pNext = src.pNext;
    exclusiveScissorCount = src.exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    if (src.pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[src.exclusiveScissorCount];
        memcpy ((void *)pExclusiveScissors, (void *)src.pExclusiveScissors, sizeof(VkRect2D)*src.exclusiveScissorCount);
    }

    return *this;
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::~safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV()
{
    if (pExclusiveScissors)
        delete[] pExclusiveScissors;
}

void safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::initialize(const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    exclusiveScissorCount = in_struct->exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    if (in_struct->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[in_struct->exclusiveScissorCount];
        memcpy ((void *)pExclusiveScissors, (void *)in_struct->pExclusiveScissors, sizeof(VkRect2D)*in_struct->exclusiveScissorCount);
    }
}

void safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::initialize(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    exclusiveScissorCount = src->exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    if (src->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[src->exclusiveScissorCount];
        memcpy ((void *)pExclusiveScissors, (void *)src->pExclusiveScissors, sizeof(VkRect2D)*src->exclusiveScissorCount);
    }
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    exclusiveScissor(in_struct->exclusiveScissor)
{
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV()
{}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    exclusiveScissor = src.exclusiveScissor;
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::operator=(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    exclusiveScissor = src.exclusiveScissor;

    return *this;
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::~safe_VkPhysicalDeviceExclusiveScissorFeaturesNV()
{
}

void safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::initialize(const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    exclusiveScissor = in_struct->exclusiveScissor;
}

void safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::initialize(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    exclusiveScissor = src->exclusiveScissor;
}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV(const VkQueueFamilyCheckpointPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    checkpointExecutionStageMask(in_struct->checkpointExecutionStageMask)
{
}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV()
{}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV(const safe_VkQueueFamilyCheckpointPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    checkpointExecutionStageMask = src.checkpointExecutionStageMask;
}

safe_VkQueueFamilyCheckpointPropertiesNV& safe_VkQueueFamilyCheckpointPropertiesNV::operator=(const safe_VkQueueFamilyCheckpointPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    checkpointExecutionStageMask = src.checkpointExecutionStageMask;

    return *this;
}

safe_VkQueueFamilyCheckpointPropertiesNV::~safe_VkQueueFamilyCheckpointPropertiesNV()
{
}

void safe_VkQueueFamilyCheckpointPropertiesNV::initialize(const VkQueueFamilyCheckpointPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    checkpointExecutionStageMask = in_struct->checkpointExecutionStageMask;
}

void safe_VkQueueFamilyCheckpointPropertiesNV::initialize(const safe_VkQueueFamilyCheckpointPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    checkpointExecutionStageMask = src->checkpointExecutionStageMask;
}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV(const VkCheckpointDataNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    stage(in_struct->stage),
    pCheckpointMarker(in_struct->pCheckpointMarker)
{
}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV()
{}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV(const safe_VkCheckpointDataNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    stage = src.stage;
    pCheckpointMarker = src.pCheckpointMarker;
}

safe_VkCheckpointDataNV& safe_VkCheckpointDataNV::operator=(const safe_VkCheckpointDataNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    stage = src.stage;
    pCheckpointMarker = src.pCheckpointMarker;

    return *this;
}

safe_VkCheckpointDataNV::~safe_VkCheckpointDataNV()
{
}

void safe_VkCheckpointDataNV::initialize(const VkCheckpointDataNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    stage = in_struct->stage;
    pCheckpointMarker = in_struct->pCheckpointMarker;
}

void safe_VkCheckpointDataNV::initialize(const safe_VkCheckpointDataNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    stage = src->stage;
    pCheckpointMarker = src->pCheckpointMarker;
}

safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL(const VkPhysicalDeviceShaderIntegerFunctions2INTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    shaderIntegerFunctions2(in_struct->shaderIntegerFunctions2)
{
}

safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL()
{}

safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL(const safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    shaderIntegerFunctions2 = src.shaderIntegerFunctions2;
}

safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL& safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::operator=(const safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    shaderIntegerFunctions2 = src.shaderIntegerFunctions2;

    return *this;
}

safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::~safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL()
{
}

void safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::initialize(const VkPhysicalDeviceShaderIntegerFunctions2INTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    shaderIntegerFunctions2 = in_struct->shaderIntegerFunctions2;
}

void safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL::initialize(const safe_VkPhysicalDeviceShaderIntegerFunctions2INTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    shaderIntegerFunctions2 = src->shaderIntegerFunctions2;
}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL(const VkPerformanceValueDataINTEL* in_struct) :
    value32(in_struct->value32),
    value64(in_struct->value64),
    valueFloat(in_struct->valueFloat),
    valueBool(in_struct->valueBool),
    valueString(in_struct->valueString)
{
}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL()
{}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL(const safe_VkPerformanceValueDataINTEL& src)
{
    value32 = src.value32;
    value64 = src.value64;
    valueFloat = src.valueFloat;
    valueBool = src.valueBool;
    valueString = src.valueString;
}

safe_VkPerformanceValueDataINTEL& safe_VkPerformanceValueDataINTEL::operator=(const safe_VkPerformanceValueDataINTEL& src)
{
    if (&src == this) return *this;


    value32 = src.value32;
    value64 = src.value64;
    valueFloat = src.valueFloat;
    valueBool = src.valueBool;
    valueString = src.valueString;

    return *this;
}

safe_VkPerformanceValueDataINTEL::~safe_VkPerformanceValueDataINTEL()
{
}

void safe_VkPerformanceValueDataINTEL::initialize(const VkPerformanceValueDataINTEL* in_struct)
{
    value32 = in_struct->value32;
    value64 = in_struct->value64;
    valueFloat = in_struct->valueFloat;
    valueBool = in_struct->valueBool;
    valueString = in_struct->valueString;
}

void safe_VkPerformanceValueDataINTEL::initialize(const safe_VkPerformanceValueDataINTEL* src)
{
    value32 = src->value32;
    value64 = src->value64;
    valueFloat = src->valueFloat;
    valueBool = src->valueBool;
    valueString = src->valueString;
}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL(const VkInitializePerformanceApiInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pUserData(in_struct->pUserData)
{
}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL()
{}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL(const safe_VkInitializePerformanceApiInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pUserData = src.pUserData;
}

safe_VkInitializePerformanceApiInfoINTEL& safe_VkInitializePerformanceApiInfoINTEL::operator=(const safe_VkInitializePerformanceApiInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pUserData = src.pUserData;

    return *this;
}

safe_VkInitializePerformanceApiInfoINTEL::~safe_VkInitializePerformanceApiInfoINTEL()
{
}

void safe_VkInitializePerformanceApiInfoINTEL::initialize(const VkInitializePerformanceApiInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pUserData = in_struct->pUserData;
}

void safe_VkInitializePerformanceApiInfoINTEL::initialize(const safe_VkInitializePerformanceApiInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pUserData = src->pUserData;
}

safe_VkQueryPoolCreateInfoINTEL::safe_VkQueryPoolCreateInfoINTEL(const VkQueryPoolCreateInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    performanceCountersSampling(in_struct->performanceCountersSampling)
{
}

safe_VkQueryPoolCreateInfoINTEL::safe_VkQueryPoolCreateInfoINTEL()
{}

safe_VkQueryPoolCreateInfoINTEL::safe_VkQueryPoolCreateInfoINTEL(const safe_VkQueryPoolCreateInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    performanceCountersSampling = src.performanceCountersSampling;
}

safe_VkQueryPoolCreateInfoINTEL& safe_VkQueryPoolCreateInfoINTEL::operator=(const safe_VkQueryPoolCreateInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    performanceCountersSampling = src.performanceCountersSampling;

    return *this;
}

safe_VkQueryPoolCreateInfoINTEL::~safe_VkQueryPoolCreateInfoINTEL()
{
}

void safe_VkQueryPoolCreateInfoINTEL::initialize(const VkQueryPoolCreateInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    performanceCountersSampling = in_struct->performanceCountersSampling;
}

void safe_VkQueryPoolCreateInfoINTEL::initialize(const safe_VkQueryPoolCreateInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    performanceCountersSampling = src->performanceCountersSampling;
}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL(const VkPerformanceMarkerInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    marker(in_struct->marker)
{
}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL()
{}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL(const safe_VkPerformanceMarkerInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    marker = src.marker;
}

safe_VkPerformanceMarkerInfoINTEL& safe_VkPerformanceMarkerInfoINTEL::operator=(const safe_VkPerformanceMarkerInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    marker = src.marker;

    return *this;
}

safe_VkPerformanceMarkerInfoINTEL::~safe_VkPerformanceMarkerInfoINTEL()
{
}

void safe_VkPerformanceMarkerInfoINTEL::initialize(const VkPerformanceMarkerInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    marker = in_struct->marker;
}

void safe_VkPerformanceMarkerInfoINTEL::initialize(const safe_VkPerformanceMarkerInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    marker = src->marker;
}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL(const VkPerformanceStreamMarkerInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    marker(in_struct->marker)
{
}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL()
{}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL(const safe_VkPerformanceStreamMarkerInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    marker = src.marker;
}

safe_VkPerformanceStreamMarkerInfoINTEL& safe_VkPerformanceStreamMarkerInfoINTEL::operator=(const safe_VkPerformanceStreamMarkerInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    marker = src.marker;

    return *this;
}

safe_VkPerformanceStreamMarkerInfoINTEL::~safe_VkPerformanceStreamMarkerInfoINTEL()
{
}

void safe_VkPerformanceStreamMarkerInfoINTEL::initialize(const VkPerformanceStreamMarkerInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    marker = in_struct->marker;
}

void safe_VkPerformanceStreamMarkerInfoINTEL::initialize(const safe_VkPerformanceStreamMarkerInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    marker = src->marker;
}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL(const VkPerformanceOverrideInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    type(in_struct->type),
    enable(in_struct->enable),
    parameter(in_struct->parameter)
{
}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL()
{}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL(const safe_VkPerformanceOverrideInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    enable = src.enable;
    parameter = src.parameter;
}

safe_VkPerformanceOverrideInfoINTEL& safe_VkPerformanceOverrideInfoINTEL::operator=(const safe_VkPerformanceOverrideInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
    enable = src.enable;
    parameter = src.parameter;

    return *this;
}

safe_VkPerformanceOverrideInfoINTEL::~safe_VkPerformanceOverrideInfoINTEL()
{
}

void safe_VkPerformanceOverrideInfoINTEL::initialize(const VkPerformanceOverrideInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    type = in_struct->type;
    enable = in_struct->enable;
    parameter = in_struct->parameter;
}

void safe_VkPerformanceOverrideInfoINTEL::initialize(const safe_VkPerformanceOverrideInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    type = src->type;
    enable = src->enable;
    parameter = src->parameter;
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL(const VkPerformanceConfigurationAcquireInfoINTEL* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    type(in_struct->type)
{
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL()
{}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL(const safe_VkPerformanceConfigurationAcquireInfoINTEL& src)
{
    sType = src.sType;
    pNext = src.pNext;
    type = src.type;
}

safe_VkPerformanceConfigurationAcquireInfoINTEL& safe_VkPerformanceConfigurationAcquireInfoINTEL::operator=(const safe_VkPerformanceConfigurationAcquireInfoINTEL& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    type = src.type;

    return *this;
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::~safe_VkPerformanceConfigurationAcquireInfoINTEL()
{
}

void safe_VkPerformanceConfigurationAcquireInfoINTEL::initialize(const VkPerformanceConfigurationAcquireInfoINTEL* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    type = in_struct->type;
}

void safe_VkPerformanceConfigurationAcquireInfoINTEL::initialize(const safe_VkPerformanceConfigurationAcquireInfoINTEL* src)
{
    sType = src->sType;
    pNext = src->pNext;
    type = src->type;
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    pciDomain(in_struct->pciDomain),
    pciBus(in_struct->pciBus),
    pciDevice(in_struct->pciDevice),
    pciFunction(in_struct->pciFunction)
{
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT()
{}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    pciDomain = src.pciDomain;
    pciBus = src.pciBus;
    pciDevice = src.pciDevice;
    pciFunction = src.pciFunction;
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::operator=(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    pciDomain = src.pciDomain;
    pciBus = src.pciBus;
    pciDevice = src.pciDevice;
    pciFunction = src.pciFunction;

    return *this;
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::~safe_VkPhysicalDevicePCIBusInfoPropertiesEXT()
{
}

void safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::initialize(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    pciDomain = in_struct->pciDomain;
    pciBus = in_struct->pciBus;
    pciDevice = in_struct->pciDevice;
    pciFunction = in_struct->pciFunction;
}

void safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::initialize(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    pciDomain = src->pciDomain;
    pciBus = src->pciBus;
    pciDevice = src->pciDevice;
    pciFunction = src->pciFunction;
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    localDimmingSupport(in_struct->localDimmingSupport)
{
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD()
{}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    localDimmingSupport = src.localDimmingSupport;
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::operator=(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    localDimmingSupport = src.localDimmingSupport;

    return *this;
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::~safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD()
{
}

void safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::initialize(const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    localDimmingSupport = in_struct->localDimmingSupport;
}

void safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::initialize(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    localDimmingSupport = src->localDimmingSupport;
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    localDimmingEnable(in_struct->localDimmingEnable)
{
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD()
{}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& src)
{
    sType = src.sType;
    pNext = src.pNext;
    localDimmingEnable = src.localDimmingEnable;
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::operator=(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    localDimmingEnable = src.localDimmingEnable;

    return *this;
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::~safe_VkSwapchainDisplayNativeHdrCreateInfoAMD()
{
}

void safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::initialize(const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    localDimmingEnable = in_struct->localDimmingEnable;
}

void safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::initialize(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD* src)
{
    sType = src->sType;
    pNext = src->pNext;
    localDimmingEnable = src->localDimmingEnable;
}
#ifdef VK_USE_PLATFORM_FUCHSIA


safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA(const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    imagePipeHandle(in_struct->imagePipeHandle)
{
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA()
{}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    imagePipeHandle = src.imagePipeHandle;
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA& safe_VkImagePipeSurfaceCreateInfoFUCHSIA::operator=(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    imagePipeHandle = src.imagePipeHandle;

    return *this;
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::~safe_VkImagePipeSurfaceCreateInfoFUCHSIA()
{
}

void safe_VkImagePipeSurfaceCreateInfoFUCHSIA::initialize(const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    imagePipeHandle = in_struct->imagePipeHandle;
}

void safe_VkImagePipeSurfaceCreateInfoFUCHSIA::initialize(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    imagePipeHandle = src->imagePipeHandle;
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT


safe_VkMetalSurfaceCreateInfoEXT::safe_VkMetalSurfaceCreateInfoEXT(const VkMetalSurfaceCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    pLayer(nullptr)
{
    if (in_struct->pLayer) {
        pLayer = new CAMetalLayer(*in_struct->pLayer);
    }
}

safe_VkMetalSurfaceCreateInfoEXT::safe_VkMetalSurfaceCreateInfoEXT() :
    pLayer(nullptr)
{}

safe_VkMetalSurfaceCreateInfoEXT::safe_VkMetalSurfaceCreateInfoEXT(const safe_VkMetalSurfaceCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pLayer = nullptr;
    if (src.pLayer) {
        pLayer = new CAMetalLayer(*src.pLayer);
    }
}

safe_VkMetalSurfaceCreateInfoEXT& safe_VkMetalSurfaceCreateInfoEXT::operator=(const safe_VkMetalSurfaceCreateInfoEXT& src)
{
    if (&src == this) return *this;

    if (pLayer)
        delete pLayer;

    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    pLayer = nullptr;
    if (src.pLayer) {
        pLayer = new CAMetalLayer(*src.pLayer);
    }

    return *this;
}

safe_VkMetalSurfaceCreateInfoEXT::~safe_VkMetalSurfaceCreateInfoEXT()
{
    if (pLayer)
        delete pLayer;
}

void safe_VkMetalSurfaceCreateInfoEXT::initialize(const VkMetalSurfaceCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    pLayer = nullptr;
    if (in_struct->pLayer) {
        pLayer = new CAMetalLayer(*in_struct->pLayer);
    }
}

void safe_VkMetalSurfaceCreateInfoEXT::initialize(const safe_VkMetalSurfaceCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    pLayer = nullptr;
    if (src->pLayer) {
        pLayer = new CAMetalLayer(*src->pLayer);
    }
}
#endif // VK_USE_PLATFORM_METAL_EXT


safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fragmentDensityMap(in_struct->fragmentDensityMap),
    fragmentDensityMapDynamic(in_struct->fragmentDensityMapDynamic),
    fragmentDensityMapNonSubsampledImages(in_struct->fragmentDensityMapNonSubsampledImages)
{
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT()
{}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fragmentDensityMap = src.fragmentDensityMap;
    fragmentDensityMapDynamic = src.fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = src.fragmentDensityMapNonSubsampledImages;
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::operator=(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fragmentDensityMap = src.fragmentDensityMap;
    fragmentDensityMapDynamic = src.fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = src.fragmentDensityMapNonSubsampledImages;

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::~safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT()
{
}

void safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::initialize(const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fragmentDensityMap = in_struct->fragmentDensityMap;
    fragmentDensityMapDynamic = in_struct->fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = in_struct->fragmentDensityMapNonSubsampledImages;
}

void safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::initialize(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fragmentDensityMap = src->fragmentDensityMap;
    fragmentDensityMapDynamic = src->fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = src->fragmentDensityMapNonSubsampledImages;
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    minFragmentDensityTexelSize(in_struct->minFragmentDensityTexelSize),
    maxFragmentDensityTexelSize(in_struct->maxFragmentDensityTexelSize),
    fragmentDensityInvocations(in_struct->fragmentDensityInvocations)
{
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT()
{}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    minFragmentDensityTexelSize = src.minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = src.maxFragmentDensityTexelSize;
    fragmentDensityInvocations = src.fragmentDensityInvocations;
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::operator=(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    minFragmentDensityTexelSize = src.minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = src.maxFragmentDensityTexelSize;
    fragmentDensityInvocations = src.fragmentDensityInvocations;

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::~safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT()
{
}

void safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::initialize(const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    minFragmentDensityTexelSize = in_struct->minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = in_struct->maxFragmentDensityTexelSize;
    fragmentDensityInvocations = in_struct->fragmentDensityInvocations;
}

void safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::initialize(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    minFragmentDensityTexelSize = src->minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = src->maxFragmentDensityTexelSize;
    fragmentDensityInvocations = src->fragmentDensityInvocations;
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT(const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fragmentDensityMapAttachment(in_struct->fragmentDensityMapAttachment)
{
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT()
{}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fragmentDensityMapAttachment = src.fragmentDensityMapAttachment;
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT& safe_VkRenderPassFragmentDensityMapCreateInfoEXT::operator=(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fragmentDensityMapAttachment = src.fragmentDensityMapAttachment;

    return *this;
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::~safe_VkRenderPassFragmentDensityMapCreateInfoEXT()
{
}

void safe_VkRenderPassFragmentDensityMapCreateInfoEXT::initialize(const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fragmentDensityMapAttachment = in_struct->fragmentDensityMapAttachment;
}

void safe_VkRenderPassFragmentDensityMapCreateInfoEXT::initialize(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fragmentDensityMapAttachment = src->fragmentDensityMapAttachment;
}

safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT(const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    scalarBlockLayout(in_struct->scalarBlockLayout)
{
}

safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT()
{}

safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    scalarBlockLayout = src.scalarBlockLayout;
}

safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::operator=(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    scalarBlockLayout = src.scalarBlockLayout;

    return *this;
}

safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::~safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT()
{
}

void safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::initialize(const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    scalarBlockLayout = in_struct->scalarBlockLayout;
}

void safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT::initialize(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    scalarBlockLayout = src->scalarBlockLayout;
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext)
{
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = in_struct->heapBudget[i];
    }
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = in_struct->heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT()
{}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = src.heapBudget[i];
    }
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = src.heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::operator=(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = src.heapBudget[i];
    }
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = src.heapUsage[i];
    }

    return *this;
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::~safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT()
{
}

void safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::initialize(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = in_struct->heapBudget[i];
    }
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = in_struct->heapUsage[i];
    }
}

void safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::initialize(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = src->heapBudget[i];
    }
    for (uint32_t i=0; i<VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = src->heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    memoryPriority(in_struct->memoryPriority)
{
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT()
{}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    memoryPriority = src.memoryPriority;
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::operator=(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    memoryPriority = src.memoryPriority;

    return *this;
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::~safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT()
{
}

void safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::initialize(const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    memoryPriority = in_struct->memoryPriority;
}

void safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::initialize(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    memoryPriority = src->memoryPriority;
}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT(const VkMemoryPriorityAllocateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    priority(in_struct->priority)
{
}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT()
{}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT(const safe_VkMemoryPriorityAllocateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    priority = src.priority;
}

safe_VkMemoryPriorityAllocateInfoEXT& safe_VkMemoryPriorityAllocateInfoEXT::operator=(const safe_VkMemoryPriorityAllocateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    priority = src.priority;

    return *this;
}

safe_VkMemoryPriorityAllocateInfoEXT::~safe_VkMemoryPriorityAllocateInfoEXT()
{
}

void safe_VkMemoryPriorityAllocateInfoEXT::initialize(const VkMemoryPriorityAllocateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    priority = in_struct->priority;
}

void safe_VkMemoryPriorityAllocateInfoEXT::initialize(const safe_VkMemoryPriorityAllocateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    priority = src->priority;
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    dedicatedAllocationImageAliasing(in_struct->dedicatedAllocationImageAliasing)
{
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV()
{}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocationImageAliasing = src.dedicatedAllocationImageAliasing;
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::operator=(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    dedicatedAllocationImageAliasing = src.dedicatedAllocationImageAliasing;

    return *this;
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::~safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV()
{
}

void safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::initialize(const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    dedicatedAllocationImageAliasing = in_struct->dedicatedAllocationImageAliasing;
}

void safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::initialize(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    dedicatedAllocationImageAliasing = src->dedicatedAllocationImageAliasing;
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    bufferDeviceAddress(in_struct->bufferDeviceAddress),
    bufferDeviceAddressCaptureReplay(in_struct->bufferDeviceAddressCaptureReplay),
    bufferDeviceAddressMultiDevice(in_struct->bufferDeviceAddressMultiDevice)
{
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT()
{}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    bufferDeviceAddress = src.bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = src.bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = src.bufferDeviceAddressMultiDevice;
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::operator=(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    bufferDeviceAddress = src.bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = src.bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = src.bufferDeviceAddressMultiDevice;

    return *this;
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::~safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT()
{
}

void safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::initialize(const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    bufferDeviceAddress = in_struct->bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = in_struct->bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = in_struct->bufferDeviceAddressMultiDevice;
}

void safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::initialize(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    bufferDeviceAddress = src->bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = src->bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = src->bufferDeviceAddressMultiDevice;
}

safe_VkBufferDeviceAddressInfoEXT::safe_VkBufferDeviceAddressInfoEXT(const VkBufferDeviceAddressInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    buffer(in_struct->buffer)
{
}

safe_VkBufferDeviceAddressInfoEXT::safe_VkBufferDeviceAddressInfoEXT()
{}

safe_VkBufferDeviceAddressInfoEXT::safe_VkBufferDeviceAddressInfoEXT(const safe_VkBufferDeviceAddressInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;
}

safe_VkBufferDeviceAddressInfoEXT& safe_VkBufferDeviceAddressInfoEXT::operator=(const safe_VkBufferDeviceAddressInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    buffer = src.buffer;

    return *this;
}

safe_VkBufferDeviceAddressInfoEXT::~safe_VkBufferDeviceAddressInfoEXT()
{
}

void safe_VkBufferDeviceAddressInfoEXT::initialize(const VkBufferDeviceAddressInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    buffer = in_struct->buffer;
}

void safe_VkBufferDeviceAddressInfoEXT::initialize(const safe_VkBufferDeviceAddressInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    buffer = src->buffer;
}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT(const VkBufferDeviceAddressCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    deviceAddress(in_struct->deviceAddress)
{
}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT()
{}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT(const safe_VkBufferDeviceAddressCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    deviceAddress = src.deviceAddress;
}

safe_VkBufferDeviceAddressCreateInfoEXT& safe_VkBufferDeviceAddressCreateInfoEXT::operator=(const safe_VkBufferDeviceAddressCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    deviceAddress = src.deviceAddress;

    return *this;
}

safe_VkBufferDeviceAddressCreateInfoEXT::~safe_VkBufferDeviceAddressCreateInfoEXT()
{
}

void safe_VkBufferDeviceAddressCreateInfoEXT::initialize(const VkBufferDeviceAddressCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    deviceAddress = in_struct->deviceAddress;
}

void safe_VkBufferDeviceAddressCreateInfoEXT::initialize(const safe_VkBufferDeviceAddressCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    deviceAddress = src->deviceAddress;
}

safe_VkImageStencilUsageCreateInfoEXT::safe_VkImageStencilUsageCreateInfoEXT(const VkImageStencilUsageCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    stencilUsage(in_struct->stencilUsage)
{
}

safe_VkImageStencilUsageCreateInfoEXT::safe_VkImageStencilUsageCreateInfoEXT()
{}

safe_VkImageStencilUsageCreateInfoEXT::safe_VkImageStencilUsageCreateInfoEXT(const safe_VkImageStencilUsageCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    stencilUsage = src.stencilUsage;
}

safe_VkImageStencilUsageCreateInfoEXT& safe_VkImageStencilUsageCreateInfoEXT::operator=(const safe_VkImageStencilUsageCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    stencilUsage = src.stencilUsage;

    return *this;
}

safe_VkImageStencilUsageCreateInfoEXT::~safe_VkImageStencilUsageCreateInfoEXT()
{
}

void safe_VkImageStencilUsageCreateInfoEXT::initialize(const VkImageStencilUsageCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    stencilUsage = in_struct->stencilUsage;
}

void safe_VkImageStencilUsageCreateInfoEXT::initialize(const safe_VkImageStencilUsageCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    stencilUsage = src->stencilUsage;
}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT(const VkValidationFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    enabledValidationFeatureCount(in_struct->enabledValidationFeatureCount),
    pEnabledValidationFeatures(nullptr),
    disabledValidationFeatureCount(in_struct->disabledValidationFeatureCount),
    pDisabledValidationFeatures(nullptr)
{
    if (in_struct->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[in_struct->enabledValidationFeatureCount];
        memcpy ((void *)pEnabledValidationFeatures, (void *)in_struct->pEnabledValidationFeatures, sizeof(VkValidationFeatureEnableEXT)*in_struct->enabledValidationFeatureCount);
    }
    if (in_struct->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[in_struct->disabledValidationFeatureCount];
        memcpy ((void *)pDisabledValidationFeatures, (void *)in_struct->pDisabledValidationFeatures, sizeof(VkValidationFeatureDisableEXT)*in_struct->disabledValidationFeatureCount);
    }
}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT() :
    pEnabledValidationFeatures(nullptr),
    pDisabledValidationFeatures(nullptr)
{}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT(const safe_VkValidationFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    enabledValidationFeatureCount = src.enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = src.disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    if (src.pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[src.enabledValidationFeatureCount];
        memcpy ((void *)pEnabledValidationFeatures, (void *)src.pEnabledValidationFeatures, sizeof(VkValidationFeatureEnableEXT)*src.enabledValidationFeatureCount);
    }
    if (src.pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[src.disabledValidationFeatureCount];
        memcpy ((void *)pDisabledValidationFeatures, (void *)src.pDisabledValidationFeatures, sizeof(VkValidationFeatureDisableEXT)*src.disabledValidationFeatureCount);
    }
}

safe_VkValidationFeaturesEXT& safe_VkValidationFeaturesEXT::operator=(const safe_VkValidationFeaturesEXT& src)
{
    if (&src == this) return *this;

    if (pEnabledValidationFeatures)
        delete[] pEnabledValidationFeatures;
    if (pDisabledValidationFeatures)
        delete[] pDisabledValidationFeatures;

    sType = src.sType;
    pNext = src.pNext;
    enabledValidationFeatureCount = src.enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = src.disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    if (src.pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[src.enabledValidationFeatureCount];
        memcpy ((void *)pEnabledValidationFeatures, (void *)src.pEnabledValidationFeatures, sizeof(VkValidationFeatureEnableEXT)*src.enabledValidationFeatureCount);
    }
    if (src.pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[src.disabledValidationFeatureCount];
        memcpy ((void *)pDisabledValidationFeatures, (void *)src.pDisabledValidationFeatures, sizeof(VkValidationFeatureDisableEXT)*src.disabledValidationFeatureCount);
    }

    return *this;
}

safe_VkValidationFeaturesEXT::~safe_VkValidationFeaturesEXT()
{
    if (pEnabledValidationFeatures)
        delete[] pEnabledValidationFeatures;
    if (pDisabledValidationFeatures)
        delete[] pDisabledValidationFeatures;
}

void safe_VkValidationFeaturesEXT::initialize(const VkValidationFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    enabledValidationFeatureCount = in_struct->enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = in_struct->disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    if (in_struct->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[in_struct->enabledValidationFeatureCount];
        memcpy ((void *)pEnabledValidationFeatures, (void *)in_struct->pEnabledValidationFeatures, sizeof(VkValidationFeatureEnableEXT)*in_struct->enabledValidationFeatureCount);
    }
    if (in_struct->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[in_struct->disabledValidationFeatureCount];
        memcpy ((void *)pDisabledValidationFeatures, (void *)in_struct->pDisabledValidationFeatures, sizeof(VkValidationFeatureDisableEXT)*in_struct->disabledValidationFeatureCount);
    }
}

void safe_VkValidationFeaturesEXT::initialize(const safe_VkValidationFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    enabledValidationFeatureCount = src->enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = src->disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    if (src->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[src->enabledValidationFeatureCount];
        memcpy ((void *)pEnabledValidationFeatures, (void *)src->pEnabledValidationFeatures, sizeof(VkValidationFeatureEnableEXT)*src->enabledValidationFeatureCount);
    }
    if (src->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[src->disabledValidationFeatureCount];
        memcpy ((void *)pDisabledValidationFeatures, (void *)src->pDisabledValidationFeatures, sizeof(VkValidationFeatureDisableEXT)*src->disabledValidationFeatureCount);
    }
}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV(const VkCooperativeMatrixPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    MSize(in_struct->MSize),
    NSize(in_struct->NSize),
    KSize(in_struct->KSize),
    AType(in_struct->AType),
    BType(in_struct->BType),
    CType(in_struct->CType),
    DType(in_struct->DType),
    scope(in_struct->scope)
{
}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV()
{}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV(const safe_VkCooperativeMatrixPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    MSize = src.MSize;
    NSize = src.NSize;
    KSize = src.KSize;
    AType = src.AType;
    BType = src.BType;
    CType = src.CType;
    DType = src.DType;
    scope = src.scope;
}

safe_VkCooperativeMatrixPropertiesNV& safe_VkCooperativeMatrixPropertiesNV::operator=(const safe_VkCooperativeMatrixPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    MSize = src.MSize;
    NSize = src.NSize;
    KSize = src.KSize;
    AType = src.AType;
    BType = src.BType;
    CType = src.CType;
    DType = src.DType;
    scope = src.scope;

    return *this;
}

safe_VkCooperativeMatrixPropertiesNV::~safe_VkCooperativeMatrixPropertiesNV()
{
}

void safe_VkCooperativeMatrixPropertiesNV::initialize(const VkCooperativeMatrixPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    MSize = in_struct->MSize;
    NSize = in_struct->NSize;
    KSize = in_struct->KSize;
    AType = in_struct->AType;
    BType = in_struct->BType;
    CType = in_struct->CType;
    DType = in_struct->DType;
    scope = in_struct->scope;
}

void safe_VkCooperativeMatrixPropertiesNV::initialize(const safe_VkCooperativeMatrixPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    MSize = src->MSize;
    NSize = src->NSize;
    KSize = src->KSize;
    AType = src->AType;
    BType = src->BType;
    CType = src->CType;
    DType = src->DType;
    scope = src->scope;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    cooperativeMatrix(in_struct->cooperativeMatrix),
    cooperativeMatrixRobustBufferAccess(in_struct->cooperativeMatrixRobustBufferAccess)
{
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV()
{}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    cooperativeMatrix = src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = src.cooperativeMatrixRobustBufferAccess;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::operator=(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    cooperativeMatrix = src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = src.cooperativeMatrixRobustBufferAccess;

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::~safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV()
{
}

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::initialize(const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    cooperativeMatrix = in_struct->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = in_struct->cooperativeMatrixRobustBufferAccess;
}

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::initialize(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    cooperativeMatrix = src->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = src->cooperativeMatrixRobustBufferAccess;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    cooperativeMatrixSupportedStages(in_struct->cooperativeMatrixSupportedStages)
{
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV()
{}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    cooperativeMatrixSupportedStages = src.cooperativeMatrixSupportedStages;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::operator=(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    cooperativeMatrixSupportedStages = src.cooperativeMatrixSupportedStages;

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::~safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV()
{
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::initialize(const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    cooperativeMatrixSupportedStages = in_struct->cooperativeMatrixSupportedStages;
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::initialize(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    cooperativeMatrixSupportedStages = src->cooperativeMatrixSupportedStages;
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    coverageReductionMode(in_struct->coverageReductionMode)
{
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV()
{}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    coverageReductionMode = src.coverageReductionMode;
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::operator=(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    coverageReductionMode = src.coverageReductionMode;

    return *this;
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::~safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV()
{
}

void safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::initialize(const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    coverageReductionMode = in_struct->coverageReductionMode;
}

void safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::initialize(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    coverageReductionMode = src->coverageReductionMode;
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV(const VkPipelineCoverageReductionStateCreateInfoNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags),
    coverageReductionMode(in_struct->coverageReductionMode)
{
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV()
{}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV(const safe_VkPipelineCoverageReductionStateCreateInfoNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageReductionMode = src.coverageReductionMode;
}

safe_VkPipelineCoverageReductionStateCreateInfoNV& safe_VkPipelineCoverageReductionStateCreateInfoNV::operator=(const safe_VkPipelineCoverageReductionStateCreateInfoNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
    coverageReductionMode = src.coverageReductionMode;

    return *this;
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::~safe_VkPipelineCoverageReductionStateCreateInfoNV()
{
}

void safe_VkPipelineCoverageReductionStateCreateInfoNV::initialize(const VkPipelineCoverageReductionStateCreateInfoNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
    coverageReductionMode = in_struct->coverageReductionMode;
}

void safe_VkPipelineCoverageReductionStateCreateInfoNV::initialize(const safe_VkPipelineCoverageReductionStateCreateInfoNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
    coverageReductionMode = src->coverageReductionMode;
}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV(const VkFramebufferMixedSamplesCombinationNV* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    coverageReductionMode(in_struct->coverageReductionMode),
    rasterizationSamples(in_struct->rasterizationSamples),
    depthStencilSamples(in_struct->depthStencilSamples),
    colorSamples(in_struct->colorSamples)
{
}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV()
{}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV(const safe_VkFramebufferMixedSamplesCombinationNV& src)
{
    sType = src.sType;
    pNext = src.pNext;
    coverageReductionMode = src.coverageReductionMode;
    rasterizationSamples = src.rasterizationSamples;
    depthStencilSamples = src.depthStencilSamples;
    colorSamples = src.colorSamples;
}

safe_VkFramebufferMixedSamplesCombinationNV& safe_VkFramebufferMixedSamplesCombinationNV::operator=(const safe_VkFramebufferMixedSamplesCombinationNV& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    coverageReductionMode = src.coverageReductionMode;
    rasterizationSamples = src.rasterizationSamples;
    depthStencilSamples = src.depthStencilSamples;
    colorSamples = src.colorSamples;

    return *this;
}

safe_VkFramebufferMixedSamplesCombinationNV::~safe_VkFramebufferMixedSamplesCombinationNV()
{
}

void safe_VkFramebufferMixedSamplesCombinationNV::initialize(const VkFramebufferMixedSamplesCombinationNV* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    coverageReductionMode = in_struct->coverageReductionMode;
    rasterizationSamples = in_struct->rasterizationSamples;
    depthStencilSamples = in_struct->depthStencilSamples;
    colorSamples = in_struct->colorSamples;
}

void safe_VkFramebufferMixedSamplesCombinationNV::initialize(const safe_VkFramebufferMixedSamplesCombinationNV* src)
{
    sType = src->sType;
    pNext = src->pNext;
    coverageReductionMode = src->coverageReductionMode;
    rasterizationSamples = src->rasterizationSamples;
    depthStencilSamples = src->depthStencilSamples;
    colorSamples = src->colorSamples;
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fragmentShaderSampleInterlock(in_struct->fragmentShaderSampleInterlock),
    fragmentShaderPixelInterlock(in_struct->fragmentShaderPixelInterlock),
    fragmentShaderShadingRateInterlock(in_struct->fragmentShaderShadingRateInterlock)
{
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT()
{}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fragmentShaderSampleInterlock = src.fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = src.fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = src.fragmentShaderShadingRateInterlock;
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::operator=(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fragmentShaderSampleInterlock = src.fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = src.fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = src.fragmentShaderShadingRateInterlock;

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::~safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT()
{
}

void safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::initialize(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fragmentShaderSampleInterlock = in_struct->fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = in_struct->fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = in_struct->fragmentShaderShadingRateInterlock;
}

void safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::initialize(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fragmentShaderSampleInterlock = src->fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = src->fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = src->fragmentShaderShadingRateInterlock;
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    ycbcrImageArrays(in_struct->ycbcrImageArrays)
{
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT()
{}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    ycbcrImageArrays = src.ycbcrImageArrays;
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::operator=(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    ycbcrImageArrays = src.ycbcrImageArrays;

    return *this;
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::~safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT()
{
}

void safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::initialize(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    ycbcrImageArrays = in_struct->ycbcrImageArrays;
}

void safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::initialize(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    ycbcrImageArrays = src->ycbcrImageArrays;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT(const VkSurfaceFullScreenExclusiveInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fullScreenExclusive(in_struct->fullScreenExclusive)
{
}

safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT()
{}

safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT(const safe_VkSurfaceFullScreenExclusiveInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fullScreenExclusive = src.fullScreenExclusive;
}

safe_VkSurfaceFullScreenExclusiveInfoEXT& safe_VkSurfaceFullScreenExclusiveInfoEXT::operator=(const safe_VkSurfaceFullScreenExclusiveInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fullScreenExclusive = src.fullScreenExclusive;

    return *this;
}

safe_VkSurfaceFullScreenExclusiveInfoEXT::~safe_VkSurfaceFullScreenExclusiveInfoEXT()
{
}

void safe_VkSurfaceFullScreenExclusiveInfoEXT::initialize(const VkSurfaceFullScreenExclusiveInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fullScreenExclusive = in_struct->fullScreenExclusive;
}

void safe_VkSurfaceFullScreenExclusiveInfoEXT::initialize(const safe_VkSurfaceFullScreenExclusiveInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fullScreenExclusive = src->fullScreenExclusive;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    fullScreenExclusiveSupported(in_struct->fullScreenExclusiveSupported)
{
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT()
{}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    fullScreenExclusiveSupported = src.fullScreenExclusiveSupported;
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::operator=(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    fullScreenExclusiveSupported = src.fullScreenExclusiveSupported;

    return *this;
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::~safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT()
{
}

void safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::initialize(const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    fullScreenExclusiveSupported = in_struct->fullScreenExclusiveSupported;
}

void safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::initialize(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    fullScreenExclusiveSupported = src->fullScreenExclusiveSupported;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR


safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    hmonitor(in_struct->hmonitor)
{
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT()
{}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    hmonitor = src.hmonitor;
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::operator=(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    hmonitor = src.hmonitor;

    return *this;
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::~safe_VkSurfaceFullScreenExclusiveWin32InfoEXT()
{
}

void safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::initialize(const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    hmonitor = in_struct->hmonitor;
}

void safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::initialize(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    hmonitor = src->hmonitor;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT(const VkHeadlessSurfaceCreateInfoEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    flags(in_struct->flags)
{
}

safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT()
{}

safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT(const safe_VkHeadlessSurfaceCreateInfoEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;
}

safe_VkHeadlessSurfaceCreateInfoEXT& safe_VkHeadlessSurfaceCreateInfoEXT::operator=(const safe_VkHeadlessSurfaceCreateInfoEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    flags = src.flags;

    return *this;
}

safe_VkHeadlessSurfaceCreateInfoEXT::~safe_VkHeadlessSurfaceCreateInfoEXT()
{
}

void safe_VkHeadlessSurfaceCreateInfoEXT::initialize(const VkHeadlessSurfaceCreateInfoEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    flags = in_struct->flags;
}

void safe_VkHeadlessSurfaceCreateInfoEXT::initialize(const safe_VkHeadlessSurfaceCreateInfoEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    flags = src->flags;
}

safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::safe_VkPhysicalDeviceHostQueryResetFeaturesEXT(const VkPhysicalDeviceHostQueryResetFeaturesEXT* in_struct) :
    sType(in_struct->sType),
    pNext(in_struct->pNext),
    hostQueryReset(in_struct->hostQueryReset)
{
}

safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::safe_VkPhysicalDeviceHostQueryResetFeaturesEXT()
{}

safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::safe_VkPhysicalDeviceHostQueryResetFeaturesEXT(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& src)
{
    sType = src.sType;
    pNext = src.pNext;
    hostQueryReset = src.hostQueryReset;
}

safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::operator=(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& src)
{
    if (&src == this) return *this;


    sType = src.sType;
    pNext = src.pNext;
    hostQueryReset = src.hostQueryReset;

    return *this;
}

safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::~safe_VkPhysicalDeviceHostQueryResetFeaturesEXT()
{
}

void safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::initialize(const VkPhysicalDeviceHostQueryResetFeaturesEXT* in_struct)
{
    sType = in_struct->sType;
    pNext = in_struct->pNext;
    hostQueryReset = in_struct->hostQueryReset;
}

void safe_VkPhysicalDeviceHostQueryResetFeaturesEXT::initialize(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT* src)
{
    sType = src->sType;
    pNext = src->pNext;
    hostQueryReset = src->hostQueryReset;
}
