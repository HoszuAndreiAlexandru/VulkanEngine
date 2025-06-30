#pragma once
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <exception>
#include "../Systems/Physics.h"
#include "../../Vulkan/VulkanTypes.h"

namespace Engine
{
	class GameObject
	{
	private:
		std::vector<GameObject> children = {};
		glm::vec3 position = glm::vec3(0);

	public:
		glm::vec3 rotation = glm::vec3(0);
		glm::vec3 scale = glm::vec3(1);

		glm::vec3 color = glm::vec3(1, 1, 1);
		float intensity = 1;

		std::string model = "bunny";
		std::string texture = "default";

		float mass = 1;

		bool hasPhysics = true; // Enables physics interactions
		bool isStatic = false; // Is affected by object hierarchy when rendered
		bool isTerrain = false;// Is affected by gravity or not
		bool isLight = false;  // Affects how object is rendered

		bool isActive = true;  // Affects game logic
		bool isVisible = true; // Whether or not the object is rendered

		btRigidBody* rigidBody = nullptr;
		glm::vec3 minBound = glm::vec3(0);
		glm::vec3 maxBound = glm::vec3(0);
		glm::vec3 worldMinBound = glm::vec3(FLT_MAX);
		glm::vec3 worldMaxBound = glm::vec3(-FLT_MAX);

		int bvhInstanceIndex = -1;

		GameObject* parent = nullptr;

		std::function<void()> onUpdate = nullptr;

		GameObject(bool isTerrain = false) : parent(nullptr), rigidBody(nullptr) // Default constructor
		{
			this->isTerrain = isTerrain;
		}

		/*
		GameObject(bool isTerrain = false)
		{
			//GameModel& objRef = *model;
			this->isTerrain = isTerrain;
			//CreateRigidBody(objRef);
		}
		*/

		void CreateRigidBody(GameModel& model)
		{
			if (&model == NULL)
			{
				return;
			}

			if (isTerrain)
			{
				btVector3 planeNormal(0, 1, 0);
				btStaticPlaneShape* terrainShape = new btStaticPlaneShape(planeNormal, 0);

				btTransform startTransform;
				startTransform.setIdentity();

				glm::vec3 position = this->position;
				btVector3 terrainPosition(position.x, position.y, position.z);
				startTransform.setOrigin(terrainPosition);

				btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

				btScalar mass = 0.0f; // Static object, so mass is zero
				btVector3 fallInertia(0, 0, 0); // No inertia needed for static object
				btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, terrainShape, fallInertia);
				this->rigidBody = new btRigidBody(rigidBodyCI);
			}
			else
			{
				std::vector<Vertex> vertices = model.vertices;

				float minX = std::numeric_limits<float>::max();
				float minY = std::numeric_limits<float>::max();
				float minZ = std::numeric_limits<float>::max();
				float maxX = -std::numeric_limits<float>::max();
				float maxY = -std::numeric_limits<float>::max();
				float maxZ = -std::numeric_limits<float>::max();
				//glm::vec3 minBound(std::numeric_limits<float>::max());
				//glm::vec3 maxBound(-std::numeric_limits<float>::max());

				for (const auto& vertex : vertices)
				{
					glm::vec3 scaledVertex = vertex.pos * this->scale;
					minX = glm::min(minX, scaledVertex.x);
					minY = glm::min(minY, scaledVertex.y);
					minZ = glm::min(minZ, scaledVertex.z);
					maxX = glm::max(maxX, scaledVertex.x);
					maxY = glm::max(maxY, scaledVertex.y);
					maxZ = glm::max(maxZ, scaledVertex.z);
					//minBound = glm::min(minBound, scaledVertex);
					//maxBound = glm::max(maxBound, scaledVertex);
				}

				minBound = glm::vec3(minX, minY, minZ);
				maxBound = glm::vec3(maxX, maxY, maxZ);
				glm::vec3 center = (maxBound + minBound) / 2.0f;
				glm::vec3 dimensions = maxBound - minBound;
				btVector3 halfExtents(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f);
				btBoxShape* boundingBoxShape = new btBoxShape(halfExtents);

				btCompoundShape* compoundShape = new btCompoundShape();

				btTransform localTransform;
				localTransform.setIdentity();
				localTransform.setOrigin(btVector3(center.x, center.y, center.z));

				//std::cout << this->model << " : " << center.x << " " << center.y << " " << center.z << std::endl;

				compoundShape->addChildShape(localTransform, boundingBoxShape);

				btVector3 fallInertia(0, 0, 0);
				compoundShape->calculateLocalInertia(mass, fallInertia);

				btTransform startTransform;
				startTransform.setIdentity();
				glm::vec3 position = this->position;
				startTransform.setOrigin(btVector3(position.x, position.y, position.z));

				btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
				btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, compoundShape, fallInertia);
				this->rigidBody = new btRigidBody(rigidBodyCI);
			}

			this->model = model.getName();

			Physics::physicsWorld->addRigidBody(this->rigidBody);
		};

		glm::vec3 getPosition()
		{
			btTransform transform = this->rigidBody->getWorldTransform();
			btVector3 position = transform.getOrigin();
			glm::vec3 glmPosition(position.x(), position.y(), position.z());
			return glmPosition;
		}

		void setPosition(glm::vec3 position)
		{
			this->position = position;
			if (this->rigidBody != nullptr)
			{
				btTransform transform = this->rigidBody->getWorldTransform();
				transform.setOrigin(btVector3(position.x, position.y, position.z));
				this->rigidBody->setWorldTransform(transform);
			}
		}

		void updateTransform()
		{
			btTransform transform = this->rigidBody->getWorldTransform();
			// Translate
			btVector3 position = transform.getOrigin();
			this->position = glm::vec3(position.x(), position.y(), position.z());
			// Rotate
			btQuaternion rotation = transform.getRotation();
			this->rotation = glm::vec3(glm::degrees(atan2(2.0f * (rotation.y() * rotation.z() + rotation.w() * rotation.x()), rotation.w() * rotation.w() - rotation.x() * rotation.x() - rotation.y() * rotation.y() + rotation.z() * rotation.z())),
				glm::degrees(asin(-2.0f * (rotation.x() * rotation.z() - rotation.w() * rotation.y()))),
				glm::degrees(atan2(2.0f * (rotation.x() * rotation.y() + rotation.w() * rotation.z()), rotation.w() * rotation.w() + rotation.x() * rotation.x() - rotation.y() * rotation.y() - rotation.z() * rotation.z())));
			// Scale is not updated from physics, so it remains the same
		}

		glm::mat4 calculateModel()
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, position);
			model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, scale);

			if (parent != nullptr)
			{
				model = parent->calculateModel() * model;
			}

			return model;
		};

		void addChild(GameObject* child)
		{
			if (child != nullptr)
			{
				child->parent = this;
				children.push_back(*child);
			}
		}

		void onUpdateInternal()
		{
			if (onUpdate)
			{
				onUpdate();
			}
		}
	};
};