#ifndef _EMILIA_SSLSOCKET_H_
#define _EMILIA_SSLSOCKET_H_

#include "emilia/net/socket.h"
#include "emilia/net/address.h"
#include <openssl/ssl.h>

namespace emilia{
namespace net{

class SSLSocket : public Socket{
public:
    using ptr = std::shared_ptr<SSLSocket>;

    SSLSocket(int family, int type, int protocol);
    //接受字节数组数据(Tcp)
    //0客户端关闭连接 -1系统错误 -2读超时 >0 接收的字节
    virtual int recv(void* buffer, size_t length) override;
    //发送字节数组数据(Tcp)
    //0客户端关闭连接 -1系统错误 -2写超时 >0 发送的字节
    virtual int send(const void* buffer, size_t length) override;
    //连接远端地址
    virtual bool connect(const IPAddress::ptr reAddr, uint64_t timeout = 0) override;
    //获得连接套接字
    virtual Socket::ptr accept() override;

    static Socket::ptr CreateAcceptV4(IPAddress::ptr serveAddr);
    static Socket::ptr CreateAcceptV6(IPAddress::ptr serveAddr);

    static Socket::ptr CreateTcpV4();
    static Socket::ptr CreateTcpV6();
private:
    std::shared_ptr<SSL_CTX> m_ctx;
    std::shared_ptr<SSL> m_ssl;
};

}
}

#endif