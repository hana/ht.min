/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.


#include "c74_min.h"

#include <ctime>
#include <time.h>
#include <chrono>
#include <cmath>

#ifdef _WIN32
#define timezone _timezone
#endif

using namespace c74::min;

class ht_alarm : public object<ht_alarm> {
public:
    MIN_DESCRIPTION    {"Bang when the time comes."};
    MIN_TAGS        {"utilities"};
    MIN_AUTHOR        {"Hananosuke Takimoto"};
    MIN_RELATED        {"ht.alarm~, date, clocker, timer"};
        
    inlet<>  input    { this, "(list) h,m,s / yyyy,m,d,h,m,s" };
    outlet<> output    { this, "(bang) output the bang when the time comes." };
    outlet<> info_out    { this, "(int) Time zone info." };
        
        
    ht_alarm(const atoms& args = {}) {
        tzset();
        updateTimezoneOffset();
    }
        
    // define an optional argument for setting the message
    argument<float> resolution_arg { this, "resolution", "Timer resolution in ms.",
        MIN_ARGUMENT_FUNCTION {
            resolution = arg;
        }
    };
    
    // the actual attribute for the message
    attribute<double> resolution { this, "resolution", 0.0,
        description {
            "Timer resolution, default 1ms"
        }
    };
    
    attribute<bool> utc {this, "utc", false, description{ "If true, the set value uses utc, otherwise local time"}};
        
        
    // respond to the bang message to do something
    message<> bang { this, "bang", "Update the internal clock in case the system clock changes.",
        MIN_FUNCTION {
            updateTimezoneOffset();
            return {};
        }
    };
        
    message<> check {
        this,
        "check",
        "Check the current alarm setting.",
        MIN_FUNCTION {
            static struct tm* wt;
              if(utc) {
                  wt = std::gmtime(&wakeup_time);
              }   else {
                  wt = std::localtime(&wakeup_time);
              }
              info_out.send(wt->tm_year+1900, wt->tm_mon+1, wt->tm_mday, wt->tm_hour, wt->tm_min, wt->tm_sec);
              return {};
        }
    };
                
    // set
    message<threadsafe::yes> set {this,"set","Set the time",
        MIN_FUNCTION {
            metro.stop();
            std::time_t now = time(nullptr);
            struct tm* localtime = std::localtime(&now);
            struct tm time_tm;
            
            // create general time info
            if(args.size() < 4) {
                 time_tm = tm{args[2], args[1], args[0], localtime->tm_mday, localtime->tm_mon, localtime->tm_year};
            } else {
                time_tm = tm{args[5], args[4], args[3], args[2], static_cast<int>(args[1])-1, static_cast<int>(args[0])-1900};
            }
            
            // generate missning time info
            wakeup_time = std::mktime(&time_tm);
            
            // adjust time depends on "utc".
            adjust(time_tm);
            
            // set adjusted time
            wakeup_time = std::mktime(&time_tm);
   
            metro.delay(resolution);
            return {};
        }
    };
                

    // called interval
    timer<> metro { this,
        MIN_FUNCTION {
            std::time_t now = time(nullptr);
            const double diff = difftime(now, wakeup_time); // difftime returns double but it does NOT contain ms values
            if( 0.0 <= diff)  {
                output.send(symbol("bang"));
                metro.stop();
            } else if (diff < -1.0) {   // more than 1 sec
                metro.delay((std::abs(diff) - 1.0) * 1000.0);
            } else {
                metro.delay(resolution);
            }
                    
            return {};
        }
    };
private:
    std::time_t wakeup_time;
    int tz_offset;
    
    void updateTimezoneOffset() {
        tzset();
        static std::time_t now;
        now = time(nullptr);
        
        struct tm* localtime = std::localtime(&now);
        
        tz_offset =  -static_cast<int>(timezone / 3600) + localtime->tm_isdst;
    };
    
    void adjust(tm& _tm)  {
        if(utc) {
            _tm.tm_hour += tz_offset;
            _tm.tm_hour -= _tm.tm_isdst;
        } else {
            _tm.tm_hour -= _tm.tm_isdst;
        }
    }
};


MIN_EXTERNAL(ht_alarm);
