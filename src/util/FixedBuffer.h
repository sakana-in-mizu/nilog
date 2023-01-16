#pragma once

#include "Noncopyable.h"

#include <memory>
#include <string>

namespace nijika {

namespace util {

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

} // namespace nijika