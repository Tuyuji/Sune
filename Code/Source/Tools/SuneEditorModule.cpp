/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <Sune/SuneTypeIds.h>
#include <SuneModuleInterface.h>
#include "SuneEditorSystemComponent.h"
#include "Components/EditorAudioPlayerComponent.h"

void InitSuneResources()
{
    Q_INIT_RESOURCE(Sune);
}

namespace Sune
{
    class SuneEditorModule
        : public SuneModuleInterface
    {
    public:
        AZ_RTTI(SuneEditorModule, SuneEditorModuleTypeId, SuneModuleInterface);
        AZ_CLASS_ALLOCATOR(SuneEditorModule, AZ::SystemAllocator);

        SuneEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                SuneEditorSystemComponent::CreateDescriptor(),
                EditorAudioPlayerComponent::CreateDescriptor()
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<SuneEditorSystemComponent>(),
            };
        }
    };
}// namespace Sune

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), Sune::SuneEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Sune_Editor, Sune::SuneEditorModule)
#endif
