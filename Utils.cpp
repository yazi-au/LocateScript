#include "Utils.hpp"

namespace LocateScript {
	std::vector<std::string_view> str_split(const std::string& input, const std::string& delimiter_str) {
        if (input.empty() || delimiter_str.empty()) {
            return input.empty() ? std::vector<std::string_view>() : std::vector<std::string_view>({ input });
        }

        size_t len = delimiter_str.size();
        std::vector<std::string_view> result;
        std::string_view str(input);
        size_t start = 0;
        size_t end = 0;

        while ((end = str.find(delimiter_str, start)) != std::string_view::npos) {
            std::string_view chunk = str.substr(start, end - start);
            result.emplace_back(chunk);
            start = end + len;
        }
        std::string_view chunk = str.substr(start);
        if (!chunk.empty()) {
            result.emplace_back(chunk);
        }

        return result;
    }
}