#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace LocateScript {
    class IMouse {
    public:
        virtual ~IMouse() = default;

        virtual bool MoveTo(int x, int y) = 0;
        virtual bool MoveBy(int dx, int dy) = 0;

        virtual bool LeftDown() = 0;
        virtual bool LeftUp() = 0;
        virtual bool LeftClick() = 0;

        virtual bool RightDown() = 0;
        virtual bool RightUp() = 0;
        virtual bool RightClick() = 0;

        virtual bool MiddleDown() = 0;
        virtual bool MiddleUp() = 0;
        virtual bool MiddleClick() = 0;

        virtual bool Scroll(int delta) = 0;
    };

    class IKeyboard {
    public:
        virtual ~IKeyboard() = default;

        virtual bool KeyDown(WORD vk) = 0;
        virtual bool KeyUp(WORD vk) = 0;
        virtual bool KeyPress(WORD vk) = 0;

        virtual bool TypeText(const std::wstring& text) = 0;
    };

    class SimpleMouse final : public IMouse {
    public:
        bool MoveTo(int x, int y) override;
        bool MoveBy(int dx, int dy) override;

        bool LeftDown() override;
        bool LeftUp() override;
        bool LeftClick() override;

        bool RightDown() override;
        bool RightUp() override;
        bool RightClick() override;

        bool MiddleDown() override;
        bool MiddleUp() override;
        bool MiddleClick() override;

        bool Scroll(int delta) override;
    };

    class SimpleKeyboard final : public IKeyboard {
    public:
        bool KeyDown(WORD vk) override;
        bool KeyUp(WORD vk) override;
        bool KeyPress(WORD vk) override;

        bool TypeText(const std::wstring& text) override;
    };
}