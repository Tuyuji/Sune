/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/PlayerAudioEffect.h>

namespace lab
{
    class BiquadFilterNode;
    class WaveShaperNode;
    class NoiseNode;
}

namespace Sune
{
    class RadioEffect
        : public IPlayerAudioEffect
        , public PlayerEffectImGuiRequestBus::Handler
    {
    public:
        bool Initialize(lab::AudioContext& ac) override;
        void Shutdown() override;

        const char* GetEffectName() const override;
        PlayerEffectOrder GetProcessingOrder() const override;
        std::shared_ptr<lab::AudioNode> GetInputNode() override;
        std::shared_ptr<lab::AudioNode> GetOutputNode() override;

        void DrawGui() override;
    private:
        std::shared_ptr<lab::BiquadFilterNode> m_highpass;
        std::shared_ptr<lab::BiquadFilterNode> m_lowpass;
        std::shared_ptr<lab::WaveShaperNode> m_distortion;
    };
} // Sune