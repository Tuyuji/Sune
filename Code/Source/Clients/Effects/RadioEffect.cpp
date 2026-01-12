/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "RadioEffect.h"

#include <LabSound/core/BiquadFilterNode.h>
#include <LabSound/core/WaveShaperNode.h>

#include "imgui/imgui.h"
#include "LabSound/core/AudioNodeOutput.h"

using namespace Sune;

bool RadioEffect::Initialize(lab::AudioContext& ac)
{
    m_highpass = std::make_shared<lab::BiquadFilterNode>(ac);
    m_lowpass = std::make_shared<lab::BiquadFilterNode>(ac);
    {
        lab::ContextGraphLock r(&ac, "idk");
        m_highpass->addOutput(r, std::make_unique<lab::AudioNodeOutput>(m_highpass.get(), 2));
        m_lowpass->addOutput(r, std::make_unique<lab::AudioNodeOutput>(m_lowpass.get(), 2));
    }
    lab::AudioNodeDescriptor desc = {nullptr, nullptr, 2};
    m_distortion = std::make_shared<lab::WaveShaperNode>(ac, desc);

    m_highpass->setType(lab::HIGHPASS);
    m_highpass->frequency()->setValue(595.0f);

    m_lowpass->setType(lab::LOWPASS);
    m_lowpass->frequency()->setValue(2312.0f);

    std::vector<float> curve(256);
    for (int i = 0; i < 256; ++i)
    {
        float x = (i / 128.0f) - 1.0f;
        curve[i] = std::tanh(x * 2.0f);
    }
    m_distortion->setCurve(curve);


    ac.connect(m_lowpass, m_highpass);
    ac.connect(m_distortion, m_lowpass);

    PlayerEffectImGuiRequestBus::Handler::BusConnect(GetId());
    return true;
}

void RadioEffect::Shutdown()
{
    PlayerEffectImGuiRequestBus::Handler::BusDisconnect();
    m_highpass = nullptr;
    m_lowpass = nullptr;
    m_distortion = nullptr;
}

const char* RadioEffect::GetEffectName() const
{
    return "Radio";
}

PlayerEffectOrder RadioEffect::GetProcessingOrder() const
{
    return PlayerEffectOrder::Filtering;
}

std::shared_ptr<lab::AudioNode> RadioEffect::GetInputNode()
{
    return m_highpass;
}

std::shared_ptr<lab::AudioNode> RadioEffect::GetOutputNode()
{
    return m_distortion;
}

void RadioEffect::DrawGui()
{
    float highPassVal = m_highpass->frequency()->value();
    if (ImGui::SliderFloat("High Pass", &highPassVal, 10.0f, 20000.0f))
    {
        m_highpass->frequency()->setValue(highPassVal);
    }

    float lowPassVal = m_lowpass->frequency()->value();
    if (ImGui::SliderFloat("Low Pass", &lowPassVal, 10.0f, 20000.0f))
    {
        m_lowpass->frequency()->setValue(lowPassVal);
    }
}
