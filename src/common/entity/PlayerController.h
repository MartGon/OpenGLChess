#pragma once

#include <math/Transform.h>

#include <entity/Block.h>
#include <entity/Map.h>
#include <entity/Player.h>

#include <util/Timer.h>

#include <vector>

namespace Entity
{
    class PlayerController
    {
    public:

        void Update(Entity::PlayerInput input, Game::Map::Map* map, Util::Time::Seconds deltaTime);
        glm::vec3 HandleCollisions(Game::Map::Map* map, float blockScale, bool gravity);
        glm::vec3 HandleCollisions(const std::vector<std::pair<Math::Transform, Game::Block>> &blocks, bool gravity);
        
        Math::Transform transform;

        Math::Transform GetECB();
        Math::Transform GetGCB();

        float speed = 5.f;
        float height = 2.0f;
        float gravitySpeed = -0.4f;
    private:
        glm::vec3 HandleGravityCollisions(Game::Map::Map* map, float blockScale);
        std::optional<glm::vec3> HandleGravityCollisionBlock(Game::Map::Map* map, glm::vec3 bottomPoint, glm::ivec3 blockIndex, glm::vec3 offset);
    };

}