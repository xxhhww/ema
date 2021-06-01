#ifndef _EMILIA_IOMANGER_H_
#define _EMILIA_IOMANGER_H_

#include "emilia/base/scheduler.h"
#include <memory>

namespace emilia{
namespace base{

class IOManger{
public:
    ~IOManger();

    //先启动其他所有线程执行threadFunc，然后再启动mainScheduler;
    void run();
    //获得m_mainScheduler
    Scheduler* getMainScheduler();
    //根据句柄的值获得Scheduler
    //句柄值%m_threadNums 就是对应的scheduler的位置
    Scheduler* getSchedulerByHandle(int fd);
    //向m_mainScheduler指派任务
    void assignForMain(std::function<void()> func);
    //根据句柄派送任务
    void assignByHandle(int fd, std::function<void()> func);

    static IOManger* Create(int threadNums = 1);

private:
    //threadNum <= 1 单线程
    //threadNum >= 2 多线程
    //默认单线程
    IOManger(int threadNum = 1);
    //启动的线程运行的函数，启动线程的scheduler
    void threadFunc();

private:
    //IOManger开启的线程数
    int m_threadNums;
    //单线程模式下
    //mainScheduler既处理监听套接字又处理连接套接字
    //多线程模式下
    //mainScheduler只处理监听套接字
    Scheduler* m_mainScheduler;
    //所有的线程对象
    std::vector<Thread::ptr> m_workThreads;
    //上述线程对象对应的Scheduler
    std::vector<Scheduler*> m_subSchedulers;
};

}
}

#endif