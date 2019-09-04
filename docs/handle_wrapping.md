<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Handle Wrapping Functionality
[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

The handle wrapping facility is a feature of the Khronos Layer which aliases all non-dispatchable Vulkan objects with a unique identifier at object-creation time. The aliased handles are used during validation to ensure that duplicate object handles are correctly managed and tracked by the validation layers. This enables consistent and coherent validation in addition to proper operation on systems which return non-unique object handles.

**Note**:

* If you are developing Vulkan extensions which include new APIs taking one or more Vulkan dispatchable objects as parameters, you may find it necessary to disable handle-wrapping in order use the validation layers. Options for disabling this facility in the Khronos validation Layer include the VkConfig utility, the vk_layer_settings.txt configuration file, the VK_LAYER_DISABLES environment variable, or the VK_EXT_validation_features extension.
