#include "include/model.h"
#ifdef _WIN32
#include <windows.h>
#endif
int main(int argc, char** argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  DWORD mode = 0;
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (GetConsoleMode(output, &mode)) SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
  try {
    utils::load_env();
    if (argc > 1 && (std::string_view(argv[1]) == "--init-db" || std::string_view(argv[1]) == "--check-db")) {
      database db; db.initialize();
      auto name = utils::env("DATABASE_PATH", "saturday");
      bool setup = std::string_view(argv[1]) == "--init-db";
      if (name.empty() || name.size() > 64 || name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos)
        throw std::runtime_error("DATABASE_PATH must be a simple database name (letters, digits, underscores)");
      db.connect(utils::env("DATABASE_HOST", "127.0.0.1"), utils::env("DATABASE_USERNAME"), utils::env("DATABASE_PASSWORD"), setup ? "" : name);
      if (setup) {
        std::string script; db.load_stmt_file("sql/schema.sql", script);
        if (script.empty()) throw std::runtime_error("Run from the Saturday project directory (sql/schema.sql missing)");
        size_t position = 0;
        while ((position = script.find("`saturday`", position)) != std::string::npos) { script.replace(position, 10, "`" + name + "`"); position += name.size() + 2; }
        size_t begin = 0, end;
        while ((end = script.find(';', begin)) != std::string::npos) {
          auto query = script.substr(begin, end - begin);
          if (mysql_query(db.conn, query.c_str())) throw std::runtime_error(mysql_error(db.conn));
          begin = end + 1;
        }
        std::cout << "Database and tables are ready.\n";
      } else std::cout << "MySQL connection successful.\n";
      return 0;
    }
    model assistant;
    assistant.initialize();
    std::string prompt, line;
    std::vector<std::string> images;
    for (;;) {
      std::cout << "msg>>> " << std::flush;
      while (std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") return 0;
        if (line == "/end") break;
        if (line == "/code" || line == "/chat" || line == "/auto") {
          assistant.set_mode(line.substr(1));
          std::cout << "Mode: " << line.substr(1) << "\n";
          continue;
        }
        if (line.starts_with("img>>> ")) images.emplace_back(line.substr(7));
        else {
          if (!prompt.empty()) prompt += '\n';
          prompt += line;
        }
      }
      if (!prompt.empty()) assistant.send_prompt(prompt, images);
      if (!std::cin) break;
      images.clear();
      prompt.clear();
    }
  } catch (const std::exception& error) {
    std::cerr << "Saturday: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
