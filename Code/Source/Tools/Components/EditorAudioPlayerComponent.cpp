/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "EditorAudioPlayerComponent.h"

using namespace Sune;

void EditorAudioPlayerComponent::Reflect(AZ::ReflectContext* context)
{
    Super::Reflect(context);
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<EditorAudioPlayerComponent, Super>()
            ->Version(0)
            ->Field("autoPlayEditor", &EditorAudioPlayerComponent::m_autoPlayEditor)
        ;

        using namespace AZ::Edit;
        auto ec = sc->GetEditContext();
        if (!ec)
            return;

        ec->Class<EditorAudioPlayerComponent>("Audio Player Component", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(AZ::Edit::Attributes::Category, "Sune")
            ->Attribute(Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
            ->Attribute(Attributes::AutoExpand, true)
            ->UIElement(UIHandlers::Button, "", "Plays the audio asset.")
                ->Attribute(Attributes::ChangeNotify, &EditorAudioPlayerComponent::OnPlayClicked)
                ->Attribute(Attributes::ButtonText, "Play")
            ->UIElement(UIHandlers::Button, "", "Stops the audio asset.")
                ->Attribute(Attributes::ChangeNotify, &EditorAudioPlayerComponent::OnStopClicked)
                ->Attribute(Attributes::ButtonText, "Stop")
            ->DataElement(UIHandlers::Default, &EditorAudioPlayerComponent::m_autoPlayEditor, "Auto Play In Editor", "Whether to play the audio automatically in editor.")
                ->Attribute(Attributes::ChangeNotify, &EditorAudioPlayerComponent::OnConfigurationChanged)
            ;

        ec->Class<AudioPlayerComponentController>("Audio Player Controller", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(Attributes::AutoExpand, true)
            ->DataElement(Attributes::Visibility, &AudioPlayerComponentController::m_config, "Configuration", "The configuration for the audio player component.")
            ->Attribute(Attributes::Visibility, PropertyVisibility::ShowChildrenOnly);

        ec->Class<AudioPlayerComponentConfig>("Audio Player Component Config", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
            ->Attribute(Attributes::Visibility, PropertyVisibility::Hide)
            ->DataElement(UIHandlers::Default, &AudioPlayerComponentConfig::m_audioAsset, "Audio Asset", "The audio asset to play.")
            ->DataElement(UIHandlers::Default, &AudioPlayerComponentConfig::m_audioBus, "Audio Bus", "The bus to output the audio to.")
            ->DataElement(UIHandlers::Default, &AudioPlayerComponentConfig::m_playMultiple, "Play multiple", "Allows multiple instances of the audio to play at the same time.")
            ->DataElement(UIHandlers::Slider, &AudioPlayerComponentConfig::m_gain, "Gain", "The volume gain.")
                ->Attribute(Attributes::Min, 0.0f)
                ->Attribute(Attributes::Max, 3.0f)
                ->Attribute(Attributes::Step, 0.1f)
            ->DataElement(UIHandlers::Default, &AudioPlayerComponentConfig::m_loop, "Loop", "Whether to loop the audio.")
            ->DataElement(UIHandlers::Default, &AudioPlayerComponentConfig::m_autoPlay, "Auto Play", "Whether to play the audio automatically.")
        ;

    }
}

EditorAudioPlayerComponent::EditorAudioPlayerComponent(const AudioPlayerComponentConfig& config)
    : Super(config)
{
}

void EditorAudioPlayerComponent::Activate()
{
    Super::Activate();
    if (m_autoPlayEditor)
    {
        m_controller.Play();
    }
}

void EditorAudioPlayerComponent::Deactivate()
{
    Super::Deactivate();
}

AZ::u32 EditorAudioPlayerComponent::OnConfigurationChanged()
{
    m_controller.OnConfigurationUpdated();
    if (m_autoPlayEditor)
    {
        SoundPlayerRequestBus::Event(m_controller.m_playerId, &SoundPlayerRequestBus::Events::StopAll);
        m_controller.Play();
    }
    return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
}

AZ::Crc32 EditorAudioPlayerComponent::OnPlayClicked()
{
    m_controller.Play();
    return AZ::Edit::PropertyRefreshLevels::None;
}

AZ::Crc32 EditorAudioPlayerComponent::OnStopClicked()
{
    SoundPlayerRequestBus::Event(m_controller.m_playerId, &SoundPlayerRequestBus::Events::StopAll);
    return AZ::Edit::PropertyRefreshLevels::None;
}
