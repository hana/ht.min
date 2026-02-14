/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <algorithm>
#include <set>

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
#include <ifaddrs.h>
#endif

using namespace c74::min;

#include "udp_sender.hpp"
#include "ht_min.h"

namespace ht {
class udpsend : public object<udpsend> {

public:
    MIN_DESCRIPTION    {"Raw UDP packet sender."};
    MIN_TAGS        {"Networking"};
    MIN_AUTHOR        {"Hananosuke Takimoto"};
    MIN_RELATED        {"udpsend, udpreceive, jit.net.send"};

#ifdef _WIN64
    using socklen_t = int;
#endif

    inlet<>  input    { this, "messages in" };
    outlet<> output    { this, "(anything) output from the incoming message." };
    
    std::string host = "127.0.0.1";
    uint16_t port = 50000;
    std::string source = "0.0.0.0";
    network::udp::sender sender;
    static constexpr auto Runner_Interval = 0;
    
    oscpkt::PacketWriter bundle;
    bool bundle_is_open = false;
    oscpkt::TimeTag timetag = oscpkt::TimeTag::immediate();
    
    argument<symbol> host_arg {this, "host", "Destination address", true,
        MIN_ARGUMENT_FUNCTION {
            host = symbol(arg).c_str();
            sender = network::udp::sender(host, port, source);
        }
    };
    
    argument<int> port_arg {this, "port", "Destination port", true,
        MIN_ARGUMENT_FUNCTION {
            port = static_cast<uint16_t>(int(arg));
            sender = network::udp::sender(host, port, source);
        }
    };
    
    argument<symbol> source_arg {this, "source", "Send source address", false,
        MIN_ARGUMENT_FUNCTION {
            source = symbol(arg).c_str();
            sender = network::udp::sender(host, port, source);
        }
    };
    
    attribute<symbol> source_attr { this, "source", "0.0.0.0",
        title {"source"},
        description {"Source address"},
        setter {
            MIN_FUNCTION {
                source = symbol(args[0]).c_str();
                sender = network::udp::sender(host, port, source);
                return args;
            }
        }
    };
    
    attribute<symbol> host_attr { this, "host", "127.0.0.1",
        title {"host"},
        description {"Destination host ID"},
        setter {
            MIN_FUNCTION {
                host = symbol(args[0]).c_str();
                sender = network::udp::sender(host, port, source);
                return args;
            }
        }
    };

    attribute<number> port_attr { this, "port", 50000,
        title {"port"},
        description {"Destination network port"},
        setter {
            MIN_FUNCTION {
                port = static_cast<int>(args[0]);
                sender = network::udp::sender(host, port, source);
                return args;
            }
        }
    };
    
    attribute<bool> raw {this, "raw", false,
        title {"raw"},
        description {"Handle input as raw binary. If true, input is supossed to be an list of uint8_t value (0-255)"},
    };
    
    attribute<bool> use_double {this, "double", false};

    message<threadsafe::yes> anything {this, "anything", "send data.",
        MIN_FUNCTION {
            if(raw) {
                if(ht::min::osc::atoms::is_convertible_to_blob(args)) {
                    const auto data = ht::min::osc::atoms::to_blob(args);

                    const auto result = sender.send(data);
                    if(result < 0) {
                        cerr << "Something went wrong while sending the data." << endl;
                    }
                }
            } else if (bundle_is_open) {
                const auto msg = ht::min::osc::atoms::to_message(args, timetag, use_double);
                bundle.addMessage(msg);
            } else {
                const auto packet = ht::min::osc::atoms::to_packet(args);
                sender.send(packet);                
            }
            
            return {};
        }
    };
    
    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "ht.udpsend ver 1.0.0" << endl;
            return {};
        }
    };
    
    message<> print {this, "print", "Output IP addresses of all interface",
        MIN_FUNCTION {
            struct ifaddrs *interfaces = nullptr;
            struct ifaddrs *temp_addr = nullptr;

            atoms result;
            result.emplace_back("0.0.0.0");
            
            // acquire NIC list
            if (getifaddrs(&interfaces) == 0) {
                temp_addr = interfaces;
                
                // loop linked list
                while (temp_addr != nullptr) {
                        if (temp_addr->ifa_addr != nullptr) {
                            int family = temp_addr->ifa_addr->sa_family;

                            if (family == AF_INET) {
                                char ip_buffer[INET6_ADDRSTRLEN];
                                void *addr_ptr = nullptr;
                                
                                addr_ptr = &((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr;

                                // convert binary IP to string
                                inet_ntop(family, addr_ptr, ip_buffer, sizeof(ip_buffer));
                                result.emplace_back(ip_buffer);
                            }
                        }
                        temp_addr = temp_addr->ifa_next;
                    }
                freeifaddrs(interfaces);
                
            } else {
                cerr << "Failed to acquire the NIC list" << endl;
            }
            std::reverse(result.begin(), result.end());
            output.send(result);
            return {};
        }
    };
    
    message<> bundle_msg {this, "bundle", "1 to open bundle, 0 to close bandle and send",
        MIN_FUNCTION {
            if(args[0] == true) {
                bundle.init();
                bundle.startBundle();
                bundle_is_open = true;
            } else {
                if(bundle_is_open) {
                    bundle.endBundle();
                    sender.send(bundle.packetData(), bundle.packetSize());
                    bundle_is_open = false;
                } else {
                    cerr << "Bundle is not open" << endl;
                }
            }
            return {};
        }
    };
};

}

MIN_EXTERNAL(ht::udpsend);
