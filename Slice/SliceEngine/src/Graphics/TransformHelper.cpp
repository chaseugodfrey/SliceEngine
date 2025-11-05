/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			TransformHelper.cpp
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Helper functions to convert vec3 to quaternions and vice versa

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "TransformHelper.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#define M_PI 3.14159265358979323846264338327950288


namespace SliceEngine
{
	glm::quat Vec3ToQuat(const glm::vec3& in)
	{
        glm::mat4x4 rotMatrix = glm::eulerAngleXYZ(glm::radians(in.x), glm::radians(in.y), glm::radians(in.z));
        return glm::quat_cast(rotMatrix);
	}

	glm::vec3 QuatToVec3(glm::quat q)
	{
        q = glm::normalize(q);
        glm::vec3 result;

        // roll (x-axis rotation)
        double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
        double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
        result.x = static_cast<float>(std::atan2(sinr_cosp, cosr_cosp)); // atan2 handles full circle

        // pitch (y-axis rotation)
        double sinp = 2.0 * (q.w * q.y - q.z * q.x);
        if (std::abs(sinp) >= 1)
            result.y = static_cast<float>(std::copysign(M_PI / 2.0, sinp)); // Handle gimbal lock (use 90 deg)
        else
            result.y = static_cast<float>(std::asin(sinp)); // Standard arcsin

        // yaw (z-axis rotation)
        double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
        result.z = static_cast<float>(std::atan2(siny_cosp, cosy_cosp));

        // Convert from radians to degrees
        return glm::degrees(result);
    }
}