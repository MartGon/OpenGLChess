#include <Editor.h>

#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstring>

#include <glm/gtc/constants.hpp>

void BlockBuster::Editor::Start()
{
    // Shaders
    shader.Use();

    // Meshes
    cube = Rendering::Primitive::GenerateCube();
    slope = Rendering::Primitive::GenerateSlope();

    // Textures
    texture = GL::Texture::FromFolder(TEXTURES_DIR, "SmoothStone.png");
    try
    {
        texture.Load();
    }
    catch(const GL::Texture::LoadError& e)
    {
        std::cout << "Error when loading texture " + e.path_.string() + ": " +  e.what() << '\n';
    }
    
    // OpenGL features
    glEnable(GL_DEPTH_TEST);

    // Camera pos
    camera.SetPos(glm::vec3 {0.0f, 6.0f, 6.0f});
    camera.SetParam(Rendering::Camera::Param::ASPECT_RATIO, (float)config.window.width / (float)config.window.height);
    camera.SetTarget(glm::vec3{0.0f});

    // World
    InitMap();

    // Gui Setup
    RenameWindow("New Map");
}

void BlockBuster::Editor::Update()
{
    // Handle Events    
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&e);
        ImGuiIO& io = ImGui::GetIO();
        
        switch(e.type)
        {
        case SDL_KEYDOWN:
            if(e.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
            break;
        case SDL_QUIT:
            std::cout << "Quitting\n";
            quit = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(!io.WantCaptureMouse)
            {
                glm::vec2 mousePos;
                mousePos.x = e.button.x;
                mousePos.y = config.window.height - e.button.y;
                UseTool(mousePos, e.button.button == SDL_BUTTON_RIGHT);
                std::cout << "Click coords " << mousePos.x << " " << mousePos.y << "\n";
            }
        }
    }
    // Camera
    UpdateCamera();

    // Draw cubes/slopes
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(int i = 0; i < blocks.size(); i++)
    {
        auto block = blocks[i];
        auto model = block.transform.GetTransformMat();
        auto type = block.type;
        auto transform = camera.GetProjViewMat() * model;

        shader.SetUniformInt("isPlayer", 0);
        shader.SetUniformMat4("transform", transform);
        auto& mesh = GetMesh(block.type);
        auto display = block.display;
        if(display.type == Game::DisplayType::TEXTURE)
            mesh.Draw(shader, &texture);
        else if(display.type == Game::DisplayType::COLOR)
            mesh.Draw(shader, display.display.color.color);
    }

    // GUI
    GUI();

    SDL_GL_SwapWindow(window_);
}

bool BlockBuster::Editor::Quit()
{
    return quit;
}

Rendering::Mesh& BlockBuster::Editor::GetMesh(Game::BlockType blockType)
{
    return blockType == Game::BlockType::SLOPE ? slope : cube;
}

Game::Block* BlockBuster::Editor::GetBlock(glm::vec3 pos)
{   
    Game::Block* block = nullptr;
    for(auto& b : blocks)
        if(b.transform.position == pos)
            block = &b;

    return block;
}

template<typename T>
void WriteToFile(std::fstream& file, T val)
{
    file.write(reinterpret_cast<char*>(&val), sizeof(T));
}

template <typename T>
T ReadFromFile(std::fstream& file)
{
    T val;
    file.read(reinterpret_cast<char*>(&val), sizeof(T));
    return val;
}

void SaveVec3(std::fstream& file, glm::vec3 vec)
{
    WriteToFile(file, vec.x);
    WriteToFile(file, vec.y);
    WriteToFile(file, vec.z);
}

glm::vec3 ReadVec3(std::fstream& file)
{
    glm::vec3 vec;
    vec.x = ReadFromFile<float>(file);
    vec.y = ReadFromFile<float>(file);
    vec.z = ReadFromFile<float>(file);

    return vec;
}

void SaveVec4(std::fstream& file, glm::vec4 vec)
{
    SaveVec3(file, vec);
    WriteToFile(file, vec.w);
}

glm::vec4 ReadVec4(std::fstream& file)
{
    glm::vec3 vec3 = ReadVec3(file);
    float w = ReadFromFile<float>(file);
    return glm::vec4{vec3, w};
}

