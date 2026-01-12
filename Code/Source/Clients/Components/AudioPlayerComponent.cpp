/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "AudioPlayerComponent.h"

using namespace Sune;

void AudioPlayerComponent::Reflect(AZ::ReflectContext* context)
{
    Super::Reflect(context);
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<AudioPlayerComponent, Super>()
            ->Version(0);
    }
}

AudioPlayerComponent::AudioPlayerComponent() = default;

AudioPlayerComponent::AudioPlayerComponent(const AudioPlayerComponentConfig& config)
    : Super(config)
{
}

void AudioPlayerComponent::Activate()
{
    Super::Activate();
    if (m_controller.GetConfiguration().m_autoPlay)
    {
        m_controller.Play();
    }
}
