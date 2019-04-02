<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2019 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK\_LAYER\_LUNARG\_parameter\_validation

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

The `VK_LAYER_LUNARG_parameter_validation` validation layer checks the input parameters to API calls for validity. This layer performs the following tasks:

- validation of structures; structures are recursed if necessary
- validation of enumerated type values
- null pointer conditions
- stateless valid usage checks
- validation of `VkResult`.
