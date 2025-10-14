#include "VulkanEngine.h"
#include "VulkanUtil.h"
#include "types.hpp"
#include "PostProcessBase.h"
#include "PipelineBuilder.h"
#include "PostProcess_ToneMapping.h"
#include "PostProcess_DOF.h"

VulkanEngine::VulkanEngine() : VulkanEngineBase()
{
	title = "VulkanEngine";
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 8.0f;
	camera.setPerspective(60.0f, (float)width / (float)height, 0.01f, 256.0f);

	camera.rotationSpeed = 0.25f;
	camera.setRotation({ 0.0f, 0.0f, 0.0f });
	camera.setPosition({ 0.f, 0.f, 0.f });

	camera.focusDistance = 1.0f;
	camera.focusRange = 1.0f;
	camera.maxBlurRadius = 4.5f;
	camera.aperture = 0.56f;
}

VulkanEngine::~VulkanEngine()
{
	vkLight::destroyLightBuffer();
	if (postProcessManager)
	{
		postProcessManager->destroyALL();
		delete postProcessManager;
		postProcessManager = nullptr;
	}

	if (device) {
		textures.environmentCube.destroy();
		textures.irradianceCube.destroy();
		textures.prefilteredCube.destroy();
		textures.lutBrdf.destroy();
		textures.albedoMap.destroy();
		textures.normalMap.destroy();
		textures.aoMap.destroy();
		textures.metallicMap.destroy();
		textures.roughnessMap.destroy();
		vkglTF::destroyEmptyTexture();
		for (auto& buffer : globalParamBuffers) {
			buffer.globalParamBuffer.destroy();
		}
		for (auto& pipeline : pipelines)
		{
			pipeline.destroy(device);
		}
		for (auto& layout : setLayouts)
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
		}
	}
	PostProcessBase::cleanUp();
}

void VulkanEngine::prepare()
{
	VulkanEngineBase::prepare();
	vkUtils::Init(this);
	//初始化主渲染模块相关资源
	{
		loadAssets();
		prepareUniformBuffers();
		prepareDescriptors();
		preparePipelines();
	}
	//初始化灯光资源
	{
		pointLights.prepare(vulkanDevice, vulkanDevice->getSupportedDepthFormat(false));
		directLight.prepare(vulkanDevice, vulkanDevice->getSupportedDepthFormat(true));
	}
	preparePostProcess();
	prepared = true;
}

void VulkanEngine::getEnabledFeatures()
{
	if (deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	}	
	enabledFeatures.imageCubeArray = VK_TRUE;//启用立方体贴图数组

	Features11.shaderDrawParameters = VK_TRUE;//用于标识指示物理设备是否支持 "着色器绘制参数"，例如gl_DrawID、gl_InstanceID、gl_BaseVertex等

	features12.bufferDeviceAddress = true; //允许应用程序获取缓冲区的设备地址（GPU可直接访问的内存地址），并在着色器中通过此地址直接访问缓冲区数据
	features12.descriptorIndexing = true; //允许在着色器中动态索引描述符数组，着色器可以使用变量（而非编译期常量）作为索引访问描述符数组

	features13.dynamicRendering = true; //启用动态渲染，这样可以减少对渲染通道（Render Pass）和帧缓冲（Framebuffer）的依赖
	features13.synchronization2 = true; //用于标识物理设备是否支持Vulkan1.3引入的第二代同步机制，旨在简化同步逻辑、减少错误，并提供更精细的同步控制

	Features11.pNext = &features12;
	features12.pNext = &features13;
	features13.pNext = nullptr;
	deviceCreatepNextChain = &Features11;
}

void VulkanEngine::getEnabledExtensions()
{	
	enabledDeviceExtensions.push_back("VK_KHR_pipeline_library");
	enabledDeviceExtensions.push_back("VK_EXT_graphics_pipeline_library");
	libraryFeatures.graphicsPipelineLibrary = VK_TRUE;
	libraryFeatures.pNext = deviceCreatepNextChain;

	deviceCreatepNextChain = &libraryFeatures;
}

