#include "../include/tool_manager.h"
#include <ctime>
#ifdef _WIN32
#include <shellapi.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif


void tool_manager::get_local_time( std::string& data ) {
  auto now = std::time(nullptr);
  std::tm local{};
#ifdef _WIN32
  localtime_s(&local, &now);
#else
  localtime_r(&now, &local);
#endif
  char text[128]{};
  std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S %Z", &local);
  data = text;
}

void tool_manager::get_global_time( std::string region ) {

}

void tool_manager::connect_to_tcp( const std::string& ip, uint16_t port, std::string& ret ) {
  if ( cl.connect( ip, port ) == 0 ) {
    ret = "log: connected successfully";
    return;
  }
  ret = "error: failed to connect: ";
  ret += cl.error_str;
}

void tool_manager::send_data_tcp( std::string_view data, std::string& ret ) {
  if ( cl.send( data ) == 0 ) {
    ret = "sent ";
    ret += std::to_string( data.size( ) );
    ret += " bytes";
  }
  else {
    ret = "send failed: ";
    ret += cl.error_str;
  }
}

void tool_manager::call_tool( tool_context* context, tool_result* ret ) {
  auto handler = tool_map.find(context->tool_name);
  if (handler == tool_map.end()) {
    context->context->store_tool_result(context->tool_name, context->tool_id, "error: unknown tool");
    return;
  }
  try { parse_arguements(context); handler->second(context); }
  catch (const std::exception& error) {
    context->context->store_tool_result(context->tool_name, context->tool_id, std::string("error: ") + error.what());
  }
}

void tool_manager::parse_arguements( tool_context* context ) {
  context->json_str = simdjson::padded_string( context->tool_arguments_json );
  context->json = context->parser.iterate(context->json_str).value();
}

void tool_manager::web_search( client_worker* web, std::string_view query ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_strcpy( doc, root, "topic", "general" );
  yyjson_mut_obj_add_strcpy( doc, root, "search_depth", "advanced" );
  yyjson_mut_obj_add_int( doc, root, "max_results", 5 );
  yyjson_mut_obj_add_bool( doc, root, "include_answer", true );
  yyjson_mut_obj_add_bool( doc, root, "include_raw_content", false );
  yyjson_mut_obj_add_bool( doc, root, "include_images", false );
  yyjson_mut_obj_add_strncpy( doc, root, "query", query.data( ), query.size( ) );
  size_t len;
  char* q = yyjson_mut_write( doc, 0, &len );
  web->req.set_post_data( std::string_view( q, len ) );
  web->buffer.clear( );
  web->req.request( );
  free( q );
  yyjson_mut_doc_free( doc );
}

void tool_manager::web_search_google( client_worker* web, std::string_view query ) {
  
}

void tool_manager::image_reverse_search( std::string_view data ) {
  
}

void tool_manager::image_reverse_search( const std::filesystem::path& path ) {

}
  
void tool_manager::image_reverse_search( void* data, size_t len ) {

}

std::string_view tool_manager::trim_web_result_str( std::string_view data, std::string& ret ) {
  auto j_str = simdjson::padded_string( data );
  auto j = parser.iterate( j_str );
  ret.clear( );
  std::string_view str = j["query"].get_string( ).value( );
  ret += "Web Search\n\n";
  ret += "Query:\n";
  ret.append( str.data( ), str.size( ) );
  ret += "\n\nQuick Summary:\n";
  str = j["answer"].get_string( ).value( );
  ret.append( str.data( ), str.size( ) );
  ret += "\n\nSources:\n";
  auto rets = j["results"].get_array( );

  int i = 1;
  for ( auto r : rets ) {
    ret += "\n";
    ret += std::to_string( i++ );
    ret += ".\n";
    ret += "URL:\n";
    str = r["url"].get_string( ).value( );
    ret.append( str.data( ), str.size( ) );
    ret += "\nTitle:\n";
    str = r["title"].get_string( ).value( );
    ret.append( str.data(), str.size( ) );
    ret += "\nSnippet:\n";
    str = r["content"].get_string( ).value( );
    ret.append( str.data( ), str.size( ) );
    ret += "\n";
  }
  return ret;
}

