#pragma once
#include "RenderBase/VulkanTexture.h"

class EnvironmentManager
{
public:
	static EnvironmentManager& Get()
	{
		static EnvironmentManager instance;
		return instance;
	}

	EnvironmentManager() = default;
	~EnvironmentManager() { Destroy(); }
	EnvironmentManager(const EnvironmentManager&) = delete;
	EnvironmentManager& operator=(const EnvironmentManager&) = delete;
	EnvironmentManager(EnvironmentManager&&) = delete;
	EnvironmentManager& operator=(EnvironmentManager&&) = delete;
public:
	void Destroy();
	void LoadIBLTextures();

	static void GenerateBRDFLUT(vks::Texture2D& lutBrdf);
	static void GenerateIrradianceCube(vks::TextureCubeMap& irradianceCube, vks::TextureCubeMap& environmentCube);
	static void GeneratePrefilteredCube(vks::TextureCubeMap& prefilteredCube, vks::TextureCubeMap& environmentCube);
public:
	struct IBLTextures {
		vks::TextureCubeMap environmentCube;
		// Generated at runtime
		vks::Texture2D lutBrdf;
		vks::TextureCubeMap irradianceCube;
		vks::TextureCubeMap prefilteredCube;
	}IBL;
	bool isIBLTexturesLoaded = false;
};