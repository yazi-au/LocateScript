#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace LocateScript {
    std::vector<std::string_view> str_split(const std::string& input, const std::string& delimiter_str);
}