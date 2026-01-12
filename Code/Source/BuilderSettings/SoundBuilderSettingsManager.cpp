/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundBuilderSettingsManager.h"
#include "SoundAssetSettings.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Json/JsonSerialization.h>
#include <AzCore/Serialization/Json/JsonUtils.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/SystemFile.h>
#include <AzCore/Utils/Utils.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzCore/std/algorithm.h>

#include "AzCore/Serialization/Utils.h"

using namespace Sune;

SoundBuilderSettingsManager* SoundBuilderSettingsManager::Get()
{
    static SoundBuilderSettingsManager settings;
    return &settings;
}

SoundBuilderSettingsManager::SoundBuilderSettingsManager()
{
    Initialise();
}

void SoundBuilderSettingsManager::Initialise()
{
    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (!fileIO)
    {
        AZ_Error("SoundBuilderSettingsManager", false, "FileIO not available");
        return;
    }

    const char* gemRootAlias = "@gemroot:Sune@";
    char resolvedPath[AZ_MAX_PATH_LEN];
    if (fileIO->ResolvePath(gemRootAlias, resolvedPath, AZ_MAX_PATH_LEN))
    {
        m_gemRoot = resolvedPath;
    }

    const char* projectRootAlias = "@projectroot@";
    if (fileIO->ResolvePath(projectRootAlias, resolvedPath, AZ_MAX_PATH_LEN))
    {
        m_projectRoot = resolvedPath;
    }

    LoadGlobalSettings();
    LoadPresets();

    m_analysisFingerprint = GenerateFingerprint();
}

void SoundBuilderSettingsManager::LoadGlobalSettings()
{
    AZStd::string gemConfigPath = AZStd::string::format("%s/Config/SoundBuilder.json", m_gemRoot.c_str());
    AZStd::string projectConfigPath = AZStd::string::format("%s/Config/SoundBuilder.json", m_projectRoot.c_str());

    m_patternList.clear();
    LoadGlobalSetting(gemConfigPath);
    LoadGlobalSetting(projectConfigPath);

    if (m_patternList.empty())
    {
        AZ_Error("SoundBuilderSettingsManager", false,
                 "FATAL: No pattern mappings found in global settings: %s. "
                 "The gem configuration is incomplete. Cannot continue.",
                 gemConfigPath.c_str());
        return;
    }

    AZ_TracePrintf("SoundBuilderSettingsManager",
                   "Loaded global settings from: %s (%zu platforms, %zu patterns)\n",
                   gemConfigPath.c_str(), m_globalSettings.size(), m_patternList.size());
}

void SoundBuilderSettingsManager::LoadGlobalSetting(const AZStd::string settingPath)
{
    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (!fileIO || !fileIO->Exists(settingPath.c_str()))
    {
        AZ_Error("SoundBuilderSettingsManager", false,
                 "FATAL: Global settings file not found at: %s. "
                 "The gem is not properly installed. Cannot continue.",
                 settingPath.c_str());
        return;
    }

    auto loadResult = AZ::JsonSerializationUtils::ReadJsonFile(settingPath);
    if (!loadResult.IsSuccess())
    {
        AZ_Error("SoundBuilderSettingsManager", false,
                 "FATAL: Failed to load global settings from: %s. Error: %s. "
                 "Cannot continue.",
                 settingPath.c_str(), loadResult.GetError().c_str());
        return;
    }

    const rapidjson::Value& root = loadResult.GetValue();

    if (root.HasMember("platforms") && root["platforms"].IsObject())
    {
        const rapidjson::Value& platforms = root["platforms"];

        for (auto it = platforms.MemberBegin(); it != platforms.MemberEnd(); ++it)
        {
            AZStd::string platformName = it->name.GetString();
            SoundBuilderSettings settings;

            AZ::JsonDeserializerSettings deserializeSettings;
            auto result = AZ::JsonSerialization::Load(settings, it->value, deserializeSettings);

            if (result.GetProcessing() == AZ::JsonSerializationResult::Processing::Completed)
            {
                m_globalSettings[platformName] = settings;
            }
        }
    }

    // Load pattern mappings
    if (root.HasMember("patterns") && root["patterns"].IsArray())
    {
        const rapidjson::Value& patterns = root["patterns"];

        for (rapidjson::SizeType i = 0; i < patterns.Size(); ++i)
        {
            const rapidjson::Value& patternObj = patterns[i];

            PatternMapping mapping;
            AZ::JsonDeserializerSettings deserializeSettings;
            auto result = AZ::JsonSerialization::Load(mapping, patternObj, deserializeSettings);
            if (result.GetProcessing() == AZ::JsonSerializationResult::Processing::Completed)
            {
                m_patternList.push_back(mapping);
            }
            else
            {
                AZ_Warning("SoundBuilderSettingsManager", false,
                           "Failed to load pattern mapping from: %s", settingPath.c_str());
            }
        }
    }
}

