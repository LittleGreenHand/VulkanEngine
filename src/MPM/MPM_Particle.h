#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "base/VulkanBuffer.h"
struct MPMParticleStruct
{
	glm::vec3 position{ 0.0f };
	float mass = 1.0f;

	glm::vec3 velocity{ 0.0f };
	float volume = 1.0f;
};
class MPMParticle
{
public:
	MPMParticle() = default;
	~MPMParticle() { Destroy(); }

	void GenerateBoxParticles(int numParticles, const glm::vec3& minPos, const glm::vec3& maxPos);
	void Destroy();

public:
	std::vector<MPMParticleStruct> particles;
	vks::Buffer particleBuffer;
};