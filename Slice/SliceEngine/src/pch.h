/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			pch.h
 author:
 email:
 brief:			Pre-compiled headers

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"		// Entire Win32 API...
#include "winsock2.h"	// ...or Winsock alone
#include "ws2tcpip.h"		// getaddrinfo()

// Tell the Visual Studio linker to include the following library in linking.
// Alternatively, we could add this file to the linker command-line parameters,
// but including it in the source code simplifies the configuration.
#pragma comment(lib, "ws2_32.lib")
#undef far
#undef FAR
#undef near
#undef CreateWindowW
#undef CreateWindow
#undef ERROR
#undef PlaySoundW
#undef PlaySound
#undef min
#undef max
#undef name
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

#include <Resource/GUID.h>
#include <Resource/Model.h>
#include "Configuration/ProjectSettings.h"

#endif 