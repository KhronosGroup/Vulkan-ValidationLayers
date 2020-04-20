/* Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
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

//   Instanceless tests
// Tests of validation of vkCreateInstance and vkDestroyInstance via the pNext debug callback.
//
// This set of test should ideally be as complete as possible. Most of the VUs are Implicit (i.e. automatically generated), but any
// of the parameters could expose a bug or inadequacy in the Loader or the debug extension.
//
// Note: testing pCreateInfo pointer, the sType of a debug struct, the debug callback pointer, the ppEnabledLayerNames pointer, and
//       the ppEnabledExtensionNames would require extenally enabled debug layers, so this is currently not performed.
//
// TODO: VkDebugReportCallbackCreateInfoEXT::flags and VkDebugUtilsMessengerCreateInfoEXT various Flags could theoretically be
//       tested if the debug extensions are made robust enough

#include <memory>
#include <vector>

#include "cast_utils.h"
#include "layer_validation_tests.h"

static VkInstance dummy_instance;

TEST_F(VkLayerTest, InstanceExtensionDependencies) {
    TEST_DESCRIPTION("Test enabling instance extension without dependencies met.");

    if (!InstanceExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s.\n", kSkipPrefix, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_TRUE(InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME));  // Driver should always provide dependencies

    Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388");
    instance_extensions_.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    const auto ici = GetInstanceCreateInfo();
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(VkLayerTest, InstanceBadStype) {
    TEST_DESCRIPTION("Test creating instance with bad sType.");

    auto ici = GetInstanceCreateInfo();

    Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-sType-sType");
    ici.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(VkLayerTest, InstanceDuplicatePnextStype) {
    TEST_DESCRIPTION("Test creating instance with duplicate sType in the pNext chain.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s.\n", kSkipPrefix, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        return;
    }
    instance_extensions_.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-sType-unique");
    const VkValidationFeaturesEXT duplicate_pnext = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, ici.pNext};
    const VkValidationFeaturesEXT first_pnext = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, &duplicate_pnext};
    ici.pNext = &first_pnext;
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(VkLayerTest, InstanceFlags) {
    TEST_DESCRIPTION("Test creating instance with invalid flags.");

    auto ici = GetInstanceCreateInfo();

    Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-flags-zerobitmask");
    ici.flags = (VkInstanceCreateFlags)1;
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(VkLayerTest, InstanceAppInfoBadStype) {
    TEST_DESCRIPTION("Test creating instance with invalid sType in VkApplicationInfo.");

    auto ici = GetInstanceCreateInfo();

    VkApplicationInfo bad_app_info = {};
    if (ici.pApplicationInfo) bad_app_info = *ici.pApplicationInfo;
    bad_app_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &bad_app_info;

    Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-VkApplicationInfo-sType-sType");
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(VkLayerTest, InstanceValidationFeaturesBadFlags) {
    TEST_DESCRIPTION("Test creating instance with invalid flags in VkValidationFeaturesEXT.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s.\n", kSkipPrefix, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        return;
    }
    instance_extensions_.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    // the test framework should not be using VkValidationFeatureEnableEXT itself
    for (auto traversable_pnext = reinterpret_cast<const VkBaseInStructure*>(ici.pNext); traversable_pnext;
         traversable_pnext = traversable_pnext->pNext) {
        ASSERT_NE(traversable_pnext->sType, VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT);
    }

    VkValidationFeaturesEXT validation_features_template = {};
    validation_features_template.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features_template.pNext = ici.pNext;

    {
        const VkValidationFeatureEnableEXT bad_enable = (VkValidationFeatureEnableEXT)0x42;
        VkValidationFeaturesEXT validation_features = validation_features_template;
        validation_features.enabledValidationFeatureCount = 1;
        validation_features.pEnabledValidationFeatures = &bad_enable;
        ici.pNext = &validation_features;

        // VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-parameter
        Monitor().SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-UnrecognizedValue");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        const VkValidationFeatureDisableEXT bad_disable = (VkValidationFeatureDisableEXT)0x42;
        VkValidationFeaturesEXT validation_features = validation_features_template;
        validation_features.disabledValidationFeatureCount = 1;
        validation_features.pDisabledValidationFeatures = &bad_disable;
        ici.pNext = &validation_features;

        // VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-parameter
        Monitor().SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-UnrecognizedValue");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }
}

TEST_F(VkLayerTest, InstanceBadValidationFlags) {
    TEST_DESCRIPTION("Test creating instance with invalid VkValidationFlagsEXT.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s.\n", kSkipPrefix, VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME);
        return;
    }
    instance_extensions_.push_back(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    // the test framework should not be using VkValidationFlagsEXT itself
    for (auto traversable_pnext = reinterpret_cast<const VkBaseInStructure*>(ici.pNext); traversable_pnext;
         traversable_pnext = traversable_pnext->pNext) {
        ASSERT_NE(traversable_pnext->sType, VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT);
    }

    VkValidationFlagsEXT validation_flags_template = {};
    validation_flags_template.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
    validation_flags_template.pNext = ici.pNext;

    {
        const VkValidationCheckEXT bad_disable = (VkValidationCheckEXT)0x42;
        VkValidationFlagsEXT validation_flags = validation_flags_template;
        validation_flags.disabledValidationCheckCount = 1;
        validation_flags.pDisabledValidationChecks = &bad_disable;
        ici.pNext = &validation_flags;

        // VUID-VkValidationFlagsEXT-pDisabledValidationChecks-parameter
        Monitor().SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-UnrecognizedValue");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        VkValidationFlagsEXT validation_flags = validation_flags_template;
        validation_flags.disabledValidationCheckCount = 0;
        ici.pNext = &validation_flags;

        // VUID-VkValidationFlagsEXT-disabledValidationCheckCount-arraylength
        Monitor().SetDesiredFailureMsg(kErrorBit, "parameter disabledValidationCheckCount must be greater than 0");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }
}

void* VKAPI_PTR DummyAlloc(void*, size_t size, size_t alignment, VkSystemAllocationScope) {
    size_t space = size + alignment - 1;
    void* mem_ptr = std::malloc(space);
    return std::align(alignment, size, mem_ptr, space);
}
void VKAPI_PTR DummyFree(void*, void* pMemory) {
    // just leak it
}
void* VKAPI_PTR DummyRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment,
                             VkSystemAllocationScope allocationScope) {
    DummyFree(pUserData, pOriginal);
    return DummyAlloc(pUserData, size, alignment, allocationScope);
}
void VKAPI_PTR DummyInfoAlloc(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope) {}
void VKAPI_PTR DummyInfoFree(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope) {}

TEST_F(VkLayerTest, DestroyInstanceAllocationCallbacksCompatibility) {
    TEST_DESCRIPTION("Test vkDestroyInstance with incompatible allocation callbacks.");

    const auto ici = GetInstanceCreateInfo();
    const VkAllocationCallbacks alloc_callbacks = {nullptr,    &DummyAlloc,     &DummyRealloc,
                                                   &DummyFree, &DummyInfoAlloc, &DummyInfoFree};

    {
        VkInstance instance;
        ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &instance));

        Monitor().SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyInstance-instance-00631");
        vk::DestroyInstance(instance, &alloc_callbacks);
        Monitor().VerifyFound();
    }
}

TEST_F(VkLayerTest, DestroyInstanceHandleLeak) {
    TEST_DESCRIPTION("Test vkDestroyInstance while leaking a VkDevice object.");

    const auto ici = GetInstanceCreateInfo();

    VkInstance instance;
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &instance));
    uint32_t physical_device_count = 1;
    VkPhysicalDevice physical_device;
    const VkResult err = vk::EnumeratePhysicalDevices(instance, &physical_device_count, &physical_device);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << vk_result_string(err);
    ASSERT_EQ(physical_device_count, 1);

    float dqci_priorities[] = {1.0};
    VkDeviceQueueCreateInfo dqci = {};
    dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqci.queueFamilyIndex = 0;
    dqci.queueCount = 1;
    dqci.pQueuePriorities = dqci_priorities;

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;

    VkDevice leaked_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(physical_device, &dci, nullptr, &leaked_device));

    // VUID-vkDestroyInstance-instance-00629
    Monitor().SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-ObjectTracker-ObjectLeak");
    vk::DestroyInstance(instance, nullptr);
    Monitor().VerifyFound();
}