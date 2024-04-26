/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace c74::min;


class ht_min_udssend : public object<ht_min_udssend> {
public:
    MIN_DESCRIPTION	{"Unix Socket Sender"};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpsend"};

    inlet<>  input	{ this, "(bang) post greeting to the max console" };
    outlet<> output	{ this, "(anything) output the message which is posted to the max console" };

    enum class SocketType { DGRAM = SOCK_DGRAM, STREAM = SOCK_STREAM};
    enum_map SocketType_range = {"DGRAM", "STREAM"};
    int sock;
    sockaddr_un adr;

    void create_socket() {
//        sock = socket(AF_UNIX, static_cast<int>(socket_type.get()), 0);
        sock = socket(AF_UNIX, SOCK_DGRAM, 0);
//        if (sock == -1) printf("failed to create a socket(errno:%d, error_str:%s)\n", errno, strerror(errno));
    }
        
    
    argument<symbol> address_arg { this, "send address", "/tmp/??", MIN_ARGUMENT_FUNCTION{
        address = arg;
        adr.sun_family = AF_UNIX;
        strcpy(adr.sun_path, address.get().c_str());
        create_socket();
        
//        if (bind(sock, reinterpret_cast<sockaddr*>(&adr), sizeof(adr)) == -1 ) {
//            close(sock);
//            cout << "failed to bind" << endl;
//            printf("failed to bind(errno:%d, error_str:%s)\n", errno, strerror(errno));
//        }
    }};
    
    attribute<symbol> address {this, "address", "/tmp/thx.sock",
        description {
            "Message destination."
        }
    };
    
    // define an optional argument for setting the message
//    argument<symbol> socket_type_arg { this, "socket type", "Either DGRAM or STREAM.",
//        MIN_ARGUMENT_FUNCTION {
//            if (arg == "STREAM") {
//                socket_type = SocketType::STREAM;
//            } else {
//                socket_type = SocketType::DGRAM;
//            }
//            create_socket();
//        }
//    };
    
    // the actual attribute for the message
//    attribute<SocketType> socket_type { this, "SocketType", SocketType::DGRAM,
//        description {
//            "Greeting to be posted. "
//            "The greeting will be posted to the Max console when a bang is received."
//        }
//    };


    // respond to the bang message to do something
//    message<> bang { this, "bang", "Post the greeting.",
//        MIN_FUNCTION {
//            symbol the_greeting = greeting;    // fetch the symbol itself from the attribute named greeting
//
//            cout << the_greeting << endl;    // post to the max console
//            output.send(the_greeting);       // send out our outlet
//            return {};
//        }
//    };
    
//    message<> bang {this, "bang", "test", MIN_FUNCTION {
//        std::vector<uint8_t> buffer(1024);
//        
////        sockaddr_un client;
//        const auto received_size = recv(sock, buffer.data(), buffer.size() - 1, 0);
//        
//        cout << "received size: " << received_size << endl;
//        if (received_size < 0) {
//            printf("failed to receive(errno:%d, error_str:%s)\n", errno, strerror(errno));
//        } else {
//            buffer.resize(received_size);
//            if(0 < buffer.size()) {
//                output.send(to_atoms(buffer));
//            }
//        }
//        return{};
//    }};
    
    message<threadsafe::yes> list {this, "list", "list to be sent.", MIN_FUNCTION {
        if(sendto(sock, args.data(), args.size(), 0, reinterpret_cast<sockaddr*>(&adr), sizeof(adr)) == -1) {
            printf("failed to send.(errno:%d, error_str:%s)\n", errno, strerror(errno));
        }
        return {};
    }};


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "hello world" << endl;
            return {};
        }
    };
};


MIN_EXTERNAL(ht_min_udssend);
