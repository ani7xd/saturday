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
#include <unistd.h>

namespace utils {
  bool str_cmp( std::string_view str1, std::string_view str2 );
  bool str_cmp( const std::string& str1, const std::string& str2 );
}
