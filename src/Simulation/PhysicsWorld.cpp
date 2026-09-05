#include "PhysicsWorld.h"
#include "Core/Log.h"

bool PhysicsWorld::Init()
{
	LOG_INFO("PhysX Version: {}", PX_PHYSICS_VERSION);

	//Foundation
	{
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
		if (!m_foundation)
		{
			LOG_ERROR("Create PxFoundation failed!");
			return false;
		}
	}

	// PhysX SDK实例
	{
		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, m_toleranceScale, true);
		if (!m_physics)
		{
			LOG_ERROR("Create PxPhysics failed!");
			return false;
		}
	}

	// PhysX Scene
	{
		m_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = m_dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_scene = m_physics->createScene(sceneDesc);
		if (!m_scene)
		{
			LOG_ERROR("Create PxScene failed!");
			return false;
		}
	}

	// 默认材质
	{
		m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.6f);
		if (!m_defaultMaterial)
		{
			LOG_ERROR("Create PxMaterial failed!");
			return false;
		}
	}

	// 添加地面Plane
	{
		physx::PxRigidStatic* groundPlane =
			physx::PxCreatePlane(
				*m_physics,
				physx::PxPlane(0, 1, 0, 0),
				*m_defaultMaterial
			);

		m_scene->addActor(*groundPlane);
	}

	// 添加一个动态刚体Box
	{
		physx::PxTransform boxTransform(physx::PxVec3(0, 10, 0));
		physx::PxBoxGeometry boxGeometry(0.5f, 0.5f, 0.5f);
		dynamicBox = m_physics->createRigidDynamic(boxTransform);
		if (dynamicBox)
		{
			physx::PxShape* boxShape = m_physics->createShape(boxGeometry, *m_defaultMaterial);
			dynamicBox->attachShape(*boxShape);
			boxShape->release(); // 释放形状，因为它已经被附加到刚体上
			m_scene->addActor(*dynamicBox);
			physx::PxRigidBodyExt::updateMassAndInertia(*dynamicBox, 10.0f); //设置刚体密度
		}
		else
		{
			LOG_ERROR("Create dynamic box failed!");
			return false;
		}
	}
	m_isInitialized = true;
	return true;
}

void PhysicsWorld::Update(float deltaTime)
{
	if (!m_isInitialized)
		return;
	mAccumulator += deltaTime;

	while (mAccumulator >= mFixedDeltaTime)
	{
		m_scene->simulate(mFixedDeltaTime);
		m_scene->fetchResults(true);

		mAccumulator -= mFixedDeltaTime;
	}
}

void PhysicsWorld::Destroy()
{
	m_isInitialized = false;
	if(m_scene)
	{
		m_scene->release();
		m_scene = nullptr;
	}
	if (m_defaultMaterial)
	{
		m_defaultMaterial->release();
		m_defaultMaterial = nullptr;
	}
	if (m_dispatcher)
	{
		m_dispatcher->release();
		m_dispatcher = nullptr;
	}
	if (m_physics)
	{
		m_physics->release();
		m_physics = nullptr;
	}
	if (m_foundation)
	{
		m_foundation->release();
		m_foundation = nullptr;
	}
}

