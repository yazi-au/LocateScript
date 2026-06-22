#include "InputSim.hpp"

#include <vector>

namespace LocateScript {
    bool SendInputs(const std::vector<INPUT>& inputs) {
        if (inputs.empty()) return true;
        UINT sent = ::SendInput(static_cast<UINT>(inputs.size()),
                                const_cast<INPUT*>(inputs.data()),
                                static_cast<int>(sizeof(INPUT)));
        return sent == inputs.size();
    }

    INPUT MakeMouseInput(DWORD flags, LONG dx = 0, LONG dy = 0, DWORD data = 0) {
        INPUT in{};
        in.type = INPUT_MOUSE;
        in.mi.dx = dx;
        in.mi.dy = dy;
        in.mi.mouseData = data;
        in.mi.dwFlags = flags;
        in.mi.time = 0;
        in.mi.dwExtraInfo = 0;
        return in;
    }

    INPUT MakeKeyboardInput(WORD vk, DWORD flags) {
        INPUT in{};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = vk;
        in.ki.wScan = 0;
        in.ki.dwFlags = flags;
        in.ki.time = 0;
        in.ki.dwExtraInfo = 0;
        return in;
    }

    INPUT MakeUnicodeKeyboardInput(wchar_t ch, DWORD flags) {
        INPUT in{};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = 0;
        in.ki.wScan = ch;
        in.ki.dwFlags = KEYEVENTF_UNICODE | flags;
        in.ki.time = 0;
        in.ki.dwExtraInfo = 0;
        return in;
    }

    bool SimpleMouse::MoveTo(int x, int y) {
        return ::SetCursorPos(x, y) != 0;
    }

    bool SimpleMouse::MoveBy(int dx, int dy) {
        POINT pt{};
        if (!::GetCursorPos(&pt)) return false;
        return ::SetCursorPos(pt.x + dx, pt.y + dy) != 0;
    }

    bool SimpleMouse::LeftDown() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_LEFTDOWN) });
    }

    bool SimpleMouse::LeftUp() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_LEFTUP) });
    }

    bool SimpleMouse::LeftClick() {
        return SendInputs({
            MakeMouseInput(MOUSEEVENTF_LEFTDOWN),
            MakeMouseInput(MOUSEEVENTF_LEFTUP)
                          });
    }

    bool SimpleMouse::RightDown() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_RIGHTDOWN) });
    }

    bool SimpleMouse::RightUp() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_RIGHTUP) });
    }

    bool SimpleMouse::RightClick() {
        return SendInputs({
            MakeMouseInput(MOUSEEVENTF_RIGHTDOWN),
            MakeMouseInput(MOUSEEVENTF_RIGHTUP)
                          });
    }

    bool SimpleMouse::MiddleDown() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_MIDDLEDOWN) });
    }

    bool SimpleMouse::MiddleUp() {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_MIDDLEUP) });
    }

    bool SimpleMouse::MiddleClick() {
        return SendInputs({
            MakeMouseInput(MOUSEEVENTF_MIDDLEDOWN),
            MakeMouseInput(MOUSEEVENTF_MIDDLEUP)
                          });
    }

    bool SimpleMouse::Scroll(int delta) {
        return SendInputs({ MakeMouseInput(MOUSEEVENTF_WHEEL, 0, 0, static_cast<DWORD>(delta)) });
    }

    bool SimpleKeyboard::KeyDown(WORD vk) {
        return SendInputs({ MakeKeyboardInput(vk, 0) });
    }

    bool SimpleKeyboard::KeyUp(WORD vk) {
        return SendInputs({ MakeKeyboardInput(vk, KEYEVENTF_KEYUP) });
    }

    bool SimpleKeyboard::KeyPress(WORD vk) {
        return SendInputs({
            MakeKeyboardInput(vk, 0),
            MakeKeyboardInput(vk, KEYEVENTF_KEYUP)
                          });
    }

    bool SimpleKeyboard::TypeText(const std::wstring& text) {
        std::vector<INPUT> inputs;
        inputs.reserve(text.size() * 2);

        for (wchar_t ch : text) {
            inputs.push_back(MakeUnicodeKeyboardInput(ch, 0));
            inputs.push_back(MakeUnicodeKeyboardInput(ch, KEYEVENTF_KEYUP));
        }

        return SendInputs(inputs);
    }
}