/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <AzCore/Component/Component.h>
#include <AzFramework/Components/ComponentAdapter.h>

#include "Configs/AudioPlayerComponentConfig.h"
#include "Controllers/AudioPlayerComponentController.h"

namespace Sune
{
    class AudioPlayerComponent
        : public AzFramework::Components::ComponentAdapter<AudioPlayerComponentController, AudioPlayerComponentConfig>
    {
        using Super = AzFramework::Components::ComponentAdapter<AudioPlayerComponentController, AudioPlayerComponentConfig>;
    public:
        AZ_COMPONENT(AudioPlayerComponent, "{2B3FADD5-5FAA-4FA7-AF28-E55417268723}", Super);

        static void Reflect(AZ::ReflectContext* context);

        AudioPlayerComponent();
        explicit AudioPlayerComponent(const AudioPlayerComponentConfig& config);

        void Activate() override;
    };
} // Sune