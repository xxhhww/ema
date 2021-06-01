#include "emilia/net/tcpserver.h"
#include "emilia/log/logmarco.h"

#include "emilia/net/sslsocket.h"

namespace emilia{
namespace net{

TcpServe::~TcpServe(){
    //run中已经回收ioManger
}

//启动ioManger
void TcpServe::run(){
    //启动ioManger
    m_ioManger->run();
    /*
    //走到这一步说明m_mainScheduler收到本地客户端通知，已经停止
    //但是ioManger管理的其他SubScheduler还未停止
    //因此，要对ioManger的资源进行回收
    //通过delete ioManger执行ioManger的析构函数
    //析构函数内部ioManger调用其他所有SubScheduler的passiveClose函数
    //通过passiveClose将这些SubScheduler的m_stop标记设置为true，然后再调用tickle进行通知
    //SubScheduler在接到停止通知后，进行对应的处理，然后从run函数退出
    //然后运行这些SubScheduler的线程对SubScheduler执行delete操作，释放他们new的资源
    */
    delete m_ioManger;
}

void TcpServe::handleAccept(){
    while(true){
        Socket::ptr connSock = m_acceptSock->accept();
        m_ioManger->assignByHandle(connSock->fd(), std::bind(&TcpServe::handleClient, this, connSock));
    }
}

//连接套接字执行的函数
void TcpServe::handleClient(Socket::ptr connSock){
    EMILIA_LOG_INFO("system") << connSock->getReAddr()->toString();
}

void TcpServe::assign(int fd, std::function<void()> func){
    m_ioManger->assignByHandle(fd, func);
}

//serveAddr服务启动地址
TcpServe::TcpServe(base::IOManger* ioManger, IPAddress::ptr serveAddr, bool isSSL)
:m_ioManger(ioManger)
,m_serveAddr(serveAddr)
//初始化、bind和Listen都在Create内部完成了
{
    if(!isSSL)
        m_acceptSock = Socket::CreateAcceptV4(serveAddr);
    else
        m_acceptSock = SSLSocket::CreateAcceptV4(serveAddr);
    //m_mainScheduler在IOManger构造函数内部就new了，因此不会段错误
    m_ioManger->assignForMain(std::bind(&TcpServe::handleAccept, this));
}

}
}