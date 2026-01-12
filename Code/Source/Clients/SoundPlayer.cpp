/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundPlayer.h"

#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Math/Sfmt.h"
#include "AzCore/std/algorithm.h"
#include "Effects/LabHrtfEffect.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/PannerNode.h"
#include "Sune/SoundAsset.h"
#include "Sune/Utils.h"

using namespace Sune;

SoundPlayer::SoundPlayer(SoundPlayerId id) : m_id(id)
{
    SoundPlayerRequestBus::Handler::BusConnect(m_id);

    auto tls = SuneInterface::Get();
    AZ_Assert(tls, "LabSoundInterface not initialized");
    auto ctx = tls->GetLabContext();
    AZ_Assert(ctx, "LabContext not initialized");

    m_node = std::make_shared<lab::SampledAudioNode>(*ctx);
    AZ_Assert(m_node, "Failed to create SampledAudioNode");
    m_gainNode = std::make_shared<lab::GainNode>(*ctx);
    AZ_Assert(m_gainNode, "Failed to create GainNode");

    SoundPlayer::SetBus("Default");
}

SoundPlayer::~SoundPlayer()
{
    AZ::Data::AssetBus::MultiHandler::BusDisconnect();
    SoundPlayerRequestBus::Handler::BusDisconnect();

    for (auto& effect : m_effects)
    {
        effect->Shutdown();
    }
    m_effects.clear();

    m_node = nullptr;
    m_gainNode = nullptr;
    m_assetBus = nullptr;
    m_currentAsset = AZ::Data::Asset<AZ::Data::AssetData>();
    m_pendingAsset = AZ::Data::Asset<AZ::Data::AssetData>();
}

void SoundPlayer::SetBus(const AZStd::string& bus)
{
    AudioBusId newBus;
    if (bus == "")
    {
        newBus = AudioBusManagerInterface::Get()->CreateBus("Default");
    }
    else
    {
       newBus = AudioBusManagerInterface::Get()->CreateBus(bus);
    }
    if (newBus == m_busId)
    {
        return;
    }

    auto ctx = SuneInterface::Get()->GetLabContext();
    AZ_Assert(ctx, "LabContext not initialized");
    if (m_gainConnected)
    {
        //Already have a connection, lets disconnect
        if (m_busId == 0)
        {
            AZ_Error("Sune", false, "Already connected but we dont have a bus on player %d", m_id);
            return;
        }

        std::shared_ptr<lab::AudioNode> dest;
        AudioBusRequestsBus::EventResult(dest, m_busId, &AudioBusRequestsBus::Events::GetInputNode);

        //disconnect
        ctx->disconnect(dest, m_gainNode);
        m_gainConnected = false;
        AZ_Info("Sune", "Disconnected bus %s from player %d", bus.c_str(), m_id);
    }

    m_busId = newBus;
    if (m_busId == 0)
    {
        AZ_Error("Sune", false, "Invalid bus set for player %d\n", m_id);
        return;
    }
    std::shared_ptr<lab::AudioNode> dest;
    AudioBusRequestsBus::EventResult(dest, m_busId, &AudioBusRequestsBus::Events::GetInputNode);

    if (dest == nullptr)
    {
        AZ_Error("Sune", false, "Bus for player %d doesn't have a input :(\n", m_id);
        return;
    }

    AZStd::string busName;
    AudioBusRequestsBus::EventResult(busName, m_busId, &AudioBusRequestsBus::Events::GetName);

    if (!ctx->isConnected(dest, m_gainNode))
    {
        ctx->connect(dest, m_gainNode);
        ctx->synchronizeConnections();
    }
    m_gainConnected = true;
    AZ_Info("Sune", "Connected bus %s to player %d\n", busName.c_str(), m_id);
}

void SoundPlayer::SetAsset(const AZ::Data::AssetId assetId)
{
    if (assetId == m_assetId)
    {
        return;
    }

    auto asset = AZ::Data::AssetManager::Instance().GetAsset<SoundAsset>(assetId, AZ::Data::AssetLoadBehavior::PreLoad);
    if (!asset)
    {
        AZ_Error("Sune", false, "Failed to load asset %s", assetId.ToString<AZStd::string>().c_str());
        return;
    }

    m_pendingAsset = asset;
    m_assetId = assetId;

    if (asset->IsReady())
    {
        OnAssetReady(asset);
    }
    else
    {
        AZ::Data::AssetBus::MultiHandler::BusConnect(assetId);
    }
}

AZ::Data::AssetId SoundPlayer::GetAsset()
{
    return m_assetId;
}

void SoundPlayer::SetPlayMultiple(bool canPlayMultiple)
{
    m_canPlayMultiple = canPlayMultiple;
}

void SoundPlayer::SetGain(float gain)
{
    m_gainNode->gain()->setValue(gain);
}

float SoundPlayer::GetGain()
{
    return m_gainNode->gain()->value();
}

AZ::Data::Asset<SoundAsset> SoundPlayer::GetAssetData()
{
    //Always give pending
    return m_pendingAsset;
}

