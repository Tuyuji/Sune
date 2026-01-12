/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "SuneBus.h"
#include <LabSound/core/PannerNode.h>

namespace lab
{
    class AudioContext;
    class AudioNode;
}

namespace Sune
{
    enum class PlayerEffectOrder : int32_t
    {
        Source = 0, // Not used by effects, represents the audio source
        Filtering = 100, //eq, filters
        Dynamics = 200, // compression limiters
        TimeBased = 300, // delay, reverb, chorus
        Spatializer = 400, // HRTF, panning
        Utility = 500, // gain, meters, analyzers
        Output = 1000 // not used by effects, represents final output
    };

    class PlayerEffectRequests
    {
    public:
        AZ_RTTI(PlayerEffectRequests, "{0CEE56BD-A993-406D-B465-E2EE1FB85EBB}");
        virtual ~PlayerEffectRequests() = default;

        virtual const char* GetEffectName() const = 0;
        virtual PlayerEffectOrder GetProcessingOrder() const = 0;

        virtual void SetEnabled(bool enabled) = 0;
        virtual bool IsEnabled() const = 0;
    };

    class PlayerEffectBusTraits
       : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = PlayerEffectId;
    };

    using PlayerEffectRequestBus = AZ::EBus<PlayerEffectRequests, PlayerEffectBusTraits>;

    class PlayerEffectImGuiRequests
    {
    public:
        virtual void DrawGui() = 0;
    };

    using PlayerEffectImGuiRequestBus = AZ::EBus<PlayerEffectImGuiRequests, PlayerEffectBusTraits>;

    class IPlayerAudioEffect : public PlayerEffectRequests
    {
    public:
        virtual bool Initialize(lab::AudioContext& ac) = 0;
        virtual void Shutdown() = 0;

        virtual std::shared_ptr<lab::AudioNode> GetInputNode() = 0;
        virtual std::shared_ptr<lab::AudioNode> GetOutputNode() = 0;

        void SetEnabled(bool enabled) override { m_enabled = enabled; }
        bool IsEnabled() const override { return m_enabled; }

    protected:
        SoundPlayerId GetPlayerId() const { return m_playerId; }
        PlayerEffectId GetId() const { return m_id; }

    private:
        friend class SoundPlayer;
        friend class SuneSystemComponent;
        PlayerEffectId m_id = PlayerEffectId();
        SoundPlayerId m_playerId = SoundPlayerId();
        bool m_enabled = true;
    };

    //Requests for spatialization type things, e.g Hrtf
    class PlayerEffectSpatializationRequests
    {
    public:
        AZ_RTTI(PlayerEffectSpatializationRequests, "{6AFDD961-5378-4CCB-918B-DDCE142D28F5}");
        virtual ~PlayerEffectSpatializationRequests() = default;

        virtual void SetTransform(const AZ::Transform& transform) {}

        virtual void SetHrtfSettings(
            lab::PannerNode::DistanceModel distanceModel,
            float refDistance,
            float maxDistance,
            float rolloffFactor,
            float coneInnerAngle, float coneOuterAngle, float coneOuterGain) {}
    };

    using PlayerEffectSpatializationRequestBus = AZ::EBus<PlayerEffectSpatializationRequests, PlayerEffectBusTraits>;
}
