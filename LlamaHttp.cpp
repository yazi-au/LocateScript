#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "LlamaHttp.hpp"
#include "httplib.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

#include <windows.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

namespace LocateScript {
    using json = nlohmann::json;

    namespace {
        bool HasPrefix(const std::string& s, const char* prefix) {
            const size_t n = std::strlen(prefix);
            return s.size() >= n && std::memcmp(s.data(), prefix, n) == 0;
        }

        std::string ToLowerCopy(std::string s) {
            std::transform(
                s.begin(),
                s.end(),
                s.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return s;
        }

        std::string GetFileExtensionLower(const std::string& path) {
            const auto dotPos = path.find_last_of('.');
            if (dotPos == std::string::npos) {
                return {};
            }
            return ToLowerCopy(path.substr(dotPos));
        }

        std::string MimeFromExtension(const std::string& extLower) {
            if (extLower == ".jpg" || extLower == ".jpeg") return "image/jpeg";
            if (extLower == ".png") return "image/png";
            if (extLower == ".webp") return "image/webp";
            if (extLower == ".bmp") return "image/bmp";
            return "image/jpeg";
        }

        std::string TrimCopyImpl(const std::string& s) {
            size_t begin = 0;
            while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) {
                ++begin;
            }

            size_t end = s.size();
            while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
                --end;
            }

            return s.substr(begin, end - begin);
        }

        std::string Utf8WideNote() {
            return {};
        }

        bool CaptureScreenMat(cv::Mat& outBgr) {
#ifdef _WIN32
            const int x = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
            const int y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
            const int width = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
            const int height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

            if (width <= 0 || height <= 0) {
                return false;
            }

            HDC screenDC = ::GetDC(nullptr);
            if (!screenDC) {
                return false;
            }

            HDC memDC = ::CreateCompatibleDC(screenDC);
            if (!memDC) {
                ::ReleaseDC(nullptr, screenDC);
                return false;
            }

            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = width;
            bmi.bmiHeader.biHeight = -height; // top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            void* bits = nullptr;
            HBITMAP dib = ::CreateDIBSection(
                screenDC,
                &bmi,
                DIB_RGB_COLORS,
                &bits,
                nullptr,
                0);

            if (!dib || !bits) {
                ::DeleteDC(memDC);
                ::ReleaseDC(nullptr, screenDC);
                return false;
            }

            HGDIOBJ oldObj = ::SelectObject(memDC, dib);

            const BOOL bltOk = ::BitBlt(
                memDC,
                0,
                0,
                width,
                height,
                screenDC,
                x,
                y,
                SRCCOPY | CAPTUREBLT);

            bool ok = false;
            if (bltOk) {
                cv::Mat bgra(height, width, CV_8UC4, bits);
                cv::cvtColor(bgra, outBgr, cv::COLOR_BGRA2BGR);
                ok = !outBgr.empty();
            }

            ::SelectObject(memDC, oldObj);
            ::DeleteObject(dib);
            ::DeleteDC(memDC);
            ::ReleaseDC(nullptr, screenDC);

            return ok;
#else
            (void) outBgr;
            return false;
#endif
        }

