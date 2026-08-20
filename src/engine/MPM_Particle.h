#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "VulkanBuffer.h"
struct MPMParticle
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
	std::vector<MPMParticle> particles;
	vks::Buffer particleBuffer;
};