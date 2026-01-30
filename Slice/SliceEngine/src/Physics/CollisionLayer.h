/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CollisionLayer.h
 author:		Aloysius Teo
 email:			teo.k@digipen.edu
 brief:			Determines collision layer checks

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef COLLISION_LAYER_H
#define COLLISION_LAYER_H

#include <pch.h>
#include <tuple>


namespace SliceEngine
{
	// ---------------- Layers ----------------
	namespace Layers
	{
		static constexpr JPH::ObjectLayer COLLISION_OFF(32u);
		static constexpr JPH::uint NUM_LAYERS = 33u;
	}

	namespace BPLayers
	{

		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS = 2;
	}

	//Defines the mapping between layer and broadphase layer
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];

	public:
		BPLayerInterfaceImpl();

		JPH::uint GetNumBroadPhaseLayers() const;

		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const;

		void SetObjectToBroadPhaseLayer(JPH::ObjectLayer inLayer, JPH::BroadPhaseLayer inBPLayer);

		#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

		const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;

		#endif
	};

	// Class that determines if object layer collides with a broadphaselayer
	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	private:
		bool m_BPCollisionMatrix[BPLayers::NUM_LAYERS][BPLayers::NUM_LAYERS];
		const BPLayerInterfaceImpl* m_BPLayerInterface;
	public:

		ObjectVsBroadPhaseLayerFilterImpl(const BPLayerInterfaceImpl* inBPLayerInterface);

		bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::BroadPhaseLayer inLayer2) const override;

		void SetCanCollide(JPH::BroadPhaseLayer inLayer1, JPH::BroadPhaseLayer inLayer2, bool canCollide);
	};

	// ObjectLayerPairFilterImpl with bitmask
	class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
	{
	public:
		ObjectLayerPairFilterImpl();

		bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;

		//void SetCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2, bool canCollide);

		uint32_t GetCollisionMask(JPH::ObjectLayer inLayer) const;

		void SetCollisionMask(JPH::ObjectLayer inLayer, uint32_t mask);

		//might do this but layer manager handles it i think
		//void EnableCollisionWithLayer(JPH::ObjectLayer inLayer, JPH::ObjectLayer layerToEnable);

	private:
		uint32_t m_CollisionMask[Layers::NUM_LAYERS];
	};

	class ObjectLayerFilterImpl final : public JPH::ObjectLayerFilter
	{
	public:
		ObjectLayerFilterImpl(uint32_t mask) : layerMask(mask) {}

		bool ShouldCollide(JPH::ObjectLayer inLayer) const override;

	private:
		uint32_t layerMask;
	};

	//class BodyFilterIgnore final : public JPH::BodyFilter
	//{
	//public:
	//	bool ShouldCollide(JPH::ObjectLayer inLayer) const override;
	//private:



	//};


}



#endif
