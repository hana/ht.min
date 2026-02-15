/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

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

#include <optional>
#include <set>
#include <span>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>

#include "udpreceiver.hpp"

#include "ht_min.h"

using namespace c74::min;

class ht_udpreceive : public object<ht_udpreceive> {
public:
    MIN_DESCRIPTION	{"Raw UDP packet receiver."};
    MIN_TAGS		{"Networking"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, udpsend, jit.net.recv"};

#ifdef _WIN64
    using socklen_t = int;
#endif

    inlet<>  input	{ this, "messages in" };
    outlet<thread_check::scheduler, thread_action::fifo> message_out	{ this, "(anything) output the incoming message." };
    outlet<thread_check::scheduler, thread_action::fifo> time_out    { this, "(int) output the OSC timetag." };
    outlet<thread_check::scheduler, thread_action::fifo> info_out    { this, "(symbol) output the host address of the incoming message." };
    
    bool connected = false;
//    static std::set<uint16_t> ports;  // port, count
    int listen_port = 7400;
    double runner_interval = 1.0;
    int sock;
    bool use_raw = false;
    sockaddr_in host_addr, client_info;
    
    using udp_receiver = udp::receiver<ht_udpreceive>;
    static std::unordered_map<int, udp_receiver> receivers;
    udp_receiver* receiver = nullptr;
    
    struct message_t {
        std::string remote_address;
        std::optional<uint64_t> timetag;
        atoms data;
    };
    fifo<message_t> queue {1024};
    std::mutex mtx;
    
    void connect() {
        if(auto itr = receivers.find(listen_port); itr != receivers.end()) {
            receiver = &itr->second;
            receiver->add_listener(this);
        } else {
            cout << "binding to port " << listen_port << endl;
            receiver = &receivers.try_emplace(listen_port, "0.0.0.0", listen_port).first->second;
            receiver->add_listener(this);
        }
        
        runner.delay(0);
    }
   
    void cleanup() {
        receiver->remove_listener(this);
        if(receiver->empty()) {
            receivers.erase(listen_port);
        }
        receiver = nullptr;
    }
    
    // define an optional argument for setting the message
    argument<int> port_arg { this, "port", "Specifies the local port that the ht.udpreceive object will use to listen for incoming messages.", true,
        MIN_ARGUMENT_FUNCTION {
            if(receiver != nullptr) cleanup();
            listen_port = static_cast<int>(arg);
            connect();
        }
    };
    
    argument<bool> raw_arg {this, "raw", "True if handle raw UDP message. Default: false.",
        MIN_ARGUMENT_FUNCTION {
            use_raw = arg;
        }
    };
    
    // respond to the bang message to do something
    message<> port { this, "port", "Set the listen port.",
        MIN_FUNCTION {
            if(receiver != nullptr) cleanup();
            listen_port = static_cast<int>(args[0]);
            connect();
            return {};
        }
    };
    
    attribute<bool> fullthrottle { this, "fullthrottle", false,
        title {"Fullthrottle"},
        description {"Enabling this option reduces latency at the cost of increased CPU usage."},
        setter {
            MIN_FUNCTION {
                if(0 < static_cast<int>(args[0])) runner_interval = 0.0;
                else runner_interval = 1.0;
                return args;
            }
        }
    };
    
    attribute<bool> raw_attr {this, "raw", false,
        title{"Raw"},
        description {"Handle incoming message as raw udp message"},
        setter {
            MIN_FUNCTION {
                use_raw = static_cast<bool>(args[0]);
                return args;
            }
        }
    };
    
    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "ht.udpreceive ver 1.0.1, build " << __DATE__ << " " << __TIME__ << endl;
            return {};
        }
    };

    timer<> runner {
        this,
        MIN_FUNCTION {
            
            message_t data;
            while(queue.try_dequeue(data)) {
                info_out.send(data.remote_address);
                if(data.timetag.has_value()) {
                    time_out.send(data.timetag.value());
                }
                message_out.send(data.data);
            };

            runner.delay(runner_interval);
            return {};
        }
    };
    
    void parse(const char* adr, const std::span<uint8_t> data) {
        oscpkt::PacketReader pr(data.data(), data.size());
        
        if (!pr.isOk()) {
            cerr << "Could not parse the data" << endl;
            return;
        }
        
        oscpkt::Message* msg;
        
        while (pr.isOk() && (msg = pr.popMessage()) != nullptr) {
            auto atm = ht::min::osc::message::to_atoms(msg);                    
            queue.try_emplace(adr, msg->timeTag(), std::move(atm));
        }
    }
    
    void on_receive(const char* adr, const std::span<uint8_t> data) {
        if (use_raw) {
            queue.try_emplace(adr, std::nullopt, to_atoms(data));
        } else {
            parse(adr, data);
        }
    }
    
    ~ht_udpreceive() {
        runner.stop();
        if(receiver != nullptr)cleanup();
    }
};

//std::set<uint16_t> ht_udpreceive::ports = {};
std::unordered_map<int, udp::receiver<ht_udpreceive>> ht_udpreceive::receivers = {};

MIN_EXTERNAL(ht_udpreceive);
