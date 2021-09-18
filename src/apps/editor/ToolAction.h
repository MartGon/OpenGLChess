#pragma once

#include <game/Block.h>
#include <game/Map.h>

#include <vector>
#include <memory>

namespace BlockBuster
{
    namespace Editor
    {
        using BlockData = std::pair<glm::ivec3, Game::Block>;
        enum class ActionType
        {
            LEFT_BUTTON,
            RIGHT_BUTTON,
            HOVER
        };

        class ToolAction
        {
        public:
            virtual ~ToolAction() {};

            virtual void Do() {};
            virtual void Undo() {};
        };

        class PlaceBlockAction : public ToolAction
        {
        public:
            PlaceBlockAction(glm::ivec3 pos, Game::Block block, Game::Map::Map* map) : 
                pos_{pos}, block_{block}, map_{map} {}

            void Do() override;
            void Undo() override;

        private:
            glm::ivec3 pos_;
            Game::Block block_;
            Game::Map::Map* map_;
        };

        class UpdateBlockAction : public ToolAction
        {
        public:
            UpdateBlockAction(glm::ivec3 pos, Game::Block update, Game::Map::Map* map) :
                pos_{pos}, update_{update}, map_{map}, prev_{*map_->GetBlock(pos)} {}

            void Do() override;
            void Undo() override;

        private:
            glm::ivec3 pos_;
            Game::Block update_;
            Game::Map::Map* map_;

            Game::Block prev_;
        };

        class RemoveAction : public ToolAction
        {
        public:
            RemoveAction(glm::ivec3 pos, Game::Block block, Game::Map::Map* map) : 
                pos_{pos}, block_{block}, map_{map} {}

            void Do() override;
            void Undo() override;

        private:
            glm::ivec3 pos_;
            Game::Block block_;
            Game::Map::Map* map_;
        };

        class PaintAction : public ToolAction
        {
        public:
            PaintAction(glm::ivec3 pos, Game::Block const* block, Game::Display display, Game::Map::Map* map) : 
                pos_{pos}, block_{block}, display_{display}, prevDisplay_{block->display}, map_{map} {}

            void Do() override;
            void Undo() override;

        private:

            void SetBlockDisplay(Game::Display d);

            glm::ivec3 pos_;

            Game::Block const* block_;
            Game::Display display_;
            Game::Display prevDisplay_;
            std::vector<Game::Block>* blocks_;
            
            Game::Map::Map* map_;
        };

        class RotateAction : public ToolAction
        {
        public:
            RotateAction(glm::ivec3 pos, Game::Block* block, Game::Map::Map* map, Game::BlockRot rot) :
                pos_{pos}, block_{block}, map_{map}, rot_{rot}, prevRot_{block->rot} {}

            void Do() override;
            void Undo() override;

        private:

            void SetBlockRot(Game::BlockRot rot);
            
            Game::Block* block_;

            glm::ivec3 pos_;
            Game::Map::Map* map_;
            Game::BlockRot rot_;
            Game::BlockRot prevRot_;
        };

        class MoveSelectionAction : public ToolAction
        {
        public:
            MoveSelectionAction(Game::Map::Map* map, std::vector<BlockData> selection, glm::ivec3 offset, glm::ivec3* cursorPos, std::vector<BlockData>* selTarget) :
                map_{map}, selection_{selection}, offset_{offset}, cursorPos_{cursorPos}, selTarget_{selTarget} {}
        
            void Do() override;
            void Undo() override;

        private:
            bool IsBlockInSelection(glm::ivec3 pos);
            void MoveSelection(glm::ivec3 offset);

            Game::Map::Map* map_;
            std::vector<std::pair<glm::ivec3, Game::Block>> selection_;
            glm::ivec3 offset_;

            glm::ivec3* cursorPos_;
            std::vector<BlockData>* selTarget_;
        };

        class BatchedAction : public ToolAction
        {
        public:
            BatchedAction(){}
            
            void Do() override;
            void Undo() override;

            void AddAction(std::unique_ptr<ToolAction>&& action);
        private:
            std::vector<std::unique_ptr<ToolAction>> actions_;
        };
    }
}