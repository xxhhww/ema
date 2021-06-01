#ifndef _EMILIA_CONFIG_H_
#define _EMILIA_CONFIG_H_

#include <memory>
#include <functional>
#include <yaml-cpp/yaml.h>
#include "emilia/singleton.h"
#include <iostream>


namespace emilia{
namespace config{
//!配置模块
//!配置模块启动时，从配置文件XXX.yaml中读取配置数据
//!有些配置只是基础类型
//!例如：Http报文最大值、协程栈的大小等等
//!有些配置却是容器类型
//!例如：日志的配置信息应该是一个set<Logger>

//!需要配置模块支持基础类型的解析和容器类型的解析

//!除此之外，配置信息还应该可以在系统运行时进行修改
//!这需要回调函数（当旧的配置信息与新的配置信息不相同时，调用回调函数）

//!配置模块的工作流程：
//!拿日志配置来说明
//!首先用户写一个继承自CustomConfig的子配置类LogConfig，重写loadYaml方法和genYaml方法和重载==符号
//!然后，用户调用ConfigMgr的lookUp(配置名称，配置描述，日志配置类，回调=nullptr)向系统注册日志配置信息(ConfigVar类)
//!系统启动时，从yaml文件中读取日志配置，根据配置名称，寻找到与之对应的ConfigVar类，调用ConfigVar的loadYaml方法
//!ConfigVar的loadYaml方法会调用CustomConfig的loadYaml方法(也就是用户重写的loadYaml方法)，生成对应的LogConfig
//!随后系统调用ConfigVar的setConfig方法，setConfig方法中，通过重载的==符号比较两个CustomConfig::ptr所指向的内容
//!如果内容不同就调用回调，如果相同则不做反应

//!用户配置类的父类，用户自定义的配置类继承此类
class CustomConfig{
public:
    using ptr = std::shared_ptr<CustomConfig>;
    virtual ~CustomConfig() {}

    //根据node节点生成对应的子配置类，返回指向此子配置类的父类指针
    virtual CustomConfig::ptr loadYaml(const YAML::Node& node) = 0;
    //将自己的数据结构生成为yaml的节点，并返回
    //virtual YAML::Node genYaml() = 0;
    virtual void toString() = 0;
};

//!每一个ConfigVar负责管理指向子配置类的父类指针，以及有关此子配置类的其他信息
//!例如：配置名称、配置描述、配置回调
class ConfigVar{
public:
    using ptr = std::shared_ptr<ConfigVar>;
    using onChangeCb = std::function<bool(const CustomConfig::ptr oldConfig, const CustomConfig::ptr newConfig)>;

    ConfigVar(const std::string& name, const std::string& desc, CustomConfig::ptr config, onChangeCb cb = nullptr)
    :m_name(name)
    ,m_desc(desc)
    ,m_config(config)
    ,m_cb(cb)
    {}

    const std::string& getName() const { return m_name; }
    const std::string& getDesc() const { return m_desc; }
    const CustomConfig::ptr getConfig() const { return m_config; }

    void setName(const std::string& name) { m_name = name; }
    void setDesc(const std::string& desc) { m_desc = desc; }
    //设置新的配置信息，调用回调函数
    void setConfig(CustomConfig::ptr config){
        //调用回调
        bool isEquel = m_cb(m_config, config);
        if(isEquel)
            m_config = config;
    }
    
    void addChangeCb(onChangeCb cb) { m_cb.swap(cb); }
    void delChangeCb() { m_cb = nullptr; }

    CustomConfig::ptr loadYaml(const YAML::Node& node) { return m_config->loadYaml(node); }
    //YAML::Node toYaml() { return m_config->genYaml(); }

private:
    //此配置名称，与yaml文件中对应
    std::string m_name;
    //此配置的描述
    std::string m_desc;
    //此配置的具体信息
    CustomConfig::ptr m_config;
    //此配置的回调
    onChangeCb m_cb;
};

//配置管理类
//注意此类是唯一的
class ConfigManger{
public:
    //返回与配置名称对应的配置类或者nullptr
    ConfigVar::ptr lookUp(const std::string& name);

    //构造并存储ConfigVar对象类，返回刚刚构造的ConfigVar的智能指针
    ConfigVar::ptr addConfig(const std::string& name, 
                             const std::string& desc, 
                             CustomConfig::ptr config, 
                             ConfigVar::onChangeCb cb = nullptr);
    
    //加载yaml文件
    void LoadFromYaml(const YAML::Node& root);
private:
    //记录配置名称和与之对应的配置信息
    std::map<std::string, ConfigVar::ptr> m_configVars;
};

//唯一类
using ConfigMgr = emilia::singleton<ConfigManger>;

}
}

#endif
