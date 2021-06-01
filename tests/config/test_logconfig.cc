#include "emilia/config/config.h"
#include <yaml-cpp/yaml.h>

int main(){
    YAML::Node root = YAML::LoadFile("/home/emilia/workspace/emilia/tests/config/logs.yaml");

    emilia::config::ConfigMgr::GetInstance()->LoadFromYaml(root);
}