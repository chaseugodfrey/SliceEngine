#include <pch.h>
#include "ContactListener.h"
#include "../ECS/GOFactory.h"
#include "../Core/Core.h"
#include "../Systems/FramerateManager.h"

//JPH::ValidateResult SliceEngine::MyContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
//{
//	return JPH::ValidateResult();
//}

//static timer for on contact persist need to figure time to publish event everytime
//first add put it in the container and publish event
//check if its in the container before adding, if in container send an event every few second time has yet to be decided
//when a pair is added to on contact remove remove it from on contact persist
//set time to 0 when nothing is in the container or when timer hits >0.5f
namespace SliceEngine
{

	void SliceEngine::MyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		// Use world space to see actual collision position
		//JPH::RVec3 contactPoint = inManifold.GetWorldSpaceContactPointOn1(0); // First contact point in world space
		////std::cout << "Collision at: (" << contactPoint.GetX() << ", "
		//	<< contactPoint.GetY() << ", " << contactPoint.GetZ() << ")" << std::endl;

		// Also log body positions to compare
		////std::cout << "Body1 pos: (" << inBody1.GetPosition().GetX() << ", "
		//	<< inBody1.GetPosition().GetY() << ", " << inBody1.GetPosition().GetZ() << ")" << std::endl;
		////std::cout << "Body2 pos: (" << inBody2.GetPosition().GetX() << ", "
		//	<< inBody2.GetPosition().GetY() << ", " << inBody2.GetPosition().GetZ() << ")" << std::endl;
		if (!mIsLastStep)
			return;

		GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody1.GetUserData()));
		GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody2.GetUserData()));

		//int test1 = static_cast<int>(inBody1.GetUserData());
		//int test2 = static_cast<int>(inBody2.GetUserData())
		if (!checkEntity1.HasComponent<ColliderShape>() || !checkEntity2.HasComponent<ColliderShape>())
		{
			std::string errorMsg = "Contact added between ";
			errorMsg += std::to_string((unsigned int)checkEntity1.GetEntity());
			errorMsg += " and ";
			errorMsg += std::to_string((unsigned int)checkEntity2.GetEntity());
			SLICE_LOG_ERROR(errorMsg);
			return;
		}

		auto& colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
		auto& colliderShape2 = checkEntity2.GetComponent<ColliderShape>();
		////std::cout << inBody1.GetUserData() << " and " << inBody2.GetUserData() << std::endl;
		////std::cout << (uint32_t)checkEntity1.GetEntity() << " and " << (uint32_t)checkEntity2.GetEntity()<< std::endl;


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
		//GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody1.GetUserData()));
		//GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody2.GetUserData()));

		//timer = collisionPairs.size() == 0 ? 0.0f : timer + Core::GetInstance()->GetFramerateManager()->getFixedDeltaTime();

		//std::pair<GameObject, GameObject> orderedPair = MakeOrderedPair(checkEntity1, checkEntity2);
		//collisionPairs.insert(orderedPair);

		//if(timer >= timeBetweenEvents)
		//{
		//	PublishCollisionPersistEvents();
		//	timer = 0.0f;
		//}
		// Top is for my reference if im using a container to store the pairs and publish events every few seconds, but for now we will just publish the event every time on contact persist is called
		if (!mIsLastStep)
			return;

		GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody1.GetUserData()));
		GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(inBody2.GetUserData()));

		//int test1 = static_cast<int>(inBody1.GetUserData());
		//int test2 = static_cast<int>(inBody2.GetUserData())
		if (!checkEntity1.HasComponent<ColliderShape>() || !checkEntity2.HasComponent<ColliderShape>())
		{
			std::string errorMsg = "Contact stay event between ";
			errorMsg += std::to_string((unsigned int)checkEntity1.GetEntity());
			errorMsg += " and ";
			errorMsg += std::to_string((unsigned int)checkEntity2.GetEntity());
			SLICE_LOG_ERROR(errorMsg);
			return;
		}

		auto& colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
		auto& colliderShape2 = checkEntity2.GetComponent<ColliderShape>();
		////std::cout << inBody1.GetUserData() << " and " << inBody2.GetUserData() << std::endl;
		////std::cout << (uint32_t)checkEntity1.GetEntity() << " and " << (uint32_t)checkEntity2.GetEntity()<< std::endl;


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

	void SliceEngine::MyContactListener::clearCollisionsPairs()
	{
		collisionPairs.clear();
	}

	std::set<std::pair<GameObject, GameObject>> MyContactListener::GetCollisionPairs() const
	{
		return collisionPairs;
	}

	std::pair<GameObject, GameObject> MyContactListener::MakeOrderedPair(const GameObject& ent1, const GameObject& ent2)
	{
		return (ent1 < ent2) ? std::make_pair(ent1, ent2) : std::make_pair(ent2, ent1);
	}

	void MyContactListener::PublishCollisionPersistEvents()
	{
		for (const auto& pair : collisionPairs)
		{

			auto& colliderShape1 = pair.first.GetComponent<ColliderShape>();
			auto& colliderShape2 = pair.second.GetComponent<ColliderShape>();

			if (colliderShape1.isTrigger || colliderShape2.isTrigger)
			{
				OnTriggerStayEvent triggerEvent1;
				OnTriggerStayEvent triggerEvent2;

				triggerEvent1.entity = pair.first.GetEntity();
				triggerEvent1.other = pair.second.GetEntity();

				triggerEvent2.entity = pair.second.GetEntity();
				triggerEvent2.other = pair.first.GetEntity();

				//EventManager::GetInstance()->Publish<OnTriggerStayEvent>(triggerEvent1);
				//EventManager::GetInstance()->Publish<OnTriggerStayEvent>(triggerEvent2);
			}
			else
			{
				OnCollisionStayEvent collisionEvent1;
				OnCollisionStayEvent collisionEvent2;

				collisionEvent1.entity = pair.first.GetEntity();
				collisionEvent1.other = pair.second.GetEntity();

				collisionEvent2.entity = pair.second.GetEntity();
				collisionEvent2.other = pair.first.GetEntity();

				//EventManager::GetInstance()->Publish<OnCollisionStayEvent>(collisionEvent1);
				//EventManager::GetInstance()->Publish<OnCollisionStayEvent>(collisionEvent2);

			}
		}
	}

	void MyContactListener::RemoveContactPair(const std::pair<GameObject, GameObject>& contactPair)
	{
		auto it = collisionPairs.find(contactPair);
		if (it != collisionPairs.end())
		{
			collisionPairs.erase(it);
		}
		else
		{
			SLICE_LOG_ERROR("Contact listener remove pair should not come here");
		}
	}

	void MyContactListener::SetLastStep(bool isLast)
	{
		mIsLastStep = isLast;
	}

	bool MyContactListener::GetLastStep() const
	{
		return mIsLastStep;
	}

	void SliceEngine::MyContactListener::clearBodiesInContact()
	{
		bodiesInContact.clear();
	}

}

