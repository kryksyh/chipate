#pragma once
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <raylib.h>
#include <sstream>
#include <string>

namespace log {

inline std::string timestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    auto t   = system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0')
        << ms.count();
    return oss.str();
}

inline void logMessage(int level, char const* file, int line, char const* fmt, ...)
{
    char    msgBuf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    va_end(args);

    std::ostringstream final;
    final << "[" << timestamp() << "] " << file << ":" << line << " - " << msgBuf;

    TraceLog(level, "%s", final.str().c_str());
}

} // namespace log

#if defined(_WIN32)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define logt(fmt, ...) log::logMessage(LOG_TRACE, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define logd(fmt, ...) log::logMessage(LOG_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define logi(fmt, ...) log::logMessage(LOG_INFO, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define logw(fmt, ...) log::logMessage(LOG_WARNING, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define loge(fmt, ...) log::logMessage(LOG_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
