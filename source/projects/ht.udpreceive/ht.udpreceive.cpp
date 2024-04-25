/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <set>

#include "ThreadingUdpServer.hpp"

using namespace c74::min;

class ht_udpreceive : public object<ht_udpreceive> {
public:
    MIN_DESCRIPTION	{"Raw UDP packet receiver."};
    MIN_TAGS		{"Networking"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, udpsend, jit.net.recv"};

    inlet<>  input	{ this, "messages in" };
    outlet<> message_out	{ this, "(anything) output the incoming message." };
    outlet<> info_out    { this, "(symbol) output the host address of the incoming message." };
    
    bool initialized = false;
    static std::set<uint16_t> ports;  // port, count
    
    ht::ThreadingUdpServer server{55555};
    
    
    ht_udpreceive(const atoms& args = {}) {
        if (args.empty()) {
            error("port required.");
        };        
        
//        server = ht::ThreadingUdpServer(55555);
    }

    // define an optional argument for setting the message
    argument<number> port_arg { this, "Port", "Initial value for the greeting attribute.",
        MIN_ARGUMENT_FUNCTION {
            listen_port = arg;
            cout << "port updated." << endl;
        }
    };


    // the actual attribute for the message
    attribute<int> listen_port { this, "port", 50000,
        description {
            "Listen port."
        },
        setter { 
            MIN_FUNCTION {
                const auto value = args[0];
                cout << "attribute: " << value << endl;
                return {value};
            }
        }
    };


    // respond to the bang message to do something
    message<> port { this, "port", "Set the listen port.",
        MIN_FUNCTION {
            runner.stop();
            const int new_port = args[0];    // fetch the symbol itself from the attribute named greeting
                        
            if (ports.find(new_port) == ports.end())  {  // new port
                cout << "new open." << endl;
                ports.emplace(new_port);                
                server.start();
            } else {
                cerr << "already opened" << endl;
            }
            
            cout << "binding to port "<< listen_port << endl;    // post to the max console
//            runner.delay(0);
            return {};
        }
    };

    message<> bang { this, "bang", "print the value",
        MIN_FUNCTION {
            cout << "count: " << server.get_value() << endl;
            if (server.available()) {
                const auto mes = server.pop();
                cout <<"mes: " << std::string(reinterpret_cast<const char*>(mes.data())) << endl;
                message_out.send(to_atoms(mes));
            }
            return{};
        }
    };
    
    message<> size { this, "size", "print the value",
        MIN_FUNCTION {
            cout << server.size() << endl;
            return{};
        }
    };

    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "HI" << endl;
            return {};
        }
    };
    
    timer<> runner {this,
        MIN_FUNCTION {
            cout << "port " << listen_port << endl;
            runner.delay(1000);
            return {};
        }
    };
};

std::set<uint16_t> ht_udpreceive::ports = {};

MIN_EXTERNAL(ht_udpreceive);
