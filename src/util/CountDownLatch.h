#pragma once

#include <condition_variable>
#include <mutex>

namespace nijika {

namespace util {

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

} // namespace util

} // namespace nijika