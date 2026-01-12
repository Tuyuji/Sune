/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "VisualizerEffect.h"

#include <LabSound/core/AnalyserNode.h>
#include <AzCore/Component/TransformBus.h>

#include "AzCore/Component/NonUniformScaleBus.h"
#include "imgui/imgui.h"

using namespace Sune;

VisualizerEffect::VisualizerEffect()
{
}

VisualizerEffect::~VisualizerEffect()
{
}

bool VisualizerEffect::Initialize(lab::AudioContext& ac)
{
    m_analyser = std::make_shared<lab::AnalyserNode>(ac);

    m_analyser->setMinDecibels(-100.0);
    m_analyser->setMaxDecibels(-15.0);
    m_analyser->setSmoothingTimeConstant(0.4);
    ac.addAutomaticPullNode(m_analyser);

    PlayerEffectImGuiRequestBus::Handler::BusConnect(GetId());
    VisualizerEffectRequestBus::Handler::BusConnect(GetId());

    return true;
}

void VisualizerEffect::Shutdown()
{
    VisualizerEffectRequestBus::Handler::BusDisconnect();
    PlayerEffectImGuiRequestBus::Handler::BusDisconnect();

    auto ctx = Sune::SuneInterface::Get()->GetLabContext();
    ctx->removeAutomaticPullNode(m_analyser);

    m_analyser = nullptr;
    m_visualizerEntities.clear();
}

const char* VisualizerEffect::GetEffectName() const
{
    return "Visualizer";
}

PlayerEffectOrder VisualizerEffect::GetProcessingOrder() const
{
    return PlayerEffectOrder::Utility;
}

std::shared_ptr<lab::AudioNode> VisualizerEffect::GetInputNode()
{
    return m_analyser;
}

std::shared_ptr<lab::AudioNode> VisualizerEffect::GetOutputNode()
{
    return nullptr;
}

void VisualizerEffect::SetVisualizerEntities(const AZStd::vector<AZ::EntityId>& entities)
{
    m_visualizerEntities = entities;

    auto ctx = Sune::SuneInterface::Get()->GetLabContext();
    lab::ContextRenderLock l(ctx.get(), "");
    m_analyser->setFftSize(l, m_visualizerEntities.size() * 2);
}

AZStd::vector<AZ::EntityId> VisualizerEffect::GetVisualizerEntities() const
{
    return m_visualizerEntities;
}

void VisualizerEffect::SetScaleMultiplier(float multiplier)
{
    m_scaleMultiplier = AZ::GetMax(0.1f, multiplier);
}

float VisualizerEffect::GetScaleMultiplier() const
{
    return m_scaleMultiplier;
}

void VisualizerEffect::SetMinScale(float minScale)
{
    m_minScale = AZ::GetMax(0.01f, minScale);
}

float VisualizerEffect::GetMinScale() const
{
    return m_minScale;
}

void VisualizerEffect::UpdateVisualization()
{
    if (!m_analyser || m_visualizerEntities.empty() || !IsEnabled())
    {
        return;
    }

    const size_t entityCount = m_visualizerEntities.size();
    const float minDb = static_cast<float>(m_analyser->minDecibels());
    const float maxDb = static_cast<float>(m_analyser->maxDecibels());
    const float dbRange = maxDb - minDb;

    std::vector<float> freqData;
    freqData.resize(m_analyser->frequencyBinCount());
    m_analyser->getFloatFrequencyData(freqData);

    const size_t binCount = freqData.size();
    const size_t binsPerEntity = binCount / entityCount;

    for (size_t i = 0; i < entityCount; ++i)
    {
        if (!m_visualizerEntities[i].IsValid())
        {
            continue;
        }

        float avgDecibels = 0.0f;
        const size_t startBin = i * binsPerEntity;
        const size_t endBin = (i == entityCount - 1) ? binCount : (i + 1) * binsPerEntity;

        for (size_t bin = startBin; bin < endBin; ++bin)
        {
            avgDecibels += freqData[bin];
        }
        avgDecibels /= static_cast<float>(endBin - startBin);

        float normalizedValue = (avgDecibels - minDb) / dbRange;
        normalizedValue = AZ::GetClamp(normalizedValue, 0.0f, 1.0f);

        float targetScale = m_minScale + (normalizedValue * m_scaleMultiplier);

        AZ::Vector3 scale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(scale, m_visualizerEntities[i], &AZ::NonUniformScaleRequests::GetScale);
        scale.SetZ(targetScale);

        AZ::NonUniformScaleRequestBus::Event(m_visualizerEntities[i], &AZ::NonUniformScaleRequests::SetScale, scale);
    }
}

void VisualizerEffect::DrawGui()
{
    ImGui::Text("Visualizer Effect");
    ImGui::Separator();

    ImGui::Text("Entities: %zu", m_visualizerEntities.size());
    if (m_analyser)
    {
        ImGui::Text("Frequency Bins: %zu", m_analyser->frequencyBinCount());
    }

    ImGui::Spacing();

    float multiplier = m_scaleMultiplier;
    if (ImGui::SliderFloat("Scale Multiplier", &multiplier, 0.1f, 20.0f))
    {
        SetScaleMultiplier(multiplier);
    }

    float minScale = m_minScale;
    if (ImGui::SliderFloat("Min Scale", &minScale, 0.001f, 2.0f))
    {
        SetMinScale(minScale);
    }

    ImGui::Spacing();
    ImGui::Text("AnalyserNode Settings:");

    if (m_analyser)
    {
        int fftSize = m_analyser->fftSize();
        if (ImGui::InputInt("FFT Size", &fftSize, 1, 5) && ImGui::IsItemDeactivatedAfterEdit())
        {
            if (fftSize < 1)
            {
                fftSize = 1;
            }
            auto ctx = Sune::SuneInterface::Get()->GetLabContext();
            lab::ContextRenderLock l(ctx.get(), "");
            m_analyser->setFftSize(l, fftSize);
        }


        float smoothing = static_cast<float>(m_analyser->smoothingTimeConstant());
        if (ImGui::DragFloat("FFT Smoothing", &smoothing, 0.1f, 0.0f, 1.0f) && ImGui::IsItemDeactivatedAfterEdit())
        {
            m_analyser->setSmoothingTimeConstant(smoothing);
        }

        float minDb = m_analyser->minDecibels();
        if (ImGui::DragFloat("Min Decibels Node", &minDb, 0.1f, -100.0f, 0.0f, "%.1f dB") && ImGui::IsItemDeactivatedAfterEdit())
        {
            m_analyser->setMinDecibels(minDb);
        }

        float maxDb = m_analyser->maxDecibels();
        if (ImGui::DragFloat("Max Decibels Node", &maxDb, 0.1f, -100.0f, 0.0f) && ImGui::IsItemDeactivatedAfterEdit())
        {
            m_analyser->setMaxDecibels(maxDb);
        }
    }

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Frequency Data Preview"))
    {
        static float minDb = -110.0f;
        static float maxDb = -10.0f;
        ImGui::SliderFloat("Min Decibels", &minDb, -200.0f, 0.0f);
        ImGui::SliderFloat("Max Decibels", &maxDb, -200.0f, 8.0f);

        std::vector<float> floatFreqData;
        floatFreqData.resize(m_analyser->frequencyBinCount());
        if (m_analyser)
        {
            m_analyser->getFloatFrequencyData(floatFreqData);

            ImGui::PlotHistogram("##Frequencies", floatFreqData.data(), static_cast<int>(floatFreqData.size()),
                                0, nullptr, minDb, maxDb, ImVec2(0, 280));
        }
    }
}