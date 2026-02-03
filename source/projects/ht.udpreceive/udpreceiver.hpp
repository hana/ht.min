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
    
#ifdef _WIN64
    using socklen_t = int;
#endif
    
    std::atomic<bool> running;
    std::set<T*> listeners;
    
    const int listen_port;
    int sock;
    std::thread t;
public:
    udpreceiver(const std::string_view host, const int port) : listen_port(port) {
        t = std::thread([&]() {
#ifdef _WIN64
            WSAData wsadata;
#endif
            sockaddr_in host_addr;
            sock = socket(AF_INET, SOCK_DGRAM, 0);
            host_addr.sin_family = AF_INET;
            host_addr.sin_addr.s_addr = inet_addr(host.data());
            host_addr.sin_port = htons(listen_port);
                        
            bind(sock, reinterpret_cast<const struct sockaddr*>(&host_addr), sizeof(host_addr));
            
            static std::vector<uint8_t> buf(1500);
            static sockaddr_in client_info;
            constexpr socklen_t sin_size = sizeof(client_info);

#ifdef __APPLE__
//            constexpr auto val = 1;
//            ioctl(sock, FIONBIO, &val);
#endif
            
            running = true;
            
            while(running.load()) {
                const auto received_size = recvfrom(sock, buf.data(), buf.size(), 0, reinterpret_cast<sockaddr*>(&client_info), const_cast<socklen_t*>(&sin_size));
                if(0 < received_size) {
                    for(const auto& instance : listeners) {
                        instance->on_receive(inet_ntoa(client_info.sin_addr), std::span{buf.begin(), static_cast<std::size_t>(received_size)});
                    }
                }
            }

#if defined (_WIN64)
            closesocket(sock);
#elif defined(__APPLE__)
            close(sock);
#endif
        });
        
    };
        
    auto get_sock() const {
        return sock;
    }

    void add_listener(T* listener) {
        listeners.emplace(listener);
    };
    
    void remove_listener(T* listener) {
        listeners.erase(listener);
    }
    
    auto size() const {
        return listeners.size();
    }
    
    auto empty() const {
        return listeners.empty();
    }

    void terminate() {
        running.store(false);
        
        // send exit message to close the socket
        sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(listen_port);
        inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);
        int temp_sock = socket(AF_INET, SOCK_DGRAM, 0);
        constexpr auto mes = "EXIT";
        sendto(temp_sock, mes, sizeof(mes), 0, (sockaddr*)&dest_addr, sizeof(dest_addr));
        close(temp_sock);
    }
    
    ~udpreceiver() {
        terminate();
        t.join();
    }
};

#endif /* udpreceiver_hpp */
