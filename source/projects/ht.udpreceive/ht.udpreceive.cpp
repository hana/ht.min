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

#include "oscpp/server.hpp"
#include "udpreceiver.hpp"

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
    
    using udp_receiver = udpreceiver<ht_udpreceive>;
    static std::unordered_map<int, udp_receiver> receivers;
    udp_receiver* receiver = nullptr;
    
    struct message_t {
        std::string remote_address;
        std::optional<uint64_t> timetag;
        atoms data;
    };
    std::queue<message_t> queue;
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
    }
    
    // define an optional argument for setting the message
    argument<int> port_arg { this, "port", "Specifies the local port that the ht.udpreceive object will use to listen for incoming messages.", true,
        MIN_ARGUMENT_FUNCTION {
            if(receiver != nullptr) cleanup();
            listen_port = static_cast<int>(arg);
            connect();
        }
    };
    
    argument<bool> raw_arg {this, "raw", "True if handle raw UDP message. Default: false.", MIN_ARGUMENT_FUNCTION {
        use_raw = arg;
    }};
    
    // respond to the bang message to do something
    message<> port { this, "port", "Set the listen port.",
        MIN_FUNCTION {
            if(receiver != nullptr) cleanup();
            listen_port = static_cast<int>(args[0]);
            connect();
            return {};
        }
    };
    
    message<> fullthrottle { this, "fullthrottle", "full throttle toggle",
        MIN_FUNCTION {
            if(0 < static_cast<int>(args[0])) runner_interval = 0.0;
            else runner_interval = 1.0;
            return{};
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
            if(queue.size()) {
                const auto& data = queue.front();
                info_out.send(data.remote_address);
                if(data.timetag.has_value()) {
                    time_out.send(data.timetag.value());
                }
                message_out.send(data.data);
                std::lock_guard<std::mutex> lock{mtx};
                queue.pop();
            }
            if(queue.size()) {
                runner.delay(0);
            } else {
                runner.delay(runner_interval);
            }

            return {};
        }
    };
    
    atoms parse_packet_to_message(const OSCPP::Server::Packet& packet) {
        OSCPP::Server::Message msg(packet);
                                                    
        OSCPP::Server::ArgStream args(msg.args());

        atoms atms;
        atms.emplace_back(msg.address());
        
        while(!args.atEnd()) {
            const auto tag = args.tag();
            switch(tag) {
                case 'i':
                    atms.emplace_back(args.int32());
                    break;
                case 'f':
                    atms.emplace_back(args.float32());
                    break;
                case 's':
                    atms.emplace_back(args.string());
                    break;
                case 'b':
                {
                    const auto blob = args.blob();
                    const auto size = blob.size();
                    std::vector<uint8_t> data(size, 0);
                    std::memcpy(data.data(), blob.data(), size);
                    atms.emplace_back("OSCBlob");
                    atms.emplace_back(size);
                    const auto a = to_atoms(data);
                    atms.reserve(atms.size() + a.size());
                    std::copy(a.begin(),a.end(),std::back_inserter(atms));
                    break;
                }
                default:
                    break;
            }
        }
        return atms;
    }
    
    void on_receive(const char* adr, const std::span<uint8_t> data) {
        if (use_raw) {
            std::lock_guard<std::mutex> lock{mtx};
            queue.emplace(adr, std::nullopt, to_atoms(data));
        } else {
            OSCPP::Server::Packet packet(data.data(), data.size());
            std::optional<uint64_t> time;
            if(packet.isMessage()) {
                std::lock_guard<std::mutex> lock{mtx};
                queue.emplace(adr, time, parse_packet_to_message(packet));
            } else if (packet.isBundle()) {
                OSCPP::Server::Bundle bundle(packet);
                time = bundle.time();
                OSCPP::Server::PacketStream packets(bundle.packets());
                while (!packets.atEnd()) {
                    std::lock_guard<std::mutex> lock{mtx};
                    queue.emplace(adr, time, parse_packet_to_message(packets.next()));
                }
            } else {
                cerr << "Packet is neither a message nor a bundle" << endl;
            }
        }
    }
    
    ~ht_udpreceive() {
        runner.stop();
        if(receiver != nullptr)cleanup();
    }
};

//std::set<uint16_t> ht_udpreceive::ports = {};
std::unordered_map<int, udpreceiver<ht_udpreceive>> ht_udpreceive::receivers = {};

MIN_EXTERNAL(ht_udpreceive);
