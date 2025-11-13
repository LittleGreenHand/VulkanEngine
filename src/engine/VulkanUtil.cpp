#include "VulkanUtil.h"
VulkanEngine* vkUtils::vkEngine = nullptr;
bool vkUtils::init = false;
bool vkUtils::debugUtilsSupported = false;
PFN_vkCreateDebugUtilsMessengerEXT vkUtils::vkCreateDebugUtilsMessengerEXT{ nullptr };
PFN_vkDestroyDebugUtilsMessengerEXT vkUtils::vkDestroyDebugUtilsMessengerEXT{ nullptr };
PFN_vkCmdBeginDebugUtilsLabelEXT vkUtils::vkCmdBeginDebugUtilsLabelEXT{ nullptr };
PFN_vkCmdInsertDebugUtilsLabelEXT vkUtils::vkCmdInsertDebugUtilsLabelEXT{ nullptr };
PFN_vkCmdEndDebugUtilsLabelEXT vkUtils::vkCmdEndDebugUtilsLabelEXT{ nullptr };
PFN_vkQueueBeginDebugUtilsLabelEXT vkUtils::vkQueueBeginDebugUtilsLabelEXT{ nullptr };
PFN_vkQueueInsertDebugUtilsLabelEXT vkUtils::vkQueueInsertDebugUtilsLabelEXT{ nullptr };
PFN_vkQueueEndDebugUtilsLabelEXT vkUtils::vkQueueEndDebugUtilsLabelEXT{ nullptr };
PFN_vkSetDebugUtilsObjectNameEXT vkUtils::vkSetDebugUtilsObjectNameEXT{ nullptr };

void vkUtils::Init(VulkanEngine* Engine)
{
	vkEngine = Engine;
	InitDebugUtils();
	init = true;
}

void vkUtils::CleanUp()
{
	vkEngine = nullptr;
	init = false;
}

// Checks if debug utils are supported (usually only when a graphics debugger is active) and does the setup necessary to use this debug utils
void vkUtils::InitDebugUtils()
{
	// Check if the debug utils extension is present (which is the case if run from a graphics debugger)
	bool extensionPresent = false;
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	for (auto& extension : extensions) {
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
			extensionPresent = true;
			break;
		}
	}

	if (extensionPresent) {
		auto instance = vkEngine->instance;
		// As with an other extension, function pointers need to be manually loaded
		vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		vkCmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
		vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
		vkQueueBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
		vkQueueInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
		vkQueueEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
		vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));

		// Set flag if at least one function pointer is present
		debugUtilsSupported = (vkCreateDebugUtilsMessengerEXT != VK_NULL_HANDLE);
	}
	else {
		std::cout << "Warning: " << VK_EXT_DEBUG_UTILS_EXTENSION_NAME << " not present, debug utils are disabled.";
		std::cout << "Try running the sample from inside a Vulkan graphics debugger (e.g. RenderDoc)" << std::endl;
	}
}

void vkUtils::cmdBeginLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color)
{
	if (!debugUtilsSupported) {
		return;
	}
	VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = label_name;
	memcpy(label.color, color.data(), sizeof(float) * 4);
	vkCmdBeginDebugUtilsLabelEXT(command_buffer, &label);
}

void vkUtils::cmdInsertLabel(VkCommandBuffer command_buffer, const char* label_name, std::vector<float> color)
{
	if (!debugUtilsSupported) {
		return;
	}
	VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = label_name;
	memcpy(label.color, color.data(), sizeof(float) * 4);
	vkCmdInsertDebugUtilsLabelEXT(command_buffer, &label);
}

void vkUtils::cmdEndLabel(VkCommandBuffer command_buffer)
{
	if (!debugUtilsSupported) {
		return;
	}
	vkCmdEndDebugUtilsLabelEXT(command_buffer);
}

// Functions for putting labels into a queue
// Labels consist of a name and an optional color
// How or if these are diplayed depends on the debugger used (RenderDoc e.g. displays both)

void vkUtils::queueBeginLabel(VkQueue queue, const char* label_name, std::vector<float> color)
{
	if (!debugUtilsSupported) {
		return;
	}
	VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = label_name;
	memcpy(label.color, color.data(), sizeof(float) * 4);
	vkQueueBeginDebugUtilsLabelEXT(queue, &label);
}

void vkUtils::queueInsertLabel(VkQueue queue, const char* label_name, std::vector<float> color)
{
	if (!debugUtilsSupported) {
		return;
	}
	VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = label_name;
	memcpy(label.color, color.data(), sizeof(float) * 4);
	vkQueueInsertDebugUtilsLabelEXT(queue, &label);
}

void vkUtils::queueEndLabel(VkQueue queue)
{
	if (!debugUtilsSupported) {
		return;
	}
	vkQueueEndDebugUtilsLabelEXT(queue);
}

// Function for naming Vulkan objects
// In Vulkan, all objects (that can be named) are opaque unsigned 64 bit handles, and can be cased to uint64_t
void vkUtils::setObjectDebugName(VkObjectType object_type, uint64_t object_handle, std::string object_name)
{
	if (!debugUtilsSupported) {
		return;
	}
	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = object_type;
	name_info.objectHandle = object_handle;
	name_info.pObjectName = object_name.c_str();
	vkSetDebugUtilsObjectNameEXT(vkEngine->device, &name_info);
}

