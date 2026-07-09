# Scoped GPU-AV

There are many ways to provide information to help reduce the amount of validation GPU-AV needs to do. This is very powerful when mixed with `Safe Mode` to detect crashes while also having very little overhead.

# Selective Instrument Shaders

Shader instrumentation can be expensive, so by providing the `VkShaderModule`/`VkPipeline`/`VkShaderEXT` we can limit it to only those shaders.

When the `Selective Instrument Shaders` (`VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS`/`gpuav_select_instrumented_shaders`) setting is on **ONLY** selected shaders are instrumented. The other shaders **will not** be instrumented.

We provide various ways to select these shaders.

## Provide a Debug Name

It is normally desirable to not need to recompile your source code nor shaders. If you already have a debug name (set by `vkSetDebugUtilsObjectNameEXT`) then you can use those names to select which shaders to instrument.

This is done either easily in vkconfig or passing a list of regex strings to `VK_LAYER_GPUAV_SHADERS_TO_INSTRUMENT`/`gpuav_shaders_to_instrument`

as environment variables

```bash
set VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS=1
set VK_LAYER_GPUAV_SHADERS_TO_INSTRUMENT=bim_bap_boom,.*_my_pipeline.*
```

or with `VK_EXT_layer_settings`

```c++
// example
std::array<const char*, 2> shader_regexes = {{"bim_bap_boom", ".*_my_pipeline.*"}};
layer_setting = {"VK_LAYER_KHRONOS_validation", "gpuav_shaders_to_instrument", VK_LAYER_SETTING_TYPE_STRING_EXT, size32(shader_regexes), shader_regexes.data()};
```

## Manually do inside your code

When `VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS` is turned on, We use the `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT` as a way to signal to GPU-AV you want those shaders instrumented.

```c++
VkValidationFeatureEnableEXT enabled = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
VkValidationFeaturesEXT features = {};
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = &enabled;
```

From here is is just a matter of passing it into the correct pNext

```c++
// vkCreateShaderModule
VkShaderModuleCreateInfo module_ci = {};
module_ci.pNext = &features;

// Pipeline (only a single shader)
VkPipelineShaderStageCreateInfo stage_ci = {};
stage_ci.pNext = &features;

// Pipeline (all shaders)
VkGraphicsPipelineCreateInfo pipeline_ci = {};
pipeline_ci.pNext = &features;

// VK_EXT_shader_object
VkShaderCreateInfoEXT shader_ci = {};
shader_ci.pNext = &features;
```

# CDL Dump

If you have used https://github.com/LunarG/CrashDiagnosticLayer you might have a CDL Dump file.

If the file path is provided to `VK_LAYER_GPUAV_CDL_DUMP_PATH`/`gpuav_cdl_dump_path` GPU-AV will parse it and extract all the infomation out it for you.

This can be provided as supplemental, and will append to any other scoped information provided.