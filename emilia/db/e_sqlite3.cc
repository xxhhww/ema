#include "e_sqlite3.h"
#include "db.h"
#include "emilia/log/logmarco.h"
#include <cstring>

namespace emilia{
namespace db{

//=====================================================
//ESQLite3

//close()方法待考虑
EDB::ptr ESQLite3::Create(const std::string& dbname, int flags)
{
    sqlite3* sqlite3Ptr;
    //sqlite3_open_v2()内部给sqlite3Ptr动态分配了资源，析构函数中要释放
    if( sqlite3_open_v2(dbname.c_str(), &sqlite3Ptr, flags, nullptr) == SQLITE_OK )
        return std::dynamic_pointer_cast<EDB>(ESQLite3::ptr (new ESQLite3(sqlite3Ptr, DBType::SQLite3)));
    //创建失败
    return nullptr;
}
    
ESQLite3::~ESQLite3()
{
    sqlite3_close(m_db);
}

int ESQLite3::execute(const std::string& sql)
{
    if(sqlite3_exec(m_db, sql.c_str(), 0, 0, 0))
        return -1;
    return sqlite3_changes(m_db);
}

EData::ptr ESQLite3::query(const std::string& sql)
{
    auto stmt = ESQLite3Stmt::Create(shared_from_this(), sql);
    if(!stmt)
        return nullptr;
    return stmt->query();
}

EStmt::ptr ESQLite3::prepare(const std::string& sql)
{
    EMILIA_LOG_INFO("test") << "test";
    return ESQLite3Stmt::Create(shared_from_this(), sql);
}

