#pragma execution_character_set("utf-8")
#include "InputSim.hpp"
#include "LlamaHttp.hpp"
#include "LocateAI.hpp"
#include <iostream>
#include <Windows.h>

using namespace LocateScript;

int main() {
    LLMClient client("127.0.0.1", 8080);
    StreamBoxFactory fac{};
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    client.StreamChatImage(
        "locate 用户头像",
        "C:/Users/yazi_/Desktop/aa.png",
        [&](const std::string& token) {
            std::cout << token;
            fac.add(token);
            return true;
        }
    );

    SimpleMouse mouse;
    Vec2i c = fac.getBoxes()->at(0).getCenter();
    mouse.MoveTo(c.x, c.y);
    std::cout << "\n";
    return 0;
}