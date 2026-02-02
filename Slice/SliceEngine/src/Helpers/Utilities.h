/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Utilities.h
 author:		Muhammad Hafiz Bin Onn
 email:			b.muhammadhafiz@digipen.edu
 brief:			Utilities for engine usage.
                Feel free to dump helper functions here.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef UTILITIES
#define UTILITIES

#include <glm/gtc/quaternion.hpp>

namespace Utilities
{
    inline glm::quat FromToRotation(glm::vec3 from, glm::vec3 to)
    {
        from = glm::normalize(from);
        to = glm::normalize(to);

        float cosTheta = glm::dot(from, to);

        // Same direction
        if (cosTheta > 0.9999f)
        {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // identity
        }

        // Opposite direction (180 degrees)
        if (cosTheta < -0.9999f)
        {
            // Find an axis perpendicular to 'from'
            glm::vec3 axis = glm::cross(from, glm::vec3(1.0f, 0.0f, 0.0f));
            if (glm::dot(axis, axis) < 0.0001f)
                axis = glm::cross(from, glm::vec3(0.0f, 1.0f, 0.0f));

            axis = glm::normalize(axis);
            return glm::angleAxis(glm::pi<float>(), axis);
        }

        // General case
        glm::vec3 axis = glm::cross(from, to);

        float s = sqrt((1.0f + cosTheta) * 2.0f);
        float invs = 1.0f / s;

        return glm::quat(
            s * 0.5f,
            axis.x * invs,
            axis.y * invs,
            axis.z * invs
        );
    }

    inline void FixMinMax(float& min, float& max)
    {
        if (min > max)
            std::swap(min, max);
    }
}

#endif
