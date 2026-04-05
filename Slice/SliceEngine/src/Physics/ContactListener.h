#ifndef CONTACTLISTENER_H
#define CONTACTLISTENER_H

#include <pch.h>
#include "ECS/GameObject.h"


namespace SliceEngine
{
	class MyContactListener : public JPH::ContactListener
	{

		//virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

		virtual void OnContactAdded(const  JPH::Body& inBody1, const  JPH::Body& inBody2, const  JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactPersisted(const  JPH::Body& inBody1, const  JPH::Body& inBody2, const  JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

	private:

		std::vector<JPH::SubShapeIDPair> bodiesInContact;
		std::set<std::pair<GameObject, GameObject>> collisionPairs;
		float timer = 0.0f;
		const float timeBetweenEvents = 1.0f;
		bool mIsLastStep = false;

	public:

		void clearBodiesInContact();

		std::vector<JPH::SubShapeIDPair> GetBodiesInContact() const;

		void clearCollisionsPairs();

		std::set<std::pair<GameObject, GameObject>> GetCollisionPairs() const;

		std::pair<GameObject, GameObject> MakeOrderedPair(const GameObject& ent1, const GameObject& ent2);

		void PublishCollisionPersistEvents();

		void RemoveContactPair(const std::pair<GameObject, GameObject>& contactPair);

		void SetLastStep(bool isLast);

		bool GetLastStep() const;

	};


}



#endif
