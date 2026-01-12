/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "SuneTypeIds.h"

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/std/containers/vector.h>

namespace lab
{
    class AudioBus;
}

namespace Sune
{
    enum class AudioImportFormat
    {
        Uncompressed,
        OriginalFile,
        Vorbis,
    };

    enum class AudioLoadMethod
    {
        DecodeOnLoad, //Decodes the audio into memory once loaded.
        DecodeOnDemand //Decodded and straemed from disc on playback
    };

    class SoundAsset
        : public AZ::Data::AssetData
    {
    public:
        AZ_CLASS_ALLOCATOR(SoundAsset, AZ::SystemAllocator, 0);
        AZ_RTTI(SoundAsset, SuneSoundAssetTypeId, AZ::Data::AssetData);

        static constexpr const char* FileExtension = "ssa";
        static constexpr const char* AssetGroup = "Sound";
        static constexpr AZ::u32 AssetSubId = 0;

        static void Reflect(AZ::ReflectContext* context);

        SoundAsset();
        ~SoundAsset() override = default;

        AudioImportFormat m_importFormat = AudioImportFormat::Vorbis;
        AudioLoadMethod m_loadMethod = AudioLoadMethod::DecodeOnDemand;

        int m_channels = 0;
        int m_sampleRate = 0;
        size_t m_totalSamples = 0;

        //Gets set once loaded
        std::shared_ptr<lab::AudioBus> m_bus = {};

        //Decode on load
        AZStd::vector<float> m_samples;

        virtual bool GetLengthInSeconds(float& lengthInSeconds) const;
    };

    using SoundDataAsset = AZ::Data::Asset<SoundAsset>;
    using SoundDataAssetVector = AZStd::vector<AZ::Data::Asset<SoundAsset>>;


    AZ_TYPE_INFO_SPECIALIZE(Sune::AudioImportFormat, "{FA81243F-BD00-4C69-A77B-2C844DBB7F7F}");
    AZ_TYPE_INFO_SPECIALIZE(Sune::AudioLoadMethod, "{9CF5D1DF-53F6-4E48-8E34-BC0CE2001759}");
}
