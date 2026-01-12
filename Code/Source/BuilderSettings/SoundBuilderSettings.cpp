/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SoundBuilderSettings.h"
#include <AzCore/Serialization/SerializeContext.h>

using namespace Sune;

void SoundBuilderSettings::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;
    sc->Class<SoundBuilderSettings>()
        ->Version(0)
        ->Field("enableStreaming", &SoundBuilderSettings::m_enableStreaming)
        ;
}

void PatternMapping::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;
    sc->Class<PatternMapping>()
        ->Version(0)
        ->Field("pattern", &PatternMapping::m_pattern)
        ->Field("preset", &PatternMapping::m_presetName)
        ;
}
