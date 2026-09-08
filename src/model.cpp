#include "../include/model.h"
#include <memory>

// void model::speak_my_lil_nigga( const std::string& str ) {
//   auto rc = piper_synthesize_start( this->piper, str.c_str( ), &model_stream.audio.options );
//   while ( rc != PIPER_DONE ) {
//     rc = piper_synthesize_next( this->piper, &model_stream.audio.chunk );
//     if ( model_stream.audio.chunk.num_samples > 0 ) {
//       this->speaker.speak( model_stream.audio.chunk.samples, model_stream.audio.chunk.num_samples );
//     } else break;
//   }
// }

void model::send_prompt( std::string_view prompt, const std::vector<std::string>& path ) {
  if ( prompt.empty( ) ) return;
  bool use_coding = utils::use_coding_model(mode, prompt);
  auto name = utils::env(use_coding ? "OLLAMA_CODING_MODEL" : "OLLAMA_MODEL", use_coding ? "qwen3.5:9b" : "gemma3:12b");
  bool tools_enabled = utils::env(use_coding ? "OLLAMA_CODING_TOOLS" : "OLLAMA_TOOLS", use_coding ? "true" : "false") == "true";
  bool thinking_enabled = utils::env(use_coding ? "OLLAMA_CODING_THINKING" : "OLLAMA_THINKING", use_coding ? "true" : "false") == "true";
  if (!tools_enabled && utils::needs_tools(prompt)) {
    std::cout << "This request needs tools. Enter /auto or /code and submit it again; "
                 "the selected chat model cannot perform a search or access files.\n";
    return;
  }
  conversation.select_model(name, tools_enabled, thinking_enabled);
  std::cout << "[model] " << name << "\n" << std::flush;
  if ( path.size( ) == 0 )
    conversation.store( "user", "text", prompt );
  else {
    yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
    yyjson_mut_val* images = yyjson_mut_arr( doc );
    yyjson_mut_doc_set_root( doc, images );
    for ( auto& i : path ) {
      yyjson_mut_arr_add_strncpy( doc, images, i.data( ), i.size( ) );
    }
    size_t len;
    char* j_str = yyjson_mut_write( doc, 0, &len );
    conversation.store( "user", "text", prompt, { j_str, len } );
    free( j_str );
    yyjson_mut_doc_free( doc );
  }
  std::string_view data = conversation.load_conversation( );

  req.set_post_data( data );
  request_response();
  handle_ctx( &model_context );
}

void model::handle_ctx( context* ctx ) {
  int i = 0;
  std::string_view data;
  while ( ctx->is_tool_call( ) ) {
    if (i >= 32) throw std::runtime_error("Tool round limit reached");
    while (!ctx->pending.empty()) {
      auto next = std::move(ctx->pending.front()); ctx->pending.pop_front();
      ctx->tool.tool_name = std::move(next.name);
      ctx->tool.tool_id = std::move(next.id);
      ctx->tool.tool_arguments_json = std::move(next.arguments);
      tool_call(&ctx->tool);
    }
    data = conversation.load_conversation( );
    req.set_post_data( data );
    req.request( );
    i++;
  }
  if ( i > 0 ) std::cout << "[tools-called] " << i << '\n';
}

void model::init_async_reply( ) {
  worker = std::jthread{ [&]( ){
    while (model_context.stream.consume()) { }
  }};
}

void model::handle_stream_async( ) {

}

void model::tool_call( tool_context* tool_ctx ) {
  tools.call_tool( tool_ctx, &tool_ctx->result );
  tool_ctx->tool_call = false;
}

void model::set_system_prompt( std::string_view prompt ) {
  // conversation.store( "system", "text", prompt );
  conversation.store_system_prompt( prompt );
}

void make_prompt( std::string& root, std::string_view value, std::string& ret ) {
  size_t ptr = root.find( "?" );
  ret.append( root.data( ), ptr );
  ret.append( value );
  ret.append( std::string_view(root).substr( ptr + 1 ) );
  return;
}

void model::initialize( ) {
  req.initialize( );
  req.set_url(utils::env("OLLAMA_URL", "http://localhost:11434/api/chat"));
  req.set_http_method_post( );
  req.set_body_write_cb( model_stream_callback );
  req.set_body_cb_data( &this->model_context );
  req.set_custom_option_list( "Content-Type", "application/json" );
  req.set_custom_options( );

  model_context.conversation = &this->conversation;
  model_context.tool.context = &this->conversation;
  model_context.ctx = this;
  model_context.buffer.reserve( 1024 * 1024 );
  model_context.response.reserve( 4096 );

  tools.init( );
  tools.load_tools( "tools/" );
  auto tools_info = tools.get_tools( );
  conversation.init( );
  for (const auto& info : *tools_info) {
    tool_data entry(info.doc, info.root);
    conversation.add_tools(&entry, 1);
  }
  init_async_reply( );
  std::string prompt;
  char* path = std::getenv( "SYSTEM_PROMPT_PATH" );
  if ( path == nullptr ) return;
  std::ifstream file{ utils::native_path(path), std::ios::binary };
  if ( file.is_open( ) ) {
    file.seekg( 0, std::ios::end );
    size_t ptr = file.tellg( );
    file.seekg( 0, std::ios::beg );
    prompt.resize( ptr );
    file.read( prompt.data( ), prompt.size( ) );
    set_system_prompt( prompt );
    file.close( );
  }
  // if ( !std::filesystem::exists( "p" ) ) {
  //   std::string prompt;
  //   std::ifstream file{ "gf_prompt" };
  //   if ( file.is_open( ) ) {
  //     file.seekg( 0, std::ios::end );
  //     size_t ptr = file.tellg( );
  //     file.seekg( 0, std::ios::beg );
  //     prompt.resize( ptr );
  //     file.read( prompt.data( ), prompt.size( ) );
  //     set_system_prompt( prompt );
  //     std::ofstream f{ "p" };
  //     if ( f.is_open( ) ) f.close( );
  //     file.close( );
  //   }
  // }
}

