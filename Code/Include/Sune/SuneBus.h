/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/SuneTypeIds.h>
#include <Sune/SoundAsset.h>
#include <Sune/SuneId.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Math/Vector3.h>

namespace Sune
{
    class IPlayerAudioEffect;
}

namespace lab
{
    class AudioDevice;
    class AudioDestinationNode;
    class AudioContext;
    class AudioBus;
}

namespace Sune
{
    constexpr AZ::Crc32 AudioPlayerServiceName = AZ_CRC_CE("AudioPlayerService");
    constexpr AZ::Crc32 AudioSpatializationEffectServiceName = AZ_CRC_CE("AudioSpatializerEffectService");
    
    AZ_SUNE_ID(SoundPlayerId, "{8F3A2E1D-4B6C-4A9F-8E2D-1C5B7A9E4F3D}");
    AZ_SUNE_ID(PlayerEffectId, "{2A7E9F4B-3D1C-4E8A-9B5F-6D2A8C4E1B7F}");

    // Player effect factory bus
    class PlayerEffectFactoryRequests
    {
    public:
        AZ_RTTI(PlayerEffectFactoryRequests, "{A5E2F8D3-6B4C-4E9A-8F5D-2C7A9B3E1F6D}");
        virtual ~PlayerEffectFactoryRequests() = default;

        virtual IPlayerAudioEffect* CreateEffect(const AZStd::string& id) = 0;
    };

    class PlayerEffectFactoryBusTraits
        : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZStd::string;
    };

    using PlayerEffectFactoryBus = AZ::EBus<PlayerEffectFactoryRequests, PlayerEffectFactoryBusTraits>;

    inline IPlayerAudioEffect* CreateEffect(const AZStd::string& effectId)
    {
        IPlayerAudioEffect* effect = nullptr;
        PlayerEffectFactoryBus::BroadcastResult(effect, &PlayerEffectFactoryRequests::CreateEffect, effectId);
        return effect;
    }

    class SuneRequests
    {
    public:
        AZ_RTTI(SuneRequests, SuneRequestsTypeId);
        virtual ~SuneRequests() = default;
        // Put your public methods here

        virtual std::shared_ptr<lab::AudioDevice> GetAudioDevice() const = 0;
        virtual std::shared_ptr<lab::AudioContext> GetLabContext() const = 0;
        virtual std::shared_ptr<lab::AudioDestinationNode> GetAudioDestination() const = 0;

        virtual int GetPeriodSizeInFrames() const = 0;

        //A generic player that can play audio assets
        virtual SoundPlayerId CreatePlayer() {return SoundPlayerId();}
        virtual void DestroyPlayer(SoundPlayerId id) {}

        //! Create an effect by name (queries the PlayerEffectFactoryBus)
        virtual IPlayerAudioEffect* CreateEffect(const AZStd::string& name)
        {
            return nullptr;
        }
    };

    class SuneBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using SuneRequestBus = AZ::EBus<SuneRequests, SuneBusTraits>;
    using SuneInterface = AZ::Interface<SuneRequests>;

    class SoundPlayerRequests
    {
    public:
        AZ_RTTI(SoundPlayerRequests, SoundPlayerRequestsTypeId);
        virtual ~SoundPlayerRequests() = default;

        virtual void SetBus(const AZStd::string& bus) = 0;

        virtual void SetAsset(const AZ::Data::AssetId assetId) = 0;
        virtual AZ::Data::AssetId GetAsset() = 0;

        virtual void SetPlayMultiple(bool canPlayMultiple) = 0;
        //GetPlayMultiple?

        virtual void SetGain(float gain) = 0;
        virtual float GetGain() = 0;

        virtual AZ::Data::Asset<SoundAsset> GetAssetData() = 0;

        //Asset info
        virtual float GetLengthInSeconds() = 0;
        virtual int GetSampleRate() = 0;

        //Controls
        virtual void Play() = 0;
        virtual void PlayAtSeconds(float seconds) = 0;
        virtual void PlayLooping(int loopCount, float seconds) = 0;
        virtual void StopAll() = 0;

        virtual bool IsPlaying() = 0;

        //Only works if PlayMultiple is false
        virtual float GetPositionInSeconds() = 0;
        virtual uint64_t GetPositionInMicroseconds() = 0;

        virtual PlayerEffectId AddEffect(const AZStd::string& effectName) = 0;
        virtual void RemoveEffect(PlayerEffectId id) = 0;

        //Finds the first spatialization effect.
        virtual PlayerEffectId GetSpatializationEffectId() = 0;
    };

    class SoundPlayerBusTraits
        : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SoundPlayerId;
    };

    using SoundPlayerRequestBus = AZ::EBus<SoundPlayerRequests, SoundPlayerBusTraits>;
} // namespace Sune

AZ_SUNE_ID_HASH(Sune::SoundPlayerId);
AZ_SUNE_ID_HASH(Sune::PlayerEffectId);
