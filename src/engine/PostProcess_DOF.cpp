#include "PostProcess_DOF.h"
#include "PipelineBuilder.h"

void PostProcessDOF::prepare()
{
	prepareDescriptorSet();
	preparePipeline();
}
void PostProcessDOF::destroy()
{
	if (pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}
	if (pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
	if (descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		descriptorSetLayout = VK_NULL_HANDLE;
	}
	for (auto& buffer : dofParamBuffer) {
		buffer.destroy();
	}
}

void PostProcessDOF::setDOFParams(float zNear, float zFar, float focusDistance, float focusRange, float maxBlurRadius, float aperture)
{
	params.zNear = zNear;
	params.zFar = zFar;
	params.focusDistance = focusDistance;
	params.focusRange = focusRange;
	params.maxBlurRadius = maxBlurRadius;
	params.aperture = aperture;
	params.texelSize = glm::vec2(1.0f / (float)width, 1.0f / (float)height);
	memcpy(dofParamBuffer[vkUtils::vkEngine->currentBuffer].mapped, &params, sizeof(DOFParams));
}

void PostProcessDOF::excute(VkCommandBuffer cmdBuffer, VkDescriptorImageInfo colorImageInfo, VkDescriptorImageInfo depthImageInfo, VkImageView writeImage)
{
	vkUtils::cmdBeginLabel(cmdBuffer, "DOF", { 1.0f, 1.0f, 1.0f });

	BindDescriptorSets(cmdBuffer, colorImageInfo, depthImageInfo);
	VkRenderingAttachmentInfo colorAttachment = vks::initializers::RenderingAttachmentInfo_Color(writeImage, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = vks::initializers::RenderingInfo({ width, height }, &colorAttachment, nullptr);
	vkCmdBeginRendering(cmdBuffer, &renderInfo);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	vkCmdEndRendering(cmdBuffer);

	vkUtils::cmdEndLabel(cmdBuffer);
}

void PostProcessDOF::BindDescriptorSets(VkCommandBuffer cmdBuffer, VkDescriptorImageInfo colorImageInfo, VkDescriptorImageInfo depthImageInfo)
{
	// 更新描述符集
	{

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSet[vkUtils::vkEngine->currentBuffer], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &dofParamBuffer[vkUtils::vkEngine->currentBuffer].descriptor),
			vks::initializers::writeDescriptorSet(descriptorSet[vkUtils::vkEngine->currentBuffer], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &colorImageInfo),
			vks::initializers::writeDescriptorSet(descriptorSet[vkUtils::vkEngine->currentBuffer], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &depthImageInfo)
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	//绑定描述符集
	{
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, LBI_GLOBAL, 1, &vkUtils::vkEngine->globalDescriptorSets[vkUtils::vkEngine->currentBuffer], 0, nullptr);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, LBI_CUSTOM, 1, &descriptorSet[vkUtils::vkEngine->currentBuffer], 0, nullptr);
	}
}
void PostProcessDOF::prepareDescriptorSet()
{
	//创建描述符集布局
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2));

		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
		descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorLayoutCI.pBindings = setLayoutBindings.data();
		descriptorLayoutCI.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &descriptorSetLayout));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)descriptorSetLayout, "PostProcess_DOF DescriptorSetLayout ");
	}

	//创建描述符集
	{
		// 分配描述符集
		for (auto& descriptor : descriptorSet)
		{
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptor));
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)descriptor, "PostProcess_DOF DescriptorSet");
		}
	}

	//创建并更新描述符缓冲
	{
		for (auto& buffer : dofParamBuffer)
		{
			VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, sizeof(DOFParams), nullptr));
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer.buffer, "PostProcess_DOF dofParamBuffer");
			buffer.map();
		}
	}
}

void PostProcessDOF::preparePipeline()
{
	std::array<VkDescriptorSetLayout, LBI_COUNT> setLayoutsVector;
	setLayoutsVector.fill(VK_NULL_HANDLE);
	setLayoutsVector[LBI_GLOBAL] = vkUtils::vkEngine->setLayouts[LBI_GLOBAL];
	setLayoutsVector[LBI_CUSTOM] = descriptorSetLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayoutsVector);
	pipelineLayoutCreateInfo.setLayoutCount = LBI_COUNT;
	pipelineLayoutCreateInfo.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;//允许描述符集可以为空
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	PipelineBuilder builder(device);
	builder.addShaderStage(fullScreenShaderStage);
	builder.addShaderStage(vkUtils::vkEngine->loadShader(vkUtils::vkEngine->getShadersPath() + "PostProcess_DOF.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
	VkPipelineRenderingCreateInfo renderInfo = vks::initializers::pipelineRenderingCreateInfo(1, &vkUtils::vkEngine->offscreenFormat);

	builder.buildPipeline(renderInfo, vkUtils::vkEngine->pipelineCache, pipelineLayout, pipeline);
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline, "PostProcess_DOF pipeline");
}