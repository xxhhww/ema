#include "emilia/net/socket.h"
#include "emilia/base/fiber.h"
#include "emilia/base/scheduler.h"
#include "emilia/log/logmarco.h"
#include <sys/fcntl.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

namespace emilia{
namespace net{

using namespace base;

//生成监听套接字(地址为V4)
Socket::ptr Socket::CreateAcceptV4(const IPAddress::ptr serverAddr){
    Socket::ptr sock(new Socket(AF_INET, SOCK_STREAM, 0));
    //函数出错内部会输出错误日志，因此这里不需要再输出日志了
    //初始化socket
    if(!sock->initSocket())
        return nullptr;
    sock->setNoBlock();
    //绑定地址
    if(!sock->bind(serverAddr))
        return nullptr;
    //监听队列
    if(!sock->listen())
        return nullptr;
    return sock;
}
//生成监听套接字(地址为V6)
Socket::ptr Socket::CreateAcceptV6(const IPAddress::ptr serverAddr){
    Socket::ptr sock(new Socket(AF_INET6, SOCK_STREAM, 0));
    if(!sock->initSocket())
        return nullptr;
    sock->setNoBlock();
    if(!sock->bind(serverAddr))
        return nullptr;
    if(!sock->listen())
        return nullptr;
    return sock;
}
//生成客户端连接套接字(地址为v4)
Socket::ptr Socket::CreateTcpV4(){
    Socket::ptr clientSock(new Socket(AF_INET, SOCK_STREAM, 0));
    if(!clientSock->initSocket())
        return nullptr;
    return clientSock;
}
//生成客户端连接套接字(地址为v6)
Socket::ptr Socket::CreateTcpV6(){
    Socket::ptr clientSock(new Socket(AF_INET6, SOCK_STREAM, 0));
    if(!clientSock->initSocket())
        return nullptr;
    return clientSock;
}
//生成ipv4 数据报 套接字
Socket::ptr Socket::CreateUDPV4(const IPAddress::ptr localAddr){
    Socket::ptr sock(new Socket(AF_INET, SOCK_DGRAM, 0));
    if(!sock->initSocket())
        return nullptr;
    sock->setNoBlock();
    if(!sock->bind(localAddr))
        return nullptr;
    return sock;
}

//生成ipv6 数据报 套接字
Socket::ptr Socket::CreateUDPV6(const IPAddress::ptr localAddr){
    Socket::ptr sock(new Socket(AF_INET6, SOCK_DGRAM, 0));
    if(!sock->initSocket())
        return nullptr;
    sock->setNoBlock();
    if(!sock->bind(localAddr))
        return nullptr;
    return sock;
}

Socket::Socket(int family, int type, int protocol)
:m_sock(-1)
,m_family(family)
,m_type(type)
,m_protocol(protocol)
,m_rdTimeOut(0)
,m_wrTimeOut(0)
,m_hasDelay(false)
,m_localAddr(nullptr)
,m_remoteAddr(nullptr)
{}

//套接字超时时调用此函数
//ev == 1 可读超时
//ev == 2 可写超时
void Socket::TimeOutCb(int ev){
    //设置为已经延迟
    m_hasDelay = true;
    //从当前Scheduler中将socket中句柄对应的事件唤醒
    if(ev == 1)
        Scheduler::GetThis()->burnReadEvent(m_sock);
    else
        Scheduler::GetThis()->burnWriteEvent(m_sock);
}

//接受字节数组数据
int Socket::recv(void* buffer, size_t length){
    //如果可读超时不是0就向Scheduler中加入定时器
    Timer::ptr timer = nullptr;
    if(m_rdTimeOut){
        timer = Scheduler::GetThis()->addTimer(m_rdTimeOut, std::bind(&Socket::TimeOutCb, this, 1));
    }

    //设置可读事件
    Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), m_rdTimeOut);
    //挂起等待可读事件
    Fiber::YieldToPend();
    //被唤醒
    //判断是因为超时被唤醒还是因为可读事件触发被唤醒
retry:
    //因为超时被唤醒返回-2
    if(m_hasDelay){
        return -2;
    }
    //因为可读事件触发被唤醒
    int rt = ::recv(m_sock, buffer, length, 0);
    //读事件被系统中断，重新读
    while(rt == -1 && errno == EINTR){
        rt = ::recv(m_sock, buffer, length, 0);
    }
    //EAGAIN 重新尝试
    if(rt == -1 && errno == EAGAIN){
        //重新设置定时器
        timer->refresh();
        Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), m_rdTimeOut);
        Fiber::YieldToPend();
        goto retry;
    }
    //其他错误
    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "Recv Fail: recv Error: " << strerror(errno);
    }
    //其他错误和没有错误都要将定时器删除
    if(timer != nullptr)
        timer->cancel();

    return rt;
}

