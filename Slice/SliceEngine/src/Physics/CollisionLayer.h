#ifndef COLLISION_LAYER_H
#define COLLISION_LAYER_H

#include <pch.h>


//std::unique_ptr<JPH::BPLayerInterfaceImpl> broadphase_layer_interface;
//std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadphaseLayerFilter;
//std::unique_ptr<JPH::ObjectLayerPairFilterImpl> objectLayerPairFilter;

namespace SliceEngine
{
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
