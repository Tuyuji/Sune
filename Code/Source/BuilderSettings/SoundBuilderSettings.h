/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

namespace Sune
{
    //Global settings for the builder, platforms can modify.
    struct SoundBuilderSettings
    {
    public:
        AZ_TYPE_INFO(SoundBuilderSettings, "{2FEE6A6C-7ED1-4799-9A81-DCE19A195D68}");
        AZ_CLASS_ALLOCATOR(SoundBuilderSettings, AZ::SystemAllocator);

        bool m_enableStreaming = true;

        static void Reflect(AZ::ReflectContext* context);
    };

    // Pattern to preset mapping entry
    struct PatternMapping
    {
    public:
        AZ_TYPE_INFO(PatternMapping, "{3DADB7B6-4F61-4927-AB72-CCFFB29879A6}");
        AZ_CLASS_ALLOCATOR(PatternMapping, AZ::SystemAllocator);

        AZStd::string m_pattern; // "*_music", "boss_*"
        AZStd::string m_presetName; // "Music"

        static void Reflect(AZ::ReflectContext* context);
    };
}
