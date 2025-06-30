//
// Created by tyler on 6/29/25.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "singleton.h"

namespace GyroEngine::XR
{
    enum class XRJoint : uint32_t
    {
        Invalid = 0,

        // XR core
        Root,
        Hips,
        SpineLower,
        SpineMiddle,
        SpineUpper,
        Chest,
        Neck,
        Head,

        // XR left arm
        LeftShoulder,
        LeftUpperArm,
        LeftLowerArm,
        LeftHand,

        // XR right arm
        RightShoulder,
        RightUpperArm,
        RightLowerArm,
        RightHand,

        // XR left leg
        LeftUpperLeg,
        LeftLowerLeg,
        LeftAnkle,
        LeftFoot,
        LeftToes,

        // XR right leg
        RightUpperLeg,
        RightLowerLeg,
        RightAnkle,
        RightFoot,
        RightToes,
    };

    struct XRPose
    {
        bool tracking = false;
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::quat orientation = {1.0f, 0.0f, 0.0f, 0.0f};
    };

    class XReality : public Utils::ISingleton<XReality>
    {

    };

}