void vkUtils::InitModelsSourceDebugName(std::map<GLTFModels, vkglTF::Model>& models)
{
	for (auto& [key, model] : models)
	{
		for (int i = 0; i < model.materials.size(); i++)
		{
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)model.materials[i].descriptorSet, model.modelName + "_Material_" + std::to_string(i) + "DescriptorSet");
		}
		for(int i = 0; i<model.linearNodes.size(); i++)
		{
			if (model.linearNodes[i]->mesh)
			{
				vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)model.linearNodes[i]->mesh->uniformBuffer.descriptorSet, model.linearNodes[i]->name + "_MeshDescriptorSet");
			}
		}
	}
}

void vkUtils::generateBRDFLUT(vks::Texture2D& lutBrdf)
{
	if (!init)
		return;
	auto tStart = std::chrono::high_resolution_clock::now();

	const VkFormat format = VK_FORMAT_R16G16_SFLOAT;	// R16G16 is supported pretty much everywhere
	const int32_t dim = 512;

	// Image
	VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = format;
	imageCI.extent.width = dim;
	imageCI.extent.height = dim;
	imageCI.extent.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VK_CHECK_RESULT(vkCreateImage(vkEngine->device, &imageCI, nullptr, &lutBrdf.image));
	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vkEngine->device, lutBrdf.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vkEngine->vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(vkEngine->device, &memAlloc, nullptr, &lutBrdf.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(vkEngine->device, lutBrdf.image, lutBrdf.deviceMemory, 0));
	// Image view
	VkImageViewCreateInfo viewCI = vks::initializers::imageViewCreateInfo();
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCI.format = format;
	viewCI.subresourceRange = {};
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.levelCount = 1;
	viewCI.subresourceRange.layerCount = 1;
	viewCI.image = lutBrdf.image;
	VK_CHECK_RESULT(vkCreateImageView(vkEngine->device, &viewCI, nullptr, &lutBrdf.view));
	// Sampler
	VkSamplerCreateInfo samplerCI = vks::initializers::samplerCreateInfo();
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 1.0f;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(vkEngine->device, &samplerCI, nullptr, &lutBrdf.sampler));

	lutBrdf.descriptor.imageView = lutBrdf.view;
	lutBrdf.descriptor.sampler = lutBrdf.sampler;
	lutBrdf.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	lutBrdf.device = vkEngine->vulkanDevice;

	// FB, Att, RP, Pipe, etc.
	VkAttachmentDescription attDesc = {};
	// Color attachment
	attDesc.format = format;
	attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create the actual renderpass
	VkRenderPassCreateInfo renderPassCI = vks::initializers::renderPassCreateInfo();
	renderPassCI.attachmentCount = 1;
	renderPassCI.pAttachments = &attDesc;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpassDescription;
	renderPassCI.dependencyCount = 2;
	renderPassCI.pDependencies = dependencies.data();

	VkRenderPass renderpass;
	VK_CHECK_RESULT(vkCreateRenderPass(vkEngine->device, &renderPassCI, nullptr, &renderpass));

	VkFramebufferCreateInfo framebufferCI = vks::initializers::framebufferCreateInfo();
	framebufferCI.renderPass = renderpass;
	framebufferCI.attachmentCount = 1;
	framebufferCI.pAttachments = &lutBrdf.view;
	framebufferCI.width = dim;
	framebufferCI.height = dim;
	framebufferCI.layers = 1;

	VkFramebuffer framebuffer;
	VK_CHECK_RESULT(vkCreateFramebuffer(vkEngine->device, &framebufferCI, nullptr, &framebuffer));

	// Descriptors
	VkDescriptorSetLayout descriptorsetlayout;
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
	VkDescriptorSetLayoutCreateInfo descriptorsetlayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vkEngine->device, &descriptorsetlayoutCI, nullptr, &descriptorsetlayout));

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = { vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) };
	VkDescriptorPoolCreateInfo descriptorPoolCI = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VkDescriptorPool descriptorpool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(vkEngine->device, &descriptorPoolCI, nullptr, &descriptorpool));

	// Descriptor sets
	VkDescriptorSet descriptorset;
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorpool, &descriptorsetlayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(vkEngine->device, &allocInfo, &descriptorset));

	// Pipeline layout
	VkPipelineLayout pipelinelayout;
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorsetlayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(vkEngine->device, &pipelineLayoutCI, nullptr, &pipelinelayout));

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelinelayout, renderpass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = 2;
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = &emptyInputState;

	// Look-up-table (from BRDF) pipeline
	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(vkEngine->device, vkEngine->pipelineCache, 1, &pipelineCI, nullptr, &pipeline));

	// Render
	VkClearValue clearValues[1]{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderpass;
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = framebuffer;

	VkCommandBuffer cmdBuf = vkEngine->vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = vks::initializers::viewport((float)dim, (float)dim, 0.0f, 1.0f);
	VkRect2D scissor = vks::initializers::rect2D(dim, dim, 0, 0);
	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdDraw(cmdBuf, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmdBuf);
	vkEngine->vulkanDevice->flushCommandBuffer(cmdBuf, vkEngine->queue);

	vkQueueWaitIdle(vkEngine->queue);

	vkDestroyPipeline(vkEngine->device, pipeline, nullptr);
	vkDestroyPipelineLayout(vkEngine->device, pipelinelayout, nullptr);
	vkDestroyRenderPass(vkEngine->device, renderpass, nullptr);
	vkDestroyFramebuffer(vkEngine->device, framebuffer, nullptr);
	vkDestroyDescriptorSetLayout(vkEngine->device, descriptorsetlayout, nullptr);
	vkDestroyDescriptorPool(vkEngine->device, descriptorpool, nullptr);

	setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)lutBrdf.image, "LutBRDF");
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating BRDF LUT took " << tDiff << " ms" << std::endl;
}

