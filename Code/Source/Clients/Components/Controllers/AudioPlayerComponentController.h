/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Component/Component.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "Clients/Components/Configs/AudioPlayerComponentConfig.h"
#include "Sune/SuneBus.h"
#include "Sune/AudioPlayerBus.h"

namespace Sune
{
    class AudioPlayerComponentController
        : protected AudioPlayerRequestBus::Handler
    {
    public:
        AZ_RTTI(AudioPlayerComponentController, "{CF17402A-3924-4C46-9756-8BAC8B27C1BB}");

        AudioPlayerComponentController() = default;
        explicit AudioPlayerComponentController(const AudioPlayerComponentConfig& config);
        virtual ~AudioPlayerComponentController() = default;

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        void SetConfiguration(const AudioPlayerComponentConfig& config);

        [[nodiscard]] const AudioPlayerComponentConfig& GetConfiguration() const;

        void Activate(const AZ::EntityComponentIdPair& entityComponentIdPair);

        void Deactivate();

        void OnConfigurationUpdated();

        SoundPlayerId GetPlayerId() const override;

        //Handles playing based on the current config
        void Play();
    private:
        friend class EditorAudioPlayerComponent;
        friend class AudioPlayerComponent;
        AZ::EntityComponentIdPair m_entityComponentIdPair;
        AudioPlayerComponentConfig m_config;

        SoundPlayerId m_playerId;
    };
} // Sune