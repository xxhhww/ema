#include "emilia/base/timer.h"
#include "emilia/base/thread.h"
#include <sys/time.h>
#include <iostream>

namespace emilia{
namespace base{

//获得当前时间(毫秒)
uint64_t GetCurrentMS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

//获得当前时间(纳秒)
uint64_t GetCurrentNS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}


bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const{
    if( lhs->m_next < rhs->m_next )
        return true;
    else if (lhs->m_next > rhs->m_next )
        return false;
    else{
        if( lhs.get() > rhs.get() )
            return true;
        else
            return false;
    }
}

Timer::Timer(uint64_t ms, 
             std::function<void()> cb, 
             bool recurring, 
             TimerManger* TimerMgr)
:m_ms(ms)
,m_cb(cb)
,m_recurring(recurring)
,m_manger(TimerMgr){
    m_next = GetCurrentMS() + m_ms;
}

bool Timer::cancel()
{
    if(m_cb){
        auto it = m_manger->m_timers.find(shared_from_this());
        m_manger->m_timers.erase(it);
        m_cb = nullptr;
        return true;
    }
    else{
        return false;
    }
}

bool Timer::refresh()
{
    if(!m_cb){
        return false;
    }
    auto it = m_manger->m_timers.find(shared_from_this());
    if(it == m_manger->m_timers.end()){
        return false;
    }
    //在manger中删除Timer
    m_manger->m_timers.erase(it);
    //重新设置时间
    m_next = GetCurrentMS() + m_ms;
    //重新插入
    m_manger->m_timers.insert(shared_from_this());
    return true;
}


Timer::ptr TimerManger::addTimer(uint64_t ms, std::function<void()> cb, bool recurring){

    Timer::ptr timer (new Timer(ms, cb, recurring, this));
    //写锁
    RWMutex::wrLock wlock(m_rwmutex);

    auto it = m_timers.insert(timer).first;

    wlock.unlock();
    //如果插入的定时器比之前的定时器触发时间都早
    //并且是其他线程插入的
    //就通知事件循环更改定时时间
    bool atFront = (it == m_timers.begin() && Thread::GetId() != m_threadId);
    if(atFront){
        //通过tickle通知
        insertTimerAtFront();
    }
    return timer;
}

uint64_t TimerManger::getNextTime()
{
    //定时容器为空
    if(m_timers.empty()){
        return ~0ull;
    }
    Timer::ptr temp = *m_timers.begin();
    uint64_t now_ms = GetCurrentMS();
    //定时器应该触发的时间在当前时间之前
    if(temp->m_next <= now_ms){
        return 0;
    }
    //返回时间差，即过多少名触发定时器
    else{
        return (temp->m_next - now_ms);
    }
}

bool TimerManger::hasTimer(){
    //读锁
    RWMutex::rdLock rlock(m_rwmutex);
    return (!m_timers.empty());
}

//列出已经触发的定时器对应的协程（定时任务）
void TimerManger::listOutTimer(std::vector<std::function<void()> >& cbs)
{
    if(m_timers.empty()){
        return;
    }
    //获得以当前时间为基础的定时器
    Timer::ptr now_timer(new Timer(0, nullptr, false, nullptr));
    //返回第最后一个m_next比now_timer小的迭代器
    auto it = m_timers.lower_bound(now_timer);
    //将时间相等的也触发
    while(it != m_timers.end() && (*it)->m_next == now_timer->m_next){
        ++it;
    }
    std::vector<Timer::ptr> outTimers;
    outTimers.insert(outTimers.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    for(auto& timer : outTimers){
        cbs.push_back(timer->m_cb);
        //事件是循环触发事件
        if(timer->m_recurring == true){
            timer->m_next = GetCurrentMS() + timer->m_ms;
            RWMutex::wrLock wlock(m_rwmutex);
            m_timers.insert(timer);
        }
        else{
            timer->m_cb == nullptr;
        }
    }

}

}
}