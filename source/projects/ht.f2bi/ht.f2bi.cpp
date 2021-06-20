/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;


class ht_f2bi : public object<ht_f2bi> {
public:
    MIN_DESCRIPTION	{"Convert float into binary32 format."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"print, jit.print, dict.print"};

    inlet<>  input	{ this, "(float) convert value into binary32" };
    outlet<> output	{ this, "(anything) output the message which is posted to the max console" };

    message<> number {this, "float", "Converts float into binary32 format.",
        MIN_FUNCTION {
            float input = static_cast<float>(args[0]);
            
            uint8_t* ptr = reinterpret_cast<uint8_t*>(&input);
            output.send(ptr[0], ptr[1], ptr[2], ptr[3]);
            return {};
        }            
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
//            cout << "hello world" << endl;
            return {};
        }
    };

};


MIN_EXTERNAL(ht_f2bi);
