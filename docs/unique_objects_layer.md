<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK\_LAYER\_GOOGLE\_unique\_objects
[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

The `VK_LAYER_LUNARG_unique_objects` is a validation-supporting utility layer which enables consistent and coherent validation in addition to proper operation on systems which return non-unique object handles.  This layer aliases all non-dispatchable Vulkan objects with a unique identifier at object-creation time. The aliased handles are used during validation to ensure that duplicate object handles are correctly managed and tracked by the validation layers.

**Note**:

* For optimal efficiency, this layer MUST be last in the chain (closest to the display driver).
* If you are developing Vulkan extensions which include new APIs taking one or more Vulkan dispatchable objects as parameters, you may find it necessary to disable the unique objects layer in order use the validation layers. The best way to do this is to explicitly load the layers in the optimal order specified earlier but without this layer. This should result in a minimal decrease in functionality but still allow you to benefit from using the validation layers.
