#pragma once
#include <glm/glm.hpp>

#include <string>

namespace spkt {

struct material
{
    std::string name;

    std::string albedoMap;
    std::string normalMap;
    std::string metallicMap;
    std::string roughnessMap;

    bool useAlbedoMap = false;
    bool useNormalMap = false;
    bool useMetallicMap = false;
    bool useRoughnessMap = false;

    glm::vec3 albedo = {1.0f, 1.0f, 1.0f};
    float     metallic = 0.0f;
    float     roughness = 1.0f;

    static material load(const std::string& file);
    static void     save(const std::string& file, const material& material);
};

}