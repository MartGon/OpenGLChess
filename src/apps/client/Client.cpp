#include <Client.h>

#include <util/Random.h>
#include <util/Time.h>

#include <networking/Command.h>
#include <debug/Debug.h>

#include <iostream>
#include <algorithm>

using namespace BlockBuster;

Client::Client(::App::Configuration config) : App{config}, 
    host{ENet::HostFactory::Get()->CreateHost(1, 2)}
{
}

void BlockBuster::Client::Start()
{
    // GL features
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Shaders
    // Load shaders
    try{
        shader = GL::Shader::FromFolder(config.openGL.shadersFolder, "circleVertex.glsl", "circleFrag.glsl");
        chunkShader = GL::Shader::FromFolder(config.openGL.shadersFolder, "chunkVertex.glsl", "chunkFrag.glsl");
    }
    catch(const std::runtime_error& e)
    {
        this->logger->LogCritical(e.what());
        quit = true;
        return;
    }

    // Meshes
    cylinder = Rendering::Primitive::GenerateCylinder(2.f, 4.f, 32, 1);

    // Camera
    camera_.SetPos(glm::vec3{0, 8, 16});
    camera_.SetTarget(glm::vec3{0});
    auto winSize = GetWindowSize();
    camera_.SetParam(Rendering::Camera::Param::ASPECT_RATIO, (float)winSize.x / (float)winSize.y);
    camera_.SetParam(Rendering::Camera::Param::FOV, config.window.fov);
    camController_ = ::App::Client::CameraController{&camera_, {window_, io_}, ::App::Client::CameraMode::EDITOR};

    // Map
    auto black = map_.cPalette.AddColor(glm::u8vec4{0, 0, 0, 255});
    auto white = map_.cPalette.AddColor(glm::u8vec4{255, 255, 255, 255});

    Game::BlockRot rot{Game::ROT_0, Game::ROT_0};
    for(int x = -8; x < 8; x++)
    {
        for(int z = -8; z < 8; z++)
        {
            auto colorId = ((x + z) & 2) ? black.data.id : white.data.id;
            Game::Display display{Game::DisplayType::COLOR, colorId};
            map_.SetBlock(glm::ivec3{x, 0, z}, Game::Block{Game::BlockType::BLOCK, rot, display});
        }
    }

    // Networking
    auto serverAddress = ENet::Address::CreateByIPAddress("127.0.0.1", 8081).value();
    host.SetOnConnectCallback([this](auto id)
    {
        this->serverId = id;
        this->logger->LogInfo("Succes on connection to server");
    });
    host.SetOnRecvCallback([this](auto id, auto channel, ENet::RecvPacket ePacket)
    {
        this->logger->LogInfo("Tick: " + std::to_string(serverTick) + " Server packet recv of size: " + std::to_string(ePacket.GetSize()));
        
        auto* packet = (Networking::Command*) ePacket.GetData();
        this->serverTick = packet->header.tick;
        if(packet->header.type == Networking::Command::Type::CLIENT_CONFIG)
        {
            auto config = packet->data.config;
            logger->LogInfo("Server tick rate is " + std::to_string(config.sampleRate));
            this->serverTickRate = config.sampleRate;
            uint64_t millisInterval = serverTickRate * 1e3;
            logger->LogInfo("Timer duration is " + std::to_string(millisInterval));
            this->sampleTimer = Util::Time::Timer{millisInterval};
            this->serverTick = packet->header.tick;
            this->playerId = config.playerId;

            this->connected = true;
        }
        else if(packet->header.type == Networking::Command::Type::PLAYER_POS_UPDATE)
        {
            Networking::Command::Server::PlayerUpdate playerUpdate = packet->data.playerUpdate;
            logger->LogInfo("Server packet with player " + std::to_string(playerUpdate.playerId) + " data");
            logger->LogInfo("Player new pos is " + glm::to_string(playerUpdate.pos));
            if(playerTable.find(playerUpdate.playerId) == playerTable.end())
            {
                Math::Transform transform{playerUpdate.pos, glm::vec3{0.0f}, glm::vec3{0.5f, 1.0f, 0.5f}};
                playerTable[playerUpdate.playerId] = Entity::Player{playerUpdate.playerId, transform};
            }
            else
            {
                auto diff = playerUpdate.pos - this->playerTable[playerUpdate.playerId].transform.position;
                auto distance = glm::length(diff);
                this->moveSpeed = distance / this->serverTickRate;
                this->playerTable[playerUpdate.playerId].transform.position = playerUpdate.pos;
            }
        }
        else if(packet->header.type == Networking::Command::Type::PLAYER_DISCONNECTED)
        {
            Networking::Command::Server::PlayerDisconnected playerDisconnect = packet->data.playerDisconnect;
            logger->LogInfo("Player with id " + std::to_string(playerDisconnect.playerId) + " disconnected");

            playerTable.erase(playerDisconnect.playerId);
        }
    });
    host.Connect(serverAddress);

    auto attempts = 0;
    while(!connected && attempts < 5)
    {
        logger->LogInfo("Connecting to server...");
        host.PollAllEvents();
        Util::Time::SleepMS(500);
        attempts++;
    }

    if(!connected)
    {
        logger->LogInfo("Could not connect to server. Quitting");
    }

    quit = !connected;

    // Debug
    lastSample = Util::Time::GetUNIXTimeMS<uint64_t>();
}

