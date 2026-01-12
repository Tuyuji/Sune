/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

namespace Sune
{
    class AudioPlayerRequests : public AZ::ComponentBus
    {
        AZ_RTTI(AudioPlayerRequests, "{71AAD8A8-39FC-4428-88C4-EBDEA7248DEA}");
    public:

        virtual SoundPlayerId GetPlayerId() const = 0;
    };

    typedef AZ::EBus<AudioPlayerRequests> AudioPlayerRequestBus;
}