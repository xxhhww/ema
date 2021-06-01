#include "emilia/log/logappender.h"
#include <iostream>

namespace emilia{
namespace log{

FileAppender::FileAppender(const std::string& fileName)
:m_fileName(fileName){}

void FileAppender::printLog(LogStream::ptr stream){
    //重新打开文件，然后调用日志输出格式的输出方法outPut()
    open();
    m_format->outPut(m_stream, stream);
}

void FileAppender::open(){
    //如果文件已经打开，则重新打开
    if(m_stream.is_open())
        m_stream.close();
    m_stream.open(m_fileName, std::ios::app);
}

void StdOutAppender::printLog(LogStream::ptr stream){
    base::Mutex lock(m_mutex);
    m_format->outPut(std::cout, stream);
}

}
}
