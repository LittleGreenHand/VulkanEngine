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

class VulkanEngine : public VulkanEngineBase
{
public:
	bool displaySkybox = true;

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
		vkglTF::Model object;
	} models;

	struct UniformBuffers {
		vks::Buffer scene;
		vks::Buffer skybox;
		vks::Buffer params;
	};
	std::array<UniformBuffers, maxConcurrentFrames> uniformBuffers;

	struct UniformDataMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec3 camPos;
	} uniformDataMatrices;

	struct UniformDataParams {
		glm::vec4 lights[4];
		float exposure = 4.5f;
		float gamma = 2.2f;
		float globalRoughness = 1.0f;
		float globalMetallic = 1.0f;
	} uniformDataParams;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	struct {
		VkPipeline skybox{ VK_NULL_HANDLE };
		VkPipeline pbr{ VK_NULL_HANDLE };
	} pipelines;

	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	struct DescriptorSets {
		VkDescriptorSet scene{ VK_NULL_HANDLE };
		VkDescriptorSet skybox{ VK_NULL_HANDLE };
	};
	std::array<DescriptorSets, maxConcurrentFrames> descriptorSets{};
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
		if (device) {
			vkDestroyPipeline(device, pipelines.skybox, nullptr);
			vkDestroyPipeline(device, pipelines.pbr, nullptr);
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
			textures.environmentCube.destroy();
			textures.irradianceCube.destroy();
			textures.prefilteredCube.destroy();
			textures.lutBrdf.destroy();
			textures.albedoMap.destroy();
			textures.normalMap.destroy();
			textures.aoMap.destroy();
			textures.metallicMap.destroy();
			textures.roughnessMap.destroy();
			for (auto& buffer : uniformBuffers) {
				buffer.scene.destroy();
				buffer.params.destroy();
				buffer.skybox.destroy();
			}
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

