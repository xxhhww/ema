#ifndef _EMILIA_COLUMN_H_
#define _EMILIA_COLUMN_H_

#include <memory>
#include <string>
#include <tinyxml2.h>

namespace emilia{
namespace orm{

class EColumn{
public:
    using ptr = std::shared_ptr<EColumn>;
    enum ColumnType{
        E_NULL = 0x00,
        E_INT8,
        E_INT16,
        E_INT32,
        E_INT64,
        E_FLOAT,
        E_DOUBLE,
        E_STRING,    //对应MySQL的char(n)
        E_VSTRING,   //对应MySQL的varchar(n)
        E_TEXT,     //对应MySQL的text(n)
        E_BLOB,     //对应MySQL的二进制数据
        E_TIMESTAMP,
        E_UNKNOWN
    };

    const std::string& getName() const { return m_name; }
    const ColumnType& getType() const { return m_colType; }
    const int8_t& getLength() const { return m_length; }
    const std::string& getDefault() const { return m_default; }
    bool isUnsigned() { return m_isUnsigned; }
    bool isNull() { return m_isNull; }
    bool isAutoInc() { return m_autoInc; }
    const std::string& getComment() const { return m_comment; }

    //根据自身的类型，返回MySQL中对应的类型
    std::string getMySQLTypeStr();

    //根据自身的类型返回SQLite3中对应的类型
    std::string getSQLite3TypeStr();

    //根据自身的类型和是否有符号，返回对应的CPP中的类型
    std::string getCppTypeStr();

    //根据自身的类型和是否有符号，返回对应的stmt中的bind后面的类型(data中get后面)
    std::string getEDBType();

    //生成对应的MYSQL的create语句
    std::string getMySQLCreateStr();
    //生成对应的SQLite3的create语句
    std::string getSQLite3CreateStr();

    //table调用
    std::string funcGetDefine();
    std::string funcSetDefine();
    std::string memberDefine();

    void setPK() { m_isPK = true; }
    bool isPK() { return m_isPK; }

    static EColumn::ptr Create(const tinyxml2::XMLElement& node);

    //将xml中的数据类型转换为ColumnType中的对应值
    static EColumn::ColumnType StringToType(const char* str);
private:
    //列名
    std::string m_name;
    //列的数据类型
    ColumnType m_colType;
    //列数据长度
    int m_length;
    //列数据类型是否有符号
    bool m_isUnsigned = false;
    //列是否可以为Null
    bool m_isNull = true;
    //列默认值
    std::string m_default;
    //列是否自增
    bool m_autoInc = false;
    //列注释
    std::string m_comment;

    bool m_isPK = false;
};

}
}

#endif