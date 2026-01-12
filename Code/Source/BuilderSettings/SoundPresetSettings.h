/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <Sune/SoundAsset.h>
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

namespace Sune
{
    // Single preset config
    struct SoundPresetSettings
    {
    public:
        AZ_TYPE_INFO(SoundPresetSettings, "{D9F85593-3FD9-4BA3-922C-70A746EA1D84}");
        AZ_CLASS_ALLOCATOR(SoundPresetSettings, AZ::SystemAllocator);

        AZStd::string m_name;
        AZStd::string m_description;

        //Processing settings
        AudioImportFormat m_format = AudioImportFormat::Vorbis;
        AudioLoadMethod m_loadMethod = AudioLoadMethod::DecodeOnLoad;
        float m_quality = 0.8f;

        static void Reflect(AZ::ReflectContext* context);
    };

    struct SoundAssetSettings;

    //Multiplatform preset wrapper (default + per-platform overrides)
    struct MultiplatformSoundPreset
    {
    public:
        AZ_TYPE_INFO(MultiplatformSoundPreset, "{A3E4EF43-940E-410D-98EC-3BCD27BBBDA6}");
        AZ_CLASS_ALLOCATOR(MultiplatformSoundPreset, AZ::SystemAllocator);

        SoundPresetSettings m_defaultSettings = {};
        AZStd::unordered_map<AZStd::string, SoundPresetSettings> m_platformOverrides = {};

        //Get preset for platform (merges override with default if platform override exists)
        const SoundPresetSettings* GetPreset(const AZStd::string& platform) const;

        static void Reflect(AZ::ReflectContext* context);
    };
}
