#pragma once
#include <functional>

namespace Engine
{
    class UiElement
    {
    public:
        virtual void render() {};
        std::function<void()> onClick;
    };
};