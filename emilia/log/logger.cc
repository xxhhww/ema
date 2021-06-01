#include "emilia/log/logger.h"
#include <iostream>

namespace emilia{
namespace log{

Logger::Logger(const std::string& name, LogLevel level, LogFormat::ptr format)
:m_name(name)
,m_level(level){
    if(format == nullptr){
        m_defaultFormat = LogFormat::ptr(new LogFormat("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }
    else
        m_defaultFormat = format;
}

//打印日志
void Logger::printLog(LogStream::ptr stream){
    if(stream->getLevel() >= m_level){
        //遍历所有日志输出地，如果日志输出地没有自己的日志输出格式，那么使用日志器的日志输出格式
        for(auto& i : m_appenders){
            //如果没有日志输出格式，将其日志输出格式设置为日志器的日志输出格式
            if(!i->hasFormat())
                i->setFormat(m_defaultFormat);
            i->printLog(stream);
        }
    }
}
LoggerManger::LoggerManger(){
    Logger::ptr rootLogger(new Logger("root"));
    rootLogger->addAppender(StdOutSingletonPtr);
    m_loggers.insert(std::pair<std::string, Logger::ptr>("root", rootLogger));
}

//根据name查找日志器，如果没有就新建一个日志器并将这个新键的日志器返回
Logger::ptr LoggerManger::lookUp(const std::string& name){
    base::Mutex::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end())
        return it->second;
    return nullptr;
}

Logger::ptr LoggerManger::defaultLogger(const std::string& name){
    Logger::ptr defLogger(new Logger(name));
    defLogger->addAppender(StdOutSingletonPtr);
    base::Mutex::Lock lock(m_mutex);
    m_loggers.insert(std::pair<std::string, Logger::ptr>(name, defLogger));
    return defLogger;
}

void LoggerManger::update(Logger::ptr logger){
    base::Mutex::Lock lock(m_mutex);
    //判断此日志器名称是否存在
    auto it = m_loggers.find(logger->getName());
    //如果存在就删除换新的
    if(it != m_loggers.end())
        m_loggers.erase(it);
    m_loggers.insert(std::pair<std::string, Logger::ptr>(logger->getName(), logger));
}

void LoggerManger::del(const std::string& name){
    if(name == "root" || name.empty())
        return;
    base::Mutex::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it == m_loggers.end())
        return;
    m_loggers.erase(it);
}

}
}
