#ifndef COLLISION_LAYER_H
#define COLLISION_LAYER_H

#include <pch.h>


namespace SliceEngine
{
	// ---------------- Layers ----------------
	namespace Layers
	{
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING = 1;
		static constexpr JPH::uint NUM_LAYERS = 2;
	}

	namespace BPLayers
	{
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS = 2;
	}

	//Defines the mapping between layer and broadphase layer
	class BPLayerInterfaceImpl final : JPH::BroadPhaseLayerInterface
	{
	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS]; // will switch to switch statement and remove this once i know how many layers I need

	public:
		BPLayerInterfaceImpl();

		JPH::uint GetNumBroadPhaseLayers();

		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer);

		#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

		const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;

		#endif
	};

	// Class that determines if object layer collides with a broadphaselayer
	class ObjectVsBroadPhaseLayerFilterImpl final : JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:

		bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::BroadPhaseLayer inLayer2) const override;
	};

	// Class that determines if object layer can collide
	class ObjectLayerPairFilterImpl final : JPH::ObjectLayerPairFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;

	};

}



#endif
