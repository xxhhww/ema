#ifndef _EMILIA_BUFFERLIST_H_
#define _EMILIA_BUFFERLIST_H_

#include <cstddef>

namespace emilia{
namespace serial{

//列表节点，用于记录数据，以及指向下一个节点
class Node{
public:
    //Node构造函数，给m_data分配内存空间
    Node(size_t nodeSize);
    //Node析构函数，给释放m_data空间
    ~Node();
private:
    //记录节点数据
    char* m_data;
    //指向下一个节点
    Node* m_next;
    //内存大小
    size_t m_size;
};

class NodeList{
public:
    //Nodelist构造函数，初始化m_header
    NodeList(size_t nodeSize = 4096);
    //Nodelist析构函数，将Node列表全部析构掉
    ~NodeList();
    //写入数据
    bool writeByte(const char* data, size_t dataSize);
    //读取数据
    bool readByte(char* data, size_t dataSize);
private:
    //Node列表表头节点
    Node* m_header;
};

}
}

#endif