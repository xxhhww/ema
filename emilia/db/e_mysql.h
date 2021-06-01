#ifndef _EMILIA_E_MYSQL_H_
#define _EMILIA_E_MYSQL_H_

#include "emilia/db/db.h"
#include "emilia/log/logmarco.h"
#include <mysql/mysql.h>
#include <map>
#include <vector>
/*
基本数据结构介绍：
    一、MYSQL*
        指向一个数据库连接的指针，代表一个数据库连接
    二、MYSQL_RES*
        指向一个结果集结构的指针，代表一个查询操作结果集
    三、MYSQL_ROW*
        指向一个结果行字段的指针，代表一个结果行
    四、MYSQL_FIFLE*
        指向一个列字段的指针，代表一个列
    五、MYSQL_STMT*
        1、调用mysql_stmt_init()返回时
            指向一个已经初始化好的预处理语句句柄的指针，代表一个预处理语句句柄
        2、调用mysql_stmt_prepare()返回时
            指向一个已经prepared sql语句的指针，其实就是句柄获得了sql语句的绑定
    六、MYSQL_BIND*
        指向一个用于绑定参数的数据结构的指针
*/
/*
整体流程：
    一、非预处理语句
        1、调用mysql_init()初始化数据库
        2、调用mysql_real_connect()连接数据库
        3、调用mysql_set_character_set()设置字符编码
        4、调用mysql_query()执行增删改查
        5、调用mysql_store_result()获得结果集
        6、对于步骤5中使用mysql_store_result()，如果执行的是增删改操作，
           则返回值值为NULL，可以调用mysql_affected_rows()获得受影响的行数
        7、对于步骤5中使用mysql_store_result()，如果执行的是查操作，那么，
           正常情况下，返回值就会是一个指向MYSQL_RES结构的指针,进入步骤8
        8、调用mysql_num_fields()获得数据列数
        9、调用mysql_fetch_field()获得列信息【可重复使用，获得未检索的剩余列字段】
        10、调用mysql_fetch_row()获得一结果行的信息【可重复使用，获得未检索的剩余结果行字段】【配合mysql_num_fields()使用】
        11、调用mysql_free_result()释放结果集
    二、预处理语句(语句中存在未绑定值的语句)
        1、数据库初始化、连接、设置字符编码
        2、调用mysql_stmt_init()放回一个预处理语句句柄
        3、调用mysql_stmt_prepare()将sql语句绑定到预处理语句句柄
        4、调用mysql_stmt_param_count()获得预处理语句中参数标记符的个数
        5、填充MYSQL_BIND数据结构
        6、调用mysql_stmt_bind_param()绑定预处理参数
        7、如果执行的是查操作，有返回的结果集，则调用mysql_stmt_result_metadata()获得以结果集形式返回的预处理元数据，转8。如果不是查询操作，转10
        8、通过获得的预处理元数据，可以判断出结果行的各个列信息，用这些信息填充将来用于接收数据的MYSQL_BIND*对象，转9
        9、调用mysql_stmt_bind_result()将步骤8填充的MYSQL_BIND*绑定到预处理语句上
        10、调用mysql_stmt_execute()，执行预处理语句
        11、如果是查询操作，有输出结果集，
                调用mysql_stmt_store_result()将结果集保存在步骤8填充的MYSQL_BIND*结构中
            如果是增删改操作
                调用mysql_stmt_affected_rows()获取操作影响的列的个数
        12、调用mysql_stmt_close()关闭预处理语句
*/

namespace emilia{
namespace db{

//用于mysql初始化的参数
struct EParam
{
    using ptr = std::shared_ptr<EParam>;

