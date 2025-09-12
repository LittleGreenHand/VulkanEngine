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
	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
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

	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
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

			vkEngine->models.skybox.draw(cmdBuf);

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

	shaderStages[0] = vkEngine->loadShader(vkEngine->getShadersPath() + "filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vkEngine->loadShader(vkEngine->getShadersPath() + "prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
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

			vkEngine->models.skybox.draw(cmdBuf);

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
