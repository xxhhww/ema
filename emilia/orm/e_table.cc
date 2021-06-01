#include "log.h"
#include "emilia/util/stringutil.h"
#include "emilia/util/fileutil.h"
#include "e_table.h"

namespace emilia{
namespace orm{

ETable::ptr ETable::Create(const tinyxml2::XMLElement& root){
    ETable::ptr table(new ETable);

#define XX(str) \
    if(!root.Attribute(str)){ \
        EMILIA_LOG_ERROR("orm") << "Table " << str << " Not Exist"; \
        return nullptr; \
    } \

    XX("name");
    table->m_name = root.Attribute("name");

    XX("namespace");
    table->m_namespace = util::StringUtil::Split(root.Attribute("namespace"), '.');

    XX("dbtype");
    //dbtypes = "mysql,sqlite3"
    std::vector<std::string> dbtypes = util::StringUtil::Split(root.Attribute("dbtype"), ',');
    for(auto& i : dbtypes)
        table->m_types.push_back(db::StringToDBType(i));
    
    if(root.Attribute("comment"))
        table->m_comment = root.Attribute("comment");

#undef XX

    //填充列
    const tinyxml2::XMLElement* nodeCols = root.FirstChildElement();
    if(!nodeCols)
        EMILIA_LOG_ERROR("orm") << "Columns Not Exist";
    
    const tinyxml2::XMLElement* nodeCol = nodeCols->FirstChildElement();
    if(!nodeCol)
        EMILIA_LOG_ERROR("orm") << "Columns Are Null";

    while(nodeCol){
        table->m_cols.push_back(EColumn::Create(*nodeCol));
        nodeCol = nodeCol->NextSiblingElement();
    }

    //填充索引
    const tinyxml2::XMLElement* nodeIndexs = nodeCols->NextSiblingElement();
    if(!nodeIndexs)
        EMILIA_LOG_ERROR("orm") << "Indexs Not Exist";

    const tinyxml2::XMLElement* nodeIndex = nodeIndexs->FirstChildElement();
    if(!nodeIndex)
        EMILIA_LOG_ERROR("orm") << "Indexs Are Null";
    
    while (nodeIndex){
        table->m_indexs.push_back(EIndex::Create(*nodeIndex));
        nodeIndex = nodeIndex->NextSiblingElement();
    }

    table->m_classDataName = util::StringUtil::GetClassName(table->m_name + table->m_suffix);
    table->m_classDaoName = util::StringUtil::GetClassName(table->m_name + table->m_suffix + "_dao");

    for(auto& idx : table->m_indexs){
        if(idx->getIndexType() == EIndex::IndexType::Primary){
            table->m_pkName = idx->getColumns().front().first;
            for(auto& col : table->m_cols){
                //列名与主键所属列名相同
                if(col->getName() == idx->getName())
                    table->m_pkType = col->getCppTypeStr();
            }
        }
        if(idx->getIndexType() == EIndex::IndexType::Unique){
            std::vector<std::string> uniqueName;
            std::vector<std::string> uniqueType;
            for(auto& idxOnCol : idx->getColumns()){
                uniqueName.push_back(idxOnCol.first);
                for(auto& col : table->m_cols){
                    //列名与索引所属列名相同
                    if(col->getName() == idxOnCol.first)
                        uniqueType.push_back(col->getCppTypeStr());
                }
            }
            table->m_uniqueName.push_back(uniqueName);
            table->m_uniqueType.push_back(uniqueType);
        }
    }
}

void ETable::gen(const std::string& prePath){
    //根据namespace生成对应目录
    std::string path = prePath;
    for(auto& i : m_namespace){
        path += i;
        path += '/';
    }
    util::FileUtil::MKDir(path);
    genHeaderFile(path);
    genSourceFile(path);
}

void ETable::genHeaderFile(const std::string& prePath){
    //在对应目录下创建对应的文件
    std::string filePath = prePath + m_name + m_suffix + ".h";
    std::ofstream of(filePath);

    //预定义 _BLOG_DATA_USER_INFO_H_
    std::string preDefine = "_";
    for(auto&i : m_namespace){
        preDefine += i;
        preDefine += "_";
    }
    preDefine += m_name;
    preDefine += m_suffix;
    preDefine += "_H_";

    preDefine = util::StringUtil::UpperAll(preDefine);
    of << "#ifndef " << preDefine << std::endl;
    of << "#define " << preDefine << std::endl;

    //include

    //namespace
    for(auto&i : m_namespace){
        of << "namespace " << i << "{" << std::endl;
    }
    of << std::endl;

    //class XxxCcc{
    of << "class " << m_classDataName << "{" << std::endl;
    of << std::endl;

    of << "public: " << std::endl;
    of << "\t" << "using ptr = std::shared_ptr<" << m_classDataName << ">;" << std::endl;
    of << std::endl;

    //get set方法
    for(auto& i : m_cols){
        of << "\t" << i->funcGetDefine();
        of << "\t" << i->funcSetDefine();
        of << std::endl;
    }

    of << "private: " << std::endl;
    //成员变量
    for(auto&i : m_cols){
        of << "\t" << i->memberDefine();
    }
    //class 结束
    of << "};" << std::endl;

    //Dao类
    genHeaderDao(of);

    of << std::endl;

    //文件尾
    for(auto&i : m_namespace)
        of << "}" << std::endl;
    of << "#endif";
}

void ETable::genHeaderDao(std::ofstream& of){
    of << "class " << m_classDaoName << "{" << std::endl;
    of << "public: " << std::endl;
    of << "\t" << "using ptr = std::shared_ptr<" << m_classDaoName << ">;" << std::endl;

    //增删改查

    //增(插入)
    //单个插入(XXXX::ptr data, emilia::db::EDB::ptr conn);
    of << "\t" << "static int Insert(" << m_classDataName << "::ptr data, ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;
    //批量插入(emilia::db::EDB::ptr conn, const std::vector<XXXX::ptr>& vec);
    of << "\t" << "static int BatchInsert(const std::vector<" << m_classDataName << "::ptr>& vec, ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;

    //删
    //只构造通用的删除方法
    //1.根据主键删除(默认主键不是多个字段组合)(emilia::db::EDB::ptr conn, const XXX& pkName)
    of << "\t" << "static int DeleteBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_pkType << "& " << m_pkName << ", ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;
    //2.根据唯一索引删除
    for(int i = 0; i < m_uniqueName.size(); i++){
        std::string suffixName;
        for(auto& uStr : m_uniqueName[i]){
            suffixName += util::StringUtil::UpperLetter(uStr, 0);
        }
        of << "\t" << "static int DeleteBy" << suffixName;
        of << "(";
        for(int k = 0; k < m_uniqueType[i].size(); k++){
            of << "const " << m_uniqueType[i][k] << "& " << m_uniqueName[i][k] << ", ";
        }
        of << m_classDBName << "::ptr conn);";
        of << std::endl;
    }

    //改
    //只构造通用的更新方法
    //根据主键更新
    of << "\t" << "static int UpdateBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_classDataName << "::ptr data, ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;

    //查
    //index(a, b, c)
    //查询全部static int QueryAll(const std::vector<XXX::ptr>& rts, emilia::db::EDB::ptr conn);
    of << "\t" << "static int QueryAll(std::vector<" << m_classDataName << "::ptr>& rts, ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;

    //根据主键查询static XXX::ptr QueryByPkName(const pkType& pkName, emilia::db::EDB::ptr conn)
    of << "\t" << "static " << m_classDataName << "::ptr QueryBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_pkType << "& " << m_pkName << ", ";
    of << m_classDBName << "::ptr conn);";
    of << std::endl;

    //根据唯一键查询
    for(int i = 0; i < m_uniqueName.size(); i++){
        std::string suffixName;
        for(auto& uStr : m_uniqueName[i]){
            suffixName += util::StringUtil::UpperLetter(uStr, 0);
        }
        of << "\t" << "static " << m_classDataName << "::ptr QueryBy" << suffixName;
        of << "(";
        for(int k = 0; k < m_uniqueType[i].size(); k++){
            of << "const " << m_uniqueType[i][k] << "& " << m_uniqueName[i][k] << ", ";
        }
        of << m_classDBName << "::ptr conn);";
        of << std::endl;
    }

    //创建SQLite3表格
    of << "\t" << "static int CreateSQLite3Table();" << std::endl;
    //创建MYSQL表格
    of << "\t" << "static int CreateMySQLTable();" << std::endl;

    of << "};" << std::endl;

    for(auto&i : m_namespace){
        of << "}" << std::endl;
    }

    of << "#endif" << std::endl;

}

void ETable::genSourceFile(const std::string& prePath){
    //在对应目录生成.cc文件
    std::string filePath = prePath + m_name + m_suffix + ".cc";
    std::ofstream of(filePath);

    of << "#include\"" << m_name << m_suffix << ".h\"" << std::endl;
    for(auto&i : m_namespace){
        of << "namespace " << i << "{" << std::endl;
    }

    genSingleInsert(of);
    genBatchInsert(of);

    genDeleteByPK(of);
    genDeleteByUK(of);

    genUpdate(of);
    
    genQueryAll(of);
    genQueryByPK(of);
    genQueryByUK(of);

    genCreateMySQL(of);
    genCreateSQLite3(of);
}

//auto eStmt = conn->prepare(sql);
//if(eStmt == nullptr){
//    EMILIA_LOG_ERROR("orm") << stmt->getErrorStr();
//    return #rtValue;    
//}
#define PrepareStmt(of, rtValue) \
    of << "\t" << "auto eStmt = conn->prepare(sql);" << std::endl; \
    of << "\t" << "if(eStmt == nullptr){" << std::endl; \
    of << "\t\t" << "EMILIA_LOG_ERROR(\"orm\") << conn->getErrorStr();" << std::endl; \
    of << "\t\t" << "return " << #rtValue << ";" << std::endl; \
    of << "\t" << "}" << std::endl; \

#define BindByPK(of) \
    of << "\t" << "eStmt->bind"; \
    for(auto& i : m_cols){ \
        if(i->getName() == m_pkName){ \
            of << i->getEDBType(); \
        } \
    } \
    of << "(1, " << m_pkName << ");" << std::endl; \

#define BindByUk(of) \
    for(int o = 0; o < m_uniqueName[i].size(); o++){ \
        for(auto& col : m_cols){ \
            if(col->getName() == m_uniqueName[i][o]){ \
                of << "\t" << "eStmt->bind" << col->getEDBType(); \
                of << "(" << o << ", " << m_uniqueName[i][o] << ");" << std::endl; \
            } \
        } \
    } \

#define RtStmtExecute(of) \
    of << "\t" << "return eStmt->execute();" << std::endl; \

#define GenEData(of, rtValue) \
    of << "\t" << "auto eData = eStmt->query(sql);" << std::endl; \
    of << "\t" << "if(eData == nullptr){" << std::endl; \
    of << "\t\t" << "EMILIA_LOG_ERROR(\"orm\") << conn->getErrorStr();" << std::endl; \
    of << "\t\t" << "return " << #rtValue << ";" << std::endl; \
    of << "\t" << "}" << std::endl; \

void ETable::genSingleInsert(std::ofstream& of){

    //增(插入)
    //单个插入int xXDao::Insert(XXXX::ptr data, emilia::db::EDB::ptr conn){}
    of << "int " <<  m_classDaoName <<"::Insert(" << m_classDataName << "::ptr data, ";
    of << m_classDBName << "::ptr conn)";
    of << std::endl;
    of <<"{" << std::endl;

    //"insert into m_name values(?,?,?,?,?)"(除去自增的主键)
    of << "\t" <<"std::string sql = \"insert into" << m_name << "values(";
    size_t colSize = m_cols.size();
    for(size_t i = 1; i < colSize; i++){
        //不是最后一个 ?,
        if(i != colSize -1)
            of << "?,";
        else
            of << "?)\";";
    }
    of << std::endl;

    PrepareStmt(of, -1)
    //根据建表类型进行预处理的绑定(除去主键)
    size_t colSize = m_cols.size();
    // id(pk) name
    // xxxx = ?
    //name的idx为1
    for(size_t i = 0; i < colSize; i++){
        //如果不是主键
        if(m_cols[i]->getName() != m_pkName){
            of << "\t" << "eStmt->bind" << m_cols[i]->getEDBType();
            of << "(" << i << ", data->m_" << m_cols[i]->getName() << ");" << std::endl;
        }
    }

    //绑定完后执行
    //return stmt->execute();
    RtStmtExecute(of)
    of << "}" << std::endl;
}

void ETable::genBatchInsert(std::ofstream& of){

    //增(批量插入)int XXXDao::BatchInsert(const std::vector<XXX::ptr>& vec, emilia::db::EDB::ptr conn){}
    of << "int " << m_classDaoName << "::BatchInsert(const std::vector<" << m_classDataName << "::ptr>& vec, ";
    of << m_classDBName << "::ptr conn)" << std::endl;
    of << "{" << std::endl;

    /*
    std::string sql = "insert into m_name values";
    std::string pre = "(?,?,?,?,?)";

    size_t vecSize = vec.Size();
    for(int i = 1; i <= vecSize; i++){
        if(i != vecSize){
            sql += pre;
            sql += ",";
        }
        else{
            sql += pre;
            sql += ";";
        }
    }
    */

    of << "\t" << "std::string sql = \"insert into " << m_name << "values \"" << std::endl;
    of << "\t" << "std::string preChars = \"(";
    size_t colSize = m_cols.size();
    for(size_t i = 1; i < colSize; i++){
        //不是最后一个 ?,
        if(i != colSize -1)
            of << "?,";
        else
            of << "?)\";";
    }
    of << std::endl << std::endl;

    //根据vec大小填充预处理语句
    of << "\t" << "size_t vecSize = vec.Size();" << std::endl;
    of << "\t" << "for(size_t i = 1; i <= vecSize; i++){" << std::endl;
    of << "\t" << "\t" << "if(i != vecSize){" << std::endl;
    of << "\t" << "\t" << "\t" << "sql += preChars;" << std::endl;
    of << "\t" << "\t" << "\t" << "sql += \",\";" << std::endl;
    of << "\t" << "\t" << "}" << std::endl;
    of << "\t" << "\t" << "else{" << std::endl;
    of << "\t" << "\t" << "\t" << "sql += preChars;" << std::endl;
    of << "\t" << "\t" << "\t" << "sql += \";\";" << std::endl;
    of << "\t" << "\t" << "}" << std::endl;
    of << "\t" << "}" << std::endl;

    PrepareStmt(of, -1)
    //绑定
    /*
    for(size_t j = 0; j < vecSize; j++){
        eStmt->bindXXXX(j*(colSize-1)+1, vec[j]->m_XXX);
    }
    */
    of << "\t" << "for(size_t j = 0; j < vecSize; j++){" << std::endl;
    for(size_t idx = 1; idx < colSize; idx++){
        of << "\t\t" << "eStmt->bind" << m_cols[idx]->getEDBType();
        of << "(j * " << colSize -  1 << " + " << idx << ", vec[j]->m_" << m_cols[idx]->getName() << std::endl;
    }
    of << "\t" << "}" << std::endl;

    RtStmtExecute(of)
    of << "}" << std::endl;
}

void ETable::genDeleteByPK(std::ofstream& of){
    //根据主键进行删除int XXXDao::DeleteByPKName(const PKType& pKName, emilia::db::EDB::ptr conn){}
    of << "int " << m_classDaoName << "::DeleteBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_pkType << "& " << m_pkName << ", ";
    of << m_classDBName << "::ptr conn)" << std::endl;
    of << "{" << std::endl;

    //delete from m_name where pkName = ?
    of << "\t" << "std::string sql = \"delete from " << m_name << " where " << m_pkName << " = ?\";";
    
    PrepareStmt(of, -1)
    //根据主键类型进行绑定
    BindByPK(of)

    RtStmtExecute(of)
    of << "}" << std::endl;
}

void ETable::genDeleteByUK(std::ofstream& of){
    //根据唯一键进行删除int XXXDao::DeleteByUKName(const UKType& uKName, ....., emilia::db::EDB::ptr conn){}
    for(int i = 0; i < m_uniqueName.size(); i++){
        std::string suffixName;
        for(auto& uStr : m_uniqueName[i]){
            suffixName += util::StringUtil::UpperLetter(uStr, 0);
        }
        of << "int " << m_classDaoName << "::DeleteBy" << suffixName;
        of << "(";
        for(int k = 0; k < m_uniqueType[i].size(); k++){
            of << "const " << m_uniqueType[i][k] << "& " << m_uniqueName[i][k] << ", ";
        }
        of << m_classDBName << "::ptr conn)" << std::endl;
        of << "{" << std::endl;
        //delete from m_name where ukName1 = ? and ukName2 = ? 
        of << "\t" << "std::string sql = \"delete from " << m_name << " where ";
        for(int j = 0; j < m_uniqueName[i].size(); j++){
            of << m_uniqueName[i][j] << " = ? ";
            if(j != m_uniqueName[i].size()-1)
                of << "and ";
        }
        of << "\";" << std::endl;
        
        PrepareStmt(of, -1)
        //根据UKType进行绑定
        BindByUk(of)

        RtStmtExecute(of)
        of << "}" << std::endl;
    }
}

void ETable::genUpdate(std::ofstream& of){

    //根据主键进行数据更新
    of << "int " << m_classDaoName << "::UpdateBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_classDataName << "::ptr data, ";
    of << m_classDBName << "::ptr conn)" << std::endl;
    of << "{" << std::endl;

    //update m_name set xxx = ?,xxx = ?,xxx = ? where pkName = ?;
    of << "\t" << "std::string sql = \"update " << m_name <<  " set ";
    //除了主键
    size_t colSize = m_cols.size();
    for(size_t i = 0; i < colSize; i++){
        //不是主键
        if(m_cols[i]->getName() != m_pkName){
            //不是最后一个
            if(i != colSize -1)
                of << m_cols[i]->getName() << " = ?,";
            else
                of << m_cols[i]->getName() << " = ? ";
        }
    }
    of << "where " << m_pkName << " = ?\";" << std::endl;
    
    PrepareStmt(of, -1)
    //根据数据类型进行绑定
    int idxFlag = 1;
    for(auto& i : m_cols){
        if(i->getName() != m_pkName){
            of << "\t" << "eStmt->bind" << i->getEDBType();
            of << "(" << idxFlag << ", data->m_" << i->getName() << ");" << std::endl;
            idxFlag++;
        }
    }

    for(auto& i : m_cols){
        if(i->getName() == m_pkName){
            of << "\t" << "eStmt->bind" << i->getEDBType();
            of << "(" << idxFlag << ", data->m_" << i->getName() << ");" << std::endl;
            break;
        } 
    }

    RtStmtExecute(of)
    of << "}" << std::endl;
}

void ETable::genQueryAll(std::ofstream& of){

    of << "int " << m_classDaoName << "::QueryAll(std::vector<" << m_classDataName << "::ptr>& rts, ";
    of << m_classDBName << "::ptr conn)" << std::endl;
    of << "{" << std::endl;

    //select * from m_name;
    of << "\t" << "std::string = \"select * from " << m_name << "\";" << std::endl;

    of << "\t" << "auto eData = conn->query(sql);" << std::endl;

    //根据列的类型调用对应的get函数
    /*
    while(eData->nextData()){
        XXXX::ptr rt(new XXXX);
        rt->m_xx = datas->getZZZZ(1);

        rts.push_back(rt);
    }
    */

    of << "\t" <<"while(eData->nextData()){" << std::endl;
    of << "\t\t" << m_classDataName << "::ptr rt(new " << m_classDataName << ");" << std::endl;

    size_t colSize = m_cols.size();
    for(size_t i = 0; i < colSize; i++){
        of << "\t\t" << "rt->m_" << m_cols[i]->getName() << " = eData->get" << m_cols[i]->getEDBType();
        of << "(" << i+1 << ");" << std::endl;
    }

    of << "\t\t" << "rts.push_back(rt)" << std::endl;
    of << "\t" << "}" << std::endl;

    of << "\t" << "return 0;" << std::endl;
    of << "}" << std::endl;
}

void ETable::genQueryByPK(std::ofstream& of){

    of << m_classDataName << "::ptr " << m_classDaoName << "::QueryBy" << util::StringUtil::UpperLetter(m_pkName, 0);
    of << "(const " << m_pkType << "& " << m_pkName << ", ";
    of << m_classDBName << "::ptr conn)" << std::endl;
    of << "{" << std::endl;

    //select * from m_name where id = ?
    of << "\t" << "std::string sql = \"select * from " << m_name << " where " << m_pkName << " = ?\";";
    PrepareStmt(of, nullptr)
    
    //根据主键类型进行绑定
    BindByPK(of)

    GenEData(of, nullptr)

    of << "\t" <<"while(eData->nextData()){" << std::endl;
    of << "\t\t" << m_classDataName << "::ptr rt(new " << m_classDataName << ");" << std::endl;

    size_t colSize = m_cols.size();
    for(size_t i = 0; i < colSize; i++){
        of << "\t\t" << "rt->m_" << m_cols[i]->getName() << " = eData->get" << m_cols[i]->getEDBType();
        of << "(" << i+1 << ");" << std::endl;
    }

    of << "\t" << "return rt;" << std::endl;
    of << "}" << std::endl;
}

void ETable::genQueryByUK(std::ofstream& of){

    //根据唯一键查询
    for(int i = 0; i < m_uniqueName.size(); i++){
        std::string suffixName;
        for(auto& uStr : m_uniqueName[i]){
            suffixName += util::StringUtil::UpperLetter(uStr, 0);
        }
        of << m_classDataName << "::ptr " << m_classDaoName << "::QueryBy" << suffixName;
        of << "(";
        for(int k = 0; k < m_uniqueType[i].size(); k++){
            of << "const " << m_uniqueType[i][k] << "& " << m_uniqueName[i][k] << ", ";
        }
        of << m_classDBName << "::ptr conn)" << std::endl;
        of << "{" << std::endl;

        //select * from m_name where ukName1 = ? and ukName2 = ? 
        of << "\t" << "std::string sql = \"select * from " << m_name << " where ";
        for(int j = 0; j < m_uniqueName[i].size(); j++){
            of << m_uniqueName[i][j] << " = ? ";
            if(j != m_uniqueName[i].size()-1)
                of << "and ";
        }
        of << "\";" << std::endl;

        PrepareStmt(of, nullptr)

        //根据UKType进行绑定
        BindByUk(of)

        GenEData(of, nullptr)

        //switch同上
        of << "\t" <<"while(eData->nextData()){" << std::endl;
        of << "\t\t" << m_classDataName << "::ptr rt(new " << m_classDataName << ");" << std::endl;

        size_t colSize = m_cols.size();
        for(size_t i = 0; i < colSize; i++){
            of << "\t\t" << "rt->m_" << m_cols[i]->getName() << " = eData->get" << m_cols[i]->getEDBType();
            of << "(" << i+1 << ");" << std::endl;
        }
        of << "\t" << "return rt;" << std::endl;
        of << "}" << std::endl;
    }
}

void ETable::genCreateMySQL(std::ofstream& of){
    of << "\t" << "std::string sql = ";
    //"CREATE TABLE m_name("
    //"XXXXXXXXX,"
    //"XXXXXXXX,"
    //"XXXXXXXX,"
    of << "\"CREATE TABLE " << m_name << "(\"" << std::endl;

    for(auto& i : m_cols){
        of << "\"";
        of << i->getMySQLCreateStr();
        of << ",\"" << std::endl;
    }

    size_t indexSize = m_indexs.size();
    for(size_t z = 0; z < indexSize; z++){
        if(m_indexs[z]->getIndexType() == EIndex::IndexType::Primary){
            of << "\"PRIMARY KEY(`" << m_pkName << "`)";  
        }
        else if(m_indexs[z]->getIndexType() == EIndex::IndexType::Unique){
            of << "\"UNIQUE KEY `" << m_indexs[z]->getName() << "` (";
            size_t indexColSize = m_indexs[z]->getColumns().size();
            for(size_t k = 0; k < indexColSize; k++){
                of << "`" << m_indexs[z]->getColumns()[k].first << "`";
                if(k == indexColSize-1)
                    of << ")";
                else
                    of << ",";
            }
        }
        else if(m_indexs[z]->getIndexType() == EIndex::IndexType::Index){
            of << "\"INDEX `" << m_indexs[z]->getName() << "` (";
            size_t indexColSize = m_indexs[z]->getColumns().size();
            for(size_t k = 0; k < indexColSize; k++){
                of << "`" << m_indexs[z]->getColumns()[k].first << "`";
                if(k == indexColSize-1)
                    of << ")";
                else
                    of << ",";
            }
        }
        //全文索引
        else{

        }

        if(z != indexSize-1)
            of << ",\"";
        else
            of << ")";
    }

    if(!m_comment.empty())
        of << " COMMENT='" << m_comment << "'";
    of << "\";";

    of << "\t" << "return conn->prepare(sql);" << std::endl;
    of << "}" << std::endl;

}

void ETable::genCreateSQLite3(std::ofstream& of){

    of << "\t" << "std::string sql = ";
    //"CREATE TABLE m_name("
    //"XXXXXXXXX,"
    //"XXXXXXXX,"
    //"XXXXXXXX,"
    of << "\"CREATE TABLE " << m_name;

    if(!m_comment.empty())
        of << " --" << m_comment;

    of << "(\"" << std::endl;

    for(auto& i : m_cols){
        of << "\"";
        of << i->getSQLite3CreateStr();
        of << ",\"" << std::endl;
    }

    size_t indexSize = m_indexs.size();
    for(size_t z = 0; z < indexSize; z++){
        //已经在前面写过
        if(m_indexs[z]->getIndexType() == EIndex::IndexType::Primary){ 
        }
        else if(m_indexs[z]->getIndexType() == EIndex::IndexType::Unique){
            of << "\"CREATE UNIQUE INDEX " << m_indexs[z]->getName() << " ON " << m_name <<  "(";
            size_t indexColSize = m_indexs[z]->getColumns().size();
            for(size_t k = 0; k < indexColSize; k++){
                of << m_indexs[z]->getColumns()[k].first;
                if(k == indexColSize-1)
                    of << ")";
                else
                    of << ",";
            }
        }
        else if(m_indexs[z]->getIndexType() == EIndex::IndexType::Index){
            of << "\"CREATE INDEX " << m_indexs[z]->getName() << " ON " << m_name << "(";
            size_t indexColSize = m_indexs[z]->getColumns().size();
            for(size_t k = 0; k < indexColSize; k++){
                of << m_indexs[z]->getColumns()[k].first;
                if(k == indexColSize-1)
                    of << ")";
                else
                    of << ",";
            }
        }
        //全文索引
        else{
        }

        if(z != indexSize-1)
            of << ",\"";
        else
            of << ")";
    }
    of << "\";";

    of << "\t" << "return conn->prepare(sql);" << std::endl;
    of << "}" << std::endl;

}









}
}