/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;

#include <vector>
#include <string>
#include <span>
#include <utility>


#include "ht_min.h"

static_assert(202002L <= __cplusplus, "C++20 or above required");

class ht_OSC : public object<ht_OSC> {
private:
    inline void decode(const c74::min::atoms& packet) {    // binary to list        
        const auto msgs = ht::min::osc::packet::to_atoms(packet, err_check_atr);
        
        for(const auto& msg : msgs) {
            timetag_out.send(msg.timetag);
            message_out.send(msg.atoms);
        }
    };
            
    inline void encode(const c74::min::atoms& atms) {   // atoms to binary
        const auto packet = ht::min::osc::atoms::to_packet(atms, double_atr);
        message_out.send(to_atoms(packet));
    }
    
    function process = MIN_FUNCTION {
        switch(operation) {
            case operations::encode:
                encode(args);
                break;
            case operations::decode:
                if(args[0].type() == c74::min::message_type::symbol_argument) {
                    cerr << "Decode only supports binaries" << endl;
                    return{};
                }
                decode(args);
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
    outlet<thread_check::scheduler, thread_action::fifo> message_out	{ this, "(list) outputs atoms or binary" };
    outlet<thread_check::scheduler, thread_action::fifo> timetag_out {this, "(int) output timetag in decode mode"};
    
    enum class operations : int {encode, decode, enum_count};
    enum_map operation_range = {"encode", "decode"};
    
    argument<symbol> operation_arg {this, "operation", "Operation mode - encode from binary to list or decode list to binary.",
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
    
    attribute<bool> double_atr { this, "double", false,
        title {"double"},
        description {"If enabled, floating-point values are handleed as double"}
    };
    
    attribute<bool> err_check_atr {this, "error_check", false,
        title {"Error check"},
        description {"Checks if the input is a list of 0-255"}
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
