# Creating Tests

This is an "up-to-speed" document for writing tests to validate the Validation Layers

## Rule #1

The first rule is to make sure you are actually running the tests on the built version of the Validation Layers you want. If you have the Vulkan SDK installed, then you will have a pre-built version of the Validation Layers set in your path and those are probably not the version you want to test.

Make sure you have the correct `VK_LAYER_PATH` set on Windows or Linux (on Android the layers are baked into the APK so there is nothing to worry about)

```bash
export VK_LAYER_PATH=/path/to/Vulkan-ValidationLayers/build/layers/
```

There is nothing worse than debugging why your layers are not reporting the VUID you added due to not actually testing that code!

## Google Test Overview

The tests take advantage of the Google Test (gtest) Framework which breaks each test into a `TEST_F(VkLayerTest, TestName)` "Test Fixture". This just means that for every test there will be a class that holds many useful member variables.

To run a test labeled `TEST_F(VkLayerTest, Foo)` is as simple as going `--gtest_filter=VkLayerTest.Foo`

## VkRenderFramework

The `VkRenderFramework` class is "base class" that abstract most things in order to allow a test writer to focus on the small part of coded needed for the test.

For most tests, it is as simple as going

```cpp
ASSERT_NO_FATAL_FAILURE(Init());

// or

ASSERT_NO_FATAL_FAILURE(InitFramework());
ASSERT_NO_FATAL_FAILURE(InitState());

// For Best Practices tests
ASSERT_NO_FATAL_FAILURE(InitBestPracticesFramework());
ASSERT_NO_FATAL_FAILURE(InitState());
```

to set it up. This will create the `VkInstance` and `VkDevice` for you.

There are other useful helper functions such as `InitSurface()`, `InitSwapchain()`, `InitRenderTarget()`, and more. Please view the class source for more of an overview of what it currently supports

## Extensions

Adding extension support is quite easy.

Here is an example of adding `VK_KHR_sampler_ycbcr_conversion` with all the extensions it requires as well. Note, most extensions will have only 1 or 2 dependency extensions needed

```cpp
// Setup extensions, including dependent instance and device extensions. This call should be made before any call to InitFramework
AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

//  Among other things, this will create the VkInstance and VkPhysicalDevice that will be used for the test.
ASSERT_NO_FATAL_FAILURE(InitFramework());

// Check that all extensions and their dependencies were enabled successfully
if (!AreRequiredExtensionsEnabled()) {
    GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
}

// Finish initializing state, including creating the VkDevice (whith extensions added) that will be used for the test
ASSERT_NO_FATAL_FAILURE(InitState());
```

The pattern breaks down to
- Check and add Instance extensions to list
- Init Framework which creates `VkInstance`
- Check and add Device extensions to list
- Init State which creates the `VkDevice`
- **Optional**: skip if test is not worth moving out without extension support (more below)

### Pattern for optional extensions

Sometimes it is worth checking for an extension, but still running the parts of a test if the extension is not supported

```cpp
AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
ASSERT_NO_FATAL_FAILURE(Init());

// need to wait until after phyiscal device creation to know if it was enabled
const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

// Check required (not optional) extensions are still supported
if (!AreRequiredExtensionsEnabled()) {
    GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
}

// If the optional extension has a command, it will need a vkGetDeviceProcAddr call
PFN_vkCmdCopyBuffer2KHR vkCmdCopyBuffer2KHR = nullptr;
if (copy_commands2) {
    vkCmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyBuffer2KHR");
}

// Validate core copy command
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
vk::CmdCopyBuffer( /* */ );
m_errorMonitor->VerifyFound();

// optional test using VK_KHR_copy_commands2
if (copy_commands2) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vkCmdCopyBuffer2KHR( /* */  );
    m_errorMonitor->VerifyFound();
}
```

### Vulkan Version

As [raised in a previous issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1553), when a Vulkan version is enabled, all extensions that are required are exposed. (For example, if the test is created as a Vulkan 1.1 application, then the `VK_KHR_16bit_storage` extension would be supported regardless as it was promoted to Vulkan 1.1 core)

If a certain version of Vulkan is needed a test writer can call

```cpp
SetTargetApiVersion(VK_API_VERSION_1_1);
ASSERT_NO_FATAL_FAILURE(InitFramework());
```

