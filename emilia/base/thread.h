#ifndef _EMILIA_THREAD_H_
#define _EMILIA_THREAD_H_

#include "emilia/base/mutex.h"
#include "emilia/base/fiber.h"

#include "pthread.h"

#include <functional>
#include <string>
#include <memory>

namespace emilia{
namespace base{


class Thread
{
public:
    using ptr = std::shared_ptr<Thread>;

    Thread(std::function<void()> cb, const std::string& name = "");   
    ~Thread();

    //线程开始函数
    void start();
    //线程回收函数
    void join();
    //获得线程id
    const pid_t& getId() const { return m_id; }
    //获得线程名称
    const std::string& getName() const { return m_name; }

    //设置线程名称
    void setName(const std::string& name) { m_name = name; }

    //供m_cb内部使用

    //获得当前线程
    static Thread* GetThis();

    //获得线程局部变量-当前线程的名称
    static const std::string& GetName();
    //获得线程局部变量-当前线程的id
    static pid_t GetId();

    //获得线程局部变量-当前线程中正在运行的协程
    static Fiber::ptr GetRunFiber();
    //获得线程局部变量-当前线程中的主协程
    static Fiber::ptr GetMainFiber();
    //获得线程局部变量-当前线程中的运行事件循环的协程
    static Fiber::ptr GetIdleFiber();

    static void SetName(const std::string& name);

    //设置线程局部变量-当前线程中正在运行的协程
    static void SetRunFiber(Fiber::ptr runFiber);
    //设置线程局部变量-当前线程中的主协程
    static void SetMainFiber(Fiber::ptr mainFiber);
    //设置线程局部变量-当前线程中运行事件循环的协程
    static void SetIdleFiber(Fiber::ptr idleFiber);

    //当前线程生产出新的协程
    //返回此协程在当前线程中的序号
    static fid_t IncFiber();
    //当前线程已有的协程析构
    static void DesFiber();
private:
    //禁止Thread的拷贝方法
    Thread(const Thread& ) = delete;
    Thread(const Thread&& ) = delete;
    Thread& operator=(const Thread& ) = delete;

    //调用线程类中的存放的函数
    static void* Run(void* arg);
private:
    //线程id
    pid_t m_id;
    //线程标识符
    pthread_t m_pthread;
    //线程名称
    std::string m_name;
    //线程运行的函数
    std::function<void()> m_cb;
    //信号量
    Semaphore m_semStart;
    Semaphore m_semJoin;
};

}
}

#endif