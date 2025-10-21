#ifndef CONTACTLISTENER_H
#define CONTACTLISTENER_H

#include <pch.h>

namespace SliceEngine
{

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{

		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

		virtual void OnContactAdded(const  JPH::Body& inBody1, const  JPH::Body& inBody2, const  JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactPersisted(const  JPH::Body& inBody1, const  JPH::Body& inBody2, const  JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

	};


}



#endif
