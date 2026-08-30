#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "base/VulkanDevice.h"

class VulkanRenderer;
namespace VulkanContext
{
	void Init(VulkanRenderer* renderer);
	void CleanUp();

	VulkanRenderer* GetVulkanRenderer();
	VkDevice GetVkDevice();
	VkInstance GetVkInstance();
	vks::VulkanDevice* GetVulkanDevice();
	uint32_t GetAPIVersion();
	VkPhysicalDevice GetVkPhysicalDevice();
	VkQueue GetGraphicsQueue();
	uint32_t GetGraphicsQueueFamily();
	uint32_t GetComputeQueueFamily();
	uint32_t GetTransferQueueFamily();
	uint32_t GetMinImageCount();
	uint32_t GetImageCount();
	VkFormat GetSwapChainColorFormat();
	VkCommandBuffer GetCurrentCommandBuffer();
	int GetRenderWidth();
	int GetRenderHeight();
};