#pragma once


#include <memory>

#include <networking/Networking.h>
#include <entity/Player.h>
#include <Snapshot.h>

namespace Networking
{
    enum OpcodeClient : uint16_t
    {
        OPCODE_CLIENT_BATCH,
        OPCODE_CLIENT_INPUT
    };

    enum OpcodeServer : uint16_t
    {
        OPCODE_SERVER_BATCH,
        OPCODE_SERVER_WELCOME,
        OPCODE_SERVER_SNAPSHOT,
        OPCODE_SERVER_PLAYER_DISCONNECTED
    };

    enum class PacketType : uint8_t
    {
        Server,
        Client
    };

    template <PacketType type>
    std::unique_ptr<Packet> MakePacket(uint16_t opCode);

    template <PacketType Type>
    class Batch : public Packet
    {
    public:
        Batch();

        void OnRead(Util::Buffer::Reader reader) override
        {
            while(!reader.IsOver())
            {
                auto len = reader.Read<uint32_t>();
                auto opCode = reader.Read<uint16_t>();

                auto packetBuffer = reader.Read(len - sizeof(opCode));
                std::unique_ptr<Packet> packet = MakePacket<Type>(opCode);
                packet->SetBuffer(std::move(packetBuffer));
                packet->Read();

                packets.push_back(std::move(packet));
            }
        }

        void OnWrite() override
        {
            buffer.Clear();

            for(auto& packet : packets)
            {
                packet->Write();
                buffer.Write(packet->GetSize());
                buffer.Append(std::move(*packet->GetBuffer()));
            }
        }

        inline uint32_t GetPacketCount()
        {
            return packets.size();
        }

        inline Packet* GetPacket(uint32_t index)
        {
            Packet* packet = nullptr;
            if(index < GetPacketCount())
                packet = packets[index].get();
            return packet;
        }

        inline void PushPacket(std::unique_ptr<Packet>&& packet)
        {
            packets.push_back(std::move(packet));
        }

    private:
        std::vector<std::unique_ptr<Packet>> packets;
    };

    namespace Packets
    {
        namespace Server
        {
            class Welcome final : public Packet
            {
            public:
                Welcome() : Packet{OpcodeServer::OPCODE_SERVER_WELCOME}
                {

                }
                void OnRead(Util::Buffer::Reader reader) override;
                void OnWrite() override;

                uint8_t playerId;
                double tickRate;
            };

            class WorldUpdate final : public Packet
            {
            public:
                WorldUpdate() : Packet{OpcodeServer::OPCODE_SERVER_SNAPSHOT}
                {

                }
                void OnRead(Util::Buffer::Reader reader) override;
                void OnWrite() override;

                uint32_t lastCmd = 0;
                Snapshot snapShot;
            };
            
            class PlayerDisconnected final : public Packet
            {
            public:
                void OnRead(Util::Buffer::Reader reader) override;
                void OnWrite() override;

            private:
            };
        }

        namespace Client
        {
            class Input final : public Packet
            {   
            public:

                struct Req
                {
                    uint32_t reqId;
                    Entity::PlayerInput playerInput;
                    float camYaw;
                    float camPitch;
                };

                Input() : Packet{OpcodeClient::OPCODE_CLIENT_INPUT}
                {

                }
                void OnRead(Util::Buffer::Reader reader) override;
                void OnWrite() override;

                Req req;
            };
        }
    }
}