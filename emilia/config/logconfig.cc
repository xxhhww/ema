#include "emilia/config/logconfig.h"

namespace emilia{
namespace config{

bool LogAppenderConfig::operator == (const LogAppenderConfig& oth) const{
    return m_type == oth.m_type
        && m_formatter == oth.m_formatter
        && m_file == oth.m_file;
}

bool LoggerConfig::operator == (const LoggerConfig& oth) const{
    return m_name == oth.m_name
        && m_level == oth.m_level
        && m_formatter == oth.m_formatter
        && m_appenders == oth.m_appenders;
}

bool LogConfig::operator == (const LogConfig& oth) const{
    return (m_loggers == oth.m_loggers);
}

bool LogAppenderConfig::loadYaml(const YAML::Node& node){
    if(node["type"].as<std::string>() == "file")
        m_type = 0;
    else if(node["type"].as<std::string>() == "stdout")
        m_type = 1;
    else{
        std::cout << "Appender Type Is UnDefined" << std::endl;
        return false;
    }

    if(node["formatter"].IsDefined())
        m_formatter = node["formatter"].as<std::string>();
    else
        m_formatter = "";

    //是文件输出地
    if(m_type == 0){
        if(node["file"].IsDefined())
            m_file = node["file"].as<std::string>();
        else{
            std::cout << "Appender Type Is File But File Is UnDefined" << std::endl;
            return false;    
        }
    }
    return true;
}

bool LoggerConfig::loadYaml(const YAML::Node& node){
    if(node["name"].IsDefined())
        m_name = node["name"].as<std::string>();
    else{
        std::cout << "Logger Name Is UnDefined" << std::endl;
        return false;
    }

    if(node["level"].IsDefined())
        m_level = StringToLevel(node["level"].as<std::string>());
    else{
        std::cout << "Logger: " << m_name << " Level Is UnDefined" << std::endl;
        return false;
    }

    if(node["formatter"].IsDefined())
        m_formatter = node["formatter"].as<std::string>();
    else{
        std::cout << "Logger: " << m_name << " Formatter Is UnDefined" << std::endl;
        return false;
    }

    std::cout << m_formatter << std::endl;

    if(!node["appenders"].IsDefined()){
        std::cout << "Logger: " << m_name << " Appenders Is UnDefined" << std::endl;
        return false;
    }
    for(size_t i = 0; i < node["appenders"].size(); i++){
        LogAppenderConfig temp;
        if(temp.loadYaml(node["appenders"][i]))
            m_appenders.push_back(temp);
        else
            return false;
    }
    return true;
}

//加载yaml文件
CustomConfig::ptr LogConfig::loadYaml(const YAML::Node& node){
    std::vector<LoggerConfig> loggers;
    for(size_t i = 0; i < node.size(); i++){
        LoggerConfig temp;
        if(temp.loadYaml(node[i]))
            loggers.push_back(temp);
        else
            return nullptr;
    }
    return CustomConfig::ptr(new LogConfig(loggers));
}

void LogConfig::toString(){
}

}
}