void BlockBuster::Editor::InitMap()
{
    blocks = {
        {Math::Transform{glm::vec3{0.0f, 0.0f, 0.0f} * blockScale, glm::vec3{0.0f, 0.0f, 0.0f}, blockScale}, Game::BLOCK},
        //{Math::Transform{glm::vec3{0.0f, 6.0f, 0.0f} * BLOCK_SCALE, glm::vec3{0.0f, 0.0f, 0.0f}, BLOCK_SCALE}, Game::BLOCK},
    };
}

void BlockBuster::Editor::SaveMap()
{
    std::fstream file{fileName, file.binary | file.out};
    if(!file.is_open())
    {
        std::cout << "Could not open file " << fileName << '\n';
        return;
    }

    // Header
    WriteToFile(file, blocks.size());
    WriteToFile(file, blockScale);

    // Save player pos

    // Blocks
    for(auto& block : blocks)
    {
        WriteToFile(file, block.type);
        SaveVec3(file, block.transform.position);
        if(block.type == Game::BlockType::SLOPE)
            SaveVec3(file, block.transform.rotation);

        WriteToFile(file, block.display.type);
        if(block.display.type == Game::DisplayType::COLOR)
            SaveVec4(file, block.display.display.color.color);
        else
            WriteToFile(file, block.display.display.texture.textureId);
    }
}

void BlockBuster::Editor::LoadMap()
{
    std::fstream file{fileName, file.binary | file.in};
    if(!file.is_open())
    {
        std::cout << "Could not open file " << fileName << '\n';
        return;
    }

    auto count = ReadFromFile<std::size_t>(file);
    blockScale = ReadFromFile<float>(file);
    blocks.clear();
    blocks.reserve(count);
    for(std::size_t i = 0; i < count; i++)
    {
        Game::Block block;
        block.type = ReadFromFile<Game::BlockType>(file);
        block.transform.position = ReadVec3(file);
        block.transform.scale = blockScale;
        if(block.type == Game::BlockType::SLOPE)
            block.transform.rotation = ReadVec3(file);

        block.display.type = ReadFromFile<Game::DisplayType>(file);
        if(block.display.type == Game::DisplayType::COLOR)
            block.display.display.color.color = ReadVec4(file);
        else
            block.display.display.texture.textureId = ReadFromFile<int>(file);

        blocks.push_back(block);
    }
}

void BlockBuster::Editor::UpdateCamera()
{
    if(io_->WantCaptureKeyboard)
        return;

    auto state = SDL_GetKeyboardState(nullptr);

    auto cameraRot = camera.GetRotation();
    float pitch = 0.0f;
    float yaw = 0.0f;
    if(state[SDL_SCANCODE_UP])
        pitch += -CAMERA_ROT_SPEED;
    if(state[SDL_SCANCODE_DOWN])
        pitch += CAMERA_ROT_SPEED;

    if(state[SDL_SCANCODE_LEFT])
        yaw += CAMERA_ROT_SPEED;
    if(state[SDL_SCANCODE_RIGHT])
        yaw += -CAMERA_ROT_SPEED;
    
    cameraRot.x = glm::max(glm::min(cameraRot.x + pitch, glm::pi<float>() - CAMERA_ROT_SPEED), CAMERA_ROT_SPEED);
    cameraRot.y = cameraRot.y + yaw;
    camera.SetRotation(cameraRot.x, cameraRot.y);
    
    auto cameraPos = camera.GetPos();
    auto front = camera.GetFront();
    auto xAxis = glm::normalize(-glm::cross(front, Rendering::Camera::UP));
    auto zAxis = glm::normalize(-glm::cross(xAxis, Rendering::Camera::UP));
    glm::vec3 moveDir{0};
    if(state[SDL_SCANCODE_A])
        moveDir += xAxis;
    if(state[SDL_SCANCODE_D])
        moveDir -= xAxis;
    if(state[SDL_SCANCODE_W])
        moveDir -= zAxis;
    if(state[SDL_SCANCODE_S])
        moveDir += zAxis;
    if(state[SDL_SCANCODE_Q])
        moveDir.y += 1;
    if(state[SDL_SCANCODE_E])
        moveDir.y -= 1;
    if(state[SDL_SCANCODE_F])
        cameraPos = glm::vec3{0.0f, 2.0f, 0.0f};
           
    auto offset = glm::length(moveDir) > 0.0f ? (glm::normalize(moveDir) * CAMERA_MOVE_SPEED) : moveDir;
    cameraPos += offset;
    camera.SetPos(cameraPos);
}

