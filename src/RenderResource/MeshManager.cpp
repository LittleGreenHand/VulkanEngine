#include "MeshManager.h"
#include "Render/VulkanContext.h"
#include "RenderBase/VulkanDevice.h"
#include "TextureManager.h"
#include "Render/VulkanDebugUtils.h"
#include "Math/MathUtils.h"
#include "Core/Log.h"

void MeshManager::Destroy()
{
	LOG_DEBUG("Destroying mesh manager resources");
	if(isModelsLoaded)
	{
		vkDeviceWaitIdle(VulkanContext::GetVkDevice());
		models.clear();
		skybox.Destroy();
		vkglTF::destroyEmptyTexture();
	}
	isModelsLoaded = false;
	LOG_DEBUG("Destroying mesh manager resources successfully");
}

void MeshManager::LoadModels()
{
	LOG_DEBUG("Loading glTF models");
	vks::VulkanDevice* vulkanDevice = VulkanContext::GetVulkanDevice();
	uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

	models[M_Cube].loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	models[M_Cube].nodes[0]->clearTransform();
	models[M_Cube].nodes[0]->scale = (glm::vec3(0.01, 0.01, 0.01));
	models[M_Cube].nodes[0]->translation = (glm::vec3(0, -0, -1));
	models[M_Cube].nodes[0]->visible = false;
	models[M_Cube].nodes[0]->update();

	models[M_Cerberus].loadFromFile(getAssetPath() + "models/cerberus/cerberus.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setBaseColorTexture(&TextureManager::Get().textures.albedoMap);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setNormalTexture(&TextureManager::Get().textures.normalMap);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setAOTexture(&TextureManager::Get().textures.aoMap);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setMetallicTexture(&TextureManager::Get().textures.metallicMap);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.setRoughnessTexture(&TextureManager::Get().textures.roughnessMap);
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.updateDescriptorSet();
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.materialParameters.metallicFactor = 1;
	models[M_Cerberus].linearNodes[0]->mesh->primitives[0]->material.materialParameters.roughnessFactor = 1;
	models[M_Cerberus].nodes[0]->clearTransform();
	models[M_Cerberus].nodes[0]->rotation = MathUtils::EularToQuaternion(glm::vec3(-90, 90, 0));
	models[M_Cerberus].nodes[0]->translation = (glm::vec3(0.2, -0.15, -2.5));
	models[M_Cerberus].nodes[0]->scale = (glm::vec3(0.2, 0.2, 0.2));
	models[M_Cerberus].nodes[0]->visible = false;
	models[M_Cerberus].nodes[0]->update();

	models[M_Sponza].loadFromFile(getAssetPath() + "models/sponza/sponza.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	models[M_Sponza].nodes[0]->clearTransform();
	models[M_Sponza].nodes[0]->rotation = MathUtils::EularToQuaternion(glm::vec3(0, 90, 0));
	models[M_Sponza].nodes[0]->translation = (glm::vec3(0, -1, 0));
	models[M_Sponza].nodes[0]->update();

	models[M_Sphere].loadFromFile(getAssetPath() + "models/sphere.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	models[M_Sphere].nodes[0]->clearTransform();
	models[M_Sphere].nodes[0]->scale = (glm::vec3(0.1, 0.1, 0.1));
	models[M_Sphere].nodes[0]->translation = (glm::vec3(0, -0, -2));
	models[M_Sphere].nodes[0]->visible = true;
	models[M_Sphere].materials[0].materialParameters.baseColorFactor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	models[M_Sphere].materials[0].materialParameters.metallicFactor = 0.0f;
	models[M_Sphere].materials[0].materialParameters.roughnessFactor = 0.5f;
	models[M_Sphere].materials[0].alphaMode = vkglTF::Material::ALPHAMODE_BLEND;
	models[M_Sphere].nodes[0]->update();

	models[M_Axis].loadFromFile(getAssetPath() + "models/axis.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	models[M_Axis].nodes[0]->clearTransform();
	models[M_Axis].nodes[0]->scale = (glm::vec3(0.1, 0.1, 0.1));
	models[M_Axis].nodes[0]->translation = (glm::vec3(0, -0.15, -1));
	models[M_Axis].nodes[0]->visible = false;
	models[M_Axis].nodes[0]->update();

	//models[M_Terrain].loadFromFile(getAssetPath() + "models/Terrain.gltf", vulkanDevice, queue, glTFLoadingFlags);

	skybox.loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, VulkanContext::GetGraphicsQueue(), glTFLoadingFlags);
	InitModelsSourceDebugName();
	isModelsLoaded = true;
	LOG_DEBUG("glTF models loaded successfully");
}

Dimensions MeshManager::GetSceneDimensions()
{
	Dimensions dimension;
	dimension.min = glm::vec3(FLT_MAX);
	dimension.max = glm::vec3(-FLT_MAX);
	for (auto& [key, model] : models)
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

void MeshManager::InitModelsSourceDebugName()
{
	for (auto& [key, model] : models)
	{
		for (int i = 0; i < model.materials.size(); i++)
		{
			VulkanDebugUtils::SetObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)model.materials[i].descriptorSet, model.modelName + "_Material_" + std::to_string(i) + "DescriptorSet");
		}
		for (int i = 0; i < model.linearNodes.size(); i++)
		{
			if (model.linearNodes[i]->mesh)
			{
				VulkanDebugUtils::SetObjectDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)model.linearNodes[i]->mesh->uniformBuffer.descriptorSet, model.linearNodes[i]->name + "_MeshDescriptorSet");
			}
		}
	}
}
