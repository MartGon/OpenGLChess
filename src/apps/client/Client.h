
#include <app/App.h>

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <gl/Texture.h>

#include <rendering/Camera.h>
#include <rendering/Mesh.h>
#include <rendering/Primitive.h>
#include <rendering/Rendering.h>

#include <game/PlayerController.h>
#include <game/Map.h>
#include <game/CameraController.h>
#include <game/ChunkMeshMgr.h>

#include <util/Time.h>
#include <util/Queue.h>

#include <entity/Player.h>

#include <networking/enetw/ENetW.h>
#include <networking/Command.h>
#include <networking/Snapshot.h>

namespace BlockBuster
{
    class Client : public App::App
    {
    public:
        Client(::App::Configuration config);

        void Start() override;
        void Update() override;
        bool Quit() override;
        
    private:

        void DoUpdate(double deltaTime);

        // Input
        void HandleSDLEvents();

        // Networking
        void RecvServerSnapshots();
        void UpdateNetworking();

        // Networking - Prediction
        void PredictPlayerMovement(Networking::Command cmd, uint32_t cmdId);
        void SmoothPlayerMovement();
        glm::vec3 PredPlayerPos(glm::vec3 pos, glm::vec3 moveDir, float deltaTime);

        // Networking - Entity Interpolation
        uint64_t GetCurrentTime();
        uint64_t GetRenderTime();
        std::optional<Networking::Snapshot> GetMostRecentSnapshot();
        uint64_t TickToMillis(uint32_t tick);
        void EntityInterpolation();
        void EntityInterpolation(Networking::Snapshot a, Networking::Snapshot b, float alpha);
        void CalculateExtrapolatedSnapshot();
        void EntityExtrapolation();

        // Rendering
        void DrawScene();
        void DrawGUI();
        void Render();

        // Scene
        ::App::Client::Map map_;

        // Rendering
        GL::Shader shader;
        GL::Shader chunkShader;
        Rendering::Mesh cylinder;
        Rendering::Camera camera_;
        int drawMode = GL_FILL;

        double prevRenderTime = 0.0;
        double frameInterval = 0.0;
        double maxFPS = 60.0;
        double minFrameInterval = 0.0;

        // Updates
        const double UPDATE_RATE = 0.020;

        // Controls
        ::App::Client::CameraController camController_;

        // Player transforms
        std::unordered_map<Entity::ID, Entity::Player> playerTable;
        std::unordered_map<Entity::ID, Entity::Player> prevPlayerPos;
        float PLAYER_SPEED = 2.f;

        // Networking
        ENet::Host host;
        ENet::PeerId serverId = 0;
        uint8_t playerId = 0;
        double serverTickRate = 0.0;
        bool connected = false;
        Util::Queue<Networking::Snapshot> snapshotHistory{16};

        // Networking - Prediction
        struct Prediction
        {
            uint32_t cmdId;
            Networking::Command cmd;
            glm::vec3 origin;
            glm::vec3 dest;
            double time;
        };
        Util::Queue<Prediction> predictionHistory_{128};
        uint32_t cmdId = 0;
        uint32_t lastAck = 0;

        // Networking - Entity Interpolation
        const double EXTRAPOLATION_DURATION = 0.25;
        Networking::Snapshot extrapolatedSnapshot;
        uint64_t offsetMillis = 0;

        // App
        bool quit = false;
    };
}