#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanDevice.h"
struct alignas(16) LightInfo {
	glm::vec4 position;
	glm::vec3 color;
};
const uint32_t MAX_LIGHTS = 16;
struct alignas(16) LightUbo {
	LightInfo lights[MAX_LIGHTS]{ {glm::vec4(0, 0, 0, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)},
					 { glm::vec4(-15, -15 * 0.5f, 15, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
					 { glm::vec4(15, -15 * 0.5f, 15, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
					 { glm::vec4(15, -15 * 0.5f, -15, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) } };
	int activeLightCount = 1;
};

class VulkanLights
{
public:
	vks::VulkanDevice* vulkanDevice;
	LightUbo lightData;

	vks::Buffer lightBuffer;
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
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
};