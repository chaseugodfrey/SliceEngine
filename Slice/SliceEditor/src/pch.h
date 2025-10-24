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
#include <stack>
#include <deque>
#include <queue>
#include <map>
#include <variant>

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

#include <algorithm>
#include <functional>
#include <type_traits>

#include <thread>

#include <math.h>

// Logger
#include "Logger/Logger.h"


// Common Types
//#include "Vectors/Vector2.h"
//#include "../Vectors/Vector3.h"

// external libs

// glfw
 #include <glew.h>
 #include <glfw3.h>

// fmod
// #include <fmod.hpp>
// #include <fmod_common.h>
// #include <fmod_studio.hpp>
// #include <fmod_studio_common.h>

#include <glm/glm.hpp>

// imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <ImGuizmo.h>
#include <imgui_node_editor.h>

// filewatcher
#include <../thirdparty/filewatch/FileWatcher.h>
#undef near
#undef far
#undef ERROR

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
#include <entt.hpp>

#include <json.hpp>

// Engine
#include <Resource/GUID.h>
#include <Core/Core.h>
#include <Core/EventManager.h>
#include <Core/Events.h>

// Editor
#include <EditorCommonTypes.h>
#include <EditorUtilities.h>
#include <History/Commands.h>
#include <Core/Registry.h>
#include <Core/EditorEvents.h>

#endif 