#pragma once

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
#include <map>
#include <vulkan/vulkan.h>
#include "vulkanEngineBase.h"
#include "VulkanglTFModel.h"
#include "VulkanLights.h"

//前向声明
class PostProcessManager;

class VulkanEngine : public VulkanEngineBase
{
public:
	bool displaySkybox = true;
	vkLight::VulkanPointLights pointLights;
	vkLight::VulkanDirectLights directLight;
	PostProcessManager* postProcessManager = nullptr;

	struct Textures {
		vks::TextureCubeMap environmentCube;
		// Generated at runtime
		vks::Texture2D lutBrdf;
		vks::TextureCubeMap irradianceCube;
		vks::TextureCubeMap prefilteredCube;
		// Object texture maps
		vks::Texture2D albedoMap;
		vks::Texture2D normalMap;
		vks::Texture2D aoMap;
		vks::Texture2D metallicMap;
		vks::Texture2D roughnessMap;
	} textures{};

	std::map<GLTFModels, vkglTF::Model> models;
	vkglTF::Model skybox;

	std::array<PipelineInfo, PL_Count> pipelines{};
	std::array<VkDescriptorSetLayout, LBI_COUNT> setLayouts{};
	VkDescriptorSet IBLDescriptorSet{ VK_NULL_HANDLE };

	struct alignas(16) GlobalParams {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat4 projection;
		glm::vec3 camPos;
		float exposure = 4.5f;
		float gamma = 2.2f;
	} globalParam;
	struct UniformBuffers {
		vks::Buffer globalParamBuffer;
	};
	std::array<UniformBuffers, maxConcurrentFrames> globalParamBuffers;

	//每帧独立使用的全局参数的描述符集
	std::array<VkDescriptorSet, maxConcurrentFrames> globalDescriptorSets{};

	VkPhysicalDeviceVulkan11Features Features11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

	VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT libraryFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT };
	VulkanEngine() : VulkanEngineBase()
	{
		title = "VulkanEngine";
		camera.type = Camera::CameraType::firstperson;
		camera.movementSpeed = 8.0f;
		camera.setPerspective(60.0f, (float)width / (float)height, 0.01f, 256.0f);

		camera.rotationSpeed = 0.25f;
		camera.setRotation({ 0.0f, 0.0f, 0.0f });
		camera.setPosition({ 0.f, 0.f, 0.f });
	}

	~VulkanEngine();

	virtual void getEnabledFeatures() override;
	virtual void getEnabledExtensions() override;
	void buildCommandBuffer();
	void loadAssets();
	void prepareDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void preparePostProcess();
	void updateUniformBuffers();
	//初始化引擎各类资源
	void prepare() override;
	virtual void render() override;
	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay) override;
	virtual void drawNodeTree() override;
	virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

