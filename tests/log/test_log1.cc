#include "emilia/log/logstreamwrap.h"
#include "emilia/log/logstream.h"
#include "emilia/log/logmarco.h"

#include <unistd.h>

using namespace emilia::log;

int main(){
    {
        LogStreamWrap(LogStream::ptr(new LogStream("root", __FILE__, __LINE__, 0, 0, "Test", time(0), LogLevel::DEBUG))).getStrStream() << "test";
    }

    EMILIA_LOG_ROOT() << "test2";

    EMILIA_LOG_ERROR("test") << "test3";

    LogAppender::ptr fileAppender1(new FileAppender("/home/emilia/workspace/emilia/bin/testlogfile.txt"));
    Logger::ptr newLogger(new Logger("file"));
    newLogger->addAppender(fileAppender1);
    newLogger->addAppender(StdOutSingletonPtr);

    LoggerMgr::GetInstance()->update(newLogger);

    EMILIA_LOG_ERROR("file") << "test4";

    EMILIA_ALOG_ERROR("test") << "test5";

    sleep(5);
    return 0;
}