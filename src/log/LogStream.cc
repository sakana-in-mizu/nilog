#include "log/LogStream.h"
#include "util/ProcessInfo.h"
#include "util/TimeUtil.h"

#include <chrono>

namespace nijika {

namespace log {

LogStream::LogStream(Logger::level level)
    : level(level), std::stringstream(std::ios::out | std::ios::ate),
      tid("[TID: " + std::to_string(get_tid()) + "]") {}

LogStream::~LogStream() { flush(); }

void LogStream::flush() {
    if (str().size() == 0) {
        return;
    }

    *this << '\n';
    Logger::get_instance().write(str());
    str("");
}

LogStream &operator<<(LogStream &ls, const NewLineBase &nl) { return nl(ls); }

NewLine::NewLine(const std::string &file, const std::string &func, int line) {
    using namespace std::chrono;

    context_ += "[FILE: " + file + "]";
    context_ += "[FUNC: " + func + "]";
    context_ += "[LINE: " + std::to_string(line) + "]";

    auto now = system_clock::now();
    auto usec =
        duration_cast<microseconds>(now.time_since_epoch()).count() % 1000000;
    context_ +=
        "[" + to_formatted_string("%F %T.", now) + std::to_string(usec) + "] ";
}

LogStream &NewLine::operator()(LogStream &ls) const {
    ls.flush();
    ls << "[" << Logger::level_str[ls.level] << "]" << ls.tid << context_;
    return ls;
}

} // namespace log

} // namespace nijika
