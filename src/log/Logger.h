#pragma once

#include "util/CountDownLatch.h"
#include "util/FixedBuffer.h"
#include "util/Noncopyable.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace nijika {

namespace log {

class LogFile;

using namespace util;

/**
 * Log system back-end. Supports both synchronous and asynchronous mode.
 * @thread-safety: safe.
 */
class Logger : Noncopyable {

  public:
    static constexpr off_t DEFAULT_ROLL_SIZE = 1 << 26;    // Aka 64MB.
    static constexpr size_t DEFUALT_BUFFER_SIZE = 1 << 23; // Aka 8MB.
    static const char *DEFAULT_PATH;
    static std::chrono::seconds DEFAULT_FLUSH_INTERVAL;

    enum level { DEBUG = 0, INFO, TRACE, WARN, ERROR, FATAL };
    static const char *level_str[6];

    ~Logger();

    // Singleton.
    static Logger &get_instance();

    /**
     * Inits the log system. Must be called once before writing.
     * @param path: log file path.
     * @param roll_size: log file roll size.
     * @param buffer_size: internal buffer size.
     * @param flush_interval: auto flush time interval. (only used in
     * async-mode)
     */
    void init(const std::string &path = DEFAULT_PATH,
              off_t roll_size = DEFAULT_ROLL_SIZE,
              size_t buffer_size = DEFUALT_BUFFER_SIZE,
              std::chrono::seconds flush_interval = DEFAULT_FLUSH_INTERVAL);

    /**
     * Turns on async-mode.
     */
    void async_run();

    /**
     * Turns off async-mode.
     */
    void async_stop();

  private:
    std::unique_ptr<LogFile> output_; // Log file.
    std::mutex mut_;                  // For thread satety.

    std::unique_ptr<std::thread> writer_;   // Writing thread(consumer).
    std::atomic<bool> run_;                 // Async-mode indicator.
    std::condition_variable cv_;            // For blocking consumer(writer).
    std::unique_ptr<CountDownLatch> latch_; // For safely turning on async-mode.

    std::chrono::seconds flush_interval_; // Auto flush time interval.
    size_t buffer_size_;                  // Internal buffer size.
    FixedBuffer curr_buf_;                // Current user buffer.
    FixedBuffer next_buf_;                // Backup user buffer.
    std::vector<FixedBuffer> buffers_;    // Buffer queue.

    // Log system front-end.
    friend class LogStream;

    // Singleton
    Logger();

    /**
     * Interface for logstream, called by logstream::flush();
     * @param line: a log line.
     */
    void write(const std::string &line);

    /**
     * Called by write in sync-mode, possibly blocking calling thread.
     * @param line: a log line.
     */
    void sync_write(const std::string &line);

    /**
     * Called by write in async-mode.
     * @param line: a log line.
     */
    void async_write(const std::string &line);

    /**
     * Writing thread routine.
     */
    void writer_func();
};

// extern size_t write_submit;
// extern size_t async_write_submit;

} // namespace log

} // namespace nijika