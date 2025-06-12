/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <cstring>
#include <array>
#include <vector>

using namespace c74::min;


class ht_blob2f32 : public object<ht_blob2f32> {
public:
    MIN_DESCRIPTION	{"Convert blob into float32 array."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"udpreceive, float"};

    inlet<>  input	{ this, "(list) Little endian float binary array" };
    outlet<> output	{ this, "(list) float values" };

    std::vector<float> vec;

//    // define an optional argument for setting the message
//    argument<symbol> greeting_arg { this, "greeting", "Initial value for the greeting attribute.",
//        MIN_ARGUMENT_FUNCTION {
//            greeting = arg;
//        }
//    };
//
//
//    // the actual attribute for the message
//    attribute<symbol> greeting { this, "greeting", "hello world",
//        description {
//            "Greeting to be posted. "
//            "The greeting will be posted to the Max console when a bang is received."
//        }
//    };
//
//
//    // respond to the bang message to do something
//    message<> bang { this, "bang", "Post the greeting.",
//        MIN_FUNCTION {
//            symbol the_greeting = greeting;    // fetch the symbol itself from the attribute named greeting
//
//            cout << the_greeting << endl;    // post to the max console
//            output.send(the_greeting);       // send out our outlet
//            return {};
//        }
//    };
    
    message<threadsafe::yes> list {
        this, "list", "blob to be converted into float32",
        MIN_FUNCTION {
            const auto size = args.size() / 4;
            vec.clear();
            
//            cout << "size: " << size << endl;
                        
            for(std::size_t i = 0; i < size; i++) {
                int ival = 0;
                for(std::size_t j = 0; j < 4; j++ ){
                    ival += int(args[i * 4 + j]) << (8 * j);
                }

                float fval;
                std::memcpy(&fval, &ival, sizeof(float));
                
//                printf("index: %ul, value: %f", i * 4 , fval);
                vec.emplace_back(fval);
                //                vec.emplace_back(*reinterpret_cast<float>(val));
                
            }
//            cout << "size: " << size << ", 0:" << args[0] << endl;
//            vec.resize(size, 0.0f);
//            std::memcpy(vec.data(), args.data(), args.size());
            output.send(to_atoms(vec));
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
//            cout << "hello world" << endl;
            vec.reserve(1000);
            return {};
        }
    };

};


MIN_EXTERNAL(ht_blob2f32);
