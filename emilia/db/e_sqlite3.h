#ifndef _EMILIA_E_SQLITE3_H_
#define _EMILIA_E_SQLITE3_H_

#include "emilia/db/db.h"

#include <sqlite3.h>

namespace emilia{
namespace db{

class ESQLite3 : public EDB
               , public std::enable_shared_from_this<ESQLite3>
{
friend class ESQLite3Stmt;
public:
    using ptr = std::shared_ptr<ESQLite3>;

    //静态函数，默认生成可读写的sqlite3
    static EDB::ptr Create(const std::string& dbname, int flags 
                        = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    
    ~ESQLite3();

    //sqlite3* getDB() { return m_db; }

    const DBType getType() const override { return m_type; }

    int execute(const std::string& sql) override;
    EData::ptr query(const std::string& sql) override;

    //重写EDB的prepare方法
    EStmt::ptr prepare(const std::string& sql) override;

    const std::string getErrorStr() const override;
   
    /*
        name:注册的函数名字
        nArg:函数参数的个数，，如果为1,就是可变长参数
        eTextRep:文本编码格式
        pApp:传递给回调函数数据，可在xFunc xStep xFinal中使用，但必须使用特殊的Api获取数据
        xFunc:普通回调函数
        xStep:聚合步骤函数，结果集的每一行都要使用xStep函数
        xFinal:处理完结果集的所有行后，，使用此函数进行整个聚合汇总处理
        xDestory:清理函数，，用于释放应用数据pApp
    */ 
    //增加回调函数
    /*
    int addCbFunc(const std::string& name, int nArg, int eTextRep, void* pApp,
            std::function<void(sqlite3_context*, int, sqlite3_value**)> xFunc,
            std::function<void(sqlite3_context*, int, sqlite3_value**)> xStep,
            std::function<void(sqlite3_context*)> xFinal,
            std::function<void(void*)> xDestroy);

    //增加比较函数
    int addCpFunc(const std::string& name, int eTextRep, void* pApp,
            std::function<int(void*, int, const void*, int, const void*)> xComp,
            std::function<void(void*)> xDestory);
    */
private:
    ESQLite3(sqlite3* db, DBType type);
private:
    //指向一个已经打开的sqlite3数据库
    sqlite3* m_db;    

    //数据库类型
    DBType m_type;
};


class ESQLite3Stmt : public EStmt
                   , public std::enable_shared_from_this<ESQLite3Stmt>
{
friend class ESQLite3;
friend class ESQLite3Data;
public:
    using ptr = std::shared_ptr<ESQLite3Stmt>;
    ~ESQLite3Stmt();

    const DBType getType() const override { return m_db->getType(); }

    //SQL执行语句，进行插入删除更新等操作
    int execute() override;

    //SQL查询语句，返回一个指向ESQLite3Data对象的EData指针
    EData::ptr query() override;

    //类型值绑定函数
    int bindInt8    (int idx, const int8_t&  value)     override;
    int bindUint8   (int idx, const uint8_t& value)     override;
    int bindInt16   (int idx, const int16_t& value)     override;
    int bindUint16  (int idx, const uint16_t& value)    override;
    int bindInt32   (int idx, const int32_t& value)     override;
    int bindUint32  (int idx, const uint32_t& value)    override;
    int bindInt64   (int idx, const int64_t& value)     override;
    int bindUint64  (int idx, const uint64_t& value)    override;

    int bindFloat   (int idx, const float&  value)      override;
    int bindDouble  (int idx, const double& value)      override;

    //int bindString  (int idx, const std::string& value) override;
    int bindString  (int idx, const char* value)        override;
    int bindVString (int idx, const char* value)        override;
    int bindText    (int idx, const char* value)        override;

    //int bindBlob    (int idx, const std::string& value) override;
    int bindBlob    (int idx, const void* value, int64_t size) override;
    int bindTime    (int idx, const time_t& value)      override;
    int bindNull    (int idx)                           override;

private:
    //仅供ESQLite3的prepare()方法调用
    static EStmt::ptr Create(ESQLite3::ptr db, const std::string& sql);

    //仅由ESQLite3Stmt的Create()方法调用
    ESQLite3Stmt(ESQLite3::ptr db, sqlite3_stmt* stmt);

    //实际的执行函数，由execute()和nextData()调用
    int step();
private:
    //stmt绑定的数据库
    ESQLite3::ptr m_db;

    //由sql语句生成的预处理语句对象
    sqlite3_stmt* m_stmt;
};

class ESQLite3Data : public EData
{
friend class ESQLite3Stmt;
public:
    using ptr = std::shared_ptr<ESQLite3Data>;
    ~ESQLite3Data();

    const DBType getType() const override { return m_stmt->getType(); }

    //获得结果集的下一条记录
    bool nextData() override;
    
    //将当前记录以字符串的形式输出
    std::string toString() override;

    //获取结果集的个数
    int getDataCount    () override;

    //获取列的个数
    int getColumnCount  () override;

    //获取idx指向列的字节长度
    int getColumnBytes  (int idx)   override;

    //获取idx对应列的类型
    int getColumnType   (int idx)   override;
    
    //获取idx对应列的名称
    std::string getColumnName(int idx) override;

    //从结果集的结果记录中获取对应的数据
    int8_t      getInt8     (int idx) override;
    uint8_t     getUint8    (int idx) override;
    int16_t     getInt16    (int idx) override;
    uint16_t    getUint16   (int idx) override;
    int32_t     getInt32    (int idx) override;
    uint32_t    getUint32   (int idx) override;
    int64_t     getInt64    (int idx) override;
    uint64_t    getUint64   (int idx) override;

    float       getFloat    (int idx) override;
    double      getDouble   (int idx) override;

    std::string getString   (int idx) override;

    bool        isNull      (int idx) override;
    std::string getBlob     (int idx) override;
    time_t     getTime      (int idx) override;

private:
    //由ESQLite3Stmt中的query方法调用
    static EData::ptr Create(ESQLite3Stmt::ptr stmt);

    //由Create方法调用
    ESQLite3Data(ESQLite3Stmt::ptr stmt);
private:
    ESQLite3Stmt::ptr m_stmt;
};
//数据库管理类

//用户自定义函数

//用户自定义聚合函数
//用户自定义排序方法
}
}


#endif