Later in the test it can also be checked
```cpp
if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
    // ...
}
```

### Getting Function Pointers

A common case for checking the version is in order to find how to correctly get extension function pointers.

```cpp
// Create aliased function pointers for 1.0 and 1.1+ contexts
PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
    vkBindImageMemory2Function = vk::BindImageMemory2;
} else {
    vkBindImageMemory2Function = reinterpret_cast<PFN_vkBindImageMemory2KHR>(vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR"));
}

// later in code
vkBindImageMemory2Function(device(), 1, &bind_image_info);
```

## Error Monitor

The `ErrorMonitor *m_errorMonitor` in the `VkRenderFramework` becomes your best friend when you write tests

This small class handles checking all things to VUID and are ultimately what will "pass or fail" a test

The few common patterns that will cover 99% of cases are:

- **By default**, all Vulkan API calls are expected to succeed. In the past, one would have to "wrap" API calls in `ExpectSuccess`/`VerifyNotFound` to ensure an API call did not trigger any errors. This is no longer the case. e.g.,
```cpp
// m_errorMonitor->ExpectSuccess(); <- implicit
vk::CreateSampler(m_device->device(), &sci, nullptr, &samplers[0]);
// m_errorMonitor->VerifyNoutFound(); <- implicit
```
The `ExpectSuccess` and `VerifyNotFound` calls are now implicit.
- For checking a call that invokes a VUID error
```cpp
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
// The following API call is expected to trigger 01646 and _only_ 01646
vk::CreateSampler(m_device->device(), &sci, NULL, &BadSampler);
m_errorMonitor->VerifyFound();

// All calls after m_errorMonitor->VerifyFound() are expected to not trigger any errors. e.g., the following API call should succeed with no validation errors being triggered.
vk::CreateImage(m_device->device(), &ci, nullptr, &mp_image);

```
- When it is possible another VUID will be triggered that you are not testing. This usually happens due to making something invalid can cause a chain effect causing other things to be invalid as well.
    - Note: If the `SetUnexpectedError` is never called it will not fail the test
```cpp
m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-02280");
vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
m_errorMonitor->VerifyFound();
```

- When you expect multpile VUID to be triggered. This is also be a case if you expect the same VUID to be called twice.
    - Note: If both VUID are not found the test will fail
```cpp
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
m_errorMonitor->VerifyFound();
```

- When a VUID is dependent on an extension being present
    - Note: The start of the test might already have a boolean that checks for extension support
```cpp
const char* vuid = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ? "VUID-vkCmdCopyImage-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
m_commandBuffer->CopyImage(image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
m_errorMonitor->VerifyFound();
```

- There should be a single Vulkan function between m_errorMontior calls to ensure the test is only applied to the single function call being tested.
- Keep it simple. Try to make each test as small and concise as possible.
- Avoid testing VUIDs in "batches" such as:
```cpp
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06003");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008");
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009");
...
vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
m_errorMonitor->VerifyFound();
```
If there is a problem with one of these checks later on, this method makes it difficult and more
time-consuming to figure out _which_ check is problematic. It also makes it difficult to understand
which Vulkan parameter/setting triggered which error. It should be relatively obvious to
tell which line(s) of code caused a validation error to be triggered (and if it isn't, comments should be
used to make it obvious).
- Make it clear how the VUID you're testing is triggered. e.g.,
```cpp
// Try to get layout for plane 3 when we only have 2
VkImageSubresource subresource{};
subresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;
VkSubresourceLayout layout{};
m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-tiling-02271");
vk::GetImageSubresourceLayout(m_device->handle(), image.handle(), &subresource, &layout);
m_errorMonitor->VerifyFound();
```
Here it is obvious that the `aspectMask` parameter is the cause of 02271.

## Device Profiles API

There are times a test writer will want to test a case where an implementation returns a certain support for a format feature or limit. Instead of just hoping to find a device that supports a certain case, there is the Device Profiles API layer. This layer will allow a test writer to inject certain values for format features and/or limits.

