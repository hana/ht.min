//
//  UdpServer.cpp
//  ht.udpreceive
//
//  Created by 瀧本　花乃介 on 2024/04/24.
//

#include "UdpServer.hpp"

using namespace ht;

UdpServer::UdpServer(const uint16_t _port) : port(_port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(port);
    
    bind(sock, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)); 
    
    constexpr auto val = 1;
    ioctl(sock, FIONBIO, &val);
}

UdpServer::buffer_type UdpServer::receive() {
    buffer_type buf(Buffer_Size, 0);
    sockaddr_in fromAddr;
    socklen_t sinSize = sizeof(fromAddr);
    const auto n = recvfrom(sock, buf.data(), buf.size(), 0, (sockaddr *)&fromAddr, &sinSize);
    if ( n < 1) {
        buf.resize(0);
        buf.shrink_to_fit();
    } else {
        
    }
    return buf;
}

UdpServer::~UdpServer() {
    close(sock);
}
