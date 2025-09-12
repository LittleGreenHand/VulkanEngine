#pragma once
#include <vulkan/vulkan.h>
#include "vulkanEngine.h"
#include "VulkanglTFModel.h"

class vkUtils
{
private:
	static bool init;
	static bool debugUtilsSupported;
	static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	static PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
	static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	static PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
	static PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;
	static PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
public:
	static VulkanEngine* vkEngine;

public:
	static void Init(VulkanEngine* Engine);
	static void CleanUp();

public:
	//调试相关
	static void InitDebugUtils();
	static void cmdBeginLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color);
	static void cmdInsertLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color);
	static void cmdEndLabel(VkCommandBuffer command_buffer);
	static void queueBeginLabel(VkQueue queue, const char* label_name, std::vector<float> color);
	static void queueInsertLabel(VkQueue queue, const char* label_name, std::vector<float> color);
	static void queueEndLabel(VkQueue queue);
	static void setObjectDebugName(VkObjectType object_type, uint64_t object_handle, std::string object_name);

public:
	//PBR生成相关

	static void generateBRDFLUT(vks::Texture2D& lutBrdf);
	static void generateIrradianceCube(vks::TextureCubeMap& irradianceCube, vks::TextureCubeMap& environmentCube);
	static void generatePrefilteredCube(vks::TextureCubeMap& prefilteredCube, vks::TextureCubeMap& environmentCube);

};