void VulkanEngine::loadAssets()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	//加载纹理
	{
		textures.environmentCube.loadFromFile(getAssetPath() + "textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);
		textures.albedoMap.loadFromFile(getAssetPath() + "models/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		textures.normalMap.loadFromFile(getAssetPath() + "models/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		textures.aoMap.loadFromFile(getAssetPath() + "models/cerberus/ao.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
		textures.metallicMap.loadFromFile(getAssetPath() + "models/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
		textures.roughnessMap.loadFromFile(getAssetPath() + "models/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	}

	//加载模型
	{
		uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;
		models[M_Cerberus].loadFromFile(getAssetPath() + "models/cerberus/cerberus.gltf", vulkanDevice, queue, glTFLoadingFlags);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setBaseColorTexture(&textures.albedoMap);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setNormalTexture(&textures.normalMap);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setAOTexture(&textures.aoMap);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setMetallicTexture(&textures.metallicMap);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setRoughnessTexture(&textures.roughnessMap);
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.updateDescriptorSet();
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.materialParameters.metallicFactor = 1;
		models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.materialParameters.roughnessFactor = 1;
		models[M_Cerberus].nodes[0]->clearTransform();
		models[M_Cerberus].nodes[0]->rotation = vkUtils::eularToQuaternion(glm::vec3(-90, 90, 0));
		models[M_Cerberus].nodes[0]->translation = (glm::vec3(0.2, -0.15, -0.5));
		models[M_Cerberus].nodes[0]->scale = (glm::vec3(0.2, 0.2, 0.2));
		models[M_Cerberus].nodes[0]->update(); 

		models[M_Cube].loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, queue, glTFLoadingFlags);
		models[M_Cube].nodes[0]->clearTransform();
		models[M_Cube].nodes[0]->scale = (glm::vec3(0.01, 0.01, 0.01));
		models[M_Cube].nodes[0]->translation = (glm::vec3(0, -0, -1));
		models[M_Cube].nodes[0]->visible = false;
		models[M_Cube].nodes[0]->update();

		models[M_Axis].loadFromFile(getAssetPath() + "models/axis.gltf", vulkanDevice, queue, glTFLoadingFlags);
		models[M_Axis].nodes[0]->clearTransform();
		models[M_Axis].nodes[0]->scale = (glm::vec3(0.1, 0.1, 0.1));
		models[M_Axis].nodes[0]->translation = (glm::vec3(0, -0.15, -1));
		models[M_Axis].nodes[0]->visible = false;
		models[M_Axis].nodes[0]->update();

		models[M_Sphere].loadFromFile(getAssetPath() + "models/sphere.gltf", vulkanDevice, queue, glTFLoadingFlags);
		models[M_Sphere].nodes[0]->clearTransform();
		models[M_Sphere].nodes[0]->scale = (glm::vec3(0.1, 0.1, 0.1));
		models[M_Sphere].nodes[0]->translation = (glm::vec3(0, -0, -1));
		models[M_Sphere].nodes[0]->visible = false;
		models[M_Sphere].nodes[0]->update();

		models[M_Sponza].loadFromFile(getAssetPath() + "models/sponza/sponza.gltf", vulkanDevice, queue, glTFLoadingFlags);
		models[M_Sponza].nodes[0]->clearTransform();
		models[M_Sponza].nodes[0]->rotation = vkUtils::eularToQuaternion(glm::vec3(0, 90, 0));
		models[M_Sponza].nodes[0]->translation = (glm::vec3(0, -1, 0));
		models[M_Sponza].nodes[0]->update();

		vkUtils::InitModelsSourceDebugName(models);

		skybox.loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, queue, glTFLoadingFlags);
	}

	//生成IBL贴图
	{
		vkUtils::generateBRDFLUT(textures.lutBrdf);
		vkUtils::generateIrradianceCube(textures.irradianceCube, textures.environmentCube);
		vkUtils::generatePrefilteredCube(textures.prefilteredCube, textures.environmentCube);
	}

	//设置调试名称
	{
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)offscreenTexture[0].image, "offscreenTexture0");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)offscreenTexture[1].image, "offscreenTexture1");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.environmentCube.image, "environmentCube");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.albedoMap.image, "albedoMap");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.normalMap.image, "normalMap");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.aoMap.image, "aoMap");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.metallicMap.image, "metallicMap");
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.roughnessMap.image, "roughnessMap");
		for (int i = 0; i < swapChain.swapChainImages.size(); i++)
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)swapChain.swapChainImages[i].image, ("swapchainImage" + std::to_string(i)));
	}

	auto tEnd = std::chrono::high_resolution_clock::now(); 
	auto takeTime = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "loadAssets cost time:" << (float)takeTime / 1000.0f << "ms" << std::endl;

}