void vkUtils::generateIrradianceCube(vks::TextureCubeMap& irradianceCube, vks::TextureCubeMap& environmentCube)
{
	if (!init)
		return;
	auto tStart = std::chrono::high_resolution_clock::now();

	const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
	const int32_t dim = 64;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

	// Pre-filtered cube map
	// Image
	VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = format;
	imageCI.extent.width = dim;
	imageCI.extent.height = dim;
	imageCI.extent.depth = 1;
	imageCI.mipLevels = numMips;
	imageCI.arrayLayers = 6;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VK_CHECK_RESULT(vkCreateImage(vkEngine->device, &imageCI, nullptr, &irradianceCube.image));
	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vkEngine->device, irradianceCube.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vkEngine->vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(vkEngine->device, &memAlloc, nullptr, &irradianceCube.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(vkEngine->device, irradianceCube.image, irradianceCube.deviceMemory, 0));
	// Image view
	VkImageViewCreateInfo viewCI = vks::initializers::imageViewCreateInfo();
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCI.format = format;
	viewCI.subresourceRange = {};
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.levelCount = numMips;
	viewCI.subresourceRange.layerCount = 6;
	viewCI.image = irradianceCube.image;
	VK_CHECK_RESULT(vkCreateImageView(vkEngine->device, &viewCI, nullptr, &irradianceCube.view));
	// Sampler
	VkSamplerCreateInfo samplerCI = vks::initializers::samplerCreateInfo();
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(numMips);
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(vkEngine->device, &samplerCI, nullptr, &irradianceCube.sampler));

	irradianceCube.descriptor.imageView = irradianceCube.view;
	irradianceCube.descriptor.sampler = irradianceCube.sampler;
	irradianceCube.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	irradianceCube.device = vkEngine->vulkanDevice;

	// FB, Att, RP, Pipe, etc.
	VkAttachmentDescription attDesc = {};
	// Color attachment
	attDesc.format = format;
	attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Renderpass
	VkRenderPassCreateInfo renderPassCI = vks::initializers::renderPassCreateInfo();
	renderPassCI.attachmentCount = 1;
	renderPassCI.pAttachments = &attDesc;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpassDescription;
	renderPassCI.dependencyCount = 2;
	renderPassCI.pDependencies = dependencies.data();
	VkRenderPass renderpass;
	VK_CHECK_RESULT(vkCreateRenderPass(vkEngine->device, &renderPassCI, nullptr, &renderpass));

	struct {
		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;
		VkFramebuffer framebuffer;
	} offscreen{};

	// Offfscreen framebuffer
	{
		// Color attachment
		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent.width = dim;
		imageCreateInfo.extent.height = dim;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateImage(vkEngine->device, &imageCreateInfo, nullptr, &offscreen.image));

		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vkEngine->device, offscreen.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = vkEngine->vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(vkEngine->device, &memAlloc, nullptr, &offscreen.memory));
		VK_CHECK_RESULT(vkBindImageMemory(vkEngine->device, offscreen.image, offscreen.memory, 0));

		VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = format;
		colorImageView.flags = 0;
		colorImageView.subresourceRange = {};
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;
		colorImageView.image = offscreen.image;
		VK_CHECK_RESULT(vkCreateImageView(vkEngine->device, &colorImageView, nullptr, &offscreen.view));

		VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
		fbufCreateInfo.renderPass = renderpass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = &offscreen.view;
		fbufCreateInfo.width = dim;
		fbufCreateInfo.height = dim;
		fbufCreateInfo.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(vkEngine->device, &fbufCreateInfo, nullptr, &offscreen.framebuffer));

		VkCommandBuffer layoutCmd = vkEngine->vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vks::tools::setImageLayout(
			layoutCmd,
			offscreen.image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		vkEngine->vulkanDevice->flushCommandBuffer(layoutCmd, vkEngine->queue, true);
	}

	// Descriptors
	VkDescriptorSetLayout descriptorsetlayout;
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};
	VkDescriptorSetLayoutCreateInfo descriptorsetlayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vkEngine->device, &descriptorsetlayoutCI, nullptr, &descriptorsetlayout));

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = { vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) };
	VkDescriptorPoolCreateInfo descriptorPoolCI = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VkDescriptorPool descriptorpool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(vkEngine->device, &descriptorPoolCI, nullptr, &descriptorpool));

	// Descriptor sets
	VkDescriptorSet descriptorset;
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorpool, &descriptorsetlayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(vkEngine->device, &allocInfo, &descriptorset));
	VkWriteDescriptorSet writeDescriptorSet = vks::initializers::writeDescriptorSet(descriptorset, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &environmentCube.descriptor);
	vkUpdateDescriptorSets(vkEngine->device, 1, &writeDescriptorSet, 0, nullptr);

	// Pipeline layout
	struct PushBlock {
		glm::mat4 mvp;
		// Sampling deltas
		float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
		float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
	} pushBlock;

	VkPipelineLayout pipelinelayout;
	std::vector<VkPushConstantRange> pushConstantRanges = {
		vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushBlock), 0),
	};
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorsetlayout, 1);
	pipelineLayoutCI.pushConstantRangeCount = 1;
	pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
	VK_CHECK_RESULT(vkCreatePipelineLayout(vkEngine->device, &pipelineLayoutCI, nullptr, &pipelinelayout));

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelinelayout, renderpass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = 2;
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.renderPass = renderpass;
	pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV });

	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(vkEngine->device, vkEngine->pipelineCache, 1, &pipelineCI, nullptr, &pipeline));

	// Render

	VkClearValue clearValues[1]{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	// Reuse render pass from example pass
	renderPassBeginInfo.renderPass = renderpass;
	renderPassBeginInfo.framebuffer = offscreen.framebuffer;
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	VkCommandBuffer cmdBuf = vkEngine->vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkViewport viewport = vks::initializers::viewport((float)dim, (float)dim, 0.0f, 1.0f);
	VkRect2D scissor = vks::initializers::rect2D(dim, dim, 0, 0);

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.layerCount = 6;

	// Change image layout for all cubemap faces to transfer destination
	vks::tools::setImageLayout(
		cmdBuf,
		irradianceCube.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	for (uint32_t m = 0; m < numMips; m++) {
		for (uint32_t f = 0; f < 6; f++) {
			viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
			viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
			vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

			// Render scene from cube face's point of view
			vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update shader push constant block
			pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

			vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

			vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

			vkEngine->skybox.draw(cmdBuf);

			vkCmdEndRenderPass(cmdBuf);

			vks::tools::setImageLayout(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Copy region for transfer from framebuffer to cube face
			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				irradianceCube.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);

			// Transform framebuffer color attachment back
			vks::tools::setImageLayout(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	vks::tools::setImageLayout(
		cmdBuf,
		irradianceCube.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subresourceRange);

	vkEngine->vulkanDevice->flushCommandBuffer(cmdBuf, vkEngine->queue);

	vkDestroyRenderPass(vkEngine->device, renderpass, nullptr);
	vkDestroyFramebuffer(vkEngine->device, offscreen.framebuffer, nullptr);
	vkFreeMemory(vkEngine->device, offscreen.memory, nullptr);
	vkDestroyImageView(vkEngine->device, offscreen.view, nullptr);
	vkDestroyImage(vkEngine->device, offscreen.image, nullptr);
	vkDestroyDescriptorPool(vkEngine->device, descriptorpool, nullptr);
	vkDestroyDescriptorSetLayout(vkEngine->device, descriptorsetlayout, nullptr);
	vkDestroyPipeline(vkEngine->device, pipeline, nullptr);
	vkDestroyPipelineLayout(vkEngine->device, pipelinelayout, nullptr);

	setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)irradianceCube.image, "irradianceCube");
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating irradiance cube with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;
}

void vkUtils::generatePrefilteredCube(vks::TextureCubeMap& prefilteredCube, vks::TextureCubeMap& environmentCube)
{
	if (!init)
		return;
	auto tStart = std::chrono::high_resolution_clock::now();

	const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
	const int32_t dim = 512;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

	// Pre-filtered cube map
	// Image
	VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = format;
	imageCI.extent.width = dim;
	imageCI.extent.height = dim;
	imageCI.extent.depth = 1;
	imageCI.mipLevels = numMips;
	imageCI.arrayLayers = 6;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VK_CHECK_RESULT(vkCreateImage(vkEngine->device, &imageCI, nullptr, &prefilteredCube.image));
	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vkEngine->device, prefilteredCube.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vkEngine->vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(vkEngine->device, &memAlloc, nullptr, &prefilteredCube.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(vkEngine->device, prefilteredCube.image, prefilteredCube.deviceMemory, 0));
	// Image view
	VkImageViewCreateInfo viewCI = vks::initializers::imageViewCreateInfo();
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCI.format = format;
	viewCI.subresourceRange = {};
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.levelCount = numMips;
	viewCI.subresourceRange.layerCount = 6;
	viewCI.image = prefilteredCube.image;
	VK_CHECK_RESULT(vkCreateImageView(vkEngine->device, &viewCI, nullptr, &prefilteredCube.view));
	// Sampler
	VkSamplerCreateInfo samplerCI = vks::initializers::samplerCreateInfo();
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(numMips);
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(vkEngine->device, &samplerCI, nullptr, &prefilteredCube.sampler));

	prefilteredCube.descriptor.imageView = prefilteredCube.view;
	prefilteredCube.descriptor.sampler = prefilteredCube.sampler;
	prefilteredCube.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	prefilteredCube.device = vkEngine->vulkanDevice;

	// FB, Att, RP, Pipe, etc.
	VkAttachmentDescription attDesc = {};
	// Color attachment
	attDesc.format = format;
	attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Renderpass
	VkRenderPassCreateInfo renderPassCI = vks::initializers::renderPassCreateInfo();
	renderPassCI.attachmentCount = 1;
	renderPassCI.pAttachments = &attDesc;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpassDescription;
	renderPassCI.dependencyCount = 2;
	renderPassCI.pDependencies = dependencies.data();
	VkRenderPass renderpass;
	VK_CHECK_RESULT(vkCreateRenderPass(vkEngine->device, &renderPassCI, nullptr, &renderpass));

	struct {
		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;
		VkFramebuffer framebuffer;
	} offscreen{};

	// Offfscreen framebuffer
	{
		// Color attachment
		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent.width = dim;
		imageCreateInfo.extent.height = dim;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateImage(vkEngine->device, &imageCreateInfo, nullptr, &offscreen.image));

		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vkEngine->device, offscreen.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = vkEngine->vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(vkEngine->device, &memAlloc, nullptr, &offscreen.memory));
		VK_CHECK_RESULT(vkBindImageMemory(vkEngine->device, offscreen.image, offscreen.memory, 0));

		VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = format;
		colorImageView.flags = 0;
		colorImageView.subresourceRange = {};
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;
		colorImageView.image = offscreen.image;
		VK_CHECK_RESULT(vkCreateImageView(vkEngine->device, &colorImageView, nullptr, &offscreen.view));

		VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
		fbufCreateInfo.renderPass = renderpass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = &offscreen.view;
		fbufCreateInfo.width = dim;
		fbufCreateInfo.height = dim;
		fbufCreateInfo.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(vkEngine->device, &fbufCreateInfo, nullptr, &offscreen.framebuffer));

		VkCommandBuffer layoutCmd = vkEngine->vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vks::tools::setImageLayout(
			layoutCmd,
			offscreen.image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		vkEngine->vulkanDevice->flushCommandBuffer(layoutCmd, vkEngine->queue, true);
	}

	// Descriptors
	VkDescriptorSetLayout descriptorsetlayout;
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};
	VkDescriptorSetLayoutCreateInfo descriptorsetlayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vkEngine->device, &descriptorsetlayoutCI, nullptr, &descriptorsetlayout));

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = { vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) };
	VkDescriptorPoolCreateInfo descriptorPoolCI = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VkDescriptorPool descriptorpool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(vkEngine->device, &descriptorPoolCI, nullptr, &descriptorpool));

	// Descriptor sets
	VkDescriptorSet descriptorset;
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorpool, &descriptorsetlayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(vkEngine->device, &allocInfo, &descriptorset));
	VkWriteDescriptorSet writeDescriptorSet = vks::initializers::writeDescriptorSet(descriptorset, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &environmentCube.descriptor);
	vkUpdateDescriptorSets(vkEngine->device, 1, &writeDescriptorSet, 0, nullptr);

	// Pipeline layout
	struct PushBlock {
		glm::mat4 mvp;
		float roughness;
		uint32_t numSamples = 32u;
	}pushBlock{};

	VkPipelineLayout pipelinelayout;
	std::vector<VkPushConstantRange> pushConstantRanges = {
		vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushBlock), 0),
	};
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorsetlayout, 1);
	pipelineLayoutCI.pushConstantRangeCount = 1;
	pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
	VK_CHECK_RESULT(vkCreatePipelineLayout(vkEngine->device, &pipelineLayoutCI, nullptr, &pipelinelayout));

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelinelayout, renderpass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = 2;
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.renderPass = renderpass;
	pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV });

	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "PBR_prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(vkEngine->device, vkEngine->pipelineCache, 1, &pipelineCI, nullptr, &pipeline));

	// Render

	VkClearValue clearValues[1]{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	// Reuse render pass from example pass
	renderPassBeginInfo.renderPass = renderpass;
	renderPassBeginInfo.framebuffer = offscreen.framebuffer;
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	VkCommandBuffer cmdBuf = vkEngine->vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkViewport viewport = vks::initializers::viewport((float)dim, (float)dim, 0.0f, 1.0f);
	VkRect2D scissor = vks::initializers::rect2D(dim, dim, 0, 0);

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.layerCount = 6;

	// Change image layout for all cubemap faces to transfer destination
	vks::tools::setImageLayout(
		cmdBuf,
		prefilteredCube.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	for (uint32_t m = 0; m < numMips; m++) {
		pushBlock.roughness = (float)m / (float)(numMips - 1);
		for (uint32_t f = 0; f < 6; f++) {
			viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
			viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
			vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

			// Render scene from cube face's point of view
			vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update shader push constant block
			pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

			vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

			vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

			vkEngine->skybox.draw(cmdBuf);

			vkCmdEndRenderPass(cmdBuf);

			vks::tools::setImageLayout(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Copy region for transfer from framebuffer to cube face
			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				prefilteredCube.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);

			// Transform framebuffer color attachment back
			vks::tools::setImageLayout(
				cmdBuf,
				offscreen.image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	vks::tools::setImageLayout(
		cmdBuf,
		prefilteredCube.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subresourceRange);

	vkEngine->vulkanDevice->flushCommandBuffer(cmdBuf, vkEngine->queue);

	vkDestroyRenderPass(vkEngine->device, renderpass, nullptr);
	vkDestroyFramebuffer(vkEngine->device, offscreen.framebuffer, nullptr);
	vkFreeMemory(vkEngine->device, offscreen.memory, nullptr);
	vkDestroyImageView(vkEngine->device, offscreen.view, nullptr);
	vkDestroyImage(vkEngine->device, offscreen.image, nullptr);
	vkDestroyDescriptorPool(vkEngine->device, descriptorpool, nullptr);
	vkDestroyDescriptorSetLayout(vkEngine->device, descriptorsetlayout, nullptr);
	vkDestroyPipeline(vkEngine->device, pipeline, nullptr);
	vkDestroyPipelineLayout(vkEngine->device, pipelinelayout, nullptr);

	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)prefilteredCube.image, "prefilteredCube");
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating pre-filtered enivornment cube with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;
}

glm::quat vkUtils::eularToQuaternion(const glm::vec3& euler)
{
	// 将角度转换为弧度
	glm::vec3 radians = glm::radians(euler);

	// 计算各个轴的旋转四元数
	glm::quat pitch = glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat yaw = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat roll = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));

	// 组合旋转：注意旋转顺序是xyz
	// 因为GLM使用的是右乘，旋转顺序是从右到左应用
	return roll * yaw * pitch;
}

