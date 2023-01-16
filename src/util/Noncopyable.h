#pragma once

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

} // namespace util

} // namespace nijika