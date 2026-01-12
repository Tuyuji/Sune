/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetTypeInfoBus.h>
#include <AzCore/Asset/AssetManager.h>

namespace Sune
{
    class SoundAsset;
    class SoundAssetHandler
        : public AZ::Data::AssetHandler
        , public AZ::AssetTypeInfoBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(SoundAssetHandler, AZ::SystemAllocator, 0);

        SoundAssetHandler();
        ~SoundAssetHandler() override;

        void Register();
        void Unregister();

        AZ::Data::AssetPtr CreateAsset(const AZ::Data::AssetId& id, const AZ::Data::AssetType& type) override;
        LoadResult LoadAssetData(const AZ::Data::Asset<AZ::Data::AssetData>& asset, AZStd::shared_ptr<AZ::Data::AssetDataStream> stream, const AZ::Data::AssetFilterCB& assetLoadFilterCB) override;
        void DestroyAsset(AZ::Data::AssetPtr ptr) override;
        void GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes) override;

        AZ::Data::AssetType GetAssetType() const override;
        void GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions) override;
        const char* GetAssetTypeDisplayName() const override;
        const char* GetBrowserIcon() const override;
        const char* GetGroup() const override;
        AZ::Uuid GetComponentTypeId() const override;
        bool CanCreateComponent(const AZ::Data::AssetId& assetId) const override;

    private:
        LoadResult HandleDecodeOnLoad(SoundAsset* soundAsset, AZStd::shared_ptr<AZ::Data::AssetDataStream> );
        LoadResult HandleDecodeOnDemand(SoundAsset* soundAsset, AZStd::shared_ptr<AZ::Data::AssetDataStream> );
    };
}
