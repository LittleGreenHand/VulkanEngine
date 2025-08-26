/*
* Vulkan Example - Basic indexed triangle rendering using Vulkan 1.3
*
* Note:
* This is a variation of the the triangle sample that makes use of Vulkan 1.3 features
* This simplifies the api a bit, esp. with dynamic rendering replacing render passes (and with that framebuffers)
*
* Copyright (C) 2024-2025 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <vector>
#include <exception>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "vulkanEngineBase.h"
#include "VulkanglTFModel.h"

class VulkanEngine : public VulkanEngineBase
{
public:
	vkglTF::Model scene;

	struct ShadingRateImage {
		VkImage image{ VK_NULL_HANDLE };
		VkDeviceMemory memory;
		VkImageView view;
	} shadingRateImage;

	bool enableShadingRate = true;
	bool colorShadingRate = false;

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model = glm::mat4(1.0f);
		glm::vec4 lightPos = glm::vec4(0.0f, 2.5f, 0.0f, 1.0f);
		glm::vec4 viewPos;
		int32_t colorShadingRate;
	} uniformData;
	std::array<vks::Buffer, maxConcurrentFrames> uniformBuffers;

	struct Pipelines {
		VkPipeline opaque{ VK_NULL_HANDLE };
		VkPipeline masked{ VK_NULL_HANDLE };
	} pipelines;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, maxConcurrentFrames> descriptorSets{};

	VkPhysicalDeviceFragmentShadingRatePropertiesKHR physicalDeviceShadingRateImageProperties{};
	VkPhysicalDeviceFragmentShadingRateFeaturesKHR enabledPhysicalDeviceShadingRateImageFeaturesKHR{};

	PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR vkGetPhysicalDeviceFragmentShadingRatesKHR{ nullptr };
	PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR{ nullptr };
	PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR{ nullptr };

	VulkanEngine();
	~VulkanEngine();
	virtual void getEnabledFeatures() override;
	void handleResize();
	void buildCommandBuffer();
	void loadAssets();
	void prepareShadingRateImage();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void prepare() override;
	void setupFrameBuffer() override;
	void setupRenderPass() override;
	virtual void render() override;
	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay) override;
};

