#include <pch.h>
#include "NetworkSystem.h"
#include <fstream>
#include "../Core/EventManager.h"
#include "../Core/Core.h"
#include "../Systems/FramerateManager.h"
#include "../ECS/GOFactory.h"

namespace SliceEngine
{
    namespace
    {
        bool keep_running{};
        std::mutex _gameObjectMutex{};
        std::mutex _stdoutMutex{};
        std::mutex _eventMutex{};
        std::queue<float> event_queue{};
        float latest_server_update{};

        bool hasConnected = false;
        float timer = TIME_SYNC;

        bool gameStart = false;
        float appTime{};
        float latestTimeStamp{};

        Player otherPlayer{};
        bool player2 = false;

        NetworkCommandID cmdIDs{};
    }

	void NetworkCommandID::Register(const std::string& cmdName)
	{
        // name doesnt exist
        if (cmdMap.find(cmdName) == cmdMap.end())
        {
            uint8_t value = nextId++;
            cmdMap[cmdName] = value;
        }
	}

    uint8_t NetworkCommandID::GetID(const std::string& cmdName)
	{
		if (cmdMap.find(cmdName) != cmdMap.end())
		{
			return cmdMap.at(cmdName);
		}
		else 
		{
			std::cout << "No function registered for ID: " << cmdName << std::endl;
		}
		
	}

	/*void NetworkCommandID::ProcessFunc(const std::string& cmdName, SOCKET pSocket, sockaddr_in pAddr)
	{
		if (cmdMap.find(cmdName) != cmdMap.end())
		{
			cmdMap.at(cmdName).execFunc(pSocket,pAddr);
		}
		else
		{
			std::cout << "No function registered for ID: " << cmdName << std::endl;
		}
	}*/

    void NetworkingThread::printAddr()
    {
        std::string clientNumber{};
        std::string portNumber{};
        std::ifstream ifile("Assets/client.txt");
        if (!ifile)
        {
            std::cerr << "cannot open client file" << std::endl;
        }
        std::getline(ifile, clientNumber);
        ifile.close();

        if (std::stoi(clientNumber) == 0)
        {
            portNumber = "12345";
        }

        if (std::stoi(clientNumber) == 1)
        {
            portNumber = "12346";
            player2 = true;
        }

        sockaddr_in cAddr{};

        // Initialize Winsock
        WSADATA wsaData{};
        int errorCode = WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData);
        if (errorCode != NO_ERROR)
        {
            std::cerr << "WSAStartup() failed." << std::endl;
            //return errorCode;
        }

        // Object hints indicates which protocols to use to fill in the info.
        addrinfo hints{};
        SecureZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;			// IPv4
        // For UDP use SOCK_DGRAM instead of SOCK_STREAM.
        hints.ai_socktype = SOCK_STREAM;	// Reliable delivery
        // Could be 0 for autodetect, but reliable delivery over IPv4 is always TCP.
        hints.ai_protocol = IPPROTO_UDP;	// UDP
        // Create a passive socket that is suitable for bind() and listen().
        hints.ai_flags = AI_PASSIVE;

