// NOTE: This header file is installed by the LunarG SDK and treated as part of VulkanHeaders
// IE: This header file is also a deliverable of VVL.
// NOTE: It needs to be in this exact location.
#include <vulkan/vk_enum_string_helper.h>

// Test other locations used by LunarG/VulkanTools
#include <generated/vk_enum_string_helper.h>
#include <vk_enum_string_helper.h>

static inline const char* foobar() { return string_VkResult(VK_SUCCESS); }

// Includes needed for LunarG/VulkanTools
#include <error_message/logging.h>
#include <containers/custom_containers.h>
#include <utils/vk_layer_extension_utils.h>
#include <utils/vk_layer_utils.h>
#include <generated/vk_dispatch_table_helper.h>
#include <generated/vk_extension_helper.h>
