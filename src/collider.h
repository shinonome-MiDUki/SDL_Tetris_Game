#pragma once

#include <variant>

enum ColliderType{
    CIRCLE,
    LINE,
    RECT
};

struct CircleData { double x; double y; double r; };
struct RectData { double x; double y; double w; double h; };
struct LineData { double x1; double x2; double y1; double y2; };

using ColliderData = std::variant<CircleData, RectData, LineData>;

struct Collider
{
    public:
        ColliderType collider_type;
        ColliderData collider_data;
        
};