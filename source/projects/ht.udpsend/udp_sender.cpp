//
//  udp_sender.cpp
//  ht.udpsend
//
//  Created by 瀧本　花乃介 on 2025/08/25.
//

#include "udp_sender.hpp"

#include <iostream>
#include <netdb.h>

#include <iostream>

using namespace ht::network;

host_t::host_t(const std::string& hostname) {
    set(hostname);
}

//auto& host_t::operator=(const std::string& hostname) {
//    set(hostname);
//    return *this;
//}

host_t& host_t::operator=(const std::string& hostname) {
    this->set(hostname);
    return *this;
}

void host_t::set(const std::string& hostname) {
    struct addrinfo hints, *res;
    struct in_addr addr;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    
    if ((err = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
        name = hostname;
    } else {
        addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
        name = inet_ntoa(addr);
        freeaddrinfo(res);
    }
    
    struct in_addr inaddr;
    if (inet_pton(AF_INET, hostname.c_str(), &inaddr) == 1) {
        address = ntohl(inaddr.s_addr); // network byte order
    }
}

udp::sender::sender() {}

udp::sender::sender(const std::string& hostname, const int port, const std::string& source) : destination(hostname, port, source) {
    if(connections.contains(destination)) {
        connection = connections[destination];
    } else {
        connections[destination] = std::make_shared<connection_t>();
        connection = connections[destination];
        
        auto& sock = connection->sock;
        auto& addr = connection->addr;
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(destination.host.c_str());
        addr.sin_port = htons(port);
        
        if (static_cast<int>(destination.source) != 0) {
            sockaddr_in local{};
            local.sin_family = AF_INET;
            local.sin_port = 0; // 自動割当
            inet_pton(AF_INET, destination.source.c_str(), &local.sin_addr);
            bind(sock, (sockaddr*)&local, sizeof(local));
        }
    }
}

ssize_t udp::sender::send(const std::vector<uint8_t>& vec) const {
        auto& sock = connection->sock;
        auto& addr = connection->addr;
        return sendto(sock, vec.data(), vec.size(), 0, (struct sockaddr *)&addr, sizeof(addr));
}

udp::sender::~sender() {
    if(connections[destination].use_count() == 1) {
        close(connection->sock);
    }    
}

std::map<destination_t, std::shared_ptr<connection_t>> udp::sender::connections;
