#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace VulkanDebugUtils
{
	void InitDebugUtils(VkInstance inst, VkDevice dev);
	void CmdBeginLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color);
	void CmdInsertLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color);
	void CmdEndLabel(VkCommandBuffer command_buffer);
	void QueueBeginLabel(VkQueue queue, const char* label_name, std::vector<float> color);
	void QueueInsertLabel(VkQueue queue, const char* label_name, std::vector<float> color);
	void QueueEndLabel(VkQueue queue);
	void SetObjectDebugName(VkObjectType object_type, uint64_t object_handle, std::string object_name);
}