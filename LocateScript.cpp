#pragma execution_character_set("utf-8")
#include "LocateAI.hpp"
#include "LlamaHttp.hpp"
#include <iostream>

using namespace LocateScript;

int main() {
    LLMClient client("127.0.0.1", 8080);
    std::cout << "=== Stream Image ===\n";
    client.StreamChatImage(
        "locate the chatgpt",
        "C:/Users/yazi_/Desktop/aa.png",
        [](const std::string& token) {
            std::cout << token;
            return true;
        }
    );

    std::cout << "\n";
    return 0;
}