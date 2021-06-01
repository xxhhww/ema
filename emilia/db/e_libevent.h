#ifndef _EMILIA_E_LIBEVENT_H_
#define _EMILIA_E_LIBEVENT_H_

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <memory>
#include <thread>
#include "mutex.h"

namespace emilia{


class Event2Loop
{
public:
    using ptr = std::shared_ptr<Event2Loop>;
    enum Status{
        INIT = 0x01,
        READY = 0x02,
        RUNNING = 0x03,
        END = 0x04,
        ERROR = 0x05
    };
    Event2Loop(const std::string& name);
    ~Event2Loop();

    void init();
    void loopStart();
    void loopStop();

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    event_base* getEventBase() const { return m_base; }
    const std::thread::id& getThreadId() const { return m_thread->get_id(); }
private:
    static void StopCb(evutil_socket_t sock, short which, void* args);
    void threadRun();
private:
    std::string m_name;
    //事件循环框架
    event_base* m_base;
    //事件循环所处的线程
    std::thread* m_thread;
    //运行状态
    Status m_status;
    //用于停止循环运行
    int m_read;
    //
    int m_write;

    Semaphore m_isOut;
};

}

#endif