void SoundBuilderSettingsManager::LoadPresets()
{
    // Load presets from gem: <gem>/Config/*.preset
    AZStd::string gemPresetsPath = AZStd::string::format("%s/Config/Sune/", m_gemRoot.c_str());
    AZStd::string projectPresetsPath = AZStd::string::format("%s/Config/Sune/", m_projectRoot.c_str());

    char resolved[AZ_MAX_PATH_LEN];

    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (!fileIO)
    {
        AZ_Error("SoundBuilderSettingsManager", false,
                 "FATAL: FileIO not available for loading presets. Cannot continue.");
        return;
    }

    AZStd::vector<AZStd::string> files;
    fileIO->FindFiles(gemPresetsPath.c_str(), "*.preset", [fileIO, &files, &resolved](const char* filename)
    {
        //resolve path using fileio
        AZStd::string resolvedPath = filename;
        if (fileIO->ResolvePath(filename, resolved, AZ_MAX_PATH_LEN))
        {
            resolvedPath = resolved;
            files.push_back(resolvedPath);
        }

        return true;
    });

    fileIO->FindFiles(projectPresetsPath.c_str(), "*.preset", [fileIO, &files, &resolved](const char* filename)
    {
        AZStd::string resolvedPath = filename;
        if (fileIO->ResolvePath(filename, resolved, AZ_MAX_PATH_LEN))
        {
            resolvedPath = resolved;
            files.push_back(resolvedPath);
        }

        return true;
    });

    AZ_TracePrintf("SoundBuilderSettingsManager", "Found %zu preset files\n", files.size());


    bool anyLoaded = false;
    for (const AZStd::string& presetPath : files)
    {
        LoadPreset(presetPath);
        anyLoaded = true;
    }

    if (!anyLoaded)
    {
        AZ_Error("SoundBuilderSettingsManager", false,
                 "FATAL: No preset files found in: %s. "
                 "The gem is not properly installed. Cannot continue.",
                 gemPresetsPath.c_str());
        return;
    }

    AZ_TracePrintf("SoundBuilderSettingsManager", "Loaded %zu presets\n",
                   m_presets.size());
}

void SoundBuilderSettingsManager::LoadPreset(const AZStd::string& presetPath)
{
    auto loadResult = AZ::JsonSerializationUtils::ReadJsonFile(presetPath);
    if (!loadResult.IsSuccess())
    {
        AZ_Warning("SoundBuilderSettingsManager", false,
                   "Failed to load preset from: %s. Error: %s",
                   presetPath.c_str(), loadResult.GetError().c_str());
        return;
    }

    MultiplatformSoundPreset preset;
    AZ::JsonDeserializerSettings settings;
    auto deserializeResult = AZ::JsonSerialization::Load(
        preset,
        loadResult.GetValue(),
        settings
    );

    if (deserializeResult.GetProcessing() != AZ::JsonSerializationResult::Processing::Completed)
    {
        auto error = deserializeResult.ToString("");
        AZ_Warning("SoundBuilderSettingsManager", false,
                   "Failed to deserialize preset from: %s JSON Error: %s", presetPath.c_str(), error.c_str());
        return;
    }

    if (!preset.m_defaultSettings.m_name.empty())
    {
        for (auto& platform : preset.m_platformOverrides)
        {
            if (platform.second.m_name.empty())
                platform.second.m_name = preset.m_defaultSettings.m_name;
            if (platform.second.m_description.empty())
                platform.second.m_description = preset.m_defaultSettings.m_description;
        }
        m_presets[preset.m_defaultSettings.m_name] = preset;
        AZ_TracePrintf("SoundBuilderSettingsManager", "Loaded preset: %s from %s\n",
                       preset.m_defaultSettings.m_name.c_str(), presetPath.c_str());
    }
}


