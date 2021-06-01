#ifndef _EMILIA_LOGGER_H_
#define _EMILIA_LOGGER_H_

#include "emilia/log/logstream.h"
#include "emilia/log/logformat.h"
#include "emilia/log/logappender.h"
#include "emilia/base/mutex.h"

#include <vector>
#include <string>

namespace emilia{
namespace log{

//!关于日志器

//!一个日志器，有多个日志输出地(LogAppender)
//!而每个日志输出地，都有其对应的日志输出格式(LogFormat)
//!如果一个日志输出地没有规定日志输出格式，那么将会使用日志器的默认日志输出格式

//!每个日志器都有其对应的日志等级(LogLevel)，日志器只会输出等级高于自身日志器等级的日志

//!每个日志器的名称都是唯一的

//!每个日志器所使用的日志输出地，如果是文件输出地，那么它必须是唯一的，
//!也就是说，一个日志文件由一个文件输出地管理。而一个文件输出地由一个日志器管理

//!然而，对于标准输出地，可能被多个日志器管理，因此要加锁

//!关于日志模块
//!工作线程生成对应的日志信息，其中包括：logStream以及输出此logStram的日志器名称
//!之后，工作线程将此日志信息放入全局唯一的日志信息队列中，然后通知休眠中的日志线程开始工作
//!日志线程读取队列中的日志信息，获得日志信息的日志器名称
//!之后，日志线程从日志器管理对象(LoggerManger)中获得对应的此名称的日志器
//!最后，日志线程调用日志器的打印日志的方法(printLog)，将日志输出到日志器的日志输出地

//!关于跨网络日志
//!日志线程监听对应socket

//日志器，负责日志的输出
class Logger{
public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const std::string& name, LogLevel level = LogLevel::DEBUG, LogFormat::ptr format = nullptr);

    //get方法
    const std::string& getName() const { return m_name; }
    LogLevel getLevel() const { return m_level; }
    const LogFormat::ptr getFormat() const { return m_defaultFormat; }
    const std::vector<LogAppender::ptr>& getAppenders() const { return m_appenders; }

    //set方法(name不可set)
    void setLevel(LogLevel level) { m_level = level; }
    void setFormat(const LogFormat::ptr fmt) { m_defaultFormat = fmt; }

    void addAppender(LogAppender::ptr appender) { m_appenders.push_back(appender); }
    void delAppender(LogAppender::ptr appender) {
        for(auto it = m_appenders.begin(); it != m_appenders.end(); it++){
            if((*it) == appender){
                m_appenders.erase(it);
                break;
            }
        }
    }
    void clearAppenders(){ m_appenders.clear(); }

    //打印日志
    void printLog(LogStream::ptr stream);
private:
    //日志器的名称
    std::string m_name;
    //日志器等级
    LogLevel m_level;
    //默认日志输出格式
    LogFormat::ptr m_defaultFormat;
    //日志器对应的日志输出地点
    std::vector<LogAppender::ptr> m_appenders;
};

//日志器管理员，负责管理所有的日志器
class LoggerManger{
public:
    //构造函数，向m_loggers中增加root日志器
    LoggerManger();
    //根据name查找日志器，如果没有就返回nullptr
    Logger::ptr lookUp(const std::string& name);
    //默认化一个日志器(只使用标准格式输出到标准输出)
    Logger::ptr defaultLogger(const std::string& name);
    //删除日志器
    void del(const std::string& name);
    //更新日志器(如果日志器名称不存在则调用addLogger函数增加新的日志器，否则修改旧的日志器)
    void update(Logger::ptr logger);
private:
    //日志器名称与日志器的映射(一个名称只对应一个日志器)
    std::map<std::string, Logger::ptr> m_loggers;
    //锁
    base::Mutex m_mutex;
};

//只有一个单列
using LoggerMgr = singleton<LoggerManger>;

}
}

#endif
