#ifndef _EMILIA_E_PUBDAO_H_
#define _EMILIA_E_PUBDAO_H_

#include <memory>
#include <vector>
#include <boost/any.hpp>
#include "emilia/db/db.h"

namespace emilia{
namespace orm{

class EPUBDao{
public:
    using ptr = std::shared_ptr<EPUBDao>;

    //bdDDTypes对应于bindDatas中的数据的类型
    //bindDatas为预处理语句待绑定值
    //返回值:-1出错 >=0 成功(并且值为sql影响的行数)
    static int Execute(const db::EDB::ptr conn, const std::string& sql
                      ,const std::vector<db::EDDType>& bdDDTypes
                      ,const std::vector<boost::any>& bindDatas);

    //bdDDTypes对应于bindDatas中的数据的类型
    //bindDatas为预处理语句待绑定值
    //rtDDTypes对应于结果集中每一列的类类型
    //rtDatas对应于结果集
    //放回值:-1出错 >=0 成功
    static int Query(const db::EDB::ptr conn, const std::string& sql
                    ,const std::vector<db::EDDType>& bdDDTypes
                    ,const std::vector<boost::any>& bindDatas
                    ,const std::vector<db::EDDType>& rtDDTypes
                    ,std::vector<std::vector<boost::any> >& rtDatas);

    //返回值:-1出错 >=0 成功(并且值为sql影响的行数)
    static int Execute(const db::EDB::ptr conn, const std::string& sql);

    //rtDDTypes对应于结果集中每一列的类类型
    //rtDatas对应于结果集
    //放回值:-1出错 >=0 成功
    static int Query(const db::EDB::ptr conn, const std::string& sql
                    ,const std::vector<db::EDDType>& rtDDTypes
                    ,std::vector<std::vector<boost::any> >& rtDatas);
};

} 
}

#endif