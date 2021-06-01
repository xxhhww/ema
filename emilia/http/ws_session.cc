#include "ws_session.h"
#include "emilia/util.h"
#include "emilia/endian.h"

#include <sstream>

namespace emilia
{
namespace http
{

const std::string WsFrameHead::ToString() const
{
    std::stringstream ss;
    ss << "fin: "       << (int)fin         << std::endl;
    ss << "rsv1: "      << (int)rsv1        << std::endl;
    ss << "rsv2: "      << (int)rsv2        << std::endl;
    ss << "rsv3: "      << (int)rsv3        << std::endl;
    ss << "opcode: "    << (int)opCode      << std::endl;
    ss << "mask: "      << (int)mask        << std::endl;
    ss << "payloadLen: "<< (int)payloadLen  << std::endl;

    return ss.str();
}

WsSession::WsSession(Socket::ptr sock)
:HttpSession(sock)
{}

HttpRequest::ptr WsSession::handleShake()
//处理http握手报文
{
    HttpRequest::ptr req = recvRequest();   //获得一个握手报文
    if( !req )
    {
        EMILIA_LOG_ERROR("system") << "Invalid HttpRequest";
        return nullptr;
    }

#define XX(str1, str2)    \
    if( strcasecmp(req->getHeader(#str1).c_str(), #str2) != 0 ) \
    {   \
        EMILIA_LOG_ERROR("system") << #str1 << " != " << #str2; \
        return nullptr;     \
    }   \

    XX(Upgrade, websocket)
    XX(Connection, upgrade)
    XX(Sec-webSocket-Version, 13)

#undef XX

    std::string key = req->getHeader("Sec-WebSocket-Key");
    if( key.empty() )
    {
        EMILIA_LOG_ERROR("system") << "Sec-WebSocket-Key == NULL";
        return nullptr;
    }

    std::string v = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    v = emilia::base64encode(emilia::SHA1(v));
    
    
}

WsFrameMessage::ptr WsSession::recvMessage()
{
}

int WsSession::sendMessage(WsFrameMessage::ptr message)
{}


}
}
