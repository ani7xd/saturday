#pragma once
#include <string>
#include <string_view>
namespace utils {
  bool needs_tools(std::string_view prompt);
  bool use_coding_model(std::string_view mode, std::string_view prompt);
  void load_env(const std::string& path = ".env");
  std::string env(const char* name, const char* fallback = "");
}
