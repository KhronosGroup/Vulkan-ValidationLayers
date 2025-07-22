# Settings

[documentation for users has been moved](./khronos_validation_layer.md#configuring-the-validation-layer)

## Developers

For those dealing with settings while developing, it can be hard as adding a setting will involve remembering many random spots to update.

To try and help relieve the pain, there is a [generate_settings.py](../scripts/generate_settings.py) file that will take the [VkLayer_khronos_validation.json.in](../layers/VkLayer_khronos_validation.json.in) and parse the settings from it and generate the other files to be checked in

## Legacy

This is only here to document the legacy of how we got to this situation.

Long ago validation created its own system to parse environmental variables as well as the `vk_layer_settings.txt` flow.

Then we created `VK_EXT_validation_features` as way to set things at `vkCreateInstance` time. `VK_EXT_validation_flags` was also created after with the same goals in mind.

As more and more layers basically needed to do what Validation Layers were doing, we just ended up creating `VK_EXT_layer_settings` as a final solution. This extension has been the way forward to a better system of creating settings for layers.

Unfortunately, we still support some legacy names, so this prevents us from making everything consistent.