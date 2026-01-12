/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <Sune/SuneTypeIds.h>

#include <AzCore/std/string/string.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace lab
{
    class AudioNode;
}

namespace Sune
{
    typedef AZ::Crc32 AudioBusId;
    constexpr AudioBusId InvalidAudioBusId = AZ::Crc32();
    class AudioBusManagerRequests
    {
    public:
        AZ_RTTI(AudioBusManagerRequests, AudioBusManagerRequestsTypeId);
        virtual ~AudioBusManagerRequests() = default;

        virtual bool BusExists(AudioBusId id) = 0;
        virtual AudioBusId CreateBus(const AZStd::string& name) = 0;
        virtual void DeleteBus(AudioBusId id) = 0;

        virtual void IterateBuses(AZStd::function<void(AudioBusId, const AZStd::string&)> cb) = 0;
    };

    using AudioBusManagerInterface = AZ::Interface<AudioBusManagerRequests>;

    class AudioBusRequests
    {
    public:
        AZ_RTTI(AudioBusRequests, AudioBusRequestsTypeId);
        virtual ~AudioBusRequests() = default;

        virtual std::shared_ptr<lab::AudioNode> GetInputNode() = 0;
        virtual std::shared_ptr<lab::AudioNode> GetOutputNode() = 0;

        virtual AZStd::string GetName() = 0;

        virtual float GetGain() = 0;
        virtual void SetGain(float newGain) = 0;
    };

    class AudioBusRequestsBusTraits
        : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AudioBusId;
    };

    using AudioBusRequestsBus = AZ::EBus<AudioBusRequests, AudioBusRequestsBusTraits>;
}
