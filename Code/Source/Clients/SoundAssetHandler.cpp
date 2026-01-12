/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundAssetHandler.h"

#include "Sune/SoundAsset.h"

#include <libnyquist/Common.h>
#include <libnyquist/Decoders.h>

#include "AzCore/Serialization/Utils.h"
#include "LabSound/core/AudioBus.h"

using namespace Sune;

SoundAssetHandler::SoundAssetHandler()
{
    Register();
}

SoundAssetHandler::~SoundAssetHandler()
{
    Unregister();
}

void SoundAssetHandler::Register()
{
    const bool assetManagerReady = AZ::Data::AssetManager::IsReady();
    AZ_Error("SoundAssetHandler", assetManagerReady, "Asset manager isn't ready.");
    if (assetManagerReady)
    {
        AZ::Data::AssetManager::Instance().RegisterHandler(this, AZ::AzTypeInfo<SoundAsset>::Uuid());
    }

    AZ::AssetTypeInfoBus::Handler::BusConnect(AZ::AzTypeInfo<SoundAsset>::Uuid());
}

void SoundAssetHandler::Unregister()
{
    AZ::AssetTypeInfoBus::Handler::BusDisconnect();

    if (AZ::Data::AssetManager::IsReady())
    {
        AZ::Data::AssetManager::Instance().UnregisterHandler(this);
    }
}

AZ::Data::AssetPtr SoundAssetHandler::CreateAsset(const AZ::Data::AssetId& id, const AZ::Data::AssetType& type)
{
    if (type == AZ::AzTypeInfo<SoundAsset>::Uuid())
    {
        return aznew SoundAsset();
    }

    return nullptr;
}
#include <vorbis/codec.h>

