#include "e_mysql.h"
#include <cstring>
#include "emilia/util/stringutil.h"

namespace emilia{
namespace db{

//=========================================================
//class EMySQL
EStmt::ptr EMySQL::prepare(const std::string& sql)
{
    return EMySQLStmt::Create(shared_from_this(), sql); 
}

EDB::ptr EMySQL::Create(EParam::ptr  eParam)
{
    //获得初始化的mysql句柄指针
    MYSQL* mysql = mysql_init(nullptr);

    mysql = mysql_real_connect(mysql
                            ,eParam->host.c_str()
                            ,eParam->user.c_str()
                            ,eParam->passwd.c_str()
                            ,eParam->dbname.c_str()
                            ,eParam->port
                            ,eParam->unixSocket.c_str()
                            ,eParam->clientFlag);

    //mysql_real_connect()失败返回null
    if(!mysql)
        return nullptr;
    return std::dynamic_pointer_cast<EDB>(EMySQL::ptr (new EMySQL(mysql, eParam)));
}

int EMySQL::execute(const std::string& sql){
    //mysql_query()成功返回0，失败返回非0
    if(mysql_query(m_mysql, sql.c_str()))
        return -1;
    return mysql_affected_rows(m_mysql);
}

EData::ptr EMySQL::query(const std::string& sql){
    //mysql_query()成功返回0，失败返非0
    if(mysql_query(m_mysql, sql.c_str()))
        return nullptr;
    //查询成功，调用mysql_store_result()获得结果集
    //mysql_store_result()成功返回非NULL，失败返回NULL
    MYSQL_RES* res = mysql_store_result(m_mysql);
    if(!res){
        //此时外部应该调用getError()来获得错误信息
        return nullptr;
    }
    return EMySQLData::Create(res);
}

const std::string EMySQL::getErrorStr() const{
    return mysql_error(m_mysql);
}

EMySQL::EMySQL(MYSQL* mysql, EParam::ptr param)
:m_mysql(mysql)
,m_param(param)
,m_type(DBType::MySQL)
{}
//========================================================
//class EMySQlStmt
int EMySQLStmt::execute()
{
    //mysql_stmt_bind_param()成功返回0，失败返回非0
    //将MYSQL_BIND绑定到MYSQL_STMT上
    if(mysql_stmt_bind_param(m_stmt, &m_binds[0]))
        return -1;
    
    //mysql_stmt_execute()成功返回0，失败返回非0
    //执行MYSQL_STMT
    if(mysql_stmt_execute(m_stmt))
        return -1;

    return mysql_stmt_affected_rows(m_stmt);
}

EData::ptr EMySQLStmt::query()
{
    if(mysql_stmt_bind_param(m_stmt, &m_binds[0]))
        return nullptr;

    return EMySQLData::Create(shared_from_this());
}

int EMySQLStmt::bindInt8(int idx, const int8_t&  value)
{
    //buffer_type:数据类型
    m_binds[--idx].buffer_type = MYSQL_TYPE_TINY;
    //为buffer分配空间
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    //将value拷贝到buffer中
    memcpy(m_binds[idx].buffer, &value, valSize);
    //is_unsigned:是否无符号
    m_binds[idx].is_unsigned = false;
    //buffer_length:缓存区长度
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindUint8(int idx, const uint8_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_TINY;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = true;
    m_binds[idx].buffer_length = valSize;
    return 0;
}
int EMySQLStmt::bindInt16(int idx, const int16_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_SHORT;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = false;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindUint16(int idx, const uint16_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_SHORT;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = true;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindInt32(int idx, const int32_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_LONG;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = false;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindUint32(int idx, const uint32_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_LONG;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = true;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindInt64(int idx, const int64_t& value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_LONGLONG;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = false;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindUint64(int idx, const uint64_t& value)    
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_LONGLONG;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].is_unsigned = true;
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindFloat(int idx, const float&  value)      
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_FLOAT;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindDouble(int idx, const double& value)      
{   
    m_binds[--idx].buffer_type = MYSQL_TYPE_DOUBLE;
    size_t valSize = sizeof(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

/*
int EMySQLStmt::bindString(int idx, const std::string& value) 
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_STRING;
    size_t valSize = value.size();
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value.c_str(), valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}
*/

int EMySQLStmt::bindString(int idx, const char* value)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_STRING;
    size_t valSize = strlen(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindVString (int idx, const char* value){
    m_binds[--idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    size_t valSize = strlen(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindText(int idx, const char* value){
    //MYSQL_TYPE_BLOB指代Blob类型或者Text类型
    m_binds[--idx].buffer_type = MYSQL_TYPE_BLOB;
    size_t valSize = strlen(value);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindTime(int idx, const time_t& value){
    m_binds[--idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
    size_t valSize = sizeof(time_t);
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, &value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

int EMySQLStmt::bindNull(int idx)                           
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_NULL;
    return 0;
}

/*
int EMySQLStmt::bindBlob(int idx, const std::string& value) 
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_BLOB;
    size_t valSize = value.size();
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value.c_str(), valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}
*/

int EMySQLStmt::bindBlob(int idx, const void* value, int64_t size)
{
    m_binds[--idx].buffer_type = MYSQL_TYPE_BLOB;
    size_t valSize = size;
    m_binds[idx].buffer = malloc(valSize);
    memcpy(m_binds[idx].buffer, value, valSize);
    m_binds[idx].buffer_length = valSize;
    return 0;
}

EStmt::ptr EMySQLStmt::Create(EMySQL::ptr mysql, const std::string& sql)
{
    //调用mysql_stmt_init()获得一个初始化的MYSQL_STMT句柄
    MYSQL_STMT * stmt = mysql_stmt_init(mysql->m_mysql);
    if(!stmt)
        return nullptr;

    //调用mysql_stmt_prepare()将sql语句绑定到stmt上
    //成功返回0，失败返回非0
    if(mysql_stmt_prepare(stmt, sql.c_str(), sql.size()))
        return nullptr;

    //调用mysql_stmt_param_count()获得未绑定的变量的个数
    int count = mysql_stmt_param_count(stmt);
    EMySQLStmt::ptr rt(new EMySQLStmt(mysql, stmt));
    rt->m_binds.resize(count);
    //初始化MYSQL_BIND结构
    memset(&rt->m_binds[0], 0, count*sizeof(MYSQL_BIND));
 
    return std::dynamic_pointer_cast<EStmt>(rt);
}

EMySQLStmt::EMySQLStmt(EMySQL::ptr mysql, MYSQL_STMT* stmt)
:m_stmt(stmt)
,m_mysql(mysql)
{}


//========================================================
//class EMySQLData
EMySQLData::EMySQLData(EMySQLStmt::ptr stmt)
:m_stmt(stmt)
,m_noStmtRes(nullptr)
{}

EMySQLData::EMySQLData(MYSQL_RES* noStmtRes)
:m_stmt(nullptr)
,m_noStmtRes(noStmtRes)
{}

EData::ptr EMySQLData::Create(EMySQLStmt::ptr stmt)
{
    //通过mysql_stmt_result_metadata()获得元数据(描述结果集的数据)
    MYSQL_RES* metaRes = mysql_stmt_result_metadata(stmt->m_stmt);

    if(!metaRes)
        return nullptr;

    EMySQLData::ptr result(new EMySQLData(stmt));

    //通过mysql_num_fields()获得元数据所描述的结果集的列数
    result->m_fieldNums = mysql_num_fields(metaRes);
    //通过mysql_fetch_fields()获得元数据所描述的结果集的列信息
    result->m_field = mysql_fetch_fields(metaRes);

    //填充m_results
    int num = result->m_fieldNums;
    MYSQL_FIELD* field = result->m_field;

    result->m_datas.resize(num);
    result->m_results.resize(num);

    for(int i = 0; i < num; i++)
    {
        result->m_datas[i].type = field[i].type;

//mysqlType mysql中数据的类型
//cType     c中的数据类型
#define xx(mysqlType, cppType)  \
    case mysqlType : \
        result->m_datas[i].alloc(sizeof(cppType)); \
        break;  \
        
        //为m_datas分配空间
        switch(field[i].type)
        {

            xx(MYSQL_TYPE_TINY, int8_t);
            xx(MYSQL_TYPE_SHORT, int16_t);
            xx(MYSQL_TYPE_LONG, int32_t);
            xx(MYSQL_TYPE_LONGLONG, int64_t);
            xx(MYSQL_TYPE_FLOAT, float);
            xx(MYSQL_TYPE_DOUBLE, double);
            xx(MYSQL_TYPE_TIMESTAMP, MYSQL_TIME);
            xx(MYSQL_TYPE_DATETIME, MYSQL_TIME);
            xx(MYSQL_TYPE_DATE, MYSQL_TIME);
            xx(MYSQL_TYPE_TIME, MYSQL_TIME);
            default:
                result->m_datas[i].alloc(field[i].length);
                break;
        }
#undef xx

        //填充对应的m_results结构
        result->m_results[i].buffer_type = result->m_datas[i].type;
        result->m_results[i].buffer = result->m_datas[i].data;
        result->m_results[i].buffer_length = result->m_datas[i].bufferContent;
        result->m_results[i].length = &result->m_datas[i].length;
        result->m_results[i].is_null = &result->m_datas[i].isNull;
        result->m_results[i].error = &result->m_datas[i].isError;
    }

    //调用mysql_stmt_bind_result(),m_reuslts用于存放输出结果
    if(mysql_stmt_bind_result(stmt->m_stmt, &result->m_results[0]))
        return nullptr;

    //执行mysql_stmt_execute(),在MySQL服务端形成结果集
    if(mysql_stmt_execute(stmt->m_stmt))
        return nullptr;

    //调用mysql_stmt_store_result(),将结果集保存到客户端上
    if(mysql_stmt_store_result(stmt->m_stmt))
        return nullptr;

    //获得结果集的行数
    result->m_rowNums = mysql_stmt_num_rows(stmt->m_stmt);

    return std::dynamic_pointer_cast<EData>(result);
}

EData::ptr EMySQLData::Create(MYSQL_RES* res)
{
    EMySQLData::ptr result (new EMySQLData(res));

    result->m_fieldNums = mysql_num_fields(res);
    result->m_field = mysql_fetch_fields(res);

    return std::dynamic_pointer_cast<EData>(result);
}

bool EMySQLData::nextData()
{
    //是由EMySQL直接调用的query()产生的Data
    if(m_noStmtRes != nullptr){
        m_row = mysql_fetch_row(m_noStmtRes);
        m_rowLength = mysql_fetch_lengths(m_noStmtRes);
        return m_row == nullptr ? false : true;
    }
    //预处理语句产生的Data
    else{
        //调用mysql_stmt_fetch()将结果集的数据放入mysql_stmt_bind_result()所指定的数据缓冲中
        return !mysql_stmt_fetch(m_stmt->m_stmt);
    }
}

std::string EMySQLData::toString()
{
    return " ";
}

int EMySQLData::getDataCount()
{
    return m_rowNums;
}

int EMySQLData::getColumnCount()
{
    return m_fieldNums;
}

int EMySQLData::getColumnBytes(int idx)
{
    return m_field[idx].length;
}

int EMySQLData::getColumnType(int idx)
{
    return m_field[idx].type;
}

std::string EMySQLData::getColumnName(int idx)
{
    char* name = m_field[idx].name;
    unsigned int nameLength = m_field[idx].name_length;

    return std::string(name, nameLength);
}

int8_t      EMySQLData::getInt8     (int idx)
{
    //结果集不是通过预处理语句产生
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToInt8(m_row[idx]);
    //结果集通过预处理语句产生
    else
        return util::StringUtil::ToInt8(m_datas[idx].data);
}

uint8_t     EMySQLData::getUint8    (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToUint8(m_row[idx]);
    else
        return util::StringUtil::ToUint8(m_datas[idx].data);
}

int16_t     EMySQLData::getInt16    (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToInt16(m_row[idx]);
    else
        return util::StringUtil::ToInt16(m_datas[idx].data);
}

uint16_t    EMySQLData::getUint16   (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToUint16(m_row[idx]);
    else
        return util::StringUtil::ToUint16(m_datas[idx].data);
}

int32_t     EMySQLData::getInt32    (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToInt32(m_row[idx]);
    else
        return util::StringUtil::ToInt32(m_datas[idx].data);
}

uint32_t    EMySQLData::getUint32   (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToUint32(m_row[idx]);
    else
        return util::StringUtil::ToUint32(m_datas[idx].data);
}

int64_t     EMySQLData::getInt64    (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToInt64(m_row[idx]);
    else
        return util::StringUtil::ToInt64(m_datas[idx].data);
}

uint64_t    EMySQLData::getUint64   (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToUint64(m_row[idx]);
    else
        return util::StringUtil::ToUint64(m_datas[idx].data);
}

float       EMySQLData::getFloat    (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToFloat(m_row[idx]);
    else
        return util::StringUtil::ToFloat(m_datas[idx].data);
}

double      EMySQLData::getDouble   (int idx)
{
    if(m_noStmtRes != nullptr)
        return util::StringUtil::ToDouble(m_row[idx]);
    else
        return util::StringUtil::ToDouble(m_datas[idx].data);
}

std::string EMySQLData::getString   (int idx)
{
    if(m_noStmtRes != nullptr)
        return std::string(m_row[idx], m_rowLength[idx]);
    else
        return std::string(m_datas[idx].data, m_datas[idx].length);
}

bool        EMySQLData::isNull      (int idx)
{
    if(m_noStmtRes != nullptr){
        if(strlen(m_row[idx]) == 0)
            return true;
        return false;
    }
    else{
        if(m_datas[idx].isNull)
            return true;
        return false;
    }
}

std::string EMySQLData::getBlob     (int idx)
{
    if(m_noStmtRes != nullptr)
        return std::string(m_row[idx], m_rowLength[idx]);
    else
        return std::string(m_datas[idx].data, m_datas[idx].length);
}

time_t      EMySQLData::getTime     (int idx)
{
    return (time_t)getInt32(idx);
}

}
}
