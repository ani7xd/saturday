#include <llama.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#if !defined( _ANI_INFERENCE_H )

void logger( ggml_log_level level, const char * text, void * user_data );

enum class message_role {
  user = 0,
  system = 1,
  tool = 2,
  assistant = 3,
  unknown = 4
};

class inference {
public:
  void init( );
  void load_from_file( const std::filesystem::path& path );
  void generate( std::string_view prompt );
  void tokenize( std::string_view prompt, std::vector<llama_token>& tokens );
  void set_logging( );
  void apply_chat_template( std::string_view prompt, char* out, size_t len );
public:
  inference( );
  ~inference( );
public:
  llama_model* model = nullptr;
  llama_context * ctx = nullptr;
  const llama_vocab * vocab;
  llama_sampler_chain_params sampler_params;
  llama_sampler * sampler = nullptr;
  llama_model_params model_params;
  const char* tmpl;
  std::vector<llama_chat_message> chat;
  std::string buffer;
  char* buf_start;
  char* buf_len;
};

#define _ANI_INFERENCE_H
#endif