# VK\_LAYER\_LUNARG\_parameter\_validation
The `VK_LAYER_LUNARG_parameter_validation` validation layer checks the input parameters to API calls for validity. This layer performs the following tasks:

 - validation of structures; structures are recursed if necessary
 - validation of enumerated type values
 - null pointer conditions
 - stateless valid usage checks
 - validation of `VkResult`.
