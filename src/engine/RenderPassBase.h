#pragma once
#include <vulkan/vulkan.h>
class RenderPassBase
{
public:
	RenderPassBase() = default;
	virtual ~RenderPassBase() = default;
	virtual void Render(VkCommandBuffer commandBuffer) = 0;

	void SetRenderResolution(int w, int h) { renderWidth = w; renderHeight = h; }
	void SetBackBufferResolution(int w, int h) { backBufferWidth = w; backBufferHeight = h; }
public:
	int renderWidth{ 0 };
	int renderHeight{ 0 };
	int backBufferWidth{ 0 };
	int backBufferHeight{ 0 };
};