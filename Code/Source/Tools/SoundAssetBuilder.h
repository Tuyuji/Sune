/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AssetBuilderSDK/AssetBuilderBusses.h>

#include "Sune/SuneTypeIds.h"

namespace nqr
{
    struct AudioData;
}

namespace Sune
{
    struct SoundAssetBuilderSettings;
    class SoundAssetBuilder
        : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        AZ_RTTI(SoundAssetBuilder, SuneSoundAssetBuilderTypeId);

        SoundAssetBuilder() = default;

        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;
        void ShutDown() override;

        //Util
        AZStd::vector<AZ::u8> CompressVorbis(const nqr::AudioData* audioData, const SoundAssetBuilderSettings& settings) const;
    };
}
