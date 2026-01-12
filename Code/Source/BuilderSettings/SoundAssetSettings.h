/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "Sune/SoundAsset.h"

namespace Sune
{
    //Per-asset override settings (stored in .assetinfo)
    struct SoundAssetSettings
    {
    public:
        AZ_TYPE_INFO(SoundAssetSettings, "{CFC59C16-1E0D-46CF-92C9-E24712D3D147}");
        AZ_CLASS_ALLOCATOR(SoundAssetSettings, AZ::SystemAllocator);

        AZStd::string m_presetName; //Override preset selection

        //Optional overrides
        AZStd::optional<AudioImportFormat> m_formatOverride;
        AZStd::optional<AudioLoadMethod> m_loadMethodOverride;
        AZStd::optional<float> m_qualityOverride;

        float m_volumeAdjustment = 1.0f;

        static void Reflect(AZ::ReflectContext* context);
    };

    //Final resolved settings
    struct SoundAssetBuilderSettings
    {
    public:
        AZStd::string m_presetName;
        AudioImportFormat m_format;
        AudioLoadMethod m_loadMethod;
        float m_quality;
        float m_volumeAdjustment;
    };
}
