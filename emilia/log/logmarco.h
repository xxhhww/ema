#ifndef _EMILIA_LOGMARCO_H_
#define _EMILIA_LOGMARCO_H_

#include "emilia/log/logstreamwrap.h"
#include "emilia/base/thread.h"



namespace emilia{

using namespace log;

//使用root日志器进行输出
#define EMILIA_LOG_ROOT() \
    if(1)   \
        LogStreamWrap(LogStream::ptr (new LogStream ("root", \
        __FILE__, __LINE__, 0, \
        emilia::base::Thread::GetId(), \
        emilia::base::Thread::GetName(), time(0), LogLevel::DEBUG) ) ).getStrStream()

//使用root日志器进行输出
#define EMILIA_LOG_NAME(name, level, isAsync) \
    if(1)   \
        LogStreamWrap(LogStream::ptr (new LogStream (name, \
        __FILE__, __LINE__, 0, \
        emilia::base::Thread::GetId(), \
        emilia::base::Thread::GetName(), time(0), level) ), isAsync).getStrStream()

//!日志模块对外接口(同步方式)
#define EMILIA_LOG_DEBUG(name) EMILIA_LOG_NAME(name, LogLevel::DEBUG, false)
#define EMILIA_LOG_INFO(name)  EMILIA_LOG_NAME(name, LogLevel::INFO, false)
#define EMILIA_LOG_WARN(name)  EMILIA_LOG_NAME(name, LogLevel::WARN, false)
#define EMILIA_LOG_ERROR(name) EMILIA_LOG_NAME(name, LogLevel::ERROR, false)
#define EMILIA_LOG_FATAL(name) EMILIA_LOG_NAME(name, LogLevel::FATAL, false)

//!日志模块对外接口(异步方式)
#define EMILIA_ALOG_DEBUG(name) EMILIA_LOG_NAME(name, LogLevel::DEBUG, true)
#define EMILIA_ALOG_INFO(name)  EMILIA_LOG_NAME(name, LogLevel::INFO, true)
#define EMILIA_ALOG_WARN(name)  EMILIA_LOG_NAME(name, LogLevel::WARN, true)
#define EMILIA_ALOG_ERROR(name) EMILIA_LOG_NAME(name, LogLevel::ERROR, true)
#define EMILIA_ALOG_FATAL(name) EMILIA_LOG_NAME(name, LogLevel::FATAL, true)

}

#endif