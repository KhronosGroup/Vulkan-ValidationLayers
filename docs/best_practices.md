<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2020 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Best Practices Validation

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

Best Practices Validation

Best Practices Validation is implemented in the `VK_LAYER_KHRONOS_validation layer`. When enabled, the Best Practices Object is
intended to highlight potential performance issues, questionable usage patterns, common mistakes, and items not specifically prohibited
by the Vulkan specification but that may lead to application problems.

Best Practices will ideally be run periodically along with normal validation checks so that issues may be addressed in early stages of development.

Best Practices can easily be enabled and configured using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK. Or you can manually enable and configure the Best Practices by following the directions below.

The specific areas covered by this layer are currently tracked in the
[Best Practices Project](https://github.com/KhronosGroup/Vulkan-ValidationLayers/projects/1).
Requests for additional checks can be requested by creating a Github issue.

## Enabling Best Practices Validation

Best Practices Validation is disabled by default. To turn on Best Practices validation, add the following to your layer settings file,
`vk_layer_settings.txt`:

```code
khronos_validation.enables = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
```

To enable using environment variables, set the following variable:

```code
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
```

To additionally enable the ARM-specific best practices checks, use the following:

```code
khronos_validation.enables = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM
```

 - or the environment variables for Windows

```code
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM
```

 - or for Linux -

```code
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM
```

Some platforms do not support configuration of the validation layers with this configuration file.
Programs running on these platforms must then use the programmatic interface.

### Enabling and Specifying Options with the Programmatic Interface

The `VK_EXT_validation_features` extension can be used to enable Best Practices Validation at CreateInstance time.

Here is sample code illustrating how to enable it:

```code
VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
VkValidationFeaturesEXT features = {};
features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = enables;

VkInstanceCreateInfo info = {};
info.pNext = &features;
```
