#ifndef _EMILIA_SOCKETSTREAM_H_
#define _EMILIA_SOCKETSTREAM_H_

#include "emilia/net/socket.h"

namespace emilia{
namespace net{

//socket读写流，由session继承，封装对原生socket的调用
class SockStream{
public:
    using ptr = std::shared_ptr<SockStream>;

protected:
    SockStream(Socket::ptr sock);

    //将数据从sock中读取到data中
    int read(void* data, size_t length);
    //期望读取length字节数据，返回实际读到的数据或者-1(错误)
    int readFixByte(void* data, size_t length);


    //将数据从data中写到sock中
    int write(const void* data, size_t length);
    //期望写length字节的数据，返回实际写的数据或者-1(错误)
    int writeFixByte(const void* data, size_t length);

protected:
    Socket::ptr m_sock;
};

}
}

#endif