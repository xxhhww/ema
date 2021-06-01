#include "http_serve.h"
#include "http_session.h"

namespace emilia{
namespace http{

HttpServe::HttpServe(IOManger* iomanger, IPAddress::ptr addr)
:TcpServe(iomanger, addr){
    setName("emilia/1.0");
    m_servletManger.reset(new ServletManger());     //初始化manger

    FileServlet::ptr FileServlet_ (new FileServlet("/home/emilia/workspace/emilia/bin"));

    m_servletManger->addServlet("/emilia/index.html", FileServlet_);
    m_servletManger->addServlet("/favicon.ico", FileServlet_);
}


void HttpServe::handleClient(Socket::ptr connSocket)
{
    HttpSession::ptr session(new HttpSession(connSocket));  //每一个连接就是一个session(会)
    EMILIA_LOG_INFO("test") << "Client Socketid: " << connSocket->fd();
    while(true){
        auto req = session->recvRequest();

        if( req == nullptr )
        {
            EMILIA_LOG_INFO("system") << "连接已断开";
            break;
        }
        
        m_servletManger->handle(req, session);

        if(req->isclose())  //请求是短连接就直接关闭
            break;
    }
}

}
}
