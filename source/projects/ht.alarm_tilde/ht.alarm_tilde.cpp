/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.



#include "c74_min.h"

#include <ctime>
#include <time.h>
#include <chrono>

#ifdef _WIN32
#define timezone _timezone
#endif

using namespace c74::min;

class ht_alarm_tilde : public object<ht_alarm_tilde>, public sample_operator<1, 0> {
public:
    MIN_DESCRIPTION	{"Bang when the time has come."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"ht.alarm, clocker, date, timer"};

    inlet<>  input	{ this, "(set) sets time - h:m:s or yyyy:m:d:h:m:s." };
//    outlet<thread_check::scheduler, thread_action::fifo> output	{ this, "(bang) output the message which is posted to the max console" };
    outlet<> output    { this, "(bang) output the message which is posted to the max console" };
    outlet<thread_check::scheduler, thread_action::fifo> info_out    { this, "(list) output the current alarm time" };

    
    ht_alarm_tilde(const atoms& args = {}) {
        tzset();
        updateTimezoneOffset();
        running = false;
    };
    
    attribute<bool> utc {
        this,
        "utc",
        false,
        description {"If true, the set value uses utc, otherwise local time"}
    };

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
        "Check the alarm time in the local time.",
        MIN_FUNCTION    {
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
    
    // set timer
    message<threadsafe::yes> set {
        this,
        "set",
        "Set the time.",
        MIN_FUNCTION {
            running = false;
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
            
            // run checking
            running = true;
            return {};
        }
    };
    
    timer<> metro {this, MIN_FUNCTION{
        output.send("bang");
        metro.stop();
        return {};
    }};
    
    void operator()(sample x)   {
        static std::time_t now;
        if(running) {
            now = time(nullptr);
            if(0.0 <= difftime(now, wakeup_time)) {
                metro.delay(0.0);
                running = false;
            }
        }
    };


private:
    bool running;
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


MIN_EXTERNAL(ht_alarm_tilde);
