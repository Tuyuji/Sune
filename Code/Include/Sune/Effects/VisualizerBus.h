/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/PlayerAudioEffect.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/std/containers/vector.h>

namespace Sune
{
    // Request bus for configuring the visualizer effect
    class VisualizerEffectRequests
    {
    public:
        AZ_RTTI(VisualizerEffectRequests, "{8B2A4E5C-9D3F-4E6A-B8C1-2F5E7D9A3B6C}");
        virtual ~VisualizerEffectRequests() = default;

        // Set the entities to visualize (each entity represents a frequency band)
        virtual void SetVisualizerEntities(const AZStd::vector<AZ::EntityId>& entities) = 0;
        virtual AZStd::vector<AZ::EntityId> GetVisualizerEntities() const = 0;

        // Visualizer-specific scaling parameters
        virtual void SetScaleMultiplier(float multiplier) = 0;
        virtual float GetScaleMultiplier() const = 0;

        virtual void SetMinScale(float minScale) = 0;
        virtual float GetMinScale() const = 0;

        // Manual update - call this per frame to update entity scales
        virtual void UpdateVisualization() = 0;
    };

    using VisualizerEffectRequestBus = AZ::EBus<VisualizerEffectRequests, PlayerEffectBusTraits>;

} // Sune