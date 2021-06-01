#include "emilia/net/tcpserver.h"
#include "emilia/log/logmarco.h"


using namespace emilia::base;
using namespace emilia::net;
using namespace emilia::log;

class EchoServe : public TcpServe{
public:
    EchoServe(IOManger* ioManger, IPAddress::ptr serveAddr)
    :TcpServe(ioManger, serveAddr){}

    virtual void handleClient(Socket::ptr connSock) override{
        char buf[200] = {0};
        connSock->setRDTimeOut(8000);
        while(true){
            int rt = connSock->recv(buf, 200);
            if(rt == 0){
                EMILIA_LOG_ERROR("test") << connSock->fd() << " 客户端关闭连接";
                return ;
            }
            if(rt == -1){
                EMILIA_LOG_ERROR("test") << connSock->fd() << " 系统出错";
                return ;
            }
            if(rt == -2){
                EMILIA_LOG_ERROR("test") << connSock->fd() << " 客户端响应超时，服务端主动关闭";
                return ;
            }
            assign(connSock->fd(), std::bind(&EchoServe::writeMsg, this, connSock));
        }
    }

    void writeMsg(Socket::ptr connSock){
        std::string buf = "收到";
        int rt = connSock->send(buf.c_str(), buf.size());
        if(rt == 0){
            EMILIA_LOG_ERROR("test") << connSock->fd() << " 客户端关闭连接";
            return ;
        }
        if(rt == -1){
            EMILIA_LOG_ERROR("test") << connSock->fd() << " 系统出错";
            return ;
        }
        if(rt == -2){
            EMILIA_LOG_ERROR("test") << connSock->fd() << " 客户端响应超时，服务端主动关闭";
            return ;
        }
    }
};

int main(){
    EchoServe echoServe(EchoServe(IOManger::Create(1), IPAddress::CreateByString("127.0.0.1", 20002)));
    echoServe.run();
}