glm::vec3 vkUtils::generateUpVector(const glm::vec3& forward) {
	// 找到与forward不共线的垂直向量
	glm::vec3 ref = (std::abs(forward.x) > std::abs(forward.z))
		? glm::vec3(forward.z, 0, -forward.x)  // 与X-Z平面垂直
		: glm::vec3(0, -forward.z, forward.y); // 与Y-Z平面垂直
	return glm::normalize(ref);
}

// 在类中添加静态变量跟踪选中的节点
vkglTF::Node* vkUtils::selectedNode = nullptr;

void vkUtils::DrawNodeTree(vkglTF::Node* node, int& nodeId)
{
	if (!node) return;

	// 保存当前节点ID
	int originalNodeId = nodeId;
	std::string baseId = std::to_string(originalNodeId);

	// 处理节点名称
	std::string displayName = node->name.empty() ? "Unnamed Node" : node->name;
	displayName += "##node_" + baseId;

	// 节点是否可展开
	bool isExpandable = !node->children.empty();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

	// 选中节点高亮显示
	if (selectedNode == node)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (!isExpandable)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	// 绘制可见性复选框
	ImGui::Checkbox(("##vis_" + baseId).c_str(), &node->visible);
	ImGui::SameLine(0, 4);

	// 绘制节点树
	bool isOpen = ImGui::TreeNodeEx(displayName.c_str(), flags);
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) 
	{
		if(selectedNode == node)
			selectedNode = nullptr; // 再次单击已选中节点，取消选择
		else
			selectedNode = node; // 单击节点文本区域，选中当前节点
	}

	// 递增节点ID
	nodeId++;

	// 递归绘制子节点
	if (isOpen && isExpandable)
	{
		for (vkglTF::Node* child : node->children)
		{
			DrawNodeTree(child, nodeId);
		}
		ImGui::TreePop();
	}
	else if (!isExpandable)
	{
		ImGui::TreePop();
	}
}

