#pragma once

#include "VulkanContext.h"

class PostProcessBase
{
public:
	static vks::VulkanDevice* vulkanDevice;
	static VkDevice device;
	static VkPipelineShaderStageCreateInfo fullScreenShaderStage;
	static VkDescriptorPool descriptorPool;
	static uint32_t width;
	static uint32_t height;

	static void preparePostProcessBase(vks::VulkanDevice* vulkandevice);
	static void cleanUp();
	static void update(uint32_t Width, uint32_t Height);

};

class PostProcessToneMapping;
class PostProcessDOF;
class PostProcessManager
{
public:
	PostProcessToneMapping* toneMappingProcess = nullptr;
	PostProcessDOF* dofProcess = nullptr;
public:
	void prepare();
	void destroyALL();
};