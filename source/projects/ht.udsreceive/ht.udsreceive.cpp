/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <vector>


using namespace c74::min;

class ht_udsreceive : public object<ht_udsreceive> {
public:
    MIN_DESCRIPTION	{"Unix Socket Sender"};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, ht.udpsend"};

    inlet<>  input	{ this, "(bang) flush the received message" };
    outlet<> output	{ this, "message received." };

    enum class SocketType { DGRAM = SOCK_DGRAM, STREAM = SOCK_STREAM};
    enum_map SocketType_range = {"DGRAM", "STREAM"};
    std::queue<std::vector<uint8_t>> message_queue;
    std::mutex message_queue_lock;
    
    struct Connection {
        int sock;
        sockaddr_un dest;
        std::atomic<bool> running;
        std::thread thread;
        std::unordered_set<ht_udsreceive*> outputs;
        std::mutex outputs_lock;
        
        void add_output(ht_udsreceive* obj) {
            guard lock {outputs_lock};
            outputs.emplace(obj);
        }
        
        void remove_output(ht_udsreceive* obj) {
            guard lock {outputs_lock};
            outputs.erase(obj);
        }
                    
        Connection(ht_udsreceive* handler, const std::string& adr) {
            add_output(handler);
            dest.sun_family = AF_UNIX;
            strcpy(dest.sun_path, adr.c_str());
            
            sock = socket(AF_UNIX, SOCK_DGRAM, 0);
                
            if(sock == -1) {
                handler->cout << "failed to create a socket : " << errno << " , " << strerror(errno) << endl;
                return;
            }
            
            if (bind(sock, reinterpret_cast<sockaddr*>(&dest), sizeof(sockaddr_un)) == -1) {
                handler->cout << "failed to bind : " << errno << " , " << strerror(errno) << endl;
                return;
            }
                    
            running.store(true, std::memory_order_relaxed);
            thread = std::thread([&]() {
//                constexpr auto Non_Blocking_Flag = 1; // non-blocking socket: https://www.geekpage.jp/programming/linux-network/nonblocking.php
//                ioctl(sock, FIONBIO, &Non_Blocking_Flag);
                            
                while(running.load(std::memory_order_relaxed)) {
                    std::vector<uint8_t> buffer(256);
                    const auto received_size = recv(sock, buffer.data(), buffer.size() - 1, 0);

                    if(0 < received_size) {
                        buffer.resize(received_size);
                        for(auto output : outputs) {
                            output->queue_message(buffer);
                        }
                    }
                }
            });
        }
        
        ~Connection() {
            running.store(false, std::memory_order_relaxed);
            close(sock);
            if(thread.joinable()) {
                thread.join();
            }
            remove(dest.sun_path);
        }
    };
    
    using ConnectionRef = std::shared_ptr<Connection>;
    using ConnectionMap = std::unordered_map<std::string, ConnectionRef>;
    static ConnectionMap connection_map;
    ConnectionRef connection;
    
    std::string get_address() {
        return static_cast<std::string>(address.get());
    }

    attribute<symbol> address {this, "address", "/tmp/thx.sock",
        description {
            "Message destination."
        }
    };
    
    void queue_message(const std::vector<uint8_t>& vec) {
        guard lock{message_queue_lock};
        message_queue.push(vec);
    }
    
    void flush() {
        guard lock{message_queue_lock};
        while(message_queue.size()) {
            output.send(to_atoms(message_queue.front()));
            message_queue.pop();
        }
    }
    
    argument<symbol> address_arg { this, "Destination address", "/tmp/thx.default.sock", MIN_ARGUMENT_FUNCTION {
        address = arg;
            
        if (connection_map.count(get_address()) == 0) {
            auto result = connection_map.emplace(get_address(), std::make_shared<Connection>(this, get_address())).first;
            connection = ConnectionRef(result->second);
        } else {
            connection = ConnectionRef(connection_map[get_address()]);
            connection->add_output(this);
        }
        update.delay(1.0);
    }};
            
    message<> bang {this, "bang", "flush the buffer", MIN_FUNCTION{
        flush();
        return {};
    }};
    
    timer<> update {this, MIN_FUNCTION {
        flush();
        update.delay(1.0);
        return{};
    }};
        
    ~ht_udsreceive() {
        if(connection) {
            if (connection.use_count() == 2) {
                connection_map.erase(get_address());
            } else {
                connection->remove_output(this);
            }
        }
    };
};

ht_udsreceive::ConnectionMap ht_udsreceive::connection_map;

MIN_EXTERNAL(ht_udsreceive);
