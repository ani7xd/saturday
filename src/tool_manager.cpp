#include "../include/tool_manager.h"


void tool_manager::get_local_time( std::string& data ) {
  auto now = std::chrono::system_clock::now( );
  auto zt = std::chrono::zoned_time( std::chrono::current_zone( ), now );
  data = std::format( "{:%F %T %Z}", zt );
}

void tool_manager::get_global_time( std::string region ) {

}

void tool_manager::connect_to_tcp( const std::string& ip, short port ) {
  cl.connect( ip, port );
}

void tool_manager::send_data_tcp( std::string_view data, std::string& ret ) {
  cl.send( data );
  cl.recv( );
  ret += buffer;
  buffer.clear( );
}

void tool_manager::tool_open_browser_window_url( std::string url ) {
  int pid = fork( );
  int in[2], out[2];
  if ( pid == 0 ) {
    char* argv[] = {
      "waterfox",
      "--new-tab",
      url.data( ),
      nullptr
    };
    execvp( "waterfox", argv );
    perror( "execvp" );
    exit( -1 );
  }
  else {
    return;
  }
}

void tool_manager::call_tool( tool_context* context, tool_result* ret ) {
  context->json_str = simdjson::padded_string( context->tool_arguments_json );
  context->parser.iterate( context->json_str );
}

void tool_manager::parse_arguements( tool_context* context ) {
  context->json_str = simdjson::padded_string( context->tool_arguments_json );
  context->json = parser.iterate( context->json_str );
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

void tool_manager::image_reverse_search( std::string_view data ) {
  
}

void tool_manager::image_reverse_search( const std::filesystem::path& path ) {

}
  
void tool_manager::image_reverse_search( void* data, size_t len ) {

}

std::string_view tool_manager::trim_web_result_str( std::string_view data ) {
  auto j_str = simdjson::padded_string( data );
  auto j = parser.iterate( j_str );
  this->trimmed_web_result.clear( );
  std::string_view str = j["query"].get_string( ).value( );
  this->trimmed_web_result += "Web Search\n\n";
  this->trimmed_web_result += "Query:\n";
  this->trimmed_web_result.append( str.data( ), str.size( ) );
  this->trimmed_web_result += "\n\nQuick Summary:\n";
  str = j["answer"].get_string( ).value( );
  this->trimmed_web_result.append( str.data( ), str.size( ) );
  this->trimmed_web_result += "\n\nSources:\n";
  auto rets = j["results"].get_array( );

  int i = 1;
  for ( auto r : rets ) {
    this->trimmed_web_result += "\n";
    this->trimmed_web_result += std::to_string( i++ );
    this->trimmed_web_result += ".\n";
    this->trimmed_web_result += "URL:\n";
    str = r["url"].get_string( ).value( );
    this->trimmed_web_result.append( str.data( ), str.size( ) );
    this->trimmed_web_result += "\nTitle:\n";
    str = r["title"].get_string( ).value( );
    this->trimmed_web_result.append( str.data(), str.size( ) );
    this->trimmed_web_result += "\nSnippet:\n";
    str = r["content"].get_string( ).value( );
    this->trimmed_web_result.append( str.data( ), str.size( ) );
    this->trimmed_web_result += "\n";
  }
  return this->trimmed_web_result;
}

std::string_view tool_manager::trim_web_result( std::string_view data ) {
  simdjson::ondemand::parser parser;
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
  this->json = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json, this->len );
}

void tool_manager::edit_file( const std::filesystem::path& file, std::string_view old_text, std::string_view new_text, edit_result* result, size_t r_count, size_t offset ) {
  result->success = false;
  result->file = file;
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
  std::ifstream f{ file };
  if ( f.is_open( ) ) {
    // std::cout << "file opened for editing...\n";
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
    
    std::ofstream out{ file, std::ios::trunc };
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
  auto& p = path.native( );
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  if ( !std::filesystem::exists( path ) ) {
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
      auto nat = entry.path( ).filename( ).native( );
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

void tool_manager::read_file( std::string path, std::string& contents ) {
  if ( !std::filesystem::exists( path ) ) {
    contents = "error: file doesn't exist";
    // store_tool_result_str( "read_file", chat_context.tool_id, "error: file doesn't exist" );
  }
  else {
    std::ifstream file{ path };
    if ( file.is_open( ) ) {
      file.seekg( 0, std::ios::end );
      size_t len = file.tellg( );
      file.seekg( 0, std::ios::beg );
      contents.resize( len );
      file.read( contents.data( ), contents.size( ) );
      // store_tool_result_str( "read_file", chat_context.tool_id, contents );
      file.close( );
    }
    else {
      contents = "error: couldn't open file";
      // store_tool_result_str( "read_file", chat_context.tool_id, "error: couldn't open file" );
    }
  }
}
  
void tool_manager::write_file( std::string path, const std::string& contents ) {
  auto f_path = std::filesystem::path( path );
  if ( !std::filesystem::exists( f_path ) ) {
    if ( f_path.has_parent_path( ) )
      std::filesystem::create_directories( f_path.parent_path( ) );
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      file.write( contents.data( ), contents.size( ) );
      file.close( ); 
    }
  } else {
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      file.write( contents.data( ), contents.size( ) );
      file.close( );
    }
  }
}


void tool_manager::fetch_url( client_worker* client, std::string_view url, fetched_resource* ret ) {
  client->req.set_url( url );
  client->header.clear( );
  client->buffer.clear( );
  client->req.request( );
  size_t ptr; 
  if ( ( ptr = client->header.find( "content-type: " ) ) != std::string::npos ) {
    ptr += 14;
    std::string_view type = client->header.subview( ptr, client->header.find( "\r\n", ptr ) - ptr );
    size_t semi = type.find( ';' );
    if ( semi != std::string_view::npos )
      type = type.subview( 0, semi );
    if ( type == "text/html" ) {
      ret->mime = type;
      ret->body.clear( );
      this->trim_html_result( client->buffer, ret->body );
    }
  }
}

std::string_view tool_manager::trim_html_result( std::string_view data, std::string& out ) {
  lxb_html_document_parse( this->document, reinterpret_cast<const u_char*>( data.data( ) ), data.size( ) );
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

void tool_manager::load_tools( const std::filesystem::path& path ) {
  std::ifstream f;
  auto dir = std::filesystem::recursive_directory_iterator( path );
  std::string tool_str;
  tool_str.reserve( 4096 );
  for ( const auto& entry : dir ) {
    if ( entry.is_regular_file( ) && entry.path( ).extension( ) == ".tool" ) {
      f.open( entry.path( ) );
      if ( f.is_open( ) ) {
        f.seekg( 0, std::ios::end );
        size_t len = f.tellg( );
        f.seekg( 0, std::ios::beg );
        tool_str.resize( len );
        f.read( tool_str.data( ), len );
        auto doc = yyjson_read( tool_str.data( ), tool_str.size( ), 0 ); 
        auto root = yyjson_doc_get_root( doc );
        this->tools.emplace_back( doc, root );
        f.close( );
      }
    }
  }
}

tool_manager::tool_manager( ) {
  this->index = 0;
  this->document = lxb_html_document_create( );
  cl.init( );
  cl.set_receive_cb( write_cb );
  cl.set_receive_cb_ctx( &buffer );
}

tool_manager::~tool_manager( ) {

}

tool_json::~tool_json( ) {
  
}

int write_cb( void* ptr, size_t len, void* ctx ) {
  auto buffer = static_cast<std::string*>( ctx );
  buffer->append( static_cast<char*>( ptr ), len );
  return recv_status::finish;
}


