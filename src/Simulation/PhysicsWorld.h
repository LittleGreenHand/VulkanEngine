#pragma once
#include "PxPhysicsAPI.h"

class PhysicsWorld
{
public:
	bool Init();
	void Update(float deltaTime);
	void Destroy();

private:
	physx::PxDefaultAllocator m_allocator;
	physx::PxDefaultErrorCallback m_errorCallback;

	bool m_isInitialized = false;
	physx::PxFoundation* m_foundation = nullptr;
	physx::PxPhysics* m_physics = nullptr;
	physx::PxTolerancesScale m_toleranceScale;
	physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
	physx::PxScene* m_scene = nullptr;
	physx::PxMaterial* m_defaultMaterial = nullptr;

	float mAccumulator = 0.0f;
	float mFixedDeltaTime = 1.0f / 60.0f;

public:
	//临时变量
	physx::PxRigidDynamic* dynamicBox = nullptr;
};