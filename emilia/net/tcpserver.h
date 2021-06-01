#ifndef _EMILIA_TCPSERVER_H_
#define _EMILIA_TCPSERVER_H_

#include "emilia/base/iomanger.h"

namespace emilia{
namespace net{

class TcpServe{
public:
    //serveAddr服务启动地址
    TcpServe(base::IOManger* ioManger, IPAddress::ptr serveAddr, bool isSSL = false);

    virtual ~TcpServe();
    //启动ioManger
    void run();
    //设置服务名称
    void setName(const std::string& name) { m_name = name; }

    //监听套接字运行的函数
    void handleAccept();
    //连接套接字执行的函数
    virtual void handleClient(Socket::ptr connSock);
    //将fd对应的任务func，放入ioManger对应的Scheduler中
    void assign(int fd, std::function<void()> func);

private:
    //任务处理器
    base::IOManger* m_ioManger;
    //服务地址
    IPAddress::ptr m_serveAddr;
    //监听套接字
    Socket::ptr m_acceptSock;
    //服务名称
    std::string m_name;
};

}
}

#endif