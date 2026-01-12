/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundAssetBuilder.h"

#include <fstream>

#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AzCore/Asset/AssetDataStream.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/IOUtils.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include "Sune/SoundAsset.h"
#include <libnyquist/Common.h>
#include <libnyquist/Decoders.h>

#include "AssetBuilderSDK/SerializationDependencies.h"
#include "AzCore/Math/Random.h"

//vorbis
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>

#include "BuilderSettings/SoundAssetSettings.h"
#include "BuilderSettings/SoundBuilderSettingsManager.h"

using namespace Sune;

AZ::SimpleLcgRandom tuls_random;

void SoundAssetBuilder::CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request,
    AssetBuilderSDK::CreateJobsResponse& response) const
{
	AssetBuilderSDK::SourceFileDependency globalConfigDep;
    globalConfigDep.m_sourceFileDependencyPath = "@gemroot:Sune@/Config/SoundBuilder.json";
    globalConfigDep.m_sourceDependencyType = AssetBuilderSDK::SourceFileDependency::SourceFileDependencyType::Absolute;

	AssetBuilderSDK::SourceFileDependency globalProjectConfigDep;
	globalProjectConfigDep.m_sourceFileDependencyPath = "@projectroot@/Config/SoundBuilder.json";
    globalProjectConfigDep.m_sourceDependencyType = AssetBuilderSDK::SourceFileDependency::SourceFileDependencyType::Absolute;

    response.m_sourceFileDependencyList.push_back(globalConfigDep);
	response.m_sourceFileDependencyList.push_back(globalProjectConfigDep);

	AssetBuilderSDK::SourceFileDependency presetDep;
	presetDep.m_sourceFileDependencyPath = "@gemroot:Sune@/Config/Sune/*.preset";
	presetDep.m_sourceDependencyType = AssetBuilderSDK::SourceFileDependency::SourceFileDependencyType::Wildcards;
	response.m_sourceFileDependencyList.push_back(presetDep);

	AssetBuilderSDK::SourceFileDependency presetProjDep;
	presetProjDep.m_sourceFileDependencyPath = "@projectroot@/Config/Sune/*.preset";
	presetProjDep.m_sourceDependencyType = AssetBuilderSDK::SourceFileDependency::SourceFileDependencyType::Wildcards;
	response.m_sourceFileDependencyList.push_back(presetProjDep);

    for (const AssetBuilderSDK::PlatformInfo& platformInfo : request.m_enabledPlatforms)
    {
        AssetBuilderSDK::JobDescriptor jobDescriptor;
        jobDescriptor.m_critical = true;
        jobDescriptor.m_jobKey = "Sune SoundAsset";
        jobDescriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

        response.m_createJobOutputs.push_back(jobDescriptor);
    }

    response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
}

