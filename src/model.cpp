#include "../include/model.h"

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
  req.request( );
  handle_ctx( &model_context );
}

void model::handle_ctx( context* ctx ) {
  int i = 0;
  std::string_view data;
  while ( ctx->is_tool_call( ) ) {
    tool_call( &ctx->tool );
    data = conversation.load_conversation( );
    req.set_post_data( data );
    req.request( );
    i++;
  }
  if ( i > 0 ) std::cout << "[tools-called] " << i << '\n';
}

void model::init_async_reply( ) {
  worker = std::jthread{ [&]( ){
    while ( true/*!token.stop_requested( )*/ ) {
      model_context.stream.consume( );
    }
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
  ret.append( root.subview( ptr + 1 ) );
  return;
}

void model::initialize( ) {
  req.initialize( );
  req.set_url( "http://localhost:11434/api/chat" );
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
  conversation.add_tools( reinterpret_cast<tool_data*>( tools_info->data( ) ), tools_info->size( ) );
  init_async_reply( );
  std::string prompt;
  char* path = std::getenv( "SYSTEM_PROMPT_PATH" );
  if ( path == nullptr ) return;
  std::ifstream file{ path };
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

size_t model_stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) { //this is for stream true, dumbass
  auto ctx = reinterpret_cast<context*>( userdata );
  ctx->buffer.append( ptr, size * nmemb );
  size_t end = 0;
  while ( ( end = ctx->buffer.find( '\n' ) ) != std::string::npos ) {
    ctx->parse_json( ctx->buffer.subview( 0, end ) );
    auto message = ctx->json["message"];
    std::string_view role = message["role"].get_string( ).value( );
    ctx->think = message["thinking"];
    if ( !ctx->think.error( ) ) {
      ctx->thinking( ctx->think.get_string( ).value( ) );
    }
    else {
      auto tools = message["tool_calls"];
      if ( !tools.error( ) ) {
        ctx->tool.tool_call = true;
        auto tool = tools.get_array( ).at( 0 );
        std::string_view json_raw = simdjson::to_json_string( tool ); 
        ctx->tool.tool_id = tool["id"].get_string( ).value( );
        ctx->tool.tool_arguments_json = simdjson::to_json_string( tool["function"]["arguments"] ).value( );
        ctx->tool.tool_name = tool["function"]["name"].get_string( ).value( );
        ctx->conversation->store( "assistant", "tool_call", json_raw );
      }
      else {
        if ( !ctx->tool.tool_call ) {
          std::string_view response = message["content"].get_string( ).value( );
          ctx->response.append( response );
          // ctx->stream.push( response );
          ctx->responding( response );
        }
      }
    }
    bool done = ctx->json["done"].get_bool( ).value( );
    if ( done ) {
      ctx->is_responding = false;
      ctx->is_thinking = false;
      if ( !ctx->tool.tool_call && !ctx->response.empty( ) ) {
        ctx->conversation->store( role, "text", ctx->response );
        ctx->response.clear( );
      }
      std::cout << "\n";
    }
    ctx->buffer.erase( 0, end + 1 );
  }
  return size * nmemb;
}

bool context::is_tool_call( ) {
  return tool.tool_call;
}

void context::parse_json( std::string_view str ) {
  j_str = str;
  json = parser.iterate( j_str );
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

}