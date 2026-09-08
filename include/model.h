#include "tool_manager.h"
#include <thread>

#include "sound.h"
#include "text_stream.h"

#if !defined(_ANI_MODEL_H)

size_t header_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
size_t write_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
size_t model_stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
void make_prompt( std::string& root, std::string_view value, std::string& ret );

constexpr std::string_view THINKING = "\033[2;37m";

struct pending_tool { std::string name, id, arguments; };

struct context {
  std::deque<pending_tool> pending;
  std::string stream_error;
  simdjson::ondemand::parser parser;
  simdjson::padded_string j_str;
  simdjson::ondemand::document json;
  simdjson::simdjson_result<simdjson::ondemand::value> think;
  memory* conversation;
  bool is_thinking = false;
  bool is_responding = false;
  std::string buffer;
  std::string response;
  tool_context tool;
  void* ctx;

  context_stream stream;

  bool is_tool_call( );
  void parse_json( std::string_view );
  void thinking( std::string_view );
  void responding( std::string_view );
  context( ) : tool( ), stream( ) { }
};

enum class stream_state {
  thinking = 0,
  responding = 1,
  tool_call = 2
};

class model {
public:
  struct search_result {
    std::string title;
    std::string url;
    std::string content;
  };
public:
  void initialize( );
  void set_mode(std::string value) { mode = std::move(value); }
  void send_prompt( std::string_view prompt, const std::vector<std::string>& path );
  void set_system_prompt( std::string_view system );
  void handle_ctx( context* ctx );
  void request_response();
  void tool_call( tool_context* tool_ctx );
  void speak_my_lil_nigga( const std::string& str );
  void handle_stream_async( );
  void init_async_reply( );
public:
  model( );
  friend size_t write_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
  ~model( );
private:
  std::string mode = "auto";
  http req;
  tool_manager tools;
  context model_context;
  memory conversation;
  sound speaker;
  std::jthread worker;
};

#define _ANI_MODEL_H
#endif
