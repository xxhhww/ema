#include "e_index.h"
#include "log.h"
#include "emilia/util/stringutil.h"

namespace emilia{
namespace orm{
//将xml中的描述转为对应的IndexType
EIndex::IndexType EIndex::StringToType(const std::string& str)
{
#define XX(type, s) \
    if(s == str) \
        return type; \

    XX(EIndex::IndexType::Primary, "primary")
    XX(EIndex::IndexType::Unique, "unique")
    XX(EIndex::IndexType::Index, "index")
    XX(EIndex::IndexType::FullText, "fulltext")

#undef XX

    return EIndex::IndexType::UnKnown;
}

//解析XMl的index节点
EIndex::ptr EIndex::Create(const tinyxml2::XMLElement& node)
{
#define XX(str) \
    if(!node.Attribute(str)){ \
        EMILIA_LOG_ERROR("Orm") << "Index " << str << " Not Exist"; \
        return nullptr; \
    } \

    EIndex::ptr rt(new EIndex); 
    XX("name");
    rt->m_name = node.Attribute("name");
    XX("type");
    rt->m_indexType = EIndex::StringToType(node.Attribute("type"));
    XX("cols");
    //填充列 cols = "name:3,age,id"
    //age与id无索引前缀，name的索引前缀为3
    std::vector<std::string> cols = util::StringUtil::Split(node.Attribute("cols"), ',');
    for(auto& i : cols){
        std::vector<std::string> indexAndSub = util::StringUtil::Split(i.c_str(), ':');
        //有前缀
        if(indexAndSub.size() == 2){
            rt->m_cols.push_back(std::make_pair(indexAndSub[0], util::StringUtil::ToUint16(indexAndSub[1].c_str())));
        }
        //无前缀
        else if(indexAndSub.size() == 1){
            rt->m_cols.push_back(std::make_pair(indexAndSub[0], 0));
        }
        else{
            EMILIA_LOG_ERROR("Orm") << "Index Cols Error";
        }
    }
    
#undef XX

    if(node.Attribute("comment")){
        rt->m_comment = node.Attribute("comment");
    }
    return rt;
}

}
}