AZStd::string SoundBuilderSettingsManager::GenerateFingerprint() const
{
    AZ::Crc32 hash;

    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (!fileIO)
    {
        return "00000000";
    }

    // Helper to hash a file's modification time and size
    auto hashFile = [&hash, fileIO](const AZStd::string& filePath)
    {
        if (fileIO->Exists(filePath.c_str()))
        {
            AZ::u64 modTime = fileIO->ModificationTime(filePath.c_str());
            AZ::u64 fileSize = 0;
            fileIO->Size(filePath.c_str(), fileSize);
            hash.Add(&modTime, sizeof(modTime));
            hash.Add(&fileSize, sizeof(fileSize));
            hash.Add(filePath.c_str(), filePath.size());
        }
    };

    AZStd::string gemConfigPath = AZStd::string::format("%s/Config/SoundBuilder.json", m_gemRoot.c_str());
    AZStd::string projectConfigPath = AZStd::string::format("%s/Config/SoundBuilder.json", m_projectRoot.c_str());
    hashFile(gemConfigPath);
    hashFile(projectConfigPath);

    AZStd::vector<AZStd::string> presetPaths;
    AZStd::string gemPresetsPath = AZStd::string::format("%s/Config/Sune/", m_gemRoot.c_str());
    AZStd::string projectPresetsPath = AZStd::string::format("%s/Config/Sune/", m_projectRoot.c_str());

    char resolved[AZ_MAX_PATH_LEN];

    // Find all preset files in gem
    fileIO->FindFiles(gemPresetsPath.c_str(), "*.preset", [fileIO, &presetPaths, &resolved](const char* filename)
    {
        AZStd::string resolvedPath = filename;
        if (fileIO->ResolvePath(filename, resolved, AZ_MAX_PATH_LEN))
        {
            resolvedPath = resolved;
            presetPaths.push_back(resolvedPath);
        }
        return true;
    });

    // Find all preset files in project
    fileIO->FindFiles(projectPresetsPath.c_str(), "*.preset", [fileIO, &presetPaths, &resolved](const char* filename)
    {
        AZStd::string resolvedPath = filename;
        if (fileIO->ResolvePath(filename, resolved, AZ_MAX_PATH_LEN))
        {
            resolvedPath = resolved;
            presetPaths.push_back(resolvedPath);
        }
        return true;
    });

    std::sort(presetPaths.begin(), presetPaths.end());

    for (const auto& presetPath : presetPaths)
    {
        hashFile(presetPath);
    }

    return AZStd::string::format("%08X", static_cast<AZ::u32>(hash));
}