std::string_view tool_manager::trim_web_result_json( std::string_view data ) {
  auto j_str = simdjson::padded_string( data );
  auto j = parser.iterate( j_str );
  std::string_view str;
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  str = j["query"].get_string( ).value( );
  yyjson_mut_obj_add_strncpy( doc, root, "query", str.data( ), str.size( ) );
  str = j["answer"].get_string( ).value( );
  yyjson_mut_obj_add_strncpy( doc, root, "answer", str.data( ), str.size( ) );
  yyjson_mut_val* results = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, root, "results", results );
  
  auto rets = j["results"].get_array( );
  for ( auto r : rets ) {
    yyjson_mut_val* result = yyjson_mut_obj( doc );
    str = r["title"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "title", str.data( ), str.size( ) );
    str = r["url"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "url", str.data( ), str.size( ) );
    str = r["content"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "content", str.data( ), str.size( ) );
    yyjson_mut_arr_add_val( results, result );
  }
  if ( this->json_str != nullptr ) free( this->json_str );
  this->json_str = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json_str, this->len );
}

std::string_view edit_result::form_json( ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_bool( doc, root, "success", this->success );
  yyjson_mut_obj_add_strn( doc, root, "file", this->file.data( ), this->file.size( ) );
  yyjson_mut_obj_add_strn( doc, root, "operation", this->operation.data( ), this->operation.size( ) );
  yyjson_mut_obj_add_uint( doc, root, "occurrence_offset", this->occurrence_offset );
  yyjson_mut_obj_add_uint( doc, root, "requested_count", this->requested_count );
  yyjson_mut_obj_add_uint( doc, root, "replaced_count", this->replaced_count );
  if ( !this->error.empty( ) ) yyjson_mut_obj_add_strn( doc, root, "error", this->error.data( ), this->error.size( ) );
  free(this->json);
  this->json = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json, this->len );
}

void tool_manager::edit_file( const std::filesystem::path& file, std::string_view old_text, std::string_view new_text, edit_result* result, size_t r_count, size_t offset ) {
  result->success = false;
  result->file = utils::path_text(file);
  result->operation = "replace";
  result->occurrence_offset = offset;
  result->requested_count = r_count;
  result->replaced_count = 0;
  result->error.clear( );
  if ( old_text.empty( ) ) {
    result->error = "error: old text field empty";
    result->form_json( );
    return;
  }
  std::ifstream f{ file, std::ios::binary };
  if ( f.is_open( ) ) {
    std::string contents{ std::istreambuf_iterator<char>( f ), std::istreambuf_iterator<char>( ) };
    f.close( );
    size_t count = 0;
    size_t ptr = 0;
    while ( offset > 0 ) {
      if ( ( ptr = contents.find( old_text, ptr ) ) == std::string::npos ) {
        result->error = "error: offset out of range";
        result->form_json( );
        return;
      }
      else {
        offset--;
        ptr += old_text.size( );
      }
    }

    if ( r_count == 0 ) {
      while ( ( ptr = contents.find( old_text, ptr ) ) != std::string::npos ) {
        contents.replace( ptr, old_text.size( ), new_text );
        count++;
        ptr += new_text.size( );
      }
    }
    else {
      while ( ( ptr = contents.find( old_text, ptr ) ) != std::string::npos && r_count > 0 ) {
        r_count--;
        contents.replace( ptr, old_text.size( ), new_text );
        count++;
        ptr += new_text.size( );
      }
    }
    
    std::ofstream out{ file, std::ios::trunc | std::ios::binary };
    if ( out.is_open( ) ) {
      out.write( contents.data( ), contents.size( ) );          // add counter to check for og r_count == count, so to tell llm upto how much count edited
      result->success = true;                                        // maybe also add line and column of where edits happened
      result->replaced_count = count;
      result->form_json( );
      out.close( );                         
    }
    else {
      result->error = "error: failed to write changes to file";
      result->form_json( );
      return;
    }
  }
  else {
    result->error = "error: couldnt open file to read data";
    result->form_json( );
    return;
  }
}

