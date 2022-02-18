
#include <app/App.h>

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <gl/Texture.h>

#include <rendering/Camera.h>
#include <rendering/Mesh.h>
#include <rendering/Primitive.h>
#include <rendering/RenderMgr.h>
#include <rendering/Rendering.h>
#include <rendering/Model.h>
#include <models/Respawn.h>
#include <game/ChunkMeshMgr.h>

#include <entity/PlayerController.h>
#include <entity/Block.h>
#include <entity/Map.h>
#include <entity/GameObject.h>
#include <game/CameraController.h>

#include <imgui/backends/imgui_impl_opengl3.h>

#include <Project.h>

#include <ToolAction.h>

#include <functional>

/* TODO LIST
    0a. Change Respawns to gameObject. Handle them in Map as a special case
    0b. Consider forbidding getting a reference to edit. Allow only to remove or place. Makes things easier overall to undo/redo
    1. Implement select object tool
    2. Change respawn color based on teamId
    3. Choose rotation in place block
*/

namespace BlockBuster
{
    namespace Editor
    {
        using BlockData = std::pair<glm::ivec3, Game::Block>;

        enum class MirrorPlane
        {
            XY,
            XZ,
            YZ,
            NOT_XY,
            NOT_XZ,
            NOT_YZ
        };

        class Editor : public App::App
        {
        public:
            Editor(::App::Configuration config) : App{config} {}

            void Start() override;
            void Update() override;
            void Shutdown() override;
            bool Quit() override;
            
        private:

            // Enums
            enum PopUpState
            {
                NONE,
                SAVE_AS,
                OPEN_MAP,
                LOAD_TEXTURE,
                UNSAVED_WARNING,
                VIDEO_SETTINGS,
                GO_TO_BLOCK,
                SET_TEXTURE_FOLDER,
                MAX
            };

            enum TabState
            {
                TOOLS_TAB,
                OPTIONS_TAB,

                DEBUG_TAB
            };

            enum class CursorMode
            {
                BLOCKS,
                SCALED
            };

            enum Tool
            {
                PLACE_BLOCK,
                ROTATE_BLOCK,
                PAINT_BLOCK,
                SELECT_BLOCKS,

                PLACE_OBJECT,
                SELECT_OBJECT
            };

            enum SelectSubTool
            {
                MOVE,
                EDIT,
                ROTATE_OR_MIRROR,
                FILL_OR_PAINT,
                END
            };

            struct Result
            {
                bool isOk = true;
                std::string info;
            };

            // Rendering
            Rendering::Mesh& GetMesh(Game::BlockType type);
            glm::vec4 GetBorderColor(glm::vec4 basecolor, glm::vec4 darkColor = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, glm::vec4 lightColor = glm::vec4{1.0f});
            Util::Result<bool> LoadTexture();
            bool IsTextureInPalette(std::filesystem::path folder, std::filesystem::path textureName);

            // World
            void NewProject();
            void SaveProject();
            Util::Result<bool> OpenProject();

            // Editor
            void UpdateEditor();
            void SetCameraMode(::App::Client::CameraMode cameraMode);
            void HandleKeyShortCut(const SDL_KeyboardEvent& e);
            void SetUnsaved(bool unsaved);
            void Exit();

            // Tools - General
            void SelectTool(Tool tool);
            void UseTool(glm::vec<2, int> mousePos, ActionType actionType = ActionType::LEFT_BUTTON);
            void QueueAction(std::unique_ptr<ToolAction> action);
            void DoToolAction(std::unique_ptr<ToolAction> action);
            void DoToolAction();
            void UndoToolAction();
            void ClearActionHistory();

            // Paint tool
            Game::Display GetBlockDisplay();
            bool IsDisplayValid();
            void SetBlockDisplay(Game::Display display);

            // Selection cursor
            void DrawCursor(Math::Transform t);
            void DrawSelectCursor(glm::ivec3 pos);
            void EnumBlocksInSelection(std::function<void(glm::ivec3 pos, glm::ivec3 offset)> onEach);
            std::vector<BlockData> GetBlocksInSelection(bool globalPos = true);
            void SelectBlocks();
            void ClearSelection();
            bool CanMoveSelection(glm::ivec3 offset);
            bool IsBlockInSelection(glm::ivec3 pos);
            void MoveSelection(glm::ivec3 offset);
            void MoveSelectionCursor(glm::ivec3 nextPos);

            // Select Tool
            void CopySelection();
            void RemoveSelection();
            void CutSelection();
            void PasteSelection();
            // TODO: Change this to use Util::Result / Oktal::Result
            Result RotateSelection(Game::RotationAxis axis, Game::RotType rotType);
            Result MirrorSelection(MirrorPlane plane);
            void FillSelection();
            void ReplaceSelection();
            void PaintSelection();
            void OnChooseSelectSubTool(SelectSubTool subTool);

            // Object Tool
            void SelectGameObject(glm::ivec3 pos);
            void EditGameObject();

            // Test Mode
            void UpdatePlayerMode();

            // Options
            void HandleWindowEvent(SDL_WindowEvent winEvent);
            void ApplyVideoOptions(::App::Configuration::WindowConfig& config);
            std::string GetConfigOption(const std::string& key, std::string defaultValue = "");

            // GUI - PopUps
            struct PopUp
            {
                std::string name;
                std::function<void()> update;
                std::function<void()> onOpen = []{};
                std::function<void(bool)> onClose = [](bool){};
            }; 
            struct EditTextPopUpParams
            {
                PopUpState popUpState;
                std::string name;
                char* textBuffer; 
                size_t bufferSize;
                std::function<Util::Result<bool>()> onAccept;
                std::function<void()> onCancel;
                std::string errorPrefix;