void BlockBuster::Client::Update()
{
    double previous = Util::Time::GetUNIXTime();
    double lag = 0.0;

    logger->LogInfo("Update rate(s) is: " + std::to_string(UPDATE_RATE));
    while(!quit)
    {
        auto now =  Util::Time::GetUNIXTime();
        auto elapsed = (now - previous);
        
        previous = now;
        lag += elapsed;

        HandleSDLEvents();
        if(connected)
            UpdateNetworking();

        while(lag >= UPDATE_RATE)
        {
            DoUpdate(UPDATE_RATE);
            lag -= UPDATE_RATE;
        }

        Render();
    }
}

bool BlockBuster::Client::Quit()
{
    return quit;
}

void BlockBuster::Client::DoUpdate(double deltaTime)
{
    camController_.Update();
}

void Client::UpdateNetworking()
{
    host.PollAllEvents();

    if(sampleTimer.IsOver())
    {
        SendPlayerMovement();
        sampleTimer.ResetToNextStep();

        // Debug
        auto now = Util::Time::GetUNIXTimeMS<uint64_t>();
        auto lag = now - lastSample;
        minSamplingLag = lag < minSamplingLag ? lag : minSamplingLag;
        maxSamplingLag = lag > maxSamplingLag ? lag : maxSamplingLag;
        samplingLag = lag;
        lastSample = now;
    }
}

void Client::SendPlayerMovement()
{
    int8_t* state = (int8_t*)SDL_GetKeyboardState(nullptr);
    glm::vec3 moveDir{0.0f};
    moveDir.x = state[SDL_SCANCODE_KP_6] - state[SDL_SCANCODE_KP_4];
    moveDir.z = state[SDL_SCANCODE_KP_2] - state[SDL_SCANCODE_KP_8];
    logger->LogInfo(std::to_string(Util::Time::GetUNIXTimeMS<uint64_t>()) + " Command sent: " + std::to_string(serverTick) + ": Move dir " + glm::to_string(moveDir));
    auto len = glm::length(moveDir);
    moveDir = len > 0 ? moveDir / len : moveDir;

    Networking::Command::Header header;
    header.type = Networking::Command::Type::PLAYER_MOVEMENT;
    header.tick = serverTick;

    Networking::Command::User::PlayerMovement playerMovement;
    playerMovement.moveDir = moveDir;

    Networking::Command::Payload data;
    data.playerMovement = playerMovement;
    
    Networking::Command packet{header, data};

    ENet::SentPacket sentPacket{&packet, sizeof(packet), ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE};
    host.SendPacket(serverId, 0, sentPacket);
}

void BlockBuster::Client::HandleSDLEvents()
{
    SDL_Event e;
    
    while(SDL_PollEvent(&e) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&e);

        switch(e.type)
        {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYDOWN:
            if(e.key.keysym.sym == SDLK_f)
                drawMode = drawMode == GL_FILL ? GL_LINE : GL_FILL;
            break;
        }
        camController_.HandleSDLEvent(e);
    }
}

void Client::Render()
{
    // Clear Buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawScene();
    DrawGUI();

    // Swap buffers
    SDL_GL_SwapWindow(window_);
}

void BlockBuster::Client::DrawScene()
{
    // Draw data
    auto view = camera_.GetProjViewMat();

    for(auto player : playerTable)
    {
        auto t = player.second.transform.GetTransformMat();
        auto transform = view * t;
        shader.SetUniformMat4("transform", transform);
        shader.SetUniformVec4("color", glm::vec4{1.0f});
        cylinder.Draw(shader, drawMode);
    }

    // Bind textures
    map_.tPalette.GetTextureArray()->Bind(GL_TEXTURE0);
    chunkShader.SetUniformInt("textureArray", 0);
    map_.cPalette.GetTextureArray()->Bind(GL_TEXTURE1);
    chunkShader.SetUniformInt("colorArray", 1);

    // Draw map
    map_.Draw(chunkShader, view);
}

void Client::DrawGUI()
{
    // Clear GUI buffer
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window_);
    ImGui::NewFrame();

    bool isOpen = true;
    if(connected)
    {
        if(ImGui::Begin("Network Stats"))
        {
            auto info = host.GetPeerInfo(serverId);

            ImGui::Text("Latency");
            ImGui::Separator();
            ImGui::Text("Ping:");ImGui::SameLine();ImGui::Text("%i ms", info.roundTripTimeMs);
            ImGui::Text("Ping variance: %i", info.roundTripTimeVariance);

            ImGui::Text("Packets");
            ImGui::Separator();
            ImGui::Text("Packets sent: %i", info.packetsSent);
            ImGui::Text("Packets ack: %i", info.packetsAck);
            ImGui::Text("Packets lost: %i", info.packetsLost);
            ImGui::Text("Packet loss: %i", info.packetLoss);

            ImGui::Text("Bandwidth");
            ImGui::Separator();
            ImGui::Text("In: %i B/s", info.incomingBandwidth);
            ImGui::Text("Out: %i B/s", info.outgoingBandwidth);

            ImGui::Text("Input Sapmling");
            ImGui::Separator();
            ImGui::Text("Lag: %lu", samplingLag); ImGui::SameLine(); ImGui::Text("Min: %lu", minSamplingLag); ImGui::SameLine(); ImGui::Text("Max: %lu", maxSamplingLag);
            ImGui::Text("Last TimeStamp: %lu", lastSample);
            ImGui::Text("Move speed: %f", moveSpeed);
        }

        ImGui::End();
    }
    
    // Draw GUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
