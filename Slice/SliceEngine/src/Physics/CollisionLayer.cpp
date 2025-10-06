/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CollisionLayer.cpp
 author:		Aloysius Teo
 email:			teo.k@digipen.edu
 brief:			Determines collision layer checks

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "CollisionLayer.h"

namespace SliceEngine 

{

	BPLayerInterfaceImpl::BPLayerInterfaceImpl()
	{
		mObjectToBroadPhase[Layers::NON_MOVING] = BPLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BPLayers::MOVING;
	}

	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return BPLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
	{
		return mObjectToBroadPhase[inLayer];
	}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

	const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
	{
		return "lol"; // leave empty for now
	}

	#endif
	// Class that determines if object layer collides with a broadphaselayer
	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::BroadPhaseLayer inLayer2) const
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BPLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}

	// Class that determines if object layer can collide
	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}


}

