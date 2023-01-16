#include "FixedBuffer.h"

#include <stdexcept>
#include <string.h>

namespace nijika {

namespace util {

void FixedBuffer::append(const char *data, size_t len) {
    if (len > avail()) {
        throw std::out_of_range("Not enough space in the buffer.");
    }

    memcpy(buf_.get() + bytes_, data, len);
    bytes_ += len;
}

FixedBuffer::FixedBuffer(FixedBuffer &&other) noexcept
    : buf_(std::move(other.buf_)), capacity_(other.capacity_),
      bytes_(other.bytes_) {
    other.capacity_ = 0;
    other.bytes_ = 0;
}

FixedBuffer &FixedBuffer::operator=(FixedBuffer &&other) noexcept {
    buf_ = std::move(other.buf_);
    capacity_ = other.capacity_;
    bytes_ = other.bytes_;

    other.capacity_ = 0;
    other.bytes_ = 0;

    return *this;
}

} // namespace util

} // namespace nijika