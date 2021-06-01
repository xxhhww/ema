#ifndef _EMILIA_HTTP_SESSION_H_
#define _EMILIA_HTTP_SESSION_H_

#include "emilia/net/socketstream.h"
#include "http.h"

using namespace emilia::net;

namespace emilia{
namespace http{

class HttpSession : public SockStream{
public:
    using ptr = std::shared_ptr<HttpSession>;

    HttpSession(Socket::ptr sock);
    HttpSession() = delete;

    //接收http报文
    HttpRequest::ptr recvRequest();
    //发送http报文
    int sendResponse(HttpResponse::ptr);
};

}
}

#endif