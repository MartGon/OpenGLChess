#include <GameState/MainMenu/MenuState.h>
#include <GameState/MainMenu/MainMenu.h>

#include <Client.h>

#include <imgui.h>
#include <imgui_internal.h>

using namespace BlockBuster::MenuState;

Base::Base(MainMenu* mainMenu) : mainMenu_{mainMenu}
{

}

// #### LOGIN #### \\

void Login::Update()
{
    auto displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if(ImGui::Begin("Block Buster", nullptr, flags))
    {
        int itFlags = mainMenu_->httpClient.IsConnecting() ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
        itFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
        bool enter = ImGui::InputText("Username", inputUsername, 16, itFlags);

        auto winWidth = ImGui::GetWindowWidth();
        auto buttonWidth = ImGui::CalcTextSize("Login").x + 8;
        ImGui::SetCursorPosX(winWidth / 2.0f - buttonWidth / 2.0f);

        auto disabled = mainMenu_->popUp.IsVisible();
        if(disabled)
            ImGui::PushDisabled();

        if(ImGui::Button("Login") || enter)
        {
            mainMenu_->Login(inputUsername);
        }

        if(disabled)
            ImGui::PopDisabled();
    }
    ImGui::End();
}

// #### SERVER BROWSER #### \\

void ServerBrowser::OnEnter()
{
    mainMenu_->ListGames();
}

void ServerBrowser::Update()
{
    auto displaySize = ImGui::GetIO().DisplaySize;
    
    // Size
    ImGui::SetNextWindowSize(ImVec2{displaySize.x * 0.7f, displaySize.y * 0.5f}, ImGuiCond_Always);

    // Centered
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    bool show = true;
    if(ImGui::Begin("Server Browser", &show, flags))
    {
        auto winSize = ImGui::GetWindowSize();

        auto tableSize = ImVec2{winSize.x * 0.975f, winSize.y * 0.8f};
        auto tFlags = ImGuiTableFlags_None | ImGuiTableFlags_ScrollY;
        ImGui::SetCursorPosX((winSize.x - tableSize.x) / 2.f);
        if(ImGui::BeginTable("#Server Table", 5, tFlags, tableSize))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Map");
            ImGui::TableSetupColumn("Mode");
            ImGui::TableSetupColumn("Players");
            ImGui::TableSetupColumn("Ping");
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible

            ImGui::TableHeadersRow();
            
            for(auto game : mainMenu_->gamesList)
            {
                ImGui::TableNextRow();
                auto gameId = game.id;
                auto name = game.name;

                ImGui::TableNextColumn();
                auto selectFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
                if(ImGui::Selectable(name.c_str(), false, selectFlags))
                {
                    if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
                        mainMenu_->JoinGame(gameId);
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", game.map.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", game.mode.c_str());

                ImGui::TableNextColumn();
                auto players = std::to_string(game.players) + "/" + std::to_string(game.maxPlayers);
                ImGui::Text("%s", players.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%i", game.ping);
            }

            ImGui::EndTable();
        }

        if(ImGui::Button("Create Game"))
        {
            mainMenu_->SetState(std::make_unique<MenuState::CreateGame>(mainMenu_));
        }
        ImGui::SameLine();
        ImGui::Button("Connect");
        ImGui::SameLine();
        ImGui::Button("Filters");
        ImGui::SameLine();


        if(ImGui::Button("Update"))
        {
            mainMenu_->ListGames();
        }
    }
    // User clicked on the X button. Go back to login page
    if(!show)
        mainMenu_->SetState(std::make_unique<MenuState::Login>(mainMenu_));

    ImGui::End();
}

// #### CREATE GAME #### \\

void CreateGame::OnEnter()
{
    std::string placeholderName = mainMenu_->user + "'s game";
    strcpy(gameName, placeholderName.c_str());
    strcpy(map, "Kobra");
    strcpy(mode, "DeathMatch");
}

