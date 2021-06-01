#ifndef _EMILIA_BYTESTREAM_H_
#define _EMILIA_BYTESTREAM_H_

#include "emilia/serialize/nodelist.h"
#include <memory>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

namespace emilia{
namespace serial{

//字节流
class ByteStream{
public:
    using ptr = std::shared_ptr<ByteStream>;

    //数据类型
    enum DataType{
        
    };

    ByteStream();
    ~ByteStream();
    //向文件写入NodeList中的二进制数据
    bool writeToFile(const std::string& fileName);
    //从文件中读取二进制数据并保存在NodeList中
    bool readFromFile(const std::string& fileName);
    //清空NodeList中的数据
    bool clear();

    //输入数据
    ByteStream& operator << (const int32_t& val);
    ByteStream& operator << (const int64_t& val);
    ByteStream& operator << (const uint32_t& val);
    ByteStream& operator << (const uint64_t& val);

    ByteStream& operator << (const float& val);
    ByteStream& operator << (const double& val);

    ByteStream& operator << (const std::string& val);

    template<typename T>
    ByteStream& operator << (const std::vector<T>& val);

    template<typename T>
    ByteStream& operator << (const std::deque<T>& val);

    template<typename T>
    ByteStream& operator << (const std::list<T>& val);

    template<typename T>
    ByteStream& operator << (const std::set<T>& val);

    template<typename T1, typename T2>
    ByteStream& operator << (const std::map<T1, T2>& val);

    //输出数据
    bool operator >> (int32_t& ret);
    bool operator >> (int64_t& ret);
    bool operator >> (uint32_t& ret);
    bool operator >> (uint64_t& ret);

    bool operator >> (float& ret);
    bool operator >> (double& ret);

    bool operator >> (std::string& ret);

    template <typename T>
    bool operator >> (std::vector<T>& ret);

    template <typename T>
    bool operator >> (std::deque<T>& ret);

    template <typename T>
    bool operator >> (std::list<T>& ret);

    template <typename T>
    bool operator >> (std::set<T>& ret);

    template <typename T1, typename T2>
    bool operator >> (std::map<T1, T2>& ret);

private:
    //写入数据(调用NodeList的写函数写入数据节点)
    //自身由public中的各种写函数调用
    bool writeByte(const char* data, size_t dataSize);
    //读取数据(调用NodeList的读函数从数据节点读取数据)
    //自身由public中的各种读函数调用
    bool readByte(char* data, size_t dataSize);

private:
    //node列表，存放字节数据
    NodeList* m_nodeList;
};

}
}

#endif