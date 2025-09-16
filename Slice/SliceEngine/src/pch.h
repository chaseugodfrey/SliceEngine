#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include <chrono>
#include <memory>

#include <string>
#include <vector>
#include <array>
#include <deque>
#include <queue>
#include <map>

#include <algorithm>
#include <functional>
#include <algorithm>
#include <type_traits>

#include <thread>

#include <math.h>

// Logger
#include "Logger/Logger.h"

// Common Types
#include "Vectors/Vector2.h"
//#include "../Vectors/Vector3.h"

// external libs

// glfw
 #include <GL/glew.h>
 #include <glfw3.h>

// fmod
// #include <fmod.hpp>
// #include <fmod_common.h>
// #include <fmod_studio.hpp>
// #include <fmod_studio_common.h>

//// imgui
//#include <imgui.h>
//#include <imgui_internal.h>
//#include <imgui_impl_glfw.h>
//#include <imgui_impl_opengl3.h>
//#include <ImGuizmo.h>

// jolt
 #include <Jolt/Jolt.h>
 #include <Jolt/RegisterTypes.h>
 #include <Jolt/Core/Factory.h>
 #include <Jolt/Core/TempAllocator.h>
 #include <Jolt/Core/JobSystemThreadPool.h>
 #include <Jolt/Physics/PhysicsSettings.h>
 #include <Jolt/Physics/PhysicsSystem.h>
 #include <Jolt/Physics/Collision/Shape/BoxShape.h>
 #include <Jolt/Physics/Collision/Shape/SphereShape.h>
 #include <Jolt/Physics/Body/BodyCreationSettings.h>
 #include <Jolt/Physics/Body/BodyActivationListener.h>

#endif 