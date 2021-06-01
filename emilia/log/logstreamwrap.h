#ifndef _EMILIA_LOGSTREAMWRAP_H_
#define _EMILIA_LOGSTREAMWRAP_H_

#include "emilia/log/logstream.h"
#include "emilia/log/logger.h"
#include "emilia/log/logthread.h"

namespace emilia{
namespace log{

class LogStreamWrap{
public:
    LogStreamWrap(LogStream::ptr stream, bool isAsync = false):m_stream(stream),m_isAsync(isAsync){}
    ~LogStreamWrap(){
        if(!m_isAsync){
            //根据日志器名称找到日志器并输出
            std::cout << "test" << std::endl;
            Logger::ptr logger = LoggerMgr::GetInstance()->lookUp(m_stream->getLoggerName());
            //日志器管理类中不存在日志器名称为此的日志器
            if(logger == nullptr)
                logger = LoggerMgr::GetInstance()->defaultLogger(m_stream->getLoggerName());
            logger->printLog(m_stream);
        }
        else{
            //将日志放入日志线程的队列中
            LogThreadEntity::GetInstance()->addLogStream(m_stream);
        }
    }

    std::stringstream& getStrStream() { return m_stream->getContentSS(); }
private:
    LogStream::ptr m_stream;
    bool m_isAsync;
};



}
}

#endif
