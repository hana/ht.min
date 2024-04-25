//
//  ThreadingUdpserver.cpp
//  ht.udpreceive
//
//  Created by 瀧本　花乃介 on 2024/04/24.
//

#include "ThreadingUdpServer.hpp"

//#include <iostream>
#include <chrono>

using namespace ht;

ThreadingUdpServer::ThreadingUdpServer(const uint16_t _port) : UdpServer(_port) {

}

void ThreadingUdpServer::start() {
    running = true;
    counter.store(0);
    th = std::thread([&]() {

        while(flag) {
            auto packet = receive();
            ++counter;
            push(std::move(packet));
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    });
}

bool ThreadingUdpServer::available() const {
    return !queue.empty();
}

UdpServer::buffer_type ThreadingUdpServer::pop() {
    const auto val = queue.front();
    queue.pop();
    return val;
}

void ThreadingUdpServer::push(buffer_type&& buf) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.emplace(buf);
}

std::size_t ThreadingUdpServer::size() const {
    return queue.size();
}

unsigned int ThreadingUdpServer::get_value() const {
    const auto val = counter.load();
    return val;
}

ThreadingUdpServer::~ThreadingUdpServer() {
    if (running) {
        flag = false;
        th.join();
    }
}
