#pragma once
#include <glm/glm.hpp>
#include "PostProcessBase.h"

class PostProcessToneMapping : public PostProcessBase
{
public:
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, maxConcurrentFrames> descriptorSet;
public:
	void prepare();
	void destroy();
	//对sampleImage进行色调映射处理，并将结果写入writeImage
	void excute(VkCommandBuffer cmdBuffer, VkDescriptorImageInfo sampleImageInfo, VkImageView writeImage);
private:
	void BindDescriptorSets(VkCommandBuffer cmdBuffer, VkDescriptorImageInfo sampleImageInfo);
	void prepareDescriptorSet();
	void preparePipeline();

};