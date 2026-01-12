/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <Sune/SuneBus.h>

namespace Sune
{
    /**
     * @brief Component that visualizes audio frequencies by scaling entities.
     *
     * Requires an AudioPlayerComponent on the same entity. On activation, it adds
     * a visualizer effect to the player and updates entity scales based on frequency
     * data each frame.
     */
    class VisualizerComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(VisualizerComponent, "{F3A8B9E2-5C7D-4E1F-9A3B-6D8E2F4C1A5E}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // AZ::Component interface
        void Activate() override;
        void Deactivate() override;

        // AZ::TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        // Serialized fields
        AZStd::vector<AZ::EntityId> m_visualizerEntities;
        float m_scaleMultiplier = 5.0f;
        float m_minScale = 0.1f;

        // Runtime data
        SoundPlayerId m_playerId;
        PlayerEffectId m_visualizerEffectId;
    };
} // namespace Sune
