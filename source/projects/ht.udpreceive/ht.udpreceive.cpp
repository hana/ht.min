/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <set>

//#include "ThreadingUdpServer.hpp"
#include "LiteOSCParser/src/LiteOSCParser.h"

using namespace c74::min;

class ht_udpreceive : public object<ht_udpreceive> {
public:
    MIN_DESCRIPTION	{"Raw UDP packet receiver."};
    MIN_TAGS		{"Networking"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, udpsend, jit.net.recv"};

    inlet<>  input	{ this, "messages in" };
    outlet<thread_check::scheduler, thread_action::fifo> message_out	{ this, "(anything) output the incoming message." };
    outlet<thread_check::scheduler, thread_action::fifo> info_out    { this, "(symbol) output the host address of the incoming message." };
    
    bool initialized = false;
    static std::set<uint16_t> ports;  // port, count
    int listen_port = 7400;
    int sock;
//    bool connected = false;
    bool use_raw = false;
    sockaddr_in addr;

    void connect(const unsigned int new_port) {
        if (0 < ports.count(new_port)) {
            error("Specified port is already opened.");
            return;
        } else {
            ports.emplace(new_port);
            listen_port = new_port;
        }
            
        close(sock);
        
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        addr.sin_port = htons(listen_port);
        
        bind(sock, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
        
        constexpr auto val = 1;
        ioctl(sock, FIONBIO, &val);
        
        cout << "binding to port " << listen_port << endl;    // post to the max console
        runner.delay(0);
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
            runner.stop();
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
    
    message<> size { this, "size", "print the value",
        MIN_FUNCTION {
            return{};
        }
    };

    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            return {};
        }
    };
    
    timer<> runner {this,
        MIN_FUNCTION {
            std::vector<uint8_t> buf(1024);
            sockaddr_in client_info;
            constexpr socklen_t sin_size = sizeof(client_info);
            const auto received_size = recvfrom(sock, buf.data(), buf.size(), 0, reinterpret_cast<sockaddr*>(&client_info), const_cast<socklen_t*>(&sin_size));
                        
            if (received_size < 1) {
                // not received
            } else {
                
                info_out.send(inet_ntoa(client_info.sin_addr));
                
                if (use_raw) {
                    buf.resize(received_size);
                    message_out.send(to_atoms(buf));
                } else {
                    qindesign::osc::LiteOSCParser osc;
                    const bool success = osc.parse(buf.data(), received_size);
                    
                    if(!success) {
                        if (osc.isMemoryError()) {
                            cout << "Memory Error" << endl;
                        } else {
                            cout << "unknown error" << endl;
                        }
                        runner.delay(0);
                        return {};
                    }
                    
                    atoms res;
                    res.emplace_back(osc.getAddress());
                    
                    const auto num_args = osc.getArgCount();
                    cout << "arg count: " << num_args << endl;
                    
                    for(int i = 0; i < num_args; i++ ) {
                        if(osc.isInt(i)) res.emplace_back(osc.getInt(i));
                        else if (osc.isFloat(i)) res.emplace_back(osc.getFloat(i));
                        else if (osc.isDouble(i)) res.emplace_back(osc.getDouble(i));
                        else if (osc.isChar(i)) res.emplace_back(osc.getChar(i));
                        else if (osc.isString(i)) res.emplace_back(osc.getString(i));
                        else if (osc.isLong(i)) res.emplace_back(osc.getLong(i));
                        else if (osc.isBlob(i))  {
                            const auto blob_length = osc.getBlobLength(i);
                            cout << "blob length: " << blob_length << endl;
                            std::vector<uint8_t> b(blob_length);
                            std::memcpy(b.data(), osc.getBlob(i), blob_length);
                            res.emplace_back("blob");
                            res.emplace_back(blob_length);
                            res.emplace_back(b);
                        }
                    }
                                 
                    message_out.send(res);
                }
            }
            
            runner.delay(0);
            return {};
        }
    };
    
    ~ht_udpreceive() {
        runner.stop();
        close(sock);
    }
};

std::set<uint16_t> ht_udpreceive::ports = {};

MIN_EXTERNAL(ht_udpreceive);
