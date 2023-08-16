#pragma once
#include <vulkan/vulkan_core.h>

VkImageLayout NormalizeDepthImageLayout(VkImageLayout layout);
VkImageLayout NormalizeStencilImageLayout(VkImageLayout layout);
VkImageLayout NormalizeSynchronization2Layout(const VkImageAspectFlags aspect_mask, VkImageLayout layout);
bool ImageLayoutMatches(const VkImageAspectFlags aspect_mask, VkImageLayout a, VkImageLayout b);
