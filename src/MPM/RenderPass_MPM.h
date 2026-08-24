#pragma once
#include <vulkan/vulkan.h>
#include "render/RenderPassBase.h"
class RenderPassMPM : public RenderPassBase
{
public:
	RenderPassMPM() = default;
	~RenderPassMPM() override;
	void Render(VkCommandBuffer commandBuffer) override;

public:

};