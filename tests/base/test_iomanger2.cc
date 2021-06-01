#include "emilia/base/iomanger.h"
#include "emilia/log/logmarco.h"

using namespace emilia::base;
using namespace emilia::net;
using namespace emilia::log;

//多线程(1个accept 2个connect)
IOManger* ioManger = IOManger::Create(3);

void handleClient(Socket::ptr sock){
    sock->setRDTimeOut(8000);
    while(true){
        char buf[200] = {0};

        int rt = sock->recv(buf, 200);
        if(rt == 0){
            EMILIA_LOG_ERROR("test") << sock->fd() << " 客户端关闭连接";
            return ;
        }
        if(rt == -1){
            EMILIA_LOG_ERROR("test") << sock->fd() << " 系统出错";
            return ;
        }
        if(rt == -2){
            EMILIA_LOG_ERROR("test") << sock->fd() << " 客户端响应超时，服务端主动关闭";
            return ;
        }

        EMILIA_LOG_INFO("test") << buf;

        rt = sock->send(buf, rt);
        if(rt == -1){
            EMILIA_LOG_INFO("test") << "连接远端关闭";
            return ;
        }
        if(rt == -2){
            EMILIA_LOG_INFO("test") << "客户端响应超时，服务端主动关闭";
            return ;
        }
    }
}

void handleAccept(){
    IPAddress::ptr addr = IPAddress::CreateByString("127.0.0.1", 20003);
    Socket::ptr acceptSock = Socket::CreateAcceptV4(addr);

    while(true){
        Socket::ptr connSock = acceptSock->accept();
        EMILIA_LOG_INFO("test") << connSock->getReAddr()->toString();
        ioManger->assignByHandle(connSock->fd(), std::bind(handleClient, connSock));
    }
}

int main(){
    ioManger->assignForMain(handleAccept);
    ioManger->run();
}