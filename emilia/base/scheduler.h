#ifndef _EMILIA_SCHEDULER_H_
#define _EMILIA_SCHEDULER_H_

#include "emilia/base/thread.h"
#include "emilia/base/fiber.h"
#include "emilia/base/mutex.h"
#include "emilia/base/toast.h"
#include "emilia/base/timer.h"

#include "emilia/net/socket.h"
#include <list>

#include <map>

namespace emilia{
namespace base{

//事件循环
class Scheduler : public TimerManger{
friend class Toast;
friend class Socket;
public:

    Scheduler();
    ~Scheduler();
    //线程运行的函数
    void run();

    //被动关闭，监听线程使用，用于通知其他SubScheduler现在该停止了
    void passiveClose();

    //向事件循环注册可读事件
    void addReadEvent(int fd, Fiber::ptr fiber, uint64_t ms);
    //向事件循环注册可写事件
    void addWriteEvent(int fd, Fiber::ptr fiber);

    //强制触发fd上的可读事件
    void burnReadEvent(int fd);
    //强制触发fd上的可写事件
    void burnWriteEvent(int fd);
    /*
    //注册异步事件
    void addASyncEvent();
    */

    //将fd对应的toast从m_toasts中删除
    void eraseToast(int fd);
    
    //派送任务，或者监听线程调用，或者本线程自己调用
    void assign(std::function<void()> cb);

    //如果是处理监听套接字的Scheduler，开放本地客户端功能
    //管理员可以通过本地客户端处理整个系统
    void initMainScheduler();

    static Scheduler* GetThis();
private:
    //用于唤醒处于idle中的线程
    void tickle();
    //真正的事件循环
    void idleLoop();
    //更新epoll上注册的事件
    void updateEpoll(int op, Toast* toast);

    //其他线程向当前线程加入了定时器，并且此定时器的剩余时间最短时触发
    virtual void insertTimerAtFront() override;
private:
    //监听句柄集
    int m_epollFd;
    //待处理的事件数
    int m_pendCount;
    //是否停止
    bool m_stop;
    //是否需要tickle
    bool m_needTickle;
    //通知管道(线程之间通信使用)
    int m_ticklepipe[2];
    //锁(线程之间通信使用)
    Mutex m_mutex;
    //存放所有toast(句柄,Toast)
    //Toast中存放因为对应句柄上的IO事件而挂起的协程
    std::map<int, Toast*> m_toasts;

    //存放新的任务（或来自监听线程，或来自线程自己）(使用assign函数存入)
    std::list<std::function<void()> > m_assignments;
    
    //存放IO事件触发后，所有对应的待处理的协程
    std::vector<Fiber::ptr> m_readyIOFibers;

    //因为其他事件而陷入等待的协程(协程id,异步事件信息)
    std::map<fid_t, ASyncCtx::ptr> m_asyncCtxs;

    //定时事件在idleLoop中进行处理
};

}
}


#endif