SoundAssetBuilderSettings SoundBuilderSettingsManager::GetSettings(
    const AZStd::string& filePath,
    const AZStd::string& platform) const
{
    AZStd::string presetName = GetSuggestedPreset(filePath);

    // Step 2: Check for .assetinfo override
    AZStd::string assetInfoPath = filePath + ".assetinfo";
    SoundAssetSettings assetSettings;

    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (fileIO && fileIO->Exists(assetInfoPath.c_str()))
    {
        // Try to load .assetinfo file
        if (AZ::Utils::LoadObjectFromFileInPlace(assetInfoPath.c_str(), assetSettings))
        {
            // Use preset override if specified
            if (!assetSettings.m_presetName.empty())
            {
                presetName = assetSettings.m_presetName;
            }
        }
    }

    // Step 3: Get platform-specific preset
    const SoundPresetSettings* preset = GetPreset(presetName, platform);
    if (!preset)
    {
        AZ_Warning("SoundBuilderSettingsManager", false,
                   "Failed to load preset '%s', using default", presetName.c_str());
        preset = GetPreset(m_defaultPreset, platform);

        if (!preset)
        {
            // Fallback to any available preset
            if (!m_presets.empty())
            {
                preset = &m_presets.begin()->second.m_defaultSettings;
            }
        }
    }

    // Step 4: Build final settings (preset + overrides)
    SoundAssetBuilderSettings finalSettings;

    if (preset)
    {
        finalSettings.m_presetName = preset->m_name;
        finalSettings.m_format = assetSettings.m_formatOverride.value_or(preset->m_format);
        finalSettings.m_loadMethod = assetSettings.m_loadMethodOverride.value_or(preset->m_loadMethod);
        finalSettings.m_quality = assetSettings.m_qualityOverride.value_or(preset->m_quality);
        finalSettings.m_volumeAdjustment = assetSettings.m_volumeAdjustment;
    }
    else
    {
        // Ultimate fallback
        finalSettings.m_format = AudioImportFormat::Vorbis;
        finalSettings.m_loadMethod = AudioLoadMethod::DecodeOnLoad;
        finalSettings.m_quality = 0.7f;
        finalSettings.m_volumeAdjustment = 1.0f;
    }

    return finalSettings;
}

const SoundPresetSettings* SoundBuilderSettingsManager::GetPreset(
    const AZStd::string& presetName,
    const AZStd::string& platform) const
{
    auto it = m_presets.find(presetName);
    if (it == m_presets.end())
    {
        return nullptr;
    }

    return it->second.GetPreset(platform);
}

AZStd::string SoundBuilderSettingsManager::GetSuggestedPreset(const AZStd::string& filePath) const
{
    AZStd::string filename = GetFilenameWithoutExtension(filePath);

    // Try patterns in reverse order, we have the gem default pattern of '*' and users configs get loaded last
    //so we need to ensure their stuff goes first.
    for (int i = m_patternList.size() - 1; i >= 0; --i)
    {
        if (WildcardMatch(filename, m_patternList[i].m_pattern))
        {
            return m_patternList[i].m_presetName;
        }
    }

    // No match - use default
    return m_defaultPreset;
}

const SoundBuilderSettings* SoundBuilderSettingsManager::GetGlobalSettings(const AZStd::string& platform) const
{
    auto it = m_globalSettings.find(platform);
    if (it != m_globalSettings.end())
    {
        return &it->second;
    }

    // Fallback to pc
    auto pcIt = m_globalSettings.find("pc");
    if (pcIt != m_globalSettings.end())
    {
        return &pcIt->second;
    }

    return nullptr;
}

AZStd::string SoundBuilderSettingsManager::GetAnalysisFingerprint() const
{
    return m_analysisFingerprint;
}

AZStd::string SoundBuilderSettingsManager::GetFilenameWithoutExtension(const AZStd::string& filePath) const
{
    AZStd::string filename;
    AzFramework::StringFunc::Path::GetFileName(filePath.c_str(), filename);

    // Strip extension
    AzFramework::StringFunc::Path::StripExtension(filename);

    return filename;
}

bool SoundBuilderSettingsManager::WildcardMatch(
    const AZStd::string& filename,
    const AZStd::string& pattern) const
{
    // Simple wildcard matching (* matches any characters)
    // Supports: "prefix*", "*suffix", "*middle*", "exact"

    if (pattern == "*")
    {
        return true;
    }

    if (pattern.find('*') == AZStd::string::npos)
    {
        return filename == pattern;  // Exact match
    }

    // Pattern contains wildcard
    size_t starPos = pattern.find('*');

    if (starPos == 0)
    {
        // Pattern: "*suffix"
        AZStd::string suffix = pattern.substr(1);
        return filename.ends_with(suffix);
    }
    else if (starPos == pattern.length() - 1)
    {
        // Pattern: "prefix*"
        AZStd::string prefix = pattern.substr(0, starPos);
        return filename.starts_with(prefix);
    }
    else
    {
        // Pattern: "pre*fix" - check both parts
        AZStd::string prefix = pattern.substr(0, starPos);
        AZStd::string suffix = pattern.substr(starPos + 1);
        return filename.starts_with(prefix) && filename.ends_with(suffix);
    }
}