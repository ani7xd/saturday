#include "../include/config.h"
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <regex>

bool utils::needs_tools(std::string_view prompt) {
  static const std::regex request(
    R"(\b(web[ -]?search|search\s+(the\s+)?(web|internet|online)|search\s+for|look\s+up|browse\s+(the\s+)?web|fetch\s+(a\s+|the\s+)?(url|page)|read\s+(a\s+|the\s+)?file|list\s+(the\s+)?(files|director(y|ies))|current\s+(date|time)|web_search|fetch_url|read_file|get_current_datetime)\b)",
    std::regex::icase);
  return std::regex_search(prompt.begin(), prompt.end(), request);
}

bool utils::use_coding_model(std::string_view mode, std::string_view prompt) {
  static const std::regex coding(
    R"(\b(code|coding|debug|compile|compiler|programming|function|refactor|python|javascript|typescript|cmake|sql|repository|implement|bug)\b|c\+\+|\.(cpp|hpp|py|js|ts)\b)",
    std::regex::icase);
  return mode == "code" || (mode == "auto" &&
    (needs_tools(prompt) || std::regex_search(prompt.begin(), prompt.end(), coding)));
}

std::string utils::env(const char* name, const char* fallback) {
  const char* value = std::getenv(name);
  return value ? value : fallback;
}
void utils::load_env(const std::string& path) {
  std::ifstream file(path);
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    auto begin = line.find_first_not_of(" \t");
    if (begin == std::string::npos || line[begin] == '#') continue;
    line.erase(0, begin);
    if (line.starts_with("export ")) line.erase(0, 7);
    auto equal = line.find('=');
    if (equal == std::string::npos) continue;
    std::string key = line.substr(0, equal), value = line.substr(equal + 1);
    auto end = key.find_last_not_of(" \t");
    if (end == std::string::npos) continue;
    key.resize(end + 1);
    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front())
      value = value.substr(1, value.size() - 2);
    if (std::getenv(key.c_str())) continue;
#ifdef _WIN32
    _putenv_s(key.c_str(), value.c_str());
#else
    setenv(key.c_str(), value.c_str(), 0);
#endif
  }
}