### Device Profile Format Feature
Here is an example of how To enable it to allow overriding format features (limits are the same idea, just different function names):
```cpp
// Will replace VK_LAYER_LUNARG_device_simulation with VK_LAYER_LUNARG_device_profile_api
//
// This can be skipped if test doesn't allow for devsim (ex. GPU Validation tests)
//
// Needs to be done BEFORE creating an VkInstance (because they are instance level layers)
if (!OverrideDevsimForDeviceProfileLayer()) {
    GTEST_SKIP() << "Failed to override devsim for device profile layer.";
}

ASSERT_NO_FATAL_FAILURE(Init());

// Load required functions
PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}
```

This is an example of injecting the `VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT` feature for the `VK_FORMAT_R32G32B32A32_UINT` format. This will force the Validations Layers to act as if the implementation had support for this feature later in the test's code.
```cpp
fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &formatProps);
formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, formatProps);
```

If you are in need of `VkFormatProperties3` the following is an example how to use the layer

```cpp
PFN_vkSetPhysicalDeviceFormatProperties2EXT fpvkSetPhysicalDeviceFormatProperties2EXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT fpvkGetOriginalPhysicalDeviceFormatProperties2EXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatProperties2EXT, fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}

auto fmt_props_3 = LvlInitStruct<VkFormatProperties3>();
auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

// Removes unwanted support
fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), image_format, &fmt_props);
// VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT == VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
// Need to edit both VkFormatFeatureFlags/VkFormatFeatureFlags2
fmt_props.formatProperties.optimalTilingFeatures = (fmt_props.formatProperties.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
// Was added with VkFormatFeatureFlags2 so only need to edit here
fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT);
fpvkSetPhysicalDeviceFormatProperties2EXT(gpu(), image_format, fmt_props);

```

### Device Profile Limits

When using the device profile layer for limits, the test maybe need to call `vkSetPhysicalDeviceLimitsEXT` prior to creating the `VkDevice` for some validation state tracking

```cpp
ASSERT_NO_FATAL_FAILURE(InitFramework());

// Load required functions
PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}

VkPhysicalDeviceProperties props;
fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
props.limits.maxPushConstantsSize = 16; // example
fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

ASSERT_NO_FATAL_FAILURE(InitState());
```

## Running Tests on DevSim and MockICD

To allow a much higher coverage of testing the Validation Layers a test writer can use the Device Simulation layer. [More details here](https://www.lunarg.com/new-vulkan-dev-sim-layer/), but the main idea is the layer intercepts the Physical Device queries allowing testing of much more device properties. The Mock ICD is a "null driver" that is used to handle the Vulkan calls as the Validation Layers mostly only care about input "input" of the driver. If your tests relies on the "output" of the driver (such that a driver/implementation is correctly doing what it should do with valid input), then it might be worth looking into the [Vulkan CTS instead](https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/vulkan_cts.md).

Both the Device Simulation Layer and MockICD can be found in the Vulkan SDK, otherwise, they will need to be cloned from [VulkanTools](https://github.com/LunarG/VulkanTools/blob/master/layersvt/VkLayer_device_simulation.json.in) and [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools/tree/master/icd) respectfully. Currently, you will need to build the MockICD from source (found in Vulkan SDK or in a local copy somewhere)

Here is an example of setting up and running the Device Simulation layer with MockICD on a Linux environment
```bash
export VULKAN_SDK=/path/to/vulkansdk
export VVL=/path/to/Vulkan-ValidationLayers

# Add built Vulkan Validation Layers... remember it was Rule #1
export VK_LAYER_PATH=$VVL/build/layers/

# This step can be skipped if the Vulkan SDK is properly installed
# Add path for device simulation layer
export VK_LAYER_PATH=$VK_LAYER_PATH:$VULKAN_SDK/etc/vulkan/explicit_layer.d/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$VULKAN_SDK/lib/

# Set MockICD to be driver
export VK_ICD_FILENAMES=/path/to/Vulkan-Tools/build/icd/VkICD_mock_icd.json

# Set device simulation profile to a valid json file
# There are a set of profiles used in CI in the device_profiles folder
export VK_DEVSIM_FILENAME=$VVL/tests/device_profiles/mobile_chip.json

# Needed or else the code `IsPlatform(kMockICD)` will not work
# Also allows profiles to use extensions exposed in .json profile file
# More details - https://github.com/LunarG/VulkanTools/issues/985
export VK_DEVSIM_MODIFY_EXTENSION_LIST=1

# Running tests just need the extra --devsim added
$VVL/build/tests/vk_layer_validation_tests --devsim --gtest_filter=TestName
```
