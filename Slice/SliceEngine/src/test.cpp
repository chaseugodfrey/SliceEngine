#include <pch.h>
#include <iostream>
#include <stdarg.h>

#include "test.h"

using namespace JPH;

// ---------------- Layers ----------------
namespace Layers
{
    static constexpr ObjectLayer NON_MOVING = 0;
    static constexpr ObjectLayer MOVING = 1;
    static constexpr uint NUM_LAYERS = 2;
}

namespace BPLayers
{
    static constexpr BroadPhaseLayer NON_MOVING(0);
    static constexpr BroadPhaseLayer MOVING(1);
    static constexpr uint NUM_LAYERS = 2;
}

// ---------------- Interfaces ----------------
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
    virtual uint GetNumBroadPhaseLayers() const override { return BPLayers::NUM_LAYERS; }

    virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
    {
        switch (inLayer)
        {
        case Layers::NON_MOVING: return BPLayers::NON_MOVING;
        case Layers::MOVING: return BPLayers::MOVING;
        default: return BPLayers::NON_MOVING;
        }
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
        switch (inLayer.GetValue())
        {
        case 0: return "NON_MOVING";
        case 1: return "MOVING";
        default: return "UNKNOWN";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer, BroadPhaseLayer) const override { return true; }
};

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer, ObjectLayer) const override { return true; }
};

// ---------------- Trace Function ----------------
//static void TraceImpl(const char* inFMT, ...)
//{
//    va_list list;
//    va_start(list, inFMT);
//    vprintf(inFMT, list);
//    va_end(list);
//}

// ---------------- Main ----------------
int test()
{
    // ---- REQUIRED GLOBAL INIT ----
    RegisterDefaultAllocator();   // Install memory allocator
    //Trace = TraceImpl;            // Debug trace callback

    // ---- ALLOCATOR + JOB SYSTEM ----
    TempAllocatorImpl tempAllocator(10 * 1024 * 1024); // 10 MB
    JobSystemThreadPool jobSystem(cMaxPhysicsJobs, cMaxPhysicsBarriers, 4);

    // ---- FACTORY + TYPES ----
    Factory::sInstance = new Factory();
    RegisterTypes();

    // ---- PHYSICS SYSTEM ----
    PhysicsSystem physicsSystem;
    BPLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectLayerPairFilter;

    physicsSystem.Init(1024, 0, 1024, 1024,
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectLayerPairFilter);

    BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();

    // ---- FLOOR ----
    BoxShapeSettings floorShapeSettings(Vec3(50.0f, 1.0f, 50.0f));
    ShapeRefC floorShape = floorShapeSettings.Create().Get();
    BodyCreationSettings floorSettings(floorShape,
        RVec3(0.0f, -1.0f, 0.0f),
        Quat::sIdentity(),
        EMotionType::Static,
        Layers::NON_MOVING);
    bodyInterface.CreateAndAddBody(floorSettings, EActivation::DontActivate);

    // ---- FALLING BOX ----
    BoxShapeSettings boxShapeSettings(Vec3(0.5f, 0.5f, 0.5f));
    ShapeRefC boxShape = boxShapeSettings.Create().Get();
    BodyCreationSettings boxSettings(boxShape,
        RVec3(0.0f, 5.0f, 0.0f),
        Quat::sIdentity(),
        EMotionType::Dynamic,
        Layers::MOVING);
    BodyID boxID = bodyInterface.CreateAndAddBody(boxSettings, EActivation::Activate);

    //std::pair<entityid, BoxID>
    //std::map<entityid, BoxID>

    std::cout << "Simulation start...\n";

    // ---- SIMULATE ----
    const float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 300; ++i) // 5 seconds
    {
        physicsSystem.Update(deltaTime, 1, &tempAllocator, &jobSystem);

        RVec3 pos = bodyInterface.GetPosition(boxID);
        std::cout << "t=" << (i + 1) * deltaTime << "s, Box Y=" << pos.GetY() << "\n";
    }

    // ---- CLEANUP ----
    UnregisterTypes();
    delete Factory::sInstance;
    Factory::sInstance = nullptr;

    return 0;
}
