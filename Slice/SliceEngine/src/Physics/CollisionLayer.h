#ifndef COLLISION_LAYER_H
#define COLLISION_LAYER_H

#include <pch.h>


//std::unique_ptr<JPH::BPLayerInterfaceImpl> broadphase_layer_interface;
//std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadphaseLayerFilter;
//std::unique_ptr<JPH::ObjectLayerPairFilterImpl> objectLayerPairFilter;

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


	class BPLayerInterfaceImpl final : JPH::BroadPhaseLayerInterface
	{

	};

	class ObjectVsBroadPhaseLayerFilterImpl final : JPH::ObjectVsBroadPhaseLayerFilter
	{

	};

	class ObjectLayerPairFilter final : JPH::ObjectLayerPairFilter
	{

	};

}



#endif
