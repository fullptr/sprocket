#include "buffer_element_types.h"

#include <glad/glad.h>

#include <ranges>

namespace spkt {

void static_vertex::set_buffer_attributes(std::uint32_t vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (int index : std::views::iota(0, 5)) {
        glEnableVertexAttribArray(index);
        glVertexAttribDivisor(index, 0);
    } 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex), (void*)offsetof(static_vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(static_vertex), (void*)offsetof(static_vertex, textureCoords));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex), (void*)offsetof(static_vertex, normal));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex), (void*)offsetof(static_vertex, tangent));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex), (void*)offsetof(static_vertex, bitangent));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void animated_vertex::set_buffer_attributes(std::uint32_t vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (int index : std::views::iota(0, 7)) {
        glEnableVertexAttribArray(index);
        glVertexAttribDivisor(index, 0);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, textureCoords));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, normal));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, tangent));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, bitangent));
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(animated_vertex), (void*)offsetof(animated_vertex, boneIndices));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(animated_vertex), (void*)offsetof(animated_vertex, boneWeights));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ui_vertex::set_buffer_attributes(std::uint32_t vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (int index : std::views::iota(0, 3)) {
        glEnableVertexAttribArray(index);
        glVertexAttribDivisor(index, 0);
    } 

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ui_vertex), (void*)offsetof(ui_vertex, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ui_vertex), (void*)offsetof(ui_vertex, colour));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ui_vertex), (void*)offsetof(ui_vertex, textureCoords));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void model_instance::set_buffer_attributes(std::uint32_t vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (int index : std::views::iota(5, 8)) {
        glEnableVertexAttribArray(index);
        glVertexAttribDivisor(index, 1);
    } 

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(model_instance), (void*)offsetof(model_instance, position));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(model_instance), (void*)offsetof(model_instance, orientation));
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(model_instance), (void*)offsetof(model_instance, scale));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}