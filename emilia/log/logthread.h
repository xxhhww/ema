#ifndef _EMILIA_LOGTHREAD_H_
#define _EMILIA_LOGTHREAD_H_

#include "emilia/base/mutex.h"
#include "emilia/log/logstream.h"
#include "emilia/singleton.h"

#include <deque>
#include <vector>

namespace emilia{
namespace log{

class LogThread{
public:
    LogThread();
    ~LogThread();

    void addLogStream(LogStream::ptr stream);

private:
    static void* Run(void* argv);
private:
    //日志缓存的大小(也就是说，只有在m_logs中的日志数量大于4时才会通知日志线程进行写处理)
    static const int LOG_ONE_TIME = 4;
    pthread_t* m_thread;
    std::deque<LogStream::ptr> m_logs;
    bool m_stop;
    base::Mutex m_mutex;
    base::Semaphore m_sem;
};

using LogThreadEntity = singleton<LogThread>;

}
}

#endif