void tool_manager::list_directories( const std::filesystem::path& path, tool_result* ret ) {
  auto p = utils::path_text(path);
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  if ( !std::filesystem::exists(path) ) {
    yyjson_mut_obj_add_bool( doc, root, "success", false );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_obj_add_strcpy( doc, root, "error", "path doesn't exists" );
  }
  else if ( !std::filesystem::is_directory( path ) ) {
    yyjson_mut_obj_add_bool( doc, root, "success", false );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_obj_add_strcpy( doc, root, "error", "path is not a directory" );
  }
  else {
    std::error_code err;
    auto dir = std::filesystem::directory_iterator( path, err );
    if ( err ) {
      yyjson_mut_obj_add_bool( doc, root, "success", false );
      yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
      yyjson_mut_obj_add_strcpy( doc, root, "error", err.message( ).c_str( ) );
      ret->json = yyjson_mut_write( doc, 0, &ret->len );
      yyjson_mut_doc_free( doc );
      return;
    }
    err.clear( );
    yyjson_mut_obj_add_bool( doc, root, "success", true );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_val* entries = yyjson_mut_arr( doc );
    size_t entry_count = 0;
    for ( const auto& entry : dir ) {
      yyjson_mut_val* e = yyjson_mut_obj( doc );  
      auto nat = utils::path_text(entry.path().filename());
      yyjson_mut_obj_add_strncpy( doc, e, "name", nat.data( ), nat.size( ) );
      auto status = entry.symlink_status( err );
      if ( err ) {
        yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
        yyjson_mut_obj_add_strcpy( doc, e, "error", err.message( ).c_str( ) );         
      }
      else {
        switch ( status.type( ) ) {
          case std::filesystem::file_type::regular:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "file" );
            break;
          case std::filesystem::file_type::directory:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "directory" );
            break;
          case std::filesystem::file_type::symlink:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "symlink" );
            break;
          case std::filesystem::file_type::socket:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "socket" );
            break;
          case std::filesystem::file_type::unknown:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
            break;
          default:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
            break;
        };
      }
      yyjson_mut_arr_add_val( entries, e );
      entry_count++;
    }
    yyjson_mut_obj_add_uint( doc, root, "entry_count", entry_count );
    yyjson_mut_obj_add_val( doc, root, "entries", entries );
  }
  ret->json = yyjson_mut_write( doc, 0, &ret->len );
  yyjson_mut_doc_free( doc );
}

void tool_manager::read_file(const std::string& path, std::string& contents) {
  std::ifstream file(utils::native_path(path), std::ios::binary);
  if (!file) { contents = "error: cannot open file"; return; }
  contents.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  if (file.bad()) contents = "error: failed to read file";
}
void tool_manager::write_file(const std::string& path, const std::string& contents) {
  std::string result;
  write_file(path, std::string_view(contents), result);
}
void tool_manager::write_file(const std::string& path, std::string_view contents, std::string& ret) {
  auto native = utils::native_path(path);
  if (native.has_parent_path()) std::filesystem::create_directories(native.parent_path());
  std::ofstream file(native, std::ios::binary | std::ios::app);
  if (!file) { ret = "error: cannot open file for appending"; return; }
  file.write(contents.data(), contents.size());
  file.flush();
  ret = file ? "Appended " + std::to_string(contents.size()) + " bytes" : "error: failed to write file";
}

void tool_manager::fetch_url( client_worker* client, std::string_view url, fetched_resource* ret ) {
  client->req.set_url( url );
  client->header.clear( );
  client->buffer.clear( );
  client->req.request( );
  if (client->req.get_status_code() >= 400) throw std::runtime_error("URL returned HTTP " + std::to_string(client->req.get_status_code()));
  ret->mime = client->req.get_content_type();
  ret->body.clear();
  if (ret->mime.starts_with("text/html")) trim_html_result(client->buffer, ret->body);
  else if (ret->mime.starts_with("text/") || ret->mime.starts_with("application/json")) ret->body = client->buffer;
  else ret->body = "Unsupported content type: " + ret->mime;

}

std::string_view tool_manager::trim_html_result( std::string_view data, std::string& out ) {
  lxb_html_document_parse( this->document, reinterpret_cast<const unsigned char*>( data.data( ) ), data.size( ) );
  this->extract_from_html( lxb_dom_interface_node( this->document ), out );
  return out;
}

