#ifndef _EMILIA_INDEX_H_
#define _EMILIA_INDEX_H_

#include <memory>
#include <string>
#include <vector>
#include <tinyxml2.h>

namespace emilia{
namespace orm{

class EIndex{
public:
    using ptr = std::shared_ptr<EIndex>;

    //索引类型
    enum IndexType{
        Primary = 0x01, //主键索引
        Unique = 0x02,  //唯一索引
        Index = 0x03,  //普通索引
        FullText = 0x04,//全文索引
        UnKnown = 0x05, //未知
    };

    const std::string& getName() const { return m_name; }
    const std::string& getComment() const { return m_comment; }
    const IndexType& getIndexType() const { return m_indexType; }
    const std::vector<std::pair<std::string, uint16_t> >& getColumns() const { return m_cols; }

    //将xml中的描述转为对应的IndexType
    static IndexType StringToType(const std::string& str);
    static EIndex::ptr Create(const tinyxml2::XMLElement& node);

private:
    //索引名称
    std::string m_name;
    //索引类型
    EIndex::IndexType m_indexType;
    //索引包含的列及对应的列前缀(最长为1000)
    std::vector<std::pair<std::string, uint16_t> > m_cols;
    //索引注释
    std::string m_comment;
};

}
}

#endif