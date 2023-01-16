#include "log/Logger.h"
#include "util/CountDownLatch.h"
#include "util/TimeUtil.h"
#include "log/LogFile.h"

#include <iostream>
#include <stdexcept>

namespace nijika {

namespace log {

using namespace util;

const char *Logger::DEFAULT_PATH = "./";

const char *Logger::level_str[6] = {"DEBUG", "INFO",  "TRACE",
                                    "WARN",  "ERROR", "FATAL"};

std::chrono::seconds Logger::DEFAULT_FLUSH_INTERVAL(3);

Logger &Logger::get_instance() {
    static Logger self;
    return self;
}

void Logger::init(const std::string &path, off_t roll_size, size_t buffer_size,
                  std::chrono::seconds flush_interval) {
    if (output_) {
        // An effective log file indicates the initialization is done.
        return;
    }

    output_ = std::make_unique<LogFile>(path, roll_size, buffer_size * 3);
    buffer_size_ = buffer_size;
    flush_interval_ = flush_interval;
}

Logger::Logger() : run_(false), buffer_size_(DEFUALT_BUFFER_SIZE) {}

Logger::~Logger() { async_stop(); }

// size_t write_submit = 0;
void Logger::write(const std::string &line) {
    if (!output_) {
        throw std::logic_error("Log file is not open.");
    }

    // write_submit += line.size();
    if (run_) {
        async_write(line);
    } else {
        sync_write(line);
    }
}

void Logger::sync_write(const std::string &line) {
    std::lock_guard<std::mutex> lock(mut_);
    output_->append(line);
}

void Logger::async_run() {
    if (run_) {
        return;
    }

    run_ = true;

    // Pre-allocation.
    curr_buf_ = FixedBuffer(buffer_size_);
    next_buf_ = FixedBuffer(buffer_size_);
    buffers_.reserve(32);

    latch_ = std::make_unique<CountDownLatch>(1);

    writer_ = std::make_unique<std::thread>(&Logger::writer_func, this);

    // Wait for writer to initialize.
    latch_->wait();
}

void Logger::async_stop() {
    if (!run_) {
        return;
    }

    run_ = false;
    {
        std::lock_guard<std::mutex> lock(mut_);
        cv_.notify_one();
    }

    writer_->join();
}

// size_t async_write_submit = 0;

void Logger::async_write(const std::string &line) {
    std::lock_guard<std::mutex> lock(mut_);

    if (curr_buf_.avail() > line.size()) {
        // Enough space in current buffer.
        curr_buf_.append(line);
        return;
    }

    buffers_.emplace_back(std::move(curr_buf_));

    if (next_buf_) {
        // Replace current buffer.
        curr_buf_ = std::move(next_buf_);
    } else {
        // Allocate new buffer(rarely happens).
        curr_buf_ = FixedBuffer(buffer_size_);
    }

    curr_buf_.append(line);

    // Notify writer.
    cv_.notify_one();
}

void Logger::writer_func() {

    // Pre-allocation.
    FixedBuffer buf1 = FixedBuffer(buffer_size_); // Backup current buffer.
    FixedBuffer buf2 = FixedBuffer(buffer_size_); // Backup next buffer.
    std::vector<FixedBuffer> buffers_to_write;     // Backup buffer queue.
    buffers_to_write.reserve(32);

    // Writer has successfully initialized.
    latch_->count_down();

    while (run_) {
        {
            // Use move and swap to shorten the critical section.
            std::unique_lock<std::mutex> lock(mut_);
            if (buffers_.empty()) {
                cv_.wait_for(lock, flush_interval_);
            }

            buffers_.emplace_back(std::move(curr_buf_));
            curr_buf_ = std::move(buf1);
            if (!next_buf_) {
                next_buf_ = std::move(buf2);
            }
            buffers_to_write.swap(buffers_);
        }

        if (buffers_to_write.size() > 25) {
            // To many buffers in the buffer queue.
            std::string msg = "Dropped log messages at ";
            msg +=
                to_formatted_string("%F %T", std::chrono::system_clock::now());
            msg += ", ";
            msg += std::to_string(buffers_to_write.size() - 2);
            msg += " larger buffers.\n";
            std::cerr << msg;

            output_->append(msg);
            buffers_to_write.erase(buffers_to_write.begin() + 2,
                                   buffers_to_write.end());
        }

        for (auto &&buffer : buffers_to_write) {
            // Writing to log file.
            output_->append(buffer);
            // async_write_submit += buffer.bytes();
        }

        if (buffers_to_write.size() > 2) {
            buffers_to_write.resize(2);
        }

        // Resume backup buffers after writing.
        if (!buf1) {
            buf1 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            buf1.clear(); // Remember to reset the buffer.
        }

        if (!buf2) {
            buf2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            buf2.clear(); // Remember to reset the buffer.
        }

        // Clear the buffer queue.
        buffers_to_write.clear();
        output_->flush();
    } // end while

    // Flush rest data in current buffer and buffer queue.
    std::lock_guard<std::mutex> lock(mut_);
    if (!curr_buf_.empty()) {
        buffers_.emplace_back(std::move(curr_buf_));
    }
    for (auto &&buf : buffers_) {
        output_->append(buf);
    }
    buffers_.clear();

    output_->flush();
}

} // namespace log

} // namespace nijika