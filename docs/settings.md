# Settings

[documentation has been moved](./khronos_validation_layer.md#configuring-the-validation-layer)

## Legacy

This is only here to document the legacy of how we got to this situation.

Long ago validation created its own system to parse environmental variables as well as the `vk_layer_settings.txt` flow.

Then we created `VK_EXT_validation_features` as way to set things at `vkCreateInstance` time. `VK_EXT_validation_flags` was also created after with the same goals in mind.

As more and more layers basically needed to do what Validation Layers were doing, we just ended up creating `VK_EXT_layer_settings` as a final solution. This extension has been the way forward to a better system of creating settings for layers.

Unfortunately, we still support some legacy names, so this prevents us from making everything consistent.