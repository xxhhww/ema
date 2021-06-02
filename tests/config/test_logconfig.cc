#include "emilia/config/config.h"
#include "emilia/log/logmarco.h"
#include <yaml-cpp/yaml.h>

using namespace emilia::log;

int main(){
    YAML::Node root = YAML::LoadFile("/home/emilia/workspace/emilia/tests/config/logs.yaml");

    emilia::config::ConfigMgr::GetInstance()->LoadFromYaml(root);

    EMILIA_LOG_DEBUG("system") << "test_system";
    EMILIA_LOG_INFO("root") << "test_root";

    return 0;
}