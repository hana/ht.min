//
//  udpreceiver.hpp
//  ht.udpreceive
//
//  Created by 瀧本　花乃介 on 2026/02/02.
//

#pragma once

#ifndef udpreceiver_hpp
#define udpreceiver_hpp

#include "c74_min.h"

#if defined(_WIN64)

#include <WinSock2.h>
#pragma comment(lib, "WS2_32")

#elif defined(__APPLE__)

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <atomic>
#include <thread>
#include <unordered_set>
#include <vector>
#include <span>
#include <chrono>

template<typename T>
class udpreceiver {

private:
    std::atomic<bool> running;
    std::vector<T*> listeners;
    
    const int listen_port;
    sockaddr_in host_addr, client_info;
    std::thread t;
public:
    udpreceiver(const char* host, const int port) : listen_port(port) {
        running = true;
        
        t = std::thread([&]() {
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            host_addr.sin_family = AF_INET;
            host_addr.sin_addr.s_addr = inet_addr(host);
            host_addr.sin_port = htons(listen_port);
            bind(sock, reinterpret_cast<const struct sockaddr*>(&host_addr), sizeof(host_addr));
            
            static std::vector<uint8_t> buf(1500);
            constexpr socklen_t sin_size = sizeof(client_info);
            
            while(running) {
                const auto received_size = recvfrom(sock, buf.data(), buf.size(), MSG_DONTWAIT, reinterpret_cast<sockaddr*>(&client_info), const_cast<socklen_t*>(&sin_size));
                
                if(0 < received_size) {
                    for(const auto& instance : listeners) {
                        instance->on_receive(client_info, std::span{buf.begin(), received_size});
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            close(sock);
        });
        
    };
    
//    udpreceiver(const int port) : listen_port(port) {
//        udpreceiver("0.0.0.0", port);
//    };

//    void add_listener(const T& listener) {
//        listeners.emplace_back(&listener);
//    };

    void add_listener(T* listener) {
        listeners.emplace_back(listener);
    };

        
    void stop() {
        running = false;
        t.join();
    }
    
    
    
};

#endif /* udpreceiver_hpp */
