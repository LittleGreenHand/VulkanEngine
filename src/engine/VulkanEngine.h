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
#include "types.hpp"

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

	std::map<GLTFModels, vkglTF::Model> models;
	vkglTF::Model skybox;

	struct UniformBuffers {
		vks::Buffer globalParamBuffer;
	};
	std::array<UniformBuffers, maxConcurrentFrames> globalParamBuffers;

	struct alignas(16) GlobalParams {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat4 projection;
		glm::vec3 camPos;
		float exposure = 4.5f;
		float gamma = 2.2f;
	} globalParam;

	struct RenderPassInfo {
		VkRenderPass renderPass{ VK_NULL_HANDLE };
		
	};
	std::array<RenderPassInfo, RP_Count> renderPasses;

	struct PipelineInfo{
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkPipeline pipeline{ VK_NULL_HANDLE };
	};
	std::array<PipelineInfo, PL_Count> pipelines;

	VkDescriptorSetLayout MaterialDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout globalParamDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout IBLDescriptorLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout meshDescriptorSetLayout{ VK_NULL_HANDLE };
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
		camera.movementSpeed = 1.0f;
		camera.setPerspective(60.0f, (float)width / (float)height, 0.01f, 256.0f);
		camera.rotationSpeed = 0.25f;
		camera.setRotation({ 0.0f, 0.0f, 0.0f });
		camera.setPosition({ 0.f, 0.f, 0.f });
	}

	~VulkanEngine()
	{
		lights.destroy();
		if (device) {
			textures.environmentCube.destroy();
			textures.irradianceCube.destroy();
			textures.prefilteredCube.destroy();
			textures.lutBrdf.destroy();
			textures.albedoMap.destroy();
			textures.normalMap.destroy();
			textures.aoMap.destroy();
			textures.metallicMap.destroy();
			textures.roughnessMap.destroy();
			vkglTF::destroyEmptyTexture();
			for (auto& buffer : globalParamBuffers) {
				buffer.globalParamBuffer.destroy();
			}
			vkDestroyDescriptorSetLayout(device, MaterialDescriptorSetLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, globalParamDescriptorSetLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, IBLDescriptorLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, meshDescriptorSetLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, emptyDescriptorLayout, nullptr);
			for(auto& pipeline : pipelines)
			{
				if(pipeline.pipeline != VK_NULL_HANDLE)
					vkDestroyPipeline(device, pipeline.pipeline, nullptr);
				if(pipeline.pipelineLayout != VK_NULL_HANDLE)
					vkDestroyPipelineLayout(device, pipeline.pipelineLayout, nullptr);
			}
			for (auto& pass : renderPasses)
			{
				if(pass.renderPass != VK_NULL_HANDLE)
				vkDestroyRenderPass(device, pass.renderPass, nullptr);
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
	virtual void drawNodeTree() override;
	virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

