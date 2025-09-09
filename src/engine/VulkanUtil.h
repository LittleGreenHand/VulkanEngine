#pragma once
#include <vulkan/vulkan.h>
#include "vulkanEngine.h"
#include "VulkanglTFModel.h"

class vkUtils
{
private:
	static bool init;
public:
	static VulkanEngine* vkEngine;
public:
	static void Init(VulkanEngine* Engine);
	static void CleanUp();
	static void generateBRDFLUT(vks::Texture2D& lutBrdf);
	static void generateIrradianceCube(vks::TextureCubeMap& irradianceCube, vks::TextureCubeMap& environmentCube);
	static void generatePrefilteredCube(vks::TextureCubeMap& prefilteredCube, vks::TextureCubeMap& environmentCube);

	static void SetImageDebugName(VkImage image, const char* name);
};