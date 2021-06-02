#include "emilia/config/config.h"
#include <iostream>

namespace emilia{
namespace config{

//返回与配置名称对应的配置类或者nullptr
ConfigVar::ptr ConfigManger::lookUp(const std::string& name){
    auto it = m_configVars.find(name);
    return (it == m_configVars.end()) ? nullptr : it->second;
}

//构造并存储ConfigVar对象类，返回刚刚构造的ConfigVar的智能指针
ConfigVar::ptr ConfigManger::addConfig(const std::string& name, 
                                       const std::string& desc, 
                                       CustomConfig::ptr config, 
                                       ConfigVar::onChangeCb cb){

    auto it = m_configVars.find(name);
    //已经存在此配置名称
    if(it != m_configVars.end()){
        std::cout << "Config: " << name << " Has Exist" << std::endl;
        return it->second;
    }
    //没有此配置名称，构造ConfigVar并存储在内存中
    ConfigVar::ptr temp(new ConfigVar(name, desc, config, cb));
    m_configVars.insert(std::make_pair(name, temp));
    return temp;
}

//加载yaml文件
void ConfigManger::LoadFromYaml(const YAML::Node& root){
    if(!root.IsMap())
        return ;

    //必须是map类型的
    for(auto it = root.begin(); it != root.end(); it++){
        //寻找配置名称对应的ConfigVar
        ConfigVar::ptr temp = lookUp(it->first.Scalar());
        if(temp == nullptr){
            std::cout << "Config Name: " << it->first.Scalar() <<" Not Exist" << std::endl;
            return ;
        }
        //将YAML节点转换为对应的配置类，并将其设置为新的配置
        CustomConfig::ptr tempRet = temp->loadYaml(it->second);
        if(tempRet == nullptr)
            return ;
        temp->setConfig(tempRet);
    }
}

}
}
