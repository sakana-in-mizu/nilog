#include "util/CountDownLatch.h"

namespace nijika {
namespace util {

CountDownLatch::CountDownLatch(int count) : count_(count) {}

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(mut_);
    cv_.wait(lock, [this]() { return count_ == 0; });
}

void CountDownLatch::count_down() {
    std::lock_guard<std::mutex> lock(mut_);
    if (--count_ == 0) {
        cv_.notify_all();
    }
}

} // namespace util

} // namespace nijika