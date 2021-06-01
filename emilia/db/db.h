#ifndef _EMILIA_DB_H_
#define _EMILIA_DB_H_

#include <memory>
#include <string>
#include "emilia/log/logmarco.h"

namespace emilia{
namespace db{

/*
整体想法：
对于更改语句：
    1、db调用prepare方法，生成中间结构stmt，并将此结构绑定至db上
    2、如果stmt中有未确定的变量，则调用bind方法
    3、调用stmt的excute方法执行sql语句

对于查询语句：
    1、db调用prepare方法，生成中间结构stmt，并将此结构绑定至db上
    2、如果stmt中有未确定的变量，则调用bind方法
    3、调用stmt的query方法执行sql语句
    4、获取第三步中产生的EData结构
    5、EData指向结果集中的第一条结果，调用get方法获得对应index上的数据
    6、调用EData的nextData方法，获得结果集中的下一条结果，继续调用get方法获取其数据
    7、如果nextData方法的返回值表明结果为空，那么结束此查询，不然回到第六步继续执行
*/

enum class DBType{
    UnKnown = 0x00,
    SQLite3 = 0x01,
    MySQL = 0x02
};

enum class EDDType{
    E_DD_NULL = 0x00,
    E_DD_INT8,
    E_DD_INT16,
    E_DD_INT32,
    E_DD_INT64,
    E_DD_UINT8,
    E_DD_UINT16,
    E_DD_UINT32,
    E_DD_UINT64,
    E_DD_FLOAT,
    E_DD_DOUBLE,
    E_DD_STRING,    //定长非二进制字符串
    E_DD_VSTRING,   //变长非二进制字符串(0-65535)
    E_DD_TEXT,      //变成二进制字符串(>65535)
    E_DD_BLOB       //变长二进制字符串
};

static DBType StringToDBType(const std::string& str){

#define XX(s, t) \
    if(s == str) \
        return t; \

    XX("sqlite3", DBType::SQLite3);
    XX("mysql", DBType::MySQL);
    EMILIA_LOG_ERROR("db") << "DBType: " << str << "Not Exist";

#undef XX
    return DBType::UnKnown;
}

class EData
{
public:
    using ptr = std::shared_ptr<EData>;
    virtual ~EData() {}

    virtual const DBType getType() const = 0;

    virtual bool nextData() = 0;

    virtual std::string toString() = 0;

    virtual int getDataCount() = 0;
    virtual int getColumnCount() = 0;
    virtual int getColumnBytes(int idx) = 0;
    virtual int getColumnType(int idx) = 0;
    virtual std::string getColumnName(int idx) = 0;

    virtual int8_t      getInt8     (int idx) = 0;
    virtual uint8_t     getUint8    (int idx) = 0;
    virtual int16_t     getInt16    (int idx) = 0;
    virtual uint16_t    getUint16   (int idx) = 0;
    virtual int32_t     getInt32    (int idx) = 0;
    virtual uint32_t    getUint32   (int idx) = 0;
    virtual int64_t     getInt64    (int idx) = 0;
    virtual uint64_t    getUint64   (int idx) = 0;

    virtual float       getFloat    (int idx) = 0;
    virtual double      getDouble   (int idx) = 0;

    virtual std::string getString   (int idx) = 0;

    virtual bool        isNull      (int idx) = 0;
    virtual std::string getBlob     (int idx) = 0;
    virtual time_t      getTime     (int idx) = 0;
};

class EStmt
{
public:
    using ptr = std::shared_ptr<EStmt>;
    virtual ~EStmt() {}

    virtual const DBType getType() const = 0;

    virtual int execute() = 0;

    virtual EData::ptr query() = 0;

    virtual int bindInt8    (int idx, const int8_t&  value)     = 0;
    virtual int bindUint8   (int idx, const uint8_t& value)     = 0;
    virtual int bindInt16   (int idx, const int16_t& value)     = 0;
    virtual int bindUint16  (int idx, const uint16_t& value)    = 0;
    virtual int bindInt32   (int idx, const int32_t& value)     = 0;
    virtual int bindUint32  (int idx, const uint32_t& value)    = 0;
    virtual int bindInt64   (int idx, const int64_t& value)     = 0;
    virtual int bindUint64  (int idx, const uint64_t& value)    = 0;

    virtual int bindFloat   (int idx, const float&  value)      = 0;
    virtual int bindDouble  (int idx, const double& value)      = 0;

    //对应精确值
    //virtual int bindDecimal (int idx, const double& value)      = 0;

    //virtual int bindString  (int idx, const std::string& value) = 0;

    //对应固定长度的非二进制字符串(char(n))
    virtual int bindString  (int idx, const char* value) = 0;

    //对应变长的非二进制字符串(varchar(n))
    virtual int bindVString (int idx, const char* value) = 0;

    //varchar可存储(0-65535)的字符数，超过使用text进行存储非二进制字符串
    virtual int bindText    (int idx, const char* value) = 0;

    //virtual int bindBlob    (int idx, const std::string& value) = 0;

    //Blob存储二进制字符串
    virtual int bindBlob    (int idx, const void* value, int64_t size) = 0;

    //对应time
    virtual int bindTime    (int idx, const time_t& time) = 0;

    //对应Null
    virtual int bindNull    (int idx) = 0;
};

class EDB
{
public:
    using ptr = std::shared_ptr<EDB>;
    virtual ~EDB() {}
    
    virtual const DBType getType() const = 0;
    //成功返回影响的记录数
    //失败返回-1
    virtual int execute(const std::string& sql) = 0;
    virtual EData::ptr query(const std::string& sql) = 0;
    virtual EStmt::ptr prepare(const std::string& sql) = 0;

    virtual const std::string getErrorStr() const = 0;
    //virtual int getError() = 0;

    //virtual bool beginTran() = 0;
    //virtual bool commitTran() = 0;
    //virtual bool rollBack() = 0;
};

}
}


#endif
