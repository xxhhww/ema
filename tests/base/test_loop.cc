#include "emilia/base/scheduler.h"
#include "emilia/net/address.h"
#include "emilia/util/macroutil.h"
#include "emilia/log/logmarco.h"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

using namespace emilia::log;

void handleClient(int connfd){
    std::cout << "I Get Client" << std::endl;

    while(true){
        //注册可读事件
        emilia::base::Scheduler::GetThis()->addReadEvent(connfd, emilia::base::Fiber::GetThis(), 0);

        //挂起
        emilia::base::Fiber::YieldToPend();

        //被唤醒，可读
        char buffer[200] = {0};
        ssize_t res = ::recv(connfd, buffer, 200, 0);
        if(res == -1 || res == 0){
            EMILIA_LOG_INFO("test") << "Client Out";
            return ;
        }

        std::cout << buffer << std::endl;
        
        //注册可写事件
        emilia::base::Scheduler::GetThis()->addWriteEvent(connfd, emilia::base::Fiber::GetThis());
        //挂起
        emilia::base::Fiber::YieldToPend();
        std::cout << "Can Write" << std::endl;

        //被唤醒，可写
        ssize_t ses = ::send(connfd, buffer, res, 0);
        if(ses == -1){
            EMILIA_LOG_ERROR("test") << "Send Fail";
        }
    }

}

void handleAccept(){
    std::cout << "1" << std::endl;
    //初始化socket
    int acceptSock = ::socket(AF_INET, SOCK_STREAM, 0);
    //设置成非阻塞
    int flags = fcntl(acceptSock, F_GETFL, 0);
    fcntl(acceptSock, F_SETFL, flags | O_NONBLOCK);
    //绑定地址
    std::cout << "2" << std::endl;
    auto addr = emilia::net::IPAddress::CreateByString("127.0.0.1", 20045);
    int rt = ::bind(acceptSock, addr->getAddr(), addr->getAddrLen());
    if(rt != 0)
        EMILIA_LOG_ERROR("test") << "bind fail";
    rt = ::listen(acceptSock, SOMAXCONN);
    if(rt != 0)
        EMILIA_LOG_ERROR("test") << "listen fail";
    while(true){
        //注册可读事件(sockfd上有可读事件就把此协程唤醒)
        emilia::base::Scheduler::GetThis()->addReadEvent(acceptSock, emilia::base::Fiber::GetThis(), 0);
        //挂起，等待可读事件
        emilia::base::Fiber::YieldToPend();

        //被唤醒，有可读事件
        //获取连接套接字
        sockaddr addr;
        socklen_t addrlen= sizeof(addr);

        int connSock = ::accept(acceptSock,(sockaddr*)&addr, &addrlen);

        emilia::net::IPAddress::ptr remoteAddr = emilia::net::IPAddress::CreateBySockaddress((const sockaddr*)&addr, sizeof(struct sockaddr));
        EMILIA_LOG_INFO("test") << remoteAddr->toString() << " " << connSock;

        emilia::base::Scheduler::GetThis()->assign(std::bind(handleClient, connSock));
    }
}

int main(){
    emilia::base::Scheduler* scheduler = new emilia::base::Scheduler();
    scheduler->assign(handleAccept);
    scheduler->run();
}