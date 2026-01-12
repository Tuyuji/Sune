/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <memory>

#include "AzCore/Asset/AssetCommon.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/PannerNode.h"
#include "Sune/AudioBusManagerInterface.h"
#include "Sune/PlayerAudioEffect.h"
#include "Sune/SuneBus.h"

namespace lab
{
    class PannerNode;
    class HRTFPanner;
}

namespace lab
{
    class SampledAudioNode;
}

namespace Sune
{
    class SoundAsset;

    struct PlaybackEvent
    {
        double seconds = 0.0;
        int loopCount = 0;
    };

    class SoundPlayer
        : public SoundPlayerRequestBus::Handler
        , protected AZ::Data::AssetBus::MultiHandler
    {
    public:
        SoundPlayer(SoundPlayerId id);
        ~SoundPlayer() override;

        SoundPlayerId GetId() const { return m_id; }
        std::shared_ptr<lab::AudioBus> GetAudioBus() const { return m_assetBus; }
        std::shared_ptr<lab::SampledAudioNode> GetNode() const { return m_node; }

    protected:
        //PlayerRequests
        void SetBus(const AZStd::string& bus) override;

        void SetAsset(const AZ::Data::AssetId assetId) override;
        AZ::Data::AssetId GetAsset() override;

        void SetPlayMultiple(bool canPlayMultiple) override;

        void SetGain(float gain) override;
        float GetGain() override;

        AZ::Data::Asset<SoundAsset> GetAssetData() override;

        float GetLengthInSeconds() override;
        int GetSampleRate() override;

        void Play() override;
        void PlayAtSeconds(float seconds) override;
        void PlayLooping(int loopCount, float seconds) override;
        void StopAll() override;

        bool IsPlaying() override;

        float GetPositionInSeconds() override;
        uint64_t GetPositionInMicroseconds() override;

        PlayerEffectId AddEffect(const AZStd::string& effectName) override;
        void RemoveEffect(PlayerEffectId id) override;

        PlayerEffectId GetSpatializationEffectId() override;

        //AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
    private:
        friend class SuneSystemComponent;
        void ReconnectGraph();

        SoundPlayerId m_id = SoundPlayerId();
        AudioBusId m_busId = InvalidAudioBusId;

        AZ::Data::AssetId m_assetId = {};
        AZ::Data::Asset<Sune::SoundAsset> m_currentAsset = {};
        AZ::Data::Asset<Sune::SoundAsset> m_pendingAsset = {};
        std::shared_ptr<lab::AudioBus> m_assetBus = {};
        AZStd::vector<PlaybackEvent> m_schedPlayEvents = {};

        //Known static nodes in the SoundPlayer graph
        std::shared_ptr<lab::SampledAudioNode> m_node = nullptr;
        std::shared_ptr<lab::GainNode> m_gainNode = nullptr;

        AZStd::vector<AZStd::unique_ptr<IPlayerAudioEffect>> m_effects;

        bool m_canPlayMultiple = true;
        bool m_gainConnected = false;
    };
} // Sune
