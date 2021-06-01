#include "sslsocket.h"
#include "emilia/base/scheduler.h"
#include "emilia/log/logmarco.h"
#include <sys/fcntl.h>

using namespace emilia::base;

namespace emilia{
namespace net{

struct _SSLInit {
    _SSLInit() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
};

 //初始化
static _SSLInit s_init;

Socket::ptr SSLSocket::CreateAcceptV4(IPAddress::ptr serveAddr){
    SSLSocket::ptr acceptSock(new SSLSocket(AF_INET, SOCK_STREAM, 0));
    if(!acceptSock->initSocket())
        return nullptr;
    acceptSock->setNoBlock();
    if(!acceptSock->bind(serveAddr))
        return nullptr;
    if(!acceptSock->listen())
        return nullptr;
    return std::dynamic_pointer_cast<Socket>(acceptSock);
}

Socket::ptr SSLSocket::CreateAcceptV6(IPAddress::ptr serveAddr){
    SSLSocket::ptr acceptSock(new SSLSocket(AF_INET6, SOCK_STREAM, 0));
    if(!acceptSock->initSocket())
        return nullptr;
    acceptSock->setNoBlock();
    if(!acceptSock->bind(serveAddr))
        return nullptr;
    if(!acceptSock->listen())
        return nullptr;
    return std::dynamic_pointer_cast<Socket>(acceptSock);
}

Socket::ptr SSLSocket::CreateTcpV4(){
    SSLSocket::ptr clientSock(new SSLSocket(AF_INET, SOCK_STREAM, 0));
    if(!clientSock->initSocket())
        return nullptr;
    return std::dynamic_pointer_cast<Socket>(clientSock);
}

Socket::ptr SSLSocket::CreateTcpV6(){
    SSLSocket::ptr clientSock(new SSLSocket(AF_INET6, SOCK_STREAM, 0));
    if(!clientSock->initSocket())
        return nullptr;
    return std::dynamic_pointer_cast<Socket>(clientSock);
}


SSLSocket::SSLSocket(int family, int type, int protocol)
:Socket(family, type, protocol){}

//接受字节数组数据(Tcp)
//0客户端关闭连接 -1系统错误 -2读超时 >0 接收的字节
int SSLSocket::recv(void* buffer, size_t length){
    //如果可读超时不是0就向Scheduler中加入定时器
    Timer::ptr timer = nullptr;
    if(m_rdTimeOut){
        timer = Scheduler::GetThis()->addTimer(m_rdTimeOut, std::bind(&SSLSocket::TimeOutCb, this, 1));
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
    int rt = ::SSL_read(m_ssl.get(), buffer, length);
    //读事件被系统中断，重新读
    while(rt == -1 && errno == EINTR){
        rt = ::SSL_read(m_ssl.get(), buffer, length);
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

//发送字节数组数据(Tcp)
//0客户端关闭连接 -1系统错误 -2写超时 >0 发送的字节
int SSLSocket::send(const void* buffer, size_t length){
    Timer::ptr timer = nullptr;

retry:
    if(m_hasDelay){
        return -2;
    }

    int rt = ::SSL_write(m_ssl.get(), buffer, length);
    //发送操作被系统中断
    while(rt == -1 && errno == EINTR){
        //继续尝试发送
        rt = ::SSL_write(m_ssl.get(), buffer, length);
    }
    //重新尝试
    if(rt == -1 && errno == EAGAIN)  {
        if(m_wrTimeOut)
            timer = Scheduler::GetThis()->addTimer(m_wrTimeOut, std::bind(&SSLSocket::TimeOutCb, this, 2));
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

//连接远端地址
bool SSLSocket::connect(const IPAddress::ptr reAddr, uint64_t timeout){
    EMILIA_LOG_DEBUG("test") << "SSL connect";
    bool v = Socket::connect(reAddr, timeout);
    if(v) {
        m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
        m_ssl.reset(SSL_new(m_ctx.get()),  SSL_free);
        SSL_set_fd(m_ssl.get(), m_sock);
        v = (SSL_connect(m_ssl.get()) == 1);
    }
    return v;
}

//获得连接套接字
Socket::ptr SSLSocket::accept(){
    Scheduler::GetThis()->addReadEvent(m_sock, Fiber::GetThis(), 0);
    Fiber::YieldToPend();

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

    SSLSocket::ptr connSocket(new SSLSocket(m_family, m_type, m_protocol));
    //设置为非阻塞
    int flags = fcntl(rt, F_GETFL, 0);
    fcntl(rt, F_SETFL, flags | O_NONBLOCK);

    connSocket->m_ssl.reset(SSL_new(connSocket->m_ctx.get()),  SSL_free);
    SSL_set_fd(connSocket->m_ssl.get(), rt);
    SSL_accept(connSocket->m_ssl.get());

    //设置connSocket的成员变量
    connSocket->setConnFd(rt);
    connSocket->setLoAddr(m_localAddr);
    connSocket->setReAddr(reAddr);
    connSocket->setRDTimeOut(m_rdTimeOut);
    connSocket->setWRTimeOut(m_wrTimeOut);

    return std::dynamic_pointer_cast<Socket>(connSocket);
}

}
}