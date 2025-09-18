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

#include <vulkan/vulkan.h>
#include "vulkanEngineBase.h"
#include "VulkanglTFModel.h"
#include "PipelineBuilder.h"
#include "VulkanLights.h"

//不同类型的描述符的set编号
enum LayoutBindIndex {
	LBI_GLOBAL = 0,
	LBI_IBL,
	LBI_LIGHTS,
	LBI_MATERIALS,
	LBI_COUNT
};
class VulkanEngine : public VulkanEngineBase
{
public:
	bool displaySkybox = true;
	VulkanLights lights;

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

	struct Meshes {
		vkglTF::Model skybox;
		vkglTF::Model cerberus;
		vkglTF::Model sponza;
	} models;

	struct UniformBuffers {
		vks::Buffer globalParamBuffer;
	};
	std::array<UniformBuffers, maxConcurrentFrames> globalParamBuffers;

	struct GlobalParams {
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 camPos;
		float exposure = 4.5f;
		float gamma = 2.2f;
	} globalParam;

	struct {
		VkPipelineLayout skyboxPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline skybox{ VK_NULL_HANDLE };

		VkPipelineLayout pbrPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline pbr{ VK_NULL_HANDLE };
	} pipelines;

	VkDescriptorSetLayout MaterialDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout globalParamDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout IBLDescriptorLayout{ VK_NULL_HANDLE };
	VkDescriptorSet IBLDescriptorSet{ VK_NULL_HANDLE };
	struct Descriptor {
		VkDescriptorSet globalParamDescriptorSet{ VK_NULL_HANDLE };
	};
	VkDescriptorSetLayout emptyDescriptorLayout{ VK_NULL_HANDLE };

	std::array<Descriptor, maxConcurrentFrames> frameDescriptorSets{};
	VkPhysicalDeviceVulkan11Features vulkan11Features{};

	VulkanEngine() : VulkanEngineBase()
	{
		title = "VulkanEngine";
		camera.type = Camera::CameraType::firstperson;
		camera.movementSpeed = 4.0f;
		camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
		camera.rotationSpeed = 0.25f;
		camera.setRotation({ -7.75f, 150.25f, 0.0f });
		camera.setPosition({ 0.7f, 0.1f, 1.7f });
	}

	~VulkanEngine()
	{
		lights.destroy();
		if (device) {
			vkDestroyPipeline(device, pipelines.skybox, nullptr);
			vkDestroyPipeline(device, pipelines.pbr, nullptr);
			vkDestroyPipelineLayout(device, pipelines.pbrPipelineLayout, nullptr);
			vkDestroyPipelineLayout(device, pipelines.skyboxPipelineLayout, nullptr);
			textures.environmentCube.destroy();
			textures.irradianceCube.destroy();
			textures.prefilteredCube.destroy();
			textures.lutBrdf.destroy();
			textures.albedoMap.destroy();
			textures.normalMap.destroy();
			textures.aoMap.destroy();
			textures.metallicMap.destroy();
			textures.roughnessMap.destroy();
			for (auto& buffer : globalParamBuffers) {
				buffer.globalParamBuffer.destroy();
			}
			vkDestroyDescriptorSetLayout(device, MaterialDescriptorSetLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, globalParamDescriptorSetLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, IBLDescriptorLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, emptyDescriptorLayout, nullptr);
		}
	}

	virtual void getEnabledFeatures() override;
	void buildCommandBuffer();
	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void prepare() override;
	virtual void render() override;
	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay) override;
	virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

