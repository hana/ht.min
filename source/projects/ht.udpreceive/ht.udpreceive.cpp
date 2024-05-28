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

#include <set>

#include "oscpp/server.hpp"

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
    outlet<thread_check::scheduler, thread_action::fifo> info_out    { this, "(symbol) output the host address of the incoming message." };
    outlet<thread_check::scheduler, thread_action::fifo> time_out    { this, "(int) output the OSC timetag." };
    
    bool connected = false;
    static std::set<uint16_t> ports;  // port, count
    int listen_port = 7400;
    int sock;
    bool use_raw = false;
    
    sockaddr_in host_addr, client_info;

    void connect(const unsigned int new_port) {
#ifdef _WIN64
        WSAData wsadata;
        //if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
        //    error("Error initializing Windows socket.");
        //    return;
        //};
#endif

        if (0 < ports.count(new_port)) {
            error("Specified port is already opened.");
            return;
        } else {
            ports.emplace(new_port);
            listen_port = new_port;
        }
                    
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        
        host_addr.sin_family = AF_INET;
        host_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        host_addr.sin_port = htons(listen_port);
        
        bind(sock, reinterpret_cast<const struct sockaddr*>(&host_addr), sizeof(host_addr));
        
#ifdef __APPLE__
        constexpr auto val = 1;
        ioctl(sock, FIONBIO, &val);
#endif

        cout << "binding to port " << listen_port << endl;    // post to the max console
        connected = true;
        runner.delay(0);
    }
   
    void cleanup() {
        runner.stop();
#if defined (_WIN64)
        closesocket(sock);
        //WSACleanup();
#elif defined(__APPLE__)
        close(sock);
#endif
        ports.erase(listen_port);
    }
    
    // define an optional argument for setting the message
    argument<int> port_arg { this, "port", "Initial value for the greeting attribute.", true,
        MIN_ARGUMENT_FUNCTION {
            connect(static_cast<int>(arg));
        }
    };
    
    argument<bool> raw_arg {this, "raw", "True if handle raw UDP message", MIN_ARGUMENT_FUNCTION {
        use_raw = arg;
    }};
    
    // respond to the bang message to do something
    message<> port { this, "port", "Set the listen port.",
        MIN_FUNCTION {
            if(connected) cleanup();
            connect(static_cast<int>(args[0]));
            return {};
        }
    };

    message<> bang { this, "bang", "print the value",
        MIN_FUNCTION {
            cout << listen_port << endl;
            return{};
        }
    };
    
    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            return {};
        }
    };
    
    timer<> runner {
        this,
        MIN_FUNCTION {
            std::vector<char> buf(1024);
            constexpr socklen_t sin_size = sizeof(client_info);
            const auto received_size = recvfrom(sock, buf.data(), buf.size(), 0, reinterpret_cast<sockaddr*>(&client_info), const_cast<socklen_t*>(&sin_size));
                        
            if (received_size < 1) {
                // not received
            } else {
                if (use_raw) {
                    buf.resize(received_size);
                    info_out.send(inet_ntoa(client_info.sin_addr));
                    message_out.send(to_atoms(buf));
                } else {
                    OSCPP::Server::Packet packet(buf.data(), received_size);
                    handle_packet(packet);
                }
            }
            
            runner.delay(0);
            return {};
        }
    };
    
    void handle_packet(const OSCPP::Server::Packet& packet)  {
        uint64_t time = 0;
        if(packet.isMessage()) {
            OSCPP::Server::Message msg(packet);
                                                        
            OSCPP::Server::ArgStream args(msg.args());

            atoms atm;
            atm.emplace_back(msg.address());
            
            while(!args.atEnd()) {
                const auto tag = args.tag();
                switch(tag) {
                    case 'i':
                        atm.emplace_back(args.int32());
                        break;
                    case 'f':
                        atm.emplace_back(args.float32());
                        break;
                    case 's':
                        atm.emplace_back(args.string());
                        break;
                    case 'b':
                    {
                        const auto blob = args.blob();
                        const auto size = blob.size();
                        std::vector<uint8_t> data(size, 0);
                        std::memcpy(data.data(), blob.data(), size);
                        atm.emplace_back("OSCBlob");
                        atm.emplace_back(size);
                        const auto a = to_atoms(data);
                        atm.reserve(atm.size() + a.size());
                        std::copy(a.begin(),a.end(),std::back_inserter(atm));
                        break;
                    }
                    default:
                        break;
                }
            }
            time_out.send(time >> 32, time & 0xffffffff);
            info_out.send(inet_ntoa(client_info.sin_addr));
            message_out.send(atm);
        } else if (packet.isBundle()) {
            OSCPP::Server::Bundle bundle(packet);
            time = bundle.time();
            OSCPP::Server::PacketStream packets(bundle.packets());
            while (!packets.atEnd()) {
                handle_packet(packets.next());
            }
        } else {
            cerr << "Packet is neither a message nor a bundle" << endl;
        }
    }
    
    ~ht_udpreceive() {
        if(connected) cleanup();
    }
};

std::set<uint16_t> ht_udpreceive::ports = {};

MIN_EXTERNAL(ht_udpreceive);
