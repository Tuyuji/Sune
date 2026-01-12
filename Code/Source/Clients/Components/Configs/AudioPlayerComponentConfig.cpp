/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "AudioPlayerComponentConfig.h"
#include <AzCore/Serialization/SerializeContext.h>

using namespace Sune;

void AudioPlayerComponentConfig::Reflect(AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Enum<lab::PannerNode::DistanceModel>()
            ->Value("Linear", lab::PannerNode::DistanceModel::LINEAR_DISTANCE)
            ->Value("Inverse", lab::PannerNode::DistanceModel::INVERSE_DISTANCE)
            ->Value("Exponential", lab::PannerNode::DistanceModel::EXPONENTIAL_DISTANCE);

        sc->Class<AudioPlayerComponentConfig>()
            ->Version(0)
            ->Field("audioAsset", &AudioPlayerComponentConfig::m_audioAsset)
            ->Field("audioBus", &AudioPlayerComponentConfig::m_audioBus)
            ->Field("playMultiple", &AudioPlayerComponentConfig::m_playMultiple)
            ->Field("gain", &AudioPlayerComponentConfig::m_gain)
            ->Field("loop", &AudioPlayerComponentConfig::m_loop)
            ->Field("autoPlay", &AudioPlayerComponentConfig::m_autoPlay)
        ;
    }
}
