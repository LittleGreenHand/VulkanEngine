#pragma once
#include "base/VulkanTexture.h"

class TextureManager
{
public:
	static TextureManager& Get()
	{
		static TextureManager instance;
		return instance;
	}

	TextureManager() = default;
	~TextureManager() { Destroy(); }
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;
public:
	void Destroy();
	void LoadTextures();

public:
	struct ObjectTextures {
		vks::Texture2D albedoMap;
		vks::Texture2D normalMap;
		vks::Texture2D aoMap;
		vks::Texture2D metallicMap;
		vks::Texture2D roughnessMap;
	} textures{};
	bool isTexturesLoaded = false;
};