/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundPresetSettings.h"
#include "SoundAssetSettings.h"
#include <AzCore/Serialization/SerializeContext.h>

using namespace Sune;

void SoundPresetSettings::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;
    sc->Class<SoundPresetSettings>()
        ->Version(0)
        ->Field("name", &SoundPresetSettings::m_name)
        ->Field("description", &SoundPresetSettings::m_description)
        ->Field("format", &SoundPresetSettings::m_format)
        ->Field("loadMethod", &SoundPresetSettings::m_loadMethod)
        ->Field("quality", &SoundPresetSettings::m_quality)
        ;
}

const SoundPresetSettings* MultiplatformSoundPreset::GetPreset(const AZStd::string& platform) const
{
    auto it = m_platformOverrides.find(platform);
    return it != m_platformOverrides.end() ? &it->second : &m_defaultSettings;
}

void MultiplatformSoundPreset::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;
    sc->Class<MultiplatformSoundPreset>()
        ->Version(0)
        ->Field("defaultSettings", &MultiplatformSoundPreset::m_defaultSettings)
        ->Field("platformOverrides", &MultiplatformSoundPreset::m_platformOverrides)
        ;
}
