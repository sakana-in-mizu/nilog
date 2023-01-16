#include "LogStream.h"

#include <chrono>
#include <iostream>
#include <thread>

void do_write(nijika::log::Logger::level level, int n) {
    nijika::log::LogStream ls(level);
    for (int i = 0; i < n; ++i) {
        ls << newl << "A long line......."
           << "..................";
    }
}

int main() {
    using namespace nijika::log;
    using namespace std::chrono;

    const int n = 1000000;

    Logger &logger = Logger::get_instance();
    logger.init();

    logger.async_run();
    auto begin1 = steady_clock::now();
    do_write(Logger::DEBUG, n);
    auto end1 = steady_clock::now();
    auto dur1 = duration_cast<milliseconds>(end1 - begin1).count();
    std::cout << "1 thread async mode: " << dur1 << "ms\n";
    logger.async_stop();

    auto begin2 = steady_clock::now();
    do_write(Logger::TRACE, n);
    auto end2 = steady_clock::now();
    auto dur2 = duration_cast<milliseconds>(end2 - begin2).count();
    std::cout << "1 thread sync mode: " << dur2 << "ms\n";

    logger.async_run();
    auto begin3 = steady_clock::now();
    std::thread threads1[5] = {
        std::thread(do_write, Logger::DEBUG, n),
        std::thread(do_write, Logger::DEBUG, n),
        std::thread(do_write, Logger::DEBUG, n),
        std::thread(do_write, Logger::DEBUG, n),
        std::thread(do_write, Logger::DEBUG, n),
    };
    for (auto &&thr : threads1) {
        thr.join();
    }
    auto end3 = steady_clock::now();
    auto dur3 = duration_cast<milliseconds>(end3 - begin3).count();
    std::cout << "5 threads async mode: " << dur3 << "ms\n";
    logger.async_stop();

    auto begin4 = steady_clock::now();
    std::thread threads2[5] = {
        std::thread(do_write, Logger::TRACE, n),
        std::thread(do_write, Logger::TRACE, n),
        std::thread(do_write, Logger::TRACE, n),
        std::thread(do_write, Logger::TRACE, n),
        std::thread(do_write, Logger::TRACE, n),
    };
    for (auto &&thr : threads2) {
        thr.join();
    }
    auto end4 = steady_clock::now();
    auto dur4 = duration_cast<milliseconds>(end4 - begin4).count();
    std::cout << "5 threads sync mode: " << dur4 << "ms\n";

    return 0;
}