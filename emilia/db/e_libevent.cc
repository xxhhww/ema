#include "e_libevent.h"
#include <unistd.h>

namespace emilia{
Event2Loop::Event2Loop(const std::string& name)
:m_name(name)
,m_base(nullptr)
,m_thread(nullptr)
,m_status(Event2Loop::Status::INIT)
,m_read(-1)
,m_write(-1)
{}

Event2Loop::~Event2Loop()
{
    m_status = Event2Loop::Status::END;
    if(m_read)
        ::close(m_read);
    if(m_write)
        ::close(m_write);
    if(m_thread)
        delete m_thread;
    if(m_base)
        event_base_free(m_base);
}

void Event2Loop::init(){
    m_status = Event2Loop::Status::INIT;
    m_base = event_base_new();
    //相当于管道m_read端读，m_write端写
    int fds[2];
    evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    m_read = fds[0];
    m_write = fds[1];
    //初始化和注册可读事件
    struct event* rEventForStop = event_new(m_base, m_read, EV_READ, StopCb, this);
    event_add(rEventForStop, nullptr);
    m_status = Event2Loop::Status::READY;
}

void Event2Loop::loopStart()
{
    if(m_thread == nullptr && m_status == Event2Loop::Status::READY){
        m_thread = new std::thread(std::bind(&Event2Loop::threadRun, this));
        m_status = Event2Loop::Status::RUNNING;
        return;
    }
}

void Event2Loop::loopStop()
{
    const char* stopFlag = "R";
    //向管道的写端写数据，读端会触发可读事件，调用StopCb()回调 
    if(::send(m_write, stopFlag, sizeof(stopFlag), 0) > 0){
        m_isOut.wait();
        //重新变为初始态,重新设置Event2Loop
        m_status = Event2Loop::Status::INIT;
        //线程重置
        delete m_thread;
        m_thread = nullptr;
        //管道重置
        ::close(m_read);
        ::close(m_write);
        m_read = -1;
        m_write = -1;
        //base重置
        event_base_free(m_base);
        m_base = nullptr;
    }
}

void Event2Loop::StopCb(evutil_socket_t sock, short which, void* args)
{
    Event2Loop* eLoop = static_cast<Event2Loop*>(args);
    char stopFlag;
    //从m_stop中读到了数据
    if(::recv(sock, &stopFlag, sizeof(stopFlag), 0) > 0){
        event_base_loopbreak(eLoop->m_base);
        //通知对方，线程已经退出
        eLoop->m_isOut.post();
        return;
    }
}

void Event2Loop::threadRun()
{
    pthread_setname_np(pthread_self(), m_name.c_str());
    event_base_loop(m_base, 0);
}

}