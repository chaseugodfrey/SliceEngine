#include <pch.h>
#include "ContactListener.h"
#include "../ECS/GOFactory.h"
#include "../Core/Core.h"

//JPH::ValidateResult SliceEngine::MyContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
//{
//	return JPH::ValidateResult();
//}

void SliceEngine::MyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	// Use world space to see actual collision position
	//JPH::RVec3 contactPoint = inManifold.GetWorldSpaceContactPointOn1(0); // First contact point in world space
	//std::cout << "Collision at: (" << contactPoint.GetX() << ", "
	//	<< contactPoint.GetY() << ", " << contactPoint.GetZ() << ")" << std::endl;

	// Also log body positions to compare
	//std::cout << "Body1 pos: (" << inBody1.GetPosition().GetX() << ", "
	//	<< inBody1.GetPosition().GetY() << ", " << inBody1.GetPosition().GetZ() << ")" << std::endl;
	//std::cout << "Body2 pos: (" << inBody2.GetPosition().GetX() << ", "
	//	<< inBody2.GetPosition().GetY() << ", " << inBody2.GetPosition().GetZ() << ")" << std::endl;


	GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody1.GetUserData()));
	GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody2.GetUserData()));

	auto& colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
	auto& colliderShape2 = checkEntity2.GetComponent<ColliderShape>();
	//std::cout << inBody1.GetUserData() << " and " << inBody2.GetUserData() << std::endl;
	//std::cout << (uint32_t)checkEntity1.GetEntity() << " and " << (uint32_t)checkEntity2.GetEntity()<< std::endl;
	if (colliderShape1.isTrigger || colliderShape2.isTrigger)
	{
		OnTriggerEnterEvent triggerEvent1;
		OnTriggerEnterEvent triggerEvent2;

		triggerEvent1.entity = checkEntity1.GetEntity();
		triggerEvent1.other = checkEntity2.GetEntity();

		triggerEvent2.entity = checkEntity2.GetEntity();
		triggerEvent2.other = checkEntity1.GetEntity();

		EventManager::GetInstance()->Publish<OnTriggerEnterEvent>(triggerEvent1);
		EventManager::GetInstance()->Publish<OnTriggerEnterEvent>(triggerEvent2);
	}
	else
	{
		OnCollisionEnterEvent collisionEvent1;
		OnCollisionEnterEvent collisionEvent2;

		collisionEvent1.entity = checkEntity1.GetEntity();
		collisionEvent1.other = checkEntity2.GetEntity();

		collisionEvent2.entity = checkEntity2.GetEntity();
		collisionEvent2.other = checkEntity1.GetEntity();

		EventManager::GetInstance()->Publish<OnCollisionEnterEvent>(collisionEvent1);
		EventManager::GetInstance()->Publish<OnCollisionEnterEvent>(collisionEvent2);

	}

}

void SliceEngine::MyContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody1.GetUserData()));
	GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody2.GetUserData()));

	auto& colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
	auto& colliderShape2 = checkEntity2.GetComponent<ColliderShape>();

	if (colliderShape1.isTrigger || colliderShape2.isTrigger)
	{
		OnTriggerStayEvent triggerEvent1;
		OnTriggerStayEvent triggerEvent2;

		triggerEvent1.entity = checkEntity1.GetEntity();
		triggerEvent1.other = checkEntity2.GetEntity();

		triggerEvent2.entity = checkEntity2.GetEntity();
		triggerEvent2.other = checkEntity1.GetEntity();

		EventManager::GetInstance()->Publish<OnTriggerStayEvent>(triggerEvent1);
		EventManager::GetInstance()->Publish<OnTriggerStayEvent>(triggerEvent2);
	}
	else
	{
		OnCollisionStayEvent collisionEvent1;
		OnCollisionStayEvent collisionEvent2;

		collisionEvent1.entity = checkEntity1.GetEntity();
		collisionEvent1.other = checkEntity2.GetEntity();

		collisionEvent2.entity = checkEntity2.GetEntity();
		collisionEvent2.other = checkEntity1.GetEntity();

		EventManager::GetInstance()->Publish<OnCollisionStayEvent>(collisionEvent1);
		EventManager::GetInstance()->Publish<OnCollisionStayEvent>(collisionEvent2);

	}

}

void SliceEngine::MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	bodiesInContact.emplace_back(inSubShapePair);
}

std::vector<JPH::SubShapeIDPair> SliceEngine::MyContactListener::GetBodiesInContact() const
{
	 return bodiesInContact; 
}

void SliceEngine::MyContactListener::clearBodiesInContact()
{
	bodiesInContact.clear();
}