[[maybe_unused]]static bool DecodeVorbisNonInterleaved(AZStd::vector<AZ::u8>& inputOggData, AZStd::vector<float>& outputPcmData, int& channels, int& sampleRate)
{
    ogg_sync_state sync_state;
    ogg_stream_state stream_state;
    ogg_page page;
    ogg_packet packet;

    vorbis_info info;
    vorbis_comment comment;
    vorbis_dsp_state dsp_state;
    vorbis_block block;

    ogg_sync_init(&sync_state);
    vorbis_info_init(&info);
    vorbis_comment_init(&comment);

    const auto originialSize = inputOggData.size();

    char* buffer = ogg_sync_buffer(&sync_state, originialSize);
    memcpy(buffer, inputOggData.data(), originialSize);
    ogg_sync_wrote(&sync_state, originialSize);
    inputOggData = {};

    if (ogg_sync_pageout(&sync_state, &page) != 1) {
        AZ_Error("SoundAssetHandler", false, "Ogg invalid.");
        ogg_sync_clear(&sync_state);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
        return false;
    }

    ogg_stream_init(&stream_state, ogg_page_serialno(&page));

    if (ogg_stream_pagein(&stream_state, &page) < 0)
    {
        AZ_Error("SoundAssetBuilder", false, "Error reading first page of Ogg bitstream.");
        ogg_stream_clear(&stream_state);
        ogg_sync_clear(&sync_state);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
        return false;
    }

    if (ogg_stream_packetout(&stream_state, &packet) != 1)
    {
        AZ_Error("SoundAssetBuilder", false, "Error reading initial header packet.");
        ogg_stream_clear(&stream_state);
        ogg_sync_clear(&sync_state);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
        return false;
    }

    if (vorbis_synthesis_headerin(&info, &comment, &packet) < 0)
    {
        AZ_Error("SoundAssetBuilder", false, "This Ogg bitstream does not contain Vorbis audio data.");
        ogg_stream_clear(&stream_state);
        ogg_sync_clear(&sync_state);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
        return false;
    }

    int i = 0;
    while (i < 2)
    {
        while (i < 2)
        {
            int result = ogg_sync_pageout(&sync_state, &page);
            if (result == 0) break;
            if (result == 1)
            {
                ogg_stream_pagein(&stream_state, &page);
                while (i < 2)
                {
                    result = ogg_stream_packetout(&stream_state, &packet);
                    if (result == 0) break;
                    if (result < 0)
                    {
                        AZ_Error("SoundAssetBuilder", false, "Corrupt secondary header.");
                        ogg_stream_clear(&stream_state);
                        ogg_sync_clear(&sync_state);
                        vorbis_comment_clear(&comment);
                        vorbis_info_clear(&info);
                        return false;
                    }
                    result = vorbis_synthesis_headerin(&info, &comment, &packet);
                    if (result < 0)
                    {
                        AZ_Error("SoundAssetBuilder", false, "Corrupt secondary header.");
                        ogg_stream_clear(&stream_state);
                        ogg_sync_clear(&sync_state);
                        vorbis_comment_clear(&comment);
                        vorbis_info_clear(&info);
                        return false;
                    }
                    i++;
                }
            }
        }
    }

    channels = info.channels;
    sampleRate = info.rate;

    if (vorbis_synthesis_init(&dsp_state, &info) != 0)
    {
        AZ_Error("SoundAssetBuilder", false, "Failed to initialize Vorbis synthesis.");
        ogg_stream_clear(&stream_state);
        ogg_sync_clear(&sync_state);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
        return false;
    }
    vorbis_block_init(&dsp_state, &block);

    AZStd::vector<AZStd::vector<float>> channelBuffers(channels);
    int estimatedSamplesPerChannel = (originialSize * 8) / channels;
    for (int ch = 0; ch < channels; ch++)
    {
        channelBuffers[ch].reserve(estimatedSamplesPerChannel);
    }

    bool eos = false;
    while (!eos)
    {
        while (!eos)
        {
            int result = ogg_sync_pageout(&sync_state, &page);
            if (result == 0) break;

            if (result > 0)
            {
                ogg_stream_pagein(&stream_state, &page);

                while (true)
                {
                    result = ogg_stream_packetout(&stream_state, &packet);
                    if (result == 0) break;
                    if (result < 0) continue;

                    if (vorbis_synthesis(&block, &packet) == 0)
                    {
                        vorbis_synthesis_blockin(&dsp_state, &block);
                    }

                    float** pcm;
                    int samples;
                    while ((samples = vorbis_synthesis_pcmout(&dsp_state, &pcm)) > 0)
                    {
                        for (int ch = 0; ch < channels; ch++)
                        {
                            channelBuffers[ch].insert(channelBuffers[ch].end(),
                                                      pcm[ch], pcm[ch] + samples);
                        }
                        vorbis_synthesis_read(&dsp_state, samples);
                    }
                }

                if (ogg_page_eos(&page)) eos = true;
            }
        }

        if (!eos)
        {
            buffer = ogg_sync_buffer(&sync_state, 4096);
            break;
        }
    }

    int totalSamples = channelBuffers[0].size();
    outputPcmData.resize(totalSamples * channels);
    float* floatOutput = outputPcmData.data();

    for (int ch = 0; ch < channels; ch++)
    {
        memcpy(floatOutput + ch * totalSamples,
            channelBuffers[ch].data(),
            totalSamples * sizeof(float));
    }

    ogg_stream_clear(&stream_state);
    ogg_sync_clear(&sync_state);
    vorbis_comment_clear(&comment);
    vorbis_info_clear(&info);
    vorbis_block_clear(&block);
    vorbis_dsp_clear(&dsp_state);
    return true;
}

AZ::Data::AssetHandler::LoadResult SoundAssetHandler::LoadAssetData(const AZ::Data::Asset<AZ::Data::AssetData>& asset,
    AZStd::shared_ptr<AZ::Data::AssetDataStream> stream, const AZ::Data::AssetFilterCB& assetLoadFilterCB)
{
    const bool result = AZ::Utils::LoadObjectFromStreamInPlace<SoundAsset>(*stream, *asset.GetAs<SoundAsset>());
    if (result == false)
    {
        AZ_Error(__FUNCTION__, false, "Failed to load asset");
        return AssetHandler::LoadResult::Error;
    }

    SoundAsset* soundAsset = asset.GetAs<SoundAsset>();
    switch (soundAsset->m_loadMethod)
    {
    case AudioLoadMethod::DecodeOnLoad:
        return HandleDecodeOnLoad(soundAsset, stream);
    case AudioLoadMethod::DecodeOnDemand:
        return HandleDecodeOnDemand(soundAsset, stream);
    default:
        AZ_Error(__FUNCTION__, false, "Unsupported load method");
        return LoadResult::Error;
    }

    return LoadResult::LoadComplete;
}

