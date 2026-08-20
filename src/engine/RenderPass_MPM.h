#pragma once
#include <vulkan/vulkan.h>
#include "RenderPassBase.h"
class RenderPassMPM : public RenderPassBase
{
public:
	RenderPassMPM() = default;
	~RenderPassMPM() override;
	void Render(VkCommandBuffer commandBuffer) override;

public:

};