//发送字节数组数据
int Socket::send(const void* buffer, size_t length){
    Timer::ptr timer = nullptr;

retry:
    if(m_hasDelay){
        return -2;
    }

    int rt = ::send(m_sock, buffer, length, 0);
    //发送操作被系统中断
    while(rt == -1 && errno == EINTR){
        //继续尝试发送
        rt = ::send(m_sock, buffer, length, 0);
    }
    //重新尝试
    if(rt == -1 && errno == EAGAIN)  {
        if(m_wrTimeOut)
            timer = Scheduler::GetThis()->addTimer(m_wrTimeOut, std::bind(&Socket::TimeOutCb, this, 2));
        Scheduler::GetThis()->addWriteEvent(m_sock, Fiber::GetThis());
        Fiber::YieldToPend();
        goto retry;
    }

    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "Send Fail: ::send Error: " << strerror(errno);
    }

    if(timer != nullptr)
        timer->cancel();  
    return rt;
}

//从addr接收字节数组数据(Udp)
//
int Socket::recvFrom(IPAddress::ptr reAddr, void* buffer, size_t length){
    Timer::ptr timer = nullptr;
    if(m_rdTimeOut)
        timer = Scheduler::GetThis()->addTimer(m_rdTimeOut, std::bind(&Socket::TimeOutCb, this, 1));

    //设置可读事件
    Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), m_rdTimeOut);
    //挂起等待可读事件
    Fiber::YieldToPend();
    //被唤醒
    //判断是因为超时被唤醒还是因为可读事件触发被唤醒

retry:
    //因为超时被唤醒返回-2
    if(m_hasDelay){
        return -2;
    }

    //两个变量，描述客户端的ip地址
    sockaddr addr;
    socklen_t addrLen= sizeof(addr);

    //因为可读事件触发被唤醒
    int rt = ::recvfrom(m_sock, buffer, length, 0, (sockaddr*)&addr, &addrLen);
    //读事件被系统中断，重新读
    while(rt == -1 && errno == EINTR){
        rt = ::recvfrom(m_sock, buffer, length, 0, (sockaddr*)&addr, &addrLen);
    }

    //EAGAIN 重新尝试
    if(rt == -1 && errno == EAGAIN){
        //重新设置定时器
        timer->refresh();
        Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), m_rdTimeOut);
        Fiber::YieldToPend();
        goto retry;
    }

    //其他错误
    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "RecvFrom Fail: recvfrom Error: " << strerror(errno);
    }
    else
        reAddr = IPAddress::CreateBySockaddress((const sockaddr*)&addr, addrLen);

    //其他错误和没有错误都要将定时器删除
    if(timer != nullptr)
        timer->cancel();
    
    return rt;
}

//发送字节数组数据到addr(Udp)
//
int Socket::sendTo(const IPAddress::ptr reAddr,const void* buffer, size_t length){
    Timer::ptr timer = nullptr;
    if(m_wrTimeOut)
        timer = Scheduler::GetThis()->addTimer(m_rdTimeOut, std::bind(&Socket::TimeOutCb, this, 1));

    //设置可读事件
    Scheduler::GetThis()->addWriteEvent(m_sock, Fiber::GetThis());
    //挂起等待可读事件
    Fiber::YieldToPend();
    //被唤醒
    //判断是因为超时被唤醒还是因为可读事件触发被唤醒

retry:
    //因为超时被唤醒返回-2
    if(m_hasDelay){
        return -2;
    }

    //因为可读事件触发被唤醒
    int rt = ::sendto(m_sock, buffer, length, 0, reAddr->getAddr(), reAddr->getAddrLen());
    //读事件被系统中断，重新读
    while(rt == -1 && errno == EINTR){
        rt = ::sendto(m_sock, buffer, length, 0, reAddr->getAddr(), reAddr->getAddrLen());
    }

    //EAGAIN 重新尝试
    if(rt == -1 && errno == EAGAIN){
        //重新设置定时器
        timer->refresh();
        Scheduler::GetThis()->addWriteEvent(m_sock, Fiber::GetThis());
        Fiber::YieldToPend();
        goto retry;
    }

    //其他错误
    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "RecvFrom Fail: recvfrom Error: " << strerror(errno);
    }

    //其他错误和没有错误都要将定时器删除
    if(timer != nullptr)
        timer->cancel();
    
    return rt;
}

