#ifndef _EMILIA_LOGSTREAM_H_
#define _EMILIA_LOGSTREAM_H_

#include <cstdint>
#include <string>
#include <sstream>
#include <memory>

namespace emilia{
namespace log{

//!实现C++流式的日志输出
//!MACRO()<<"日志输出";
//!在宏内部构造LogStreamWrap对象，调用LogStream的getContentSS()方法，实现流输入
//!在LogStreamWrap析构时，将LogStream对象发送到全局唯一日志队列中

enum class LogLevel{
    UNKNOWN = 0x00,
    DEBUG = 0x01,
    INFO = 0x02,
    WARN = 0x03,
    ERROR = 0x04,
    FATAL = 0x05
};

std::string LevelToString(LogLevel level);
LogLevel StringToLevel(const std::string& str);

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;
//日志流(日志的输出数据)
class LogStream{
public:
    using ptr = std::shared_ptr<LogStream>;

    LogStream(const std::string& loggerName
             ,const std::string& file
             ,uint32_t line, uint64_t elapse
             ,uint32_t threadId
             ,std::string threadName
             ,uint64_t time
             ,LogLevel level);

    const std::string& getLoggerName() const { return m_loggerName; }
    const std::string& getFile() const { return m_file; }
    uint32_t getLine() const { return m_line; }
    uint64_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    const std::string& getThreadName() const { return m_threadName; }
    uint64_t getTime() const { return m_time; }
    //获得内容流的内容
    std::string getContent() const { return m_content.str(); }
    //获得内容流
    std::stringstream& getContentSS() { return m_content; } 
    LogLevel getLevel() const { return m_level; }

private:
    //输出此日志的日志器的名称
    std::string m_loggerName;
    //产生日志的文件名
    std::string m_file;
    //产生日志的文件行号
    uint32_t m_line;
    //程序启动到现在的毫秒数
    uint64_t m_elapse;
    //产生日志的线程
    uint32_t m_threadId;
    //产生日志的线程名
    std::string m_threadName;
    //日志产生的时间
    uint64_t m_time;
    //日志等级
    LogLevel m_level;
    //日志内容流
    std::stringstream m_content;
};

}
}

#endif
