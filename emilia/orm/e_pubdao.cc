#include "e_pubdao.h"
#include "log.h"

namespace emilia{
namespace orm{




//ddTypes对应于bindDatas中的数据的类型
//bindDatas为预处理语句待绑定值
//返回值:-1出错 >=0 成功(并且值为sql影响的行数)
int EPUBDao::Execute(const db::EDB::ptr conn, const std::string& sql
                    ,const std::vector<db::EDDType>& bdDDTypes
                    ,const std::vector<boost::any>& bindDatas){
    db::EStmt::ptr eStmt = conn->prepare(sql);
    if(eStmt == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }
    size_t bDSize = bindDatas.size();
    for(size_t i = 0; i < bDSize; i++){
        switch(bdDDTypes[i]){
            case db::EDDType::E_DD_INT8 :
                eStmt->bindInt8(i, boost::any_cast<int8_t>(bindDatas[i]));
                break;
            //....
        }
    }
    
    return eStmt->execute();
}

//ddTypes对应于结果集中每一列的类类型
//rtDatas对应于结果集
//放回值:-1出错 >=0 成功
int EPUBDao::Query(const db::EDB::ptr conn, const std::string& sql
                ,const std::vector<db::EDDType>& bdDDTypes
                ,const std::vector<boost::any>& bindDatas
                ,const std::vector<db::EDDType>& rtDDTypes
                ,std::vector<std::vector<boost::any> >& rtDatas){
    db::EStmt::ptr eStmt = conn->prepare(sql);
    if(eStmt == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }
    //绑定同上
    //...

    //获取数据
    db::EData::ptr eData = eStmt->query();
    if(eData == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }
    size_t rtSize = rtDDTypes.size();
    while(eData->nextData()){
        std::vector<boost::any> rt;
        for(size_t z = 0; z < rtSize; z++){
            switch(rtDDTypes[z]){
                case db::EDDType::E_DD_INT8 :
                    rt.push_back(eData->getInt8(z));
                    break;
                //...
            }
        }
        rtDatas.push_back(rt);
    }
    return 0;
}

//返回值:-1出错 >=0 成功(并且值为sql影响的行数)
int EPUBDao::Execute(const db::EDB::ptr conn, const std::string& sql){
    db::EStmt::ptr eStmt = conn->prepare(sql);
    if(eStmt == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }
    return eStmt->execute();
}

//rtDDTypes对应于结果集中每一列的类类型
//rtDatas对应于结果集
//放回值:-1出错 >=0 成功
int EPUBDao::Query(const db::EDB::ptr conn, const std::string& sql
                  ,const std::vector<db::EDDType>& rtDDTypes
                  ,std::vector<std::vector<boost::any> >& rtDatas){
    db::EStmt::ptr eStmt = conn->prepare(sql);
    if(eStmt == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }
    db::EData::ptr eData = eStmt->query();
    if(eData == nullptr){
        EMILIA_LOG_ERROR("orm") << conn->getErrorStr();
        return -1;
    }

    size_t rtSize = rtDDTypes.size();
    while(eData->nextData()){
        std::vector<boost::any> rt;
        for(size_t z = 0; z < rtSize; z++){
            switch(rtDDTypes[z]){
                case db::EDDType::E_DD_INT8 :
                    rt.push_back(eData->getInt8(z));
                    break;
                //...
            }
        }
        rtDatas.push_back(rt);
    }
    return 0;
}

}
}