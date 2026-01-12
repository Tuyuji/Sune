/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <vector>
#include <Sune/AudioBusManagerInterface.h>
#include <AzCore/std/containers/map.h>

namespace lab
{
    class AudioNode;
    class GainNode;
}

namespace Sune
{
    class Bus final
        : public AudioBusRequestsBus::Handler
    {
    public:
        Bus(AudioBusId id, const AZStd::string& name);
        ~Bus() override;

        //Client Api to connect its node into the bus
        std::shared_ptr<lab::AudioNode> GetInputNode() override;
        std::shared_ptr<lab::AudioNode> GetOutputNode() override;

        AZStd::string GetName() override;
        float GetGain() override;
        void SetGain(float newGain) override;

    private:
        friend class BusManager;
        AudioBusId m_id;
        AZStd::string m_name;

        std::shared_ptr<lab::GainNode> m_gainNode = nullptr;
    };

    class BusManager
        : public AudioBusManagerRequests
    {
    public:
        BusManager();
        ~BusManager() override;

        bool BusExists(AudioBusId id) override;
        AudioBusId CreateBus(const AZStd::string& name) override;
        void DeleteBus(AudioBusId id) override;

        void IterateBuses(AZStd::function<void(AudioBusId, const AZStd::string&)> cb) override;
    private:
        AZStd::map<AudioBusId, AZStd::unique_ptr<Bus>> m_busMap;
    };
} // Sune