/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <Sune/SuneTypeIds.h>
#include <SuneModuleInterface.h>
#include "SuneSystemComponent.h"

namespace Sune
{
    class SuneModule
        : public SuneModuleInterface
    {
    public:
        AZ_RTTI(SuneModule, SuneModuleTypeId, SuneModuleInterface);
        AZ_CLASS_ALLOCATOR(SuneModule, AZ::SystemAllocator);
    };
}// namespace Sune

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), Sune::SuneModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Sune, Sune::SuneModule)
#endif
