/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/Asset/AssetManager.h>

#include <Clients/SuneSystemComponent.h>
#include <Tools/SoundAssetBuilder.h>

#include "API/ToolsApplicationAPI.h"

namespace Sune
{
    /// System component for Sune editor
    class SuneEditorSystemComponent
        : public SuneSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = SuneSystemComponent;
    public:
        AZ_COMPONENT_DECL(SuneEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        SuneEditorSystemComponent();
        ~SuneEditorSystemComponent() override;

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        void NotifyRegisterViews() override;

        SoundAssetBuilder m_soundAssetBuilder;
    };
} // namespace Sune
