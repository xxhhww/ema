#include "emilia/base/iomanger.h"
#include "emilia/log/logmarco.h"

namespace emilia{
namespace base{

IOManger* IOManger::Create(int threadNums){
    return new IOManger(threadNums);
}
//threadNum <= 1 单线程
//threadNum >= 2 多线程
//默认单线程
IOManger::IOManger(int threadNum)
:m_threadNums(threadNum)
,m_mainScheduler(new Scheduler)
{}

//delete时触发，将new的资源释放
IOManger::~IOManger(){
    //thread是由智能指针管理的，不用管他

    //对所有的Scheduler发送停止信息
    //scheduler收到信息之后，就会停止
    //线程就会执行delet scheduler，然后退出threadFunc
    //再之后线程会调用m_semJoin.post() 通知此时可以回收线程了
    for(auto& s : m_subSchedulers){
        s->passiveClose();
    }

    //回收线程
    for(auto& t : m_workThreads){
        t->join();
    }

    //析构函数中回收m_mainScheduler中的new的toast
    delete m_mainScheduler;
}

//先启动其他所有线程执行threadFunc，然后再启动mainScheduler;
void IOManger::run(){
    Thread::SetName("MainScheduler");
    //单线程模式不会进入for循环
    for(int i = 1; i < m_threadNums; i++){
        Thread::ptr thread(new Thread(std::bind(&IOManger::threadFunc, this), "SubScheduler" + std::to_string(i)));
        m_workThreads.push_back(thread);
        //Sub线程启动，并且有信号量确保在start()之后线程已经启动
        thread->start();
    }
    //MainScheduler启动
    m_mainScheduler->run();
}

//获得m_mainScheduler
Scheduler* IOManger::getMainScheduler(){
    return m_mainScheduler;
}

//根据句柄的值获得Scheduler
//句柄值%m_threadNums 就是对应的scheduler的位置
Scheduler* IOManger::getSchedulerByHandle(int fd){
    //单线程模式启动
    if(m_threadNums == 1){
        return m_mainScheduler;
    }
    else{
        //取模后的数
        int reNum = fd % (m_threadNums-1);
        return m_subSchedulers[reNum];
    }
}
//向m_mainScheduler指派任务
void IOManger::assignForMain(std::function<void()> func){
    m_mainScheduler->assign(func);
}

//根据句柄派送任务
void IOManger::assignByHandle(int fd, std::function<void()> func){
    //获得fd对应的Scheduler
    Scheduler* scheduler = getSchedulerByHandle(fd);
    //将fd上的任务指派给其对应的scheduler
    scheduler->assign(func);
}

//启动的线程运行的函数，启动线程的scheduler
void IOManger::threadFunc(){
    //初始化此线程的scheduler
    Scheduler* scheduler = new Scheduler;
    m_subSchedulers.push_back(scheduler);
    //启动此线程的scheduler
    scheduler->run();
    //运行到这里说明scheduler已经停止，释放资源
    delete scheduler;
}

}
}