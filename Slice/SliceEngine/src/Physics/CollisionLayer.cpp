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
		for(unsigned int i = 0; i < Layers::NUM_LAYERS; ++i)
		{
			mObjectToBroadPhase[i] = BPLayers::MOVING;
		}
	}

	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return BPLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
	{
		if (inLayer < 0 || inLayer >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return mObjectToBroadPhase[0];
		}
		return mObjectToBroadPhase[inLayer];
	}

	void BPLayerInterfaceImpl::SetObjectToBroadPhaseLayer(JPH::ObjectLayer inLayer, JPH::BroadPhaseLayer inBPLayer)
	{
		if(inLayer < 0 || inLayer >= Layers::NUM_LAYERS)
		{
			JPH_ASSERT(false, "Object layer out of bounds");
			return;
		}
		mObjectToBroadPhase[inLayer] = inBPLayer;
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

		if (bpLayer.GetValue() < 0 || bpLayer.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers()
			|| inLayer2.GetValue() < 0 || inLayer2.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers())
		{
			JPH_ASSERT(false, "Layer out of bounds");
			return false;
		}

		return m_BPCollisionMatrix[bpLayer.GetValue()][inLayer2.GetValue()];

	}

	void ObjectVsBroadPhaseLayerFilterImpl::SetCanCollide(JPH::BroadPhaseLayer inLayer1, JPH::BroadPhaseLayer inLayer2,  bool canCollide)
	{

		if (inLayer1.GetValue() < 0 || inLayer1.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers()
			|| inLayer2.GetValue() < 0 || inLayer2.GetValue() >= m_BPLayerInterface->GetNumBroadPhaseLayers())
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
		for(unsigned int i = 0; i < Layers::NUM_LAYERS; ++i)
		{
			for(unsigned int j = 0; j < Layers::NUM_LAYERS; j++)
			{
				m_CollisionMatrix[i][j] = true;
			}
		}
	}

	// Class that determines if object layer can collide
	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
			if(inObject1 >= Layers::NUM_LAYERS < 0 || inObject1 >= Layers::NUM_LAYERS || inObject2 < 0 || inObject2 >= Layers::NUM_LAYERS)
			{
				JPH_ASSERT(false, "Object layer out of bounds");
				return false;
			}
			return m_CollisionMatrix[inObject1][inObject2];
	}

	void ObjectLayerPairFilterImpl::SetCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2, bool canCollide)
	{
		m_CollisionMatrix[inObject1][inObject2] = canCollide;
		m_CollisionMatrix[inObject2][inObject1] = canCollide;
	}


}

