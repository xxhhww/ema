#include "emilia/net/socketstream.h"

namespace emilia{
namespace net{

SockStream::SockStream(Socket::ptr sock)
:m_sock(sock){}

int SockStream::read(void* data, size_t length){
    return m_sock->recv(data, length);
}

int SockStream::readFixByte(void* data, size_t length){
    int offset = 0;
    int left = length;

    while(left > 0){
        int read_len = read((char*)data + offset, left);
        if( read_len <= 0 )
            return read_len;
        
        offset += read_len;
        left -= offset;
    }
    return length;
}


int SockStream::write(const void* data, size_t length){
    return m_sock->send(data, length);
}

int SockStream::writeFixByte(const void* data, size_t length){
    int offset = 0;
    int left = length;

    while(left > 0){
        int write_len = write((char*)data + offset, left);
        if( write_len <= 0 )
            return write_len;
        
        offset += write_len;
        left -= offset;
    }
    return length;
}

}
}