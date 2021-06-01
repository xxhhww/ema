#include "toast.h"
#include "emilia/base/scheduler.h"

#include <sys/epoll.h>
#include <iostream>

namespace emilia{
namespace base{

//设置Toast关注的读事件，事件对应的动作，事件的超时情况
//并更新事件循环的注册情况
void Toast::addRDEvent(Fiber::ptr fiber, uint64_t timeOut){
    m_readFiber = fiber;
    //之前没有关注过任何事件
    if(!m_events){
        m_events = (EVType)(m_events | EVType::EV_READ);
        //使用EPOLL_CTL_ADD
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_ADD, this);
    }
    else{
        m_events = (EVType)(m_events | EVType::EV_READ);
        //使用EPOLL_CTL_MOD
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_MOD, this);
    }
}

//设置Toast关注的写事件，事件对应的动作，事件的超时情况
//并更新事件循环的注册情况
void Toast::addWREvent(Fiber::ptr fiber){
    m_writeFiber = fiber;
    //之前没有关注过任何事件
    if(!m_events){
        m_events = (EVType)(m_events | EVType::EV_WRITE);
        //使用EPOLL_CTL_ADD
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_ADD, this);
    }
    else{
        m_events = (EVType)(m_events | EVType::EV_WRITE);
        //使用EPOLL_CTL_MOD
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_MOD, this);
    }
}

//撤销Toast关注的读事件，并更新事件循环
void Toast::delRDEvent(){
    m_events = (EVType)(m_events &~ EVType::EV_READ);
    m_readFiber = nullptr;
    //撤销事件后，关注的事件为空
    if(!m_events)
        //使用EPOLL_CTL_DEL
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_DEL, this);
    else
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_MOD, this);
}

//撤销Toast关注的写事件，并更新事件循环
void Toast::delWREvent(){
    m_events = (EVType)(m_events &~ EVType::EV_WRITE);
    m_writeFiber = nullptr;
    //撤销事件后，关注的事件为空
    if(!m_events)
        //使用EPOLL_CTL_DEL
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_DEL, this);
    else
        Scheduler::GetThis()->updateEpoll(EPOLL_CTL_MOD, this);
}

}
}