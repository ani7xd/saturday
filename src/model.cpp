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
  int i = 0;
  while ( model_context.tool.tool_call ) {
    this->tool_call( &model_context.tool );
    i++;
  }
  std::cout << "[tools-called] " << i << '\n';
}

void model::tool_call( tool_context* tool_ctx ) {
  tools.call_tool( tool_ctx, &tool_ctx->result );
}

// void model::handle_tool_call_stream( stream_context* model_context ) {
//   if ( model_context->tool_name == "web_search" ) {
//     calling_tool( "searching web..." );
//     model_context->j_str = simdjson::padded_string( model_context->tool_arguments_json );
//     model_context->json = model_context->parser.iterate( model_context->j_str );
//     // tools.parse_arguements( &model_context->tool );
//     tools.web_search( &this->web, model_context->json["query"].get_string( ).value( ) );
//     std::string_view trimmed_web_result_in_str = tools.trim_web_result_str( web.buffer );
//     model_context->conversation->store_tool_result( model_context->tool_name, model_context->tool_id, trimmed_web_result_in_str );
//   } else if ( model_context->tool_name == "read_file" ) {
//     calling_tool( "reading file..." );
//     std::string contents;
//     tools.parse_arguements( &model_context->tool );
//     tools.read_file( std::string( model_context->json["path"].get_string( ).value( ) ), contents );
//     model_context->conversation->store_tool_result_str( model_context->tool_name, model_context->tool_id, contents );
//   } else if ( model_context->tool_name == "get_current_datetime" ) {
//     calling_tool( "fetching time..." );
//     std::string time_str;
//     tools.get_local_time( time_str );
//     model_context->conversation->store_tool_result_str( model_context->tool_name, model_context->tool_id, time_str );
//   } else if ( model_context->tool_name == "write_file" ) {
//     calling_tool( "writing to file..." );
//     tools.parse_arguements( &model_context->tool );
//     tools.write_file( std::string( model_context->json["path"].get_string( ).value( ) ), std::string( model_context->json["content"].get_string( ).value( ) ) ); 
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, "written successfully" ); // maybe need to upgrade later
//   } else if ( model_context->tool_name == "fetch_url" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string url;
//     url = model_context->json["url"].get_string( ).value( );
//     calling_tool( "fetching url... ( " + url + " )" );
//     tools.fetch_url( &this->client, url, &this->url_result );
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, this->url_result.body );
//   } else if ( model_context->tool_name == "open_browser" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string url;
//     url = model_context->json["url"].get_string( ).value( );
//     calling_tool( "opening url ( " + url + " ) ..." );
//     tools.tool_open_browser_window_url( url );
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, std::string( "opened " ) + url + " successfully in browser" );
//   } else if ( model_context->tool_name == "edit_file" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string path;
//     ssize_t c = 1;
//     size_t o = 0;
//     path = model_context->json["file"].get_string( ).value( );
//     std::string_view old_text, new_text;
//     old_text = model_context->json["old_text"].get_string( ).value( );
//     new_text = model_context->json["new_text"].get_string( ).value( );
//     auto count = model_context->json["occurrence_count"];
//     auto offset = model_context->json["occurrence_offset"];
//     calling_tool( "editing file... ( " + path + " ) ..." );
//     if ( !count.error( ) ) c = count.get_int64( );
//     if ( !offset.error( ) ) o = offset.get_uint64( );
//     edit_result result;
//     tools.edit_file( path, old_text, new_text, &result, c, o ); 
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, std::string_view( result.json, result.len ) );
//     free( result.json );
//   } else if ( model_context->tool_name == "list_dir" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string path;
//     path = model_context->json["path"].get_string( ).value( );
//     calling_tool( "listing dir ( " + path + " )..." );
//     tool_result ret;
//     tools.list_directories( path, &ret );
//     // std::cout << "\nret->\n"; std::cout.write( ret.json, ret.len ); std::cout << "\n";
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, { ret.json, ret.len } );
//     ret.clear( );
//   } else if ( model_context->tool_name == "connect_to_tcp" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string ip;
//     ip = model_context->json["ip"].get_string( ).value( );
//     short port = model_context->json["port"].get_uint32( ).value( );
//     calling_tool( "connecting to ( " + ip + " )..." );
//     std::string ret;
//     tools.connect_to_tcp( ip, port, ret );
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, "connected successfully" );
//   } else if ( model_context->tool_name == "send_data_tcp" ) {
//     tools.parse_arguements( &model_context->tool );
//     std::string data;
//     data = model_context->json["data"].get_string( ).value( );
//     calling_tool( "sending data..." );
//     std::string result;
//     result.append( "Received:\n" );
//     tools.send_data_tcp( data, result );
//     this->conversation.store_tool_result_str( model_context->tool_name, model_context->tool_id, result );
//   }
//   model_context->tool_call = false;
//   std::string_view data = conversation.load_conversation( );
//   req.set_post_data( data );
//   req.request( );
// }

