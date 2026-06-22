#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "json.hpp"

namespace LocateScript {
    class LLMClient {
    public:
        using StreamCallback = std::function<bool(const std::string& token)>;

        LLMClient(
            std::string host = "127.0.0.1",
            int port = 8080,
            std::string model = "",
            std::string apiKey = "");

        bool Chat(
            const std::string& prompt,
            std::string& response,
            const std::string& systemPrompt = "");

        bool ChatImage(
            const std::string& prompt,
            const std::string& imagePath,
            std::string& response,
            const std::string& systemPrompt = "");

        bool ChatScreen(
            const std::string& prompt,
            std::string& response,
            const std::string& systemPrompt = "");

        bool StreamChat(
            const std::string& prompt,
            const StreamCallback& onToken,
            const std::string& systemPrompt = "");

        bool StreamChatImage(
            const std::string& prompt,
            const std::string& imagePath,
            const StreamCallback& onToken,
            const std::string& systemPrompt = "");

        bool StreamChatScreen(
            const std::string& prompt,
            const StreamCallback& onToken,
            const std::string& systemPrompt = "");

        static bool CaptureScreen(const std::string& savePath);
        static bool CaptureScreenToJpeg(std::vector<std::uint8_t>& outJpeg);
        static std::string Base64Encode(const std::vector<std::uint8_t>& data);

    private:
        bool SendChatRequest(
            const nlohmann::json& requestBody,
            std::string& response,
            bool stream,
            const StreamCallback* onToken) const;

        static bool ReadFileBinary(
            const std::string& path,
            std::vector<std::uint8_t>& outBytes);

        static bool WriteFileBinary(
            const std::string& path,
            const std::vector<std::uint8_t>& data);

        static std::string GuessMimeTypeFromPath(
            const std::string& path);

        static std::string MakeDataUriFromFile(
            const std::string& imagePath);

        static std::string MakeDataUriFromJpeg(
            const std::vector<std::uint8_t>& jpegBytes);

        static std::string TrimCopy(
            const std::string& s);

    private:
        std::string m_host;
        int m_port;
        std::string m_model;
        std::string m_apiKey;
    };
}