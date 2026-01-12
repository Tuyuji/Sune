/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Component/ComponentBus.h"
#include "LabSound/core/PannerNode.h"
#include "Sune/SoundAsset.h"

namespace Sune
{
    class AudioPlayerComponentConfig : public AZ::ComponentConfig
    {
        AZ_RTTI(AudioPlayerComponentConfig, "{DAB37FF8-B86B-4EF7-8319-142976A862A8}");
    public:
        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<SoundAsset> m_audioAsset = {};
        AZStd::string m_audioBus = "Default";
        bool m_playMultiple = true;
        float m_gain = 1.0f;
        bool m_loop = false;
        bool m_autoPlay = true;
    };
} // Sune

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(lab::PannerNode::DistanceModel, "{5FCE6AD1-8202-40D9-8658-61E48E28D3DE}");
}