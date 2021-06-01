#ifndef _EMILIA_TIMER_H_
#define _EMILIA_TIMER_H_

#include "emilia/base/thread.h"
#include "emilia/base/fiber.h"
#include "emilia/base/mutex.h"
#include <vector>
#include <set>

namespace emilia{
namespace base{

class TimerManger;
class Timer : public std::enable_shared_from_this<Timer>
{
friend TimerManger;
public:
    using ptr = std::shared_ptr<Timer>;

    //ms:定时时间，
    //fiber:对应操作
    //recurring:是否循环
    //TimerMgr:所属Manger
    Timer(uint64_t ms, 
          std::function<void()> cb, 
          bool recurring, 
          TimerManger* TimerMgr);

    ~Timer(){}

    //取消定时
    bool cancel();

    //刷新定时
    bool refresh();

private:
    uint64_t m_ms;
    std::function<void()> m_cb;
    bool m_recurring;
    TimerManger* m_manger;

    uint64_t m_next;

    struct Comparator{
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };

};

class TimerManger
{
friend Timer;
public:
    TimerManger():m_threadId(Thread::GetId()){}
    virtual ~TimerManger(){}

    //添加一个定时器
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

    //获得下一个要触发的定时器的触发时间与当前系统时间之差，也就是说隔多少时间触发下一个定时器
    uint64_t getNextTime();

    //列出已经触发的定时器对应的协程（定时任务）
    void listOutTimer(std::vector<std::function<void()> >& cbs);
    bool hasTimer();

protected:
    virtual void insertTimerAtFront() = 0;

    pid_t m_threadId;

private:
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    RWMutex m_rwmutex;
};

}
}

#endif