/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <AzCore/Component/Component.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>

#include "Clients/Components/Configs/AudioPlayerComponentConfig.h"
#include "Clients/Components/Controllers/AudioPlayerComponentController.h"
#include "Clients/Components/AudioPlayerComponent.h"

namespace Sune
{
    class EditorAudioPlayerComponent
        : public AzToolsFramework::Components::EditorComponentAdapter
            <AudioPlayerComponentController, AudioPlayerComponent, AudioPlayerComponentConfig>
    {
        using Super = AzToolsFramework::Components::EditorComponentAdapter<AudioPlayerComponentController, AudioPlayerComponent, AudioPlayerComponentConfig>;
    public:
        AZ_EDITOR_COMPONENT(EditorAudioPlayerComponent, "{7E5CD8A3-D283-464B-8FAC-EF95CD0875A4}", Super);

        static void Reflect(AZ::ReflectContext* context);

        EditorAudioPlayerComponent() = default;
        explicit EditorAudioPlayerComponent(const AudioPlayerComponentConfig& config);

        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnConfigurationChanged() override;

        AZ::Crc32 OnPlayClicked();
        AZ::Crc32 OnStopClicked();

        bool m_autoPlayEditor = false;
    };
} // Sune