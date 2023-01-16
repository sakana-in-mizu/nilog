/**
 * This is a public header.
 */

#pragma once

#include "Logger.h"

#include <sstream>

namespace nijika {

namespace log {

/**
 * C++ stream style log system front-end, used to construct
 * log line.
 * @thread-safety: unsafe.
 */
class LogStream : public std::stringstream {
  public:
    /**
     * Constructor.
     * @param level: log level.(DEBUG, INFO, TRACE, WARN, ERROR,
     * FATAL)
     */
    explicit LogStream(Logger::level level);
    ~LogStream();

    /**
     * Flush the log line to the back end and clear the internal
     * buffer of the stream.
     */
    void flush();

    Logger::level level;   // log level
    const std::string tid; // [TID]
};

/**
 * IO manipulator for LogStream.
 * By defining their own NewLine class extends this class,
 * users can customize their own log line format.
 */
class NewLineBase {
  public:
    NewLineBase() = default;
    virtual LogStream &operator()(LogStream &ls) const = 0;
    virtual ~NewLineBase() = default;
};

/**
 * Default NewLine class used by LogStream.
 */
class NewLine : public NewLineBase {
  public:
    NewLine(const std::string &file, const std::string &func, int line);

    LogStream &operator()(LogStream &ls) const;

  private:
    std::string context_;
};

#define newl nijika::log::NewLine(__FILE__, __func__, __LINE__)

LogStream &operator<<(LogStream &ls, const NewLineBase &nl);

#define NIJIKA_LOG_DEBUG nijika::log::logstream(nijika::log::logger::DEBUG)
#define NIJIKA_LOG_INFO nijika::log::logstream(nijika::log::logger::INFO)
#define NIJIKA_LOG_TRACE nijika::log::logstream(nijika::log::logger::TRACE)
#define NIJIKA_LOG_WARN nijika::log::logstream(nijika::log::logger::WARN)
#define NIJIKA_LOG_ERROR nijika::log::logstream(nijika::log::logger::ERROR)
#define NIJIKA_LOG_FATAL nijika::log::logstream(nijika::log::logger::FATAL)

} // namespace log

} // namespace nijika