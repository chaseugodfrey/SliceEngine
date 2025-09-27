#include <pch.h>
#include "NetworkSystem.h"
#include <fstream>

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

        bool gameStart = false;
        float appTime{};
        float latestTimeStamp{};

        Player otherPlayer{};
        bool player2 = false;

        NetworkCommandID cmdIDs{};
    }

	void NetworkCommandID::Register(const std::string& cmdName)
	{
        uint8_t value = nextId++;
		//CmdObj obj{ func,value };
		cmdMap[cmdName] = value;
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

        std::cout << "Server is listening on port " << portNumber << " ip " << serverIPAddr << " Player: "<< clientNumber << " ...\n";


        // REGISTER ID HERE
        cmdIDs.Register("N_REQ_CONNECT");
        cmdIDs.Register("N_RSP_CONNECT");


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
            int bytes = SendTo(soc, pkt, player1Dest);
            if (bytes == SOCKET_ERROR || bytes == 0)
            {
                std::cerr << "UDP send fail: " << WSAGetLastError() << std::endl;
                //closesocket(pSocket);
            }
        }
    }

    int NetworkingThread::SendTo(const SOCKET& soc, const Packet& pkt, sockaddr_in pAddr)
    {
        return sendto(soc, reinterpret_cast<const char*>(pkt.msg.data()), (int)pkt.msg.size(), 0, reinterpret_cast<sockaddr*>(&pAddr), sizeof(pAddr));
    }

    int NetworkingThread::RecvFrom(const SOCKET& soc, char(&pkt)[MAX_STR_LEN], sockaddr_in& pAddr, int& size)
    {
        return recvfrom(soc, pkt, sizeof(pkt), 0, reinterpret_cast<sockaddr*> (&pAddr), &size);
    }

    void NetworkingThread::ReceiveThread(SOCKET otherPlayerSoc)
	{
        char buffer[MAX_STR_LEN];
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);

        while (keep_running)
        {
            //int bytes_received = recvfrom(otherPlayerSoc, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*> (&client_addr), &client_addr_len);
            int bytes_received = RecvFrom(otherPlayerSoc, buffer, client_addr, client_addr_len);

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

            std::string IpPort = client_ip;
            IpPort += ":";
            IpPort += std::to_string(ntohs(client_addr.sin_port));

            if (buffer[0] == cmdIDs.GetID("N_REQ_CONNECT"))
            {
                std::cout << "REQ received....\n";
                Packet pkt{};
                pkt << cmdIDs.GetID("N_RSP_CONNECT");

                //int bytes = { sendto(otherPlayerSoc,  reinterpret_cast<const char*>(pkt.msg.data()), (int)pkt.msg.size(), 0, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr)) };
                int bytes = SendTo(otherPlayerSoc, pkt, client_addr);
                if (bytes == SOCKET_ERROR || bytes == 0)
                {
                    std::cerr << "UDP send fail: " << WSAGetLastError() << std::endl;
                    //closesocket(pSocket);
                }
                
                // commented reference
                {
                    //cmdIDs.ProcessFunc("N_REQ_CONNECT", otherPlayerSoc, client_addr);

                    //if (clients.size() < TOTAL_PLAYERS)
                    //{
                        /*std::pair<std::string, int> newIndex{};

                        {
                            std::lock_guard<std::mutex> lock(_eventMutex);
                            newIndex = std::pair<std::string, int>(IpPort, (int)clients.size());
                            playersIndex.insert(newIndex);

                            std::pair<std::string, sockaddr_in> newClient(IpPort, client_addr);
                            clients.insert(newClient);
                        }*/

                        //std::string message{};
                        //message += cmdIDs.GetID("N_RSP_CONNECT");

                        /*int tmp = htonl(newIndex.second);
                        message.append((char*)(&tmp), (char*)(&tmp) + 4);*/

                        //sendto(otherPlayerSoc, message.c_str(), (int)message.length(), 0, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));

                        //if (clients.size() == TOTAL_PLAYERS)
                        //{
                            //gameStart = true;
                            //appTime = 0;

                            // send all clients
                          /*  for (auto& client : clients)
                            {
                                sendto(serverSock, message.c_str(), (int)message.length(), 0, reinterpret_cast<sockaddr*>(&client.second), sizeof(client.second));
                            }*/
                            //}
                        //}
                }
            }

            if (buffer[0] == cmdIDs.GetID("N_RSP_CONNECT"))
            {
                //cmdIDs.ProcessFunc("N_RSP_CONNECT", otherPlayerSoc, client_addr);
                std::cout << "connected " << std::endl;
                keep_running = false;
            }

            // Player fire
            if (buffer[0] == cmdIDs.GetID("N_REQ_FIRE"))
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


                float timestamp = ntohf(*(uint32_t*)(buffer + 1));

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
            if (buffer[0] == cmdIDs.GetID("N_STATE_UPDATE"))
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

                latestTimeStamp = ntohf(*(uint32_t*)(buffer + 1));

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
                pos.x = ntohf(*(uint32_t*)(buffer + 5));
                pos.y = ntohf(*(uint32_t*)(buffer + 9));

                Vector2 scale{ 0.f,0.f };
                scale.x = ntohf(*(uint32_t*)(buffer + 13));
                scale.y = ntohf(*(uint32_t*)(buffer + 17));

                float rot = ntohf(*(uint32_t*)(buffer + 21));

                Vector2 vel{ 0.f,0.f };
                vel.x = ntohf(*(uint32_t*)(buffer + 25));
                vel.y = ntohf(*(uint32_t*)(buffer + 29));

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

    }

	void NetworkSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
        keep_running = true;
        //NetworkCommandID cmds{};
	}
	void NetworkSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
        keep_running = false;
	}
	void NetworkSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}
}
