#include "emilia/config/config.h"
#include <iostream>

using namespace emilia::config;

class Person{
public:
    Person(){}
    Person(const std::string& name, int age, bool sex)
    :m_name(name)
    ,m_age(age)
    ,m_sex(sex){}

    bool operator == (const Person& oth) const {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
    }

public:
    std::string m_name;
    int m_age;
    bool m_sex;
};

class PerSonConfig : public emilia::config::CustomConfig{
public:
    using ptr = std::shared_ptr<PerSonConfig>;
    PerSonConfig(){}

    PerSonConfig(const std::vector<Person>& pers)
    :m_pers(pers){}

    CustomConfig::ptr loadYaml(const YAML::Node& node) override {
        std::vector<Person> pers;
        for(size_t i = 0; i < node.size(); i++){
            pers.push_back(Person(node[i]["name"].Scalar(), node[i]["age"].as<int>(), node[i]["sex"].as<bool>()));
        }
        return CustomConfig::ptr(new PerSonConfig(pers));
    }

    bool operator == (const PerSonConfig& oth) const{
        return m_pers == oth.m_pers;
    }

    void toString() override{
        for(size_t i = 0; i < m_pers.size(); i++){
            std::cout << i << "==============" << std::endl;
            std::cout << "name: " << m_pers[i].m_name << std::endl;
            std::cout << "age: " << m_pers[i].m_age << std::endl;
            std::cout << "sex: " << m_pers[i].m_sex << std::endl;
        }
    }
public:
    std::vector<Person> m_pers;
};

ConfigVar::ptr t = ConfigMgr::GetInstance()->addConfig("persons", "per_desc", CustomConfig::ptr(new PerSonConfig()));


int main(){
    
    YAML::Node root = YAML::LoadFile("/home/emilia/workspace/emilia/tests/config/testyaml.yaml");

    t->addChangeCb([](const CustomConfig::ptr oldConfig, const CustomConfig::ptr newConfig){
        PerSonConfig::ptr oldVal = std::dynamic_pointer_cast<PerSonConfig>(oldConfig);
        PerSonConfig::ptr newVal = std::dynamic_pointer_cast<PerSonConfig>(newConfig);
        //配置相同返回
        if(*oldVal == *newVal)
            return false;
        
        std::cout << "配置改变了！" << std::endl;
        return true;
    }
    );

    ConfigMgr::GetInstance()->LoadFromYaml(root);

    t->getConfig()->toString();
}
