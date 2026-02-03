//
//  udp_sender.hpp
//  ht.udpsend
//
//  Created by 瀧本　花乃介 on 2025/08/25.
//
#pragma once

#ifndef udp_sender_hpp
#define udp_sender_hpp

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <string.h>

//#include <compare>
#include <map>
#include <memory>
#include <string>
#include <vector>


static_assert(__cplusplus >= 202002L, "number of array elements must greater than 0");

namespace ht::network {
struct host_t {
    host_t(const std::string& host_name = "127.0.0.1");
    uint32_t address;
    std::string name;
//    auto& operator=(const host_t& other);
//    auto& operator=(const std::string& hostname);
    
    host_t& operator=(const host_t& other) = default;
    host_t& operator=(const std::string& name);
    auto operator<=> (const host_t&) const = default;
    void set(const std::string& hostname);
    constexpr auto c_str() const {
        return name.c_str();
    }
    
    operator unsigned int() const {
        return address;
    }
};

struct destination_t {
    host_t host;
    uint16_t port;
    host_t source;
    auto operator<=>(const destination_t&) const = default;
};

struct connection_t {
    int sock;
    sockaddr_in addr;
};

namespace udp {
class sender {
public:
    sender();
    sender(const std::string& hostname, const int port, const std::string& source = "0.0.0.0");
    ssize_t send(const std::vector<uint8_t>& data) const;
//    int send(char* data, const std::size_t size) const;
    ~sender();
    
    destination_t destination;
    std::shared_ptr<connection_t> connection = nullptr;

private:
    static std::map<destination_t, std::shared_ptr<connection_t>> connections;
};
}

}


#endif /* udp_sender_hpp */
