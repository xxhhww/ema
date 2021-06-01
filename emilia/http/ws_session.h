#ifndef _EMILIA_WS_SESSION_H_
#define _EMILIA_WS_SESSION_H_

#include "http_session.h"

namespace emilia
{

namespace http
{

/*
About WebSocket:
    在建立WebSocket之前，浏览器先向服务端发送Http握手报文，服务器处理此报文，若无问题，则返回Http握手报文，表明WebSocket连接已经建立
    浏览器Http握手报文格式:
        GET / HTTP/1.1
        Upgrade: websocket
        Connection: Upgrade
        Host: example.com
        Origin: http://example.com
        Sec-WebSocket-Key: sN9cRrP/n9NdMgdcy2VJFQ==
        Sec-WebSocket-Version: 13
    解释:
        Connection 必须设置 Upgrade，表示客户端希望连接升级。
        Upgrade 字段必须设置 Websocket，表示希望升级到 Websocket 协议。
        Sec-WebSocket-Key 是随机的字符串，服务器端会用这些数据来构造出一个 SHA-1 的信息摘要。
            把 “Sec-WebSocket-Key” 加上一个特殊字符串 “258EAFA5-E914-47DA-95CA-C5AB0DC85B11”，
            然后计算 SHA-1 摘要，之后进行 BASE-64 编码，将结果做为 “Sec-WebSocket-Accept” 头的值，返回给客户端。
            如此操作，可以尽量避免普通 HTTP 请求被误认为 Websocket 协议。
        Sec-WebSocket-Version 表示支持的 Websocket 版本。RFC6455 要求使用的版本是 13，之前草案的版本均应当弃用。
        Origin 字段是可选的，通常用来表示在浏览器中发起此 Websocket 连接所在的页面，类似于 Referer。
            但是，与 Referer 不同的是，Origin 只包含了协议和主机名称。
        其他一些定义在 HTTP 协议中的字段，如 Cookie 等，也可以在 Websocket 中使用。

    服务器回应Http握手报文格式:
        HTTP/1.1 101 Switching Protocols
        Upgrade: websocket
        Connection: Upgrade
        Sec-WebSocket-Accept: fFBooB7FAkLlXgRSz0BT3v4hq5s=
        Sec-WebSocket-Location: ws://example.com/

*/

/*
WebScoket数据帧格式如下:
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+


　　FIN:1位，用于描述消息是否结束，如果为1则该消息为消息尾部,如果为0则还有后续数据包;
　　RSV1,RSV2,RSV3：各1位，用于扩展定义的,如果没有扩展约定的情况则必须为0
　　OPCODE:4位，用于表示消息接收类型，如果接收到未知的opcode，接收端必须关闭连接。长连接探活包就是这里标识的。

　　OPCODE定义的范围：

　　　　0x0表示附加数据帧
　　　　0x1表示文本数据帧
　　　　0x2表示二进制数据帧
　　　　0x3-7暂时无定义，为以后的非控制帧保留
　　　　0x8表示连接关闭
　　　　0x9表示ping
　　　　0xA表示pong
　　　　0xB-F暂时无定义，为以后的控制帧保留

    MASK:1位，用于标识PayloadData是否经过掩码处理，客户端发出的数据帧需要进行掩码处理，所以此位是1。数据需要解码。

　  Payload length === x,

　　    如果 x值在0-125，则是payload的真实长度。
　　    如果 x值是126，则后面2个字节形成的16位无符号整型数的值是payload的真实长度。
　　    如果 x值是127，则后面8个字节形成的64位无符号整型数的值是payload的真实长度。

　　此外，如果payload length占用了多个字节的话，payload length的二进制表达采用网络序（big endian，重要的位在前）。


    服务器发送给客户端的数据包中第二个字节中MASK位为0，
    这说明服务器发送的数据帧未经过掩码处理，
    这个我们从客户端和服务端的数据包截图中也可以发现，
    客户端的数据被加密处理，而服务端的数据则没有。
    （如果服务器收到客户端发送的未经掩码处理的数据包，则会自动断开连接；
    反之，如果客户端收到了服务端发送的经过掩码处理的数据包，也会自动断开连接）。

此外还有
    保活数据包ping/pong + TCP-ACK(服务端ping，客户端pong)

    WebSocket的ping包总长度2个字节:
        1... ....   FIN: True
        .000 ....   RSV: 000
        .... 1001   OpCode: 0x9 == ping
        0... ....   MASK: False
        .000 0000   PayLoad: length == 0 

    WebSocket的Pong包总长度6个字节，包括4个字节的Mask-Key:
        1... ....   FIN:True
        .000 ....   RSV: 000
        .... 1010   OpCode: 0xA == pong
        1... ....   MASK: True
        .000 0000   PayLoad: length == 0
        Masking-key: 78acc086   [占4字节]
*/

struct WsFrameHead
//WebSocket数据帧头部
{
    enum OpCode{
        CONTINUE = 0x0,         // 数据分片帧
        TEXT_FRAME = 0x1,       // 文本帧
        BIN_FRAME = 0x2,        // 二进制帧
        CLOSE = 0x8,            // 断开连接
        PING = 0x9,             // PING
        PONG = 0xA,             // PONG
    };

    uint8_t fin : 1;            //只分配一位
    uint8_t rsv1 : 1;
    uint8_t rsv2 : 1;
    uint8_t rsv3 : 1;

    uint8_t opCode : 4;
    uint8_t mask : 1;
    uint8_t payloadLen : 7;

    const std::string ToString() const;
    const uint32_t isFin() const { return fin; }
    const uint32_t getOpCode() const { return opCode; }
    const uint32_t isMask() const { return mask; }
    const uint32_t getPayloadLen() const { return payloadLen; }
};

class WsFrameMessage
{
public:
    using ptr = std::shared_ptr<WsFrameMessage>;

    const WsFrameHead& getWsFrameHead() const { return m_head; }
    const size_t getLength() const { return m_length; }
    const uint32_t getMaskKey() const { return m_maskKey; }
    const std::string& getData() const { return m_data; }

    void setWsFrameHead(const WsFrameHead& head) { m_head = head; }
    void setMaskKey(uint32_t maskkey) { m_maskKey = maskkey; }
    void setData(const std::string& data) { m_data = data; }

private:
    WsFrameHead m_head;     //数据帧头部字段
    size_t m_length;        //数据字段长度
    uint32_t m_maskKey;     //mask-Key
    std::string m_data;     //数据帧数据字段

};

class WsSession : public HttpSession 
{
public:
    using ptr = std::shared_ptr<WsSession>;

    WsSession(Socket::ptr sock);
    ~WsSession(){}

    HttpRequest::ptr handleShake();      //处理WebSocket的http握手

    WsFrameMessage::ptr recvMessage();
    int sendMessage(WsFrameMessage::ptr message);

};

}
}

#endif