void CreateGame::Update()
{
    auto displaySize = ImGui::GetIO().DisplaySize;
    
    // Size
    //ImGui::SetNextWindowSize(ImVec2{displaySize.x * 0.7f, displaySize.y * 0.5f}, ImGuiCond_Always);

    // Centered
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    bool show = true;
    if(ImGui::Begin("Create Game", &show, flags))
    {
        auto textFlags = ImGuiInputTextFlags_None;
        ImGui::InputText("Name", gameName, 32, textFlags);

        auto comboFlags = ImGuiComboFlags_None;
        if(ImGui::BeginCombo("Map", map, comboFlags))
        {   
            bool selected = std::strcmp("Kobra", map) == 0;
            if(ImGui::Selectable("Kobra", true))
            {
                std::strcpy(map, "Kobra");
            }
            ImGui::EndCombo();
        }

        if(ImGui::BeginCombo("Mode", mode, comboFlags))
        {
            bool selected = std::strcmp("DeathMatch", mode) == 0;
            if(ImGui::Selectable("DeathMatch", true))
            {
                std::strcpy(mode, "DeathMatch");
            }
            ImGui::EndCombo();
        }

        auto sliderFlags = ImGuiSliderFlags_::ImGuiSliderFlags_None;
        ImGui::SliderInt("Max players", &maxPlayers, 2, 16, "%i", sliderFlags);

        if(ImGui::Button("Create Game"))
        {
            // TODO: Change map and mode
            mainMenu_->CreateGame(gameName, "Kobra", "DeathMatch", maxPlayers);
        }
    }
    // User clicked on the X button. Go back
    if(!show)
        mainMenu_->SetState(std::make_unique<MenuState::ServerBrowser>(mainMenu_));

    ImGui::End();
}

// #### LOBBY #### \\

void Lobby::OnEnter()
{
    mainMenu_->lobby = this;
    OnGameInfoUpdate();
    mainMenu_->UpdateGame();
}

void Lobby::OnExit()
{
    mainMenu_->lobby = nullptr;
}

void Lobby::OnGameInfoUpdate()
{
    auto chatData = mainMenu_->currentGame->game.chat;
    char* chatPtr = this->chat;
    for(auto it = chatData.begin(); it != chatData.end(); it++)
    {
        auto lineSize = it->size();
        auto size = chatPtr - this->chat + lineSize;
        if(size < 4096 /*Chat Size*/)
        {
            strcpy(chatPtr, it->c_str());
            chatPtr += lineSize;
        }
    }
}

