//
// Created by Manish on 10/17/19.
//

#ifndef FMERGERCLIENT_FMERGERCLIENT_H
#define FMERGERCLIENT_FMERGERCLIENT_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <netinet/tcp.h>
//#include "include/fileContent.pb.h"
#include <sstream>

struct Packet
{
    std::size_t lineNumber;
    std::string msg;

    Packet()
    {}

    Packet( const Packet& obj) : lineNumber(obj.lineNumber), msg(obj.msg)
    {  }

    Packet(std::size_t line, std::string& pmsg) : lineNumber(line), msg(pmsg)
    {}

    std::size_t size()
    {
        std::size_t len{};
        len += sizeof(lineNumber);
        len += sizeof(std::size_t);
        len += msg.size();

        return len;
    }

    void serialize(char **ptr)
    {
        std::memcpy(&ptr, &lineNumber, sizeof(lineNumber));
        ptr += sizeof(std::size_t);

        std::size_t size = msg.length();
        std::memcpy(&ptr, &size, sizeof(std::size_t));
        ptr += sizeof(std::size_t);

        std::strncpy(*ptr, msg.c_str(), size);
        ptr += size;
    }
};

class Client
{
    std::string remotePeer;
    struct sockaddr_in serverAddr;
    int sockFd;
    std::string file;
public:
    Client(const std::string& resource, const std::string& f) : remotePeer(resource), sockFd(0), file(f)
    {
        std::memset(&serverAddr, 0x0, sizeof(sockaddr_in));
    }

    Client() = default;

    bool connectPeer()
    {
        int setOpt = 1;
        sockFd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockFd < 0)
        {
            std::cout << "socket creation failed"<< std::endl;
            return false;
        }

        size_t colonPos = remotePeer.find(':');
        std::string addr = remotePeer.substr(0, colonPos);
        std::string portS = remotePeer.substr(colonPos+1);
        int port = std::stoi(portS);

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(addr.c_str());
        serverAddr.sin_port = htons(port);

        if ((setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, (void *)& setOpt, sizeof(setOpt))) < 0)
        {
            std::cout << "Tcp no delay set failed..." << std::endl;
            return false;
        }

        setOpt = 1;
        if ((setsockopt( sockFd, IPPROTO_TCP, TCP_QUICKACK, (void *)& setOpt, sizeof(setOpt))) < 0)
        {
            std::cout << "TCP quick acknowledgement set failed" << std::endl;
            return false;
        }

        if ((connect(sockFd, (sockaddr *) &serverAddr, sizeof(sockaddr_in))) < 0)
        {
            std::cout << "Connect request failed with " << errno << "to " << remotePeer;
            return false;
        }

	std::cout << "Connected " << remotePeer << std::endl;
        return true;
    }

    void send()
    {
        std::ifstream inFile;
	    std::uint32_t lNumber;
        std::string msg;

        inFile.open(file, std::ios::in);

        while (inFile >> lNumber >> msg)
        {
            char raw_msg[2048];
            char *ptr = &raw_msg[0];
            std::memcpy(ptr, &lNumber, 4);
            ptr += 4;


            std::uint32_t lineSize = msg.size();
            std::memcpy(ptr, &lineSize, 4);
            ptr += 4;

            std::strcpy(ptr, msg.c_str());

            std::uint32_t sendSize = 8 + msg.size();
            write(sockFd, raw_msg, sendSize);
        }

	    sleep(2);
        inFile.close();
    }
};

#endif //FMERGERCLIENT_FMERGERCLIENT_H
