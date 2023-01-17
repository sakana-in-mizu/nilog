# NILOG

## Overview

nilog is a light-weight C++ stream style log system.

- C++ stream style front-end.
- Supports customized log line format(through io manipulator).
- Supports high performance async-mode and sync-mode.
- Supports log file rotation.



## E.G.

#### E.G.1	Basic Usage

```C++
#include "LogStream.h"

int main() {
    using namespace nijika::log;
    
    Logger &logger = Logger::get_instance();	// Logger is a singleton.
    logger.init(); // Must call this before writing.
    logger.async_run(); // Turn on async-mode.
    
    LogStream ls(Logger::DEBUG);	// LogStream is not thread-safe and should not be shared between threads.
    ls << newl << "a log line"; 
    /* 
     * newl is a io manipulator, which flushes the LogStream internal buffer to the log system back-end
     * and starts a new log line.
     * You will see something like: 
     *
     * [DEBUG][TID: 18834][FILE: src/test/main.cc][FUNC: main][LINE: 11][2023-01-16 22:07:37.422158] a log line
     * |<----------------------------- inserted by newl ------------------------------------------>|
     *
     * in a log file named like nijika-18834-20230116-220739-4421.log (nijika-pid-date-time-rand.log)
     */
    return 0;
}
```



#### E.G.2	Define Your Own newl

The following is the default newl implementation provided by nilog:

```C++
// In LogStream.h
namespace nijika {

namespace log {
    
class NewLine : public NewLineBase {
  public:
    NewLine(const std::string &file, const std::string &func, int line);

    LogStream &operator()(LogStream &ls) const;

  private:
    std::string context_;
};

// Use macro to simplify usage.
#define newl nijika::log::NewLine(__FILE__, __func__, __LINE__)
 
}	// namespace log
   
}	// namespace nijika


// In LogStream.cc
namespace nijika {

namespace log {
    
NewLine::NewLine(const std::string &file, const std::string &func, int line) {
    using namespace std::chrono;

    context_ += "[FILE: " + file + "]";
    context_ += "[FUNC: " + func + "]";
    context_ += "[LINE: " + std::to_string(line) + "]";

    auto now = system_clock::now();
    auto usec = duration_cast<microseconds>(now.time_since_epoch()).count() % 1000000;
    
    // to_formatted_string is a wrapper of strftime.
    context_ += "[" + to_formatted_string("%F %T.", now) + std::to_string(usec) + "] ";
}

LogStream &NewLine::operator()(LogStream &ls) const {
    ls.flush();
    ls << "[" << Logger::level_str[ls.level] << "]"
       << "[TID: " << ls.tid << "]" << context_;
    return ls;
}
 
}	// namespace log
    
}	// namespace nijika
```

By defining a class inheriting NewLineBase, you can customize log line format.