// void model::handle_tool_call(  ) {
//   int i = 0;
//   while ( chat_context.tool_call ) {
//     i++;
//     if ( chat_context.tool_name == "web_search" ) {
//       web_result.clear( );
//       std::cout << TOOL << "searching web...\n" << RESET;
//       chat_context.json_str = simdjson::padded_string( chat_context.tool_arguments_json );
//       chat_context.json = chat_context.parser.iterate( chat_context.json_str );
//       this->websearch( chat_context.json["query"].get_string( ).value( ) );
//       std::string_view trimmed_web_result_in_str = trim_web_result_str( web_result );
//       store_tool_result_str( "web_search", chat_context.tool_id, trimmed_web_result_in_str );
//     } else if ( chat_context.tool_name == "read_file" ) {
//       std::cout << TOOL << "reading file...\n" << RESET;
//       std::string contents;
//       chat_context.json_str = simdjson::padded_string( chat_context.tool_arguments_json );
//       chat_context.json = chat_context.parser.iterate( chat_context.json_str );
//       tool_read_file( std::string( chat_context.json["path"].get_string( ).value( ) ), contents );
//       // store_tool_result( "tool_read_file", contents );
//     } else if ( chat_context.tool_name == "get_local_time" ) {
//       std::cout << TOOL << "fetching time...\n" << RESET;
//       std::string time_str;
//       get_local_time( time_str );
//       store_tool_result_str( "get_local_time", chat_context.tool_id, time_str );
//     } else if ( chat_context.tool_name == "write_file" ) {
//       std::cout << TOOL << "writing to file...\n" << RESET;
//       chat_context.json_str = simdjson::padded_string( chat_context.tool_arguments_json );
//       chat_context.json = chat_context.parser.iterate( chat_context.json_str );
//       tool_write_file( std::string( chat_context.json["path"].get_string( ).value( ) ), std::string( chat_context.json["content"].get_string( ).value( ) ) ); 
//       store_tool_result_str( "write_file", chat_context.tool_id, "written successfully" );
//     } else if ( chat_context.tool_name == "fetch_url" ) {
//       chat_context.json_str = simdjson::padded_string( chat_context.tool_arguments_json );
//       chat_context.json = chat_context.parser.iterate( chat_context.json_str );
//       std::string_view url = chat_context.json["url"].get_string( ).value( );
//       std::cout << TOOL << "fetching url... ( " << url << " )\n" << RESET;
//       tool_fetch_url( url, &this->url_result );
//       store_tool_result_str( chat_context.tool_name, chat_context.tool_id, this->url_result.body );
//     }
//     chat_context.tool_call = false;
//     std::string_view data = conversation.load_conversation( );
//     req.set_post_data( data );
//     req.request( );
//     parse_response( &this->chat_context );
//   }
//   std::cout << "tool called-> " << i << "\n";
// }