void VulkanEngine::prepareDescriptors()
{
	// 创建DescriptorPool
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxConcurrentFrames * 8),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxConcurrentFrames * 16)
		};
		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, maxConcurrentFrames * 2);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)descriptorPool, "MainDescriptorPool");
	}
	
	// 创建DescriptorSetLayout
	{
		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(nullptr, 0);

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0)};
		descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &setLayouts[LBI_GLOBAL]));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)setLayouts[LBI_GLOBAL], "globalParamDescriptorSetLayout");

		setLayoutBindings.clear();
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3));
		descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &setLayouts[LBI_IBL]));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)setLayouts[LBI_IBL], "IBLDescriptorLayout");
	}

	// 创建DescriptorSet
	{
		//全局参数
		for (auto i = 0; i < globalDescriptorSets.size(); i++) {
			// 分配描述符集
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &setLayouts[LBI_GLOBAL], 1);
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &globalDescriptorSets[i]));
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)globalDescriptorSets[i], "frameDescriptorSets[" + std::to_string(i) + "].globalParamDescriptorSet ");

			// 更新描述符集
			std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
				vks::initializers::writeDescriptorSet(globalDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &globalParamBuffers[i].globalParamBuffer.descriptor) };
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}

		//灯光
		{
			vkLight::preperDescriptor(vulkanDevice, descriptorPool);
		}

		//IBL贴图
		{
			// 分配描述符集
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &setLayouts[LBI_IBL], 1);
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &IBLDescriptorSet));
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)IBLDescriptorSet, "IBLDescriptorSet ");

			// 更新描述符集
			std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
				vks::initializers::writeDescriptorSet(IBLDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &textures.irradianceCube.descriptor),
				vks::initializers::writeDescriptorSet(IBLDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.prefilteredCube.descriptor),
				vks::initializers::writeDescriptorSet(IBLDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.lutBrdf.descriptor),
				vks::initializers::writeDescriptorSet(IBLDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &textures.environmentCube.descriptor)
			};
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}
	setLayouts[LBI_LIGHTS] = vkLight::descriptorSetLayout;
	setLayouts[LBI_MATERIALS] = vkglTF::MaterialDescriptorSetLayout;
	setLayouts[LBI_MESH] = vkglTF::MeshDescriptorSetLayout;
}

