#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanDevice.h"
#include "types.hpp"

namespace vkLight
{
	struct alignas(16) PointLightInfo {
		glm::vec4 position;
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 0.f);
		float range = 10.0f;			//光源影响范围
		int attenuationMode = 0;	//衰减模式 0:线性衰减 1:平方反比衰减 2:物理衰减
	};
	struct alignas(16) DirectLightInfo {
		glm::vec4 direct = glm::vec4(0.f, 1.0f, 1.0f, 0.f);
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 0.f);
		glm::mat4 directLightViewProj[8];
		int cascadeCount;
		int usePCF = 0;
	};
	const uint32_t MAX_POINTLIGHTS = 16;
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

		//glm::vec3 lightPos = glm::vec3(0, 0, 10);
		glm::vec3 center = glm::vec3(0.0f);
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::mat4 VP;//光源视角的视图投影矩阵
		// 尽量缩小深度范围，以获得更好的阴影贴图精度
		float zNear = 1.0f;
		float zFar = 96.0f;
		// 深度偏移因子,用于避免阴影伪影
		float depthBiasConstant = 1.25f;
		// 深度偏移斜率因子，根据多边形的斜率进行应用
		float depthBiasSlope = 1.75f;
		//光源与世界原点的距离，影响了从世界哪个位置开始绘制阴影贴图
		float lightDistance = 45.0f;
		//场景包围盒边长，值越小，阴影越清晰
		float boundSize = 10.0f;
		Dimensions sceneDimensions;

		RenderPassInfo* renderPass = nullptr;//此pass用于生成ShadowMap
		VkFormat shadowMapFormat{ VK_FORMAT_D16_UNORM };//ShadowMap的格式
		uint32_t shadowMapize{ 2048 };
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
				vulkanDevice = nullptr;
			}
			renderPass = nullptr;
		}
		void updateDimensions();
		void updateVPMatrix();

		void prepare(vks::VulkanDevice* vulkanDevice, VkFormat depthFormat, RenderPassInfo* renderPass)
		{
			this->vulkanDevice = vulkanDevice;
			device = vulkanDevice->logicalDevice;
			this->renderPass = renderPass;
			shadowMapFormat = depthFormat;
			prepareFramebuffer();
			preperPipeline();
			updateVPMatrix();
		}
		void prepareFramebuffer();
		void preperRenderPass(bool useDepth = true);
		void preperPipeline();

		//绘制阴影贴图
		void Render(VkCommandBuffer cmdBuffer);
	};
}