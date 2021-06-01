#include "emilia/config/config.h"
#include "emilia/log/logstream.h"
#include "emilia/log/logger.h"
#include <yaml-cpp/yaml.h>
#include <string>
#include <set>
#include <memory>
#include <vector>

using namespace emilia::log;

namespace emilia{
namespace config{

//日志的配置文件如下所示
//Ps：日志器需要标准输出时，不需要增加文件file和格式formatter
/*
logs:
  - name: root
    level: info
    formatter: "%d%T%m%n"
    appenders:
      - type: file
        file: log.txt
        formatter: "%d%T%m%n%T%T"
      - type: stdout
  - name: system
    level: debug
    formatter: "%d%T%m%n"
    appenders:
      - type: file
        file: system.txt
      - type: stdout
*/

//!以下三个类对应于日志配置文件中的数据结构

//日志输出地配置
class LogAppenderConfig{
public:
    LogAppenderConfig(){}
    bool loadYaml(const YAML::Node& node);
    bool operator == (const LogAppenderConfig& oth) const;
public:
    //type == 0 file type == 1 stdout
    int m_type;
    //日志输出地的日志输出格式，如不存在，则使用日志器的日志输出格式
    std::string m_formatter;
    //日志输出地的文件信息如果是标准输出则此成员变量为空
    std::string m_file;
};

//日志器配置类
class LoggerConfig{
public:
    LoggerConfig(){}
    bool loadYaml(const YAML::Node& node);
    bool operator == (const LoggerConfig& oth) const;
public:
    //日志器名称
    std::string m_name;
    //日志器等级
    LogLevel m_level;
    //日志器日志输出格式
    std::string m_formatter;
    //日志器的日志输出地
    std::vector<LogAppenderConfig> m_appenders;
};

//日志配置类
class LogConfig : public emilia::config::CustomConfig{
public:
    using ptr = std::shared_ptr<LogConfig>;
    LogConfig(){}
    LogConfig(const std::vector<LoggerConfig>& loggers):m_loggers(loggers){}

    //重写CustomBase中的loadYaml方法，加载yaml文件
    CustomConfig::ptr loadYaml(const YAML::Node& node) override;
    //重写CustomBase中的toString方法，将日志配置输出为yaml字符串
    void toString() override;
    //回调函数中会比较新旧配置类是否相等，因此必须重载==符号
    bool operator == (const LogConfig& oth) const;
public:
    std::vector<LoggerConfig> m_loggers;
};

ConfigVar::ptr logconfig = 
ConfigMgr::GetInstance()->addConfig("log", "logconfig"
                                    ,CustomConfig::ptr(new LogConfig())
,[](const CustomConfig::ptr oldConfig, const CustomConfig::ptr newConfig){
    LogConfig::ptr oldVal = std::dynamic_pointer_cast<LogConfig>(oldConfig);
    LogConfig::ptr newVal = std::dynamic_pointer_cast<LogConfig>(newConfig);
    if(*oldVal == *newVal)
        return false;

    std::cout << "Config Change!" << std::endl;
    
    //将新配置加载到系统中,也就是解析LoggerConfig
    for(size_t i = 0; i < newVal->m_loggers.size(); i++){
        LoggerConfig loggerConfig = newVal->m_loggers[i];
        Logger::ptr logger(new Logger(loggerConfig.m_name));

        if(loggerConfig.m_level != LogLevel::UNKNOWN)
            logger->setLevel(loggerConfig.m_level);
        if(!loggerConfig.m_formatter.empty())
            logger->setFormat(LogFormat::ptr(new LogFormat(loggerConfig.m_formatter)));

        std::vector<LogAppender::ptr> logAppenders;
        for(size_t j = 0; j < loggerConfig.m_appenders.size(); j++){
            LogAppenderConfig appConfig = loggerConfig.m_appenders[j];
            LogAppender::ptr app;
            //0为文件输出地
            if(appConfig.m_type == 0){
                FileAppender::ptr fileApp(new FileAppender(appConfig.m_file));
                if(!appConfig.m_formatter.empty())
                    fileApp->setFormat(LogFormat::ptr(new LogFormat(appConfig.m_formatter)));
                app = std::dynamic_pointer_cast<LogAppender>(fileApp);
            }
            //1为标准输出地(标准输出地是唯一的)
            else{
                app = std::dynamic_pointer_cast<LogAppender>(StdOutSingletonPtr);
            }
            logger->addAppender(app);
        }
        //如果Logger对应的名称相同则将老日志器删除，增加新日志器。否则直接增加新日志器
        LoggerMgr::GetInstance()->update(logger);
    }
    return true;
}
);

}
}
