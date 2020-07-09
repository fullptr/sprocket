#include "SimpleUI.h"
#include "KeyboardProxy.h"
#include "MouseProxy.h"
#include "MouseCodes.h"
#include "Log.h"
#include "Maths.h"
#include "RenderContext.h"

#include <glad/glad.h>

namespace Sprocket {
namespace {

float TextWidth(Font& font, const std::string& text)
{
    float width = 0.0f;
    for (char c : text) {
        auto glyph = font.GetGlyph(c);
        width += glyph.advance.x;
    }

    char first = text.front();
    width -= font.GetGlyph(first).offset.x;

    char last = text.back();
    width += font.GetGlyph(last).width;
    width += font.GetGlyph(last).offset.x;
    width -= font.GetGlyph(last).advance.x;

    return width;
}

}

SimpleUI::SimpleUI(Window* window)
    : d_window(window)
    , d_shader("Resources/Shaders/SimpleUI.vert",
               "Resources/Shaders/SimpleUI.frag")
    , d_bufferLayout(sizeof(BufferVertex))
    , d_font(1024, 1024)
{
    d_keyboard.ConsumeAll(false);

    d_bufferLayout.AddAttribute(DataType::FLOAT, 2);
    d_bufferLayout.AddAttribute(DataType::FLOAT, 4);
    d_bufferLayout.AddAttribute(DataType::FLOAT, 2);
    d_buffer.SetBufferLayout(d_bufferLayout);

    if (!d_font.Load("Resources/Fonts/Arial.ttf", 36.0f)) {
        SPKT_LOG_ERROR("Could not load font!");
    }
}

void SimpleUI::OnEvent(Event& event)
{
    d_keyboard.OnEvent(event);
    d_mouse.OnEvent(event);
}

void SimpleUI::OnUpdate(double dt)
{
    d_mouse.OnUpdate();

    if (d_mouse.IsButtonReleased(Mouse::LEFT)) {
        d_clicked = -1;
    }
}

void SimpleUI::StartFrame()
{
    d_quadVertices.clear();
    d_quadIndices.clear();

    d_textVertices.clear();
    d_textIndices.clear();
}

void SimpleUI::EndFrame()
{
    Sprocket::RenderContext rc;  
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    Maths::mat4 proj = Maths::Ortho(0, d_window->Width(), d_window->Height(), 0);
    d_shader.Bind();
    d_shader.LoadUniform("u_proj_matrix", proj);
    d_buffer.Bind();

    Texture::White().Bind();
    d_buffer.SetVertexData(
        sizeof(BufferVertex) * d_quadVertices.size(),
        d_quadVertices.data());
    d_buffer.SetIndexData(
        sizeof(unsigned int) * d_quadIndices.size(),
        d_quadIndices.data());
    glDrawElements(GL_TRIANGLES, (int)d_quadIndices.size(), GL_UNSIGNED_INT, nullptr);

    d_font.Bind();
    d_buffer.SetVertexData(
        sizeof(BufferVertex) * d_textVertices.size(),
        d_textVertices.data());
    d_buffer.SetIndexData(
        sizeof(unsigned int) * d_textIndices.size(),
        d_textIndices.data());
    glDrawElements(GL_TRIANGLES, (int)d_textIndices.size(), GL_UNSIGNED_INT, nullptr);
    
    d_buffer.Unbind();
    
}

void SimpleUI::Quad(float x, float y,
                    float width, float height,
                    const Maths::vec4& colour)
{
    unsigned int index = d_quadVertices.size();
    d_quadVertices.push_back({{x,         y},          colour});
    d_quadVertices.push_back({{x + width, y},          colour});
    d_quadVertices.push_back({{x,         y + height}, colour});
    d_quadVertices.push_back({{x + width, y + height}, colour});

    d_quadIndices.push_back(index + 0);
    d_quadIndices.push_back(index + 1);
    d_quadIndices.push_back(index + 2);
    d_quadIndices.push_back(index + 2);
    d_quadIndices.push_back(index + 1);
    d_quadIndices.push_back(index + 3);
}

bool SimpleUI::Button(
    int id, const std::string& name,
    float x, float y,
    float width, float height)
{
    auto mouse = d_mouse.GetMousePos();
    auto hovered = d_mouse.InRegion(x, y, width, height);
    auto clicked = hovered && d_mouse.IsButtonClicked(Mouse::LEFT);
    if (clicked) { d_clicked = id; }

    Maths::vec4 colour = d_theme.baseColour;
    if (d_clicked == id) {
        colour = d_theme.clickedColour;
    }
    else if (hovered) {
        colour = d_theme.hoveredColour;
    }

    Quad(x, y, width, height, colour);
    AddText(x, y, name, 0.6f * height, width, height);
    return clicked;
}

void SimpleUI::Slider(int id, const std::string& name,
                      float x, float y, float width, float height,
                      float* value, float min, float max)
{
    auto mouse = d_mouse.GetMousePos();
    auto hovered = d_mouse.InRegion(x, y, width, height);
    auto clicked = hovered && d_mouse.IsButtonClicked(Mouse::LEFT);
    if (clicked) { d_clicked = id; }

    float ratio = (*value - min) / (max - min);
    Quad(x, y, ratio * width, height, d_theme.hoveredColour);
    Quad(x + ratio * width, y, (1 - ratio) * width, height, d_theme.baseColour);
    
    std::stringstream text;
    text << name << ": " << Maths::ToString(*value, 0);
    
    AddText(x, y, text.str(), 0.6f * height, width, height);

    if (d_clicked == id) {
        Maths::Clamp(mouse.x, x, x + width);
        float r = (mouse.x - x) / width;
        *value = (1 - r) * min + r * max;
    }    
}

void SimpleUI::AddText(float x, float y, const std::string& text, float size, float width, float height)
{
    Maths::vec4 colour = {1.0, 1.0, 1.0, 1.0};
    float fontSize = 1.0f;

    Maths::vec2 pen{x, y};

    pen.y += d_font.Size();
    pen.x += (width - TextWidth(d_font, text)) / 2.0f;
    
    for (std::size_t i = 0; i != text.size(); ++i) {
        auto glyph = d_font.GetGlyph(text[i]);

        if (i > 0) {
            float kerning = d_font.GetKerning(text[i-1], text[i]);
            pen.x += kerning;
        }

        float xPos = pen.x + glyph.offset.x * fontSize;
        float yPos = pen.y - glyph.offset.y * fontSize;

        float width = glyph.width * fontSize;
        float height = glyph.height * fontSize;

        float x = glyph.texture.x;
        float y = glyph.texture.y;
        float w = glyph.texture.z;
        float h = glyph.texture.w;

        pen += glyph.advance * fontSize;

        unsigned int index = d_textVertices.size();
        d_textVertices.push_back({{xPos,         yPos},          colour, {x,     y    }});
        d_textVertices.push_back({{xPos + width, yPos},          colour, {x + w, y    }});
        d_textVertices.push_back({{xPos,         yPos + height}, colour, {x,     y + h}});
        d_textVertices.push_back({{xPos + width, yPos + height}, colour, {x + w, y + h}});

        d_textIndices.push_back(index + 0);
        d_textIndices.push_back(index + 1);
        d_textIndices.push_back(index + 2);
        d_textIndices.push_back(index + 2);
        d_textIndices.push_back(index + 1);
        d_textIndices.push_back(index + 3);
    }
}

}