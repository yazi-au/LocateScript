#pragma execution_character_set("utf-8")
#include "LlamaHttp.hpp"
#include "LocateAI.hpp"
#include <iostream>

using namespace LocateScript;

int main() {
    LLMClient client("127.0.0.1", 8080);
    StreamBoxFactory fac{};
    client.StreamChatImage(
        "locate the chatgpt",
        "C:/Users/yazi_/Desktop/aa.png",
        [&](const std::string& token) {
            fac.add(token);
            return true;
        }
    );

    std::cout << "\n";
    return 0;
}