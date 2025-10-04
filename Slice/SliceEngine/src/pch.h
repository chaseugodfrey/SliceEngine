#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <filesystem>
#include <fstream>
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
#include <type_traits>
#include <assert.h>
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

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


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


// RTTR
#include <rttr/registration.h>
#include <rttr/type.h>
#include <rttr/property.h>
#include <rttr/method.h>
#include <rttr/variant.h>

// nlohmann json
#include <json.hpp>

#include "Configuration/ProjectSettings.h"

#endif 