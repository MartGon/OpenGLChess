#pragma once

#include <math/Transform.h>

#include <string>

namespace Game
{
    enum DisplayType : uint8_t
    {
        TEXTURE,
        COLOR
    };

    struct Display
    {
        DisplayType type;
        int id;
    };

    enum BlockType : uint8_t
    {
        NONE,
        BLOCK,
        SLOPE
    };

    enum RotType : uint8_t
    {
        ROT_0,
        ROT_90,
        ROT_180,
        ROT_270
    };

    struct BlockRot 
    {
        RotType y = ROT_0;
        RotType z = ROT_0;
    };

    class Block
    {
    public:

        BlockType type;
        BlockRot rot;
        Display display;

        glm::vec3 GetRotation() const;
    };

    Math::Transform GetBlockTransform(const Block& block, glm::ivec3 pos, float blockScale);
}