float SoundPlayer::GetLengthInSeconds()
{
    if (m_currentAsset.GetId() != m_assetId || !m_currentAsset.IsReady())
    {
        return 0.0f;
    }

    float result = 0;
    if (!m_currentAsset->GetLengthInSeconds(result))
    {
        return 0.0f;
    }

    return result;
}

int SoundPlayer::GetSampleRate()
{
    if (m_currentAsset.GetId() != m_assetId)
    {
        return 0;
    }

    return m_currentAsset->m_sampleRate;
}

void SoundPlayer::Play()
{
    if (!m_assetId.IsValid())
    {
        AZ_Error("Sune", false, "No asset set for player %d", m_id);
        return;
    }

    if (m_currentAsset.GetId() != m_assetId)
    {
        //Still in the process of waiting on the bus to be loaded
        auto ctx = SuneInterface::Get()->GetLabContext();
        m_schedPlayEvents.push_back({ctx->currentTime()});
        return;
    }

    if (!m_canPlayMultiple)
    {
        StopAll();
    }

    m_node->schedule(0.0f);
}

void SoundPlayer::PlayAtSeconds(float seconds)
{
    if (!m_assetId.IsValid())
    {
        AZ_Error("Sune", false, "No asset set for player %d", m_id);
        return;
    }

    if (m_currentAsset.GetId() != m_assetId)
    {
        //Still in the process of waiting on the bus to be loaded
        auto ctx = SuneInterface::Get()->GetLabContext();
        m_schedPlayEvents.push_back({ctx->currentTime() + seconds});
        return;
    }

    if (!m_canPlayMultiple)
    {
        StopAll();
    }

    m_node->schedule(0.0f, seconds, 0);
}

void SoundPlayer::PlayLooping(int loopCount, float seconds)
{
    if (!m_assetId.IsValid())
    {
        AZ_Error("Sune", false, "No asset set for player %d", m_id);
        return;
    }

    if (m_currentAsset.GetId() != m_assetId)
    {
        auto ctx = SuneInterface::Get()->GetLabContext();
        m_schedPlayEvents.push_back({ctx->currentTime() + seconds, loopCount});
        return;
    }

    if (!m_canPlayMultiple)
    {
        StopAll();
    }

    m_node->schedule(0.0f, seconds, loopCount);
}

void SoundPlayer::StopAll()
{
    m_node->clearPlayback();
}

bool SoundPlayer::IsPlaying()
{
    if (m_node->playbackState() == lab::SchedulingState::PLAYING
        || m_node->playbackState() == lab::SchedulingState::SCHEDULED)
    {
        auto ctx = SuneInterface::Get()->GetLabContext();
        lab::ContextRenderLock l(ctx.get(), "");
        return m_node->getCursor() > 0;
    }
    return false;
}

float SoundPlayer::GetPositionInSeconds()
{
    if (!m_currentAsset.GetId().IsValid())
    {
        return 0;
    }

    int32_t sampleCursor = -1;
    {
        auto ctx = SuneInterface::Get()->GetLabContext();
        lab::ContextRenderLock l(ctx.get(), "");
        sampleCursor = m_node->getCursor();
    }

    if (sampleCursor < 0)
    {
        return 0;
    }

    float sampleRate = m_assetBus->sampleRate();
    if (sampleRate <= 0.0f)
    {
        return 0;
    }

    return static_cast<float>(sampleCursor) / sampleRate;
}

uint64_t SoundPlayer::GetPositionInMicroseconds()
{
    if (!m_currentAsset.GetId().IsValid())
    {
        return 0;
    }

    int32_t sampleCursor = -1;
    {
        auto ctx = SuneInterface::Get()->GetLabContext();
        lab::ContextRenderLock l(ctx.get(), "");
        sampleCursor = m_node->getCursor();
    }
    if (sampleCursor < 0)
    {
        return 0;
    }

    float sampleRate = m_assetBus->sampleRate();
    if (sampleRate <= 0.0f)
    {
        return 0;
    }

    const double micros = (static_cast<double>(sampleCursor) / sampleRate) * 1'000'000.0;
    return static_cast<uint64_t>(micros);
}

PlayerEffectId SoundPlayer::AddEffect(const AZStd::string& effectName)
{
    PlayerEffectId id = PlayerEffectId();
    auto ctx = SuneInterface::Get()->GetLabContext();

    auto effect = CreateEffect(effectName);
    if (effect != nullptr)
    {
        AZ::Sfmt& smft = AZ::Sfmt::GetInstance();
        id = PlayerEffectId(smft.Rand64());
        effect->m_id = id;
        effect->m_playerId = m_id;
        effect->Initialize(*ctx);
        m_effects.push_back(AZStd::move(AZStd::unique_ptr<IPlayerAudioEffect>(effect)));
    }

    if (id != PlayerEffectId())
    {
        ReconnectGraph();
    }

    return id;
}

void SoundPlayer::RemoveEffect(PlayerEffectId id)
{
    auto it =
        AZStd::find_if(m_effects.begin(), m_effects.end(),
            [id](const AZStd::unique_ptr<IPlayerAudioEffect>& effect)
            {
                return effect->m_id == id;
            });

    if (it != m_effects.end())
    {
        it->get()->Shutdown();
        m_effects.erase(it);
        ReconnectGraph();
    }
}

