#ifndef _EMILIA_HTTP_SERVE_H_
#define _EMILIA_HTTP_SERVE_H_

#include "emilia/net/tcpserver.h"
#include "http_servlet.h"

using namespace emilia::net;
using namespace emilia::base;

namespace emilia{
namespace http{

class HttpServe : public TcpServe{
public:
    HttpServe(IOManger* iomanger, IPAddress::ptr addr);
    ~HttpServe(){}

    void handleClient(Socket::ptr connSocket) override;

private:
    ServletManger::ptr m_servletManger;
};

}
}

#endif