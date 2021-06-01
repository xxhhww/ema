#include "emilia/base/scheduler.h"
#include "emilia/base/thread.h"
#include "emilia/util/macroutil.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

namespace emilia{
namespace base{

static thread_local Scheduler* e_scheduler_this = nullptr;

Scheduler::Scheduler()
:m_epollFd(epoll_create(50))
,m_pendCount(0)
,m_stop(false)
,m_needTickle(false){
    e_scheduler_this = this;

    int rt = pipe(m_ticklepipe);
    EMILIA_ASSERT_LOG(!rt, "Scheduler Construct Fail: pipe Error");

    //向epollFd注册管道[0]的可读事件
    epoll_event ep_event;
    ep_event.events = EPOLLIN;
    ep_event.data.fd = m_ticklepipe[0];

    rt = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_ticklepipe[0], &ep_event);
    EMILIA_ASSERT_LOG(!rt, "Scheduler Construct Fail: epoll_ctl Error");
}

Scheduler::~Scheduler(){
    //回收toasts
    auto it = m_toasts.begin();
    while(it != m_toasts.end()){
        Toast* toast = it->second;
        delete toast;
        m_toasts.erase(it++);
    }
}

//线程运行的函数
void Scheduler::run(){
    //创建主协程(调度协程)
    Fiber::CreateMainFiber();
    //创建事件循环协程和设置线程局部变量-线程的事件循环协程
    Fiber::ptr idleFiber(new Fiber(std::bind(&Scheduler::idleLoop, this)));
    Thread::SetIdleFiber(idleFiber);
    bool isOut = false;

    while(true){
        //处理触发事件对应的协程
        for(auto& i : m_readyIOFibers){
            i->swapIn();
        }
        m_readyIOFibers.clear();

        {
            //加锁(因为m_assignments可以由本线程访问，也可以由监听线程访问)
            Mutex::Lock lock(m_mutex);
            m_needTickle = true;
            Fiber::ptr temp = nullptr;
            for(auto& t : m_assignments){
                temp = Fiber::ptr(new Fiber(t));
                temp->swapIn();
            }
            m_assignments.clear();
        }
        //m_readyIOFibers处理完毕，主协程切换到事件循环协程
        idleFiber->swapIn();
        //直接返回，析构函数中会释放资源
        {
            Mutex::Lock lock(m_mutex);
            if(m_stop)
                isOut = true;
            else
                isOut = false;
        }

        //执行善后工作
        if(isOut){
            return;
        //让所有的等待事件都强制触发
        }
    }
}

//被动关闭，监听线程使用，用于通知其他SubScheduler现在该停止了
void Scheduler::passiveClose(){
    //加锁
    Mutex::Lock lock(m_mutex);
    m_stop = true;
    //通知子线程
    tickle();
}

//向事件循环注册可读事件
void Scheduler::addReadEvent(int fd, Fiber::ptr fiber, uint64_t ms){
    //新加入的fd
    if(m_toasts.find(fd) == m_toasts.end()){
        //new了注意delete
        Toast* toast = new Toast(fd);
        //设置toast并且注册到事件循环上
        toast->addRDEvent(fiber, ms);
        m_toasts[fd] = toast;
    }
    //已经加入到toast中了
    else{
        Toast* toast = m_toasts[fd];
        toast->addRDEvent(fiber);
    }
}

//向事件循环注册可写事件
void Scheduler::addWriteEvent(int fd, Fiber::ptr fiber){
    //新加入的fd
    if(m_toasts.find(fd) == m_toasts.end()){
        //new了注意delete
        Toast* toast = new Toast(fd);
        //设置toast并且注册到事件循环上
        toast->addWREvent(fiber);
        m_toasts[fd] = toast;
    }
    //已经加入到toast中了
    else{
        Toast* toast = m_toasts[fd];
        toast->addWREvent(fiber);
    }
}

//强制触发fd上的可读事件
void Scheduler::burnReadEvent(int fd){
    Toast* toast = m_toasts[fd];
    m_readyIOFibers.push_back(toast->getReadCb());
    toast->delRDEvent();
}

//强制触发fd上的可写事件
void Scheduler::burnWriteEvent(int fd){
    Toast* toast = m_toasts[fd];
    m_readyIOFibers.push_back(toast->getWriteCb());
    toast->delWREvent();
}

//或者监听线程调用，或者本线程自己调用
void Scheduler::assign(std::function<void()> cb){
    //本线程自己调用此函数的时机：
    //1处于主协程遍历m_readyIOFibers中
    //2处于事件循环协程处理定时任务时
    //在这两种情况下，都不需要加锁

    //监听线程调用此函数时涉及到共享容器m_assignment
    //因此要加锁
    if(Thread::GetId() == m_threadId){
        m_assignments.push_back(cb);
    }
    else{
        Mutex::Lock lock(m_mutex);
        m_assignments.push_back(cb);
        if(m_needTickle)
            tickle();
        m_needTickle = false;
    }
}

Scheduler* Scheduler::GetThis(){
    return e_scheduler_this;
}

void Scheduler::tickle(){
    const char* temp = "T";
    //向管道写数据
    ssize_t n = write(m_ticklepipe[1], temp, 1);
    EMILIA_ASSERT_LOG(n == 1, "Scheduler Tickle Fail: write Error")
}

//真正的事件循环
void Scheduler::idleLoop(){
    while(true){
        epoll_event ep_events[64];
        //距离下一个定时器触发所剩的时间
        uint64_t nextTimeOut = getNextTime();

        int rt = epoll_wait(m_epollFd, ep_events, 64, nextTimeOut);
        //如果出错而且不是系统中断
        if(rt < 0 && errno != EINTR){
            EMILIA_ASSERT_LOG(false, "epoll_wait fail")
        }
        //出错是系统中断就重新执行epoll_wait
        else if(rt < 0 && errno == EINTR){
            continue;
        }
        //超时，触发定时器
        else if(rt == 0){
            std::vector<std::function<void()> > cbs;
            listOutTimer(cbs);
            //Fiber::ptr temp = nullptr;
            for(auto& i : cbs){
                //因为只能由主协程进行协程的切换，因此在事件循环协程里这样写是错的
                //temp = Fiber::ptr(new Fiber(i));
                //temp->swapIn();
                i();
            }
            //回到主协程
            Fiber::YieldToPend();
            continue;
        }
        //rt > 0，并且触发的事件都装载到ep_events上了
        else{
            for(int i = 0; i < rt; i ++){
                //被其他线程(监听线程)唤醒
                if(ep_events[i].data.fd == m_ticklepipe[0]){
                    char temp;
                    ssize_t n = read(m_ticklepipe[0], &temp, 1);
                    EMILIA_ASSERT_LOG(n == 1 , "Idle Fiber Fail: Read From Pipe Error")
                }
                //标准输入可读
                else if(ep_events[i].data.fd == STDIN_FILENO){
                    m_stop = true;
                    Fiber::YieldToPend();
                    //不过一般是回不来了
                    continue;
                }
                //事件触发
                else{
                    //可读事件触发
                    if( ep_events[i].events & EPOLLIN ){
                        Toast* temp = (Toast*)ep_events[i].data.ptr;
                        //取出读事件对应的操作函数
                        m_readyIOFibers.push_back(temp->getReadCb());
                        //更新Toast和事件循环
                        temp->delRDEvent();
                    }
                    if( ep_events[i].events & EPOLLOUT ){
                        Toast* temp = (Toast*)ep_events[i].data.ptr;
                        //取出写事件对应的操作函数
                        m_readyIOFibers.push_back(temp->getWriteCb());
                        //更新Toast和事件循环
                        temp->delWREvent();
                    }
                }
            }
            //返回主协程
            Fiber::YieldToPend();
            continue;
        }
    }
}

void Scheduler::updateEpoll(int op, Toast* toast){
    epoll_event ep_event;
    bzero(&ep_event, sizeof(ep_event));
    ep_event.events = toast->getEVType();
    ep_event.data.ptr = static_cast<void*>(toast);
    int fd = toast->getFd();

    EMILIA_ASSERT_LOG(!epoll_ctl(m_epollFd, op, fd, &ep_event), "UpdateEpoll Fail: epoll_ctl Error")
}

void Scheduler::eraseToast(int fd){
    //m_toasts中没有此fd
    if(m_toasts.find(fd) == m_toasts.end()){
        EMILIA_LOG_INFO("system") << "EraseToast Info: Fd Not Exist";
    }
    else{
        Toast* toast = m_toasts[fd];
        //释放内存资源
        delete toast;
        //在map中删除此节点
        m_toasts.erase(fd);
        EMILIA_LOG_INFO("system") << "EraseToast Info: Fd Has Erase";
    }
}

void Scheduler::insertTimerAtFront(){
    //通知处于idle状态中的线程
    if(Thread::GetId() != m_threadId)
        tickle();
}

//如果是处理监听套接字的Scheduler，开放本地客户端功能
//管理员可以通过本地客户端处理整个系统
void Scheduler::initMainScheduler(){
    //向epollfd上注册标准输入的可读事件
    epoll_event ep_event;
    bzero(&ep_event, sizeof(ep_event));
    ep_event.events = EPOLLIN;

    EMILIA_ASSERT_LOG(!epoll_ctl(m_epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &ep_event), "UpdateEpoll Fail: epoll_ctl Error")
}

}
}