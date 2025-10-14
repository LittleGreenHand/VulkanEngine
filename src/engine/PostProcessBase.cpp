#include "PostProcessBase.h"
#include "PostProcess_ToneMapping.h"
#include "PostProcess_DOF.h"

vks::VulkanDevice* PostProcessBase::vulkanDevice = nullptr;
VkDevice PostProcessBase::device = VK_NULL_HANDLE;
VkPipelineShaderStageCreateInfo PostProcessBase::fullScreenShaderStage;
VkDescriptorPool PostProcessBase::descriptorPool = VK_NULL_HANDLE;
uint32_t PostProcessBase::width;
uint32_t PostProcessBase::height;
void PostProcessBase::preparePostProcessBase(vks::VulkanDevice* vulkandevice)
{
	vulkanDevice = vulkandevice;
	device = vulkanDevice->logicalDevice;

	//预加载全屏三角形顶点着色器
	fullScreenShaderStage = vkUtils::vkEngine->loadShader(vkUtils::vkEngine->getShadersPath() + "PostProcess_fullScreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);

	// 创建所有后处理共享的DescriptorPool
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32)
		};
		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 32);
		descriptorPoolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)descriptorPool, "PostProcess DescriptorPool");
	}
}
void PostProcessBase::cleanUp()
{
	if (descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vulkanDevice = nullptr;
	device = VK_NULL_HANDLE;
}

void PostProcessBase::update(uint32_t Width, uint32_t Height)
{
	width = Width;
	height = Height;
}

void PostProcessManager::prepare()
{
	if (!toneMappingProcess)
		toneMappingProcess = new PostProcessToneMapping();
	toneMappingProcess->prepare();

	if (!dofProcess)
		dofProcess = new PostProcessDOF();
	dofProcess->prepare();
}

void PostProcessManager::destroyALL()
{
	if(toneMappingProcess)
	{
		toneMappingProcess->destroy();
		delete toneMappingProcess;
		toneMappingProcess = nullptr;
	}

	if(dofProcess)
	{
		dofProcess->destroy();
		delete dofProcess;
		dofProcess = nullptr;
	}
}