void model::set_system_prompt( std::string_view system ) {
  conversation.store( "system", "text", system );
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
  

  // this->chat_context.response_buffer.reserve( 1024 * 1024 );

  // this->speaker.init( );
  // model_stream.speaker = &this->speaker;
  // this->piper = piper_create( "models/en_GB-cori-high.onnx", "models/en_GB-cori-high.onnx.json", "models/espeak-ng-data" );
  // model_stream.audio.piper = this->piper;
  // model_stream.audio.options = piper_default_synthesize_options( piper );
  // model_stream.audio.options.noise_w_scale = 0.8f;
  // model_stream.audio.options.noise_scale = 0.667f;

  

  tools.init( );
  tools.load_tools( "tools/" );
  auto tools_info = tools.get_tools( );
  conversation.init( );
  conversation.add_tools( reinterpret_cast<tool_data*>( tools_info->data( ) ), tools_info->size( ) );
  if ( !std::filesystem::exists( "p" ) ) {
    std::string prompt;
    std::ifstream file{ "gf_prompt" };
    if ( file.is_open( ) ) {
      file.seekg( 0, std::ios::end );
      size_t ptr = file.tellg( );
      file.seekg( 0, std::ios::beg );
      prompt.resize( ptr );
      file.read( prompt.data( ), prompt.size( ) );
      set_system_prompt( prompt );
      std::ofstream f{ "p" };
      if ( f.is_open( ) ) f.close( );
      file.close( );
    }
  }
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


// size_t model_stream_speak_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) { //this is for stream true, dumbass
//   auto context = reinterpret_cast<stream_context*>( userdata );
//   context->full_response.append( ptr, size * nmemb );
//   context->buffer.append( ptr, size * nmemb );
//   size_t end = 0;
//   while ( ( end = context->buffer.find( '\n' ) ) != std::string::npos ) {
//     context->j_str = simdjson::padded_string( context->buffer.subview( 0, end ) );
//     context->json = context->parser.iterate( context->j_str );
//     auto message = context->json["message"];
//     std::string_view role = message["role"].get_string( ).value( );
//     context->think = message["thinking"];
//     if ( !context->think.error( ) ) {
//       if ( context->thinking ) {
//         std::string_view think = context->think.get_string( ).value( );
//         // context->audio.buffer.append( think );
//         // if ( context->audio.buffer.size( ) > 40 ) {
//         //   size_t ptr = std::max( context->audio.buffer.find_last_of( ',' ), context->audio.buffer.find_last_of( '.' ) );
//         //   // reinterpret_cast<model*>( context->ctx )->speak_my_lil_nigga( context->audio.buffer.substr( 0, ptr ) );
//         //   auto audio = context->audio.buffer.substr( 0, ptr );
//         //   auto rc = piper_synthesize_start( context->audio.piper, audio.data( ), &context->audio.options );
//         //   while ( rc != PIPER_DONE ) {
//         //     rc = piper_synthesize_next( context->audio.piper, &context->audio.chunk );
//         //     if ( context->audio.chunk.num_samples > 0 ) {
//         //       context->speaker->speak( context->audio.chunk.samples, context->audio.chunk.num_samples );
//         //     }
//         //   }
//         //   context->audio.buffer.erase( 0, ptr );
//         // }
//         std::cout << THINKING << think << RESET;
//       }
//       else {
//         context->thinking = true;
//         std::string_view think = context->think.get_string( ).value( );
//         // context->audio.buffer.append( think );
//         // if ( context->audio.buffer.size( ) > 40 ) {
//         //   size_t ptr = std::max( context->audio.buffer.find_last_of( ',' ), context->audio.buffer.find_last_of( '.' ) );
//         //   auto audio = context->audio.buffer.substr( 0, ptr );
//         //   auto rc = piper_synthesize_start( context->audio.piper, audio.data( ), &context->audio.options );
//         //   while ( rc != PIPER_DONE ) {
//         //     rc = piper_synthesize_next( context->audio.piper, &context->audio.chunk );
//         //     if ( context->audio.chunk.num_samples > 0 ) {
//         //       context->speaker->speak( context->audio.chunk.samples, context->audio.chunk.num_samples );
//         //     }
//         //   }
//         //   context->audio.buffer.erase( 0, ptr );
//         // }
//         std::cout << THINKING << "thinking: " << think << RESET;
//       }
//     }
//     else {
//       auto tools = message["tool_calls"];
//       if ( !tools.error( ) ) {
//         context->tool_call = true;
//         auto tool = tools.get_array( ).at( 0 );
//         std::string_view json_raw = simdjson::to_json_string( tool ); 
//         context->tool_id = tool["id"].get_string( ).value( );
//         context->tool_arguments_json = simdjson::to_json_string( tool["function"]["arguments"] ).value( );
//         context->tool_name = tool["function"]["name"].get_string( ).value( );
//         context->conversation->store( "assistant", "tool_call", json_raw );
//       }
//       else {
//         if ( !context->tool_call ) {
//           if ( context->responding ) {
//             std::string_view response = message["content"].get_string( ).value( );
//             context->response.append( response );
//             std::cout << response;
//           }
//           else {
//             std::string_view response = message["content"].get_string( ).value( );
//             context->response.append( response );
//             std::cout << "\n\033[1;36mresponse\033[0m-> " << response;
//             context->responding = true;
//           }
//         }
//       }
//     }
//     bool done = context->json["done"].get_bool( ).value( );
//     if ( done ) {
//       context->responding = false;
//       context->thinking = false;
//       if ( !context->tool_call && !context->response.empty( ) ) {
//         context->conversation->store( role, "text", context->response );
//         //reinterpret_cast<model*>( context->ctx )->speak_my_lil_nigga( context->response );
//         // auto rc = piper_synthesize_start( context->audio.piper, context->response.data( ), &context->audio.options );
//         // while ( rc != PIPER_DONE ) {
//         //   rc = piper_synthesize_next( context->audio.piper, &context->audio.chunk );
//         //   if ( context->audio.chunk.num_samples > 0 ) {
//         //     context->speaker->speak( context->audio.chunk.samples, context->audio.chunk.num_samples );
//         //   }
//         // }
//         context->response.clear( );
//       }
//       std::cout << "\n";
//     }
//     context->buffer.erase( 0, end + 1 );
//   }
//   return size * nmemb;
// }

size_t model_stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) { //this is for stream true, dumbass
  auto ctx = reinterpret_cast<context*>( userdata );
  // ctx->full_response.append( ptr, size * nmemb );
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

// void model::parse_response( context* chat ) {
//   chat->json_str = simdjson::padded_string( chat->response_buffer );
//   chat->json = chat->parser.iterate( chat->json_str );
//   std::string_view role = chat->json["message"]["role"].get_string( ).value( );
//   chat->response = chat->json["message"]["content"].get_string( ).value( );
//   auto tools = chat->json["message"]["tool_calls"];
//   if ( !tools.error( ) ) {
//     chat->tool_call = true;
//     auto tool = tools.get_array( ).at( 0 );
//     std::string_view json_raw = simdjson::to_json_string( tool ); 
//     chat->tool_id = tool["id"].get_string( ).value( );
//     chat->tool_arguments_json = simdjson::to_json_string( tool["function"]["arguments"] ).value( );
//     chat->tool_name = tool["function"]["name"].get_string( ).value( );
//     chat->conversation->store( "assistant", "tool_call", json_raw );
//   }
//   bool done = chat->json["done"].get_bool( );
//   if ( done && !chat->tool_call ) {
//     if ( !chat->response.empty( ) ) {
//       chat->conversation->store( role, "text", chat->response );
//       std::cout << "response-> " << chat->response << "\n";
//       chat->response.clear( );
//     }
//   }
//   chat->response_buffer.clear( );
// }

void context::parse_json( std::string_view str ) {
  j_str = str;
  json = parser.iterate( j_str );
}

void context::thinking( std::string_view str ) {
  if ( is_thinking ) {
    std::cout << THINKING << str << RESET;
  }
  else {
    is_thinking = true;
    std::cout << THINKING << "thinking: " << str << RESET;
  }
}
  
void context::responding( std::string_view str ) {
  if ( is_responding ) {
    std::cout << str;
  }
  else {
    std::cout << "\n\033[1;36mresponse\033[0m-> " << str;
    is_responding = true;
  }
}

model::model( ) : model_context( ) {

}

model::~model( ) {

}