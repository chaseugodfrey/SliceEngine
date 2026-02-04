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
	//BPLayerInterfaceImpl implementation
	BPLayerInterfaceImpl::BPLayerInterfaceImpl()
	{
		for(unsigned int i = 0u; i < Layers::NUM_LAYERS; ++i)
		{
			mObjectToBroadPhase[i] = BPLayers::MOVING; // all set to moving by default
		}
	}

	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return BPLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
	{
		if (static_cast<uint16_t>(inLayer) < 0u || static_cast<uint16_t>(inLayer) >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return mObjectToBroadPhase[0];
		}
		return mObjectToBroadPhase[static_cast<uint16_t>(inLayer)];
	}

	void BPLayerInterfaceImpl::SetObjectToBroadPhaseLayer(JPH::ObjectLayer inLayer, JPH::BroadPhaseLayer inBPLayer)
	{
		if(static_cast<uint16_t>(inLayer) < 0u || static_cast<uint16_t>(inLayer) >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return;
		}
		mObjectToBroadPhase[static_cast<uint16_t>(inLayer)] = inBPLayer;
	}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

	const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
	{
		return "lol"; // leave empty for now
	}

	#endif


	//ObjectVsBroadPhaseLayerFilterImpl implementation

	ObjectVsBroadPhaseLayerFilterImpl::ObjectVsBroadPhaseLayerFilterImpl(const BPLayerInterfaceImpl* inBPLayerInterface)
		: m_BPLayerInterface(inBPLayerInterface)
	{

		JPH::BroadPhaseLayer::Type NON_MOVE = BPLayers::NON_MOVING.GetValue();
		JPH::BroadPhaseLayer::Type MOVE = BPLayers::MOVING.GetValue();

		// Initialize all BP layers[ to collide with each other by default
		m_BPCollisionMatrix[NON_MOVE][NON_MOVE] = false;
		m_BPCollisionMatrix[NON_MOVE][MOVE] = true;
		m_BPCollisionMatrix[MOVE][NON_MOVE] = true;
		m_BPCollisionMatrix[MOVE][MOVE] = true;
	}
	// Class that determines if object layer collides with a broadphaselayer
	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::BroadPhaseLayer inLayer2) const
	{

		JPH::BroadPhaseLayer bpLayer = m_BPLayerInterface->GetBroadPhaseLayer(inObject1);

		if (bpLayer.GetValue() < 0u || bpLayer.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers()
			|| inLayer2.GetValue() < 0u || inLayer2.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers())
		{
			JPH_ASSERT(false, "Layer out of bounds");
			return false;
		}

		return m_BPCollisionMatrix[bpLayer.GetValue()][inLayer2.GetValue()];

	}

	void ObjectVsBroadPhaseLayerFilterImpl::SetCanCollide(JPH::BroadPhaseLayer inLayer1, JPH::BroadPhaseLayer inLayer2,  bool canCollide)
	{

		if (inLayer1.GetValue() < 0u || inLayer1.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers()
			|| inLayer2.GetValue() < 0u || inLayer2.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers())
		{
			JPH_ASSERT(false, "Layer out of bounds");
			return;
		}

		m_BPCollisionMatrix[inLayer1.GetValue()][inLayer2.GetValue()] = canCollide;
		m_BPCollisionMatrix[inLayer2.GetValue()][inLayer1.GetValue()] = canCollide;
	}

	//ObjectLayerPairFilterImpl implementation
	ObjectLayerPairFilterImpl::ObjectLayerPairFilterImpl()
	{
		// Default: all layers collide with all (all bits set)
		for (unsigned int i = 0u; i < Layers::NUM_LAYERS; ++i)
		{
			m_CollisionMask[i] = 0xFFFFFFFF; // Set all 32 bits to 1
		}
		m_CollisionMask[Layers::COLLISION_OFF] = 0u; // COLLISION_OFF collides with nothing
	}

	// Class that determines if object layer can collide
	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		if (inObject1 < 0u || inObject1 >= Layers::NUM_LAYERS || inObject2 < 0u || inObject2 >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return false;
		}

		// COLLISION_OFF layer collides with nothing
		if(inObject1 == Layers::COLLISION_OFF || inObject2 == Layers::COLLISION_OFF)
		{
			return false;
		}
		return (m_CollisionMask[inObject1] & (1u << inObject2)) != 0;
	}

	//let layer manager handle this
	//void ObjectLayerPairFilterImpl::SetCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2, bool canCollide)
	//{
	//	if (inObject1 >= 32 || inObject2 >= 32)
	//	{
	//		JPH_ASSERT(false, "Object layer out of bounds");
	//		return;
	//	}

	//	if (canCollide)
	//	{
	//		m_CollisionMask[inObject1] |= (1u << inObject2);
	//		m_CollisionMask[inObject2] |= (1u << inObject1);
	//	}
	//	else
	//	{
	//		m_CollisionMask[inObject1] &= ~(1u << inObject2);
	//		m_CollisionMask[inObject2] &= ~(1u << inObject1);
	//	}
	//}

	uint32_t ObjectLayerPairFilterImpl::GetCollisionMask(JPH::ObjectLayer inLayer) const
	{
		if (inLayer < 0u || inLayer >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return 0u;
		}
		return m_CollisionMask[inLayer];
	}

	void ObjectLayerPairFilterImpl::SetCollisionMask(JPH::ObjectLayer inLayer, uint32_t mask)
	{
		if (inLayer < 0u || inLayer >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return;
		}

		// COLLISION_OFF layer cannot have its mask changed
		if(inLayer == Layers::COLLISION_OFF)
		{
			return;
		}
		m_CollisionMask[inLayer] = mask;
	}


	bool ObjectLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer) const
	{
		if (inLayer < 0u || inLayer >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return false;
		}

		// COLLISION_OFF layer collides with nothing
		if (inLayer == Layers::COLLISION_OFF)
			return false;

		return layerMask & (1u << inLayer);
	}

	bool BodyFilterIgnore::ShouldCollideLocked(const JPH::Body& inBody) const
	{
		return !inBody.IsSensor();
	}

}