void SoundAssetBuilder::ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request,
    AssetBuilderSDK::ProcessJobResponse& response) const
{
	SoundBuilderSettingsManager* settingsManager = SoundBuilderSettingsManager::Get();
	if (settingsManager == nullptr)
	{
		response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
		AZ_Error("SoundAssetBuilder", false, "Failed to get SoundBuilderSettingsManager.");
		return;
	}

    const AZStd::string& fromFile = request.m_fullPath;

	auto settings = settingsManager->GetSettings(fromFile, request.m_platformInfo.m_identifier);
	AZ_Info("SoundAssetBuilder",
		"Processing sound asset using preset '%s' quality: %.2f",
		settings.m_presetName.c_str(), settings.m_quality);

	AZStd::vector<AZ::u8> rawAudioData;
    AZ::Data::Asset<SoundAsset> soundAsset;
    soundAsset.Create(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));

    auto assetDataStream = AZStd::make_shared<AZ::Data::AssetDataStream>();
    {
		nqr::NyquistIO nyquist_io;

        AZ::IO::FileIOStream stream(fromFile.c_str(), AZ::IO::OpenMode::ModeRead);
        if (!AZ::IO::RetryOpenStream(stream))
        {
            AZ_Error("SoundAssetBuilder", false, "Source file '%s' could not be opened.", fromFile.c_str());
            return;
        }

        AZStd::vector<AZ::u8> fileBuffer(stream.GetLength());
        const size_t bytesRead = stream.Read(fileBuffer.size(), fileBuffer.data());
        if (bytesRead != stream.GetLength())
        {
            AZ_Error("SoundAssetBuilder", false, "Source file '%s' could not be read.", fromFile.c_str());
            return;
        }

    	auto audioData = AZStd::make_unique<nqr::AudioData>();
    	nyquist_io.Load(audioData.get(), std::vector<uint8_t>{fileBuffer.data(), fileBuffer.data() + fileBuffer.size()});
    	if (audioData->samples.empty())
    	{
    		AZ_Error("SoundAssetBuilder", false, "Failed to load audio data, failed to decode source.");
    		return;
    	}

        soundAsset->m_importFormat = settings.m_format;
    	soundAsset->m_loadMethod = settings.m_loadMethod;
		soundAsset->m_channels = audioData->channelCount;
		soundAsset->m_sampleRate = audioData->sampleRate;
		soundAsset->m_totalSamples = audioData->samples.size();

		if (settings.m_volumeAdjustment != 1.0f)
		{
			AZ_Warning("SoundAssetBuilder", false, "Volume adjustment isn't supported yet.");
		}

    	switch (soundAsset->m_importFormat)
    	{
    		case AudioImportFormat::OriginalFile:
    			rawAudioData = AZStd::move(fileBuffer);
    			break;
    	case AudioImportFormat::Vorbis:
    			rawAudioData = CompressVorbis(audioData.get(), settings);
    			if (rawAudioData.empty())
    			{
    				AZ_Error("SoundAssetBuilder", false, "Failed to compress file '%s' to OGG Vorbis.", fromFile.c_str());
    				response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
    				return;
    			}
    			break;
    		default:
    			AZ_Error("SoundAssetBuilder", false, "Unknown import format.");
				response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
    			return;
    	}
    }

	AZStd::string filename;
	AzFramework::StringFunc::Path::GetFileName(request.m_sourceFile.c_str(), filename);

	AZStd::string outputExtension = SoundAsset::FileExtension;

	AzFramework::StringFunc::Path::ReplaceExtension(filename, outputExtension.c_str());

	AZStd::string outputPath;
	AzFramework::StringFunc::Path::ConstructFull(request.m_tempDirPath.c_str(), filename.c_str(), outputPath, true);

	AZ::IO::FileIOStream dataStream(outputPath.c_str(), AZ::IO::OpenMode::ModeWrite | AZ::IO::OpenMode::ModeBinary);
	if (!AZ::IO::RetryOpenStream(dataStream))
	{
		AZ_Error("SoundAssetBuilder", false, "Failed to open file '%s' for writing.", outputPath.c_str());
		response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
		return;
	}

	if (!AZ::Utils::SaveObjectToStream(dataStream, AZ::DataStream::ST_BINARY, soundAsset.Get()))
	{
		AZ_Error("SoundAssetBuilder", false, "Failed to save Sune metadata to file '%s'!", outputPath.c_str());
		response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
		return;
	}

	size_t bytesWritten = dataStream.Write(rawAudioData.size(), rawAudioData.data());
	if (bytesWritten != rawAudioData.size())
	{
		AZ_Error("SoundAssetBuilder", false, "Failed to write raw audio data to file '%s'.", outputPath.c_str());
		response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
		return;
	}
	dataStream.Close();

	AssetBuilderSDK::JobProduct soundJobProduct;
	if (!AssetBuilderSDK::OutputObject(
			soundAsset.Get(), outputPath, azrtti_typeid<SoundAsset>(), SoundAsset::AssetSubId, soundJobProduct))
	{
		AZ_Error("SoundAssetBuilder", false, "Failed to output product dependencies.");
		response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
	}

	AZStd::vector<AZStd::string> configFiles = {
		"@gemroot:Sune@/Config/SoundBuilder.json",
		"@gemroot:Sune@/Config/Sune/*.preset",
		"@projectroot@/Config/SoundBuilder.json",
		"@projectroot@/Config/Sune/*.preset"
	};

	for (const auto& configFile : configFiles)
	{
		AssetBuilderSDK::ProductPathDependency dep;
		dep.m_dependencyPath = configFile;
		dep.m_dependencyType = AssetBuilderSDK::ProductPathDependencyType::SourceFile;
		soundJobProduct.m_pathDependencies.emplace(dep);
	}

	response.m_outputProducts.push_back(AZStd::move(soundJobProduct));

	response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
}

