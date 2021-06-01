#include "emilia/base/fiber.h"
#include "emilia/base/thread.h"
#include "emilia/util/macroutil.h"
#include "emilia/config.h"

namespace emilia{
namespace base{

//协程的栈大小配置(1024*1024)
ConfigVar<uint32_t>::ptr g_fiber_stack_size =
Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024, "fiber_stack_size");

//栈空间分配类
class StackAllocator{
public:
    //分配栈空间
    static void* Alloc(size_t size){
        return malloc(size);
    }
    //回收栈空间
    static void Dealloc(void* vp, size_t size){
        return free(vp);
    }
};

//线程中主协程的初始化
Fiber::Fiber(){
    m_state = EXEC;

    //获得当前上下文信息，并保存至m_ctx(getcontext成功返回0)
    EMILIA_ASSERT_LOG(!getcontext(&m_ctx), "MainFiber Construct Fail: getcontext Error");

    //获得主协程id
    m_id = Thread::IncFiber();
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
:m_id(Thread::IncFiber())
,m_cb(cb){

    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    //通过malloc分配栈空间
    m_stack = StackAllocator::Alloc(m_stacksize);
    //将当前的寄存器信息保存到变量m_ctx中
    EMILIA_ASSERT_LOG(!getcontext(&m_ctx), "SubFiber Construct Fail: getcontext Error");
    //此上下文的父亲上下文
    //即，当此上下文运行完毕后，控制权返回至父亲上下文
    m_ctx.uc_link = &Thread::GetRunFiber()->m_ctx;
    //指定上下文的栈空间
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_stack.ss_flags = 0;
    //指定此上下文执行Run函数
    makecontext(&m_ctx, &Fiber::Run, 0);
    m_state = INIT;
}

Fiber::~Fiber(){
    Thread::DesFiber();
    //如果有栈未释放，是非主协程，要对栈进行回收
    if(m_stack){
        EMILIA_ASSERT_LOG(m_state == TERM || m_state == INIT, "SubFiber Deconstruct Fail: State Mismatch");
        StackAllocator::Dealloc(m_stack, m_stacksize);
    }
    //主协程的析构
    else{
        //主协程的cb为nullptr
        EMILIA_ASSERT_LOG(!m_cb, "Cb Not Empty");
        EMILIA_ASSERT_LOG(m_state == EXEC, "MainFiber Deconstruct Fail: State Mismatch");

        Thread::SetRunFiber(nullptr);
        Thread::SetMainFiber(nullptr);
    }
}

//重置协程，为协程设置新的任务函数，避免释放栈空间后再分配的效率损失
void Fiber::reset(std::function<void()> cb){
    //只有拥有栈空间才能reset
    EMILIA_ASSERT_LOG(m_stack, "SubFiber Reset Fail: Stack Empty");
    
    EMILIA_ASSERT_LOG(m_state == INIT || m_state == TERM, "Fiber Reset Fail: State Mismatch");
    m_cb.swap(cb);

    EMILIA_ASSERT_LOG(!getcontext(&m_ctx), "SubFiber Reset Fail: getcontext Error");

    m_ctx.uc_link = &Thread::GetRunFiber()->m_ctx;
    
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_stack.ss_flags = 0;

    makecontext(&m_ctx, &Fiber::Run, 0);

    m_state = INIT;
}

//获得执行权
void Fiber::swapIn(){
    EMILIA_ASSERT_LOG(m_state != EXEC, "Fiber SwapIn Fail: State Mismatch")
    //将协程设置为线程当前执行协程
    Thread::SetRunFiber(shared_from_this());
    //切换(协程切换都是由主协程调度的，因此这里是GetMainFiber())
    m_state = Fiber::STATE::EXEC;
    EMILIA_ASSERT_LOG(!swapcontext(&Thread::GetMainFiber()->m_ctx, &m_ctx), "Fiber SwapIn Fail: swapcontext Error")
}

//释放执行权
void Fiber::swapOut(){
    //设置协程状态为挂起状态
    m_state = Fiber::STATE::PEND;
    //设置当前执行的协程为主协程(切出和切入都是由主协程控制)
    Thread::SetRunFiber(Thread::GetMainFiber());
    EMILIA_ASSERT_LOG(!swapcontext(&m_ctx, &Thread::GetMainFiber()->m_ctx), "Fiber SwapOut Fail: swapcontext Error")
}

//在当前线程中创建出主协程
void Fiber::CreateMainFiber(){
    //确保当前线程中没有主协程
    EMILIA_ASSERT_LOG((Thread::GetMainFiber() == nullptr), "CreateMainFiber Fail: MainFiber Exists")
    //创建主协程
    Fiber::ptr mainFiber(new Fiber());
    //设置线程的主协程和当前运行协程
    Thread::SetMainFiber(mainFiber);
    Thread::SetRunFiber(mainFiber);
}

//获得当前运行协程的id
fid_t Fiber::GetId(){
    return Thread::GetRunFiber()->getId();
}

//获得当前协程
Fiber::ptr Fiber::GetThis(){
    return Thread::GetRunFiber();
}

//协程切换到后台，并设置Pend状态
void Fiber::YieldToPend(){
    //获得当前正在执行的协程
    Fiber::ptr runFiber = Thread::GetRunFiber();
    //释放当前协程的执行权，返回给主协程
    runFiber->swapOut();
}

//调用协程类中的存放的函数
void Fiber::Run(){
    //获得当前执行协程
    Fiber::ptr runFiber = Thread::GetRunFiber();
    EMILIA_ASSERT_LOG((runFiber != nullptr), "Fiber Run Fail: Run Fiber Empty");
    //执行cb
    runFiber->m_cb();
    //协程执行完毕
    runFiber->m_cb = nullptr;
    runFiber->m_state = TERM;
    //将控制权送回主协程
    Thread::SetRunFiber(Thread::GetMainFiber());
}


} 
}
