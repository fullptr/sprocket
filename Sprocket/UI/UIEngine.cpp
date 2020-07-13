#include "UIEngine.h"
#include "MouseCodes.h"
#include "Log.h"

namespace Sprocket {

UIEngine::UIEngine(KeyboardProxy* keyboard, MouseProxy* mouse)
    : d_keyboard(keyboard)
    , d_mouse(mouse)
{
}

WidgetInfo UIEngine::RegisterWidget(const std::string& name,
                                    const Maths::vec4& region)
{
    WidgetInfo info;
    d_quads.push_back({name, region});
    std::size_t hash = std::hash<std::string>{}(name);

    if (hash == d_clicked) {
        info.clicked = d_clickedTime;
    }
    else {
        info.unclicked = d_time - d_unclickedTimes[hash];
    }

    if (hash == d_hovered) {
        info.hovered = d_hoveredTime;
    }
    else {
        info.unhovered = d_time - d_unhoveredTimes[hash];
    }

    if (d_onClick == hash) {
        d_onClick = 0;
        info.onClick = true;
    }

    return info;
}

void UIEngine::StartFrame()
{
    d_hoveredFlag = false;
    d_clickedFlag = false;
    d_quads.clear();
}

void UIEngine::EndFrame()
{
    bool isHovered = false;
    bool isClicked = false;

    for (std::size_t i = d_quads.size(); i != 0;) {
        --i;
        const auto& quad = d_quads[i];
        std::size_t hash = std::hash<std::string>{}(quad.name);
        auto hovered = d_mouse->InRegion(quad.region.x, quad.region.y, quad.region.z, quad.region.w);
        auto clicked = hovered && d_mouse->IsButtonClicked(Mouse::LEFT);

        if (clicked) {
            d_onClick = hash;
        }

        if (((d_clicked == hash) || clicked) && !isClicked) {
            isClicked = true;
            if (d_clicked != hash) {
                d_clicked = hash;
                d_clickedTime = 0.0;
            }
        }
        
        if (hovered && !isHovered) {
            isHovered = true;
            if (d_hovered != hash) {
                d_unhoveredTimes[d_hovered] = d_time;
                d_hovered = hash;
                d_hoveredTime = 0.0;
            }
        }
    }

    if (isHovered == false) {
        d_hoveredTime = 0.0;
        if (d_hovered > 0) {
            d_unhoveredTimes[d_hovered] = d_time;
            d_hovered = 0;
        }
    }
}

void UIEngine::OnUpdate(double dt)
{
    d_time += dt;
    d_clickedTime += dt;
    d_hoveredTime += dt;

    if (d_mouse->IsButtonReleased(Mouse::LEFT)) {
        d_clickedTime = 0.0;
        if (d_clicked > 0) {
            d_unclickedTimes[d_clicked] = d_time;
            d_clicked = 0;
        }
    }
}

}