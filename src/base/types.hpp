#pragma once
#include <vulkan/vulkan.h>
#include "VulkanTexture.h"

//不同类型的描述符的set编号
enum LayoutBindIndex {
	LBI_GLOBAL = 0,
	LBI_IBL,
	LBI_LIGHTS,
	LBI_MATERIALS,
	LBI_MESH,
	LBI_CUSTOM,
	LBI_COUNT
};

enum GLTFModels {
	M_Cube,
	M_Cerberus,
	M_Sponza,
	M_Sphere,
	M_Axis
};

struct RenderPassInfo {
	int32_t width = 0;
	int32_t height = 0;
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkFramebuffer frameBuffer{ VK_NULL_HANDLE };
	std::vector<vks::Texture> colorAttachments;
	vks::Texture depthAttachment;
		
	void destroy(VkDevice device)
	{
		if (renderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(device, renderPass, nullptr);
		if (frameBuffer != VK_NULL_HANDLE)
			vkDestroyFramebuffer(device, frameBuffer, nullptr);
		for (auto& color : colorAttachments)
			color.destroy();
		colorAttachments.clear();
		depthAttachment.destroy();
	}
};
//RenderPass索引
enum RenderPassesIndex {
	RP_PointLight = 0,	//用于生成ShadowMap
	RP_DirectLight,	//用于生成ShadowMap
	RP_Count
};

struct PipelineInfo {
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };
};
//Pipeline索引
enum PipelinesIndex {
	PL_Skybox = 0,
	PL_PBR,
	PL_Count
};