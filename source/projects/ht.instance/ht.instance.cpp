/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"
#include <map>
#include <string>


using namespace c74::min;

class ht_instance : public object<ht_instance> {
public:
    MIN_DESCRIPTION	{"Count the instances of the class."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"value"};

    inlet<>  input	{ this, "(bang) output" };
    outlet<> out_left	{ this, "(int) index of this instance. Starting from 0" };
    outlet<> out_right    { this, "(anything) total number of the instances belonging to the same class" };

    // define an optional argument for setting the message
    argument<symbol> class_name_arg { this, "class_name", "The name of the class the object belongs to.", true,
        MIN_ARGUMENT_FUNCTION {
            class_name_str = static_cast<std::string>(arg);
            if(class_map.find(class_name_str) == class_map.end()) { // if the key doesnt exist
                class_map[class_name_str] = 0;
                instance_index = 0;
            } else {
                instance_index = ++class_map[class_name_str]; // increment then apply
            }
        }
    };
    
    // respond to the bang message to do something
    message<> bang { this, "bang", "get the output.",
        MIN_FUNCTION {
            out_right.send(class_map[class_name_str] + 1);
            out_left.send(instance_index);
            return {};
        }
    };
    
    ~ht_instance() {
        class_map[class_name_str]--;
    }

private:
    std::string class_name_str;
    static std::map<std::string, int> class_map;
    int instance_index;
};
            
std::map<std::string, int> ht_instance::class_map;


MIN_EXTERNAL(ht_instance);
