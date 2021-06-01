#include "bytestream.h"

namespace emilia{
namespace serial{

//输入数据
ByteStream& ByteStream::operator << (const int32_t& val){

}

ByteStream& ByteStream::operator << (const int64_t& val){

}

ByteStream& ByteStream::operator << (const uint32_t& val){

}

ByteStream& ByteStream::operator << (const uint64_t& val){

}

ByteStream& ByteStream::operator << (const float& val){

}

ByteStream& ByteStream::operator << (const double& val){

}

ByteStream& ByteStream::operator << (const std::string& val){

}

template<typename T>
ByteStream& ByteStream::operator << (const std::vector<T>& val){

}

template<typename T>
ByteStream& ByteStream::operator << (const std::deque<T>& val){

}

template<typename T>
ByteStream& ByteStream::operator << (const std::list<T>& val){

}

template<typename T>
ByteStream& ByteStream::operator << (const std::set<T>& val){

}

template<typename T1, typename T2>
ByteStream& ByteStream::operator << (const std::map<T1, T2>& val){
    
}

}
}