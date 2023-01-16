#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace nijika {

namespace util {

class Noncopyable {
  private:
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;

  protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

class CountDownLatch {
  public:
    /**
     * Constructor.
     * @param count: initial value of internal counter.
     */
    explicit CountDownLatch(int count);

    /**
     * Waits until the counter decrease to zero.
     */
    void wait();

    /**
     * Decreases the counter.
     */
    void count_down();

  private:
    int count_;
    std::mutex mut_;
    std::condition_variable cv_;
};


/**
 * Movable fix-sized output buffer. Forbids copy for the sake of performace.
 * @thread-safety: unsafe.
 */
class FixedBuffer : Noncopyable {
  public:
    FixedBuffer() noexcept : capacity_(0), bytes_(0) {}

    /**
     * Constructs a buffer of specified capacity.
     * @param capacity: User specified capacity.
     */
    explicit FixedBuffer(size_t capacity)
        : buf_(new char[capacity]), capacity_(capacity), bytes_(0) {}

    FixedBuffer(FixedBuffer &&other) noexcept;
    FixedBuffer &operator=(FixedBuffer &&other) noexcept;

    /**
     * Operator bool.
     * @return: returns true if the object holds an non-null pointer to the
     * buffer.
     */
    operator bool() const noexcept { return (bool)buf_; }

    /**
     * Clear the data in the buffer.
     */
    void clear() noexcept { bytes_ = 0; }

    /**
     * @return: the size of data in the buffer.
     */
    size_t bytes() const noexcept { return bytes_; }

    /**
     * @return: the capacity of the buffer.
     */
    size_t capacity() const noexcept { return capacity_; }

    /**
     * @return: the size of empty space in the buffer.
     */
    size_t avail() const noexcept { return capacity_ - bytes_; }

    /**
     * @return a pointer to the data in the buffer.(read-only)
     */
    const char *data() const noexcept { return buf_.get(); }

    /**
     * Appends data to the buffer.
     * @param data: pointer to the user data.
     * @param len: size of the data.
     */
    void append(const char *data, size_t len);

    /**
     * Appends data to the buffer.
     * @param data: string holding the user data.
     */
    void append(const std::string &data) { append(data.data(), data.size()); }

    /**
     * @return: true if the buffer is empty.
     */
    bool empty() const noexcept { return bytes_ == 0; }

  private:
    std::unique_ptr<char[]> buf_; // Buffer pointer.
    size_t capacity_;             // Capacity of the buffer.
    size_t bytes_;                // Size of data in the buffer.
};

} // namespace util

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