// 新增函数：绘制属性编辑面板
void vkUtils::DrawNodePropertiesPanel()
{
	if (!selectedNode)
	{
		ImGui::Text("Select a node to edit properties");
		return;
	}

	// 属性面板标题
	ImGui::Text("Node: %s",
		selectedNode->name.empty() ? "Unnamed Node" : selectedNode->name.c_str());
	ImGui::Separator();

	// 节点名称编辑
	char nameBuffer[256];
	strncpy_s(nameBuffer, selectedNode->name.c_str(), sizeof(nameBuffer) - 1);
	nameBuffer[sizeof(nameBuffer) - 1] = '\0';
	if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
	{
		selectedNode->name = nameBuffer;
	}

	// 可见性控制
	ImGui::Checkbox("Visible", &selectedNode->visible);

	// 平移
	ImGui::InputFloat3("Translation", &selectedNode->translation.x);

	// 旋转 (欧拉角)
	glm::vec3 rotationEuler = glm::eulerAngles(selectedNode->rotation) * (180.0f / glm::pi<float>());
	if (ImGui::InputFloat3("Rotation (deg)", &rotationEuler.x))
	{
		selectedNode->rotation = glm::quat(glm::radians(rotationEuler));
	}

	// 缩放
	ImGui::InputFloat3("Scale", &selectedNode->scale.x);
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();

	// 材质属性（如果有网格）
	if (selectedNode->mesh)
	{
		ImGui::Text(" ");
		ImGui::Text(" ");
		ImGui::Text("Material Properties");
		ImGui::Separator();
		ImGui::Text("Mesh: %s", selectedNode->mesh->name.c_str());

		for (int i = 0; i < selectedNode->mesh->primitives.size(); ++i)
		{
			auto& primitive = selectedNode->mesh->primitives[i];
			auto& material = primitive->material;

			ImGui::PushID(i);
			ImGui::Text("Primitive %d Material", (int)i);
			ImGui::Separator();

			ImGui::ColorEdit4("Base Color Factor", &material.materialParameters.baseColorFactor.x);
			ImGui::SliderFloat("Metallic Factor", &material.materialParameters.metallicFactor, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness Factor", &material.materialParameters.roughnessFactor, 0.0f, 1.0f);
			ImGui::SliderFloat("Alpha Cutoff", &material.materialParameters.alphaCutoff, 0.0f, 1.0f);
			ImGui::SliderFloat("Anisotropic Factor", &material.materialParameters.anisotropicFactor, -1.0f, 1.0f);
			ImGui::InputFloat4("tangent", &material.materialParameters.tangent.x);

			const char* alphaModes[] = { "Opaque", "Mask", "Blend" };
			ImGui::Combo("Alpha Mode", (int*)&material.alphaMode, alphaModes, IM_ARRAYSIZE(alphaModes));
			// 定义纹理参数与对应显示文本的映射关系
			std::vector<std::pair<bool, const char*>> textureInfo = {
				{!material.materialParameters.baseColorTextureEmpty, "Base Color Texture: Present"},
				{!material.materialParameters.normalTextureEmpty, "Normal Texture: Present"},
				{!material.materialParameters.metallicRoughnessTextureEmpty, "Metallic-Roughness Texture: Present"},
				{!material.materialParameters.metallicTextureEmpty, "Metallic Texture: Present"},
				{!material.materialParameters.roughnessTextureEmpty, "Roughness Texture: Present"},
				{!material.materialParameters.occlusionTextureEmpty, "Occlusion Texture: Present"},
				{!material.materialParameters.emissiveTextureEmpty, "Emissive Texture: Present"},
				{!material.materialParameters.AOTextureEmpty, "AO Texture: Present"},
				{!material.materialParameters.diffuseTextureEmpty, "Diffuse Texture: Present"},
				{!material.materialParameters.specularGlossinessTextureEmpty, "Specular-Glossiness Texture: Present"}
			};

			// 循环显示存在的纹理信息
			for (const auto& [isPresent, text] : textureInfo) {
				if (isPresent) {
					ImGui::Text("%s", text);
				}
			}

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Text(" ");
			ImGui::PopID();
		}
	}
	selectedNode->update();
	// 取消选择按钮
	if (ImGui::Button("Deselect"))
	{
		selectedNode = nullptr;
	}
}

Dimensions vkUtils::GetSceneDimensions()
{
	Dimensions dimension;
	dimension.min = glm::vec3(FLT_MAX);
	dimension.max = glm::vec3(-FLT_MAX);
	for (auto& [key, model] : vkEngine->models)
	{
		model.getSceneDimensions();
		if (dimension.min.x > model.dimensions.min.x) { dimension.min.x = model.dimensions.min.x; }
		if (dimension.min.y > model.dimensions.min.y) { dimension.min.y = model.dimensions.min.y; }
		if (dimension.min.z > model.dimensions.min.z) { dimension.min.z = model.dimensions.min.z; }
		if (dimension.max.x < model.dimensions.max.x) { dimension.max.x = model.dimensions.max.x; }
		if (dimension.max.y < model.dimensions.max.y) { dimension.max.y = model.dimensions.max.y; }
		if (dimension.max.z < model.dimensions.max.z) { dimension.max.z = model.dimensions.max.z; }
	}
	dimension.size = dimension.max - dimension.min;
	dimension.center = (dimension.min + dimension.max) / 2.0f;
	dimension.radius = glm::distance(dimension.min, dimension.max) / 2.0f;
	return dimension;
}

void vkUtils::transitionImageLayout(VkCommandBuffer cmd, vks::Texture& texture, VkImageLayout newLayout, VkImageAspectFlags aspectMask, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
{
	if(texture.imageLayout == newLayout)
		return;
	//VkImageMemoryBarrier2 是Vulkan1.3引入的扩展（VK_KHR_synchronization2）中定义的同步原语，用于更精细地控制图像内存访问顺序和布局转换
	VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
	imageBarrier.pNext = nullptr;

	//VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT 是一个特殊的管线阶段标志，用于表示所有可能的管线阶段
	imageBarrier.srcStageMask = srcStageMask;//指定哪些管线阶段必须在屏障前完成
	imageBarrier.srcAccessMask = srcAccessMask;//确保内存写入在屏障前完成
	imageBarrier.dstStageMask = dstStageMask;//指定屏障后允许继续执行的管线阶段
	imageBarrier.dstAccessMask = dstAccessMask;//指定屏障后允许对资源进行的具体访问操作（如读、写），确保数据一致性

	imageBarrier.oldLayout = texture.imageLayout;
	imageBarrier.newLayout = newLayout;

	//指定图像的哪些子资源（Mip 层级、数组层）受屏障影响，要尽量缩小subresourceRange的范围（避免全局屏障）
	imageBarrier.subresourceRange = vks::initializers::ImageSubresourceRange(aspectMask);

	imageBarrier.image = texture.image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	//在命令缓冲区中插入显式屏障，或者说在命令缓冲区中记录屏障指令，实际屏障的执行发生在GPU运行该命令缓冲区时（提交到队列后）
	//vkCmdPipelineBarrier2强制GPU在屏障前完成srcStageMask指定的所有操作，并阻塞dstStageMask之前的操作，直到屏障完成
	//过度使用屏障会破坏 GPU 并行性，应尽量合并多个屏障到一次调用
	vkCmdPipelineBarrier2(cmd, &depInfo);
	texture.imageLayout = newLayout;
	texture.updateDescriptor();
}

void vkUtils::copyImageToImage(VkCommandBuffer cmd, vks::Texture& srcTexture, vks::Texture& dstTexture) {
	assert(srcTexture.device != nullptr && dstTexture.device != nullptr && "Texture 的 device 指针不可为空");
	assert(srcTexture.image != VK_NULL_HANDLE && dstTexture.image != VK_NULL_HANDLE && "源/目标图像句柄不可为空");
	assert(srcTexture.view != VK_NULL_HANDLE && dstTexture.view != VK_NULL_HANDLE && "源/目标图像视图不可为空");
	assert(srcTexture.width == dstTexture.width && srcTexture.height == dstTexture.height && "源/目标图像宽高必须一致");
	assert(srcTexture.mipLevels == dstTexture.mipLevels && "源/目标图像 Mip 层级必须一致");
	assert(srcTexture.layerCount == dstTexture.layerCount && "源/目标图像图层数量必须一致");
	assert(srcTexture.format == dstTexture.format && "源/目标图像格式必须一致");

	// 确定图像的 AspectMask（颜色/深度/模板）
	VkImageAspectFlags aspectMask = 0;
	switch (srcTexture.format) {
		// 纯深度格式
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
		// 深度+模板格式（需确保 ImageView 创建时仅包含一个 Aspect，遵循 VUID-01976）
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		// 注意：此处默认使用深度 Aspect，若你需要复制模板，需改为 VK_IMAGE_ASPECT_STENCIL_BIT
		// （需确保 ImageView 创建时的 Aspect 与此处一致）
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
		// 颜色格式（默认所有非深度/模板格式为颜色）
	default:
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	}

	// 保存原始布局（用于复制后恢复）
	VkImageLayout srcOriginalLayout = srcTexture.imageLayout;
	VkImageLayout dstOriginalLayout = dstTexture.imageLayout;

	// 转换源/目标图像到复制所需布局
	transitionImageLayout(cmd, srcTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, aspectMask);
	transitionImageLayout(cmd, dstTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aspectMask);

	// 配置复制区域（支持 Mip 层级和图层）
	std::vector<VkImageCopy> copyRegions;
	copyRegions.reserve(srcTexture.mipLevels * srcTexture.layerCount);

	for (uint32_t mip = 0; mip < srcTexture.mipLevels; ++mip) {
		for (uint32_t layer = 0; layer < srcTexture.layerCount; ++layer) {
			VkImageCopy region{};
			// 1. 源图像子资源配置
			region.srcSubresource.aspectMask = aspectMask;    // 与上面确定的 Aspect 一致
			region.srcSubresource.mipLevel = mip;             // 当前 Mip 层级
			region.srcSubresource.baseArrayLayer = layer;     // 当前图层
			region.srcSubresource.layerCount = 1;             // 每次复制 1 个图层
			region.srcOffset = { 0, 0, 0 };                    // 复制起始偏移（左上角）

			// 2. 目标图像子资源配置（与源完全对齐）
			region.dstSubresource.aspectMask = aspectMask;
			region.dstSubresource.mipLevel = mip;
			region.dstSubresource.baseArrayLayer = layer;
			region.dstSubresource.layerCount = 1;
			region.dstOffset = { 0, 0, 0 };

			// 3. 复制尺寸（Mip 层级缩放：原始尺寸 / 2^mip，确保不为 0）
			region.extent.width = std::max(1U, srcTexture.width >> mip);    // 宽度缩放
			region.extent.height = std::max(1U, srcTexture.height >> mip);  // 高度缩放
			region.extent.depth = 1;                                        // 你的类无 depth，默认 2D 图像

			copyRegions.push_back(region);
		}
	}

	// --------------------------
	// 步骤6：执行图像复制命令
	// --------------------------
	vkCmdCopyImage(
		cmd,
		srcTexture.image,                          // 源图像
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,      // 源布局（必须与转换后一致）
		dstTexture.image,                          // 目标图像
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,      // 目标布局（必须与转换后一致）
		static_cast<uint32_t>(copyRegions.size()), // 复制区域数量
		copyRegions.data()                         // 复制区域数组
	);
}