/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/PlayerAudioEffect.h>

namespace Sune
{
    class LabHrtfEffect
        : public IPlayerAudioEffect
        , public PlayerEffectSpatializationRequestBus::Handler
        , protected PlayerEffectImGuiRequestBus::Handler
    {
    public:
        bool Initialize(lab::AudioContext& ac) override;
        void Shutdown() override;

        std::shared_ptr<lab::AudioNode> GetInputNode() override;
        std::shared_ptr<lab::AudioNode> GetOutputNode() override;

        const char* GetEffectName() const override
        {
            return "LabHrtf";
        }
        PlayerEffectOrder GetProcessingOrder() const override
        {
            return PlayerEffectOrder::Spatializer;
        }

        void SetHrtfSettings(lab::PannerNode::DistanceModel distanceModel, float refDistance, float maxDistance, float rolloffFactor, float coneInnerAngle, float coneOuterAngle, float coneOuterGain) override;

        void DrawGui() override;
    private:
        std::shared_ptr<lab::PannerNode> m_node;
    };
} // Sune