size_t write_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) {
  auto* str = static_cast<std::string*>( userdata );
  str->append( ptr, size * nmemb );
  return size * nmemb;
}

size_t header_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) {
  auto* str = static_cast<std::string*>( userdata );
  str->append( ptr, size * nmemb );
  return size * nmemb;
}

size_t model_stream_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto ctx = static_cast<context*>(userdata);
  try {
    ctx->buffer.append(ptr, size * nmemb);
    size_t end;
    while ((end = ctx->buffer.find('\n')) != std::string::npos) {
      std::string line = ctx->buffer.substr(0, end);
      ctx->buffer.erase(0, end + 1);
      if (line.empty()) continue;
      auto doc = std::unique_ptr<yyjson_doc, decltype(&yyjson_doc_free)>(yyjson_read(line.data(), line.size(), 0), yyjson_doc_free);
      if (!doc) throw std::runtime_error("Invalid JSON in Ollama response");
      auto root = yyjson_doc_get_root(doc.get());
      auto error = yyjson_obj_get(root, "error");
      if (yyjson_is_str(error)) throw std::runtime_error(yyjson_get_str(error));
      auto message = yyjson_obj_get(root, "message");
      auto text = [](yyjson_val* value) -> std::string_view {
        return yyjson_is_str(value) ? std::string_view(yyjson_get_str(value), yyjson_get_len(value)) : std::string_view{};
      };
      auto thinking = text(yyjson_obj_get(message, "thinking"));
      if (!thinking.empty()) ctx->thinking(thinking);
      auto content = text(yyjson_obj_get(message, "content"));
      if (!content.empty()) { ctx->response.append(content); ctx->responding(content); }
      auto calls = yyjson_obj_get(message, "tool_calls");
      size_t index, count;
      yyjson_val* call;
      yyjson_arr_foreach(calls, index, count, call) {
        auto function = yyjson_obj_get(call, "function");
        pending_tool next;
        next.name = text(yyjson_obj_get(function, "name"));
        next.id = text(yyjson_obj_get(call, "id"));
        auto arguments = yyjson_obj_get(function, "arguments");
        if (next.name.empty() || !yyjson_is_obj(arguments)) throw std::runtime_error("Invalid tool call from Ollama");
        char* args = yyjson_val_write(arguments, 0, nullptr);
        next.arguments = args; free(args);
        char* raw = yyjson_val_write(call, 0, nullptr);
        std::string stored = raw; free(raw);
        ctx->conversation->store("assistant", "tool_call", stored);
        ctx->pending.push_back(std::move(next));
        ctx->tool.tool_call = true;
      }
      if (yyjson_get_bool(yyjson_obj_get(root, "done"))) {
        ctx->is_responding = false;
        ctx->is_thinking = false;
        if (!ctx->response.empty()) {
          ctx->conversation->store("assistant", "text", ctx->response);
          ctx->response.clear();
        }
        ctx->stream.push("\n");
      }
    }
    return size * nmemb;
  } catch (const std::exception& error) {
    ctx->stream_error = error.what();
    return 0;
  }
}

bool context::is_tool_call( ) {
  return tool.tool_call;
}

void context::parse_json( std::string_view str ) {
  j_str = str;
  json = parser.iterate(j_str).value();
}

void context::thinking( std::string_view str ) {
  if ( is_thinking ) {
    stream.push( THINKING );
    stream.push( str );
    stream.push( RESET );
    // std::cout << THINKING << str << RESET;
  }
  else {
    is_thinking = true;
    stream.push( THINKING );
    stream.push( "thinking: " );
    stream.push( str );
    stream.push( RESET );
    // std::cout << THINKING << "thinking: " << str << RESET;
  }
}

void context::responding( std::string_view str ) {
  if ( is_responding ) {
    stream.push( str );
    // std::cout << str;
  }
  else {
    stream.push( "\n\033[1;36mAssisstant\033[0m-> " );
    stream.push( str );
    // std::cout << "\n\033[1;36mAssisstant\033[0m-> " << str;
    is_responding = true;
  }
}

model::model( ) : model_context( ) {

}

model::~model( ) {
  model_context.stream.close();
  if (worker.joinable()) worker.join();

}
void model::request_response() {
  model_context.stream_error.clear();
  try { req.request(); }
  catch (...) {
    if (!model_context.stream_error.empty()) throw std::runtime_error(model_context.stream_error);
    throw;
  }
  if (!model_context.buffer.empty()) {
    char newline = '\n';
    model_stream_callback(&newline, 1, 1, &model_context);
  }
  if (!model_context.stream_error.empty()) throw std::runtime_error(model_context.stream_error);
  if (req.get_status_code() >= 400) throw std::runtime_error("Ollama HTTP error " + std::to_string(req.get_status_code()));
}
