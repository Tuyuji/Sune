/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <memory>

#include "AzCore/Math/Vector3.h"
#include "LabSound/core/FloatPoint3D.h"

namespace lab
{
    class AudioBus;
}

namespace Sune
{
    std::shared_ptr<lab::AudioBus> MakeBusFromAudioData(void* fileData, size_t size);
    //Coord system in LabSound is most likely the same as OpenAL according to its docs.
    //So this is a helper function to convert to AL's right-handed Y+ coord system.
    inline lab::FloatPoint3D ToLab(const AZ::Vector3& vector)
    {
        return {vector.GetX(), vector.GetZ(), -vector.GetY()};
    }
}