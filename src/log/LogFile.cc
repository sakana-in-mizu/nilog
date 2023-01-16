#include "log/LogFile.h"
#include "util/ProcessInfo.h"
#include "util/TimeUtil.h"

#include <stdexcept>
#include <system_error>

#include <stdlib.h>

namespace nijika {

namespace log {

LogFile::LogFile(const std::string &path, off_t roll_size, size_t buffer_size)
    : file_size_(0), roll_size_(roll_size), buffer_(buffer_size), path_(path) {
    fd_ = open(get_file_name().c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd_ == -1) {
        throw std::system_error(
            std::error_code(errno, std::generic_category()));
    }
}

std::string LogFile::get_file_name() {
    using namespace std::chrono;

    std::string name = path_ + "nijika-";
    name += std::to_string(get_pid()) + "-";
    name += to_formatted_string("%Y%m%d-%H%M%S", system_clock::now());
    name += "-";
    name += std::to_string(1000 + rand() % 9000); // 1000-9999
    name += ".log";

    return name;
}

void LogFile::append(const char *data, size_t len) {
    if (len > buffer_.avail()) {
        flush();
    }

    buffer_.append(data, len);
}

// size_t write_to_file = 0;
void LogFile::flush() {
    size_t n = buffer_.bytes();

    if (file_size_ + n > roll_size_) {
        roll();
    }

    size_t written = 0;
    while (written < n) {
        ssize_t res = write(fd_, buffer_.data() + written, n - written);
        if (res == -1) {
            int ec = errno;
            if (ec == EINTR) {
                continue;
            }
            throw std::system_error(ec, std::generic_category());
        }
        written += res;
    }

    // write_to_file += n;
    file_size_ += n;
    buffer_.clear();
}

void LogFile::roll() {
    close(fd_);
    file_size_ = 0;
    fd_ = open(get_file_name().c_str(), O_WRONLY | O_CREAT, 0644);
}

} // namespace log

} // namespace nijika