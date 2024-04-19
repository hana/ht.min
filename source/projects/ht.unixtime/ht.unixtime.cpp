/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;

#include <chrono>

class ht_min_unixtime : public object<ht_min_unixtime> {
public:
    MIN_DESCRIPTION	{"Output the unix time"};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"metro, date, filedate"};

    inlet<>  input	{ this, "(bang) output the unix time" };
    outlet<> output	{ this, "(int) output the unix time" };


    // respond to the bang message to do something
    message<threadsafe::yes> bang { this, "bang", "Post the greeting.",
        MIN_FUNCTION {
            const auto now = std::chrono::system_clock::now();
            const auto unixtime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            output.send(unixtime);       // send out our outlet
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            return {};
        }
    };

};


MIN_EXTERNAL(ht_min_unixtime);
