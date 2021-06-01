#include "emilia/log/logstream.h"

namespace emilia{
namespace log{

std::string LevelToString(LogLevel level){
#define XX(type, str) \
    case type : \
        return str; \

    switch(level){
        XX(LogLevel::UNKNOWN, "UNKNOWN")
        XX(LogLevel::DEBUG, "DEBUG")
        XX(LogLevel::INFO, "INFO")
        XX(LogLevel::WARN, "WARN")
        XX(LogLevel::ERROR, "ERROR")
        XX(LogLevel::FATAL, "FATAL")
    }
    return "UNKNOWN";
#undef XX
}

LogLevel StringToLevel(const std::string& cppstr){
#define XX(type, str) \
    if(cppstr == str) \
        return type; \

    XX(LogLevel::DEBUG, "debug")
    XX(LogLevel::INFO, "info")
    XX(LogLevel::WARN, "warn")
    XX(LogLevel::ERROR, "error")
    XX(LogLevel::FATAL, "fatal")
    return LogLevel::UNKNOWN;
#undef XX
}

LogStream::LogStream(const std::string& loggerName
            ,const std::string& file
            ,uint32_t line, uint64_t elapse
            ,uint32_t threadId
            ,std::string threadName
            ,uint64_t time
            ,LogLevel level)
:m_loggerName(loggerName)
,m_file(file)
,m_line(line)
,m_elapse(elapse)
,m_threadId(threadId)
,m_threadName(threadName)
,m_time(time)
,m_level(level)
{}

}
}
