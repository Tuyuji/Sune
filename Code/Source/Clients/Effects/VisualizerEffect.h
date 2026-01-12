/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/Effects/VisualizerBus.h>

namespace lab
{
    class AnalyserNode;
}

namespace Sune
{
    class VisualizerEffect
        : public IPlayerAudioEffect
        , public PlayerEffectImGuiRequestBus::Handler
        , public VisualizerEffectRequestBus::Handler
    {
    public:
        VisualizerEffect();
        virtual ~VisualizerEffect() override;

        // IPlayerAudioEffect interface
        bool Initialize(lab::AudioContext& ac) override;
        void Shutdown() override;

        const char* GetEffectName() const override;
        PlayerEffectOrder GetProcessingOrder() const override;
        std::shared_ptr<lab::AudioNode> GetInputNode() override;
        std::shared_ptr<lab::AudioNode> GetOutputNode() override;

        // PlayerEffectImGuiRequests
        void DrawGui() override;

        // VisualizerEffectRequests
        void SetVisualizerEntities(const AZStd::vector<AZ::EntityId>& entities) override;
        AZStd::vector<AZ::EntityId> GetVisualizerEntities() const override;

        void SetScaleMultiplier(float multiplier) override;
        float GetScaleMultiplier() const override;

        void SetMinScale(float minScale) override;
        float GetMinScale() const override;

        void UpdateVisualization() override;

    private:
        std::shared_ptr<lab::AnalyserNode> m_analyser;
        AZStd::vector<AZ::EntityId> m_visualizerEntities;

        float m_scaleMultiplier = 5.0f; // How much to scale entities based on frequency
        float m_minScale = 0.1f; // Minimum entity scale
    };
} // Sune