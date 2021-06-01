#include "e_column.h"
#include "emilia/log.h"
#include "emilia/util/stringutil.h"

namespace emilia{
namespace orm{

//xml中的string映射为EColumn中的类型
EColumn::ColumnType EColumn::StringToType(const char* str){

#define XX(type, s) \
    if(s == str) \
        return type; \
    
    XX(EColumn::ColumnType::E_NULL, "null");
    XX(EColumn::ColumnType::E_INT8, "int8");
    XX(EColumn::ColumnType::E_INT16, "int16");
    XX(EColumn::ColumnType::E_INT32, "int32");
    XX(EColumn::ColumnType::E_INT64, "int64");
    XX(EColumn::ColumnType::E_FLOAT, "float");
    XX(EColumn::ColumnType::E_DOUBLE, "double");
    XX(EColumn::ColumnType::E_STRING, "string");
    XX(EColumn::ColumnType::E_VSTRING, "vstring");
    XX(EColumn::ColumnType::E_TEXT, "text");
    XX(EColumn::ColumnType::E_BLOB, "blob");
    XX(EColumn::ColumnType::E_TIMESTAMP, "timestamp");

#undef XX
    return EColumn::ColumnType::E_UNKNOWN;
}


std::string EColumn::getMySQLTypeStr(){

#define XX(type, str) \
    case type : \
        return str; \

    switch(m_colType){
        XX(EColumn::ColumnType::E_NULL, "null");
        XX(EColumn::ColumnType::E_INT8, "tinyint");
        XX(EColumn::ColumnType::E_INT16, "smallint");
        XX(EColumn::ColumnType::E_INT32, "int");
        XX(EColumn::ColumnType::E_INT64, "bigint");
        XX(EColumn::ColumnType::E_FLOAT, "float");
        XX(EColumn::ColumnType::E_DOUBLE, "double");
        XX(EColumn::ColumnType::E_STRING, "char");
        XX(EColumn::ColumnType::E_VSTRING, "varchar");
        XX(EColumn::ColumnType::E_TEXT, "text");
        XX(EColumn::ColumnType::E_BLOB, "blob");
        XX(EColumn::ColumnType::E_TIMESTAMP, "timestamp");
        default:
            return "unknown";
    }
#undef XX
}

std::string EColumn::getSQLite3TypeStr(){

#define XX(type, str) \
    case type : \
        return str; \

    switch(m_colType){
        XX(EColumn::ColumnType::E_NULL, "NULL");
        XX(EColumn::ColumnType::E_INT8, "INTEGER");
        XX(EColumn::ColumnType::E_INT16, "INTEGER");
        XX(EColumn::ColumnType::E_INT32, "INTEGER");
        XX(EColumn::ColumnType::E_INT64, "INTEGER");
        XX(EColumn::ColumnType::E_FLOAT, "REAL");
        XX(EColumn::ColumnType::E_DOUBLE, "REAL");
        XX(EColumn::ColumnType::E_STRING, "TEXT");
        XX(EColumn::ColumnType::E_VSTRING, "TEXT");
        XX(EColumn::ColumnType::E_TEXT, "TEXT");
        XX(EColumn::ColumnType::E_BLOB, "BLOB");
        XX(EColumn::ColumnType::E_TIMESTAMP, "TIMESTAMP");
        default:
            return "UNKNOWN";
    }
}

//EColumn中的类型映射为c++中的类型(注意符号)
std::string EColumn::getCppTypeStr(){

#define XX1(t, str) \
    case t : \
        return str; \

#define XX2(t, str1, str2) \
    case t : \
        if(m_isUnsigned) \
            return str2; \
        else    \
            return str1; \

    switch(m_colType){
        XX2(EColumn::ColumnType::E_INT8, "int8_t", "uint8_t");
        XX2(EColumn::ColumnType::E_INT16, "int16_t", "uint16_t");
        XX2(EColumn::ColumnType::E_INT32, "int32_t", "uint32_t");
        XX2(EColumn::ColumnType::E_INT64, "int64_t", "uint64_t");
        XX1(EColumn::ColumnType::E_FLOAT, "float");
        XX1(EColumn::ColumnType::E_DOUBLE, "double");
        XX1(EColumn::ColumnType::E_STRING, "std::string");
        XX1(EColumn::ColumnType::E_VSTRING, "std::string");
        XX1(EColumn::ColumnType::E_TEXT, "std::string");
        XX1(EColumn::ColumnType::E_BLOB, "std::string");
        XX1(EColumn::ColumnType::E_TIMESTAMP, "time_t");
        default:
            return "UnKnown";
    }

#undef XX1
#undef XX2
}

std::string EColumn::getEDBType(){

#define XX1(t, str) \
    case t : \
        return str; \

#define XX2(t, str1, str2) \
    case t : \
        if(m_isUnsigned) \
            return str2; \
        else    \
            return str1; \

    switch(m_colType){
        XX2(EColumn::ColumnType::E_INT8, "Int8", "Uint8");
        XX2(EColumn::ColumnType::E_INT16, "Int16", "Uint16");
        XX2(EColumn::ColumnType::E_INT32, "Int32", "Uint32");
        XX2(EColumn::ColumnType::E_INT64, "Int64", "Uint64");
        XX1(EColumn::ColumnType::E_FLOAT, "Float");
        XX1(EColumn::ColumnType::E_DOUBLE, "Double");
        XX1(EColumn::ColumnType::E_STRING, "String");
        XX1(EColumn::ColumnType::E_VSTRING, "VString");
        XX1(EColumn::ColumnType::E_TEXT, "Text");
        XX1(EColumn::ColumnType::E_BLOB, "Blob");
        XX1(EColumn::ColumnType::E_TIMESTAMP, "Time");
        default:
            return "UnKnown";
    }

#undef XX1
#undef XX2
}

EColumn::ptr EColumn::Create(const tinyxml2::XMLElement& node){

    EColumn::ptr column(new EColumn);

#define XX(str) \
    if(!node.Attribute(str)){ \
        EMILIA_LOG_ERROR("Orm") << "Column " << str << " Not Exist"; \
        return nullptr; \
    } \

    XX("name");
    column->m_name = node.Attribute("name");
    XX("type");
    column->m_colType = StringToType(node.Attribute("type"));

    if(node.Attribute("length"))
        column->m_length = util::StringUtil::ToUint32(node.Attribute("length"));

    //是否为无符号
    //没有unsigned属性就默认为false
    //存在unsigned属性就获取其布尔值
    column->m_isUnsigned = node.BoolAttribute("unsigned", false);

    //字段是否可以为Null
    column->m_isNull = node.BoolAttribute("null", false);

    //字段默认值
    if(node.Attribute("default"))
        column->m_default = node.Attribute("default");
    else
        column->m_default = "";
    
    //字段是否自增
    column->m_autoInc = node.BoolAttribute("increment", false);
    
    //字段注释
    if(node.Attribute("comment"))
        column->m_comment = node.Attribute("comment");
    else
        column->m_comment = "";

    return column;
}

std::string EColumn::funcGetDefine(){
    std::stringstream ss;
    ss << "const " << getCppTypeStr() << "& ";
    ss << "get" << util::StringUtil::UpperLetter(m_name.c_str()) << "() " << "const ";
    ss << "{ return m_" << m_name << "; }";
    ss << std::endl;
    return ss.str();
}

std::string EColumn::funcSetDefine(){
    std::stringstream ss;
    ss << "void set" << util::StringUtil::UpperLetter(m_name.c_str());
    ss << "(const " << getCppTypeStr() << "& " << m_name << ")";
    ss << "{ m_" << m_name << " = " << m_name << "; }";
    ss << std::endl;
    return ss.str();
}

std::string EColumn::memberDefine(){
    std::stringstream ss;
    ss << getCppTypeStr() << " m_" << m_name << ";" << std::endl;
    return ss.str();
}

//生成对应的MYSQL的create语句
std::string EColumn::getMySQLCreateStr(){
    std::stringstream ss;
    ss << "`" << m_name << "` " <<  getMySQLTypeStr() << "(" << m_length << ") ";
    if(m_isUnsigned){
        ss << "UNSIGNED ";
    }
    if(m_autoInc)
        ss << "AUTO_INCREMENT ";
    if(!m_isNull && !m_isPK){
        ss << "NOT NULL ";
    }
    if(!m_default.empty()){
        ss << "DEFAULT " << m_default;
    }
    if(!m_comment.empty()){
        ss << "COMMENT '" << m_comment << "'";
    }
    return ss.str();
}

//生成对应的SQLite3的create语句
std::string EColumn::getSQLite3CreateStr(){

    std::stringstream ss;
    ss << m_name <<  getSQLite3TypeStr() << "(" << m_length << ") ";
    if(m_isPK)
        ss << "PRIMARY KEY";
    if(m_autoInc)
        ss << "AUTOINCREMENT ";
    if(!m_isNull && !m_isPK){
        ss << "NOT NULL ";
    }
    if(!m_default.empty()){
        ss << "DEFAULT " << m_default;
    }
    if(!m_comment.empty()){
        ss << "--" << m_comment;
    }
    return ss.str();
}

}
}