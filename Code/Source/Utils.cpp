/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <cstring>
#include <Sune/Utils.h>
#include <libnyquist/Common.h>
#include <libnyquist/Decoders.h>

#include "LabSound/core/AudioBus.h"

using namespace Sune;

nqr::NyquistIO nyquist_io;

std::shared_ptr<lab::AudioBus> LoadInternal(nqr::AudioData * audioData, bool mixToMono)
{
    int numSamples = static_cast<int>(audioData->samples.size());
    if (!numSamples) return nullptr;

    int length = int(numSamples / audioData->channelCount);
    const int busChannelCount = mixToMono ? 1 : (audioData->channelCount);

    std::vector<float> planarSamples(numSamples);

    // Create AudioBus where we'll put the PCM audio data
    std::shared_ptr<lab::AudioBus> audioBus(new lab::AudioBus(busChannelCount, length));
    audioBus->setSampleRate((float) audioData->sampleRate);

    // Deinterleave stereo into LabSound/WebAudio planar channel layout
    nqr::DeinterleaveChannels(audioData->samples.data(), planarSamples.data(), length, audioData->channelCount, length);

    // Mix to mono if stereo -- easier to do in place instead of using libnyquist helper functions
    // because we've already deinterleaved
    if (audioData->channelCount == lab::Channels::Stereo && mixToMono)
    {
        float * destinationMono = audioBus->channel(0)->mutableData();
        float * leftSamples = planarSamples.data();
        float * rightSamples = planarSamples.data() + length;

        for (int i = 0; i < length; i++)
        {
            destinationMono[i] = 0.5f * (leftSamples[i] + rightSamples[i]);
        }
    }
    else
    {
        for (int i = 0; i < busChannelCount; ++i)
        {
            std::memcpy(audioBus->channel(i)->mutableData(), planarSamples.data() + (i * length), length * sizeof(float));
        }
    }

    delete audioData;

    return audioBus;
}

std::shared_ptr<lab::AudioBus> Sune::MakeBusFromAudioData(void* fileData, size_t size)
{
    auto audioData = new nqr::AudioData();
    std::vector<uint8_t> rawData((uint8_t*)fileData, (uint8_t*)fileData + size);
    nyquist_io.Load(audioData, rawData);

    auto bus = LoadInternal(audioData, false);
    if (!bus)
    {
        return nullptr;
    }

    return bus;
}
