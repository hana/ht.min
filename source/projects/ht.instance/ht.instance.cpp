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
    MIN_DESCRIPTION	{"Post to the Max Console."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Cycling '74"};
    MIN_RELATED		{"print, jit.print, dict.print"};

    inlet<>  input	{ this, "(bang) post greeting to the max console" };
    outlet<> output_left	{ this, "(int) index of this instance. Starts from 0." };
    outlet<> output_right    { this, "(int) number of all instances with the same name." };
    
    
    //Constructor
    ht_instance(const atoms& args = {}) {
        instance_index = 0;
        
        const auto name_str = static_cast<std::string>(instance_name.get());
        
        if(instance_counter.find(name_str) == instance_counter.end()) { // the key doesnt exist
            instance_index = 0;
            instance_counter[name_str] = 0;
        } else {
            instance_index = ++instance_counter[name_str];
        }
    };
    
    // define an optional argument for setting the message
    argument<symbol> instance_name_arg { this, "instance_name", "Instance name", true,
        MIN_ARGUMENT_FUNCTION {
            instance_name = arg;
        }
    };
    
    attribute<symbol> instance_name { this, "instance_name", "hogehoge",
           description {
               "Greeting to be posted. "
               "The greeting will be posted to the Max console when a bang is received."
           }
       };

    // respond to the bang message to do something
    message<> bang { this, "bang", "Post the greeting.",
        MIN_FUNCTION {
            auto name_str = static_cast<std::string>(instance_name.get());
            cout << name_str << endl;
//            cout << instance_counter[name_str] << endl;
//            cout << ++instance_counter[name_str] << endl;
            
//            instance_index = ++instance_counter[name_str];
            
//            output_right.send(instance_counter[name_str]);
            output_right.send(instance_counter[name_str]);
            output_left.send(instance_index);
            return {};
        }
    };
            
    

       
private:
    int instance_index;
    static std::map<std::string, int> instance_counter;
};

std::map<std::string, int> ht_instance::instance_counter;


MIN_EXTERNAL(ht_instance);
