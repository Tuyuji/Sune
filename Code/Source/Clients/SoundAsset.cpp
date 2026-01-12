/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <Sune/SoundAsset.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/ObjectStream.h>

using namespace Sune;

void SoundAsset::Reflect(AZ::ReflectContext* context)
{
    if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
    {
        serializeContext
            ->Enum<AudioImportFormat>()
            ->Value("Uncompressed", AudioImportFormat::Uncompressed)
            ->Value("OriginalFile", AudioImportFormat::OriginalFile)
            ->Value("Vorbis", AudioImportFormat::Vorbis);

        serializeContext->Enum<AudioLoadMethod>()
            ->Value("DecodeOnLoad", AudioLoadMethod::DecodeOnLoad)
            ->Value("DecodeOnDemand", AudioLoadMethod::DecodeOnDemand);

        serializeContext
            ->Class<SoundAsset, AZ::Data::AssetData>()
                ->Version(2)
                ->Field("m_importFormat", &SoundAsset::m_importFormat)
                ->Field("m_loadMethod", &SoundAsset::m_loadMethod)
                ->Field("m_channels", &SoundAsset::m_channels)
                ->Field("m_sampleRate", &SoundAsset::m_sampleRate)
                ->Field("m_totalSamples", &SoundAsset::m_totalSamples)
        ;

        serializeContext->RegisterGenericType<AZ::Data::Asset<SoundAsset>>();

        if (auto* editContext = serializeContext->GetEditContext())
        {
            editContext->Class<SoundAsset>("Sune SoundAsset", "")->ClassElement(AZ::Edit::ClassElements::EditorData, "");
        }
    }
}

SoundAsset::SoundAsset()
{}

bool SoundAsset::GetLengthInSeconds(float& lengthInSeconds) const
{
    if (m_totalSamples == 0 || m_channels == 0 || m_sampleRate == 0)
    {
        return false;
    }
    const auto channelSamples = m_totalSamples / static_cast<size_t>(m_channels);
    lengthInSeconds = static_cast<float>(channelSamples) / static_cast<float>(m_sampleRate);
    return true;
}
