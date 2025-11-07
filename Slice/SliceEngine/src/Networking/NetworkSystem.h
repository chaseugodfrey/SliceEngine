/*!
\file		NetworkSystem.h
\author		Muhammad Rayan (muhammadrayan.b@digipen.edu)
\co-author
\brief
	Functions that deal with the networking stuff

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/

#ifdef NETWORKSYSTEM_H
//#define	NETWORKSYSTEM_H

#include <mutex>
#include <unordered_map>
#include <fstream>
#include <array>
#include <cmath>
#include <ctime>
#include <random>

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "ECS/GameObject.h"
#include "../Core/Events.h"

#undef WINSOCK_VERSION
#define WINSOCKK_VERSION     2
#define WINSOCK_SUBVERSION  2
#define MAX_STR_LEN         1000

#define MAX_PLAYERS         2
#define TOTAL_PLAYERS       2
#define UPDATE_RATE         50
#define TIME_SYNC           2
#define TOTAL_TIME          60

namespace SliceEngine
{
	struct NetworkEntity 
    {
    };

	struct NetworkObj 
    {
        bool client = false;
        SOCKET soc;
        std::string port;
        std::string IP;
        sockaddr_in otherPlayer;
    };


    struct Player
    {
        Transform go;
        //int score;
        float timestamp;
    };

    struct Packet 
    {
        std::vector<uint8_t> msg;
        size_t offset = 0;
    };



    //std::unordered_map<std::string, sockaddr_in> clients;  // Map of "IP:Port" -> SOCKET

    // Stores id and function pointer for the command
    // Size function pointer - 8 bytes, id - 1 byte
   /* struct CmdObj
    {
        void (*execFunc)(SOCKET,sockaddr_in);
        uint8_t id;
    };*/


	/// <summary>
    /// 
	/// Network Command ID to set how each will send and rcv bytes.
	/// 
	/// </summary>
    class NetworkCommandID 
    {
    public:
        /*
            Register name and it will set the id incrementally
        */
        void Register(const std::string& cmdName);

        /*
            Get id of registered name
        */
        uint8_t GetID(const std::string& name);

    private:
        uint8_t nextId = 1;
        std::unordered_map<std::string, uint8_t> cmdMap;
    };

    

    namespace NetworkingThread 
    {
        extern std::unordered_map<uint32_t, uint32_t> HtoCID;
        extern std::unordered_map<uint32_t, uint32_t> CtoHID;

        void printAddr();
        void ReceiveThread(SOCKET serverSock);
        void SendThread(SOCKET serverSock,bool client, sockaddr_in otherPlayer);
        void SendTo(const SOCKET& Sock, const Packet& pkt, sockaddr_in pAddr);
        int RecvFrom(const SOCKET& Sock, Packet& pkt, sockaddr_in& pAddr, int& size);
    }


	struct NetworkSystem //: BaseSystem<NetworkEntity, NetworkObj>
	{
        NetworkObj data{};

        //entt::entity currEntity;

		//void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		//void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		//void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
        void Init();
        void Exit();
        void BindSocket(const NetworkBindPortEvent& event);
        void UpdateObjects();
        void SubscribeToAllNetworkEvents();
        void OnConnectReq(const NetworkClientConnectEvent& event);
        void OnGONetworkEvent(const GONetworkEvent& event);
	};

    // general template
    template<typename T>
    Packet& operator<<(Packet& msg, const T& data)
    {
        static_assert(sizeof(T) == 0, "Unsupported");

        return msg;
    }

    // general template
    template<typename T>
    Packet& operator>>(Packet& msg, T& data)
    {
        static_assert(sizeof(T) == 0, "Unsupported");

        return msg;
    }

    // 1 byte signed
    template<>
    inline Packet& operator<<(Packet& pkt, const int8_t& data)
    {
        int8_t tmp = static_cast<uint8_t>(data);

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, int8_t& data)
    {
        data = static_cast<int8_t>(pkt.msg[pkt.offset++]);
        return pkt;
    }

    // 1 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint8_t& data)
    {
        uint8_t tmp = data;

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, uint8_t& data)
    {
        data = pkt.msg[pkt.offset++];
        return pkt;
    }

    // 2 byte signed
    template<>
    inline Packet& operator<<(Packet& pkt, const int16_t& data)
    {
        uint16_t tmp = htons(static_cast<uint16_t>(data));

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, int16_t& data)
    {
        int16_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = static_cast<int16_t>(ntohs(tmp));
        return pkt;
    }

    // 2 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint16_t& data)
    {
        uint16_t tmp = htons(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, uint16_t& data)
    {
        uint16_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = ntohs(tmp);
        return pkt;
    }

    // 4 byte signed
    template<>
    inline Packet& operator<<(Packet& pkt, const int32_t& data)
    {
        uint32_t tmp = htonl(static_cast<uint32_t>(data));
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, int32_t& data)
    {
        int32_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = static_cast<int32_t>(ntohl(tmp));
        return pkt;
    }

    // 4 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint32_t& data)
    {
        uint32_t tmp = htonl(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, uint32_t& data)
    {
        uint32_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = ntohl(tmp);
        return pkt;
    }

    // 4 byte float
    template<>
    inline Packet& operator<<(Packet& pkt, const float& data)
    {
        //float tmp = htonf(data);
        uint32_t tmp{};
        std::memcpy(&tmp, &data, 4);
        tmp = htonl(tmp);

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, float& data)
    {
        //float tmp{};
        uint32_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        //data = static_cast<float>(ntohf(tmp));

        uint32_t bits_host = ntohl(tmp);
        std::memcpy(&data, &bits_host, 4);
        return pkt;
    }

    // 8 byte signed
    template<>
    inline Packet& operator<<(Packet& pkt, const int64_t& data)
    {
        uint64_t tmp = htonll(static_cast<uint64_t>(data));
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, int64_t& data)
    {
        int64_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = static_cast<int64_t>(ntohll(tmp));
        return pkt;
    }

    // 8 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint64_t& data)
    {
        uint64_t tmp = htonll(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    template<>
    inline Packet& operator>>(Packet& pkt, uint64_t& data)
    {
        uint64_t tmp{};
        std::memcpy(&tmp, pkt.msg.data() + pkt.offset, sizeof(tmp));
        pkt.offset += sizeof(tmp);
        data = ntohll(tmp);
        return pkt;
    }



}

#endif

