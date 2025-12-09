# Updating from VK_EXT_validation_features

Starting in the Vulkan 1.4.335 SDK you will now receive a warning such as

```
Validation Warning: [ VALIDATION-SETTINGS ] | MessageID = 0x7f1922d7
vkCreateInstance(): Application is using deprecated "enables" (VK_LAYER_ENABLES) and "disables" (VK_LAYER_DISABLES) layer settings.
Deprecated settings and new settings cannot be mixed, and deprecated ones take precedence. Consider only using the new settings:
  Deprecated: VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT | New: "gpuav_enable" (VK_LAYER_GPUAV_ENABLE=1)
  Deprecated: VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT | New: "validate_core" (VK_LAYER_VALIDATE_CORE=0)
  Deprecated: VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT | New: "object_lifetime" (VK_LAYER_OBJECT_LIFETIME=0)
  Deprecated: VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT | New: "stateless_param" (VK_LAYER_STATELESS_PARAM=0)
  Deprecated: VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT | New: "thread_safety" (VK_LAYER_THREAD_SAFETY=0)
```

Long ago the `VK_EXT_validation_features` extension was added, it is **never going away**, but it was not a sustainable option as we add new things, so instead the `VK_EXT_layer_settings` should be used.

> Please see [Configuring the Validation Layer](./khronos_validation_layer.md#configuring-the-validation-layer) for full details of setting the layers

## If using environment variables

You just need to remove your `VK_LAYER_ENABLES=....` and `VK_LAYER_DISABLES=...` and replace with the new dedicated setting:

- `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT` -> `VK_LAYER_GPUAV_ENABLE=1`
- `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT` -> (removed, we always reserve a slot now)
- `VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT` -> `VK_LAYER_VALIDATE_BEST_PRACTICES=1`
- `VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT` -> `VK_LAYER_PRINTF_ENABLE=1`
- `VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT` -> `VK_LAYER_VALIDATE_SYNC=1`
- `VK_VALIDATION_FEATURE_DISABLE_ALL_EXT` -> (removed, other means to turn off layer completly)
- `VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT` -> `VK_LAYER_CHECK_SHADERS=0`
- `VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT` -> `VK_LAYER_THREAD_SAFETY=0`
- `VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT` -> `VK_LAYER_STATELESS_PARAM=0`
- `VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT` -> `VK_LAYER_OBJECT_LIFETIME=0`
- `VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT` -> `VK_LAYER_VALIDATE_CORE=0`
- `VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT` -> `VK_LAYER_UNIQUE_HANDLES=0`
- `VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT` -> `VK_LAYER_CHECK_SHADERS_CACHING=0`

Example

```patch
-set VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
-set VK_LAYER_DISABLES=VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT;VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
+set VK_LAYER_VALIDATE_SYNC=1
+set VK_LAYER_VALIDATE_CORE=0
+set VK_LAYER_UNIQUE_HANDLES=0
```

## If using VkValidationFeaturesEXT at vkCreateInstance

If you have code such as

```c++
VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

VkValidationFeaturesEXT features;
features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
features.pNext = nullptr;
features.enabledValidationFeatureCount = 1;
features.disabledValidationFeatureCount = 2;
features.pEnabledValidationFeatures = enables;
features.pDisabledValidationFeatures = disables;

VkInstanceCreateInfo instance_create_info;
instance_create_info.pNext = &features;
```

The simple replacement is to make use of the new `VkLayerSettingsCreateInfoEXT` struct

```c++
const VkBool32 turn_on = VK_TRUE;
const VkBool32 turn_off = VK_FALSE;

const VkLayerSettingEXT settings[3] = {
    {"VK_LAYER_KHRONOS_validation", "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &turn_on},
    {"VK_LAYER_KHRONOS_validation", "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &turn_off},
    {"VK_LAYER_KHRONOS_validation", "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &turn_off}
};

VkLayerSettingsCreateInfoEXT settings_create_info;
settings_create_info.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
settings_create_info.pNext = nullptr;
settings_create_info.settingCount = 3;
settings_create_info.pSettings = settings;

VkInstanceCreateInfo instance_create_info;
instance_create_info.pNext = &settings_create_info;
```