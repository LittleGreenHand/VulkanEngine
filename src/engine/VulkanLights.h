#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanDevice.h"
#include "types.hpp"

namespace vkLight
{
	const uint32_t MAX_POINTLIGHTS = 16;
	const uint32_t MAX_CASCADES = 8;
	struct alignas(16) PointLightInfo {
		glm::vec4 position;
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 0.f);
		float range = 10.0f;			//光源影响范围
		int attenuationMode = 0;	//衰减模式 0:线性衰减 1:平方反比衰减 2:物理衰减
	};
	struct alignas(16) DirectLightInfo {
		glm::vec4 direct = glm::vec4(0.f, 1.0f, 1.0f, 0.f);
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 0.f);
		glm::mat4 ViewProj[MAX_CASCADES];
		//每个float元素按16字节对齐（std140要求）
		struct alignas(16) CascadeSplit {
			float value;
		} cascadeSplits[MAX_CASCADES];
		int cascadeCount = 3; //实际使用的级联数量
		int usePCF = 1;
		int colorCascades = 0;

	};
	struct alignas(16) LightUbo {
		PointLightInfo pointLights[MAX_POINTLIGHTS]{};
		int activePointLightCount = 1;

		DirectLightInfo directLight;
	};

	//此layout由VulkanEngine统一销毁,其中存储的是所有灯光资源的描述符集布局
	extern VkDescriptorSetLayout descriptorSetLayout;
	extern VkDescriptorSet descriptorSet;
	extern LightUbo lightData;
	extern vks::Buffer lightBuffer;
	void preperDescriptor(vks::VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool);
	void updateLightBuffer();
	void destroyLightBuffer();

	class VulkanPointLights
	{
	public:
		vks::VulkanDevice* vulkanDevice;
		VkDevice device;

		RenderPassInfo* renderPass = nullptr;//此pass用于生成ShadowMap
		VkFormat shadowMapFormat;//ShadowMap的格式
		uint32_t shadowMapize{ 2048 };
	public:
		~VulkanPointLights()
		{
			destroy();
		}
		void destroy()
		{
			vulkanDevice = nullptr;
			renderPass = nullptr;
		}
		void setLightPosition(uint32_t index, glm::vec3 pos) {
			if (index >= MAX_POINTLIGHTS) return;
			lightData.pointLights[index].position = glm::vec4(pos, 1.0f);
		}

		void setLightColor(uint32_t index, glm::vec3 color) {
			if (index >= MAX_POINTLIGHTS) return;
			lightData.pointLights[index].color = glm::vec4(color, 1.0f);
		}

		void setActiveLightCount(uint32_t count) {
			if (count > MAX_POINTLIGHTS) count = MAX_POINTLIGHTS;
			lightData.activePointLightCount = count;
		}

		void prepare(vks::VulkanDevice* vulkanDevice, VkFormat depthFormat, RenderPassInfo* renderPass)
		{
			this->vulkanDevice = vulkanDevice;
			device = vulkanDevice->logicalDevice;
			this->renderPass = renderPass;
			shadowMapFormat = depthFormat;
			prepareFramebuffer();
		}
		void prepareFramebuffer();
		void preperRenderPass(bool useDepth = true);
	};

	class VulkanDirectLights
	{
	public:
		vks::VulkanDevice* vulkanDevice{nullptr};
		VkDevice device{ VK_NULL_HANDLE };
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

		//是否需要更新描述符集
		bool isDescriptorUpdated = true;
		// 深度偏移因子,用于避免阴影伪影
		float depthBiasConstant = 0.f;
		// 深度偏移斜率因子，根据多边形的斜率进行应用
		float depthBiasSlope = 0.f;
		float cascadeSplitLambda = 0.75f;

		RenderPassInfo* renderPass = nullptr;//此pass用于生成ShadowMap
		VkFormat shadowMapFormat{ VK_FORMAT_D16_UNORM };//ShadowMap的格式
		uint32_t shadowMapize{ 4096 };
		struct Cascade {
			VkFramebuffer frameBuffer{ VK_NULL_HANDLE };
			VkImageView view{ VK_NULL_HANDLE };
			float splitDepth{0.f};
			glm::mat4 viewProjMatrix;
			glm::mat4 viewMat;
			glm::mat4 Proj;

			void destroy(VkDevice device) const {
				if (view != VK_NULL_HANDLE)
					vkDestroyImageView(device, view, nullptr);
				if (frameBuffer != VK_NULL_HANDLE)
					vkDestroyFramebuffer(device, frameBuffer, nullptr);
			}
		};
		Cascade cascades[MAX_CASCADES];
	public:
		~VulkanDirectLights()
		{
			destroy();
		}
		void destroy()
		{
			if (vulkanDevice) {
				if (pipeline != VK_NULL_HANDLE)
					vkDestroyPipeline(vulkanDevice->logicalDevice, pipeline, nullptr);
				if (pipelineLayout != VK_NULL_HANDLE)
					vkDestroyPipelineLayout(vulkanDevice->logicalDevice, pipelineLayout, nullptr);
				for (int i = 0; i < MAX_CASCADES; ++i)
					cascades[i].destroy(vulkanDevice->logicalDevice);
				vulkanDevice = nullptr;
			}
			renderPass = nullptr;
		}
		void updateCascades();

		void prepare(vks::VulkanDevice* vulkanDevice, VkFormat depthFormat, RenderPassInfo* renderPass)
		{
			std::cout << "directLight.cascadeSplits offset: " << offsetof(DirectLightInfo, cascadeSplits) << std::endl;
			std::cout << "directLight.cascadeCount offset: " << offsetof(DirectLightInfo, cascadeCount) << std::endl;
			this->vulkanDevice = vulkanDevice;
			device = vulkanDevice->logicalDevice;
			this->renderPass = renderPass;
			shadowMapFormat = depthFormat;
			prepareFramebuffer();
			preperPipeline();
			updateCascades();
		}
		void prepareFramebuffer();
		void preperRenderPass(bool useDepth = true);
		void preperPipeline();

		//绘制阴影贴图
		void Render(VkCommandBuffer cmdBuffer);
	};
}