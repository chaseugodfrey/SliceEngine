/*!
\file		NetworkSystem.h
\author		Muhammad Rauan (muhammadrayan.b@digipen.edu)
\co-author
\brief
	Functions that deal with the networking stuff

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/

#ifndef NETWORKSYSTEM_H
#define	NETWORKSYSTEM_H

#include <mutex>
#include <unordered_map>
#include <fstream>
#include <array>
#include <cmath>
#include <ctime>
#include <random>

// Tell the Visual Studio linker to include the following library in linking.
// Alternatively, we could add this file to the linker command-line parameters,
// but including it in the source code simplifies the configuration.
#pragma comment(lib, "ws2_32.lib")

#include "winsock2.h"	// ...or Winsock alone
#include "ws2tcpip.h"		// getaddrinfo()
#include "Windows.h"		// Entire Win32 API...

#undef far
#undef near

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "ECS/GameObject.h"

#define WINSOCK_VERSION     2
#define WINSOCK_SUBVERSION  2
#define MAX_STR_LEN         1000

#define MAX_PLAYERS         2
#define TOTAL_PLAYERS       2
#define UPDATE_RATE         50
#define TIME_SYNC           5
#define TOTAL_TIME          60

namespace SliceEngine
{
	struct NetworkEntity {};
	struct NetworkObj 
    {
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
            Register name and it will set the id incrementally, and define a function for the command ID
        */
        void Register(const std::string& cmdName);

        /*
            Get id of registered name
        */
        uint8_t GetID(const std::string& name);

        /*
          Executes function associated with command name
        */
        //void ProcessFunc(const std::string& name, SOCKET, sockaddr_in player);

    private:
        uint8_t nextId = 1;
        std::unordered_map<std::string, uint8_t> cmdMap;
    };

    

    namespace NetworkingThread 
    {
        void printAddr();
        void ReceiveThread(SOCKET serverSock);
        void SendThread(SOCKET serverSocket);
    }


	struct NetworkSystem : BaseSystem<NetworkEntity, NetworkObj>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
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
    Packet& operator>>(Packet& msg, const T& data)
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

    // 1 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint8_t& data)
    {
        uint8_t tmp = data;

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

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

    // 2 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint16_t& data)
    {
        uint16_t tmp = htons(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

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

    // 4 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint32_t& data)
    {
        uint32_t tmp = htonl(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }

    // 4 byte float
    template<>
    inline Packet& operator<<(Packet& pkt, const float& data)
    {
        float tmp = htonf(data);

        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

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

    // 8 byte unsigned
    template<>
    inline Packet& operator<<(Packet& pkt, const uint64_t& data)
    {
        uint64_t tmp = htonll(data);
        
        auto ptr = reinterpret_cast<const uint8_t*>(&tmp);
        pkt.msg.insert(pkt.msg.end(), ptr, ptr + sizeof(tmp));

        return pkt;
    }



}

#endif

