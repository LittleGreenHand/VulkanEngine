#pragma once
#include <vulkan/vulkan.h>
#include "base/VulkanTexture.h"

namespace VulkanImageUtils
{
	void TransitionImageLayout(VkCommandBuffer cmd, vks::Texture& texture, VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, VkPipelineStageFlags2 srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VkAccessFlags2 srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT, VkPipelineStageFlags2 dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VkAccessFlags2 dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT);
	void CopyImageToImage(VkCommandBuffer cmd, vks::Texture& srcTexture, vks::Texture& dstTexture);

}