void SoundAssetBuilder::ShutDown()
{}

AZStd::vector<AZ::u8> SoundAssetBuilder::CompressVorbis(const nqr::AudioData* audioData, const SoundAssetBuilderSettings& settings) const
{
	AZStd::vector<AZ::u8> encodedData;

	vorbis_info vi;
	vorbis_comment vc;
	vorbis_dsp_state vd;
	vorbis_block vb;

	ogg_stream_state os;
	ogg_page og;
	ogg_packet op;

	vorbis_info_init(&vi);

	float quality = settings.m_quality;
	int ret = vorbis_encode_init_vbr(&vi, audioData->channelCount, audioData->sampleRate, quality);

	if (ret != 0)
	{
		AZ_Error("SoundAssetBuilder", false, "Failed to initialize vorbis encoder.");
		vorbis_info_clear(&vi);
		return encodedData;
	}

	vorbis_comment_init(&vc);
	vorbis_comment_add_tag(&vc, "ENCODER", "Sune");

	vorbis_analysis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);

	ogg_stream_init(&os, tuls_random.GetRandom());

	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;

	vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
	ogg_stream_packetin(&os, &header);
	ogg_stream_packetin(&os, &header_comm);
	ogg_stream_packetin(&os, &header_code);

	while (ogg_stream_flush(&os, &og))
	{
		encodedData.insert(encodedData.end(),
						  reinterpret_cast<AZ::u8*>(og.header),
						  reinterpret_cast<AZ::u8*>(og.header) + og.header_len);
		encodedData.insert(encodedData.end(),
						  reinterpret_cast<AZ::u8*>(og.body),
						  reinterpret_cast<AZ::u8*>(og.body) + og.body_len);
	}

	constexpr int BUFFER_SIZE = 1024;
	int samples_per_channel = audioData->samples.size() / audioData->channelCount;

	for (int i = 0; i < samples_per_channel; i += BUFFER_SIZE)
	{
		int samples_to_process = AZStd::min(BUFFER_SIZE, samples_per_channel - i);

		float** buffer = vorbis_analysis_buffer(&vd, samples_to_process);

		//non interleaved (converting from interleaved input)
		for (int ch = 0; ch < audioData->channelCount; ch++) {
			for (int j = 0; j < samples_to_process; j++) {
				buffer[ch][j] = audioData->samples[(i + j) * audioData->channelCount + ch];
			}
		}

		vorbis_analysis_wrote(&vd, samples_to_process);

		while (vorbis_analysis_blockout(&vd, &vb) == 1)
		{
			vorbis_analysis(&vb, NULL);
			vorbis_bitrate_addblock(&vb);

			while (vorbis_bitrate_flushpacket(&vd, &op))
			{
				ogg_stream_packetin(&os, &op);

				while (ogg_stream_pageout(&os, &og))
				{
					encodedData.insert(encodedData.end(),
									  reinterpret_cast<AZ::u8*>(og.header),
									  reinterpret_cast<AZ::u8*>(og.header) + og.header_len);
					encodedData.insert(encodedData.end(),
									  reinterpret_cast<AZ::u8*>(og.body),
									  reinterpret_cast<AZ::u8*>(og.body) + og.body_len);
				}
			}
		}
	}

	vorbis_analysis_wrote(&vd, 0);

	while (vorbis_analysis_blockout(&vd, &vb) == 1) {
		vorbis_analysis(&vb, NULL);
		vorbis_bitrate_addblock(&vb);

		while (vorbis_bitrate_flushpacket(&vd, &op)) {
			ogg_stream_packetin(&os, &op);

			while (ogg_stream_flush(&os, &og)) {
				encodedData.insert(encodedData.end(),
								  reinterpret_cast<AZ::u8*>(og.header),
								  reinterpret_cast<AZ::u8*>(og.header) + og.header_len);
				encodedData.insert(encodedData.end(),
								  reinterpret_cast<AZ::u8*>(og.body),
								  reinterpret_cast<AZ::u8*>(og.body) + og.body_len);
			}
		}
	}

	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);

	return encodedData;
}
