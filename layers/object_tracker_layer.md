# VK\_LAYER\_LUNARG\_object\_tracker
The `VK_LAYER_LUNARG_object_tracker` layer tracks all Vulkan objects. Object lifetimes are validated along with issues related to unknown objects and object destruction and cleanup.

All Vulkan dispatchable and non-dispatchable objects are tracked by the `VK_LAYER_LUNARG_object_tracker` layer.

This layer validates that:

 - only known objects are referenced and destroyed
 - lookups are performed only on objects being tracked
 - objects are correctly freed/destroyed.

The `VK_LAYER_LUNARG_object_tracker` layer will print errors if validation checks are not correctly met and warnings if improper reference of objects is detected.
