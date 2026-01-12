/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SuneModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <Sune/SuneTypeIds.h>

#include <Clients/SuneSystemComponent.h>

#include "Clients/Components/AudioPlayerComponent.h"
#include "Clients/Components/VisualizerComponent.h"

namespace Sune
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(SuneModuleInterface,
        "SuneModuleInterface", SuneModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(SuneModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(SuneModuleInterface, AZ::SystemAllocator);

    SuneModuleInterface::SuneModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            SuneSystemComponent::CreateDescriptor(),
            AudioPlayerComponent::CreateDescriptor(),
            VisualizerComponent::CreateDescriptor()
            });
    }

    AZ::ComponentTypeList SuneModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<SuneSystemComponent>(),
        };
    }
} // namespace Sune