//获得连接套接字
Socket::ptr Socket::accept(){
    //向事件循环注册可读事件，并且读超时为0
    Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), 0);
    //挂起，等待可读事件
    Fiber::YieldToPend();
    //被唤醒说明有新连接

    //两个变量，描述客户端的ip地址
    sockaddr addr;
    socklen_t addrLen= sizeof(addr);

    int rt = ::accept(m_sock,(sockaddr*)&addr, &addrLen);

    //系统中断
    while(rt == -1 && errno == EINTR){
        //重新尝试
        rt = ::accept(m_sock, (sockaddr*)&addr, &addrLen);
    }
    //系统出错
    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "Accept Fail: accept Error: " << strerror(errno);
        return nullptr;
    }

    //远端地址信息
    const IPAddress::ptr reAddr = IPAddress::CreateBySockaddress((const sockaddr*)&addr, sizeof(struct sockaddr));

    Socket::ptr connSocket(new Socket(m_family, m_type, m_protocol));
    //将connFd设置为非阻塞
    connSocket->setNoBlock();
    
    //设置connSocket的成员变量
    connSocket->setConnFd(rt);
    connSocket->setLoAddr(m_localAddr);
    connSocket->setReAddr(reAddr);
    connSocket->setRDTimeOut(m_rdTimeOut);
    connSocket->setWRTimeOut(m_wrTimeOut);

    return connSocket;
}

//连接远端地址(timeout是连接超时时间)
bool Socket::connect(const IPAddress::ptr reAddr, uint64_t timeout){
    int rt = ::connect(m_sock, reAddr->getAddr(), reAddr->getAddrLen());
    //系统中断重试
    while(rt == -1 && errno == EINTR){
        rt = ::connect(m_sock, reAddr->getAddr(), reAddr->getAddrLen());
    }
    if(rt == -1){
        EMILIA_LOG_ERROR("system") << "Connect Fail: ::connect Error: " << strerror(errno);
        return false;
    }
    return true;
}

//初始化套接字
bool Socket::initSocket(){
    m_sock = ::socket(m_family, m_type, m_protocol);
    if(m_sock == -1){
        EMILIA_LOG_ERROR("system") << "InitSocket Fail: ::socket Error: " << strerror(errno);
        return false;
    }
    //设置端口可重用
    int opt = SO_REUSEADDR;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    return true;
}

//设置为非阻塞
void Socket::setNoBlock(){
    int flags = fcntl(m_sock, F_GETFL, 0);
    fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);
}

//绑定监听套接字地址
bool Socket::bind(const IPAddress::ptr addr){
    int rt = ::bind(m_sock, addr->getAddr(), addr->getAddrLen());
    //错误是因为系统中断
    while(rt == -1 && errno == EINTR){
        rt = ::bind(m_sock, addr->getAddr(), addr->getAddrLen());
    }
    if(rt != 0){
        EMILIA_LOG_ERROR("system") << "Bind Fail: ::bind Error: " << strerror(errno);
        return false;
    }
    //设置本地地址
    m_localAddr = addr;
    return true;
}

//开启监听队列
bool Socket::listen(int backlog){
    int rt = ::listen(m_sock, backlog);
    //系统中断
    while(rt == -1 && errno == EINTR){
        rt = ::listen(m_sock, backlog);
    }
    if(rt != 0){
        EMILIA_LOG_ERROR("system") << "Listen Fail: ::listen Error: " << strerror(errno);
        return false;
    }
    return true;
}

//关闭句柄
bool Socket::close(){
    //将socket对应的Toast从Scheduler中删除
    Scheduler::GetThis()->eraseToast(m_sock);
    //关闭句柄
    int rt = ::close(m_sock);
    if(rt != 0){
        EMILIA_LOG_ERROR("system") << "Close Fail: ::close Error: " << strerror(errno);
        return false;        
    }
    return true;
}
}
}