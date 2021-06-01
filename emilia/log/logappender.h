#ifndef _EMILIA_LOGAPPENDER_H_
#define _EMILIA_LOGAPPENDER_H_

#include "emilia/log/logformat.h"
#include "emilia/log/logstream.h"
#include "emilia/base/mutex.h"

#include <memory>
#include <fstream>

namespace emilia{
namespace log{

//!关于日志模块的配置信息
/*
logs:
  - name: root
    level: info
    formatter: "%d%T%m%n"
    appenders:
      - type: file
        file: log.txt
        formatter: "%d%T%m%n%T%T"
      - type: StdoutLogAppender
  - name: system
    level: debug
    formatter: "%d%T%m%n"
    appenders:
      - type: file
        file: system.txt
      - type: stdout
*/
//!包含两个日志器：root、system
//!关于root
//!等级：info
//!拥有两个日志输出地(file&stdout)

//!关于system
//!等级：debug
//!拥有两个日志输出地(file&stdout)


//日志输出地
class LogAppender{
public:
    using ptr = std::shared_ptr<LogAppender>;

    LogAppender(){}
    virtual ~LogAppender(){}

    virtual const LogFormat::ptr getFormat() const { return m_format; }
    virtual void setFormat(LogFormat::ptr format) { m_format = format; }

    virtual bool hasFormat() { return (m_format != nullptr) ? true : false; }

    virtual void printLog(LogStream::ptr stream) = 0;
protected:
    LogFormat::ptr m_format;
};

//输出到文件
class FileAppender : public LogAppender{
public:
    using ptr = std::shared_ptr<FileAppender>;

    FileAppender(const std::string& fileName);
    virtual void printLog(LogStream::ptr stream) override;
    void open();
private:
    std::string m_fileName;
    std::ofstream m_stream;

};

//输出到屏幕(标准输出)
class StdOutAppender : public LogAppender{
public:
    using ptr = std::shared_ptr<StdOutAppender>;

    virtual void printLog(LogStream::ptr stream) override;
private:
    base::Mutex m_mutex;
};

//!标准输出地是单例
//!所有的日志器，凡是要输出到标准输出，都共享StdOutSingletonPtr，这样多线程下不会混乱
static LogAppender::ptr StdOutSingletonPtr(new StdOutAppender);
}
}

#endif
