# GPU-AV Selective Shader Instrumentation

With the `khronos_validation.gpuav_select_instrumented_shaders`/`VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS` feature, an application can control which shaders are instrumented and thus, will return GPU-AV errors.

With the feature enabled, all SPIR-V will not be modified by default.

Inside your `VkShaderModuleCreateInfo` or `vkCreateShadersEXT` pass in a `VkValidationFeaturesEXT` into the `pNext` with `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT` to have the shader instrumented.

```c++
// Example
VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
VkValidationFeaturesEXT features = {};
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = enabled;

VkShaderModuleCreateInfo module_ci = {};
module_ci.pNext = &features;
```