 const std::string ESQLite3::getErrorStr() const
 {
    return sqlite3_errmsg(m_db);
 }

ESQLite3::ESQLite3(sqlite3* db, DBType type)
:m_db(db)
,m_type(type)
{}


/*
int ESQLite3::addCbFunc(const std::string& name, int nArg, int eTextRep, void* pApp,
        std::function<void(sqlite3_context*, int, sqlite3_value**)> xFunc,
        std::function<void(sqlite3_context*, int, sqlite3_value**)> xStep,
        std::function<void(sqlite3_context*)> xFinal,
        std::function<void(void*)> xDestroy)
{
    int ret = sqlite3_create_function_v2(m_db, name.c_str(), nArg, eTextRep, pApp,
                                         xFunc, xStep, xFinal, xDestroy);
    if( ret != SQLITE_OK )
    {
        EMILIA_LOG_ERROR("system") << "sqlite3_create_function_v2 error for "
                                   << name << " " << sqlite3_errmsg(m_db);
    }
    return 0;
}

int ESQLite3::addCpFunc(const std::string& name, int eTextRep, void* pApp,
        std::function<int(void*, int, const void*, int, const void*)> xComp,
        std::function<void(void*)> xDestory)
{
    int ret = sqlite3_create_collation_v2(m_db, name.c_str(), eTextRep, pApp, xComp, xDestory);
    if( ret != SQLITE_OK )
    {
        EMILIA_LOG_ERROR("system") << "sqlite3_create_collation_v2 error for "
                                   << name << " " << sqlite3_errmsg(m_db);
    }
    return 0;
}
*/

//=====================================================
//ESQLiteStmt

//按名绑定未做，待考虑
ESQLite3Stmt::~ESQLite3Stmt()
{
    sqlite3_finalize(m_stmt);
}

int ESQLite3Stmt::execute()
{
    EMILIA_LOG_INFO("test") << "test";
    if(step() != SQLITE_DONE)
        return -1;
    return sqlite3_changes(m_db->m_db);
}

EData::ptr ESQLite3Stmt::query()
{
    return ESQLite3Data::Create(shared_from_this());
}

int ESQLite3Stmt::step()
{
    EMILIA_LOG_INFO("test") << "test";
    return sqlite3_step(m_stmt);
}

int ESQLite3Stmt::bindInt8    (int idx, const int8_t&  value)     
{
   return sqlite3_bind_int(m_stmt, idx, (int32_t)value); 
}

int ESQLite3Stmt::bindUint8   (int idx, const uint8_t& value)     
{
    return sqlite3_bind_int(m_stmt, idx, (int32_t)value);
}

int ESQLite3Stmt::bindInt16   (int idx, const int16_t& value)     
{
    return sqlite3_bind_int(m_stmt, idx, (int32_t)value);
}

int ESQLite3Stmt::bindUint16  (int idx, const uint16_t& value)    
{
    return sqlite3_bind_int(m_stmt, idx, (int32_t)value);
}

int ESQLite3Stmt::bindInt32   (int idx, const int32_t& value)     
{
    return sqlite3_bind_int(m_stmt, idx, value);
}

int ESQLite3Stmt::bindUint32  (int idx, const uint32_t& value)    
{
    return sqlite3_bind_int(m_stmt, idx, (int32_t)value);
}

int ESQLite3Stmt::bindInt64   (int idx, const int64_t& value)     
{
    return sqlite3_bind_int64(m_stmt, idx, value);
}

int ESQLite3Stmt::bindUint64  (int idx, const uint64_t& value)    
{
    return sqlite3_bind_int64(m_stmt, idx, (int64_t)value);
}

int ESQLite3Stmt::bindFloat   (int idx, const float&  value)      
{
    return sqlite3_bind_double(m_stmt, idx, (double)value);
}

int ESQLite3Stmt::bindDouble  (int idx, const double& value)      
{
    return sqlite3_bind_double(m_stmt, idx, value);
}

/*
int ESQLite3Stmt::bindString  (int idx, const std::string& value) 
{
    //最后一个参数，待考虑，先使用SQLITE_TRANSIENT
    //SQLITE_STATIC告诉sqlite3_bind_text函数字符串为常量，可以放心使用；
    //而SQLITE_TRANSIENT会使得sqlite3_bind_text函数对字符串做一份拷贝
    return sqlite3_bind_text(m_stmt, idx, value.c_str(), value.size(), SQLITE_TRANSIENT);
}
*/

int ESQLite3Stmt::bindString  (int idx, const char* value)
{
    return sqlite3_bind_text(m_stmt, idx, value, strlen(value), SQLITE_TRANSIENT);
}

int ESQLite3Stmt::bindVString (int idx, const char* value){
    return sqlite3_bind_text(m_stmt, idx, value, strlen(value), SQLITE_TRANSIENT);
}

int ESQLite3Stmt::bindText (int idx, const char* value){
    return sqlite3_bind_text(m_stmt, idx, value, strlen(value), SQLITE_TRANSIENT);
}

int ESQLite3Stmt::bindNull    (int idx)                           
{
    return sqlite3_bind_null(m_stmt, idx);
}

/*
int ESQLite3Stmt::bindBlob    (int idx, const std::string& value) 
{
    //最后一个参数，待考虑
    return sqlite3_bind_blob(m_stmt, idx, (const void*)value.c_str(), value.size(), SQLITE_TRANSIENT);
}
*/

int ESQLite3Stmt::bindBlob    (int idx, const void* value, int64_t size)
{
    //最后一个参数，待考虑
    return sqlite3_bind_blob(m_stmt, idx, value, size, SQLITE_TRANSIENT);
}

int ESQLite3Stmt::bindTime(int idx, const time_t& value){
    return sqlite3_bind_int(m_stmt, idx, (int)value);
}

EStmt::ptr ESQLite3Stmt::Create(ESQLite3::ptr db, const std::string& sql)
{
    sqlite3_stmt* stmtPtr;
    //内部为stmtPtr动态分配资源，析构函数中要释放
    EMILIA_LOG_INFO("test") << "test";
    if(sqlite3_prepare_v2(db->m_db, sql.c_str(), sql.size(), &stmtPtr, nullptr) == SQLITE_OK){
        EMILIA_LOG_INFO("test") << "test";
        return  std::dynamic_pointer_cast<EStmt>(ESQLite3Stmt::ptr(new ESQLite3Stmt(db, stmtPtr)));
    }
    EMILIA_LOG_INFO("test") << "test " << db->getErrorStr();
    return nullptr;
}
ESQLite3Stmt::ESQLite3Stmt(ESQLite3::ptr db, sqlite3_stmt* stmt)
:m_db(db)
,m_stmt(stmt)
{}

//=====================================================
//ESQLite3Data
ESQLite3Data::~ESQLite3Data()
{
    EMILIA_LOG_INFO("test") << "wuhu";
}

//true表明还有数据，false表明没有数据或者查询出错
bool ESQLite3Data::nextData()                   
{
    return m_stmt->step() == SQLITE_ROW ? true : false;
}

std::string ESQLite3Data::toString()
{
    //输出格式：
    //当前列名 ： 当前列值\n
    return "";
}

int ESQLite3Data::getDataCount    ()
{
    return sqlite3_data_count(m_stmt->m_stmt);
}
        
int ESQLite3Data::getColumnCount  ()
{
    return sqlite3_column_count(m_stmt->m_stmt);
} 
         
int ESQLite3Data::getColumnBytes  (int idx)
{
    return sqlite3_column_bytes(m_stmt->m_stmt, idx);
}

int ESQLite3Data::getColumnType   (int idx)
{
    return sqlite3_column_type(m_stmt->m_stmt, idx);
}

std::string ESQLite3Data::getColumnName(int idx)
{
    const char* name = sqlite3_column_name(m_stmt->m_stmt, idx);
    return name;
}

int8_t      ESQLite3Data::getInt8     (int idx)
{
    return getInt32(idx);
} 
uint8_t     ESQLite3Data::getUint8    (int idx) 
{
    return getInt32(idx);
}
int16_t     ESQLite3Data::getInt16    (int idx) 
{
    return getInt32(idx);
}
uint16_t    ESQLite3Data::getUint16   (int idx) 
{
    return getInt32(idx);
}
int32_t     ESQLite3Data::getInt32    (int idx) 
{
    return sqlite3_column_int(m_stmt->m_stmt, idx);
}

uint32_t    ESQLite3Data::getUint32   (int idx) 
{
    return getInt32(idx);
}

int64_t     ESQLite3Data::getInt64    (int idx) 
{
    return sqlite3_column_int64(m_stmt->m_stmt, idx);
}

uint64_t    ESQLite3Data::getUint64   (int idx) 
{
    return getInt64(idx);
}

float       ESQLite3Data::getFloat    (int idx) 
{
    return getDouble(idx);
}

double      ESQLite3Data::getDouble   (int idx) 
{
    return sqlite3_column_double(m_stmt->m_stmt, idx);
}

std::string ESQLite3Data::getString   (int idx) 
{
    const char* data = (const char*)sqlite3_column_text(m_stmt->m_stmt, idx);
    return std::string(data, getColumnBytes(idx));
}

bool        ESQLite3Data::isNull      (int idx) 
{
    return false;
}

std::string ESQLite3Data::getBlob     (int idx)
{
    const char* data = (const char*)sqlite3_column_blob(m_stmt->m_stmt, idx);
    return std::string(data, getColumnBytes(idx));
}

time_t ESQLite3Data::getTime      (int idx)
{
    return (time_t)sqlite3_column_int(m_stmt->m_stmt, idx);
}

EData::ptr ESQLite3Data::Create(ESQLite3Stmt::ptr stmt)
{
    return std::dynamic_pointer_cast<EData>(ESQLite3Data::ptr (new ESQLite3Data(stmt)));
}

ESQLite3Data::ESQLite3Data(ESQLite3Stmt::ptr stmt)
:m_stmt(stmt)
{}
//=====================================================
}
}
