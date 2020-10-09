#include "Material.h"
#include "Yaml.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace Sprocket {

std::shared_ptr<Material> Material::FromFile(const std::string& file)
{
    std::string filepath = std::filesystem::absolute(file).string();
    auto material = std::make_shared<Material>();
    material->file = filepath;

    std::ifstream stream(filepath);
    std::stringstream sstream;
    sstream << stream.rdbuf();

    YAML::Node data = YAML::Load(sstream.str());

    if (auto name = data["Name"]) {
        material->name = name.as<std::string>();
    }
    else {
        material->name = "Bad material";
    }

    if (auto albedoMap = data["AlbedoMap"]) {
        material->albedoMap = albedoMap.as<std::string>();
    }
    if (auto normalMap = data["NormalMap"]) {
        material->normalMap = normalMap.as<std::string>();
    }
    if (auto metallicMap = data["MetallicMap"]) {
        material->metallicMap = metallicMap.as<std::string>();
    }
    if (auto roughnessMap = data["RoughnessMap"]) {
        material->roughnessMap = roughnessMap.as<std::string>();
    }

    if (auto useAlbedoMap = data["UseAlbedoMap"]) {
        material->useAlbedoMap = useAlbedoMap.as<bool>();
    }
    if (auto useNormalMap = data["UseNormalMap"]) {
        material->useNormalMap = useNormalMap.as<bool>();
    }
    if (auto useMetallicMap = data["UseMetallicMap"]) {
        material->useMetallicMap = useMetallicMap.as<bool>();
    }
    if (auto useRoughnessMap = data["UseRoughnessMap"]) {
        material->useRoughnessMap = useRoughnessMap.as<bool>();
    }

    if (auto albedo = data["Albedo"]) {
        material->albedo = albedo.as<Maths::vec3>();
    }
    if (auto metallic = data["Metallic"]) {
        material->metallic = metallic.as<float>();
    }
    if (auto roughness = data["Roughness"]) {
        material->roughness = roughness.as<float>();
    }

    return material;
}

}