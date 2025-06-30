#pragma once
#include <btBulletDynamicsCommon.h>
#include "../Globals.h"

namespace Engine
{
	class Physics
	{
	private:
		btBroadphaseInterface* broadphase;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btSequentialImpulseConstraintSolver* solver;

	public:
		static btDiscreteDynamicsWorld* physicsWorld;

		Physics()
		{
			broadphase = new btDbvtBroadphase();
			collisionConfiguration = new btDefaultCollisionConfiguration();
			dispatcher = new btCollisionDispatcher(collisionConfiguration);
			solver = new btSequentialImpulseConstraintSolver();
			physicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

			physicsWorld->setGravity(btVector3(0, -9.81f, 0));
		}

		void stepSimulation(double deltaTime)
		{
			physicsWorld->stepSimulation(deltaTime);
		}
	};

	btDiscreteDynamicsWorld* Physics::physicsWorld = nullptr;
}