void VulkanEngine::preparePipelines()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	//skybox pipeline
	{
		PipelineBuilder builder(device);

		std::vector<VkDescriptorSetLayout> setLayoutsVector;
		setLayoutsVector.resize(2);
		setLayoutsVector[LBI_GLOBAL] = setLayouts[LBI_GLOBAL];
		setLayoutsVector[LBI_IBL] = setLayouts[LBI_IBL];
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayoutsVector);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines[PL_Skybox].layout));

		// Skybox pipeline
		builder.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		builder.addShaderStage(loadShader(getShadersPath() + "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
		builder.addShaderStage(loadShader(getShadersPath() + "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
		builder.buildPipeline(mainRenderPass.renderPass, pipelineCache, pipelines[PL_Skybox].layout, pipelines[PL_Skybox].pipeline);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines[PL_Skybox].pipeline, "skybox pipeline");
	}

	// PBR pipeline
	{
		std::vector<VkDescriptorSetLayout> setLayoutsVector;
		setLayoutsVector.resize(5);
		setLayoutsVector[LBI_GLOBAL] = setLayouts[LBI_GLOBAL];
		setLayoutsVector[LBI_IBL] = setLayouts[LBI_IBL];
		setLayoutsVector[LBI_LIGHTS] = setLayouts[LBI_LIGHTS];
		setLayoutsVector[LBI_MATERIALS] = setLayouts[LBI_MATERIALS];
		setLayoutsVector[LBI_MESH] = setLayouts[LBI_MESH];
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayoutsVector);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines[PL_PBR_OPAQUE].layout));
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines[PL_PBR_MASK].layout));
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines[PL_PBR_BLEND].layout));

		PipelineBuilder builder(device);
		//启用深度测试与写入
		builder.depthStencilState.depthWriteEnable = VK_TRUE;
		builder.depthStencilState.depthTestEnable = VK_TRUE;
		builder.addShaderStage(loadShader(getShadersPath() + "PBR_Render.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
		builder.addShaderStage(loadShader(getShadersPath() + "PBR_Render.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

		//通过特化常量设置不同的透明度模式
		uint32_t ALPHAMODE;
		VkSpecializationMapEntry specializationMapEntry = vks::initializers::specializationMapEntry(0, 0, sizeof(uint32_t));
		VkSpecializationInfo specializationInfo = vks::initializers::specializationInfo(1, &specializationMapEntry, sizeof(uint32_t), &ALPHAMODE);
		builder.shaderStages[1].pSpecializationInfo = &specializationInfo;

		ALPHAMODE = 0;
		builder.buildPipeline(mainRenderPass.renderPass, pipelineCache, pipelines[PL_PBR_OPAQUE].layout, pipelines[PL_PBR_OPAQUE].pipeline);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines[PL_PBR_OPAQUE].pipeline, "PBR_OPAQUE pipeline");

		ALPHAMODE = 1;
		builder.buildPipeline(mainRenderPass.renderPass, pipelineCache, pipelines[PL_PBR_MASK].layout, pipelines[PL_PBR_MASK].pipeline);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines[PL_PBR_MASK].pipeline, "PBR_MASK pipeline");

		ALPHAMODE = 2;
		builder.depthStencilState.depthWriteEnable = VK_FALSE;
		builder.enableBlendingAlphaBlend();
		builder.buildPipeline(mainRenderPass.renderPass, pipelineCache, pipelines[PL_PBR_BLEND].layout, pipelines[PL_PBR_BLEND].pipeline);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines[PL_PBR_BLEND].pipeline, "PBR_BLEND pipeline");
	}

	auto tEnd = std::chrono::high_resolution_clock::now();
	auto takeTime = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "preparePipelines cost time:" << (float)takeTime / 1000.0f << "ms" << std::endl;
}

void VulkanEngine::prepareUniformBuffers()
{
	for (auto& buffer : globalParamBuffers) {
		// Scene matrices uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.globalParamBuffer, sizeof(globalParam)));
		VK_CHECK_RESULT(buffer.globalParamBuffer.map());
	}
}

void VulkanEngine::updateUniformBuffers()
{
	// 3D object
	globalParam.view = camera.matrices.view;
	globalParam.inverseView = glm::inverse(camera.matrices.view);
	globalParam.projection = camera.matrices.perspective;
	globalParam.camPos = camera.position;
	globalParam.nearPlane = camera.znear;
	globalParam.farPlane = camera.zfar;	
	memcpy(globalParamBuffers[currentBuffer].globalParamBuffer.mapped, &globalParam, sizeof(GlobalParams));

	PostProcessBase::update(width, height);
	postProcessManager->dofProcess->setDOFParams(camera.znear, camera.zfar, camera.focusDistance, camera.focusRange, camera.maxBlurRadius, camera.aperture);
}

void VulkanEngine::preparePostProcess() 
{
	PostProcessBase::preparePostProcessBase(vulkanDevice);
	if (!postProcessManager)
	{
		postProcessManager = new PostProcessManager();
		postProcessManager->prepare();
	}
}

