#pragma once
#include <glm/glm.hpp>

#include <memory>
#include <string_view>
#include <string>
#include <span>
#include <vector>

namespace spkt {

class shader
{
    std::string d_vert_source;
    std::string d_frag_source;

    std::uint32_t d_program_id;
    std::uint32_t d_vert_shader_id;
    std::uint32_t d_frag_shader_id;

    std::uint32_t uniform_location(const std::string& name) const;

    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

public:
    shader(std::string_view vert_shader_file, std::string_view frag_shader_file);
    ~shader();

    bool reload();
    
    std::string& vertex_source() { return d_vert_source; }
    std::string& fragment_source() { return d_frag_source; }

    void bind() const;
    void unbind() const;

    // Shader Uniform Setters
    void load(const std::string& name, int value) const;
    void load(const std::string& name, float value) const;
    void load(const std::string& name, const glm::vec2& vector) const;
    void load(const std::string& name, const glm::vec3& vector) const;
    void load(const std::string& name, const glm::vec4& vector) const;
    void load(const std::string& name, const glm::quat& quat) const;
    void load(const std::string& name, const glm::mat4& matrix) const;

    void load(const std::string& name, std::span<const float> values) const;
    void load(const std::string& name, std::span<const glm::vec3> values) const;
    void load(const std::string& name, std::span<const glm::mat4> values) const;
};

using shader_ptr = std::unique_ptr<spkt::shader>;

// Formats an OpenGL array indexed name 
std::string array_name(std::string_view uniform_name, std::size_t index);

}