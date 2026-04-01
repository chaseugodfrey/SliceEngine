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

        std::thread recv_thread, send_thread;

        std::queue<float> event_queue{};
        float latest_server_update{};

        bool hasConnected = false;
        float timer = UPDATE_RATE;

        bool gameStart = false;
        float appTime{};
        float latestTimeStamp{};

        Player otherPlayer{};
        bool player2 = false;

        NetworkCommandID cmdIDs{};

        // COMPONENTS --------------------------
        // transform
        uint8_t TRF_COMPONENTMASK = 0;
        // render
        uint8_t REN_COMPONENTMASK = 1;
    }

    namespace NetworkingThread
    {
        std::unordered_map<uint32_t, uint32_t> HtoCID{};
        std::unordered_map<uint32_t, uint32_t> CtoHID{};
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
            std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
			//std::cout << "No function registered for ID: " << cmdName << std::endl;
		}
		
        return 0;
	}

	/*void NetworkCommandID::ProcessFunc(const std::string& cmdName, SOCKET pSocket, sockaddr_in pAddr)
	{
		if (cmdMap.find(cmdName) != cmdMap.end())
		{
			cmdMap.at(cmdName).execFunc(pSocket,pAddr);
		}
		else
		{
			//std::cout << "No function registered for ID: " << cmdName << std::endl;
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
        int errorCode = WSAStartup(MAKEWORD(WINSOCKK_VERSION, WINSOCK_SUBVERSION), &wsaData);
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
        cAddr.sin_port = htons(static_cast<u_short>(std::stoi(portNumber)));

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

        //std::cout << "Server is listening on port " << portNumber << " ip " << serverIPAddr << " Player: " << clientNumber << " ...\n";


        // REGISTER ID HERE
        cmdIDs.Register("N_REQ_CONNECT");
        cmdIDs.Register("N_RSP_CONNECT");
        cmdIDs.Register("N_TIME_UPDATE");


        keep_running = true;

        // split the threads
        //std::thread recv_thread(ReceiveThread, soc);
        //recv_thread.detach();
         
        if (player2)
        {
            //std::cout << "sending req....\n";

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
            std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
            std::cerr << "UDP send fail: " << WSAGetLastError() << std::endl;
            //closesocket(pSocket);
        }
    }

    int NetworkingThread::RecvFrom(const SOCKET& soc, Packet& pkt, sockaddr_in& pAddr, int& size)
    {
        char buffer[MAX_STR_LEN];
        int bytes =  recvfrom(soc, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*> (&pAddr), &size);
        if(bytes > 0)
        {
            pkt.msg.clear();
            pkt.msg.insert(pkt.msg.end(), buffer, buffer + bytes);
        }

        return bytes;
    }

    void NetworkingThread::ReceiveThread(SOCKET soc)
	{
        //char buffer[MAX_STR_LEN];
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);
        auto& factory = SliceEngine::Core::GetInstance()->mFactory;
        auto& reg = Core::GetInstance()->GetRegistry();
        auto entityView = reg.view<SliceEntity>();

        while (keep_running)
        {
            client_addr_len = sizeof(client_addr);
            Packet recvPkt{};
            recvPkt.msg.reserve(MAX_STR_LEN);
            //int bytes_received = recvfrom(otherPlayerSoc, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*> (&client_addr), &client_addr_len);
            int bytes_received = RecvFrom(soc, recvPkt, client_addr, client_addr_len);

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
                std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
                //std::cout << "REQ received....\n";
                Packet pkt{};
                pkt << cmdIDs.GetID("N_RSP_CONNECT");
                pkt << static_cast<uint64_t>(entityView.size());

                for (auto entity : entityView)
                {
                    GameObject tmpGO = factory.GetGOByEntity(entity);
                    pkt << static_cast<uint32_t>(entity);
                }

                SendTo(soc, pkt, client_addr);
                hasConnected = true;
                Core::GetInstance()->GetNetwork()->data.otherPlayer = client_addr;
                
                //std::thread send_thread(SendThread, soc, false, client_addr);
                //send_thread.detach();

                send_thread = std::thread(SendThread,soc, false, client_addr);
            }

            if (inID == cmdIDs.GetID("N_RSP_CONNECT"))
            {
                std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
                //std::cout << "connected " << std::endl;
                uint64_t numOfGOs{};
                recvPkt >> numOfGOs;

                if (static_cast<uint64_t>(entityView.size()) == numOfGOs)
                {
                    for (auto entity : entityView)
                    {
                        GameObject tmpGO = factory.GetGOByEntity(entity);
                        uint32_t hostEntID{};
                        recvPkt >> hostEntID;

                        NetworkingThread::CtoHID[static_cast<uint32_t>(entity)] = hostEntID;
                        NetworkingThread::HtoCID[hostEntID] = static_cast<uint32_t>(entity);
                    }
                }


                hasConnected = true;
            }

            // Format for sending GO
            // ID - 1b, HtoCID - 4b, ComponentMask - 2b, transform(scale, rotate, pos) - 3x4b, - 3x4b, - 3x4b
            if (inID == cmdIDs.GetID("N_REQ_CREATE_GO"))
            {
                auto go = factory.CreateGO();
                go.AddComponent<SliceEngine::Renderer>();

                Packet pkt{};
                // just testing render, transform
                uint16_t componentMask{};
                if (go.HasComponent<Transform>())
                {
                    componentMask = static_cast<uint16_t>(componentMask | (1u << TRF_COMPONENTMASK));
                }
                if (go.HasComponent<Renderer>())
                {
                    componentMask = static_cast<uint16_t>(componentMask | (1u << REN_COMPONENTMASK));
                }

                pkt << cmdIDs.GetID("N_RSP_CREATE_GO");
                pkt << static_cast<uint32_t>(go.GetEntity());
                pkt << componentMask;
                pkt << go.GetComponent<Transform>().scale.x;
                pkt << go.GetComponent<Transform>().scale.y;
                pkt << go.GetComponent<Transform>().scale.z;
                pkt << go.GetComponent<Transform>().rotation.x;
                pkt << go.GetComponent<Transform>().rotation.y;
                pkt << go.GetComponent<Transform>().rotation.z;
                pkt << go.GetComponent<Transform>().position.x;
                pkt << go.GetComponent<Transform>().position.y;
                pkt << go.GetComponent<Transform>().position.z;


                NetworkingThread::SendTo(soc, pkt, client_addr);

            }

            if (inID == cmdIDs.GetID("N_RSP_CREATE_GO"))
            {
                auto go = factory.CreateGO();

                uint32_t hostEntID{};
                recvPkt >> hostEntID;
                
                NetworkingThread::CtoHID[static_cast<uint32_t>(go.GetEntity())] = hostEntID;
                NetworkingThread::HtoCID[hostEntID] = static_cast<uint32_t>(go.GetEntity());

                uint16_t cmpmask{};
                recvPkt >> cmpmask;

                // check for trf and reder
                if (static_cast<uint16_t>((cmpmask >> TRF_COMPONENTMASK) & 1u))
                {
                    float tmpUnpack{};

                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().scale.x = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().scale.y = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().scale.z = tmpUnpack;

                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().rotation.x = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().rotation.y = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().rotation.z = tmpUnpack;

                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().position.x = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().position.y = tmpUnpack;
                    recvPkt >> tmpUnpack;
                    go.GetComponent<Transform>().position.z = tmpUnpack;
                }

                if (static_cast<uint16_t>((cmpmask >> REN_COMPONENTMASK) & 1u))
                {
                    go.AddComponent<Renderer>();
                }

            }

            if (inID == cmdIDs.GetID("N_TIME_UPDATE"))
            {
                uint8_t client = 0;
                recvPkt >> client;
                uint32_t hID;
                recvPkt >> hID;
                if (!client)
                {
                    for (auto entity : entityView)
                    {
                        GameObject tmpGO = factory.GetGOByEntity(entity);
                        if (static_cast<uint32_t>(entity) == HtoCID[hID])
                        {
                            float tmpUnpack{};

                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().scale.x = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().scale.y = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().scale.z = tmpUnpack;

                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().rotation.x = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().rotation.y = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().rotation.z = tmpUnpack;

                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().position.x = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().position.y = tmpUnpack;
                            recvPkt >> tmpUnpack;
                            tmpGO.GetComponent<Transform>().position.z = tmpUnpack;
                        }
                    }
                }
            }
        }
	}

    void NetworkingThread::SendThread(SOCKET serverSocket,bool client, sockaddr_in _otherPlayer)
    {
        //auto dt = Core::GetInstance()->GetFramerateManager()->getDeltaTime();
        auto& reg = Core::GetInstance()->GetRegistry();
        auto& GOfact = Core::GetInstance()->mFactory;

        while (keep_running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_RATE));
            if (hasConnected)
            {
                /*if (timer <= 0.0f)
                {
                    timer = UPDATE_RATE;
                }

                timer -= dt;*/

                auto entityView = reg.view<SliceEntity>();
                //std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
                ////std::cout << " entities size: " << entityView.size() << std::endl;
                for (auto entity : entityView)
                {
                    ////std::cout << mEntityToGO[entity].GetName() << std::endl;
                    Packet pkt{};
                    pkt << cmdIDs.GetID("N_TIME_UPDATE");
                    pkt << static_cast<uint8_t>(client);

                    GameObject tmpGO = GOfact.GetGOByEntity(entity);

                    // just testing render, transform
                    uint16_t componentMask{};
                    if (tmpGO.HasComponent<Transform>())
                    {
                        componentMask = static_cast<uint16_t>(componentMask | (1u << TRF_COMPONENTMASK));
                    }
                    if (tmpGO.HasComponent<Renderer>())
                    {
                        componentMask = static_cast<uint16_t>(componentMask | (1u << REN_COMPONENTMASK));
                    }

                    if (client)
                    {
                        pkt << CtoHID[static_cast<uint32_t>(entity)];
                    }
                    else
                    {
                        pkt << static_cast<uint32_t>(entity);
                    }

                    if (tmpGO.HasComponent<Transform>())
                    {
                        pkt << tmpGO.GetComponent<Transform>().scale.x;
                        pkt << tmpGO.GetComponent<Transform>().scale.y;
                        pkt << tmpGO.GetComponent<Transform>().scale.z;
                        pkt << tmpGO.GetComponent<Transform>().rotation.x;
                        pkt << tmpGO.GetComponent<Transform>().rotation.y;
                        pkt << tmpGO.GetComponent<Transform>().rotation.z;
                        pkt << tmpGO.GetComponent<Transform>().position.x;
                        float tmpF = tmpGO.GetComponent<Transform>().position.y;
                        pkt << tmpF;
                        pkt << tmpGO.GetComponent<Transform>().position.z;
                    }

                    SendTo(serverSocket, pkt, _otherPlayer);
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::seconds(TIME_SYNC));
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

        // Subscribe to GO event
        eventManager->Subscribe<GONetworkEvent, &NetworkSystem::OnGONetworkEvent>(this);
    }

    void NetworkSystem::Init()
    {

        ////std::cout << "Server is listening on port " << portNumber << " ip " << serverIPAddr << " Player: " << clientNumber << " ...\n";


        // REGISTER ID HERE
        cmdIDs.Register("N_REQ_CONNECT");
        cmdIDs.Register("N_RSP_CONNECT");
        cmdIDs.Register("N_TIME_UPDATE");
        cmdIDs.Register("N_REQ_CREATE_GO");
        cmdIDs.Register("N_RSP_CREATE_GO");
        cmdIDs.Register("N_REQ_DESTROY_GO");
        cmdIDs.Register("N_RSP_DESTROY_GO");


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

    void NetworkSystem::Exit()
    {
        if (data.soc != INVALID_SOCKET) ::shutdown(data.soc, SD_BOTH);
        keep_running = false;

        if (recv_thread.joinable())
            recv_thread.join();
        if (send_thread.joinable())
            send_thread.join();
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
        int errorCode = WSAStartup(MAKEWORD(WINSOCKK_VERSION, WINSOCK_SUBVERSION), &wsaData);
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
        hints.ai_socktype = SOCK_DGRAM;	// Reliable delivery
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
        cAddr.sin_port = htons(static_cast<u_short>(std::stoi(event.port)));

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
        

        //std::cout << "Server is listening on port " << event.port << " ip " << serverIPAddr << " Player: " << " ...\n";

        // split the threads
        //std::thread recv_thread(NetworkingThread::ReceiveThread, soc);
        //recv_thread.detach();

        recv_thread = std::thread(NetworkingThread::ReceiveThread, soc);
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

        data.otherPlayer = player1Dest;

        Packet pkt{};
        pkt << cmdIDs.GetID("N_REQ_CONNECT");

        std::lock_guard<std::mutex> usersLock{ _stdoutMutex };
        //std::cout << "sending to " << event.ip << " " << event.port << std::endl;

        NetworkingThread::SendTo(data.soc, pkt, player1Dest);
        data.client = true;
        
        //hasConnected = true;

        // split the threads
        //std::thread send_thread(NetworkingThread::SendThread, data.soc,data.client, player1Dest);
        //send_thread.detach();
        send_thread = std::thread(NetworkingThread::SendThread, data.soc, data.client, player1Dest);
    }

    void NetworkSystem::OnGONetworkEvent(const GONetworkEvent& event)
    {
        if (hasConnected)
        {
            if(event.create)
            {
                Packet pkt{};
                if (data.client)
                {
                    auto& GOfact = Core::GetInstance()->mFactory;
                    GOfact.Destroy(event.entity);
                    pkt << cmdIDs.GetID("N_REQ_CREATE_GO");
                    NetworkingThread::SendTo(data.soc, pkt, data.otherPlayer);
                    return;
                }

                // Format for sending GO
                // ID - 1b, ComponentMask - 2b, transform(scale, rotate, pos) - 3x4b, - 3x4b, - 3x4b
                else
                {
                    auto& GOfact = Core::GetInstance()->mFactory;
                    GameObject tmpGO = GOfact.GetGOByEntity(event.entity);

                    // just testing render, transform
                    uint16_t componentMask{};
                    if (tmpGO.HasComponent<Transform>())
                    {
                        componentMask = static_cast<uint16_t>(componentMask | (1u << TRF_COMPONENTMASK));
                    }
                    if (tmpGO.HasComponent<Renderer>())
                    {
                        componentMask = static_cast<uint16_t>(componentMask | (1u << REN_COMPONENTMASK));
                    }

                    pkt << cmdIDs.GetID("N_RSP_CREATE_GO");
                    pkt << static_cast<uint32_t>(event.entity);
                    pkt << componentMask;
                    pkt << tmpGO.GetComponent<Transform>().scale.x;
                    pkt << tmpGO.GetComponent<Transform>().scale.y;
                    pkt << tmpGO.GetComponent<Transform>().scale.z;
                    pkt << tmpGO.GetComponent<Transform>().rotation.x;
                    pkt << tmpGO.GetComponent<Transform>().rotation.y;
                    pkt << tmpGO.GetComponent<Transform>().rotation.z;
                    pkt << tmpGO.GetComponent<Transform>().position.x;
                    pkt << tmpGO.GetComponent<Transform>().position.y;
                    pkt << tmpGO.GetComponent<Transform>().position.z;


                    NetworkingThread::SendTo(data.soc, pkt, data.otherPlayer);
                }
            }
            else
            {

            }
        }
    }
}
