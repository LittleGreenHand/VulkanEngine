#include "VulkanEngine.h"
#include "VulkanUtil.h"
#include "types.hpp"
void VulkanEngine::getEnabledFeatures()
{
	if (deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	}

	vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vulkan11Features.shaderDrawParameters = VK_TRUE;

	deviceCreatepNextChain = &vulkan11Features;
}

void VulkanEngine::loadAssets()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	//加载纹理
	textures.environmentCube.loadFromFile(getAssetPath() + "textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);
	textures.albedoMap.loadFromFile(getAssetPath() + "models/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
	textures.normalMap.loadFromFile(getAssetPath() + "models/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
	textures.aoMap.loadFromFile(getAssetPath() + "models/cerberus/ao.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	textures.metallicMap.loadFromFile(getAssetPath() + "models/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	textures.roughnessMap.loadFromFile(getAssetPath() + "models/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.environmentCube.image, "environmentCube");
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.albedoMap.image, "albedoMap");
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.normalMap.image, "normalMap");
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.aoMap.image, "aoMap");
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.metallicMap.image, "metallicMap");
	vkUtils::setObjectDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)textures.roughnessMap.image, "roughnessMap");

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


	auto tEnd = std::chrono::high_resolution_clock::now(); 
	auto takeTime = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "loadAssets cost time:" << (float)takeTime / 1000.0f << "ms" << std::endl;

}

void VulkanEngine::setupDescriptors()
{
	MaterialDescriptorSetLayout = vkglTF::MaterialDescriptorSetLayout;
	meshDescriptorSetLayout = vkglTF::MeshDescriptorSetLayout;
	lights.preperDescriptor(vulkanDevice);
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
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &emptyDescriptorLayout));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)emptyDescriptorLayout, "emptyDescriptorLayout");

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0)};
		descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &globalParamDescriptorSetLayout));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)globalParamDescriptorSetLayout, "globalParamDescriptorSetLayout");

		setLayoutBindings.clear();
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2));
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3));
		descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &IBLDescriptorLayout));
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)IBLDescriptorLayout, "IBLDescriptorLayout");
	}

	// 创建DescriptorSet
	{
		//全局参数
		for (auto i = 0; i < frameDescriptorSets.size(); i++) {
			// 分配描述符集
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &globalParamDescriptorSetLayout, 1);
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &frameDescriptorSets[i].globalParamDescriptorSet));
			vkUtils::setObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)frameDescriptorSets[i].globalParamDescriptorSet, "frameDescriptorSets[" + std::to_string(i) + "].globalParamDescriptorSet ");

			// 更新描述符集
			std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
				vks::initializers::writeDescriptorSet(frameDescriptorSets[i].globalParamDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &globalParamBuffers[i].globalParamBuffer.descriptor) };
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}

		//IBL贴图
		{
			// 分配描述符集
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &IBLDescriptorLayout, 1);
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
}

