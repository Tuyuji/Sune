/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundAssetSettings.h"

void Sune::SoundAssetSettings::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (sc)
    {
        sc->Class<SoundAssetSettings>()
            ->Version(1)
            ->Field("presetName", &SoundAssetSettings::m_presetName)
            ->Field("format", &SoundAssetSettings::m_formatOverride)
            ->Field("loadMethod", &SoundAssetSettings::m_loadMethodOverride)
            ->Field("quality", &SoundAssetSettings::m_qualityOverride)
            ->Field("volume", &SoundAssetSettings::m_volumeAdjustment)
            ;
    }
}