        bool EncodeMatToJpeg(
            const cv::Mat& bgr,
            std::vector<std::uint8_t>& outJpeg) {
            if (bgr.empty()) {
                return false;
            }

            std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, 95 };
            return cv::imencode(".jpg", bgr, outJpeg, params);
        }

        std::string BytesToBase64(const std::vector<std::uint8_t>& data) {
            static constexpr char kTable[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            std::string out;
            out.reserve(((data.size() + 2) / 3) * 4);

            int val = 0;
            int valb = -6;

            for (std::uint8_t c : data) {
                val = (val << 8) | c;
                valb += 8;
                while (valb >= 0) {
                    out.push_back(kTable[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }

            if (valb > -6) {
                out.push_back(kTable[((val << 8) >> (valb + 8)) & 0x3F]);
            }

            while (out.size() % 4 != 0) {
                out.push_back('=');
            }

            return out;
        }
    }

    LLMClient::LLMClient(
        std::string host,
        int port,
        std::string model,
        std::string apiKey)
        : m_host(std::move(host)),
        m_port(port),
        m_model(std::move(model)),
        m_apiKey(std::move(apiKey)) {}

    std::string LLMClient::TrimCopy(const std::string& s) {
        return TrimCopyImpl(s);
    }

    std::string LLMClient::Base64Encode(const std::vector<std::uint8_t>& data) {
        return BytesToBase64(data);
    }

    bool LLMClient::ReadFileBinary(
        const std::string& path,
        std::vector<std::uint8_t>& outBytes) {
        outBytes.clear();

#ifdef _WIN32
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) {
            return false;
        }

        ifs.seekg(0, std::ios::end);
        const std::streamsize size = ifs.tellg();
        if (size < 0) {
            return false;
        }
        ifs.seekg(0, std::ios::beg);

        outBytes.resize(static_cast<size_t>(size));
        if (!ifs.read(reinterpret_cast<char*>(outBytes.data()), size)) {
            outBytes.clear();
            return false;
        }

        return true;
#else
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) {
            return false;
        }

        ifs.seekg(0, std::ios::end);
        const std::streamsize size = ifs.tellg();
        if (size < 0) {
            return false;
        }
        ifs.seekg(0, std::ios::beg);

        outBytes.resize(static_cast<size_t>(size));
        if (!ifs.read(reinterpret_cast<char*>(outBytes.data()), size)) {
            outBytes.clear();
            return false;
        }

        return true;
#endif
    }

    bool LLMClient::WriteFileBinary(
        const std::string& path,
        const std::vector<std::uint8_t>& data) {
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs) {
            return false;
        }

        ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return static_cast<bool>(ofs);
    }

    std::string LLMClient::GuessMimeTypeFromPath(const std::string& path) {
        return MimeFromExtension(GetFileExtensionLower(path));
    }

    std::string LLMClient::MakeDataUriFromFile(const std::string& imagePath) {
        std::vector<std::uint8_t> bytes;
        if (!ReadFileBinary(imagePath, bytes) || bytes.empty()) {
            return {};
        }

        const std::string mime = GuessMimeTypeFromPath(imagePath);
        return "data:" + mime + ";base64," + Base64Encode(bytes);
    }

    std::string LLMClient::MakeDataUriFromJpeg(const std::vector<std::uint8_t>& jpegBytes) {
        if (jpegBytes.empty()) {
            return {};
        }

        return "data:image/jpeg;base64," + Base64Encode(jpegBytes);
    }

    bool LLMClient::CaptureScreenToJpeg(std::vector<std::uint8_t>& outJpeg) {
        cv::Mat bgr;
        if (!CaptureScreenMat(bgr)) {
            return false;
        }

        return EncodeMatToJpeg(bgr, outJpeg);
    }

    bool LLMClient::CaptureScreen(const std::string& savePath) {
        cv::Mat bgr;
        if (!CaptureScreenMat(bgr)) {
            return false;
        }

        const std::string ext = GetFileExtensionLower(savePath);
        std::string encodeExt = ".jpg";

        if (ext == ".png") encodeExt = ".png";
        else if (ext == ".bmp") encodeExt = ".bmp";
        else if (ext == ".webp") encodeExt = ".webp";

        std::vector<std::uint8_t> encoded;
        std::vector<int> params;

        if (encodeExt == ".jpg") {
            params = { cv::IMWRITE_JPEG_QUALITY, 95 };
        }

        if (!cv::imencode(encodeExt, bgr, encoded, params)) {
            return false;
        }

        return WriteFileBinary(savePath, encoded);
    }

    bool LLMClient::SendChatRequest(
        const json& requestBody,
        std::string& response,
        bool stream,
        const StreamCallback* onToken) const {
        response.clear();

        httplib::Client cli(m_host, m_port);

        cli.set_read_timeout(300, 0);
        cli.set_write_timeout(300, 0);
        cli.set_connection_timeout(10, 0);

        httplib::Headers headers;
        headers.emplace("Content-Type", "application/json");

        if (!m_apiKey.empty()) {
            headers.emplace("Authorization", "Bearer " + m_apiKey);
        }

        std::string body = requestBody.dump();

        // =========================
        // 非流式（完全正常）
        // =========================
        if (!stream) {
            auto res = cli.Post(
                "/v1/chat/completions",
                body,
                "application/json"
            );

            if (!res) {
                return false;
            }

            if (res->status < 200 || res->status >= 300) {
                response = res->body;
                return false;
            }

            try {
                auto j = json::parse(res->body);

                if (j.contains("choices") &&
                    j["choices"].is_array() &&
                    !j["choices"].empty()) {
                    auto& c = j["choices"][0];

                    if (c.contains("message") &&
                        c["message"].contains("content")) {
                        response = c["message"]["content"].get<std::string>();
                        return true;
                    }
                }

                response = res->body;
                return true;
            } catch (...) {
                response = res->body;
                return true;
            }
        }

        // =========================
        // 流式（关键修复）
        // =========================

        std::string sseBuffer;
        bool cancelled = false;

        auto res = cli.Post(
            "/v1/chat/completions",
            headers,
            body,
            "application/json",
            [&](const char* data, size_t len) {
                sseBuffer.append(data, len);

                size_t pos = 0;

                while (true) {
                    size_t end = sseBuffer.find('\n', pos);
                    if (end == std::string::npos)
                        break;

                    std::string line = sseBuffer.substr(pos, end - pos);

                    if (!line.empty() && line.back() == '\r')
                        line.pop_back();

                    pos = end + 1;

                    line = TrimCopy(line);

                    if (!HasPrefix(line, "data:"))
                        continue;

                    std::string payload = TrimCopy(line.substr(5));

                    if (payload == "[DONE]")
                        return false;

                    try {
                        auto j = json::parse(payload);

                        auto& delta = j["choices"][0]["delta"];

                        if (delta.contains("content")) {
                            std::string token = delta["content"];

                            if (onToken && !token.empty()) {
                                if (!(*onToken)(token)) {
                                    cancelled = true;
                                    return false;
                                }
                            }
                        }
                    } catch (...) {
                    }
                }

                if (pos > 0)
                    sseBuffer.erase(0, pos);

                return !cancelled;
            }
        );

        if (!res) {
            return false;
        }

        return !cancelled;
    }

    bool LLMClient::Chat(
        const std::string& prompt,
        std::string& response,
        const std::string& systemPrompt) {
        json req;
        if (!m_model.empty()) {
            req["model"] = m_model;
        }

        req["messages"] = json::array();

        if (!systemPrompt.empty()) {
            req["messages"].push_back({
                {"role", "system"},
                {"content", systemPrompt}
                                      });
        }

        req["messages"].push_back({
            {"role", "user"},
            {"content", prompt}
                                  });

        return SendChatRequest(req, response, false, nullptr);
    }

    bool LLMClient::ChatImage(
        const std::string& prompt,
        const std::string& imagePath,
        std::string& response,
        const std::string& systemPrompt) {
        const std::string dataUri = MakeDataUriFromFile(imagePath);
        if (dataUri.empty()) {
            return false;
        }

        json req;
        if (!m_model.empty()) {
            req["model"] = m_model;
        }

        req["messages"] = json::array();

        if (!systemPrompt.empty()) {
            req["messages"].push_back({
                {"role", "system"},
                {"content", systemPrompt}
                                      });
        }

        req["messages"].push_back({
            {"role", "user"},
            {"content", json::array({
                json{
                    {"type", "image_url"},
                    {"image_url", {
                        {"url", dataUri},
                        {"detail", "auto"}
                    }}
                }
            })}
                                  });

        req["messages"].push_back({
            {"role", "user"},
            {"content", prompt}
                                  });

        return SendChatRequest(req, response, false, nullptr);
    }

    bool LLMClient::StreamChat(
        const std::string& prompt,
        const StreamCallback& onToken,
        const std::string& systemPrompt) {
        json req;
        if (!m_model.empty()) {
            req["model"] = m_model;
        }

        req["stream"] = true;
        req["messages"] = json::array();

        if (!systemPrompt.empty()) {
            req["messages"].push_back({
                {"role", "system"},
                {"content", systemPrompt}
                                      });
        }

        req["messages"].push_back({
            {"role", "user"},
            {"content", prompt}
                                  });

        std::string dummy;
        return SendChatRequest(req, dummy, true, &onToken);
    }

    bool LLMClient::StreamChatImage(
        const std::string& prompt,
        const std::string& imagePath,
        const StreamCallback& onToken,
        const std::string& systemPrompt) {
        const std::string dataUri = MakeDataUriFromFile(imagePath);
        if (dataUri.empty()) {
            return false;
        }

        json req;
        if (!m_model.empty()) {
            req["model"] = m_model;
        }

        req["stream"] = true;
        req["messages"] = json::array();

        if (!systemPrompt.empty()) {
            req["messages"].push_back({
                {"role", "system"},
                {"content", systemPrompt}
            });
        }

        req["messages"].push_back({
            {"role", "user"},
            {"content", json::array({
                json{
                    {"type", "image_url"},
                    {"image_url", {
                        {"url", dataUri},
                        {"detail", "auto"}
                    }}
                }
            })}
        });

        req["messages"].push_back({
            {"role", "user"},
            {"content", prompt}
        });

        std::string dummy;
        return SendChatRequest(req, dummy, true, &onToken);
    }
}