void VulkanEngine::buildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = drawCmdBuffers[currentBuffer];

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	//绘制方向光和点光的阴影贴图	
	directLight.Render(cmdBuffer);
	pointLights.Render(cmdBuffer);

	VkClearValue clearValues[3]{};
	clearValues[0].color = { { 0.25f, 0.25f, 0.25f, 1.0f } };;
	clearValues[1].depthStencil = { 1.0f, 0 };
	clearValues[2].color = { {0.0f, 0.0f, 0.0f, 0.0f} };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = mainRenderPass.renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 3;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = mainRenderPass.frameBuffers[0];

	const VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	const VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	const int descriptorSetCount = 3;
	std::vector<VkDescriptorSet> descriptorSetsArray(descriptorSetCount);
	descriptorSetsArray[LBI_GLOBAL] = globalDescriptorSets[currentBuffer];
	descriptorSetsArray[LBI_IBL] = IBLDescriptorSet;
	descriptorSetsArray[LBI_LIGHTS] = vkLight::descriptorSet;

	// Skybox
	if (displaySkybox)
	{
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline skybox", { 1.0f, 1.0f, 1.0f });
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_Skybox].pipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_Skybox].layout, 0, 2, descriptorSetsArray.data(), 0, nullptr);
		skybox.draw(cmdBuffer);//不需要绑定材质描述符集
		vkUtils::cmdEndLabel(cmdBuffer);
	}

	//PBR
	{
		//不透明物体
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline PBR_OPAQUE", { 1.0f, 1.0f, 1.0f });
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_OPAQUE].layout, 0, descriptorSetCount, descriptorSetsArray.data(), 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_OPAQUE].pipeline);
		for (auto& [key, model] : models)
		{
			model.draw(cmdBuffer, vkglTF::RenderFlags::BindMaterial | vkglTF::RenderFlags::RenderOpaqueNodes, pipelines[PL_PBR_OPAQUE].layout);
		}
		vkUtils::cmdEndLabel(cmdBuffer);

		//遮罩物体
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline PBR_MASK", { 1.0f, 1.0f, 1.0f });
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_MASK].layout, 0, descriptorSetCount, descriptorSetsArray.data(), 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_MASK].pipeline);
		for (auto& [key, model] : models)
		{
			model.draw(cmdBuffer, vkglTF::RenderFlags::BindMaterial | vkglTF::RenderFlags::RenderAlphaMaskedNodes, pipelines[PL_PBR_MASK].layout);
		}
		vkUtils::cmdEndLabel(cmdBuffer);

		//透明物体
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline PBR_BLEND", { 1.0f, 1.0f, 1.0f });
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_BLEND].layout, 0, descriptorSetCount, descriptorSetsArray.data(), 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PL_PBR_BLEND].pipeline);
		for (auto& [key, model] : models)
		{
			model.draw(cmdBuffer, vkglTF::RenderFlags::BindMaterial | vkglTF::RenderFlags::RenderAlphaBlendedNodes, pipelines[PL_PBR_BLEND].layout);
		}
		vkUtils::cmdEndLabel(cmdBuffer);

	}

	vkCmdEndRenderPass(cmdBuffer);
	//更新布局
	offscreenTexture[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthStencil.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	offscreenTexture[0].updateDescriptor();
	depthStencil.updateDescriptor();

	//后处理
	{
		//景深
		if(camera.enableDOF)
		{
			vkUtils::transitionImageLayout(cmdBuffer, offscreenTexture[1], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			postProcessManager->dofProcess->excute(cmdBuffer, offscreenTexture[0].descriptor, depthStencil.descriptor, offscreenTexture[1].view);
			vkUtils::copyImageToImage(cmdBuffer, offscreenTexture[1], offscreenTexture[0]);
		}
		//色调映射
		{
			vkUtils::transitionImageLayout(cmdBuffer, offscreenTexture[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vkUtils::transitionImageLayout(cmdBuffer, swapChain.swapChainImages[currentImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			postProcessManager->toneMappingProcess->excute(cmdBuffer, offscreenTexture[0].descriptor, swapChain.swapChainImages[currentImageIndex].view);
		}
	}

	// UI
	{
		vkUtils::cmdBeginLabel(cmdBuffer, "ImGUI", { 1.0f, 1.0f, 1.0f });
		vkUtils::transitionImageLayout(cmdBuffer, swapChain.swapChainImages[currentImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo colorAttachment = vks::initializers::RenderingAttachmentInfo_Color(swapChain.swapChainImages[currentImageIndex].view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo renderInfo = vks::initializers::RenderingInfo({ width, height }, &colorAttachment, nullptr);
		vkCmdBeginRendering(cmdBuffer, &renderInfo);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
		drawUI(cmdBuffer);
		vkCmdEndRendering(cmdBuffer);
		vkUtils::cmdEndLabel(cmdBuffer);
	}
	vkUtils::transitionImageLayout(cmdBuffer, swapChain.swapChainImages[currentImageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void VulkanEngine::render()
{
	if (!prepared)
		return;
	VulkanEngineBase::prepareFrame();
	updateUniformBuffers();
	buildCommandBuffer();
	VulkanEngineBase::submitFrame();
}

void VulkanEngine::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (ImGui::CollapsingHeader("相机")) {
		ImGui::Indent();
		{
			ImGui::SliderFloat("移动速度", &camera.movementSpeed, 0.1f, 10);
			ImGui::SliderFloat("旋转速度", &camera.rotationSpeed, 0.1f, 10);
			ImGui::InputFloat3("位置", (float*)&camera.position);
			ImGui::InputFloat3("旋转", (float*)&camera.rotation);
			ImGui::Checkbox("景深", &camera.enableDOF);
			if (camera.enableDOF)
			{
				ImGui::InputFloat("焦距", &camera.focusDistance, 0.1f, 1, "%.1f");
				ImGui::InputFloat("焦平面范围", &camera.focusRange, 0.1f, 1, "%.1f");
				ImGui::InputFloat("光圈", &camera.aperture, 0.1f, 1, "%.1f");
				ImGui::InputFloat("最大模糊半径", &camera.maxBlurRadius, 0.1f, 1, "%.1f");
			}
			if(ImGui::Checkbox("正交视图", &camera.useOrthographic))
				camera.switchProjectionType();
			if (camera.useOrthographic)
			{
				float left = camera.orthoLeft;
				float right = camera.orthoRight;
				float top = camera.orthoTop;
				float bottom = camera.orthoBottom;
				float znear = camera.znear;
				float zfar = camera.zfar;
				ImGui::InputFloat("Left", &left, 0.5f, 5, "%.1f");
				ImGui::InputFloat("Right", &right, 0.5f, 5, "%.1f");
				ImGui::InputFloat("Top", &top, 0.5f, 5, "%.1f");
				ImGui::InputFloat("Bottom", &bottom, 0.5f, 5, "%.1f");
				ImGui::InputFloat("NearPlane", &znear, 1, 100, "%.4f");
				ImGui::InputFloat("FarPlane", &zfar, 1, 100, "%.1f");
				if (left != camera.orthoLeft || right != camera.orthoRight || bottom != camera.orthoBottom || top != camera.orthoTop || znear != camera.znear || zfar != camera.zfar)
					camera.setOrthographic(left, right, bottom, top, znear, zfar);
			}
			else
			{
				float fov = camera.fov;
				float znear = camera.znear;
				float zfar = camera.zfar;
				ImGui::InputFloat("FOV", &fov, 0.5f, 5, "%.1f");
				ImGui::InputFloat("NearPlane", &znear, 1, 100, "%.4f");
				ImGui::InputFloat("FarPlane", &zfar, 1, 100, "%.1f");
				if (fov != camera.fov || znear != camera.znear || zfar != camera.zfar)
				{
					camera.setPerspective(fov, (float)width / (float)height, znear, zfar);
					directLight.updateCascades();
				}
			}
		}
		ImGui::Unindent();
	}
	if (ImGui::CollapsingHeader("全局设置")) {
		ImGui::Indent();
		{
			ImGui::InputFloat("曝光", &globalParam.exposure, 0.01f, 0.1f, "%.2f");
			ImGui::InputFloat("Gamma", &globalParam.gamma, 0.01f, 0.1f, "%.2f");
			ImGui::Checkbox("Skybox", &displaySkybox);
		}
		ImGui::Unindent();
	}
	if (ImGui::CollapsingHeader("光源设置")) {
		ImGui::Indent();
		{
			bool isRnder = vkLight::lightData.directLight.isRnder;
			if (ImGui::Checkbox("##vis_SunLight", &isRnder))
			{
				vkLight::lightData.directLight.isRnder = isRnder;
				vkLight::updateLightBuffer();
			}
			ImGui::SameLine(0, 4);
			if (ImGui::CollapsingHeader("太阳光")) {
				bool PCF = vkLight::lightData.directLight.usePCF;
				bool colorCascades = vkLight::lightData.directLight.colorCascades;
				if (ImGui::Checkbox("PCF", &PCF) ||
					ImGui::Checkbox("colorCascades", &colorCascades) ||
					ImGui::InputFloat("深度偏移", &directLight.depthBiasConstant) ||
					ImGui::InputFloat("深度偏移斜率", &directLight.depthBiasSlope) || 
					ImGui::ColorEdit3("太阳光颜色", (float*)&vkLight::lightData.directLight.color))
				{
					vkLight::lightData.directLight.usePCF = PCF;
					vkLight::lightData.directLight.colorCascades = colorCascades;
					vkLight::updateLightBuffer();
				}
				if (ImGui::InputFloat3("太阳方向", (float*)&vkLight::lightData.directLight.direct) ||
					ImGui::SliderFloat("Split lambda", &directLight.cascadeSplitLambda, 0.1f, 1.f))
				{
					directLight.updateCascades();
				}
			}

			if (ImGui::CollapsingHeader("点光源")) {
				int oldCount = vkLight::lightData.activePointLightCount;
				if (ImGui::SliderInt("光源数量", &vkLight::lightData.activePointLightCount, 0, vkLight::MAX_POINTLIGHTS))
				{
					vkLight::updateLightBuffer();
					if (vkLight::lightData.activePointLightCount > oldCount)
					{
						vkDeviceWaitIdle(device);
						pointLights.destroy();
						pointLights.prepare(vulkanDevice, vulkanDevice->getSupportedDepthFormat(false));
					}
				}
				ImGui::Separator();
				ImGui::Separator();
				ImGui::Text("");
				for (int i = 0; i < vkLight::lightData.activePointLightCount; i++)
				{
					std::string lightName = "点光源" + std::to_string(i);
					std::string str_id = "##PointLight" + std::to_string(i);
					bool isRnder = vkLight::lightData.pointLights[i].isRnder;
					if(ImGui::Checkbox((str_id + "isRnder").c_str(), &isRnder))
					{
						vkLight::lightData.pointLights[i].isRnder = isRnder;
						vkLight::updateLightBuffer();
					}
					ImGui::SameLine(0, 4);
					if (ImGui::CollapsingHeader(lightName.c_str())) {
						if (ImGui::InputFloat3(("位置" + str_id).c_str(), (float*)&vkLight::lightData.pointLights[i].position, "%.2f") ||
							ImGui::ColorEdit3(("颜色" + str_id).c_str(), (float*)&vkLight::lightData.pointLights[i].color) ||
							ImGui::SliderFloat(("范围" + str_id).c_str(), &vkLight::lightData.pointLights[i].range, 0, 256) ||
							ImGui::SliderInt(("衰减模式" + str_id).c_str(), &vkLight::lightData.pointLights[i].attenuationMode, 0, 2))
						{
							vkLight::updateLightBuffer();
						}
					}
				}
			}
		}
		ImGui::Unindent();
	}
}

void VulkanEngine::drawNodeTree()
{
	int nodeId = 0;
	{
		// 创建左右分栏布局
		ImGui::Begin("场景树");

		// 左侧节点树
		ImGui::BeginChild("节点树", ImVec2(300, 800), true);
		static bool visibleAll = true;
		if(ImGui::Checkbox("显示所有模型", &visibleAll))
		{
			if(visibleAll)
			{
				for (auto& [key, model] : models)
				{
					model.nodes[0]->visible = true;
				}
			}
			else
			{
				for (auto& [key, model] : models)
				{
					model.nodes[0]->visible = false;
				}
			}
		}
		for (auto& [key, model] : models)
		{
			vkUtils::DrawNodeTree(model.nodes[0], nodeId);
		}
		ImGui::EndChild();

		ImGui::SameLine(0, 4);
		models[M_Sphere].getSceneDimensions();
		// 右侧属性面板
		ImGui::BeginChild("节点属性", ImVec2(550, 800), true);
		vkUtils::DrawNodePropertiesPanel();
		ImGui::EndChild();

		ImGui::End();
	}
}
void VulkanEngine::OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
}