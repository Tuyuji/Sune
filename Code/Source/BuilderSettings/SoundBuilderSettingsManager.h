/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "SoundBuilderSettings.h"
#include "SoundPresetSettings.h"
#include "AzCore/std/containers/unordered_map.h"
#include "AzCore/std/string/string.h"
#include "AzCore/std/typetraits/add_cv.h"

namespace Sune
{
    // Forward declarations
    struct SoundAssetSettings;
    struct SoundAssetBuilderSettings;

    /// Singleton manager for loading and resolving audio asset settings
    class SoundBuilderSettingsManager
    {
    public:
        static SoundBuilderSettingsManager* Get();

        SoundBuilderSettingsManager(const SoundBuilderSettingsManager&) = delete;
        SoundBuilderSettingsManager& operator=(const SoundBuilderSettingsManager&) = delete;

        ///  Get final resolved settings for an asset
        SoundAssetBuilderSettings GetSettings(
            const AZStd::string& filePath,
            const AZStd::string& platform = "pc"
        ) const;

        /// Get preset by name for specific platform
        const SoundPresetSettings* GetPreset(
            const AZStd::string& presetName,
            const AZStd::string& platform = "pc"
        ) const;

        /// Auto-detect preset from filename pattern
        AZStd::string GetSuggestedPreset(const AZStd::string& filePath) const;

        /// Get global settings for platform
        const SoundBuilderSettings* GetGlobalSettings(const AZStd::string& platform = "pc") const;

        /// Fingerprint for asset processor cache invalidation
        AZStd::string GetAnalysisFingerprint() const;

    private:
        SoundBuilderSettingsManager();
        ~SoundBuilderSettingsManager() = default;

        void Initialise();
        void LoadGlobalSettings();
        void LoadGlobalSetting(const AZStd::string settingPath);
        void LoadPresets();
        void LoadPreset(const AZStd::string& presetPath);
        AZStd::string GenerateFingerprint() const;

        /// Extract filename without extension and path
        AZStd::string GetFilenameWithoutExtension(const AZStd::string& filePath) const;

        /// Match filename against wildcard pattern
        bool WildcardMatch(const AZStd::string& filename, const AZStd::string& pattern) const;

        AZStd::string m_gemRoot;
        AZStd::string m_projectRoot;
        AZStd::unordered_map<AZStd::string, SoundBuilderSettings> m_globalSettings;
        AZStd::vector<PatternMapping> m_patternList;
        AZStd::unordered_map<AZStd::string, MultiplatformSoundPreset> m_presets;
        AZStd::string m_defaultPreset = "Default";
        AZStd::string m_analysisFingerprint;
    };
}
