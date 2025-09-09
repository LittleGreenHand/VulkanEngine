#include "VulkanEngine.h"
#include "VulkanUtil.h"
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

	uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
	models.object.loadFromFile(getAssetPath() + "models/cerberus/cerberus.gltf", vulkanDevice, queue, glTFLoadingFlags);
	textures.environmentCube.loadFromFile(getAssetPath() + "textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue);
	textures.albedoMap.loadFromFile(getAssetPath() + "models/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
	textures.normalMap.loadFromFile(getAssetPath() + "models/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
	textures.aoMap.loadFromFile(getAssetPath() + "models/cerberus/ao.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	textures.metallicMap.loadFromFile(getAssetPath() + "models/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	textures.roughnessMap.loadFromFile(getAssetPath() + "models/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	vkUtils::SetImageDebugName(textures.environmentCube.image, "environmentCube");
	vkUtils::SetImageDebugName(textures.albedoMap.image, "albedoMap");
	vkUtils::SetImageDebugName(textures.normalMap.image, "normalMap");
	vkUtils::SetImageDebugName(textures.aoMap.image, "aoMap");
	vkUtils::SetImageDebugName(textures.metallicMap.image, "metallicMap");
	vkUtils::SetImageDebugName(textures.roughnessMap.image, "roughnessMap");
		
	models.skybox.loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, queue, glTFLoadingFlags);

	auto tEnd = std::chrono::high_resolution_clock::now(); 
	auto takeTime = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "loadAssets cost time:" << (float)takeTime / 1000.0f << "ms" << std::endl;

}

void VulkanEngine::setupDescriptors()
{
	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxConcurrentFrames * 4),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxConcurrentFrames * 16)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, maxConcurrentFrames * 2);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

	// Descriptor set layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 7),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 9),
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

	// Sets per frame, just like the buffers themselves
	// Images do not need to be duplicated per frame, we reuse the same one for each frame
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
	for (auto i = 0; i < uniformBuffers.size(); i++) {
		// Scene
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[i].scene));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[i].scene.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers[i].params.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.irradianceCube.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &textures.lutBrdf.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &textures.prefilteredCube.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &textures.albedoMap.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, &textures.normalMap.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, &textures.aoMap.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, &textures.metallicMap.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 9, &textures.roughnessMap.descriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Sky box
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[i].skybox));
		writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSets[i].skybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[i].skybox.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].skybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers[i].params.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets[i].skybox, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.environmentCube.descriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
}

void VulkanEngine::preparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	// Pipelines
	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV, vkglTF::VertexComponent::Tangent });

	// Skybox pipeline (background cube)
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	shaderStages[0] = loadShader(getShadersPath() + "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getShadersPath() + "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.skybox));

	// PBR pipeline
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	shaderStages[0] = loadShader(getShadersPath() + "pbrtexture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getShadersPath() + "pbrtexture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	// Enable depth test and write
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthTestEnable = VK_TRUE;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.pbr));
}

void VulkanEngine::prepareUniformBuffers()
{
	for (auto& buffer : uniformBuffers) {
		// Scene matrices uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.scene, sizeof(UniformDataMatrices)));
		VK_CHECK_RESULT(buffer.scene.map());
		// Skybox matrices uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.skybox, sizeof(UniformDataMatrices)));
		VK_CHECK_RESULT(buffer.skybox.map());
		// Shared parameter uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.params, sizeof(UniformDataParams)));
		VK_CHECK_RESULT(buffer.params.map());
	}
}

void VulkanEngine::updateUniformBuffers()
{
	// 3D object
	uniformDataMatrices.projection = camera.matrices.perspective;
	uniformDataMatrices.view = camera.matrices.view;
	//uniformDataMatrices.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	uniformDataMatrices.model = glm::mat4(1);
	uniformDataMatrices.camPos = camera.position * -1.0f;
	//uniformDataMatrices.camPos = camera.viewPos;
	memcpy(uniformBuffers[currentBuffer].scene.mapped, &uniformDataMatrices, sizeof(UniformDataMatrices));

	// Skybox
	uniformDataMatrices.model = glm::mat4(glm::mat3(camera.matrices.view));
	memcpy(uniformBuffers[currentBuffer].skybox.mapped, &uniformDataMatrices, sizeof(UniformDataMatrices));

	// PBR parameters
	const float p = 15.0f;
	uniformDataParams.lights[0] = glm::vec4(-p, -p * 0.5f, -p, 1.0f);
	uniformDataParams.lights[1] = glm::vec4(-p, -p * 0.5f, p, 1.0f);
	uniformDataParams.lights[2] = glm::vec4(p, -p * 0.5f, p, 1.0f);
	uniformDataParams.lights[3] = glm::vec4(p, -p * 0.5f, -p, 1.0f);

	memcpy(uniformBuffers[currentBuffer].params.mapped, &uniformDataParams, sizeof(UniformDataParams));
}

void VulkanEngine::prepare()
{
	VulkanEngineBase::prepare();
	vkUtils::Init(this);
	loadAssets();
	vkUtils::generateBRDFLUT(textures.lutBrdf);
	vkUtils::generateIrradianceCube(textures.irradianceCube, textures.environmentCube);
	vkUtils::generatePrefilteredCube(textures.prefilteredCube, textures.environmentCube);
	vkUtils::SetImageDebugName(textures.lutBrdf.image, "LutBRDF");
	vkUtils::SetImageDebugName(textures.irradianceCube.image, "irradianceCube");
	vkUtils::SetImageDebugName(textures.prefilteredCube.image, "prefilteredCube");
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

	// Skybox
	if (displaySkybox)
	{
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentBuffer].skybox, 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skybox);
		models.skybox.draw(cmdBuffer);
	}
	// Objects
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentBuffer].scene, 0, nullptr);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.pbr);
	models.object.draw(cmdBuffer);
	
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
			float fov = camera.fov;
			float znear = camera.znear;
			float zfar = camera.zfar;
			ImGui::InputFloat("FOV", &fov, 0.5f, 5, 1);
			ImGui::InputFloat("NearPlane", &znear, 1, 100, 1);
			ImGui::InputFloat("FarPlane", &zfar, 1, 100, 1);
			if(fov != camera.fov || znear != camera.znear || zfar != camera.zfar)
				camera.setPerspective(fov, (float)width / (float)height, znear, zfar);
		}
		ImGui::Unindent();
	}
	if (ImGui::CollapsingHeader("PBR设置"), ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::Indent();
		{
			ImGui::InputFloat("曝光", &uniformDataParams.exposure, 0.01f, 0.1f, 2);
			ImGui::InputFloat("Gamma", &uniformDataParams.gamma, 0.01f, 0.1f, 2);
			ImGui::SliderFloat("粗糙度", &uniformDataParams.globalRoughness, 0.01f, 1);
			ImGui::SliderFloat("金属度", &uniformDataParams.globalMetallic, 0.01f, 1);
			ImGui::Checkbox("Skybox", &displaySkybox);
		}
		ImGui::Unindent();
	}
}
void VulkanEngine::OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	// Only react to keyboard input if ImGui is active
	if (io.WantCaptureKeyboard) {
		// Character input
		if (uMsg == WM_CHAR) {
			if (wParam > 0 && wParam < 0x10000) {
				io.AddInputCharacter((unsigned short)wParam);
			}
		}
		// Special keys (tab, cursor, etc.)
		if ((wParam < 256) && (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)) {
			io.KeysDown[wParam] = true;
		}
		if ((wParam < 256) && (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)) {
			io.KeysDown[wParam] = false;
		}
	}
}