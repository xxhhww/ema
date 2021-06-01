#ifndef _EMILIA_SOCKET_H_
#define _EMILIA_SOCKET_H_

#include "emilia/net/address.h"
#include <memory>

namespace emilia{
namespace net{

class Socket{
public:
    using ptr = std::shared_ptr<Socket>;

    Socket(int family, int type, int protocol);
    virtual ~Socket() { close(); }

    //接受字节数组数据(Tcp)
    //0客户端关闭连接 -1系统错误 -2读超时 >0 接收的字节
    virtual int recv(void* buffer, size_t length);
    //发送字节数组数据(Tcp)
    //0客户端关闭连接 -1系统错误 -2写超时 >0 发送的字节
    virtual int send(const void* buffer, size_t length);

    //reAddr是数据发送地址
    virtual int recvFrom(IPAddress::ptr reAddr, void* buffer, size_t length);

    //reAddr是数据接收地址
    virtual int sendTo(const IPAddress::ptr reAddr,const void* buffer, size_t length);
    //获得连接套接字
    virtual Socket::ptr accept();

    //连接远端地址
    virtual bool connect(const IPAddress::ptr reAddr, uint64_t timeout = 0);

    //get方法
    int fd() const { return m_sock; }
    bool isDelay() { return m_hasDelay; }
    uint64_t getRDTimeOut() const { return m_rdTimeOut; }
    uint64_t getWRTimeOut() const { return m_wrTimeOut; }
    const IPAddress::ptr getLoAddr() const { return m_localAddr; }
    const IPAddress::ptr getReAddr() const { return m_remoteAddr; }

    //set方法
    void setRDTimeOut(uint64_t timeout) { m_rdTimeOut = timeout; }
    void setWRTimeOut(uint64_t timeout) { m_wrTimeOut = timeout; }
    void setLoAddr(IPAddress::ptr loAddr) { m_localAddr = loAddr; }
    void setReAddr(IPAddress::ptr reAddr) { m_remoteAddr = reAddr; }


    //生成监听套接字(地址为V4)
    static Socket::ptr CreateAcceptV4(const IPAddress::ptr serverAddr);
    //生成监听套接字(地址为V6)
    static Socket::ptr CreateAcceptV6(const IPAddress::ptr serverAddr);

    //生成客户端连接套接字(地址为v4)
    static Socket::ptr CreateTcpV4();
    //生成客户端连接套接字(地址为v6)
    static Socket::ptr CreateTcpV6();

    //生成ipv4 数据报 套接字
    static Socket::ptr CreateUDPV4(const IPAddress::ptr localAddr);
    //生成ipv6 数据报 套接字
    static Socket::ptr CreateUDPV6(const IPAddress::ptr localAddr);

protected:

    //初始化套接字(由Create函数组调用)
    bool initSocket();
    //设置为非阻塞
    void setNoBlock();

    //绑定监听套接字地址(由CreateAcceptV4V6调用)
    bool bind(const IPAddress::ptr addr);
    //开启监听队列(由CreateAcceptV4V6调用)
    bool listen(int backlog = SOMAXCONN);
    //关闭句柄(析构函数调用)
    bool close();

    //用于设置连接套接字的m_sock(由accept调用)
    void setConnFd(int fd) { m_sock = fd; }

    //套接字超时时调用此函数
    //ev == 1 可读超时
    //ev == 2 可写超时
    virtual void TimeOutCb(int ev);
protected:
    //socket句柄
    int m_sock;
    //协议簇(ipv4 / ipv6)
    int m_family;
    //描述符类型(tcp / udp)
    int m_type;
    //协议(一般为0)
    int m_protocol;
    //读超时(注意，监听套接字中的此变量是给它生成的连接套接字设定的)
    //监听套接字没有读超时，只有连接套接字有读超时，具体看代码
    uint64_t m_rdTimeOut;
    //写超时(同上)
    uint64_t m_wrTimeOut;
    //是否超时
    bool m_hasDelay;
    // 本地地址
    IPAddress::ptr m_localAddr;
    // 远端地址
    IPAddress::ptr m_remoteAddr;
};

}
}

#endif