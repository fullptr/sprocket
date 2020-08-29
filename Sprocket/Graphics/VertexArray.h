#pragma once
#include "Resources.h"
#include "Model3D.h"

#include <memory>

namespace Sprocket {

class VertexArray
{
    std::shared_ptr<VAO> d_vao;

    std::shared_ptr<Model3D> d_model;

public:
    VertexArray();

    void SetModel(std::shared_ptr<Model3D> model);

    void Draw() const;
};

}