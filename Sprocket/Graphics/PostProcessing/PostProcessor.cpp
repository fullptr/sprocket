#include "PostProcessor.h"

namespace Sprocket {
namespace {

VertexBuffer GetVertices()
{
    Vertex bottomLeft;
    bottomLeft.position = {-1.0f, -1.0f, 0.0f};
    bottomLeft.textureCoords = {0.0f, 0.0f};

    Vertex bottomRight;
    bottomRight.position = {1.0f, -1.0f, 0.0f};
    bottomRight.textureCoords = {1.0f, 0.0f};

    Vertex topRight;
    topRight.position = {1.0f, 1.0f, 0.0f};
    topRight.textureCoords = {1.0f, 1.0f};

    Vertex topLeft;
    topLeft.position = {-1.0f, 1.0f, 0.0f};
    topLeft.textureCoords = {0.0f, 1.0f};

    return VertexBuffer{
        bottomLeft,
        bottomRight,
        topRight,
        topLeft
    };
}

}

PostProcessor::PostProcessor(int width, int height)
    : d_width(width)
    , d_height(height)
    , d_quad(GetVertices(), {0, 1, 2, 0, 2, 3})
{}

void PostProcessor::AddEffect(std::shared_ptr<Effect> effect)
{
    d_effects.push_back(effect);
}

void PostProcessor::Bind() const
{
    d_effects[0]->BindForWrite();
}

void PostProcessor::Unbind() const
{
    d_effects[0]->UnbindForWrite();
}

void PostProcessor::Draw()
{
    d_quad.Bind();
    for (int i = 0; i != d_effects.size(); ++i) {
        
        // Final effect, so draw to screen.
        if (i + 1 == d_effects.size()) {
            d_effects[i]->Draw();
        } else {
            d_effects[i]->Draw(d_effects[i+1]);
        }
    }
}

void PostProcessor::SetScreenSize(int width, int height)
{
    d_width = width;
    d_height = height;
    for (auto& effect : d_effects) {
        effect->SetScreenSize(width, height);
    }
}
    
}