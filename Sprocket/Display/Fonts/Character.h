#pragma once
#include "Maths.h"
#include "Quad.h"
#include "Model2D.h"

namespace Sprocket {

class Character
{
    int d_id;
        // ASCII ID

    Quad d_atlasQuad;
        // Describes the area in the texture map that shows this character.

    float d_xOffset;
    float d_yOffset;
        // Position on the line of the top left of this character

    float d_advance;
        // How far the cursor should advance after printing this character

    Model2D d_model;
        // Model of the character to load.

    Texture d_atlas;
        // Texture atlas that holds this character

public:
    Character(const Texture& atlas,
              int id,
              const Maths::vec2& texTopLeft,
              float width,
              float height,
              float xOffset,
              float yOffset,
              float advance);

    void Bind() const;
    void Unbind() const;

    int Id() const { return d_id; }

    Quad GetAtlasQuad() const { return d_atlasQuad; }

    float XOffset() const { return d_xOffset; }
    float YOffset() const { return d_yOffset; }
    Maths::vec3 Offset() const { return {d_xOffset, -d_yOffset, 0.0}; }

    float Advance() const { return d_advance; }

    float Width() const { return d_atlasQuad.width; }
    float Height() const { return d_atlasQuad.height; }

    Texture Atlas() const { return d_atlas; }
};

}