    //从配置文件的Mysql配置中获得map结构，并解析map结构获得相应的配置数据
    EParam(std::map<std::string, std::string>& mysqlParam)
    {
#define XX(str1){   \
    if(mysqlParam.find(#str1) != mysqlParam.end()){ \
        str1 = mysqlParam[#str1];   \
    }   \
}   \

    XX(host);
    XX(user);
    XX(passwd);
    XX(dbname);

    std::string portStr;
    XX(portStr);
    port = atoi(portStr.c_str());

    XX(unixSocket);

    std::string clientFlagStr;
    XX(clientFlagStr);
    clientFlag = atoi(clientFlagStr.c_str());

#undef XX
    }
    //主机地址
    std::string host;

    //用户名
    std::string user;

    //密码
    std::string passwd;

    //要连接的数据库
    std::string dbname;

    //端口，一般填0.MySQL默认端口3306
    unsigned int port;

    //unix_socket 本地套接字，一般为NULL
    std::string unixSocket;
    
    //client_flag连接标志一般为0
    unsigned long clientFlag; 
};

class EMySQLStmt;
class EMySQLData;

class EMySQL : public EDB
             , public std::enable_shared_from_this<EMySQL>
{
friend class EMySQLStmt;
public:
    using ptr = std::shared_ptr<EMySQL>;

    //将sql语句绑定到预处理语句句柄上
    EStmt::ptr prepare(const std::string& sql) override;

    //通过eParam获得配置参数，并通过配置参数完成初始化
    static EDB::ptr Create(EParam::ptr  eParam);

    const EParam::ptr getParam() const { return m_param; }

    const DBType getType() const override { return m_type; }

    //增删改操作
    int execute(const std::string& sql) override;

    //查询操作
    EData::ptr query(const std::string& sql) override;

    const std::string getErrorStr() const override;
private:
    //由EMySQL::Create()内部调用，构造函数
    EMySQL(MYSQL* mysql, EParam::ptr param);
private:
    MYSQL* m_mysql;
    EParam::ptr  m_param;
    DBType m_type;
};

class EMySQLStmt : public EStmt
                 , public std::enable_shared_from_this<EMySQLStmt>
{
friend class EMySQL;
friend class EMySQLData;

public:
    using ptr = std::shared_ptr<EMySQLStmt>;

    //执行增删改操作
    int execute() override;
    
    //执行查询操作
    EData::ptr query() override;

    const DBType getType() const override { return m_mysql->getType(); }

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

    int bindString  (int idx, const char* value) override;
    int bindVString (int idx, const char* value) override;
    int bindText    (int idx, const char* value) override;

    int bindNull    (int idx)                           override;

    //int bindBlob    (int idx, const std::string& value) override;

    int bindBlob    (int idx, const void* value, int64_t size) override;
    int bindTime    (int idx, const time_t& time) override;
    
private:
    //仅由EMySQL的prepare()方法内部调用
    static EStmt::ptr Create(EMySQL::ptr mysql, const std::string& sql);

    //构造函数，仅由EMySQLStmt的Create()方法调用
    EMySQLStmt(EMySQL::ptr mysql, MYSQL_STMT* stmt);
private:
    //预处理语句句柄
    MYSQL_STMT* m_stmt;

    //预处理绑定结构
    std::vector<MYSQL_BIND> m_binds;

    //Stmt对应的数据库
    EMySQL::ptr m_mysql;
};

class EMySQLData : public EData
                 , public std::enable_shared_from_this<EMySQLData>
{
friend class EMySQLStmt;
friend class EMySQL;
public:
    using ptr = std::shared_ptr<EMySQLData>;

    const DBType getType() const override { return DBType::MySQL; }

    bool nextData() override;

    std::string toString() override;

    //结果集总行数
    int getDataCount() override;
    //结果集总列数
    int getColumnCount() override;
    int getColumnBytes(int idx) override;
    int getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

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
    time_t      getTime     (int idx) override;

private:
    //构造函数,仅有EMySQLDatade的Create()方法调用
    EMySQLData(EMySQLStmt::ptr stmt);
    //构造函数，仅有EMySQLDatade的Create()方法调用
    EMySQLData(MYSQL_RES* res);

    //仅由EMySQLStmt的query()方法调用
    static EData::ptr Create(EMySQLStmt::ptr stmt);
    //仅由EMySQL中的query()调用
    static EData::ptr Create(MYSQL_RES* res);
private:
    struct Data{
        Data()
        :isNull(false)
        ,isUnsigned(false)
        ,isError(false)
        ,length(0){}
        
        ~Data(){
            if(data)
                delete [] data;
        }
        
        //分配存储空间的函数
        void alloc(size_t size){
            if(data)
                delete [] data;
            data = new char[size];
            bufferContent = size;
        }

        //当前列数据是否为空
        my_bool isNull;

        //当前列是否有符号
        my_bool isUnsigned;

        //当前列是否出错
        my_bool isError;

        //当前列数据的类型
        enum_field_types type;

        //当前列数据的真实长度
        unsigned long length;

        //当前列缓存的长度
        int32_t bufferContent;

        //当前列数据的客户端缓存
        char* data;
    };
private:
    //与数据对应的预处理语句
    EMySQLStmt::ptr m_stmt;

    //结果集在客户端的缓存
    std::vector<Data> m_datas;

    //用于绑定输出结果的MYSQL_BIND*
    std::vector<MYSQL_BIND> m_results;

    //非预处理查询语句的结果集
    MYSQL_RES* m_noStmtRes;

    //结果集的每一行
    MYSQL_ROW m_row;

    //每一行中列的长度
    unsigned long* m_rowLength;

    //结果集的列信息
    MYSQL_FIELD* m_field;

    //结果集的列数
    int m_fieldNums;

    //结果集的行数
    int m_rowNums;
};

}
}
#endif
