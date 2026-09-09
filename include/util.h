#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <functional>
#include <filesystem>
#include <vector>
#include <chrono>
#include <format>
#include <string.h>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>

namespace utils {
  inline std::filesystem::path native_path(std::string_view value) {
    return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size()));
  }
  inline std::string path_text(const std::filesystem::path& value) {
    auto text = value.generic_u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
  }
  bool str_cmp( std::string_view str1, std::string_view str2 );
  bool str_cmp( const std::string& str1, const std::string& str2 );
}

#include "config.h"
