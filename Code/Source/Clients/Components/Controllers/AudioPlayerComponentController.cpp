/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "AudioPlayerComponentController.h"
#include <AzCore/Serialization/SerializeContext.h>

#include "Sune/PlayerAudioEffect.h"

using namespace Sune;

AudioPlayerComponentController::AudioPlayerComponentController(const AudioPlayerComponentConfig& config)
{
    m_config = config;
}

void AudioPlayerComponentController::Reflect(AZ::ReflectContext* context)
{
    AudioPlayerComponentConfig::Reflect(context);

    if (auto sc = azdynamic_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<AudioPlayerComponentController>()
            ->Version(0)
            ->Field("Config", &AudioPlayerComponentController::m_config);
    }
}

void AudioPlayerComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(Sune::AudioPlayerServiceName);
}

void AudioPlayerComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(Sune::AudioPlayerServiceName);
}

void AudioPlayerComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
}

void AudioPlayerComponentController::SetConfiguration(const AudioPlayerComponentConfig& config)
{
    m_config = config;
}

const AudioPlayerComponentConfig& AudioPlayerComponentController::GetConfiguration() const
{
    return m_config;
}

void AudioPlayerComponentController::Activate(const AZ::EntityComponentIdPair& entityComponentIdPair)
{
    m_entityComponentIdPair = entityComponentIdPair;
    m_playerId = SuneInterface::Get()->CreatePlayer();
    OnConfigurationUpdated();

    AudioPlayerRequestBus::Handler::BusConnect(m_entityComponentIdPair.GetEntityId());
}

void AudioPlayerComponentController::Deactivate()
{
    AudioPlayerRequestBus::Handler::BusDisconnect();
    SuneInterface::Get()->DestroyPlayer(m_playerId);
    m_playerId = SoundPlayerId();
}

void AudioPlayerComponentController::OnConfigurationUpdated()
{
    SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::SetBus, m_config.m_audioBus);
    SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::SetAsset, m_config.m_audioAsset.GetId());
    SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::SetPlayMultiple, m_config.m_playMultiple);
    SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::SetGain, m_config.m_gain);
}

SoundPlayerId AudioPlayerComponentController::GetPlayerId() const
{
    return m_playerId;
}

void AudioPlayerComponentController::Play()
{
    if (m_config.m_loop)
    {
        SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::PlayLooping, -1, 0.0f);
    }
    else
    {
        SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::Play);
    }
}
