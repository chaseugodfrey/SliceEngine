#include <pch.h>
#include "ContactListener.h"

JPH::ValidateResult SliceEngine::MyContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
{
	return JPH::ValidateResult();
}

void SliceEngine::MyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
}

void SliceEngine::MyContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
}

void SliceEngine::MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
}
