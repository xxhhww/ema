#include "emilia/net/socket.h"
#include "emilia/log/logmarco.h"
#include "emilia/base/scheduler.h"

using namespace emilia::base;
using namespace emilia::net;
using namespace emilia::log;

void handleClient(Socket::ptr sock){
    sock->setRDTimeOut(5000);
    while(true){
        char buf[200] = {0};

        int rt = sock->recv(buf, 200);
        if(rt == 0){
            EMILIA_LOG_INFO("test") << sock->fd() << " 客户端关闭连接";
            return ;
        }
        if(rt == -1){
            EMILIA_LOG_INFO("test") << sock->fd() << " 系统出错";
            return ;
        }
        if(rt == -2){
            EMILIA_LOG_INFO("test") << sock->fd() << " 客户端响应超时，服务端主动关闭";
            return ;
        }
        rt = sock->send(buf, rt);
    }
}

void handleAccept(){
    IPAddress::ptr addr = IPAddress::CreateByString("127.0.0.1", 20002);
    Socket::ptr acceptSock = Socket::CreateAcceptV4(addr);

    while(true){
        Socket::ptr connSock = acceptSock->accept();
        EMILIA_LOG_INFO("test") << connSock->getReAddr()->toString();
        Scheduler::GetThis()->assign(std::bind(handleClient, connSock));
    }
}

int main(){
    Scheduler* scheduler = new Scheduler();
    scheduler->assign(handleAccept);
    scheduler->run();
}