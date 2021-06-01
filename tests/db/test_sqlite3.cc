#include "emilia/db/e_sqlite3.h"
#include "emilia/db/db.h"
#include "emilia/log/logmarco.h"

using namespace emilia;

int main()
{

    emilia::db::EDB::ptr edb = emilia::db::ESQLite3::Create("test.db");

    std::string sql_drop = "DROP TABLE COMPANY";

    int acLine = edb->execute(sql_drop);
    if(acLine == -1)
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();

    std::string sql_create = "CREATE TABLE COMPANY("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                             "NAME VARCHAR(8) NOT NULL,"
                             "AGE INTEGER NOT NULL,"
                             "ADDRESS VARCHAR(30),"
                             "SALARY REAL);";

    acLine = edb->execute(sql_create);
    if(acLine == -1){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
    }

    std::string sql_insert = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
                             "VALUES (1, 'Paul', 32, 'California', 20000.00 );";

    EMILIA_LOG_ERROR("test") << "wuhu";
    
    acLine = edb->execute(sql_insert);
    if(acLine == -1){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
    }
    
    std::string sql_insert_pre = "INSERT INTO COMPANY (NAME,AGE,ADDRESS,SALARY)"
                                 "VALUES (?, ?, ?, ?);";    

    EMILIA_LOG_ERROR("test") << "wuhu";

    emilia::db::EStmt::ptr eStmt = edb->prepare(sql_insert_pre);
    if(eStmt == nullptr){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
        return 0;
    }

    eStmt->bindVString(1, "XHW");
    eStmt->bindInt32(2, 19);
    eStmt->bindVString(3, "江苏南通");
    eStmt->bindDouble(4, 20000.0);

    EMILIA_LOG_ERROR("test") << "wuhu";

    acLine = eStmt->execute();
    if(acLine == -1){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
        return 0;
    }

    EMILIA_LOG_ERROR("test") << "wuhu";

    std::string sql_query = "select * from COMPANY where id = ?";
    emilia::db::EStmt::ptr eStmt_query = edb->prepare(sql_query);
    if(eStmt_query == nullptr){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
        return 0;
    }

    eStmt_query->bindInt32(1, 2);

    emilia::db::EData::ptr eData = eStmt_query->query();
    if(eData == nullptr){
        EMILIA_LOG_ERROR("test") << edb->getErrorStr();
        return 0;
    }

    EMILIA_LOG_ERROR("test") << "wuhu";

    while(eData->nextData()){
        EMILIA_LOG_ERROR("test") << "ID: " << eData->getInt32(0);
        EMILIA_LOG_ERROR("test") << "NAME: " << eData->getString(1);
        EMILIA_LOG_ERROR("test") << "AGE: " << eData->getInt32(2);
        EMILIA_LOG_ERROR("test") << "ADDRESS: " << eData->getString(3);
        EMILIA_LOG_ERROR("test") << "SALARY: " << eData->getDouble(4);
    }

    return 0;
}
