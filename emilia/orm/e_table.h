#ifndef _EMILIA_TABLE_H_
#define _EMILIA_TABLE_H_

#include <string>
#include <memory>
#include <fstream>
#include "e_index.h"
#include "e_column.h"
#include "emilia/db/db.h"
#include <tinyxml2.h>

namespace emilia{
namespace orm{

class ETable{

public:
    using ptr = std::shared_ptr<ETable>;

    const std::string& getName() const { return m_name; }
    const std::string& getSuffix() const { return m_suffix; }
    const std::vector<std::string>& getNamespace() const { return m_namespace; }
    const std::string& getComment() const { return m_comment; }
    const std::vector<db::DBType>& getDBType() const { return m_types; }

    const std::vector<EColumn::ptr>& getColumns() const { return m_cols; }
    const std::vector<EIndex::ptr>& getIndexs() const { return m_indexs; }

    static ETable::ptr Create(const tinyxml2::XMLElement& root);

public:
    void gen(const std::string& prePath);
private:
    //生成头文件内容
    void genHeaderFile(const std::string& prePath);
    //生成源文件内容
    void genSourceFile(const std::string& prePath);

private:
    //生成头文件中的Dao类(genHeaderFile调用)
    void genHeaderDao(std::ofstream& of);

    void genSingleInsert(std::ofstream& of);
    void genBatchInsert(std::ofstream& of);

    void genDeleteByPK(std::ofstream& of);
    void genDeleteByUK(std::ofstream& of);

    void genUpdate(std::ofstream& of);

    void genQueryAll(std::ofstream& of);
    void genQueryByPK(std::ofstream& of);
    void genQueryByUK(std::ofstream& of);
    
    void genCreateMySQL(std::ofstream& of);
    void genCreateSQLite3(std::ofstream& of);

private:
    //表格名称
    std::string m_name;
    //后缀信息(用于构成文件名和类名)
    std::string m_suffix = "_info";
    //表格对应代码的命名域
    std::vector<std::string> m_namespace;
    //表格注释
    std::string m_comment;
    //表格使用的数据库类型
    std::vector<db::DBType> m_types;

    //表格列信息
    std::vector<EColumn::ptr> m_cols;
    //表格索引信息
    std::vector<EIndex::ptr> m_indexs;

    //数据映射类名称
    std::string m_classDataName;
    //Dao类名称
    std::string m_classDaoName;
    //抽象数据库接口类名
    std::string m_classDBName = "emilia::db::EDB";

    //表格主键
    std::string m_pkName;
    //主键类型
    std::string m_pkType;
    //表格唯一索引
    std::vector<std::vector<std::string> > m_uniqueName;
    //唯一索引类型
    std::vector<std::vector<std::string> > m_uniqueType;
    

};

}
}

#endif