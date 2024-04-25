//
//  ThreadingUdpServer.hpp
//  ht.udpreceive
//
//  Created by 瀧本　花乃介 on 2024/04/24.
//

#pragma once

#ifndef ThreadingUdpServer_hpp
#define ThreadingUdpServer_hpp

#include "UdpServer.hpp"

#include <atomic>
#include <stdio.h>
#include <thread>
#include <queue>

namespace ht {
class ThreadingUdpServer : public UdpServer {
public:
    ThreadingUdpServer(const uint16_t port);
    ~ThreadingUdpServer();
    void start();
    unsigned int get_value() const;
    std::size_t size() const;
    bool available() const;
    UdpServer::buffer_type pop();
    
    
private:
    std::thread th;
    std::mutex mtx;
    std::queue<buffer_type> queue;
    void push(buffer_type&& buf);
    
    std::atomic_uint counter = 0;
    
    bool running = false;
    bool flag = true;
};
}


#endif /* ThreadingUdpServer_hpp */