void Lobby::Update()
{
    auto displaySize = ImGui::GetIO().DisplaySize;
    
    // Size
    auto windowSize = ImVec2{displaySize.x * 0.7f, displaySize.y * 0.75f};
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    // Centered
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});

    // CurrentGame should have a value, in order to operate correctly
    bool show = true;
    if(!mainMenu_->currentGame.has_value())
    {
        mainMenu_->GetLogger()->LogError("Current game didn't have a value. Returning to server browser");
        show = false;
    }

    MainMenu::GameDetails gameDetails = mainMenu_->currentGame.value();
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    if(ImGui::Begin(gameDetails.game.name.c_str(), &show, flags))
    {
        windowSize = ImGui::GetWindowSize();

        auto layoutFlags = ImGuiTableFlags_None;
        auto layoutSize = ImVec2{windowSize.x * 0.975f, windowSize.y * 1.0f};
        if(ImGui::BeginTable("#Layout", 2, layoutFlags))
        {
            // Columns Setup
            auto colFlags = ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed;
            float leftColSize = layoutSize.x * 0.65f;
            ImGui::TableSetupColumn("#PlayerTable", colFlags, leftColSize);
            float rightColSize = 1.0f - leftColSize;
            ImGui::TableSetupColumn("#Game info", colFlags, rightColSize);

            // Player Table
            ImGui::TableNextColumn();
            auto playerTableFlags = ImGuiTableFlags_None | ImGuiTableFlags_ScrollY;
            auto playerTableSize = ImVec2{leftColSize * 0.975f, layoutSize.y * 0.5f};
            if(ImGui::BeginTable("Player List", 3, playerTableFlags, playerTableSize))
            {   
                float lvlSize = 0.1f * playerTableSize.x;
                float readySize = 0.3f * playerTableSize.x;
                float playerNameSize = playerTableSize.x - lvlSize - readySize;
                ImGui::TableSetupColumn("Lvl", colFlags, lvlSize);
                ImGui::TableSetupColumn("Player Name", colFlags, playerNameSize);
                ImGui::TableSetupColumn("Status", colFlags, readySize);
                ImGui::TableSetupScrollFreeze(0, 1); // Freeze first row;

                ImGui::TableHeadersRow();

                // Print player's info
                for(auto playerInfo : mainMenu_->currentGame->playersInfo)
                {
                    // Lvl Col
                    ImGui::TableNextColumn();
                    ImGui::Text("25");

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", playerInfo.playerName.c_str());

                    ImGui::TableNextColumn();
                    
                    std::string status;
                    if(playerInfo.isHost)
                        status = "Host";
                    else
                        status = playerInfo.isReady ? "Ready" : "Not ready";
                    ImGui::Text("%s", status.c_str());
                }
            
                ImGui::EndTable();
            }

            // Map Picture
            auto gameInfo = gameDetails.game;
            ImGui::TableNextColumn();
            ImGui::Text("Map: Kobra");

            ImVec2 imgSize{0, layoutSize.y * 0.5f};
            //ImGui::Image()
            ImGui::Dummy(imgSize);

            // Chat window
            ImGui::TableNextColumn();
            auto inputLineSize = ImGui::CalcTextSize("Type ");

            ImGui::Text("Chat Window");
            auto chatFlags = ImGuiInputTextFlags_ReadOnly;
            auto height = ImGui::GetCursorPosY();
            float marginSize = 4.0f;
            ImVec2 chatSize{playerTableSize.x, (layoutSize.y - height) - inputLineSize.y * 2.0f - marginSize};

            ImGui::InputTextMultiline("##Chat Window", this->chat, 256, chatSize, chatFlags);

            auto chatLineFlags = ImGuiInputTextFlags_EnterReturnsTrue;
            ImGui::Text("Type"); ImGui::SameLine(); 
            ImGui::PushItemWidth(playerTableSize.x - inputLineSize.x);
            if(ImGui::InputText("##Type", chatLine, 128, chatLineFlags))
            {
                mainMenu_->SendChatMsg(chatLine);
                chatLine[0] = '\0';
            }
            ImGui::PopItemWidth();

            // Map Info
            ImGui::TableNextColumn();
            ImGui::Text("Game Info");
            auto gitFlags = 0;
            ImVec2 gitSize{layoutSize.x - leftColSize, 0};
            if(ImGui::BeginTable("Game Info", 2, gitFlags, gitSize))
            {
                ImGui::TableNextColumn();
                ImGui::Text("%s", "Name");

                ImGui::TableNextColumn();
                ImGui::Text("%s", gameInfo.name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", "Map");

                ImGui::TableNextColumn();
                ImGui::Text("%s", gameInfo.map.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", "Mode");

                ImGui::TableNextColumn();
                ImGui::Text("%s", gameInfo.mode.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", "Max Players");

                ImGui::TableNextColumn();
                ImGui::Text("%i", gameInfo.maxPlayers);

                ImGui::TableNextColumn();
                if(ImGui::Button("Ready"))
                {
                    mainMenu_->ToggleReady();
                }
                ImGui::EndTable();
            }

            // Start Game Button
            if(!IsPlayerHost() || !IsEveryoneReady())
                ImGui::PushDisabled();

            ImGui::BeginChild("Start Game Button", ImVec2(0, 0), false, 0);
            auto size = ImGui::GetContentRegionAvail();
                if(ImGui::Button("Start Game", size))
                {
                    mainMenu_->StartGame();
                }
            ImGui::EndChild();

            if(!IsPlayerHost() || !IsEveryoneReady())
                ImGui::PopDisabled();

            ImGui::EndTable();
        }
    }
    // User clicked on the X button. Go back
    if(!show)
    {
        mainMenu_->LeaveGame();
    }

    ImGui::End();
}

bool Lobby::IsPlayerHost()
{
    for(auto playerInfo : mainMenu_->currentGame->playersInfo)
    {
        if(mainMenu_->user == playerInfo.playerName)
            return playerInfo.isHost;
    }

    return false;
}

bool Lobby::IsEveryoneReady()
{
    for(auto playerInfo : mainMenu_->currentGame->playersInfo)
    {
        if(!playerInfo.isHost && !playerInfo.isReady)
            return false;
    }

    return true;
}