/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;


class ht_between : public object<ht_between> {
public:
    MIN_DESCRIPTION	{"See if the number is between two values."};
    MIN_TAGS		{"math"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"minimum, maximum, =="};

    inlet<>  val_in	{ this, "(number) the value to be evaluated"};
    inlet<>  min_in    { this, "(number) the lower threshold"};
    inlet<>  max_in   { this, "(number) the higher threshold"};
    outlet<thread_check::scheduler, thread_action::fifo> low_out    { this, "(bang) when the value is below the lower threshould." };
    outlet<thread_check::scheduler, thread_action::fifo> mid_out	{ this, "(bang) when the value is between the threshoulds." };
    outlet<thread_check::scheduler, thread_action::fifo> high_out    { this, "(bang) when the value is above the higher threshould." };
    
    // define an optional argument for setting the message
    argument<number> min_arg { this, "value min", "Minimum Threshold.",
        MIN_ARGUMENT_FUNCTION {
            min = arg;
        }
    };
    
    argument<number> max_arg { this, "value max", "Maximum Threshold.",
        MIN_ARGUMENT_FUNCTION {
            max = arg;
        }
    };
    
    attribute<number> min {this, "min", 0.0,
        title {"Mimimum Threshold"},
        description {"Lower value between a and b"},
        category {"Range"}, order {1}
    };
    
    attribute<number> max {this, "max", 1.0,
        title {"Maximum Threshold"},
        description {"Lower value between a and b"},
        category {"Range"}, order {2}
    };
    
    attribute<bool> exclude {this, "exclude", true,
        title {"Exclude"},
        description {"True exludes a and b. Default:true"},
    };
    
    
    message<threadsafe::yes> number { this, "number", "number to be evaluated",
        MIN_FUNCTION {
            switch(inlet) {
                case 0:
                    checkMinMax();
                    evaluate(args[0], min, max);
                    break;
                case 1:
                    min = args[0];
                    break;
                case 2:
                    max = args[0];
                    break;
                default:
                    break;
            }
            return {};
        }
    };
    
    message<threadsafe::yes> list {this, "list", "val, min, max",
        MIN_FUNCTION {
            min = args[1];
            max = args[2];
            checkMinMax();
            evaluate(args[0], min, max);
            return {};
        }
    };

private:
    inline void evaluate(c74::min::number val, c74::min::number min, c74::min::number max)    {
        if(exclude) {
            if(min < val && val < max) {
                mid_out.send(k_sym_bang);
            } else if (max <= val)  {
                high_out.send(k_sym_bang);
            } else {
                low_out.send(k_sym_bang);
            }
        } else {
            if(min <= val && val <= max) {
                mid_out.send(k_sym_bang);
            } else if (max < val) {
                high_out.send(k_sym_bang);
            } else {
                low_out.send(k_sym_bang);
            }
        }
    }
            
    inline void checkMinMax() {
        if (max < min) {
            const double tmp_min = min;
            const double tmp_max = max;
            
            min = tmp_max;
            max = tmp_min;
        }
    }
};


MIN_EXTERNAL(ht_between);
