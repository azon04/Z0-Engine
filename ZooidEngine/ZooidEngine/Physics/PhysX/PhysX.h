#ifndef __PHYSX_H__
#define __PHYSX_H__

#include "Physics/Physics.h"

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxDefaultCpuDispatcher;
	class PxScene;
	class PxPvd;
}

namespace ZE
{
	class PhysXEngine : public IPhysics
	{

	public:
		
		virtual void Setup() override;
		virtual void PreUpdate() override;
		virtual void Update() override;
		virtual void PostUpdate() override;
		virtual void DrawDebug() override;
		virtual void Destroy() override;

		virtual Handle CreateTriggerCollision(PhysicsShape _shape, void* _data) override;
		virtual Handle CreateDynamicRigidBody(PhysicsShape _shape, void* _data) override;
		virtual Handle CreateStaticRigidBody(PhysicsShape _shape, void* _data) override;

		virtual void DestroyPhysicsObject(Handle handle) override;

	private:
		physx::PxFoundation* m_physxFoundation;
		physx::PxPhysics* m_physxPhysics;
		physx::PxDefaultCpuDispatcher* m_physxCPUDispatcher;
		physx::PxScene* m_physxScene;
		physx::PxPvd* m_physxPVD;
	};
}
#endif