        SOCKET soc{ socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) };
        if (soc == INVALID_SOCKET)
        {
            std::cerr << "UDP socket() failed." << std::endl;
            //return false;
        }


        char host[MAX_STR_LEN];
        gethostname(host, MAX_STR_LEN);

        addrinfo* info = nullptr;
        errorCode = getaddrinfo(host, portNumber.c_str(), &hints, &info);
        if ((errorCode) || (info == nullptr))
        {
            std::cerr << "getaddrinfo() failed." << std::endl;
            WSACleanup();
            //return errorCode;
        }

        // Set up server address
        cAddr.sin_family = AF_INET;
        cAddr.sin_addr.s_addr = INADDR_ANY;
        cAddr.sin_port = htons(std::stoi(portNumber));

        // Bind the socket
        if (bind(soc, reinterpret_cast<sockaddr*>(&cAddr), sizeof(cAddr)) != NO_ERROR)
        {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(soc);
            WSACleanup();
            throw std::runtime_error("Bind failed");
        }

        u_long enable = 1;
        ioctlsocket(soc, FIONBIO, &enable);

        char serverIPAddr[MAX_STR_LEN];
        inet_ntop(AF_INET, &(cAddr.sin_addr), serverIPAddr, INET_ADDRSTRLEN);
        getnameinfo(info->ai_addr, static_cast <socklen_t> (info->ai_addrlen), serverIPAddr, sizeof(serverIPAddr), nullptr, 0, NI_NUMERICHOST);

        std::cout << "Server is listening on port " << portNumber << " ip " << serverIPAddr << " Player: " << clientNumber << " ...\n";


        // REGISTER ID HERE
        cmdIDs.Register("N_REQ_CONNECT");
        cmdIDs.Register("N_RSP_CONNECT");
        cmdIDs.Register("N_TIME_UPDATE");


        keep_running = true;

        // split the threads
        std::thread recv_thread(ReceiveThread, soc);
        recv_thread.detach();
         
        if (player2)
        {
            std::cout << "sending req....\n";

            sockaddr_in player1Dest{};
            player1Dest.sin_family = AF_INET;		//ipv4
            player1Dest.sin_port = htons((u_short)std::stoi("12345"));
            inet_pton(AF_INET, serverIPAddr, &player1Dest.sin_addr);

            Packet pkt{};
            pkt << cmdIDs.GetID("N_REQ_CONNECT");

            //int bytes = { sendto(soc, reinterpret_cast<const char*>(pkt.msg.data()), (int)pkt.msg.size(), 0, reinterpret_cast<sockaddr*>(&player1Dest), sizeof(player1Dest)) };
            SendTo(soc, pkt, player1Dest);
        }
    }

    void NetworkingThread::SendTo(const SOCKET& soc, const Packet& pkt, sockaddr_in pAddr)
    {
        int bytes = sendto(soc, reinterpret_cast<const char*>(pkt.msg.data()), (int)pkt.msg.size(), 0, reinterpret_cast<sockaddr*>(&pAddr), sizeof(pAddr));
        if (bytes == SOCKET_ERROR || bytes == 0)
        {
            std::cerr << "UDP send fail: " << WSAGetLastError() << std::endl;
            //closesocket(pSocket);
        }
    }

    int NetworkingThread::RecvFrom(const SOCKET& soc, Packet& pkt, sockaddr_in& pAddr, int& size)
    {
        char buffer[MAX_STR_LEN];
        int bytes =  recvfrom(soc, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*> (&pAddr), &size);
        pkt.msg.insert(pkt.msg.end(), buffer, buffer + std::strlen(buffer));

        return bytes;
    }

    void NetworkingThread::ReceiveThread(SOCKET otherPlayerSoc)
	{
        //char buffer[MAX_STR_LEN];
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);

        while (keep_running)
        {
            Packet recvPkt{};
            recvPkt.msg.reserve(MAX_STR_LEN);
            //int bytes_received = recvfrom(otherPlayerSoc, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*> (&client_addr), &client_addr_len);
            int bytes_received = RecvFrom(otherPlayerSoc, recvPkt, client_addr, client_addr_len);

            if (bytes_received == SOCKET_ERROR) 
            {
                //std::cerr << "Recvfrom failed: " << WSAGetLastError() << std::endl;
                //std::this_thread::sleep_for(std::chrono::milliseconds(40)); // Avoid tight loop
                //continue;

                size_t errorCode = WSAGetLastError();
                if (errorCode == WSAEWOULDBLOCK)
                {
                    // A non-blocking call returned no data; sleep and try again.
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_RATE));

                    std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
                    //std::cerr << "trying again..." << std::endl;
                    continue;
                }
                std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
                std::cerr << "recv() failed." << std::endl;
                break;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

            uint8_t inID{};
            recvPkt >> inID;

            //std::string IpPort = client_ip;
            //IpPort += ":";
            //IpPort += std::to_string(ntohs(client_addr.sin_port));

            if (inID == cmdIDs.GetID("N_REQ_CONNECT"))
            {
                std::cout << "REQ received....\n";
                Packet pkt{};
                pkt << cmdIDs.GetID("N_RSP_CONNECT");
                SendTo(otherPlayerSoc, pkt, client_addr);
            }

            if (inID == cmdIDs.GetID("N_RSP_CONNECT"))
            {

                std::cout << "connected " << std::endl;
                hasConnected = true;
            }

            // Player fire
            if (inID == cmdIDs.GetID("N_REQ_FIRE"))
            {
                //int tmpId{};
                //// find player ID who sent

                //{
                //    std::lock_guard<std::mutex> lock(_eventMutex);
                //    for (auto& player : playersIndex)
                //    {
                //        if (player.first == IpPort)
                //        {
                //            tmpId = player.second;
                //        }
                //    }
                //}


                float timestamp = ntohf(*(uint32_t*)(inID + 1));

                std::string message{};
                message += cmdIDs.GetID("N_RSP_FIRE");

                unsigned int tmp = htonf(appTime);
                message.append((char*)(&tmp), (char*)(&tmp) + 4);

                /*tmp = htonl(tmpId);
                message.append((char*)(&tmp), (char*)(&tmp) + 4);*/

                // Send to all that player ID fire
                //for (auto& ips : clients)
                {
                    sendto(otherPlayerSoc, message.c_str(), (int)message.length(), 0, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
                }

                //Shoot(playersInfo[tmpId].go.t.pos, playersInfo[tmpId].go.t.rot, tmpId);
            }

            // State update from client
            // id - 1b, timestamp - 4b, pos - 8b, scale - 8b, rot - 4b, vel - 8b
            if (inID == cmdIDs.GetID("N_STATE_UPDATE"))
            {
                //int tmpId{};
                //// find player ID who sent
                //{
                //    std::lock_guard<std::mutex> lock(_eventMutex);
                //    for (auto& player : playersIndex)
                //    {
                //        if (player.first == IpPort)
                //        {
                //            tmpId = player.second;
                //        }
                //    }
                //}

                latestTimeStamp = ntohf(*(uint32_t*)(inID + 1));

                {
                    std::lock_guard<std::mutex> lock(_eventMutex);
                    if (latestTimeStamp > otherPlayer.timestamp)
                    {
                        otherPlayer.timestamp = latestTimeStamp;
                    }
                    else
                    {
                        continue;
                    }
                }

                Vector2 pos{0.f,0.f};
                pos.x = ntohf(*(uint32_t*)(inID + 5));
                pos.y = ntohf(*(uint32_t*)(inID + 9));

                Vector2 scale{ 0.f,0.f };
                scale.x = ntohf(*(uint32_t*)(inID + 13));
                scale.y = ntohf(*(uint32_t*)(inID + 17));

                float rot = ntohf(*(uint32_t*)(inID + 21));

                Vector2 vel{ 0.f,0.f };
                vel.x = ntohf(*(uint32_t*)(inID + 25));
                vel.y = ntohf(*(uint32_t*)(inID + 29));

                {
                    std::lock_guard<std::mutex> lock(_eventMutex);
                    otherPlayer.go.position = glm::vec3{}; // = pos;
                    otherPlayer.go.scale = glm::vec3{}; //scale;
                    otherPlayer.go.rotation = glm::vec3{}; //rot;
                    //playersInfo[tmpId].go.vel = vel;

                    // interpolate
                    //InterpolateGameobject(playersInfo[tmpId].go, latest_timestamp);
                }
            }


            {
                //std::lock_guard<std::mutex> lock(_eventMutex); // RAII lock
                //buffer[bytes_received] = '\0';

                //std::cout << "Received from " << IpPort << std::endl;
            }
        }
	}

    void NetworkingThread::SendThread(SOCKET serverSocket)
    {
        while (keep_running)
        {
            if (hasConnected)
            {
                if (timer <= 0.0f)
                {
                    timer = TIME_SYNC;
                }

                auto dt = Core::GetInstance()->GetFramerateManager()->getDeltaTime();
                auto& reg = Core::GetInstance()->GetRegistry();
                auto& GOfact = Core::GetInstance()->mFactory;

                timer -= dt;

                auto entityView = reg.view<SliceEntity>();
                for (auto entity : entityView)
                {
                    //std::cout << mEntityToGO[entity].GetName() << std::endl;
                    Packet pkt{};
                    pkt << cmdIDs.GetID("N_TIME_UPDATE");

                    GameObject tmpGO = GOfact.GetGOByEntity(entity);
                    if (tmpGO.HasComponent<Transform>())
                    {
                        Transform trf = tmpGO.GetComponent<Transform>();
                        pkt << trf.position.x;
                        pkt << trf.position.y;
                        pkt << trf.position.z;
                    }

                    //SendTo(serverSocket,pkt,)
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_RATE));
            }
        }
        
        
    }

  	//void NetworkSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	//{
    //       keep_running = false;
	//}
	//void NetworkSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	//{

	//}


    void NetworkSystem::SubscribeToAllNetworkEvents()
    {
        // Get the EventManager instance and subscribe our member functions.
        auto* eventManager = EventManager::GetInstance();

        // Subscribe to the connect event
        eventManager->Subscribe<NetworkClientConnectEvent, &NetworkSystem::OnConnectReq>(this);

        // Subcribe to bind event
        eventManager->Subscribe<NetworkBindPortEvent, &NetworkSystem::BindSocket>(this);
    }

    void NetworkSystem::Init()
    {

        //std::cout << "Server is listening on port " << portNumber << " ip " << serverIPAddr << " Player: " << clientNumber << " ...\n";


        // REGISTER ID HERE
        cmdIDs.Register("N_REQ_CONNECT");
        cmdIDs.Register("N_RSP_CONNECT");
        cmdIDs.Register("N_TIME_UPDATE");
        cmdIDs.Register("N_REQ_CREATE_GO");
        cmdIDs.Register("N_RSP_CREATE_GO");


        keep_running = true;

        

        /*if (player2)
        {
            data.client = true;
        }*/

        SubscribeToAllNetworkEvents();

        // split the threads
        //std::thread recv_thread(NetworkingThread::ReceiveThread, soc);
        //recv_thread.detach();
    }

    void NetworkSystem::BindSocket(const NetworkBindPortEvent& event)
    {
        if (event.port.empty())
        {
            return;
        }

        sockaddr_in cAddr{};

        // Initialize Winsock
        WSADATA wsaData{};
        int errorCode = WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData);
        if (errorCode != NO_ERROR)
        {
            std::cerr << "WSAStartup() failed." << std::endl;
            //return errorCode;
        }

        // Object hints indicates which protocols to use to fill in the info.
        addrinfo hints{};
        SecureZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;			// IPv4
        // For UDP use SOCK_DGRAM instead of SOCK_STREAM.
        hints.ai_socktype = SOCK_STREAM;	// Reliable delivery
        // Could be 0 for autodetect, but reliable delivery over IPv4 is always TCP.
        hints.ai_protocol = IPPROTO_UDP;	// UDP
        // Create a passive socket that is suitable for bind() and listen().
        hints.ai_flags = AI_PASSIVE;

        SOCKET soc{ socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) };
        if (soc == INVALID_SOCKET)
        {
            std::cerr << "UDP socket() failed." << std::endl;
            //return false;
        }


        char host[MAX_STR_LEN];
        gethostname(host, MAX_STR_LEN);

        addrinfo* info = nullptr;
        errorCode = getaddrinfo(host, event.port.c_str(), &hints, &info);
        if ((errorCode) || (info == nullptr))
        {
            std::cerr << "getaddrinfo() failed." << std::endl;
            WSACleanup();
            //return errorCode;
        }

        // Set up server address
        cAddr.sin_family = AF_INET;
        cAddr.sin_addr.s_addr = INADDR_ANY;
        cAddr.sin_port = htons(std::stoi(event.port));

        // Bind the socket
        if (bind(soc, reinterpret_cast<sockaddr*>(&cAddr), sizeof(cAddr)) != NO_ERROR)
        {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(soc);
            WSACleanup();
            //throw std::runtime_error("Bind failed");
            std::cerr << "Bind Failed" << std::endl;
        }

        u_long enable = 1;
        ioctlsocket(soc, FIONBIO, &enable);

        char serverIPAddr[MAX_STR_LEN];
        inet_ntop(AF_INET, &(cAddr.sin_addr), serverIPAddr, INET_ADDRSTRLEN);
        getnameinfo(info->ai_addr, static_cast <socklen_t> (info->ai_addrlen), serverIPAddr, sizeof(serverIPAddr), nullptr, 0, NI_NUMERICHOST);

        data.soc = soc;
        data.IP = serverIPAddr;
        data.port = event.port;
        
        std::cout << "Server is listening on port " << event.port << " ip " << serverIPAddr << " Player: " << " ...\n";

        // split the threads
        std::thread recv_thread(NetworkingThread::ReceiveThread, soc);
        recv_thread.detach();
    }

    void NetworkSystem::UpdateObjects()
    {
        //
    }


    void NetworkSystem::OnConnectReq(const NetworkClientConnectEvent& event)
    {
        

        //auto GO = FactoryInstance.GetGOByEntity(event.entity);
        //auto& networkComponent = GO.GetComponent<NetworkObj>();
        
        sockaddr_in player1Dest{};
        player1Dest.sin_family = AF_INET;		//ipv4
        player1Dest.sin_port = htons((u_short)std::stoi(event.port.c_str()));
        inet_pton(AF_INET, event.ip.c_str(), &player1Dest.sin_addr);

        Packet pkt{};
        pkt << cmdIDs.GetID("N_REQ_CONNECT");

        NetworkingThread::SendTo(data.soc, pkt, player1Dest);
        data.client = true;
        
        hasConnected = true;
    }
}
