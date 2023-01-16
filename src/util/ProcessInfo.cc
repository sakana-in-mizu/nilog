#include "ProcessInfo.h"

#include <sys/syscall.h>

namespace nijika {

namespace util {

pid_t get_pid() {
    static pid_t pid = getpid();
    return pid;
}

pid_t get_tid() {
    thread_local pid_t tid = syscall(SYS_gettid);
    return tid;
}

} // namespace util

} // namespace nijika