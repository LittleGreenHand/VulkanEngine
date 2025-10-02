#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanDevice.h"
struct alignas(16) LightInfo {
	glm::vec4 position;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float range = 10.0f;			//光源影响范围
	int attenuationMode = 0;	//衰减模式 0:线性衰减 1:平方反比衰减 2:物理衰减
};
const uint32_t MAX_LIGHTS = 16;
struct alignas(16) LightUbo {
	LightInfo lights[MAX_LIGHTS]{};
	int activeLightCount = 1;
};

const uint32_t shadowMapize{ 2048 };
class VulkanLights
{
public:
	vks::VulkanDevice* vulkanDevice;
	LightUbo lightData;

	vks::Buffer lightBuffer;
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

	VkRenderPass renderPass{ VK_NULL_HANDLE };//用于生成ShadowMap
public:
	~VulkanLights()
	{
		destroy();
	}
	void destroy()
	{
		if (vulkanDevice) {
			vkDestroyDescriptorSetLayout(vulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
			vkDestroyDescriptorPool(vulkanDevice->logicalDevice, descriptorPool, nullptr);
			lightBuffer.destroy();
			vulkanDevice = nullptr;
		}
	}
	void setLightPosition(uint32_t index, glm::vec3 pos) {
		if (index >= MAX_LIGHTS) return;
		lightData.lights[index].position = glm::vec4(pos, 1.0f);
	}

	void setLightColor(uint32_t index, glm::vec3 color) {
		if (index >= MAX_LIGHTS) return;
		lightData.lights[index].color = glm::vec4(color, 1.0f);
	}

	void setActiveLightCount(uint32_t count) {
		if (count > MAX_LIGHTS) count = MAX_LIGHTS;
		lightData.activeLightCount = count;
	}

	void preperDescriptor(vks::VulkanDevice* vulkanDevice);

	void updateLightBuffer();

	VkRenderPass preperRenderPass(VkFormat depthFormat, bool useDepth = true);
};