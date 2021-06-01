#include "emilia/base/thread.h"
#include "emilia/util/macroutil.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>

namespace emilia{
namespace base{

//线程局部变量(对于每个线程来说都是不同的)

//指向当前线程的指针
static thread_local Thread* e_thread_this = nullptr;
//当前线程的名称
static thread_local std::string e_thread_name = "UnDefined";
//当前线程的id
static thread_local pid_t e_thread_id = syscall(SYS_gettid);
//当前线程正在运行的协程
static thread_local Fiber::ptr e_thread_run_fiber = nullptr;
//当前线程的主协程
static thread_local Fiber::ptr e_thread_main_fiber = nullptr;
//当前线程的事件循环协程
static thread_local Fiber::ptr e_thread_idle_fiber = nullptr;
//当前线程的协程总数
static thread_local fid_t e_thread_fiber_count = 0;
//当前线程的最大协程id号(id不会因为协程析构而减少)
static thread_local fid_t e_thread_fiber_id = 0;

//初始化线程给予对应的值
Thread::Thread(std::function<void()> cb, const std::string& name){
    if(name.empty())
        m_name = "UnDefined";
    else
        m_name = name;
    m_cb.swap(cb);
    //剩下的值需要等线程启动才可以确定
}

//线程析构
Thread::~Thread(){
    //如果没有调用过join函数，就在析构函数中调用
    if(m_pthread){
        //此处确保在回收时，线程的函数肯定执行完毕了
        m_semJoin.wait();
        //非0报错
        EMILIA_ASSERT_LOG(!pthread_join(m_pthread, nullptr), "Thread Deconstruct Fail: pthread_join Error")
        m_pthread = 0;
    }
}

//线程开始函数
void Thread::start(){
    //启动线程执行run函数
    //非0报错
    EMILIA_ASSERT_LOG(!pthread_create(&m_pthread, nullptr, &Thread::Run, this), "Thread Start Fail: pthread_create Error")
    m_semStart.wait();
}

//线程回收函数
void Thread::join(){
    //线程存在
    if(m_pthread){
        m_semJoin.wait(); //此处确保在回收时，线程的函数肯定执行完毕了
        //非0报错
        EMILIA_ASSERT_LOG(!pthread_join(m_pthread, nullptr), "Thread Join Fail: pthread_join Error")
        m_pthread = 0;
    }
}

//供m_cb内部使用

//获得当前线程
Thread* Thread::GetThis(){
    return e_thread_this;
}

//获得线程局部变量-当前线程的名称
const std::string& Thread::GetName(){
    return e_thread_name;
}

//获得线程局部变量-当前线程的id
pid_t Thread::GetId(){
    return e_thread_id;
}

//获得线程局部变量-当前线程中正在运行的协程
Fiber::ptr Thread::GetRunFiber(){
    return e_thread_run_fiber;
}
//获得线程局部变量-当前线程中的主协程
Fiber::ptr Thread::GetMainFiber(){
    return e_thread_main_fiber;
}
//获得线程局部变量-当前线程中的运行事件循环的协程
Fiber::ptr Thread::GetIdleFiber(){
    return e_thread_idle_fiber;
}

void Thread::SetName(const std::string& name){
    e_thread_name = name;
}
//设置线程局部变量-当前线程中正在运行的协程
void Thread::SetRunFiber(Fiber::ptr runFiber){
    e_thread_run_fiber = runFiber;
}
//设置线程局部变量-当前线程中的主协程
void Thread::SetMainFiber(Fiber::ptr mainFiber){
    e_thread_main_fiber = mainFiber;
}
//设置线程局部变量-当前线程中运行事件循环的协程
void Thread::SetIdleFiber(Fiber::ptr idleFiber){
    e_thread_idle_fiber = idleFiber;
}

//当前线程生产出新的协程
//返回新协程在当前线程中的序号
//协程的序号从1开始而非0 
fid_t Thread::IncFiber(){
    e_thread_fiber_count++;
    e_thread_fiber_id++;
    return e_thread_fiber_id;
}
//当前线程已有的协程析构
void Thread::DesFiber(){
    e_thread_fiber_count--;
}

//调用线程类中的存放的函数
void* Thread::Run(void* arg){
    Thread* thread = (Thread*)arg;
    
    //获得线程id
    thread->m_id = syscall(SYS_gettid);
    e_thread_this = thread;
    e_thread_name = thread->m_name;

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    //通知start函数启动成功
    thread->m_semStart.post();
    cb();

    //通知join函数线程执行完毕
    thread->m_semJoin.post();
    return nullptr;
}

}
}