void BlockBuster::Editor::UseTool(glm::vec<2, int> mousePos, bool rightButton)
{
    // Window to eye
    auto ray = Rendering::ScreenToWorldRay(camera, mousePos, glm::vec2{config.window.width, config.window.height});

    // Sort blocks by proximity to camera
    auto cameraPos = this->camera.GetPos();
    std::sort(blocks.begin(), blocks.end(), [cameraPos](Game::Block a, Game::Block b)
    {
        auto toA = glm::length(a.transform.position - cameraPos);
        auto toB = glm::length(b.transform.position- cameraPos);
        return toA < toB;
    });

    // Check intersection
    int index = -1;
    Collisions::RayIntersection intersection;
    for(int i = 0; i < blocks.size(); i++)
    {  
        auto model = blocks[i].transform.GetTransformMat();
        auto type = blocks[i].type;

        if(type == Game::SLOPE)
            intersection = Collisions::RaySlopeIntersection(ray, model);
        else
            intersection = Collisions::RayAABBIntersection(ray, model);

        if(intersection.intersects)
        {
            index = i;
            break;
        }
    }
    
    if(index != -1)
    {
        // Use appropiate Tool
        switch(tool)
        {
            case PLACE_BLOCK:
            {
                if(rightButton)
                {
                    if(blocks.size() > 1)
                        blocks.erase(blocks.begin() + index);
                    break;
                }
                else
                {
                    auto yaw = (std::round(glm::degrees(camera.GetRotation().y) / 90.0f) * 90.0f) - 90.0f;

                    auto block = blocks[index];
                    auto pos = block.transform.position;
                    auto scale = block.transform.scale;

                    auto newBlockPos = pos + intersection.normal * scale;
                    auto newBlockRot = glm::vec3{0.0f, yaw, 0.0f};
                    auto newBlockType = blockType;

                    Game::Display display;
                    display.type = displayType;
                    if(displayType == Game::DisplayType::COLOR)
                        display.display.color.color = displayColor;
                    else if(displayType == Game::DisplayType::TEXTURE)
                        display.display.texture.textureId = 0;

                    if(!GetBlock(newBlockPos))
                    {
                        blocks.push_back({Math::Transform{newBlockPos, newBlockRot, scale}, newBlockType, display});
                        std::cout << "Block added at \n";
                    }
                }

                break;
            }            
            case ROTATE_BLOCK:
            {
                auto& block = blocks[index];
                float mod = rightButton ? -90.0f : 90.0f;
                if(block.type == Game::BlockType::SLOPE)
                {
                    if(axis == RotationAxis::X)
                        block.transform.rotation.x += mod;
                    else if(axis == RotationAxis::Y)
                        block.transform.rotation.y += mod;
                    else
                        block.transform.rotation.z += mod;
                    break;
                }
            }
        }
    }
}

void BlockBuster::Editor::RenameWindow(std::string name)
{
    std::string title = "Editor - " + name;
    SDL_SetWindowTitle(window_, title.c_str());
}

