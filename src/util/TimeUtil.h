#pragma once

#include <chrono>
#include <string>

#include <time.h>

namespace nijika {

namespace util {

#define __NIJIKA_MAX_TIME_STR 512

template <typename Clock, typename Dur>
std::string to_formatted_string(const std::string &fmt,
                                std::chrono::time_point<Clock, Dur> tp,
                                bool local = true) {
    using namespace std::chrono;
    time_t tt = duration_cast<seconds>(tp.time_since_epoch()).count();
    tm tm;
    if (local) {
        localtime_r(&tt, &tm);
    } else {
        gmtime_r(&tt, &tm);
    }

    char ans[__NIJIKA_MAX_TIME_STR];
    strftime(ans, sizeof(ans), fmt.c_str(), &tm);

    return ans;
}

} // namespace util

} // namespace nijika