void tool_manager::extract_from_html( lxb_dom_node_t* node, std::string& out ) {
  if ( !node )
      return;

  if ( node->type == LXB_DOM_NODE_TYPE_TEXT ) {
    auto* text = lxb_dom_interface_character_data( node );
    out.append( reinterpret_cast<const char*>( text->data.data ), text->data.length );
    out += ' ';
    return;
  }

  if ( node->type == LXB_DOM_NODE_TYPE_ELEMENT ) {
    switch ( lxb_dom_node_tag_id( node ) ) {
      case LXB_TAG_SCRIPT:
      case LXB_TAG_STYLE:
      case LXB_TAG_NOSCRIPT:
      case LXB_TAG_TEMPLATE:
          return;

      case LXB_TAG_P:
      case LXB_TAG_SECTION:
      case LXB_TAG_ARTICLE:
          out += '\n';
          break;

      case LXB_TAG_H1:
          out += "\n# ";
          break;

      case LXB_TAG_H2:
          out += "\n## ";
          break;

      case LXB_TAG_H3:
          out += "\n### ";
          break;

      case LXB_TAG_LI:
          out += "\n- ";
          break;

      case LXB_TAG_BR:
          out += '\n';
          break;

      default:
          break;
    }    
  }

  for ( lxb_dom_node_t* child = node->first_child; child != nullptr; child = child->next ) {
    this->extract_from_html( child, out );
  }
}

std::vector<tool_json>* tool_manager::get_tools( ) {
  return &this->tools;
}

void tool_manager::load_tools_map( ) {
  // a more sexy idea would be to add call_tool() with 
  // unordered map of arguements, and pass parameters to tool
  // after extracting to map from the json array 

  tool_map.emplace("open_browser", [&](tool_context* context) {
    std::string url(context->json["url"].get_string().value());
    if (!url.starts_with("https://") && !url.starts_with("http://")) throw std::runtime_error("Browser URL must use HTTP or HTTPS");
#ifdef _WIN32
    auto path = utils::native_path(url);
    auto result = ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) throw std::runtime_error("Cannot open default browser");
#else
    pid_t pid = fork();
    if (pid == 0) { execlp("xdg-open", "xdg-open", url.c_str(), static_cast<char*>(nullptr)); _exit(127); }
    int status = 0;
    if (pid < 0 || waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) throw std::runtime_error("Cannot open default browser");
#endif
    context->context->store_tool_result(context->tool_name, context->tool_id, "Opened URL in default browser");
  });

  tool_map.emplace( "web_search", [&](tool_context* context) {
    // parse_arguements( context );
    calling_tool( "searching web..." );
    web_search( &web_client, context->json["query"].get_string( ).value( ) );
    std::string_view trimmed_web_result_in_str = trim_web_result_str( web_client.buffer, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });
    
  tool_map.emplace( "read_file", [&](tool_context* context) {
    // parse_arguements( context );
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "reading file ( " + path + " )..." );
    read_file( path, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });

  tool_map.emplace( "get_current_datetime", [&](tool_context* context) {
    calling_tool( "fetching time..." );
    get_local_time( context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.str 
    );
  });

  tool_map.emplace( "write_file", [&](tool_context* context) {
    // parse_arguements( context );
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "writing to file ( " + path + " )..." );
    write_file( 
      path, 
      context->json["content"].get_string( ).value( ),
      context->result.str 
    );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });
  
  tool_map.emplace( "fetch_url", [&](tool_context* context) { 
    // parse_arguements( context );
    std::string url{ context->json["url"].get_string( ).value( ) };
    calling_tool( "fetching url... ( " + url + " )" );
    fetch_url( &page_client, url, context->result.resource );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.resource->body 
    );
  });
  
  tool_map.emplace( "edit_file", [&](tool_context* context) {
    // parse_arguements( context );
    std::string path{ context->json["file"].get_string( ).value( ) };
    size_t c = 1;
    size_t o = 0;
    std::string_view old_text, new_text;
    old_text = context->json["old_text"].get_string( ).value( );
    new_text = context->json["new_text"].get_string( ).value( );
    auto count = context->json["occurrence_count"];
    auto offset = context->json["occurrence_offset"];
    if ( !count.error( ) ) c = count.get_int64( );
    if ( !offset.error( ) ) o = offset.get_uint64( );
    calling_tool( "editing file... ( " + path + " ) ..." );
    edit_file( utils::native_path(path), old_text, new_text, context->result.edit, c, o );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      { context->result.edit->json, context->result.edit->len } 
    );
    context->clear( );
  });
  
  tool_map.emplace( "list_dir", [&](tool_context* context) {
    // parse_arguements( context );
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "listing dir ( " + path + " )..." );
    list_directories( 
      utils::native_path(context->json["path"].get_string().value()),
      &context->result 
    );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      { context->result.json, context->result.len } 
    );
    context->clear( );
  });
  
  tool_map.emplace( "connect_to_tcp", [&](tool_context* context) {
    // parse_arguements( context );
    std::string ip{ context->json["ip"].get_string( ).value( ) };
    auto port_value = context->json["port"].get_uint64().value();
    if (port_value == 0 || port_value > 65535) throw std::runtime_error("TCP port must be 1..65535");
    uint16_t port = static_cast<uint16_t>(port_value);
    calling_tool( "connecting to ( " + ip + ":" + std::to_string( port ) + " )..." );
    connect_to_tcp( ip, port, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.str );
  });
  
  tool_map.emplace( "send_data_tcp", [&](tool_context* context) {
    // parse_arguements( context );
    std::string data;
    data = context->json["data"].get_string( ).value( );
    calling_tool( "sending data..." );
    send_data_tcp( data, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });
}

