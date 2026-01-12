/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "LabHrtfEffect.h"

#include "Sune/Utils.h"
#include "imgui/imgui.h"

using namespace Sune;

bool LabHrtfEffect::Initialize(lab::AudioContext& ac)
{
    m_node = std::make_shared<lab::PannerNode>(ac);
    m_node->setPanningModel(lab::PanningModel::HRTF);

    PlayerEffectSpatializationRequestBus::Handler::BusConnect(GetId());
    PlayerEffectImGuiRequestBus::Handler::BusConnect(GetId());
    return m_node != nullptr;
}

void LabHrtfEffect::Shutdown()
{
    PlayerEffectImGuiRequestBus::Handler::BusDisconnect();
    PlayerEffectSpatializationRequestBus::Handler::BusDisconnect();
    m_node = nullptr;
}

std::shared_ptr<lab::AudioNode> LabHrtfEffect::GetInputNode()
{
    return m_node;
}

std::shared_ptr<lab::AudioNode> LabHrtfEffect::GetOutputNode()
{
    return m_node;
}

void LabHrtfEffect::SetHrtfSettings(lab::PannerNode::DistanceModel distanceModel, float refDistance, float maxDistance,
    float rolloffFactor, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
{
    m_node->setDistanceModel(distanceModel);
    m_node->setRefDistance(refDistance);
    m_node->setMaxDistance(maxDistance);
    m_node->setRolloffFactor(rolloffFactor);
    m_node->setConeInnerAngle(coneInnerAngle);
    m_node->setConeOuterAngle(coneOuterAngle);
    m_node->setConeOuterGain(coneOuterGain);
}

void LabHrtfEffect::DrawGui()
{
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "HRTF Spatialization Controls:");

    // Distance model
    static int distanceModelIndex = m_node->distanceModel();
    const char* distanceModels[] = {
        "Linear", "Inverse", "Exponential"
    };

    if (ImGui::Combo("Distance Model", &distanceModelIndex, distanceModels, IM_ARRAYSIZE(distanceModels)))
    {
        lab::PannerNode::DistanceModel model;
        switch (distanceModelIndex)
        {
        case 0: model = lab::PannerNode::LINEAR_DISTANCE; break;
        case 1: model = lab::PannerNode::INVERSE_DISTANCE; break;
        case 2: model = lab::PannerNode::EXPONENTIAL_DISTANCE; break;
        default: model = lab::PannerNode::EXPONENTIAL_DISTANCE;
        }
        m_node->setDistanceModel(model);
    }

    // Distance parameters
    static float refDistance = 1.0f;
    static float maxDistance = 10000.0f;
    static float rolloffFactor = 1.0f;

    if (ImGui::DragFloat("Ref Distance", &refDistance, 0.1f, 0.1f, 100.0f))
    {
        m_node->setRefDistance(refDistance);
    }

    if (ImGui::DragFloat("Max Distance", &maxDistance, 1.0f, 1.0f, 100000.0f))
    {
        m_node->setMaxDistance(maxDistance);
    }

    if (ImGui::DragFloat("Rolloff Factor", &rolloffFactor, 0.01f, 0.0f, 10.0f))
    {
        m_node->setRolloffFactor(rolloffFactor);
    }

    // Cone parameters
    if (ImGui::TreeNode("Cone Settings"))
    {
        static float coneInnerAngle = 360.0f;
        static float coneOuterAngle = 360.0f;
        static float coneOuterGain = 0.0f;

        if (ImGui::SliderFloat("Inner Angle", &coneInnerAngle, 0.0f, 360.0f))
        {
            m_node->setConeInnerAngle(coneInnerAngle);
        }

        if (ImGui::SliderFloat("Outer Angle", &coneOuterAngle, 0.0f, 360.0f))
        {
            m_node->setConeOuterAngle(coneOuterAngle);
        }

        if (ImGui::SliderFloat("Outer Gain", &coneOuterGain, 0.0f, 1.0f))
        {
            m_node->setConeOuterGain(coneOuterGain);
        }

        ImGui::TreePop();
    }
}
