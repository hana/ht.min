/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;

#include <vector>
#include <string>

#include "oscpp/server.hpp"
#include "oscpp/client.hpp"

class ht_OSC : public object<ht_OSC> {
private:
    c74::min::atoms decode(const c74::min::atoms& packet) {    // binary to list
        std::vector<uint8_t> p;
        p.reserve(packet.size());
        
        constexpr auto min = std::numeric_limits<uint8_t>::min();
        constexpr auto max = std::numeric_limits<uint8_t>::max();
        
        for(const auto& byte : packet) {
            const int val = static_cast<int>(byte);
            if(max < val) {
                p.emplace_back(max);
            } else if (val < min) {
                p.emplace_back(min);
            } else {
                p.emplace_back(val);
            }
        }
        
        OSCPP::Server::Message msg (OSCPP::Server::Packet(p.data(), p.size()));
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
        return atm;
    }
    
    c74::min::atoms encode(const c74::min::atoms& atms) {   // list to binary
        static std::vector<uint8_t> buffer;
        buffer.resize(2048);
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        
        const auto adr = static_cast<std::string>(atms[0]);
        const auto& num_args = atms.size() - 1;
        
        packet.openMessage(adr.c_str(), num_args);
                        
        for(auto i = 1; i < atms.size(); i++) {
            const auto& arg = atms[i];
            switch(arg.type()) {
                case c74::min::message_type::int_argument:
                    packet.int32(arg);
                    break;
                case c74::min::message_type::float_argument:
                    packet.float32(arg);
                    break;
                case c74::min::message_type::symbol_argument:
                    packet.string(static_cast<std::string>(arg).c_str());
                    break;
                default:
                    break;
            }
        }
        
        packet.closeMessage();
        buffer.resize(packet.size());
                    
        return to_atoms(buffer);
    }
    
    function process = MIN_FUNCTION {
        switch(operation) {
            case operations::encode:
                output.send(encode(args));
                break;
            case operations::decode:
                if(args[0].type() == c74::min::message_type::symbol_argument) {
                    cerr << "Decode only supports binaries" << endl;
                    return{};
                }

                output.send(decode(args));
                break;
            default:
                break;
        }

        return {};
    };
    
public:
    MIN_DESCRIPTION	{"Encode list to binary, decode binary to list."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, udpsend, ht.udpreceive, ht.udpsend"};
    
    inlet<>  input	{ this, "(list) things to be handled" };
    outlet<> output	{ this, "(list) outputs atoms or binary" };
    
    enum class operations : int {encode, decode, enum_count};
    enum_map operation_range = {"encode", "decode"};
    
    argument<symbol> operation_arg = {this, "operation", "Operation mode - encode from binary to list or decode list to binary.",
        MIN_ARGUMENT_FUNCTION {
            if (arg == "decode") {
                operation = operations::decode;
            } else {
                operation = operations::encode;
            }
        }
    };
    
    attribute<operations> operation { this, "operation", operations::encode, operation_range,
        description {
            "Choose the operation to perfome with the input. encode or decode."
        }
    };
    
    message<threadsafe::yes> list {this, "list", "binary or list", process};
    message<threadsafe::yes> anything {this, "anything", "binary or list", process};

    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "ht.OSC v1.0.0" << endl;
            return {};
        }
    };

};


MIN_EXTERNAL(ht_OSC);
