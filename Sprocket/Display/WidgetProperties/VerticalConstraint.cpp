#include "VerticalConstraint.h"
#include "Log.h"
#include "Widget.h"

namespace Sprocket {

VerticalConstraint::VerticalConstraint(Type type, float offset, Window* window)
    : d_type(type)
    , d_offset(offset)
    , d_window(window)
{
}

void VerticalConstraint::update(Widget* widget)
{
    auto current_position = widget->position();

    switch (d_type) {
        case Type::TOP: {
            widget->position({
                current_position.x,
                d_offset
            });
        } break;
        case Type::BOTTOM: {
            widget->position({
                current_position.x,
                (float)d_window->height() - widget->height() - d_offset
            });
        } break;
        case Type::CENTRE: {
            widget->position({
                current_position.x,
                ((float)d_window->height() - widget->height())/2.0f
            });
        } break;
        default: {
            SPKT_LOG_ERROR("VerticalConstraint encountered an unknown type!");
        }  
    }
}

}