#pragma once

#include "util/FixedBuffer.h"
#include "util/Noncopyable.h"

#include <memory>
#include <string>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace nijika {

namespace log {

using namespace util;

/**
 * Buffered log file class. Supports log roatation.
 * @thread-safety: unsafe.
 */
class LogFile : Noncopyable {
  public:
    /**
     * Constructor.
     * @param path: log file path.
     * @param roll_size: log roll path.
     * @param buffer_size: internal buffer size.
     */
    LogFile(const std::string &path, off_t roll_size, size_t buffer_size);

    ~LogFile() {
        flush();
        close(fd_);
    }

    /**
     * Appends data to the file, possibly flushing the buffer to hold it.
     * @param data: pointer to the user data.
     * @param len: size of the data.
     */
    void append(const char *data, size_t len);

    /**
     * Appends data to the file, possibly flushing the buffer to hold it.
     * @param data: string holding the user data.
     */
    void append(const std::string &data) { append(data.data(), data.size()); }

    /**
     * Appends data to the file, possibly flushing the buffer to hold it.
     * @param data: buffer holding the user data.
     */
    void append(const FixedBuffer &data) { append(data.data(), data.bytes()); }

    /**
     * Flushes the buffer, possibly rolling the file if current size will exceed
     * roll size.
     */
    void flush();

  private:
    int fd_;             // Current file descriptor.
    off_t file_size_;    // Current file size.
    off_t roll_size_;    // Log file maximum size.
    FixedBuffer buffer_; // Internal buffer.
    std::string path_;   // Log file path.

    /**
     * Generates a file name according to current log path, pid and timestamp.
     * @return: a full path to the log file.
     */
    std::string get_file_name();

    /**
     * Opens a new log file.
     */
    void roll();
};

// extern size_t write_to_file;

} // namespace log

} // namespace nijika