AZ::Data::AssetHandler::LoadResult SoundAssetHandler::HandleDecodeOnLoad(SoundAsset* soundAsset, AZStd::shared_ptr<AZ::Data::AssetDataStream> stream)
{
    //Read the rest into memory
    auto start = stream->GetCurPos();
    auto readSize = stream->GetLength() - start;
    AZStd::vector<AZ::u8> fileBuffer(readSize);
    const size_t bytesRead = stream->Read(fileBuffer.size(), fileBuffer.data());
    if (bytesRead != readSize)
    {
        AZ_Error(__FUNCTION__, false, "Failed to read source file.");
        return LoadResult::Error;
    }

    switch (soundAsset->m_importFormat)
    {
    case AudioImportFormat::Vorbis:
        if (!DecodeVorbisNonInterleaved(fileBuffer, soundAsset->m_samples, soundAsset->m_channels, soundAsset->m_sampleRate))
        {
            AZ_Error(__FUNCTION__, false, "Failed to decode OGG Vorbis data.");
            return LoadResult::Error;
        }
        soundAsset->m_totalSamples = soundAsset->m_samples.size();

        //Share a universal buffer for every labsound player to share.
        //this lab::AudioBus isn't allocating anything and is just pointing at our existing memory.
        soundAsset->m_bus = std::make_shared<lab::AudioBus>(soundAsset->m_channels, soundAsset->m_totalSamples / soundAsset->m_channels, false);
        soundAsset->m_bus->setSampleRate(soundAsset->m_sampleRate);
        {
            auto length = soundAsset->m_totalSamples  / soundAsset->m_channels;
            float* data = (float*)soundAsset->m_samples.data();
            for (int i = 0; i < soundAsset->m_channels; ++i)
            {
                float* channelMemory = data + (i * length);
                soundAsset->m_bus->setChannelMemory(i, channelMemory, length);
            }
        }
        return LoadResult::LoadComplete;
    default:
        AZ_Error(__FUNCTION__, false, "Unsupported import format");
        return LoadResult::Error;
    }

    return LoadResult::Error;
}

AZ::Data::AssetHandler::LoadResult SoundAssetHandler::HandleDecodeOnDemand(SoundAsset* soundAsset, AZStd::shared_ptr<AZ::Data::AssetDataStream> )
{
    //TODO: We need to improve labsound to support streaming audio, this will be done but for now just fail to process.
    return LoadResult::Error;
}

void SoundAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
{
    SoundAsset* asset = azdynamic_cast<SoundAsset*>(ptr);
    if (!asset)
    {
        return;
    }

    if (asset->m_bus.use_count() > 1)
    {
        AZ_Error("SoundAssetHandler", false, "LabSound AudioBus is still in use.");
        //It's probably better to possibly leak memory than destroy the users ears.
        //If it just resulted in a crash, then I'd be fine with that, but playing loud static? nah.
        return;
    }

    asset->m_bus = nullptr;
    asset->m_samples.clear();
}

void SoundAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes)
{
    assetTypes.push_back(AZ::AzTypeInfo<SoundAsset>::Uuid());
}

AZ::Data::AssetType SoundAssetHandler::GetAssetType() const
{
    return AZ::AzTypeInfo<SoundAsset>::Uuid();
}

void SoundAssetHandler::GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions)
{
    extensions.push_back(SoundAsset::FileExtension);
}

const char* SoundAssetHandler::GetAssetTypeDisplayName() const
{
    return "Sound Asset (Sune Gem)";
}

const char* SoundAssetHandler::GetBrowserIcon() const
{
    return "Icons/Components/ColliderMesh.svg";
}

const char* SoundAssetHandler::GetGroup() const
{
    return "Sound";
}

AZ::Uuid SoundAssetHandler::GetComponentTypeId() const
{
    // return AZ::Uuid(EditorMiniAudioPlaybackComponentTypeId);
    return AZ::Uuid::CreateNull();
}

bool SoundAssetHandler::CanCreateComponent(const AZ::Data::AssetId& assetId) const
{
    return false;
}