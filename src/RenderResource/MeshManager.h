#pragma once
#include <map>
#include "Types.hpp"
#include "RenderBase/VulkanglTFModel.h"

class MeshManager
{
public:
	static MeshManager& Get()
	{
		static MeshManager instance;
		return instance;
	}

	MeshManager() = default;
	~MeshManager() { Destroy(); }
	MeshManager(const MeshManager&) = delete;
	MeshManager& operator=(const MeshManager&) = delete;
	MeshManager(MeshManager&&) = delete;
	MeshManager& operator=(MeshManager&&) = delete;
public:
	void Destroy();
	void LoadModels();
	//计算并获取场景包围盒
	Dimensions GetSceneDimensions();
	void InitModelsSourceDebugName();
public:
	std::map<GLTFModels, vkglTF::Model> models;
	vkglTF::Model skybox;
	bool isModelsLoaded = false;
};