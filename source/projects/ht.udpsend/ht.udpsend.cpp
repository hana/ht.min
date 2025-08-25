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

#include "udp_sender.hpp"

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
    outlet<thread_check::scheduler, thread_action::fifo> message_out    { this, "(anything) output the incoming message." };
    outlet<thread_check::scheduler, thread_action::fifo> info_out    { this, "(symbol) output the host address of the incoming message." };
    outlet<thread_check::scheduler, thread_action::fifo> time_out    { this, "(int) output the OSC timetag." };
    
    std::string host;
    uint16_t port;
    std::string source = "0.0.0.0";
    network::udp::sender sender;
    static constexpr auto Runner_Interval = 0;
    
    argument<symbol> host_arg {this, "host", "Destination address", true,
        MIN_ARGUMENT_FUNCTION {
            host = symbol(arg).c_str();
        }
    };
    
    argument<int> port_arg {this, "port", "Destination network port", true,
        MIN_ARGUMENT_FUNCTION {
            port = static_cast<uint16_t>(int(arg));
            sender = network::udp::sender(host, port, "0.0.0.0");
        }
    };
    
    argument<symbol> source_arg {this, "source", "Send source address", false,
        MIN_ARGUMENT_FUNCTION {
            source = symbol(arg).c_str();
            sender = network::udp::sender(host, port, source);
        }
    };
    
    message<threadsafe::yes> list {this, "list", "send data.",
        MIN_FUNCTION {
            std::vector<uint8_t> data;
            for(const auto& arg : args) {
                data.emplace_back(static_cast<uint8_t>(int(arg)));
            }
            const auto result = sender.send(data);
            if(result < 0) {
                cerr << "Something went wrong while sending the data." << endl;
            }
            return {};
        }
    };
    
    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "ht.udpsend ver 0.0.1" << endl;
            return {};
        }
    };
};

}

MIN_EXTERNAL(ht::udpsend);
