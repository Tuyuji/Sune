/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "VisualizerComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Sune/AudioPlayerBus.h>
#include <Sune/Effects/VisualizerBus.h>

using namespace Sune;

void VisualizerComponent::Reflect(AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<VisualizerComponent, AZ::Component>()
            ->Version(1)
            ->Field("VisualizerEntities", &VisualizerComponent::m_visualizerEntities)
            ->Field("ScaleMultiplier", &VisualizerComponent::m_scaleMultiplier)
            ->Field("MinScale", &VisualizerComponent::m_minScale);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<VisualizerComponent>("Audio Visualizer", "Visualizes audio frequencies by scaling entities")
                ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::Category, "Sune")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                ->DataElement(AZ::Edit::UIHandlers::Default, &VisualizerComponent::m_visualizerEntities,
                    "Visualizer Entities", "Entities to scale based on frequency data (low to high)")
                ->DataElement(AZ::Edit::UIHandlers::Slider, &VisualizerComponent::m_scaleMultiplier,
                    "Scale Multiplier", "Amplification factor for frequency values")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.1f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.0f)
                ->DataElement(AZ::Edit::UIHandlers::Slider, &VisualizerComponent::m_minScale,
                    "Min Scale", "Minimum scale to prevent invisible entities")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.01f)
                    ->Attribute(AZ::Edit::Attributes::Max, 2.0f);
        }
    }
}

void VisualizerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(AudioPlayerServiceName);
}

void VisualizerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC_CE("AudioVisualizerService"));
}

void VisualizerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC_CE("AudioVisualizerService"));
}

void VisualizerComponent::Activate()
{
    // Get the player ID from the AudioPlayerComponent on this entity
    AudioPlayerRequestBus::EventResult(m_playerId, GetEntityId(), &AudioPlayerRequestBus::Events::GetPlayerId);

    if (!m_playerId.IsValid())
    {
        AZ_Error("Sune", false, "VisualizerComponent requires an AudioPlayerComponent on the same entity.");
        return;
    }

    // Add the visualizer effect to the player
    SoundPlayerRequestBus::EventResult(
        m_visualizerEffectId,
        m_playerId,
        &SoundPlayerRequestBus::Events::AddEffect,
        "visualizer");

    if (!m_visualizerEffectId.IsValid())
    {
        AZ_Error("Sune", false, "Failed to add visualizer effect to player.");
        return;
    }

    // Configure the visualizer effect
    VisualizerEffectRequestBus::Event(m_visualizerEffectId, &VisualizerEffectRequests::SetVisualizerEntities, m_visualizerEntities);
    VisualizerEffectRequestBus::Event(m_visualizerEffectId, &VisualizerEffectRequests::SetScaleMultiplier, m_scaleMultiplier);
    VisualizerEffectRequestBus::Event(m_visualizerEffectId, &VisualizerEffectRequests::SetMinScale, m_minScale);

    // Connect to tick bus to update visualization each frame
    AZ::TickBus::Handler::BusConnect();
}

void VisualizerComponent::Deactivate()
{
    AZ::TickBus::Handler::BusDisconnect();

    // Remove the visualizer effect from the player
    if (m_visualizerEffectId.IsValid())
    {
        SoundPlayerRequestBus::Event(m_playerId, &SoundPlayerRequestBus::Events::RemoveEffect, m_visualizerEffectId);
        m_visualizerEffectId = PlayerEffectId();
    }

    m_playerId = SoundPlayerId();
}

void VisualizerComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
{
    if (m_visualizerEffectId.IsValid())
    {
        // Update the visualization each frame
        VisualizerEffectRequestBus::Event(m_visualizerEffectId, &VisualizerEffectRequests::UpdateVisualization);
    }
}