void tool_manager::load_tools( const std::filesystem::path& path ) {
  load_tools_map( );
  std::ifstream f;
  auto dir = std::filesystem::recursive_directory_iterator( path );
  std::string tool_str;
  tool_str.reserve( 4096 );
  for ( const auto& entry : dir ) {
    if ( entry.is_regular_file( ) && entry.path( ).extension( ) == ".tool" ) {
      f.open(entry.path(), std::ios::binary);
      if ( f.is_open( ) ) {
        f.seekg( 0, std::ios::end );
        size_t len = f.tellg( );
        f.seekg( 0, std::ios::beg );
        if ( len > tool_str.size( ) ) tool_str.resize( len );
        f.read( tool_str.data( ), len );
        auto doc = yyjson_read( tool_str.data( ), len, 0 );
        if (!doc) throw std::runtime_error("Invalid tool definition: " + utils::path_text(entry.path()));
        auto root = yyjson_doc_get_root( doc );
        this->tools.emplace_back( doc, root );
        f.close( );
      }
    }
  }
}

void tool_manager::init( ) {
  cl.init( );
  cl.set_receive_cb( client_write_cb );
  cl.set_receive_cb_ctx( &buffer );
  web_client.req.initialize( );
  web_client.req.set_url( "https://api.tavily.com/search" );
  web_client.req.set_http_method_post( );
  web_client.req.set_custom_option_list( "Content-Type", "application/json" );
  const char* api_key = std::getenv("TAVILY_API_KEY");
  std::string key = api_key ? api_key : "";
  web_client.req.set_custom_option_list( "Authorization", "Bearer " + key );
  web_client.req.set_body_write_cb( web_write_cb );
  web_client.req.set_body_cb_data( &web_client.buffer );
  web_client.req.set_custom_options( );

  page_client.req.initialize( );
  page_client.req.set_body_write_cb( web_write_cb );
  page_client.req.set_body_cb_data( &page_client.buffer );
  page_client.req.set_header_cb_data( &page_client.header );
  page_client.req.set_header_write_cb( web_write_cb );
  page_client.req.set_follow_redirect( true );
}

tool_manager::tool_manager( ) {
  this->index = 0;
  this->document = lxb_html_document_create( );
  
}

tool_manager::~tool_manager( ) {
  lxb_html_document_destroy(document);
  free(json_str);
  for ( auto& tool : tools )
    tool.release( );
}

void tool_json::release( ) {
  yyjson_doc_free( doc );
}

tool_json::~tool_json( ) {
  
}

size_t tool_result::size( ) { 
  return len; 
}
  
char* tool_result::data( ) { 
  return json; 
}
  
void tool_result::clear( ) {
  if ( json != nullptr ) {
    free( json );
    json = nullptr;
  }
  str.clear( );
  len = 0;
};

tool_result::~tool_result( ) {
  free(edit_storage.json);
  if ( json != nullptr ) free( json );
}

size_t web_write_cb( char* ptr, size_t size, size_t nmemb, void* userdata ) {
  auto* str = static_cast<std::string*>( userdata );
  str->append( ptr, size * nmemb );
  return size * nmemb;
}

int client_write_cb( void* ptr, size_t len, void* ctx ) {
  auto buffer = static_cast<std::string*>( ctx );
  buffer->append( static_cast<char*>( ptr ), len );
  return recv_status::finish;
}

void calling_tool( std::string_view str ) {
  std::cout << TOOL << "[tool-call] " << str << RESET << "\n";
}


