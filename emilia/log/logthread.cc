#include "emilia/log/logthread.h"
#include "emilia/log/logger.h"
#include <cassert>

namespace emilia{
namespace log{

LogThread::LogThread()
:m_stop(false){
    m_thread = new pthread_t;
    //启动线程
    if(pthread_create(m_thread, nullptr, &LogThread::Run, this) != 0){
        delete m_thread;
        assert(false);
    }
    //设置为脱离线程
    /*
    if(pthread_detach(*m_thread)){
        delete m_thread;
        assert(false);
    }
    */
}

LogThread::~LogThread(){
    m_stop = true;
    //通知结束
    m_sem.post();
    if(pthread_join(*m_thread, nullptr)){
        assert(false);
    }

    delete m_thread;
}

void LogThread::addLogStream(LogStream::ptr stream){
    base::Mutex lock(m_mutex);
    m_logs.push_back(stream);
    //通知
    m_sem.post();
}

void* LogThread::Run(void* argv){
    LogThread* thread = static_cast<LogThread*>(argv);
    std::deque<LogStream::ptr> logDeque;
    while(!(thread->m_stop)){
        //等待任务
        thread->m_sem.wait();
        base::Mutex lock(thread->m_mutex);
        logDeque.swap(thread->m_logs);
        thread->m_logs.clear();
        lock.unlock();
        for(auto& i : logDeque){
            LoggerMgr::GetInstance()->lookUp(i->getLoggerName())->printLog(i);
        }
        logDeque.clear();
    }
    std::cout << "结束" << std::endl;
    return nullptr;
}


}
}