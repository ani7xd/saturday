#include "model.h"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <type_traits>
void check(bool ok) { if (!ok) throw std::runtime_error("Core test failed"); }
int main() {
  check(utils::use_coding_model("auto", "web search and tell me about spyderman bnd"));
  check(utils::use_coding_model("auto", "Search the internet for Spider-Man news"));
  check(utils::use_coding_model("auto", "look up Spider-Man Brand New Day"));
  check(utils::use_coding_model("auto", "Use web_search to find official sources"));
  check(utils::use_coding_model("auto", "read main.cpp and explain it"));
  check(!utils::use_coding_model("auto", "Tell me a Spider-Man story"));
  check(!utils::use_coding_model("chat", "web search for Spider-Man"));
  check(utils::needs_tools("web search for Spider-Man"));
  static_assert(std::is_same_v<decltype(statement{}.param.lengths)::value_type, unsigned long>);
  memory conversation;
  std::string encoded;
  for (auto pair : {std::pair{"", ""}, {"f", "Zg=="}, {"fo", "Zm8="}, {"foo", "Zm9v"}, {"foobar", "Zm9vYmFy"}}) {
    conversation.encode_image(pair.first, encoded);
    check(encoded == pair.second);
  }
  context ctx;
  std::string first = R"({"message":{"role":"assistant","thinking":"step","content":"hel)";
  check(model_stream_callback(first.data(), 1, first.size(), &ctx) == first.size());
  check(ctx.response.empty());
  std::string second = "lo\"},\"done\":false}\n";
  check(model_stream_callback(second.data(), 1, second.size(), &ctx) == second.size());
  check(ctx.response == "hello");
  check(ctx.is_thinking && ctx.is_responding);
  ctx.response.clear();
  std::string done = "{\"done\":true}\n";
  check(model_stream_callback(done.data(), 1, done.size(), &ctx) == done.size());
  check(!ctx.is_responding && !ctx.is_thinking);
  std::string error = "{\"error\":\"model missing\"}\n";
  check(model_stream_callback(error.data(), 1, error.size(), &ctx) == 0);
  check(ctx.stream_error == "model missing");
  tool_manager tools;
  tools.load_tools(std::filesystem::path(SATURDAY_SOURCE_DIR) / "tools");
  check(tools.tools.size() == 10);
  for (const auto& schema : tools.tools) {
    auto function = yyjson_obj_get(schema.root, "function");
    auto name = yyjson_get_str(yyjson_obj_get(function, "name"));
    check(name && tools.tool_map.contains(name));
  }
  auto path = std::filesystem::current_path() / std::filesystem::path(u8"core-test-\u00e9-\u6d4b\u8bd5.txt");
  {std::ofstream f(path); f << "one two one";}
  edit_result result;
  tools.edit_file(path, "one", "three", &result, 1, 1);
  check(result.success && result.replaced_count == 1);
  std::string contents;
  tools.read_file(utils::path_text(path), contents);
  check(contents == "one two three");
  std::string written;
  tools.write_file(utils::path_text(path), "!", written);
  tools.read_file(utils::path_text(path), contents);
  check(contents == "one two three!");
  free(result.json);
  std::filesystem::remove(path);
  tool_result output;
  tools.list_directories(std::filesystem::current_path(), &output);
  check(output.json != nullptr);
  output.clear(); output.clear();
  check(output.json == nullptr);
  std::cout << "Core tests passed\n";
}
