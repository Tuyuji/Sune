 /*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SuneSystemComponent.h"

#include <Sune/SuneTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include <LabSound/LabSound.h>
#include <LabSound/backends/AudioDevice_Miniaudio.h>

#include "SoundAssetHandler.h"
#include "AzCore/Math/Sfmt.h"
#include "AzCore/Settings/SettingsRegistry.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "imgui/imgui.h"
#include "Sune/SoundAsset.h"
#include "SoundPlayer.h"
#include "AzFramework/Components/CameraBus.h"
#include "Sune/Utils.h"
#include <Sune/PlayerAudioEffect.h>
#include <AzCore/std/function/function_template.h>

#include "Effects/LabHrtfEffect.h"
#include "Effects/RadioEffect.h"
#include "Effects/VisualizerEffect.h"
#include "Sune/AudioPlayerBus.h"

namespace Sune
{
    AZ_COMPONENT_IMPL(SuneSystemComponent, "SuneSystemComponent",
        SuneSystemComponentTypeId);

    void SuneSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        SoundAsset::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SuneSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            AZ_SUNE_ID_SERIALIZE_REFLECT(SoundPlayerId);
            AZ_SUNE_ID_SERIALIZE_REFLECT(PlayerEffectId);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            AZ_SUNE_ID_BEHAVIOR_REFLECT(SoundPlayerId, "Sune");
            AZ_SUNE_ID_BEHAVIOR_REFLECT(PlayerEffectId, "Sune");

            behaviorContext->EBus<SuneRequestBus>("SuneRequestBus")
                ->Attribute(AZ::Script::Attributes::Module, "Sune")
                ->Event("CreatePlayer", &SuneRequestBus::Events::CreatePlayer)
                ->Event("DestroyPlayer", &SuneRequestBus::Events::DestroyPlayer)
                ;

            behaviorContext->EBus<SoundPlayerRequestBus>("TuSoundPlayerRequestBus")
                ->Attribute(AZ::Script::Attributes::Module, "Sune")
                // Asset and Bus Configuration
                ->Event("SetBus", &SoundPlayerRequestBus::Events::SetBus,
                    {{{"Bus", "Name of the bus this player should route its audio to."}}})
                ->Event("SetAsset", &SoundPlayerRequestBus::Events::SetAsset,
                    {{{"AssetId", "Asset ID of the sound file to load and play."}}})
                ->Event("GetAsset", &SoundPlayerRequestBus::Events::GetAsset)
                // Playback Behavior
                ->Event("SetPlayMultiple", &SoundPlayerRequestBus::Events::SetPlayMultiple,
                    {{{"CanPlayMultiple", "If false, the player will always stop what's currently playing and queue a new playback."}}})
                // Volume Control
                ->Event("SetGain", &SoundPlayerRequestBus::Events::SetGain,
                    {{{"Gain", "Volume multiplier (0.0 = silent, 1.0 = normal, >1.0 = amplified)."}}})
                ->Event("GetGain", &SoundPlayerRequestBus::Events::GetGain)
                // Asset Information
                ->Event("GetLengthInSeconds", &SoundPlayerRequestBus::Events::GetLengthInSeconds)
                ->Event("GetSampleRate", &SoundPlayerRequestBus::Events::GetSampleRate)
                // Playback Controls
                ->Event("Play", &SoundPlayerRequestBus::Events::Play)
                ->Event("PlayAtSeconds", &SoundPlayerRequestBus::Events::PlayAtSeconds,
                    {{{"Seconds", "Starting position in seconds to begin playback from.", AZ::BehaviorDefaultValuePtr(aznew AZ::BehaviorDefaultValue(0.0f))}}})
                ->Event("PlayLooping", &SoundPlayerRequestBus::Events::PlayLooping,
                    {{{"LoopCount", "Number of times to loop the sound (-1 for infinite).", AZ::BehaviorDefaultValuePtr(aznew AZ::BehaviorDefaultValue(-1))},
                      {"Seconds", "Starting position in seconds to begin playback from.", AZ::BehaviorDefaultValuePtr(aznew AZ::BehaviorDefaultValue(0.0f))}}})
                ->Event("StopAll", &SoundPlayerRequestBus::Events::StopAll)
                // Playback State
                ->Event("IsPlaying", &SoundPlayerRequestBus::Events::IsPlaying)
                ->Event("GetPositionInSeconds", &SoundPlayerRequestBus::Events::GetPositionInSeconds)
                ->Event("GetPositionInMicroseconds", &SoundPlayerRequestBus::Events::GetPositionInMicroseconds)
                // Audio Effects
                ->Event("AddEffect", &SoundPlayerRequestBus::Events::AddEffect,
                    {{{"EffectName", "Name of the registered effect to add (e.g., 'labhrtf', 'radio', 'visualizer')."}}})
                ->Event("RemoveEffect", &SoundPlayerRequestBus::Events::RemoveEffect,
                    {{{"Id", "Effect ID returned from AddEffect to remove."}}})
                ->Event("GetSpatializationEffectId", &SoundPlayerRequestBus::Events::GetSpatializationEffectId)
                ;

            behaviorContext->EBus<AudioPlayerRequestBus>("AudioPlayerRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "Sune")
                ->Event("GetPlayerId", &AudioPlayerRequestBus::Events::GetPlayerId)
            ;
        }
    }

    void SuneSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("SuneService"));
    }

    void SuneSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("SuneService"));
    }

    void SuneSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void SuneSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    SuneSystemComponent::SuneSystemComponent()
    {
        if (SuneInterface::Get() == nullptr)
        {
            SuneInterface::Register(this);
        }

        ImGui::ImGuiUpdateListenerBus::Handler::BusConnect();

        PlayerEffectFactoryBus::MultiHandler::BusConnect("labhrtf");
        PlayerEffectFactoryBus::MultiHandler::BusConnect("radio");
        PlayerEffectFactoryBus::MultiHandler::BusConnect("visualizer");
    }

    SuneSystemComponent::~SuneSystemComponent()
    {
        ImGui::ImGuiUpdateListenerBus::Handler::BusDisconnect();

        m_players.clear();
        if (SuneInterface::Get() == this)
        {
            SuneInterface::Unregister(this);
        }
    }

    SoundPlayerId SuneSystemComponent::CreatePlayer()
    {
        AZ::Sfmt& smft = AZ::Sfmt::GetInstance();
        SoundPlayerId id(smft.Rand64());
        auto player = AZStd::make_shared<SoundPlayer>(id);
        m_players[id] = player;
        return id;
    }

    void SuneSystemComponent::DestroyPlayer(SoundPlayerId id)
    {
        m_players.erase(id);
    }

    IPlayerAudioEffect* SuneSystemComponent::CreateEffect(const AZStd::string& name)
    {
        if (name == "labhrtf")
            return aznew LabHrtfEffect();
        if (name == "radio")
            return aznew RadioEffect();
        if (name == "visualizer")
            return aznew VisualizerEffect();

        return nullptr;
    }

    void SuneSystemComponent::Init()
    {
    }

    static void LabSoundLogCallback(void*, int level, const char* message)
    {
        switch (level)
        {
        default:
        case LOGLEVEL_TRACE:
        case LOGLEVEL_DEBUG:
        case LOGLEVEL_INFO:
            printf("LabSound: %s\n", message);
            break;
        case LOGLEVEL_WARN:
            printf("LabSound: %s\n", message);
            break;
        case LOGLEVEL_ERROR:
            printf("LabSound: %s\n", message);
        case LOGLEVEL_FATAL:
            printf("LabSound: %s\n", message);
            break;
        }
    }

    void SuneSystemComponent::Activate()
    {
        SuneRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();

        //Stops LS from outputting to stderr
        log_set_quiet(true);
        //Log to our callback
        log_set_write(LabSoundLogCallback);
        log_set_level(LOGLEVEL_TRACE);

        bool bEnableOutput = true;

        auto settingsRegistry = AZ::SettingsRegistry::Get();
        if (settingsRegistry)
        {
            settingsRegistry->Get(bEnableOutput, "/Audio/Enabled");
        }

        const lab::AudioDeviceInfo* defaultOutput = nullptr;
        const lab::AudioDeviceInfo* defaultInput = nullptr;

        if (bEnableOutput)
        {
            const std::vector<lab::AudioDeviceInfo> audioDevices = lab::AudioDevice_Miniaudio::MakeAudioDeviceList();
            for (const auto& device : audioDevices)
            {
                if (device.is_default_output)
                {
                    defaultOutput = &device;
                }
                if (device.is_default_input)
                {
                    defaultInput = &device;
                }
            }

            if (defaultOutput == nullptr || defaultOutput->num_output_channels < 1)
            {
                AZ_Error("Sune", false, "No default output device found, stopping.\n");
                return;
            }

            AZ_Info("Sune", "Using default output device: %s\n", defaultOutput->identifier.c_str());
            if (defaultInput != nullptr)
            {
                AZ_Info("Sune", "Using default input device: %s\n", defaultInput->identifier.c_str());
            }else
            {
                AZ_Error("Sune", false, "No default input device found\n");
            }

            lab::AudioStreamConfig config = {};
            lab::AudioStreamConfig inputConfig = {};
            config.desired_channels = 2;
            config.device_index = defaultOutput->index;
            config.desired_samplerate = defaultOutput->nominal_samplerate;

            if (defaultInput != nullptr)
            {
                inputConfig.desired_channels = 2;
                inputConfig.device_index = -1;
                inputConfig.desired_samplerate = defaultInput->nominal_samplerate;
            }

            m_device = std::make_shared<lab::AudioDevice_Miniaudio>(config, inputConfig);
        }

        m_context = std::make_shared<lab::AudioContext>(!bEnableOutput, true);
        if (m_device)
        {
            m_destination = std::make_shared<lab::AudioDestinationNode>(*m_context, m_device);
            m_device->setDestinationNode(m_destination);
            m_context->setDestinationNode(m_destination);
        }

        m_context->synchronizeConnections();

        //TODO: Make a patch to LabSound to support a custom loader so we can use the Asset System.
        // m_context->loadHrtfDatabase("/home/drogonmar/Desktop/LabSound/assets/hrtf");

        m_busManager = AZStd::make_shared<BusManager>();

        //create default bus
        m_busManager->CreateBus("Default");

        {
            SoundAssetHandler* handler  = aznew SoundAssetHandler();
            AZ::Data::AssetCatalogRequestBus::Broadcast(
                &AZ::Data::AssetCatalogRequests::EnableCatalogForAsset, AZ::AzTypeInfo<SoundAsset>::Uuid());
            AZ::Data::AssetCatalogRequestBus::Broadcast(&AZ::Data::AssetCatalogRequests::AddExtension, SoundAsset::FileExtension);
            m_assetHandlers.emplace_back(handler);
        }
    }

    void SuneSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        SuneRequestBus::Handler::BusDisconnect();

        if (!m_players.empty())
        {
            AZ_Warning("Sune", false, "There are still %zu players active, forcibly destroying them during shutdown.", m_players.size());
        }

        m_players.clear();
        m_busManager.reset();

        m_assetHandlers.clear();

        m_destination.reset();
        m_context.reset();
        m_device.reset();
    }

    void SuneSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        AZ::Transform cameraTransform;
        Camera::ActiveCameraRequestBus::BroadcastResult(cameraTransform, &Camera::ActiveCameraRequestBus::Events::GetActiveCameraTransform);
        auto position = cameraTransform.GetTranslation();
        auto upVector = cameraTransform.GetBasisZ();
        auto forwardVector = cameraTransform.GetBasisY();

        auto listener = m_context->listener();

        listener->setPosition(ToLab(position));

        listener->setForward(ToLab(forwardVector));
        listener->setUpVector(ToLab(upVector));
    }

    static bool g_igShowPlayers = false;
    static bool g_igShowLabSoundBus = false;
    void SuneSystemComponent::OnImGuiMainMenuUpdate()
    {
        if (ImGui::BeginMenu("Sune"))
        {
            ImGui::MenuItem("SoundPlayers", nullptr, &g_igShowPlayers);
            ImGui::MenuItem("Bus", nullptr, &g_igShowLabSoundBus);
            ImGui::EndMenu();
        }
    }

    static void BuildNomnomlGraph(lab::ContextRenderLock* lock, lab::AudioNode* node, AZStd::string& output, AZStd::set<uintptr_t>& visitedNodes)
    {
        if (!node)
            return;

        uintptr_t nodePtr = reinterpret_cast<uintptr_t>(node);

        if (visitedNodes.contains(nodePtr))
            return;

        visitedNodes.insert(nodePtr);

        AZStd::string nodeId = AZStd::string::format("%s_%04lx", node->name(), nodePtr & 0xFFFF);

        AZStd::string nodeLabel = AZStd::string::format("[%s", nodeId.c_str());

        auto params = node->params();
        if (!params.empty())
        {
            nodeLabel += "|";
            bool first = true;
            for (auto param : params)
            {
                if (!first) nodeLabel += "; ";
                nodeLabel += AZStd::string::format("%s: %.2f", param->name().c_str(), param->value());
                first = false;
            }
        }
        nodeLabel += "]\n";

        output += nodeLabel;

        for (int i = 0; i < node->numberOfInputs(); ++i)
        {
            auto input = node->input(i);
            if (!input)
                continue;

            int c = input->numberOfRenderingConnections(*lock);
            for (int j = 0; j < c; ++j)
            {
                lab::AudioNode* sourceNode = input->renderingOutput(*lock, j)->sourceNode();
                if (!sourceNode)
                    continue;

                uintptr_t sourcePtr = reinterpret_cast<uintptr_t>(sourceNode);
                AZStd::string sourceId = AZStd::string::format("%s_%04lx", sourceNode->name(), sourcePtr & 0xFFFF);

                if (!visitedNodes.contains(sourcePtr))
                {
                    BuildNomnomlGraph(lock, sourceNode, output, visitedNodes);
                }

                output += AZStd::string::format("[%s] -> [%s]\n", sourceId.c_str(), nodeId.c_str());
            }
        }
    }

    [[maybe_unused]]static AZStd::string GenerateNomnomlGraph(lab::AudioContext* context)
    {
        if (!context || !context->destinationNode())
            return "";

        AZStd::string output = "#direction: right\n#spacing: 40\n#padding: 10\n\n";
        AZStd::set<uintptr_t> visitedNodes;

        lab::ContextRenderLock lock(context, "GenerateNomnomlGraph");
        BuildNomnomlGraph(&lock, context->destinationNode().get(), output, visitedNodes);

        return output;
    }

    void SuneSystemComponent::OnImGuiUpdate()
    {
        if (g_igShowPlayers)
        {
            if (ImGui::Begin("SoundPlayers"))
            {
                ImGui::Text("Active Players: %zu", m_players.size());
                ImGui::Separator();

                static SoundPlayerId selectedPlayer;

                // Left pane: Player list
                ImGui::BeginChild("PlayerList", ImVec2(230, 0), true);
                {
                    for (const auto& pair : m_players)
                    {
                        const auto& playerId = pair.first;
                        const auto& player = pair.second;
                        ImGui::PushID(static_cast<AZ::u64>(playerId));

                        AZStd::string idHex = AZStd::string::format("%08llX", static_cast<AZ::u64>(playerId));

                        bool isSelected = (selectedPlayer == playerId);
                        AZStd::string label = AZStd::string::format("%s", idHex.c_str());

                        if (ImGui::Selectable(label.c_str(), isSelected))
                        {
                            selectedPlayer = playerId;
                        }

                        if (player->IsPlaying())
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Playing]");
                        }

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // Right pane: Player details
                ImGui::BeginChild("PlayerDetails", ImVec2(0, 0), false);
                {
                    if (selectedPlayer.IsValid() && m_players.contains(selectedPlayer))
                    {
                        auto& player = m_players[selectedPlayer];

                        ImGui::Text("Player ID: %s", selectedPlayer.ToString().c_str());
                        ImGui::Separator();

                        // Playback controls
                        if (ImGui::CollapsingHeader("Playback", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            bool isPlaying = player->IsPlaying();
                            ImGui::Text("Status: %s", isPlaying ? "Playing" : "Stopped");

                            if (isPlaying)
                            {
                                ImGui::Text("Position: %.2fs", player->GetPositionInSeconds());
                            }

                            float gain = player->GetGain();
                            if (ImGui::SliderFloat("Gain", &gain, 0.0f, 2.0f))
                            {
                                player->SetGain(gain);
                            }

                            ImGui::Spacing();

                            if (ImGui::Button("Play"))
                                player->Play();
                            ImGui::SameLine();
                            if (ImGui::Button("Stop"))
                                player->StopAll();
                        }

                        // Asset info
                        if (ImGui::CollapsingHeader("Asset"))
                        {
                            auto assetId = player->GetAsset();
                            if (assetId.IsValid())
                            {
                                ImGui::Text("Asset ID: %s", assetId.ToString<AZStd::string>().c_str());
                                ImGui::Text("Length: %.2fs", player->GetLengthInSeconds());
                                ImGui::Text("Sample Rate: %d Hz", player->GetSampleRate());
                            }
                            else
                            {
                                ImGui::TextDisabled("No asset loaded");
                            }
                        }

                        // Bus info
                        if (ImGui::CollapsingHeader("Bus"))
                        {
                            ImGui::Text("Bus ID: %llu", static_cast<AZ::u64>(player->m_busId));

                            AZStd::string busName;
                            AudioBusRequestsBus::EventResult(busName, player->m_busId,
                                &AudioBusRequestsBus::Events::GetName);
                            ImGui::Text("Bus Name: %s", busName.c_str());
                        }

                        // Effects
                        if (ImGui::CollapsingHeader("Effects", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Text("Effect Count: %zu", player->m_effects.size());

                            // Horizontal scrollable button bar for adding effects
                            ImGui::Text("Add Effect:");
                            ImGui::BeginChild("EffectButtons", ImVec2(0, 40), true, ImGuiWindowFlags_HorizontalScrollbar);
                            /*{
                                for (const auto& [effectName, _] : m_playerEffects)
                                {
                                    if (ImGui::Button(effectName.c_str()))
                                    {
                                        player->AddEffect(effectName);
                                    }
                                    ImGui::SameLine();
                                }
                            }*/
                            ImGui::EndChild();

                            ImGui::Spacing();
                            ImGui::Separator();

                            // List all effects
                            for (size_t i = 0; i < player->m_effects.size(); ++i)
                            {
                                auto& effect = player->m_effects[i];

                                ImGui::PushID(static_cast<int>(i));

                                // Effect header
                                AZStd::string effectLabel = AZStd::string::format(
                                    "%s [Order: %d]",
                                    effect->GetEffectName(),
                                    static_cast<int>(effect->GetProcessingOrder())
                                );

                                bool isOpen = ImGui::TreeNode(effectLabel.c_str());

                                // Effect enabled checkbox (same line as header)
                                ImGui::SameLine();
                                bool enabled = effect->IsEnabled();
                                if (ImGui::Checkbox("##enabled", &enabled))
                                {
                                    effect->SetEnabled(enabled);
                                    player->ReconnectGraph();
                                }

                                if (isOpen)
                                {
                                    ImGui::Text("Effect ID: %s", effect->m_id.ToString().c_str());
                                    ImGui::Text("Player ID: %s", effect->m_playerId.ToString().c_str());
                                    ImGui::Text("Enabled: %s", effect->IsEnabled() ? "Yes" : "No");

                                    // Remove button
                                    if (ImGui::Button("Remove Effect"))
                                    {
                                        player->RemoveEffect(effect->m_id);
                                        ImGui::TreePop();
                                        ImGui::PopID();
                                        break; // Exit loop since we modified the vector
                                    }

                                    // Delegate effect-specific UI to the effect via ImGui bus
                                    PlayerEffectImGuiRequestBus::Event(
                                        effect->m_id,
                                        &PlayerEffectImGuiRequests::DrawGui
                                    );

                                    ImGui::TreePop();
                                }

                                ImGui::PopID();
                                ImGui::Separator();
                            }

                            if (player->m_effects.empty())
                            {
                                ImGui::TextDisabled("No effects");
                            }
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("Select a player from the list");
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }

        if (g_igShowLabSoundBus)
        {
            if (ImGui::Begin("Sune Buses"))
            {
                AudioBusManagerInterface::Get()->IterateBuses([](AudioBusId id, const AZStd::string& name)
                {
                    ImGui::PushID(static_cast<int>(id.GetValue()));
                    ImGui::Text("%s", name.c_str());
                    ImGui::Text("Id: %u", id.GetValue());
                    float gain = 0.0f;
                    AudioBusRequestsBus::EventResult(gain, id, &AudioBusRequestsBus::Events::GetGain);
                    if (ImGui::InputFloat("Gain", &gain, 0.01f, 0.1f))
                    {
                        AudioBusRequestsBus::Event(id, &AudioBusRequestsBus::Events::SetGain, gain);
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                });
            }
            ImGui::End();
        }
    }
} // namespace Sune
