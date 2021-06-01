#ifndef _EMILIA_TOAST_H_
#define _EMILIA_TOAST_H_

#include "emilia/base/fiber.h"

namespace emilia{
namespace base{

class Toast{
public:
    //事件类型
    enum EVType{
        EV_NONE = 0x00,
        //可读事件(EPOLLIN)
        EV_READ = 0x01,
        //可写事件(EPOLLOUT)
        EV_WRITE = 0x04,
    };

    Toast(int fd):m_fd(fd),m_events(EV_NONE){}

    //设置Toast关注的读事件，事件对应的动作，事件的超时情况
    //并更新事件循环的注册情况
    void addRDEvent(Fiber::ptr fiber, uint64_t timeOut = 0);
    //设置Toast关注的写事件，事件对应的动作，事件的超时情况
    //并更新事件循环的注册情况
    void addWREvent(Fiber::ptr fiber);

    //撤销Toast关注的读事件，并更新事件循环
    void delRDEvent();
    //撤销Toast关注的写事件，并更新事件循环
    void delWREvent();

    //get方法
    int getFd() const { return m_fd; }
    uint64_t getRMS() const { return m_rMs; }
    EVType getEVType() const { return m_events; }
    Fiber::ptr getReadCb() const { return m_readFiber; }
    Fiber::ptr getWriteCb() const { return m_writeFiber; }

private:
    //关注的句柄
    int m_fd;
    //关注的事件
    EVType m_events;
    //事件对应的操作函数(事件触发调用对应的函数)
    //可读事件触发时启动
    Fiber::ptr m_readFiber;
    //可写事件触发时启动
    Fiber::ptr m_writeFiber;

    //读超时
    uint64_t m_rMs;
};

class ASyncCtx{
public:
    using ptr = std::shared_ptr<ASyncCtx>;
private:
    //异步事件是否已经超时
    bool m_isDelay;
    //异步事件触发时调用的协程
    Fiber::ptr m_fiber;
};

}
}


#endif