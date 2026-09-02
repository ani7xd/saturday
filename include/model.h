#include "conversation.h"
#include <chrono>
#include <format>
#include <filesystem>
#include <llama.h>
#include <llhttp.h>
#include <sys/wait.h>
#include "sound.h"
// #include <piper/piper.h>

#if !defined(_ANI_MODEL_H)

size_t header_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
size_t write_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
size_t model_stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
size_t model_stream_speak_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
void make_prompt( std::string& root, std::string_view value, std::string& ret );

constexpr std::string_view THINKING = "\033[2;37m";
constexpr std::string_view RESET    = "\033[0m";
constexpr std::string_view TOOL     = "\033[1;34m";

struct context {
  simdjson::ondemand::parser parser;
  simdjson::padded_string json_str;
  simdjson::fallback::ondemand::document_stream::iterator::value_type json;
  memory* conversation;
  bool tool_call;
  std::string response;
  std::string response_buffer;
  std::string function;
  // std::string parameter;
  std::string tool_arguments_json;
  std::string tool_name;
  std::string tool_id;
  void* ctx;
};

enum class stream_state {
  thinking = 0,
  responding = 1,
  tool_call = 2
};

// struct audio_chunk {
//   std::string buffer;
//   piper_synthesize_options options;
//   piper_synthesizer* piper;
//   piper_audio_chunk chunk;
// };

struct stream_context {
  std::string full_response;
  std::string buffer;
  std::string response;
  simdjson::ondemand::parser parser;
  simdjson::padded_string j_str;
  simdjson::fallback::ondemand::document_stream::iterator::value_type json;
  simdjson::simdjson_result<simdjson::fallback::ondemand::value> think;
  bool thinking;
  bool responding;
  bool tool_call;
  stream_state state;
  memory* conversation;
  std::string tool_arguments_json;
  std::string tool_name;
  std::string tool_id;
  tool_context tool;
  // audio_chunk audio;
  sound* speaker;
  void* ctx;
  stream_context( ) : thinking( false ), responding( false ), tool_call( false ) { };
};

void calling_tool( std::string_view str );

class model {
public:
  struct search_result {
    std::string title;
    std::string url;
    std::string content;
  };
public:
  void initialize( );
  void send_prompt( std::string_view prompt, const std::vector<std::string>& path );
  void set_system_prompt( std::string_view system );
  void parse_response( context* chat );
  void handle_tool_call_stream( stream_context* context );
  void speak_my_lil_nigga( const std::string& str );
public:
  model( );
  friend size_t write_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
  friend size_t prompt_write_callback( char* ptr, size_t size, size_t nmemb, void* userdata );
  ~model( );
private:
  http req;
  client_worker web;
  client_worker client;
  tool_manager tools;
  stream_context model_stream;
  context chat_context;
  memory conversation;
  fetched_resource url_result;
  llhttp_t http_parser;
  sound speaker;
  // piper_synthesizer* piper;
};

#define _ANI_MODEL_H
#endif
