#include "VulkanContext.h"
#include "VulkanRenderer.h"

VulkanRenderer* vulkanRenderer = nullptr;
bool init = false;
namespace VulkanContext
{
	void Init(VulkanRenderer* renderer)
	{
		vulkanRenderer = renderer;
		init = true;
	}

	void CleanUp()
	{
		vulkanRenderer = nullptr;
		init = false;
	}

	VulkanRenderer* GetVulkanRenderer()
	{
		return vulkanRenderer;
	}

	VkDevice GetVkDevice()
	{
		return vulkanRenderer->vulkanDevice->logicalDevice;
	}

	VkInstance GetVkInstance()
	{
		return vulkanRenderer->instance;
	}

	vks::VulkanDevice* GetVulkanDevice()
	{
		return vulkanRenderer->vulkanDevice;
	}

	uint32_t GetAPIVersion()
	{
		return vulkanRenderer->apiVersion;
	}

	VkPhysicalDevice GetVkPhysicalDevice()
	{
		return vulkanRenderer->vulkanDevice->physicalDevice;
	}

	VkQueue GetGraphicsQueue()
	{
		return vulkanRenderer->m_queue;
	}

	uint32_t GetGraphicsQueueFamily()
	{
		return vulkanRenderer->vulkanDevice->queueFamilyIndices.graphics;
	}

	uint32_t GetComputeQueueFamily()
	{
		return vulkanRenderer->vulkanDevice->queueFamilyIndices.compute;
	}

	uint32_t GetTransferQueueFamily()
	{
		return vulkanRenderer->vulkanDevice->queueFamilyIndices.transfer;
	}

	uint32_t GetMinImageCount()
	{
		return vulkanRenderer->swapChain.minImageCount;
	}

	uint32_t GetImageCount()
	{
		return vulkanRenderer->swapChain.imageCount;
	}

	VkFormat GetSwapChainColorFormat()
	{
		return vulkanRenderer->swapChain.colorFormat;
	}

	VkCommandBuffer GetCurrentCommandBuffer()
	{
		return vulkanRenderer->drawCmdBuffers[vulkanRenderer->currentBuffer];
	}

	int GetRenderWidth()
	{
		return vulkanRenderer->m_renderWidth;
	}

	int GetRenderHeight()
	{
		return vulkanRenderer->m_renderHeiht;
	}
}