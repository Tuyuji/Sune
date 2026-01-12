/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "BusManager.h"

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "Sune/SuneBus.h"
#include <vector>
#include <algorithm>

#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/GainNode.h"

using namespace Sune;

Bus::Bus(AudioBusId id, const AZStd::string& name)
    : m_id(id)
    , m_name(name)
{
    auto ctx = SuneInterface::Get()->GetLabContext();
    AudioBusRequestsBus::Handler::BusConnect(m_id);
    m_gainNode = std::make_shared<lab::GainNode>(*ctx);

    if (ctx->destinationNode())
    {
        AZ_Info("Sune", "Bus '%s' connecting to destination node\n", name.c_str());
        ctx->connect(ctx->destinationNode(), m_gainNode);
    }
    else
    {
        AZ_Error("Sune", false, "Bus '%s' - NO DESTINATION NODE FOUND! Audio will not play!\n", name.c_str());
    }
}

Bus::~Bus()
{
    AudioBusRequestsBus::Handler::BusDisconnect();
}

std::shared_ptr<lab::AudioNode> Bus::GetInputNode()
{
    return m_gainNode;
}

std::shared_ptr<lab::AudioNode> Bus::GetOutputNode()
{
    return m_gainNode;
}

AZStd::string Bus::GetName()
{
    return m_name;
}

float Bus::GetGain()
{
    return m_gainNode->gain()->value();
}

void Bus::SetGain(float newGain)
{
    m_gainNode->gain()->setValue(newGain);
}


BusManager::BusManager()
{
    AudioBusManagerInterface::Register(this);
}

BusManager::~BusManager()
{
    AudioBusManagerInterface::Unregister(this);
}

bool BusManager::BusExists(AudioBusId id)
{
    return m_busMap.contains(id);
}

AudioBusId BusManager::CreateBus(const AZStd::string& name)
{
    const auto id = AudioBusId(name);
    if (BusExists(id))
    {
        return id;
    }

    m_busMap[id] = AZStd::make_unique<Bus>(id, name);
    return id;
}


void BusManager::DeleteBus(AudioBusId id)
{
    if (BusExists(id))
    {
        m_busMap.erase(id);
    }
}

void BusManager::IterateBuses(AZStd::function<void(AudioBusId, const AZStd::string&)> cb)
{
    for (const auto& [id, bus] : m_busMap)
    {
        cb(id, bus->m_name);
    }
}
