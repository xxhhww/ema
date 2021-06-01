#ifndef _EMILIA_FIBER_H_
#define _EMILIA_FIBER_H_

#include <memory>
#include <ucontext.h>

namespace emilia{
namespace base{

using fid_t = uint32_t;

class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    using ptr = std::shared_ptr<Fiber>;

    //协程状态
    enum STATE{
        //初始态
        INIT = 0x00,
        //运行态
        EXEC,
        //挂起态
        PEND,
        //终止态
        TERM,
    };

    Fiber();
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    //重置协程，为协程设置新的任务函数，避免释放栈空间后再分配的效率损失
    void reset(std::function<void()> cb);
    //获得执行权
    void swapIn();
    //释放执行权
    void swapOut();

    //获得协程状态
    Fiber::STATE getState() const { return m_state; }
    //获得协程id
    const fid_t& getId() const { return m_id; }

public:
    //在当前线程中创建出主协程
    static void CreateMainFiber();

    //获得当前运行协程的id
    static fid_t GetId();

    //获得当前协程
    static Fiber::ptr GetThis();

    //协程切换到后台，并设置Pend状态
    static void YieldToPend();

    //运行m_cb
    static void Run();
private:
    //协程状态
    STATE m_state;
    //协程id
    fid_t m_id;
    //协程上下文
    ucontext_t m_ctx;
    //协程栈
    void* m_stack = nullptr;
    //栈大小
    size_t m_stacksize;
    //协程要执行的函数，通过MainFunc()调用
    std::function<void()> m_cb;
};

}
}

#endif