void VulkanEngine::preparePipelines()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	//skybox pipeline
	{
		PipelineBuilder builder(device);
		std::vector<VkDescriptorSetLayout> setLayouts;
		setLayouts.resize(2);
		setLayouts[vks::LBI_GLOBAL] = globalParamDescriptorSetLayout;
		setLayouts[vks::LBI_IBL] = IBLDescriptorLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayouts);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines.skyboxPipelineLayout));

		// Skybox pipeline
		builder.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		builder.addShaderStage(loadShader(getShadersPath() + "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
		builder.addShaderStage(loadShader(getShadersPath() + "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
		builder.buildPipeline(renderPass, pipelineCache, pipelines.skyboxPipelineLayout, pipelines.skybox);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines.skybox, "skybox pipeline");
	}

	// PBR pipeline
	{
		PipelineBuilder builder(device);
		std::vector<VkDescriptorSetLayout> setLayouts;
		setLayouts.resize(vks::LBI_COUNT);
		setLayouts[vks::LBI_GLOBAL] = globalParamDescriptorSetLayout;
		setLayouts[vks::LBI_IBL] = IBLDescriptorLayout;
		setLayouts[vks::LBI_LIGHTS] = lights.descriptorSetLayout;
		setLayouts[vks::LBI_MATERIALS] = vkglTF::MaterialDescriptorSetLayout;
		setLayouts[vks::LBI_CUSTOM] = meshDescriptorSetLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayouts);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelines.pbrPipelineLayout));

		builder.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		//启用深度测试与写入
		builder.depthStencilState.depthWriteEnable = VK_TRUE;
		builder.depthStencilState.depthTestEnable = VK_TRUE;
		builder.addShaderStage(loadShader(getShadersPath() + "PBRRender.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
		builder.addShaderStage(loadShader(getShadersPath() + "PBRRender.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
		builder.buildPipeline(renderPass, pipelineCache, pipelines.pbrPipelineLayout, pipelines.pbr);
		vkUtils::setObjectDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipelines.pbr, "PBRRender pipeline");
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
	
	memcpy(globalParamBuffers[currentBuffer].globalParamBuffer.mapped, &globalParam, sizeof(GlobalParams));
}

void VulkanEngine::prepare()
{
	VulkanEngineBase::prepare();
	vkUtils::Init(this);
	loadAssets();
	vkUtils::generateBRDFLUT(textures.lutBrdf);
	vkUtils::generateIrradianceCube(textures.irradianceCube, textures.environmentCube);
	vkUtils::generatePrefilteredCube(textures.prefilteredCube, textures.environmentCube);
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	prepared = true;
}

void VulkanEngine::buildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = drawCmdBuffers[currentBuffer];

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[3]{};
	clearValues[0].color = { { 0.25f, 0.25f, 0.25f, 1.0f } };;
	clearValues[1].depthStencil = { 1.0f, 0 };
	clearValues[2].color = { {0.0f, 0.0f, 0.0f, 0.0f} };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 3;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = frameBuffers[currentImageIndex];

	const VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	const VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	const int descriptorSetCount = 3;
	std::vector<VkDescriptorSet> descriptorSetsArray(descriptorSetCount);
	descriptorSetsArray[vks::LBI_GLOBAL] = frameDescriptorSets[currentBuffer].globalParamDescriptorSet;
	descriptorSetsArray[vks::LBI_IBL] = IBLDescriptorSet;
	descriptorSetsArray[vks::LBI_LIGHTS] = lights.descriptorSet;
	// Skybox
	if (displaySkybox)
	{
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline skybox", { 1.0f, 1.0f, 1.0f });
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skybox);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skyboxPipelineLayout, 0, 2, descriptorSetsArray.data(), 0, nullptr);
		skybox.draw(cmdBuffer);//不需要绑定材质描述符集
		vkUtils::cmdEndLabel(cmdBuffer);
	}

	//PBR
	{
		vkUtils::cmdBeginLabel(cmdBuffer, "Pipeline PBR", { 1.0f, 1.0f, 1.0f });
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.pbr);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.pbrPipelineLayout, 0, descriptorSetCount, descriptorSetsArray.data(), 0, nullptr);

		for (auto& [key, model] : models)
		{
			model.draw(cmdBuffer, vkglTF::RenderFlags::BindMaterial, pipelines.pbrPipelineLayout);
		}
		vkUtils::cmdEndLabel(cmdBuffer);
	}

	// UI
	drawUI(cmdBuffer);
	vkCmdEndRenderPass(cmdBuffer);
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
	if (ImGui::CollapsingHeader("相机"), ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::Indent();
		{
			ImGui::SliderFloat("移动速度", &camera.movementSpeed, 0.1f, 10);
			ImGui::SliderFloat("旋转速度", &camera.rotationSpeed, 0.1f, 10);
			ImGui::InputFloat3("位置", (float*)&camera.position);
			ImGui::InputFloat3("旋转", (float*)&camera.rotation);
			float fov = camera.fov;
			float znear = camera.znear;
			float zfar = camera.zfar;
			ImGui::InputFloat("FOV", &fov, 0.5f, 5, "%.1f");
			ImGui::InputFloat("NearPlane", &znear, 1, 100, "%.4f");
			ImGui::InputFloat("FarPlane", &zfar, 1, 100, "%.1f");
			if(fov != camera.fov || znear != camera.znear || zfar != camera.zfar)
				camera.setPerspective(fov, (float)width / (float)height, znear, zfar);
		}
		ImGui::Unindent();
	}
	if (ImGui::CollapsingHeader("全局设置"), ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::Indent();
		{
			ImGui::InputFloat("曝光", &globalParam.exposure, 0.01f, 0.1f, "%.2f");
			ImGui::InputFloat("Gamma", &globalParam.gamma, 0.01f, 0.1f, "%.2f");
			ImGui::Checkbox("Skybox", &displaySkybox);
		}
		ImGui::Unindent();
	}
	if (ImGui::CollapsingHeader("光源设置"), ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::Indent();
		{
			if(ImGui::SliderInt("光源数量", &lights.lightData.activeLightCount, 0, MAX_LIGHTS))
			{
				lights.updateLightBuffer();
			}
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Text("");
			for (int i = 0; i < lights.lightData.activeLightCount; i++)
			{
				std::string lightName = "光源" + std::to_string(i);
				if (ImGui::InputFloat3((lightName + "位置").c_str(), (float*)&lights.lightData.lights[i].position, "%.2f") ||
					ImGui::ColorEdit3((lightName + "颜色").c_str(), (float*)&lights.lightData.lights[i].color) ||
					ImGui::SliderFloat((lightName + "范围").c_str(), &lights.lightData.lights[i].range, 0, 256) ||
					ImGui::SliderInt((lightName + "衰减模式").c_str(), &lights.lightData.lights[i].attenuationMode, 0, 2))
				{
					lights.updateLightBuffer();
				}
				ImGui::Text("");
				ImGui::Separator();
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