void BlockBuster::Editor::SaveAsPopUp()
{
    if(state == PopUpState::SAVE_AS)
        ImGui::OpenPopup("Save as");

    auto displaySize = io_->DisplaySize;
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    if(ImGui::BeginPopupModal("Save as", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {   
        ImGui::InputText("File Name", fileName, 16, ImGuiInputTextFlags_EnterReturnsTrue);
        if(ImGui::Button("Accept"))
        {
            SaveMap();
            RenameWindow(fileName);
            newMap = false;

            ImGui::CloseCurrentPopup();
            state = PopUpState::NONE;
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            state = PopUpState::NONE;
        }

        ImGui::EndPopup();
    }
}

void BlockBuster::Editor::OpenMapPopUp()
{
    if(state == PopUpState::OPEN_MAP)
        ImGui::OpenPopup("Open Map");

    auto displaySize = io_->DisplaySize;
    ImGui::SetNextWindowPos(ImVec2{displaySize.x * 0.5f, displaySize.y * 0.5f}, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    if(ImGui::BeginPopupModal("Open Map", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {   
        ImGui::InputText("File Name", fileName, 16, ImGuiInputTextFlags_EnterReturnsTrue);
        if(ImGui::Button("Accept"))
        {
            LoadMap();
            RenameWindow(fileName);
            newMap = false;

            ImGui::CloseCurrentPopup();
            state = PopUpState::NONE;
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            state = PopUpState::NONE;
        }

        ImGui::EndPopup();
    }
}

void BlockBuster::Editor::MenuBar()
{
    // Pop Ups
    OpenMapPopUp();
    SaveAsPopUp();

    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("File", true))
        {
            if(ImGui::MenuItem("New Map", "Ctrl + N"))
            {
                InitMap();
                std::strcpy(fileName, "NewMap.bbm");
                RenameWindow("New Map");
                newMap = true;
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Open Map", "Ctrl + O"))
            {
                state = PopUpState::OPEN_MAP;
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Save", "Ctrl + S"))
            {
                if(newMap)
                    state = PopUpState::SAVE_AS;
                else
                    SaveMap();
            }

            if(ImGui::MenuItem("Save As", "Ctrl + Shift + S"))
            {
                state = PopUpState::SAVE_AS;
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit", true))
        {
            if(ImGui::MenuItem("Undo", "Ctrl + Z"))
            {
                std::cout << "Undoing\n";
            }

            if(ImGui::MenuItem("Redo", "Ctrl + Shift + Z"))
            {
                std::cout << "Redoing\n";
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Go to Block", "Ctrl + G"))
            {
                std::cout << "Going to block\n";
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Mouse camera mode", "C", mouseCamera))
            {
                mouseCamera = !mouseCamera;
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Settings", true))
        {
            if(ImGui::MenuItem("Graphics"))
            {

            }

            if(ImGui::MenuItem("Display"))
            {

            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void BlockBuster::Editor::GUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window_);
    ImGui::NewFrame();

    bool open;
    ImGui::SetNextWindowPos(ImVec2{0, 0}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2{(float)config.window.width, 0}, ImGuiCond_Always);
    auto windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar;
    if(ImGui::Begin("Editor", &open, windowFlags))
    {
        MenuBar();

        bool pbSelected = tool == PLACE_BLOCK;
        bool rotbSelected = tool == ROTATE_BLOCK;

        ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit;
        if(ImGui::BeginTable("#Tools", 2, tableFlags, ImVec2{0, 0}))
        {
            // Title Column 1
            ImGui::TableSetupColumn("Tools", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Tool Options", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            
            // Tools Table
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::BeginTable("##Tools", 2, 0, ImVec2{120, 0}))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if(ImGui::Selectable("Place", &pbSelected, 0, ImVec2{0, 0}))
                {
                    std::cout << "Placing block enabled\n";
                    tool = PLACE_BLOCK;
                }

                ImGui::TableNextColumn();
                if(ImGui::Selectable("Rotate", &rotbSelected, 0, ImVec2{0, 0}))
                {
                    std::cout << "Rotating block enabled\n";
                    tool = ROTATE_BLOCK;
                }

                ImGui::EndTable();
            }

            // Tools Options
            ImGui::TableNextColumn();

            if(pbSelected)
            {
                ImGui::Text("Block Type");
                ImGui::SameLine();
                ImGui::RadioButton("Block", &blockType, Game::BlockType::BLOCK);
                ImGui::SameLine();

                ImGui::RadioButton("Slope", &blockType, Game::BlockType::SLOPE);

                ImGui::Text("Display Type");
                ImGui::SameLine();
                ImGui::RadioButton("Texture", &displayType, Game::DisplayType::TEXTURE);
                ImGui::SameLine();

                ImGui::RadioButton("Color", &displayType, Game::DisplayType::COLOR);

                if(displayType == Game::DisplayType::COLOR)
                {
                    ImGui::Text("Color");
                    ImGui::SameLine();
                    ImGui::ColorEdit4("", &displayColor.x);
                }
            }

            if(rotbSelected)            
            {
                ImGui::Text("Axis");

                ImGui::SameLine();
                ImGui::RadioButton("X", &axis, RotationAxis::X);
                ImGui::SameLine();
                ImGui::RadioButton("Y", &axis, RotationAxis::Y);
                ImGui::SameLine();
                ImGui::RadioButton("Z", &axis, RotationAxis::Z);
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}