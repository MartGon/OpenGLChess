#include <Client.h>

#include <util/Random.h>
#include <util/BBTime.h>

#include <math/Interpolation.h>

#include <debug/Debug.h>
#include <nlohmann/json.hpp>
#include <httplib/httplib.h>

#include <iostream>
#include <algorithm>

#include <GameState/InGame/InGame.h>
#include <GameState/MainMenu/MainMenu.h>

using namespace BlockBuster;

Client::Client(::App::Configuration config) : AppI{config}
{
}

void Client::Start()
{
    auto mapsFolder = GetConfigOption("mapsFolder", "./maps");
    mapMgr.SetMapsFolder(mapsFolder);

    state = std::make_unique<MainMenu>(this);
    state->Start();
    
    //LaunchGame("localhost", 8081, "Alpha2", "NULL PLAYER UUID", "Defu");
}

void Client::Shutdown()
{
    state->Shutdown();

    config.options["mapsFolder"] = mapMgr.GetMapsFolder().string();
}

void Client::Update()
{
    state->Update();

    if(nextState.get() != nullptr)
    {
        if(saveState)
            oldState = std::move(state);
        else
            state->Shutdown();

        state = std::move(nextState);
        state->Start();
        nextState = nullptr;

        saveState = false;
    }
}

bool Client::Quit()
{
    return quit;
}

void Client::LaunchGame(std::string address, uint16_t port, std::string map, std::string playerUuid, std::string playerName)
{
    nextState = std::make_unique<InGame>(this, address, port, map, playerUuid, playerName);
    saveState = true;
}

void Client::GoBackToMainMenu()
{
    saveState = false;
    if(oldState.get() != nullptr)
        nextState = std::move(oldState);
    else
        nextState = std::make_unique<MainMenu>(this);
}

void Client::ApplyVideoOptions(App::Configuration::WindowConfig& winConfig)
{
    AppI::ApplyVideoOptions(winConfig);
    
    this->state->ApplyVideoOptions(winConfig);
}