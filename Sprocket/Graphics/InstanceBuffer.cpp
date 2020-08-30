#include "InstanceBuffer.h"

#include <glad/glad.h>

namespace Sprocket {

InstanceBuffer::InstanceBuffer()
    : d_buffer(std::make_shared<VBO>())
{
    d_data.reserve(1000);
    glBindBuffer(GL_ARRAY_BUFFER, d_buffer->Value());
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * 1000, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, d_buffer->Value());
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * d_data.size(), d_data.data(), GL_DYNAMIC_DRAW);
}

void InstanceBuffer::Clear()
{
    d_data.clear();
}

void InstanceBuffer::Add(const InstanceData& data)
{
    d_data.push_back(data);
}

}