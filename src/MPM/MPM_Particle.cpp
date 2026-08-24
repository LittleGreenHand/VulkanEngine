#include "MPM/MPM_Particle.h"

void MPMParticle::GenerateBoxParticles(int numParticles, const glm::vec3& minPos, const glm::vec3& maxPos)
{

}

void MPMParticle::Destroy()
{
	particles.clear();
	particleBuffer.destroy();
}