                bool& onError;
                std::string& errorText;
            };
            struct BasicPopUpParams
            {
                PopUpState state;
                std::string name;
                std::function<void()> inPopUp;
            };

            void EditTextPopUp(const EditTextPopUpParams& params);
            void BasicPopUp(const BasicPopUpParams& params);

            void OpenMapPopUp();
            void SaveAsPopUp();
            void LoadTexturePopUp();
            void VideoOptionsPopUp();
            void UnsavedWarningPopUp();
            void GoToBlockPopUp();
            void SetTextureFolderPopUp();

            void OpenWarningPopUp(std::function<void()> onExit);

            void InitPopUps();
            void OpenPopUp(PopUpState puState);
            void UpdatePopUp();
            void ClosePopUp(bool accept = false);

            // Widgets
            void HelpMarker(const char* text);

            // GUI
            void MenuBar();

            // GUI - Misc
            void SyncGUITextures();

            // Menu - File
            void MenuNewMap();
            void MenuOpenMap();
            void MenuSave();
            void MenuSaveAs();

            // GUI - Help
            void HelpShortCutWindow();

            // Tools - GUI
            void ToolsTab();
            void PlaceBlockGUI();
            void SelectBlockTypeGUI();
            void SelectBlockDisplayGUI();
            void RotateBlockGUI();
            void SelectBlocksGUI();
            void PlaceObjectGUI();

            // GameObject
            void PropertyInput(Entity::GameObject* go, const char* key, Entity::GameObject::Property::Type type);

            void ToolOptionsGUI();
            void GUI();

            // Project
            Project project;

            // Rendering
            Rendering::RenderMgr renderMgr;
            GL::Shader shader;
            GL::Shader chunkShader;
            Rendering::Mesh cube;
            Rendering::Mesh slope;
            Rendering::Mesh cylinder;
            Rendering::Camera camera;
            ::App::Client::CameraController cameraController;

            // Models
            Game::Models::Respawn respawnModel;
            
            // Editor
            bool quit = false;
            bool unsaved = true;
            std::function<void()> onWarningExit;
            glm::ivec3 goToPos;

            // Tools
            Tool tool = PLACE_BLOCK;
            unsigned int actionIndex = 0;
            std::vector<std::unique_ptr<BlockBuster::Editor::ToolAction>> actionHistory;

            // Tool - Place
            Game::BlockType blockType = Game::BlockType::BLOCK;

            // Tool - Rotate
            Game::RotationAxis axis = Game::RotationAxis::Y;

            // Tool - Paint
            Game::DisplayType displayType = Game::DisplayType::COLOR;
            int textureId = 0;
            int colorId = 0;

            char textureFilename[32] = "texture.png";

            const int MAX_TEXTURES = 32;
            std::vector<ImGui::Impl::Texture> guiTextures;

            bool pickingColor = false;
            bool newColor = false;
            glm::vec4 colorPick;

            const glm::vec4 yellow = glm::vec4{1.0f, 1.0f, 0.0f, 1.0f};
            const glm::vec4 darkBlue = glm::vec4{20.f / 255.f, 0.0f, 0.5f, 1.0f};

            glm::ivec3 pointedBlockPos;

            // Tools - Cursor
            struct Cursor{
                bool show = true;
                bool enabled = false;
                glm::ivec3 pos = glm::ivec3{0};
                Game::BlockRot rot {Game::RotType::ROT_0, Game::RotType::ROT_0};
                glm::ivec3 scale{1};
                glm::vec4 color = glm::vec4{1.0f, 1.0f, 0.0f, 1.0f};
                Game::BlockType type = Game::BlockType::BLOCK;
                CursorMode mode = CursorMode::SCALED;
            };
            bool intersecting = false;
            Cursor cursor;

            // Tools - Select
            std::vector<BlockData> selection;
            bool movingSelection = false;
            glm::ivec3 savedPos{0};
            SelectSubTool selectTool = SelectSubTool::MOVE;

            // Tools - Copy/Cut
            std::vector<BlockData> clipboard;

            // Tools - Select - Rotate or Mirror
            Game::RotationAxis selectRotAxis = Game::RotationAxis::Y;
            MirrorPlane selectMirrorPlane = MirrorPlane::XY;
            Game::RotType selectRotType = Game::RotType::ROT_90;
            std::string selectRotErrorText;

            // Objects
            Entity::GameObject placedGo;
            int objectType = Entity::GameObject::RESPAWN;
            glm::ivec3 selectedObj{0};

            // GUIGL
            GL::VertexArray gui_vao;
            PopUpState state = PopUpState::NONE;
            TabState tabState = TabState::TOOLS_TAB;
            bool onError = false;
            std::string errorText;
            PopUp popUps[PopUpState::MAX];

            // File
            std::filesystem::path mapsFolder = ".";
            char fileName[16];
            char textureFolderPath[128] = "";
            const int magicNumber = 0xB010F0;
            bool newMap = true;

            // Config
            ::App::Configuration::WindowConfig preConfig;

            // Options
            float blockScale = 2.0f;

            // Debug
            #ifdef _DEBUG
                uint32_t modelId = 2;
                float sliderPrecision = 5.0f;
                glm::vec3 modelOffset{0.0f};
                glm::vec3 modelRot{0.0f};
                glm::vec3 modelScale{1.0f};
                bool showDemo = false;
                bool newMapSys = true;
                bool optimizeIntersection = true;
                bool useTextureArray = true;
            #endif
            bool drawChunkBorders = false;

            // Help
            bool showShortcutWindow = false;

            // Test
            bool playerMode = false;
            Entity::PlayerController player;
        };
    }
}