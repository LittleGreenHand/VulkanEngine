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

	//为glTF模型中的vulkan资源设置调试名称
	static void InitModelsSourceDebugName(std::map<GLTFModels, vkglTF::Model>& models);
public:
	//PBR生成相关
	static void generateBRDFLUT(vks::Texture2D& lutBrdf);
	static void generateIrradianceCube(vks::TextureCubeMap& irradianceCube, vks::TextureCubeMap& environmentCube);
	static void generatePrefilteredCube(vks::TextureCubeMap& prefilteredCube, vks::TextureCubeMap& environmentCube);

public:
	//数学相关
	static glm::quat eularToQuaternion(const glm::vec3& euler);
	static glm::vec3 generateUpVector(const glm::vec3& forward);

public:
	//UI相关
	static vkglTF::Node* selectedNode;
	static void DrawNodeTree(vkglTF::Node* node, int &nodeId);
	static void DrawNodePropertiesPanel();

public:
	//场景相关
	//计算并获取场景包围盒
	static Dimensions GetSceneDimensions();

public:
	//纹理相关
	//转换图像布局
	static void transitionImageLayout(VkCommandBuffer cmd, vks::Texture& texture, VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
	static void copyImageToImage(VkCommandBuffer cmd, vks::Texture& srcTexture, vks::Texture& dstTexture);
};