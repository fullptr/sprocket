#pragma once
#include "Maths.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

using namespace Sprocket;

template<>
struct convert<glm::vec2>
{
    static Node encode(const glm::vec2& rhs);
    static bool decode(const Node& node, glm::vec2& rhs);
};

template<>
struct convert<glm::vec3>
{
    static Node encode(const glm::vec3& rhs);
    static bool decode(const Node& node, glm::vec3& rhs);
};

template<>
struct convert<Sprocket::Maths::quat>
{
    static Node encode(const Sprocket::Maths::quat& rhs);
    static bool decode(const Node& node, Sprocket::Maths::quat& rhs);
};

template<>
struct convert<glm::mat4>
{
    static Node encode(const glm::mat4& rhs);
    static bool decode(const Node& node, glm::mat4& rhs);
};

}

namespace Sprocket {
 
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v);
YAML::Emitter& operator<<(YAML::Emitter& out, const Maths::quat& q);
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& m);

}