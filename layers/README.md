# Layer Description and Status

## Layer Library Interface

All layer libraries must support the layer library interface defined in
[`LoaderAndLayerInterface.md`][].

[`LoaderAndLayerInterface.md`]: ../loader/LoaderAndLayerInterface.md#layer-library-interface

## Overview

Layer libraries can be written to intercept or hook VK entry points for various
debug and validation purposes.  One or more VK entry points can be defined in your Layer
library.  Undefined entrypoints in the Layer library will be passed to the next Layer which
may be the driver.  Multiple layer libraries can be chained (actually a hierarchy) together.
vkEnumerateInstanceLayerProperties can be called to list the
available layers and their properties.  Layers can intercept all Vulkan commands
that take a dispatchable object as it's first argument. I.e.  VkInstance, VkPhysicalDevice,
VkDevice, VkCommandBuffer, and VkQueue.
vkXXXXGetProcAddr is used internally by the Layers and Loader to initialize dispatch tables.
Layers can also be activated via the VK_INSTANCE_LAYERS environment variable.

All validation layers work with the DEBUG_REPORT extension to provide validation feedback.
When a validation layer is enabled, it will look for a vk_layer_settings.txt file (see"Using
Layers" section below for more details) to define its logging behavior, which can include
sending output to a file, stdout, or debug output (Windows). Applications can also register
debug callback functions via the DEBUG_REPORT extension to receive callbacks when validation
events occur. Application callbacks are independent of settings in a vk_layer_settings.txt
file which will be carried out separately. If no vk_layer_settings.txt file is present and
no application callbacks are registered, error messages will be output through default
logging callbacks.

### Layer library example code

Note that some layers are code-generated and will therefore exist in the directory `(build_dir)/layers`

`include/vkLayer.h` - header file for layer code.

### Standard Validation
NOTE: This meta-layer is being deprecated -- users should load the Khronos validation layer (name = `VK_LAYER_KHRONOS_validation`) in its place.
This is a meta-layer managed by the loader. (name = `VK_LAYER_LUNARG_standard_validation`) - specifying this layer name will cause the loader to load the standard validation layer:  `VK_LAYER_KHRONOS_validation`. Other layers can be specified and the loader will remove duplicates.

### The Khronos Validation Layer

This layer emcompasses all of the functionality that used to be contained in the following layers: VK_LAYER_GOOGLE_threading, VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects. Each of these functional areas can still disabled individually, and are described below.

### Object Validation and Statistics
The object lifetime tracking will track object creation, use, and destruction. As objects are created their handles are stored in a data structure. As objects are used the layer verifies they exist in the data structure and output errors for unknown objects. As objects are destroyed they are removed from the data structure. At `vkDestroyDevice()` and `vkDestroyInstance()` times, if any objects have not been destroyed they are reported as leaked objects. If a debug callback function is registered this layer will use callback function(s) for reporting, otherwise it will use stdout.

### Validate API State and Shaders
The set of core checks does the bulk of the API validation that requires storing state. Some of the state it tracks includes the Descriptor Set, Pipeline State, Shaders, and dynamic state, and memory objects and bindings. It performs some point validation as states are created and used, and further validation Draw call and QueueSubmit time. Of primary interest is making sure that the resources bound to Descriptor Sets correctly align with the layout specified for the Set. Also, all of the image and buffer layouts are validated to make sure explicit layout transitions are properly managed. Related to memory, core\_validation includes tracking object bindings, memory hazards, and memory object lifetimes. It also validates several other hazard-related issues related to command buffers, fences, and memory mapping. Additionally core\_validation include shader validation (formerly separate shader\_checker layer) that inspects the SPIR-V shader images and fixed function pipeline stages at PSO creation time. It flags errors when inconsistencies are found across interfaces between shader stages. The exact behavior of the checks depends on the pair of pipeline stages involved. If a debug callback function is registered, this layer will use callback function(s) for reporting, otherwise uses stdout.  This layer also validates correct usage of image- and buffer-related APIs, including image and buffer parameters, formats, and correct use.

### Stateless parameter checking
The stateless validation checks the input parameters to API calls for validity. If a debug callback function is registered, this layer will use callback function(s) for reporting otherwise uses stdout.

### Thread Safety Checking
The thread-safety validation will check the multithreading of API calls for validity. Currently this checks that only one thread at a time uses an object in free-threaded API calls. If a debug callback function is registered, this layer will use callback function(s) for reporting, otherwise uses stdout.

### Handle Wrapping
The khronos layer framework also supports Vulkan handle wrapping.  The Vulkan specification allows objects to have non-unique handles. This makes tracking object lifetimes difficult in that it is unclear which object is being referenced on deletion. When this functionalty is enabled (as it is by default) it will alias all objects with a unique object representation, allowing proper object lifetime tracking. This functionality may interfere with the development of proprietary Vulkan extension development, and is not strictly required for the proper operation of validation. One sign that it is needed is the appearance of errors emitted from the object_tracker layer indicating the use of previously destroyed objects.

## Using Layers

1. Build VK loader using normal steps (cmake and make)
2. Place `libVkLayer_khronos_validation.so` in the same directory as your VK test or app:

    `cp build/layer/libVkLayer_khronos_validation.so  build/tests`

    This is required for the Loader to be able to scan and enumerate your library.
    Alternatively, use the `VK_LAYER_PATH` environment variable to specify where the layer libraries reside.

3. To specify how your layers should behave, create a vk_layer_settings.txt file. This file can exist in the same directory as your app or in a directory given by the `VK_LAYER_SETTINGS_PATH` environment variable. Alternatively, you can use any filename you want if you set `VK_LAYER_SETTINGS_PATH` to the full path of the file, rather than the directory that contains it.

    Model the file after the following example:  [*vk_layer_settings.txt*](vk_layer_settings.txt)

4. Specify which layers to activate using environment variables.

    `export VK\_INSTANCE\_LAYERS=VK\_LAYER\_KHRONOS\_validation`
    `cd build/tests; ./vkinfo`


## Status


### Current known issues

