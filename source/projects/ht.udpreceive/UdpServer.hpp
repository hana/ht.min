//
//  UdpServer.hpp
//  ht.udpreceive
//
//  Created by 瀧本　花乃介 on 2024/04/24.
//

#pragma once

#ifndef udpserver_hpp
#define udpserver_hpp

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string_view>

#include <vector>

namespace ht {
class UdpServer {
public:
    using buffer_type = std::vector<uint8_t>;
    
    UdpServer(const uint16_t _port);
    buffer_type receive();
    ~UdpServer();
private:
    const uint16_t port;
    void foo();
    
    static constexpr auto Buffer_Size = 1460;

    int sock;
    sockaddr_in addr;
};
}

#endif /* udpserver_hpp */