PlayerEffectId SoundPlayer::GetSpatializationEffectId()
{
    for (auto& effect : m_effects)
    {
        if (effect->GetProcessingOrder() == PlayerEffectOrder::Spatializer)
            return effect->m_id;
    }

    return PlayerEffectId();
}

void SoundPlayer::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
{
    if (m_pendingAsset.GetId() != asset.GetId())
    {
        return;
    }

    auto soundAsset = asset.GetAs<SoundAsset>();
    if (!soundAsset)
    {
        AZ_Error("Sune", false, "Failed to load asset %s\n", asset.GetId().ToString<AZStd::string>().c_str());
        return;
    }
    m_assetBus = soundAsset->m_bus;

    m_node->setBus(m_assetBus);
    ReconnectGraph();

    //Good!
    AZ::Data::AssetBus::MultiHandler::BusDisconnect(m_assetId);
    m_currentAsset = asset;

    //Handled schedualed playbacks
    if (!m_schedPlayEvents.empty())
    {
        auto ctx = SuneInterface::Get()->GetLabContext();
        if (m_canPlayMultiple)
        {
            for ([[maybe_unused]]auto& sched : m_schedPlayEvents)
            {
                double offset =  ctx->currentTime() - sched.seconds;
                m_node->schedule(0.0f, offset, sched.loopCount);
            }
            m_schedPlayEvents.clear();
        }else
        {
            auto event = m_schedPlayEvents.back();
            m_node->clearPlayback();
            m_node->schedule(0.0f, ctx->currentTime() - event.seconds, event.loopCount);
            m_schedPlayEvents.clear();
        }
    }
}

static void DisconnectAll(lab::ContextGraphLock* l, bool bIsInput, lab::AudioNode* node)
{
    lab::ContextRenderLock rl(l->context(), "DisconnectAll");
    if (bIsInput)
    {
        for (size_t i = 0; i < node->numberOfInputs(); ++i)
        {
            auto input = node->input(i);
            if (!input)
                continue;
            lab::AudioNodeInput::disconnectAll(*l, input);
            AZ_Info("Sune", "Disconnected input %d from node %s channel count: %d\n", i, node->name(), input->numberOfChannels(rl));
        }
    }
    else
    {
        for (size_t i = 0; i < node->numberOfOutputs(); ++i)
        {
            auto output = node->output(i);
            if (!output)
                continue;
            lab::AudioNodeOutput::disconnectAll(*l, output);
            AZ_Info("Sune", "Disconnected output %d from node \"%s\" channel count: %d\n", i, node->name(), output->numberOfChannels());
        }
    }
}

void SoundPlayer::ReconnectGraph()
{
    if (m_assetBus == nullptr)
    {
        //sample player needs a bus first to be properly connected.
        return;
    }

    auto ctx = SuneInterface::Get()->GetLabContext();
    AZ_Info("Sune", "Reconnecting graph for player %d\n", m_id);

    lab::ContextGraphLock r(ctx.get(), "SoundPlayer::ReconnectGraph");

    //Disconnect all internal connections but not the grain node output to bus
    {
        if (m_node != nullptr)
        {
            AZ_Info("Sune", "Disconnecting sample player %d\n", m_id);
            //Disconnect all outputs from our sample player
            DisconnectAll(&r, false, m_node.get());
        }

        //Disconnect all inputs from gain (our main output node)
        if (m_gainNode)
        {
            AZ_Info("Sune", "Disconnecting gain node %d\n", m_id);
            DisconnectAll(&r, true, m_gainNode.get());
        }

        //disconnect effects
        for (auto& effect : m_effects)
        {
            if (auto inputNode = effect->GetInputNode())
                DisconnectAll(&r, true, inputNode.get());
            if (auto outputNode = effect->GetOutputNode())
                DisconnectAll(&r, false, outputNode.get());
        }
    }

    AZStd::vector<IPlayerAudioEffect*> enabledEffects;
    for (auto& effect : m_effects)
    {
        if (effect->IsEnabled())
            enabledEffects.push_back(effect.get());
    }

    //sort effects based on GetProcessingOrder
    std::ranges::sort(enabledEffects,
                      [](const IPlayerAudioEffect* a, const IPlayerAudioEffect* b)
                      {
                          return a->GetProcessingOrder() < b->GetProcessingOrder();
                      });

    std::shared_ptr<lab::AudioNode> currentOutput = m_node;

    // Connect through each effect
    for (auto* effect : enabledEffects)
    {
        auto inputNode = effect->GetInputNode();
        if (inputNode && currentOutput)
        {
            lab::AudioNodeInput::connect(r, inputNode->input(0), currentOutput->output(0));
        }
        auto nextOutput = effect->GetOutputNode();
        if (nextOutput != nullptr)
        {
            currentOutput = nextOutput;
        }
    }

    if (currentOutput)
    {
        lab::AudioNodeInput::connect(r, m_gainNode->input(0), currentOutput->output(0));
    }
}
